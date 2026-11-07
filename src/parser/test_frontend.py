"""Frontend regression checks. Run: python src/parser/test_frontend.py [gcc]

Builds a temporary harness; leaves no build products or fixtures in the repo.
The compiler argument may be an absolute GCC/Clang-compatible executable path.
"""

import os
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
HARNESS = r'''
#include "src/parser/parser.h"
#include <assert.h>
#include <math.h>

bool shouldPrint = false;

static void checkNode(pTreeNode *node) {
    assert(node && node->symbol && node->lineNumber > 0);
    if (!node->symbol->isNonTerminal) {
        assert(node->ste);
        assert(node->ste->tokenType == node->symbol->tOrNt.t);
    }
    for (int i = 0; i < node->size; ++i) checkNode(node->children[i]);
}

int main(int argc, char **argv) {
    assert(argc == 3);
    if (!strcmp(argv[1], "lex")) {
        FILE *fp = fopen(argv[2], "rb");
        assert(fp);
        linkedList *tokens = getAllTokens(fp);
        assert(tokens);
        fclose(fp);
        for (tokenInfo *node = tokens->head; node;) {
            printf("%s|%s|%.17g\n", tokenToString[node->STE->tokenType],
                   node->STE->lexeme, node->STE->valueIfNumber);
            tokenInfo *next = node->next;
            free(node);
            node = next;
        }
        free(tokens);
        return 0;
    }
    int expectValid = atoi(argv[1]);
    int errors = 7;
    pTree *first = parseSourceFile(argv[2], &errors);
    assert((first != NULL) == expectValid);
    assert(expectValid ? errors == 7 : errors > 7);
    int secondErrors = 0;
    pTree *second = parseSourceFile(argv[2], &secondErrors);
    assert((second != NULL) == expectValid);
    assert(expectValid ? secondErrors == 0 : secondErrors > 0);
    freeParseTree(second);
    if (first) checkNode(first->root);
    freeParseTree(first);
    freeParseTree(NULL);
    assert(!parseSourceFile(NULL, NULL));
    return 0;
}
'''


def main():
    compiler = sys.argv[1] if len(sys.argv) > 1 else "gcc"
    with tempfile.TemporaryDirectory(prefix="compiler-frontend-") as directory:
        work = Path(directory)
        source = work / "harness.c"
        binary = work / ("frontend.exe" if os.name == "nt" else "frontend")
        source.write_text(HARNESS)
        command = [
            compiler, *(["cc"] if Path(compiler).stem.lower() == "zig" else []),
            "-std=c11", "-Wall", "-Wextra", f"-I{ROOT}", str(source),
            str(ROOT / "src/lexer/lexer.c"), str(ROOT / "src/parser/parser.c"),
            "-lm", "-o", str(binary),
        ]
        subprocess.run(command + [f'-DCOMPILER_ROOT="{ROOT.as_posix()}"'],
                       check=True, timeout=120)

        checks = 0

        def run(mode, path, cwd=work):
            nonlocal checks
            result = subprocess.run([str(binary), str(mode), str(path)], cwd=cwd,
                                    capture_output=True, text=True, timeout=10)
            if result.returncode:
                raise AssertionError(f"{mode}: {path}\n{result.stdout}\n{result.stderr}")
            checks += 1
            return result.stdout

        for number in (2, 3, 4, 5):
            run(1, ROOT / f"tests/t{number}.txt")
        run(0, ROOT / "tests/t6.txt")
        run(0, work / "missing.txt")

        valid = b"_main\nreturn;\nend"
        cases = [
            (valid, True), (valid + b" % no final newline", True),
            (valid + b"\n\n", True), (b"", False), (b"% only comment", False),
            (b"_main\nreturn;", False), (b"_main\nend", False),
            (valid + b"\nwrite(1);", False), (valid + b"\n$", False),
            (valid + b"\0return;", False),
            (b"_main\nwrite(1.2);return;end", False),
            (b"_main\nwrite(1.23E+0);return;end", False),
            (b"_main\n" + b"a" * 4096 + b" return;end", False),
            (b"_main\n" + b"#" + b"a" * 4096 + b" return;end", False),
            (b"_main\nwrite(" + b"9" * 4096 + b");return;end", False),
            (b"_main\n" + b"_" + b"a" * 31 + b"2 return;end", False),
            (b"_main\n" + b"b2" + b"b" * 19 + b"2 return;end", False),
            (b"_main\nwrite(1.23E02);write(1.23E+02);write(1.23E-02);return;end", True),
        ]
        fixture = work / "source.txt"
        for data, expected in cases:
            fixture.write_bytes(data)
            run(int(expected), fixture)

        # Each offset exercises half-buffer refill and one/double retraction.
        for offset in (0, 250, 251, 252, 253, 254, 255, 256, 507, 508, 509, 510, 511, 512):
            fixture.write_bytes(b" " * offset + b"12. <-> <= <--- 1.23E02 1.23E+02 1.23E-02 %EOF")
            rows = [line.split("|") for line in run("lex", fixture).splitlines()]
            assert [row[0] for row in rows] == [
                "TK_NUM", "TK_DOT", "TK_LT", "TK_MINUS", "TK_GT", "TK_LE",
                "TK_ASSIGNOP", "TK_RNUM", "TK_RNUM", "TK_RNUM", "TK_COMMENT", "TK_DOLLAR",
            ], rows
            assert [float(row[2]) for row in rows[7:10]] == [123.0, 123.0, 0.0123], rows
            fixture.write_bytes(b" " * offset + valid + b" %EOF")
            run(1, fixture)

        # Without the macro, missing grammar fails cleanly; the legacy cwd
        # lookup still succeeds when launched from the repository.
        subprocess.run(command, check=True, timeout=120)
        fixture.write_bytes(valid)
        run(0, fixture)
        run(1, fixture, cwd=ROOT)
        print(f"Frontend regression checks passed ({checks} harness runs).")


if __name__ == "__main__":
    main()
