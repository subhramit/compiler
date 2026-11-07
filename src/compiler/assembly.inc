/* Direct GAS Intel backend. Semantic offsets are initialization proofs only;
 * physical layouts below use overlapping union storage plus an active word.
 * Every expression is spilled, so no value survives a call in volatile regs.
 */
#if !defined(__x86_64__) && !defined(_M_X64)
#error "The assembly backend requires an x86-64 host"
#endif
#if !defined(_WIN32) && !defined(__linux__)
#error "The assembly backend supports Windows COFF and Linux ELF only"
#endif
#ifdef _WIN32
#define A1 "rcx"
#define A2 "rdx"
#define SHADOW 32
#else
#define A1 "rdi"
#define A2 "rsi"
#define SHADOW 0
#endif

typedef struct Layout Layout;
struct Layout { Ty *type; int size; Layout *next; };
typedef struct { int off; Sym *global; } Address;
typedef struct {
    Ctx *c; Layout *layouts; int used, label; int *symbols;
    FILE *scratch;
} Assembly;

static void out(Ctx *c, const char *format, ...)
{
    va_list ap; va_start(ap, format);
    if (vfprintf(c->out, format, ap) < 0) {
        va_end(ap); fail(c, 0, "failed to write assembly");
    }
    va_end(ap);
}
static int layout(Assembly *a, Ty *t)
{
    Layout *l; Field *f; int n = 0;
    if (primitive(t)) return 8;
    for (l = a->layouts; l; l = l->next) if (l->type == t) return l->size;
    for (f = t->fields; f; f = f->next) {
        int size = layout(a, f->type);
        if (t->kind == TY_UNION) { if (size > n) n = size; }
        else {
            if (n > INT_MAX - size) fail(a->c, t->line, "physical layout too large");
            n += size;
        }
    }
    if (t->kind == TY_UNION) n += 8;
    l = alloc(a->c, sizeof(*l)); l->type = t; l->size = n;
    l->next = a->layouts; a->layouts = l; return n;
}
static int fieldOffset(Assembly *a, Ty *t, Field *wanted)
{
    Field *f; int n = 0;
    if (t->kind == TY_UNION) return 0;
    for (f = t->fields; f != wanted; f = f->next) n += layout(a, f->type);
    return n;
}
static Address plus(Address p, int n) { p.off += n; return p; }
static Address fieldAddress(Assembly *a, Address p, Ty *t, Field *f)
{ return plus(p, fieldOffset(a, t, f)); }
static char *address(Assembly *a, Address p)
{
    if (p.global) return fmt(a->c, "rip + .Lglobal%d + %d", p.global->id, p.off);
    return fmt(a->c, "rbp %+d", p.off);
}
static Address slot(Assembly *a, int n)
{
    Address p;
    if (n < 0 || a->used > INT_MAX - n - 48)
        fail(a->c, 0, "assembly stack frame too large");
    a->used += n; p.off = -a->used; p.global = NULL; return p;
}
static Address symbol(Assembly *a, Sym *s)
{
    Address p; p.global = s->global ? s : NULL;
    p.off = s->global ? 0 : a->symbols[s->id]; return p;
}
static void load(Assembly *a, const char *reg, Address p)
{ out(a->c, "    mov %s, QWORD PTR [%s]\n", reg, address(a, p)); }
static void store(Assembly *a, Address p, const char *reg)
{ out(a->c, "    mov QWORD PTR [%s], %s\n", address(a, p), reg); }
static void zero(Assembly *a, Address p, int size)
{
    int l = ++a->label;
    if (!size) return;
    out(a->c, "    lea r10, [%s]\n    mov r11, %d\n    xor eax, eax\n.Lzero%d:\n"
        "    mov QWORD PTR [r10], rax\n    add r10, 8\n    dec r11\n    jnz .Lzero%d\n",
        address(a, p), size / 8, l, l);
}
/* Copies only between disjoint temporaries, locals, and call vectors. */
static void copy(Assembly *a, Address dest, Address src, int size)
{
    int l = ++a->label;
    if (!size) return;
    out(a->c, "    lea r10, [%s]\n    lea r11, [%s]\n    mov ecx, %d\n.Lcopy%d:\n"
        "    mov rax, QWORD PTR [r10]\n    mov QWORD PTR [r11], rax\n"
        "    add r10, 8\n    add r11, 8\n    dec ecx\n    jnz .Lcopy%d\n",
        address(a, src), address(a, dest), size / 8, l, l);
}
enum { ERR_ADD, ERR_SUB, ERR_MUL, ERR_DIV, ERR_ZERO, ERR_REAL,
       ERR_RANGE, ERR_TAG, ERR_MEMBER, ERR_ACTIVE, ERR_COUNT };
static const char *const errors[] = {
    "integer addition overflow", "integer subtraction overflow",
    "integer multiplication overflow", "integer division overflow",
    "division by zero", "nonfinite real arithmetic",
    "record scalar result outside int64_t range", "invalid union tagvalue",
    "inactive union member access", "union member has not been initialized for this tagvalue"
};
/* The condition names the successful path; all error sites retain source lines. */
static void require(Assembly *a, const char *condition, int err, int line)
{
    int l = ++a->label;
    out(a->c, "    j%s .Lok%d\n    mov " A1 ", %d\n"
        "    lea " A2 ", [rip + .Lerror%d]\n    call cc_error\n    ud2\n.Lok%d:\n",
        condition, l, line, err, l);
}
static void finite(Assembly *a, int line)
{
    out(a->c, "    movq rax, xmm0\n    mov r10, 0x7ff0000000000000\n"
        "    and rax, r10\n    cmp rax, r10\n");
    require(a, "ne", ERR_REAL, line);
}
/* Produces an integral 1-based tag in rax. */
static void tagValue(Assembly *a, Address tag, Ty *type, Ty *u, int line)
{
    load(a, "rax", tag);
    if (type->kind == TY_REAL) {
        out(a->c, "    movq xmm0, rax\n    cvttsd2si rax, xmm0\n"
            "    cvtsi2sd xmm1, rax\n    ucomisd xmm0, xmm1\n");
        require(a, "np", ERR_TAG, line); require(a, "e", ERR_TAG, line);
    }
    out(a->c, "    cmp rax, 1\n"); require(a, "ge", ERR_TAG, line);
    out(a->c, "    cmp rax, %d\n", u->tail->id); require(a, "le", ERR_TAG, line);
}
static void active(Assembly *a, Address base, Ty *u, int line)
{
    out(a->c, "    cmp QWORD PTR [%s], rax\n", address(a, plus(base, layout(a, u) - 8)));
    require(a, "e", ERR_ACTIVE, line);
}
static void validate(Assembly *a, Ty *t, Address base, int line)
{
    Field *f;
    if (primitive(t)) return;
    for (f = t->fields; f; f = f->next) {
        Address p = fieldAddress(a, base, t, f);
        if (f->type->kind == TY_UNION) {
            tagValue(a, fieldAddress(a, base, t, t->tag), t->tag->type, f->type, line);
            active(a, p, f->type, line);
        } else validate(a, f->type, p, line);
    }
}
static Address reference(Assembly *a, Expr *e, int writing, Address *tag, Ty **tagType)
{
    Path *p; Ty *t = e->sym->type; Address base = symbol(a, e->sym);
    Address sibling = base; Ty *st = NULL;
    for (p = e->path; p; p = p->next) {
        Field *f = p->field;
        if (t->kind == TY_RECORD && f->type->kind == TY_UNION) {
            sibling = fieldAddress(a, base, t, t->tag); st = t->tag->type;
        }
        if (t->kind == TY_UNION) {
            tagValue(a, sibling, st, t, e->line);
            out(a->c, "    cmp rax, %d\n", f->id); require(a, "e", ERR_MEMBER, e->line);
            if (writing) {
                int l = ++a->label;
                out(a->c, "    cmp QWORD PTR [%s], %d\n    je .Lsame%d\n",
                    address(a, plus(base, layout(a, t) - 8)), f->id, l);
                zero(a, base, layout(a, f->type));
                out(a->c, "    mov QWORD PTR [%s], %d\n.Lsame%d:\n",
                    address(a, plus(base, layout(a, t) - 8)), f->id, l);
            } else active(a, base, t, e->line);
        }
        base = fieldAddress(a, base, t, f); t = f->type;
    }
    if (tag) *tag = sibling;
    if (tagType) *tagType = st;
    return base;
}
static void scalar(Assembly *a, Ty *dest, Address d, Address x, Ty *xt,
                   Token op, Address y, Ty *yt, int line)
{
    load(a, "rax", x); load(a, "r11", y);
    if (dest->kind == TY_INT && xt->kind == TY_INT && yt->kind == TY_INT) {
        if (op == DIV) {
            int l = ++a->label;
            out(a->c, "    test r11, r11\n"); require(a, "ne", ERR_ZERO, line);
            out(a->c, "    mov r10, 0x8000000000000000\n    cmp rax, r10\n"
                "    jne .Ldivide%d\n    cmp r11, -1\n", l);
            require(a, "ne", ERR_DIV, line);
            out(a->c, ".Ldivide%d:\n    cqo\n    idiv r11\n", l);
        } else {
            out(a->c, "    %s rax, r11\n", op == PLUS ? "add" : op == MINUS ? "sub" : "imul");
            require(a, "no", op == PLUS ? ERR_ADD : op == MINUS ? ERR_SUB : ERR_MUL, line);
        }
    } else {
        out(a->c, xt->kind == TY_INT ? "    cvtsi2sd xmm0, rax\n" : "    movq xmm0, rax\n");
        out(a->c, yt->kind == TY_INT ? "    cvtsi2sd xmm1, r11\n" : "    movq xmm1, r11\n");
        if (op == DIV) {
            out(a->c, "    xorpd xmm2, xmm2\n    ucomisd xmm1, xmm2\n");
            require(a, "ne", ERR_ZERO, line);
        }
        out(a->c, "    %s xmm0, xmm1\n", op == PLUS ? "addsd" : op == MINUS ? "subsd" : op == MUL ? "mulsd" : "divsd");
        finite(a, line);
        if (dest->kind == TY_INT) {
            out(a->c, "    mov rax, 0xc3e0000000000000\n    movq xmm1, rax\n"
                "    ucomisd xmm0, xmm1\n"); require(a, "ae", ERR_RANGE, line);
            out(a->c, "    mov rax, 0x43e0000000000000\n    movq xmm1, rax\n"
                "    ucomisd xmm0, xmm1\n"); require(a, "b", ERR_RANGE, line);
            out(a->c, "    cvttsd2si rax, xmm0\n");
        } else out(a->c, "    movq rax, xmm0\n");
    }
    store(a, d, "rax");
}
static void recordArithmetic(Assembly *a, Ty *t, Address d, Address x, Ty *xt,
                             Token op, Address y, Ty *yt, int line)
{
    Field *f;
    if (primitive(t)) { scalar(a, t, d, x, xt, op, y, yt, line); return; }
    for (f = t->fields; f; f = f->next)
        recordArithmetic(a, f->type, fieldAddress(a, d, t, f),
            primitive(xt) ? x : fieldAddress(a, x, xt, f), primitive(xt) ? xt : f->type, op,
            primitive(yt) ? y : fieldAddress(a, y, yt, f), primitive(yt) ? yt : f->type, line);
}
static Address expressionAsm(Assembly *a, Expr *e)
{
    Address d = slot(a, layout(a, e->type)), x, y;
    Ctx *c = a->c;
    if (e->kind == EX_VAR) {
        Address tag; Ty *tt;
        x = reference(a, e, 0, &tag, &tt);
        if (e->type->kind == TY_UNION) { tagValue(a, tag, tt, e->type, e->line); active(a, x, e->type, e->line); }
        else validate(a, e->type, x, e->line);
        copy(a, d, x, layout(a, e->type));
    } else if (e->kind == EX_INT || e->kind == EX_REAL) {
        uint64_t bits;
        if (e->kind == EX_INT) bits = (uint64_t)strtoumax(e->text, NULL, 10);
        else { double r = strtod(e->text, NULL); memcpy(&bits, &r, 8); }
        out(c, "    mov rax, 0x%016" PRIx64 "\n", bits); store(a, d, "rax");
    } else if (e->kind == EX_NOT) {
        x = expressionAsm(a, e->left); load(a, "rax", x);
        out(c, "    test rax, rax\n    sete al\n    movzx eax, al\n"); store(a, d, "rax");
    } else if (e->op == AND || e->op == OR) {
        int l = ++a->label;
        x = expressionAsm(a, e->left); load(a, "rax", x);
        out(c, "    test rax, rax\n    setne al\n    movzx eax, al\n"); store(a, d, "rax");
        out(c, "    test eax, eax\n    j%s .Lshort%d\n", e->op == AND ? "z" : "nz", l);
        y = expressionAsm(a, e->right); load(a, "rax", y);
        out(c, "    test rax, rax\n    setne al\n    movzx eax, al\n"); store(a, d, "rax");
        out(c, ".Lshort%d:\n", l);
    } else {
        x = expressionAsm(a, e->left); y = expressionAsm(a, e->right);
        if (e->op == LT || e->op == LE || e->op == EQ || e->op == GT || e->op == GE || e->op == NE) {
            const char *condition;
            int real = e->left->type->kind == TY_REAL;
            load(a, "rax", x); load(a, "r11", y);
            if (real) out(c, "    movq xmm0, rax\n    movq xmm1, r11\n    ucomisd xmm0, xmm1\n");
            else out(c, "    cmp rax, r11\n");
            condition = e->op == LT ? (real ? "b" : "l") : e->op == LE ? (real ? "be" : "le") :
                e->op == GT ? (real ? "a" : "g") : e->op == GE ? (real ? "ae" : "ge") : e->op == EQ ? "e" : "ne";
            out(c, "    set%s al\n    movzx eax, al\n", condition); store(a, d, "rax");
        } else recordArithmetic(a, e->type, d, x, e->left->type, e->op, y, e->right->type, e->line);
    }
    return d;
}
static void ioAsm(Assembly *a, Ty *t, Address base, int reading, int line, Address tag, Ty *tt)
{
    Field *f; Ctx *c = a->c;
    if (primitive(t)) {
        if (reading) {
            out(c, "    mov " A1 ", %d\n    call cc_read_%s\n", line, t->kind == TY_INT ? "int" : "real");
            store(a, base, "rax");
        } else {
            load(a, A1, base);
            out(c, "    mov " A2 ", %d\n    call cc_write_%s\n", line, t->kind == TY_INT ? "int" : "real");
        }
    } else if (t->kind == TY_UNION) {
        int end = ++a->label; Address choice = slot(a, 8);
        tagValue(a, tag, tt, t, line); store(a, choice, "rax");
        if (reading) {
            zero(a, base, layout(a, t)); load(a, "rax", choice);
            store(a, plus(base, layout(a, t) - 8), "rax");
        } else active(a, base, t, line);
        for (f = t->fields; f; f = f->next) {
            int next = ++a->label;
            load(a, "rax", choice);
            out(c, "    cmp rax, %d\n    jne .Lnext%d\n", f->id, next);
            ioAsm(a, f->type, base, reading, line, base, NULL);
            out(c, "    jmp .Lioend%d\n.Lnext%d:\n", end, next);
        }
        out(c, ".Lioend%d:\n", end);
    } else {
        if (t->variant) ioAsm(a, t->tag->type, fieldAddress(a, base, t, t->tag), reading, line, base, NULL);
        for (f = t->fields; f; f = f->next) {
            if (t->variant && f == t->tag) continue;
            ioAsm(a, f->type, fieldAddress(a, base, t, f), reading, line,
                t->tag ? fieldAddress(a, base, t, t->tag) : base, t->tag ? t->tag->type : NULL);
        }
    }
}
static void assignAsm(Assembly *a, Expr *e, Address value, int line)
{
    Address tag, dest; Ty *tt;
    dest = reference(a, e, 1, &tag, &tt);
    if (e->type->kind == TY_UNION) {
        tagValue(a, tag, tt, e->type, line); active(a, value, e->type, line);
    }
    copy(a, dest, value, layout(a, e->type));
}
static int vectorSize(Assembly *a, Sym *s)
{
    int n = 0;
    for (; s; s = s->paramNext) {
        int size = layout(a, s->type);
        if (n > INT_MAX - size) fail(a->c, s->line, "argument vector too large");
        n += size;
    }
    return n;
}
static void statementsAsm(Assembly *a, Stmt *s)
{
    Ctx *c = a->c;
    for (; s; s = s->next) {
        Address x, y, tag; Ty *tt;
        if (s->kind == ST_ASSIGN) {
            y = expressionAsm(a, s->right); assignAsm(a, s->left, y, s->line);
        } else if (s->kind == ST_READ || s->kind == ST_WRITE) {
            int reading = s->kind == ST_READ;
            tt = NULL; tag.off = 0; tag.global = NULL;
            if (s->left->kind == EX_VAR) {
                x = reference(a, s->left, reading, &tag, &tt);
                if (!reading && s->left->type->kind != TY_UNION) validate(a, s->left->type, x, s->line);
            } else x = expressionAsm(a, s->left);
            ioAsm(a, s->left->type, x, reading, s->line, tag, tt);
        } else if (s->kind == ST_CALL) {
            Arg *arg; int off = 0;
            Address ins = slot(a, vectorSize(a, s->called->inputs));
            Address outs = slot(a, vectorSize(a, s->called->outputs));
            for (arg = s->inputs; arg; arg = arg->next) {
                x = expressionAsm(a, arg->expr);
                copy(a, plus(ins, off), x, layout(a, arg->expr->type)); off += layout(a, arg->expr->type);
            }
            out(c, "    lea " A1 ", [%s]\n    lea " A2 ", [%s]\n    call .Lfn%d\n",
                address(a, ins), address(a, outs), s->called->id);
            off = 0;
            for (arg = s->outputs; arg; arg = arg->next) {
                assignAsm(a, arg->expr, plus(outs, off), s->line); off += layout(a, arg->expr->type);
            }
        } else if (s->kind == ST_IF) {
            int other = ++a->label, end = ++a->label;
            x = expressionAsm(a, s->left); load(a, "rax", x);
            out(c, "    test rax, rax\n    jz .Lbranch%d\n", other);
            statementsAsm(a, s->body);
            out(c, "    jmp .Lbranch%d\n.Lbranch%d:\n", end, other);
            statementsAsm(a, s->other); out(c, ".Lbranch%d:\n", end);
        } else {
            int start = ++a->label, end = ++a->label;
            out(c, ".Lloop%d:\n", start); x = expressionAsm(a, s->left); load(a, "rax", x);
            out(c, "    test rax, rax\n    jz .Lloop%d\n", end); statementsAsm(a, s->body);
            out(c, "    jmp .Lloop%d\n.Lloop%d:\n", start, end);
        }
    }
}
static void functionAsm(Assembly *a, Fn *f)
{
    Ctx *c = a->c; FILE *dest = c->out; Sym *s; Arg *arg;
    int off, frame, ch, i, n = 0; Address *returns;
    Address inptr = { -8, NULL }, outptr = { -16, NULL };
    a->used = 16; c->current = f;
    for (s = f->symbols; s; s = s->next) a->symbols[s->id] = slot(a, layout(a, s->type)).off;
    a->scratch = tmpfile();
    if (!a->scratch) fail(c, 0, "cannot create assembly staging stream");
    c->out = a->scratch;
    /* Inputs are copied into local values before any user statement. */
    off = 0;
    for (s = f->inputs; s; s = s->paramNext) {
        Address temp = slot(a, layout(a, s->type)); int l = ++a->label;
        load(a, "r10", inptr);
        out(c, "    add r10, %d\n    lea r11, [%s]\n    mov ecx, %d\n.Linput%d:\n"
            "    mov rax, QWORD PTR [r10]\n    mov QWORD PTR [r11], rax\n"
            "    add r10, 8\n    add r11, 8\n    dec ecx\n    jnz .Linput%d\n",
            off, address(a, temp), layout(a, s->type) / 8, l, l);
        copy(a, symbol(a, s), temp, layout(a, s->type)); off += layout(a, s->type);
    }
    statementsAsm(a, f->body);
    for (arg = f->returns; arg; arg = arg->next) ++n;
    returns = alloc(c, (size_t)n * sizeof(*returns));
    for (arg = f->returns, i = 0; arg; arg = arg->next) returns[i++] = expressionAsm(a, arg->expr);
    off = 0;
    for (s = f->outputs, i = 0; s; s = s->paramNext, ++i) {
        int l = ++a->label;
        load(a, "r11", outptr);
        out(c, "    add r11, %d\n    lea r10, [%s]\n    mov ecx, %d\n.Loutput%d:\n"
            "    mov rax, QWORD PTR [r10]\n    mov QWORD PTR [r11], rax\n"
            "    add r10, 8\n    add r11, 8\n    dec ecx\n    jnz .Loutput%d\n",
            off, address(a, returns[i]), layout(a, s->type) / 8, l, l);
        off += layout(a, s->type);
    }
    if (f->main) out(c, "    call cc_finish\n    xor eax, eax\n");
    frame = (a->used + SHADOW + 15) & ~15;
    out(c, "    add rsp, %d\n    pop rbp\n    ret\n", frame);
    c->out = dest;
    if (f->main) out(c, "\n.globl main\n");
#ifdef _WIN32
    out(c, ".seh_proc %s\n", f->main ? "main" : fmt(c, ".Lfn%d", f->id));
#endif
    if (f->main) out(c, "main:\n");
    else out(c, ".Lfn%d:\n", f->id);
    out(c, "    push rbp\n");
#ifdef _WIN32
    out(c, "    .seh_pushreg rbp\n");
#endif
    /* RBP addresses slots but is not the unwind frame register: it is set
     * before allocation, so describing it as final RSP + 0 would be wrong. */
    out(c, "    mov rbp, rsp\n");
    /* Probe without changing RSP so faults anywhere in this fixed-size
     * prologue unwind using only the saved RBP. The loop keeps even very
     * large frames within Win64's 255-byte prologue limit. */
    if (frame >= 4096) {
        int l = ++a->label;
        out(c, "    mov r10, rsp\n    mov r11, %d\n.Lprobe%d:\n    sub r10, 4096\n"
            "    mov BYTE PTR [r10], 0\n    dec r11\n    jnz .Lprobe%d\n", frame / 4096, l, l);
        if (frame % 4096)
            out(c, "    sub r10, %d\n    mov BYTE PTR [r10], 0\n", frame % 4096);
    }
    out(c, "    sub rsp, %d\n", frame);
#ifdef _WIN32
    out(c, "    .seh_stackalloc %d\n    .seh_endprologue\n", frame);
#endif
    { Address bottom = { -a->used, NULL }; zero(a, bottom, a->used); }
    if (!f->main) { store(a, inptr, A1); store(a, outptr, A2); }
    if (fflush(a->scratch) || fseek(a->scratch, 0, SEEK_SET)) fail(c, 0, "assembly staging stream failure");
    while ((ch = fgetc(a->scratch)) != EOF) if (fputc(ch, dest) == EOF) fail(c, 0, "failed to write assembly");
    if (ferror(a->scratch)) fail(c, 0, "assembly staging read failure");
#ifdef _WIN32
    out(c, ".seh_endproc\n");
#endif
    fclose(a->scratch); a->scratch = NULL; c->current = NULL;
}
static void programAsm(Assembly *a)
{
    Ctx *c = a->c; Sym *s; Fn *f; int i;
    a->symbols = alloc(c, ((size_t)c->nextSym + 1) * sizeof(*a->symbols));
    out(c, ".intel_syntax noprefix\n.bss\n");
    for (s = c->globals; s; s = s->next)
        out(c, ".balign 8\n.Lglobal%d:\n    .zero %d\n", s->id, layout(a, s->type));
#ifdef _WIN32
    out(c, ".section .rdata,\"dr\"\n");
#else
    out(c, ".section .rodata\n");
#endif
    for (i = 0; i < ERR_COUNT; ++i) out(c, ".Lerror%d:\n    .asciz \"%s\"\n", i, errors[i]);
    out(c, ".text\n");
    for (f = c->functions; f; f = f->next) functionAsm(a, f);
#ifndef _WIN32
    out(c, ".section .note.GNU-stack,\"\",@progbits\n");
#endif
}

static int compileProgram(pTree *tree, FILE *output, int emitting)
{
    Ctx *c = calloc(1, sizeof(*c)); Assembly *a = calloc(1, sizeof(*a));
    volatile int result = 1; Block *b; Ty *t;
    if (!c || !a) { free(c); free(a); fputs("compiler: out of memory\n", stderr); return 1; }
    a->c = c;
    if (setjmp(c->abort)) goto done;
    if (!tree || !tree->root || (emitting && !output))
        fail(c, 0, "compiler requires a parse tree and, for emission, an output stream");
    c->integer.name = "int"; c->integer.kind = TY_INT; c->integer.slots = 1;
    c->integer.state = 2; c->integer.canonical = &c->integer;
    c->real.name = "real"; c->real.kind = TY_REAL; c->real.slots = 1;
    c->real.state = 2; c->real.canonical = &c->real;
    flatten(c, tree->root); c->at = c->tokens;
    while (peek(c) == FUNID) parseFunction(c, 0);
    parseFunction(c, 1);
    if (c->at) fail(c, c->at->line, "tokens after main function");
    for (t = c->types; t; t = t->next) resolve(c, t);
    checkSymbols(c); checkFunctions(c);
    if (emitting) {
        c->out = output; programAsm(a);
        if (fflush(output) == EOF || ferror(output)) fail(c, 0, "failed to write assembly");
    }
    result = 0;
done:
    if (a->scratch) fclose(a->scratch);
    while ((b = c->blocks) != NULL) { c->blocks = b->next; free(b->ptr); free(b); }
    free(a); free(c); return result;
}
int emitAssemblyProgram(pTree *tree, FILE *output) { return compileProgram(tree, output, 1); }
int checkProgram(pTree *tree) { return compileProgram(tree, NULL, 0); }
#undef A1
#undef A2
#undef SHADOW
