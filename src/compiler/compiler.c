/* Post-parser semantic analysis and portable C11 lowering.
 * Source identifiers never become C identifiers. All per-compilation storage is
 * arena-owned, including the normalized AST; frontend STEs remain borrowed.
 */
#include "compiler.h"
#include <stdarg.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include <errno.h>
#include <math.h>
#include <setjmp.h>

#define CC_MAX_DEPTH 256
#define CC_MAX_SLOTS 1048576

typedef struct Block Block;
typedef struct Ctx Ctx;
typedef struct Lex Lex;
typedef struct Ty Ty;
typedef struct Field Field;
typedef struct RefTy RefTy;
typedef struct Sym Sym;
typedef struct Fn Fn;
typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct Arg Arg;
typedef struct Path Path;

struct Block { Block *next; void *ptr; };
struct Lex { Token tok; const char *text; int line; Lex *next; };
enum { TY_INT, TY_REAL, TY_RECORD, TY_UNION, TY_ALIAS };
struct RefTy { const char *name; int kind, line; Ty *type; };
struct Field {
    const char *name; RefTy ref; Ty *type; int id, offset, line;
    Field *next;
};
struct Ty {
    const char *name; int kind, id, line, state, emitted, slots, height;
    int containsUnion; Field *fields, *tail, *tag, *variant;
    RefTy target; Ty *canonical, *next;
};
struct Sym {
    const char *name; RefTy ref; Ty *type; int id, line, global, mode, offset;
    Sym *next, *paramNext;
};
struct Path { const char *name; Field *field; Path *next; int line; };
enum { EX_VAR, EX_INT, EX_REAL, EX_BINARY, EX_NOT };
struct Expr {
    int kind, line; Token op; const char *text; Expr *left, *right;
    Path *path; Sym *sym; Ty *type; int offset;
};
struct Arg { Expr *expr; Arg *next; };
enum { ST_ASSIGN, ST_READ, ST_WRITE, ST_CALL, ST_IF, ST_WHILE };
struct Stmt {
    int kind, line; Expr *left, *right; Stmt *body, *other, *next;
    const char *callee; Fn *called; Arg *inputs, *outputs;
};
struct Fn {
    const char *name; int id, line, main, slots;
    Sym *symbols, *tail, *inputs, *inTail, *outputs, *outTail;
    Stmt *body; Arg *returns; Fn *next;
};
struct Ctx {
    jmp_buf abort; Block *blocks; Lex *tokens, *tokenTail, *at;
    Ty integer, real, *types, *typeTail;
    Sym *globals, *globalTail; Fn *functions, *fnTail, *current;
    int nextType, nextSym, nextFn, temporary, depth;
    FILE *out;
};

static void fail(Ctx *c, int line, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "compiler: ");
    if (line > 0) fprintf(stderr, "line %d: ", line);
    va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
    fputc('\n', stderr);
    longjmp(c->abort, 1);
}

static void *alloc(Ctx *c, size_t n)
{
    Block *b = (Block *)malloc(sizeof(*b));
    void *p = calloc(1, n ? n : 1);
    if (!b || !p) { free(b); free(p); fail(c, 0, "out of memory"); }
    b->ptr = p; b->next = c->blocks; c->blocks = b;
    return p;
}

static char *fmt(Ctx *c, const char *format, ...)
{
    va_list ap, copy; int n; char *p;
    va_start(ap, format); va_copy(copy, ap);
    n = vsnprintf(NULL, 0, format, copy); va_end(copy);
    if (n < 0) { va_end(ap); fail(c, 0, "formatting error"); }
    p = (char *)alloc(c, (size_t)n + 1);
    vsnprintf(p, (size_t)n + 1, format, ap); va_end(ap);
    return p;
}

static void enter(Ctx *c, int line)
{
    if (++c->depth > CC_MAX_DEPTH) fail(c, line, "nesting exceeds compiler limit (%d)", CC_MAX_DEPTH);
}
static void leave(Ctx *c) { --c->depth; }

static void flatten(Ctx *c, pTreeNode *node)
{
    typedef struct Pending { pTreeNode *node; struct Pending *next; } Pending;
    Pending *stack = (Pending *)alloc(c, sizeof(*stack));
    stack->node = node;
    /* Grammar list productions are right-recursive, not semantic nesting. */
    while (stack) {
        int i; Lex *l;
        node = stack->node; stack = stack->next;
        if (!node || !node->symbol) fail(c, 0, "incomplete parse tree");
        if (node->symbol->isNonTerminal) {
            if (node->size < 1 || !node->children) fail(c, node->lineNumber, "incomplete parse tree");
            for (i = node->size - 1; i >= 0; --i) {
                Pending *p = (Pending *)alloc(c, sizeof(*p));
                p->node = node->children[i]; p->next = stack; stack = p;
            }
        } else if (node->symbol->tOrNt.t != EPS && node->symbol->tOrNt.t != DOLLAR) {
            if (!node->ste) fail(c, node->lineNumber, "terminal lacks token information");
            l = (Lex *)alloc(c, sizeof(*l));
            l->tok = node->symbol->tOrNt.t; l->text = node->ste->lexeme;
            l->line = node->lineNumber;
            if (c->tokenTail) c->tokenTail->next = l; else c->tokens = l;
            c->tokenTail = l;
        }
    }
}

static Token peek(Ctx *c) { return c->at ? c->at->tok : DOLLAR; }
static Lex *take(Ctx *c, Token t)
{
    Lex *l = c->at;
    if (!l || l->tok != t) fail(c, l ? l->line : 0, "malformed parse tree: expected token %d, found %s", (int)t, l ? l->text : "end of tree");
    c->at = l->next; return l;
}
static int accept(Ctx *c, Token t)
{
    if (peek(c) != t) return 0;
    take(c, t); return 1;
}

static RefTy parseType(Ctx *c, int field)
{
    RefTy r; Lex *l = c->at;
    memset(&r, 0, sizeof(r)); r.kind = -1; r.line = l ? l->line : 0;
    if (accept(c, INT)) { r.type = &c->integer; return r; }
    if (accept(c, REAL)) { r.type = &c->real; return r; }
    if (!field && (peek(c) == RECORD || peek(c) == UNION)) {
        r.kind = take(c, peek(c))->tok == RECORD ? TY_RECORD : TY_UNION;
    }
    r.name = take(c, RUID)->text;
    return r;
}

static Ty *findType(Ctx *c, const char *name)
{
    Ty *t;
    for (t = c->types; t; t = t->next) if (!strcmp(t->name, name)) return t;
    return NULL;
}

static Ty *newType(Ctx *c, Lex *name, int kind)
{
    Ty *t;
    if (findType(c, name->text)) fail(c, name->line, "duplicate type or alias '%s'", name->text);
    t = (Ty *)alloc(c, sizeof(*t)); t->name = name->text;
    t->line = name->line; t->kind = kind; t->id = ++c->nextType;
    if (c->typeTail) c->typeTail->next = t; else c->types = t;
    c->typeTail = t;
    return t;
}

static void parseTypes(Ctx *c)
{
    while (peek(c) == RECORD || peek(c) == UNION || peek(c) == DEFINETYPE) {
        Ty *t;
        if (accept(c, DEFINETYPE)) {
            int k = peek(c) == RECORD ? TY_RECORD : TY_UNION;
            RefTy r; Lex *name;
            if (peek(c) != RECORD && peek(c) != UNION) take(c, RECORD);
            take(c, peek(c)); memset(&r, 0, sizeof(r));
            name = take(c, RUID); r.name = name->text; r.line = name->line; r.kind = k;
            take(c, AS); t = newType(c, take(c, RUID), TY_ALIAS); t->target = r;
        } else {
            Token start = take(c, peek(c))->tok; int count = 0;
            t = newType(c, take(c, RUID), start == RECORD ? TY_RECORD : TY_UNION);
            while (accept(c, TYPE)) {
                Field *f = (Field *)alloc(c, sizeof(*f)), *old;
                Lex *name;
                f->ref = parseType(c, 1); take(c, COLON); name = take(c, FIELDID);
                f->name = name->text; f->line = name->line; f->id = ++count;
                for (old = t->fields; old; old = old->next)
                    if (!strcmp(old->name, f->name)) fail(c, f->line, "duplicate field '%s' in '%s'", f->name, t->name);
                take(c, SEM);
                if (t->tail) t->tail->next = f; else t->fields = f;
                t->tail = f;
            }
            if (count < 2) fail(c, t->line, "type '%s' needs at least two fields", t->name);
            take(c, start == RECORD ? ENDRECORD : ENDUNION);
        }
    }
}

static Sym *newSym(Ctx *c, Fn *fn, RefTy r, Lex *name, int mode, int global)
{
    Sym *s = (Sym *)alloc(c, sizeof(*s));
    s->name = name->text; s->line = name->line; s->ref = r;
    s->id = ++c->nextSym; s->mode = mode; s->global = global;
    if (global) {
        if (c->globalTail) c->globalTail->next = s; else c->globals = s;
        c->globalTail = s;
    } else {
        if (fn->tail) fn->tail->next = s; else fn->symbols = s;
        fn->tail = s;
    }
    if (mode == 1) {
        if (fn->inTail) fn->inTail->paramNext = s; else fn->inputs = s;
        fn->inTail = s;
    } else if (mode == 2) {
        if (fn->outTail) fn->outTail->paramNext = s; else fn->outputs = s;
        fn->outTail = s;
    }
    return s;
}

static void parseParams(Ctx *c, Fn *fn, int mode)
{
    take(c, mode == 1 ? INPUT : OUTPUT); take(c, PARAMETER); take(c, LIST); take(c, SQL);
    do {
        RefTy r = parseType(c, 0);
        newSym(c, fn, r, take(c, ID), mode, 0);
    } while (accept(c, COMMA));
    take(c, SQR);
}

static Expr *expression(Ctx *c);
static Expr *boolean(Ctx *c);
static Stmt *statements(Ctx *c);

static Expr *nodeExpr(Ctx *c, int kind, int line)
{
    Expr *e = (Expr *)alloc(c, sizeof(*e)); e->kind = kind; e->line = line; return e;
}

static Expr *variable(Ctx *c)
{
    Lex *l = c->at; Expr *e;
    if (peek(c) == NUM || peek(c) == RNUM) {
        take(c, l->tok); e = nodeExpr(c, l->tok == NUM ? EX_INT : EX_REAL, l->line);
        e->text = l->text; return e;
    }
    l = take(c, ID); e = nodeExpr(c, EX_VAR, l->line); e->text = l->text;
    {
        Path **tail = &e->path;
        while (accept(c, DOT)) {
            Path *p = (Path *)alloc(c, sizeof(*p)); l = take(c, FIELDID);
            p->name = l->text; p->line = l->line; *tail = p; tail = &p->next;
        }
    }
    return e;
}

static Expr *factorExpr(Ctx *c)
{
    Expr *e;
    enter(c, c->at ? c->at->line : 0);
    if (accept(c, OP)) { e = expression(c); take(c, CL); }
    else e = variable(c);
    leave(c); return e;
}

static Expr *binaryExpr(Ctx *c, Expr *left, Token op, Expr *right)
{
    Expr *e = nodeExpr(c, EX_BINARY, left->line);
    e->left = left; e->right = right; e->op = op; return e;
}

static Expr *termExpr(Ctx *c)
{
    Expr *e = factorExpr(c);
    while (peek(c) == MUL || peek(c) == DIV) {
        Token op = take(c, peek(c))->tok; Expr *r = factorExpr(c);
        e = binaryExpr(c, e, op, r);
    }
    return e;
}

static Expr *expression(Ctx *c)
{
    Expr *e = termExpr(c);
    while (peek(c) == PLUS || peek(c) == MINUS) {
        Token op = take(c, peek(c))->tok; Expr *r = termExpr(c);
        e = binaryExpr(c, e, op, r);
    }
    return e;
}

static Expr *boolean(Ctx *c)
{
    Expr *e, *r; Token op;
    enter(c, c->at ? c->at->line : 0);
    if (accept(c, NOT)) {
        e = nodeExpr(c, EX_NOT, c->at ? c->at->line : 0);
        take(c, OP); e->left = boolean(c); take(c, CL);
    } else if (accept(c, OP)) {
        e = boolean(c); take(c, CL);
        if (peek(c) != AND && peek(c) != OR) take(c, AND);
        op = take(c, peek(c))->tok; take(c, OP); r = boolean(c); take(c, CL);
        e = binaryExpr(c, e, op, r);
    } else {
        e = variable(c); op = peek(c);
        if (op != LT && op != LE && op != EQ && op != GT && op != GE && op != NE) take(c, EQ);
        take(c, op); r = variable(c); e = binaryExpr(c, e, op, r);
    }
    leave(c); return e;
}

static Arg *idArgs(Ctx *c)
{
    Arg *head = NULL, **tail = &head;
    take(c, SQL);
    do {
        Lex *l = take(c, ID); Arg *a = (Arg *)alloc(c, sizeof(*a));
        a->expr = nodeExpr(c, EX_VAR, l->line); a->expr->text = l->text;
        *tail = a; tail = &a->next;
    } while (accept(c, COMMA));
    take(c, SQR); return head;
}

static Stmt *statement(Ctx *c)
{
    Stmt *s = (Stmt *)alloc(c, sizeof(*s));
    s->line = c->at ? c->at->line : 0;
    enter(c, s->line);
    if (peek(c) == ID) {
        s->kind = ST_ASSIGN; s->left = variable(c); take(c, ASSIGNOP);
        s->right = expression(c); take(c, SEM);
    } else if (peek(c) == READ || peek(c) == WRITE) {
        s->kind = take(c, peek(c))->tok == READ ? ST_READ : ST_WRITE;
        take(c, OP); s->left = variable(c); take(c, CL); take(c, SEM);
    } else if (peek(c) == CALL || peek(c) == SQL) {
        s->kind = ST_CALL;
        if (peek(c) == SQL) { s->outputs = idArgs(c); take(c, ASSIGNOP); }
        take(c, CALL); s->callee = take(c, FUNID)->text;
        take(c, WITH); take(c, PARAMETERS); s->inputs = idArgs(c); take(c, SEM);
    } else if (accept(c, WHILE)) {
        s->kind = ST_WHILE; take(c, OP); s->left = boolean(c); take(c, CL);
        s->body = statements(c); take(c, ENDWHILE);
        if (!s->body) fail(c, s->line, "empty loop in parse tree");
    } else if (accept(c, IF)) {
        s->kind = ST_IF; take(c, OP); s->left = boolean(c); take(c, CL); take(c, THEN);
        s->body = statements(c);
        if (accept(c, ELSE)) s->other = statements(c);
        take(c, ENDIF);
        if (!s->body) fail(c, s->line, "empty conditional in parse tree");
    } else fail(c, s->line, "unexpected statement token '%s'", c->at ? c->at->text : "end of tree");
    leave(c); return s;
}

static Stmt *statements(Ctx *c)
{
    Stmt *head = NULL, **tail = &head;
    while (peek(c) == ID || peek(c) == READ || peek(c) == WRITE ||
           peek(c) == CALL || peek(c) == SQL || peek(c) == WHILE || peek(c) == IF) {
        Stmt *s = statement(c); *tail = s; tail = &s->next;
    }
    return head;
}

static void parseFunction(Ctx *c, int mainFn)
{
    Lex *name = take(c, mainFn ? MAIN : FUNID); Fn *old;
    Fn *f = (Fn *)alloc(c, sizeof(*f));
    f->name = name->text; f->line = name->line; f->main = mainFn; f->id = ++c->nextFn;
    for (old = c->functions; old; old = old->next)
        if (!strcmp(old->name, f->name)) fail(c, f->line, "duplicate function '%s'", f->name);
    if (c->fnTail) c->fnTail->next = f; else c->functions = f;
    c->fnTail = f;
    if (!mainFn) {
        parseParams(c, f, 1);
        if (peek(c) == OUTPUT) parseParams(c, f, 2);
        take(c, SEM);
    }
    parseTypes(c);
    while (accept(c, TYPE)) {
        RefTy r = parseType(c, 0); Lex *l; int global = 0;
        take(c, COLON); l = take(c, ID);
        if (accept(c, COLON)) { take(c, GLOBAL); global = 1; }
        take(c, SEM); newSym(c, f, r, l, 0, global);
    }
    f->body = statements(c); take(c, RETURN);
    if (peek(c) == SQL) f->returns = idArgs(c);
    take(c, SEM); take(c, END);
}

static Ty *resolve(Ctx *c, Ty *t);
static Ty *resolveRef(Ctx *c, RefTy *r)
{
    Ty *t = r->type;
    if (!t) {
        t = findType(c, r->name);
        if (!t) fail(c, r->line, "unknown type '%s'", r->name);
        t = resolve(c, t);
        if (r->kind >= 0 && t->kind != r->kind)
            fail(c, r->line, "'%s' is not a %s type", r->name, r->kind == TY_RECORD ? "record" : "union");
        r->type = t;
    }
    return t;
}

static Ty *resolve(Ctx *c, Ty *t)
{
    Field *f; int slots;
    if (t->state == 2) return t->canonical;
    if (t->state == 1) fail(c, t->line, "cyclic alias or by-value type involving '%s'", t->name);
    enter(c, t->line); t->state = 1;
    if (t->kind == TY_ALIAS) t->canonical = resolveRef(c, &t->target);
    else {
        t->canonical = t; slots = t->kind == TY_UNION ? 2 : 0;
        for (f = t->fields; f; f = f->next) {
            f->type = resolveRef(c, &f->ref); f->offset = slots;
            if (f->type->height >= CC_MAX_DEPTH) fail(c, t->line, "constructed type nesting exceeds compiler limit (%d)", CC_MAX_DEPTH);
            if (f->type->height + 1 > t->height) t->height = f->type->height + 1;
            if (f->type->slots > CC_MAX_SLOTS - slots) fail(c, t->line, "type '%s' is too large", t->name);
            slots += f->type->slots;
            if (f->type->kind == TY_UNION || f->type->containsUnion) t->containsUnion = 1;
            if (t->kind == TY_RECORD && !strcmp(f->name, "tagvalue")) t->tag = f;
            if (t->kind == TY_RECORD && f->type->kind == TY_UNION) {
                if (t->variant) fail(c, t->line, "record '%s' has multiple unions sharing one tagvalue", t->name);
                t->variant = f;
            }
            if (t->kind == TY_UNION && (f->type->kind == TY_UNION || f->type->containsUnion))
                fail(c, f->line, "union '%s' contains another union; nested union layouts are unsupported", t->name);
        }
        t->slots = slots;
        if (t->variant && (!t->tag || (t->tag->type->kind != TY_INT && t->tag->type->kind != TY_REAL)))
            fail(c, t->line, "variant record '%s' requires a primitive sibling field named tagvalue", t->name);
    }
    t->state = 2; leave(c); return t->canonical;
}

static void checkSymbols(Ctx *c)
{
    Sym *s, *p; Fn *f;
    for (s = c->globals; s; s = s->next) {
        for (p = c->globals; p != s; p = p->next)
            if (!strcmp(p->name, s->name)) fail(c, s->line, "duplicate global '%s'", s->name);
        s->type = resolveRef(c, &s->ref);
        if (s->type->kind == TY_UNION) fail(c, s->line, "standalone union '%s' lacks a sibling tagvalue", s->name);
    }
    for (f = c->functions; f; f = f->next) {
        for (s = f->symbols; s; s = s->next) {
            for (p = c->globals; p; p = p->next)
                if (!strcmp(p->name, s->name)) fail(c, s->line, "local or parameter '%s' shadows a global", s->name);
            for (p = f->symbols; p != s; p = p->next)
                if (!strcmp(p->name, s->name)) fail(c, s->line, "duplicate variable or parameter '%s'", s->name);
            s->type = resolveRef(c, &s->ref);
            if (s->type->kind == TY_UNION) fail(c, s->line, "standalone union '%s' lacks a sibling tagvalue", s->name);
            s->offset = f->slots;
            if (s->type->slots > CC_MAX_SLOTS - f->slots) fail(c, s->line, "function scope is too large");
            f->slots += s->type->slots;
        }
    }
}

static Sym *lookup(Ctx *c, const char *name, int line)
{
    Sym *s;
    for (s = c->current->symbols; s; s = s->next) if (!strcmp(s->name, name)) return s;
    for (s = c->globals; s; s = s->next) if (!strcmp(s->name, name)) return s;
    fail(c, line, "undeclared variable '%s' in '%s'", name, c->current->name);
    return NULL;
}

static int primitive(Ty *t) { return t->kind == TY_INT || t->kind == TY_REAL; }

static void bindVar(Ctx *c, Expr *e)
{
    Path *p; Ty *t; int offset = 0;
    if (e->kind != EX_VAR) fail(c, e->line, "destination must be a variable or record field");
    e->sym = lookup(c, e->text, e->line); t = e->sym->type;
    for (p = e->path; p; p = p->next) {
        Field *f;
        if (primitive(t)) fail(c, p->line, "field '%s' selected from a primitive", p->name);
        for (f = t->fields; f; f = f->next) if (!strcmp(f->name, p->name)) break;
        if (!f) fail(c, p->line, "type '%s' has no field '%s'", t->name, p->name);
        p->field = f; offset += f->offset; t = f->type;
    }
    e->type = t; e->offset = offset;
}

/* Union slots carry aggregate-initialization and tag-synchronization bits.
 * Branch intersection can prove a variant initialized even when branches choose
 * different members. A synchronized tag also permits safe partial updates of a
 * copied variant: a successful member check proves its storage is unchanged.
 */
static int initialized(Ty *t, const unsigned char *bits)
{
    Field *f;
    if (primitive(t) || t->kind == TY_UNION) return bits[0] != 0;
    for (f = t->fields; f; f = f->next)
        if (!initialized(f->type, bits + f->offset)) return 0;
    return 1;
}

/* A whole variant value guarantees its active member, not every overlapping
 * member. Keep those leaf proofs unknown until individual fields are written.
 */
static void setInitialized(Ty *t, unsigned char *bits)
{
    Field *f;
    if (primitive(t)) { bits[0] = 1; return; }
    if (t->kind == TY_UNION) {
        memset(bits, 0, (size_t)t->slots); bits[0] = bits[1] = 1; return;
    }
    for (f = t->fields; f; f = f->next) setInitialized(f->type, bits + f->offset);
}

static void needRef(Ctx *c, Expr *e, unsigned char *bits, int readValue)
{
    Path *p; Ty *t = e->sym->type; int off = e->sym->offset, covered = 0;
    if (e->sym->global) return;
    for (p = e->path; p; p = p->next) {
        Field *f = p->field;
        if (t->kind == TY_RECORD && f->type->kind == TY_UNION) {
            if (!initialized(t->tag->type, bits + off + t->tag->offset))
                fail(c, e->line, "tagvalue for '%s' may be uninitialized", e->text);
        }
        if (t->kind == TY_UNION && bits[off]) covered = 1;
        off += f->offset; t = f->type;
    }
    if (readValue && !covered && !initialized(e->type, bits + e->sym->offset + e->offset))
        fail(c, e->line, "variable or field '%s' may be uninitialized", e->text);
}

static void markRef(Expr *e, unsigned char *bits)
{
    Path *p; Ty *t = e->sym->type; int off = e->sym->offset;
    Ty *unions[CC_MAX_DEPTH];
    int bases[CC_MAX_DEPTH], members[CC_MAX_DEPTH], preserved[CC_MAX_DEPTH], n = 0, i;
    if (e->sym->global) return;
    /* Entering a different member invalidates proofs about old union storage. */
    for (p = e->path; p; p = p->next) {
        Field *f = p->field;
        if (t->kind == TY_RECORD && t->variant && f == t->tag)
            bits[off + t->variant->offset + 1] = 0;
        if (t->kind == TY_UNION) {
            Field *other;
            preserved[n] = bits[off] && bits[off + 1];
            unions[n] = t; bases[n] = off; members[n++] = f->id;
            for (other = t->fields; other; other = other->next)
                if (other != f) memset(bits + off + other->offset, 0, (size_t)other->type->slots);
            bits[off] = 0; bits[off + 1] = 1;
        }
        off += f->offset; t = f->type;
    }
    setInitialized(t, bits + off);
    for (i = n - 1; i >= 0; --i) {
        Field *f;
        for (f = unions[i]->fields; f; f = f->next) if (f->id == members[i]) break;
        bits[bases[i]] = (unsigned char)(preserved[i] || initialized(f->type, bits + bases[i] + f->offset));
    }
}

static Ty *checkExpr(Ctx *c, Expr *e, unsigned char *bits)
{
    Ty *a, *b; int op;
    enter(c, e->line);
    if (e->kind == EX_VAR) {
        bindVar(c, e); needRef(c, e, bits, 1);
    } else if (e->kind == EX_INT) {
        char *end; uintmax_t value;
        errno = 0; value = strtoumax(e->text, &end, 10);
        if (errno || *end || value > INT64_MAX) fail(c, e->line, "integer literal exceeds int64_t range: %s", e->text);
        e->type = &c->integer;
    } else if (e->kind == EX_REAL) {
        char *end; double value;
        errno = 0; value = strtod(e->text, &end);
        if (errno || *end || !isfinite(value)) fail(c, e->line, "real literal outside double range: %s", e->text);
        e->type = &c->real;
    } else if (e->kind == EX_NOT) {
        checkExpr(c, e->left, bits); e->type = &c->integer;
    } else {
        a = checkExpr(c, e->left, bits); b = checkExpr(c, e->right, bits); op = e->op;
        if (op == AND || op == OR) e->type = &c->integer;
        else if (op == LT || op == LE || op == EQ || op == GT || op == GE || op == NE) {
            if (!primitive(a) || a != b) fail(c, e->line, "comparison requires matching primitive types");
            e->type = &c->integer;
        } else if (primitive(a) && primitive(b)) {
            if (a != b) fail(c, e->line, "arithmetic operands must have identical primitive types");
            e->type = a;
        } else if ((op == PLUS || op == MINUS) && a == b && a->kind == TY_RECORD) {
            if (a->containsUnion) fail(c, e->line, "arithmetic on variant records is not defined");
            e->type = a;
        } else if ((op == MUL || op == DIV) && a->kind == TY_RECORD && primitive(b)) {
            if (a->containsUnion) fail(c, e->line, "arithmetic on variant records is not defined");
            e->type = a;
        } else if (op == MUL && primitive(a) && b->kind == TY_RECORD) {
            if (b->containsUnion) fail(c, e->line, "arithmetic on variant records is not defined");
            e->type = b;
        } else fail(c, e->line, "incompatible arithmetic operand types '%s' and '%s'", a->name, b->name);
    }
    leave(c); return e->type;
}

static Fn *findFunction(Ctx *c, const char *name)
{
    Fn *f;
    for (f = c->functions; f; f = f->next) if (!strcmp(f->name, name)) return f;
    return NULL;
}

static void checkStatements(Ctx *c, Stmt *s, unsigned char *bits)
{
    for (; s; s = s->next) {
        enter(c, s->line);
        if (s->kind == ST_ASSIGN) {
            Ty *r = checkExpr(c, s->right, bits);
            bindVar(c, s->left); needRef(c, s->left, bits, 0);
            if (s->left->type != r)
                fail(c, s->line, "assignment requires identical types (got '%s', expected '%s')", r->name, s->left->type->name);
            markRef(s->left, bits);
        } else if (s->kind == ST_READ) {
            bindVar(c, s->left); needRef(c, s->left, bits, 0); markRef(s->left, bits);
        } else if (s->kind == ST_WRITE) {
            if (s->left->kind == EX_VAR) { bindVar(c, s->left); needRef(c, s->left, bits, 1); }
            else checkExpr(c, s->left, bits);
        } else if (s->kind == ST_CALL) {
            Fn *f = findFunction(c, s->callee); Arg *a; Sym *p;
            if (!f) fail(c, s->line, "unknown function '%s'", s->callee);
            if (f->id >= c->current->id || f->main)
                fail(c, s->line, "call to '%s' must target an earlier function (no recursion)", s->callee);
            s->called = f;
            for (a = s->inputs, p = f->inputs; a && p; a = a->next, p = p->paramNext)
                if (checkExpr(c, a->expr, bits) != p->type) fail(c, a->expr->line, "input type mismatch in call to '%s'", f->name);
            if (a || p) fail(c, s->line, "input arity mismatch in call to '%s'", f->name);
            for (a = s->outputs, p = f->outputs; a && p; a = a->next, p = p->paramNext) {
                bindVar(c, a->expr);
                if (a->expr->type != p->type) fail(c, a->expr->line, "output type mismatch in call to '%s'", f->name);
            }
            if (a || p) fail(c, s->line, "output arity mismatch in call to '%s'", f->name);
            for (a = s->outputs; a; a = a->next) markRef(a->expr, bits);
        } else {
            unsigned char *branch, *other; int i, n = c->current->slots;
            branch = (unsigned char *)alloc(c, (size_t)n);
            if (s->kind == ST_IF) {
                checkExpr(c, s->left, bits);
                memcpy(branch, bits, (size_t)n); checkStatements(c, s->body, branch);
                other = (unsigned char *)alloc(c, (size_t)n); memcpy(other, bits, (size_t)n);
                checkStatements(c, s->other, other);
                for (i = 0; i < n; ++i) bits[i] = (unsigned char)(branch[i] && other[i]);
            } else {
                int changed;
                /* Zero iterations add no facts. Union member switches can
                 * remove facts, so solve the loop invariant to a fixed point,
                 * checking reads again under the weaker back-edge state.
                 */
                do {
                    changed = 0; checkExpr(c, s->left, bits);
                    memcpy(branch, bits, (size_t)n); checkStatements(c, s->body, branch);
                    for (i = 0; i < n; ++i) {
                        unsigned char value = (unsigned char)(bits[i] && branch[i]);
                        if (value != bits[i]) changed = 1;
                        bits[i] = value;
                    }
                } while (changed);
            }
        }
        leave(c);
    }
}

static void checkFunctions(Ctx *c)
{
    Fn *f;
    for (f = c->functions; f; f = f->next) {
        Sym *s; Arg *a; unsigned char *bits = (unsigned char *)alloc(c, (size_t)f->slots);
        c->current = f;
        for (s = f->inputs; s; s = s->paramNext) setInitialized(s->type, bits + s->offset);
        checkStatements(c, f->body, bits);
        for (s = f->outputs, a = f->returns; s && a; s = s->paramNext, a = a->next) {
            if (checkExpr(c, a->expr, bits) != s->type)
                fail(c, a->expr->line, "return type mismatch for output '%s'", s->name);
        }
        if (s || a) fail(c, f->line, "return arity does not match output parameters in '%s'", f->name);
        for (s = f->outputs; s; s = s->paramNext)
            if (!initialized(s->type, bits + s->offset)) fail(c, s->line, "output '%s' is not definitely initialized on every path", s->name);
    }
    c->current = NULL;
}

#include "assembly.inc"
