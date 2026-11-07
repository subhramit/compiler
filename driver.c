/* Command-line compiler and the original interactive frontend driver. */
#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "src/parser/parser.h"
#include "src/compiler/compiler.h"
#include "src/platform/platform.h"
#ifdef _WIN32
#include <io.h>
#include <process.h>

#define close_fd _close
#define fd_stream _fdopen
#define process_id _getpid
#else
#include <unistd.h>

#define close_fd close
#define fd_stream fdopen
#define process_id getpid
#endif
#ifndef DEFAULT_CC
#define DEFAULT_CC "cc"
#endif
#ifndef DEFAULT_RUNTIME
#define DEFAULT_RUNTIME "build/runtime.o"
#endif

bool shouldPrint = true;

static void usage(FILE *out)
{
    fputs("Usage:\n"
          "  compiler SOURCE -o EXECUTABLE [--cc TOOLCHAIN] [--runtime RUNTIME.o]\n"
          "  compiler --check SOURCE\n"
          "  compiler --emit-asm SOURCE -o OUTPUT.s\n"
          "  compiler --legacy SOURCE TREE_OUTPUT\n"
          "  compiler --help\n\n"
          "TOOLCHAIN is one GCC/Clang/Zig executable, used only to assemble and link.\n"
          "The backend emits x86-64 machine instructions, not C or LLVM IR.\n"
          "Runtime precedence: --runtime, RUNTIME environment, build-time default.\n"
          "Precedence: --cc, CC environment variable, build-time default.\n"
          "Zig's cc subcommand is added automatically. No shell is used.\n"
          "Two bare positional arguments retain the original interactive menu.\n",
          out);
}

static int legacy(const char *input, const char *output)
{
    int choice = -1;
    puts("\nCompiler frontend: lexer and predictive parser with error recovery.");
    while (choice) {
        puts("\nSelect your option:\n 0: Exit\n 1: Remove comments\n"
             " 2: Print tokens\n 3: Parse and print the parse tree\n 4: Time frontend analysis");
        if (scanf("%d", &choice) != 1) return 0;
        shouldPrint = true;
        switch (choice) {
        case 0: return 0;
        case 1: removeComments((char *)input, (char *)output); break;
        case 2: {
            FILE *fp = fopen(input, "rb");
            if (!fp) { perror(input); return 1; }
            linkedList *tokens = LexInput(fp, (char *)output);
            if (tokens) printTokensOnConsole(tokens);
            fclose(fp);
            if (tokens) {
                tokenInfo *node = tokens->head;
                while (node) { tokenInfo *next = node->next; free(node); node = next; }
                free(tokens);
            }
            break;
        }
        case 3: parseInputSourceCode((char *)input, (char *)output); break;
        case 4: {
            clock_t start = clock();
            shouldPrint = false;
            parseInputSourceCode((char *)input, (char *)output);
            printf("Frontend CPU time: %.6f seconds\n", (double)(clock() - start) / CLOCKS_PER_SEC);
            break;
        }
        default: fputs("Please enter a valid option.\n", stderr); break;
        }
    }
    return 0;
}

/* Reserve siblings of the requested destination, so publication stays on the
 * same filesystem and failed compilations never replace an existing output. */
static FILE *temporary(const char *destination, const char *suffix, char **name)
{
    static unsigned counter;
    size_t capacity = strlen(destination) + strlen(suffix) + 80;
    char *path = malloc(capacity);
    if (!path) return NULL;
    for (unsigned attempt = 0; attempt < 100; ++attempt) {
        /* Defeat compiler option/response-file interpretation of relative paths. */
        const char *prefix = destination[0] == '-' || destination[0] == '@' ? "./" : "";
        snprintf(path, capacity, "%s%s.tmp-%ld-%u%s", prefix, destination,
                 (long)process_id(), ++counter, suffix);
#ifdef _WIN32
        int fd = _open(path, _O_CREAT | _O_EXCL | _O_RDWR | _O_BINARY, _S_IREAD | _S_IWRITE);
#else
        int fd = open(path, O_CREAT | O_EXCL | O_RDWR, 0600);
#endif
        if (fd >= 0) {
            FILE *stream = fd_stream(fd, "w+b");
            if (!stream) { close_fd(fd); remove(path); free(path); return NULL; }
            *name = path;
            return stream;
        }
        if (errno != EEXIST) break;
    }
    free(path);
    return NULL;
}



static int isZig(const char *path)
{
    const char *base = path;
    for (const char *p = path; *p; ++p) if (*p == '/' || *p == '\\') base = p + 1;
#ifdef _WIN32
    return _stricmp(base, "zig") == 0 || _stricmp(base, "zig.exe") == 0;
#else
    return strcmp(base, "zig") == 0;
#endif
}

static int runToolchain(const char *cc, const char *source, const char *runtime, const char *output)
{
    const char *args[16]; int n = 0;
    args[n++] = cc;
    if (isZig(cc)) args[n++] = "cc";
    args[n++] = "-g0";
    if (isZig(cc)) args[n++] = "-s";
    args[n++] = "-x"; args[n++] = "assembler";
    args[n++] = source;
    args[n++] = "-x"; args[n++] = "none";
    args[n++] = runtime;
    args[n++] = "-o"; args[n++] = output;
    args[n++] = "-lm";
    args[n] = NULL;
    return executeProgram(cc, args);
}

int main(int argc, char **argv)
{
    const char *source = NULL, *output = NULL, *cc = NULL, *runtime = NULL;
    enum { NATIVE, CHECK, EMIT_ASM } mode = NATIVE;
    int errors = 0, result = 1;
    char *assembly = NULL, *binary = NULL;
    FILE *stream = NULL;
    pTree *tree = NULL;
    if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        usage(stdout); return 0;
    }
    if (argc == 4 && strcmp(argv[1], "--legacy") == 0) return legacy(argv[2], argv[3]);
    if (argc == 3 && argv[1][0] != '-' && argv[2][0] != '-') return legacy(argv[1], argv[2]);
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--check") == 0 || strcmp(argv[i], "--emit-asm") == 0) {
            if (mode != NATIVE) goto badUsage;
            mode = strcmp(argv[i], "--check") == 0 ? CHECK : EMIT_ASM;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (output || ++i >= argc || !*argv[i]) goto badUsage;
            output = argv[i];
        } else if (strcmp(argv[i], "--cc") == 0) {
            if (cc || ++i >= argc || !*argv[i]) goto badUsage;
            cc = argv[i];
        } else if (strcmp(argv[i], "--runtime") == 0) {
            if (runtime || ++i >= argc || !*argv[i]) goto badUsage;
            runtime = argv[i];
        } else if (argv[i][0] == '-' || source) goto badUsage;
        else source = argv[i];
    }
    if (!source || (mode == CHECK && (output || cc)) || (mode == EMIT_ASM && (!output || cc))) goto badUsage;
    if (!output) {
#ifdef _WIN32
        output = "a.exe";
#else
        output = "a.out";
#endif
    }
    if (mode != CHECK && sameFile(source, output)) {
        fputs("compiler: source and output must be different files\n", stderr); return 1;
    }
    shouldPrint = false;
    tree = parseSourceFile(source, &errors);
    if (!tree) return 1;
    if (mode == CHECK) { result = checkProgram(tree); goto done; }
    stream = temporary(output, ".s", &assembly);
    if (!stream) { fprintf(stderr, "compiler: cannot create output: %s\n", strerror(errno)); goto done; }
    result = emitAssemblyProgram(tree, stream);
    if (fclose(stream) != 0) { perror("compiler: close output"); result = 1; }
    stream = NULL;
    if (result) goto done;
    if (mode == EMIT_ASM) { result = publishFile(assembly, output); goto done; }
    if (!runtime) runtime = getenv("RUNTIME");
    if (!runtime || !*runtime) runtime = DEFAULT_RUNTIME;
    stream = fopen(runtime, "rb");
    if (!stream) {
        fprintf(stderr, "compiler: cannot open runtime object '%s': %s\n", runtime, strerror(errno));
        result = 1; goto done;
    }
    fclose(stream); stream = NULL;
    stream = temporary(output, ".exe", &binary);
    if (!stream) { perror("compiler: reserve executable"); result = 1; goto done; }
    if (fclose(stream) != 0) { stream = NULL; result = 1; goto done; }
    stream = NULL;
    if (!cc) cc = getenv("CC");
    if (!cc || !*cc) cc = DEFAULT_CC;
    /* A response-file-like relative runtime path is a filename, never an option. */
    char *runtimePath = NULL;
    if (runtime[0] == '-' || runtime[0] == '@') {
        runtimePath = malloc(strlen(runtime) + 3);
        if (!runtimePath) { result = 1; goto done; }
        strcpy(runtimePath, "./"); strcat(runtimePath, runtime);
    }
    result = runToolchain(cc, assembly, runtimePath ? runtimePath : runtime, binary);
    free(runtimePath);
    if (result) fputs("compiler: native compilation failed; output was not replaced\n", stderr);
    else result = publishFile(binary, output);
 done:
    if (stream) fclose(stream);
    if (assembly) { remove(assembly); free(assembly); }
    if (binary) { remove(binary); free(binary); }
    freeParseTree(tree);
    return result;
 badUsage:
    usage(stderr); return 2;
}
