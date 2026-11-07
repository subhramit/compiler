r"""Black-box compiler regressions using only the Python standard library.

Run from any directory (build the CLI first)::

    python tests/test_compiler.py --compiler build/compiler.exe \
        --cc build/toolchain/ziglang/zig.exe

Options are project-relative or absolute paths; bare executable names use PATH.
The default assembler/linker toolchain is the project's Zig, then cc/gcc/clang
on PATH. Zig's ``cc`` subcommand is inserted automatically for assembly linking.
Unittest selectors and options may follow, e.g. ``CompilerTests.test_demo -v``.
``python -m unittest discover -s tests -p test_compiler.py`` also uses defaults.

All generated sources, assembly files, binaries, and compiler caches live in
temporary directories. Every subprocess has a deadline; no frontend internals
or harness are used. Successful cases go through --check and native compile/run; a separate
round-trip test assembles --emit-asm output and links build/runtime.o with the
selected toolchain. No generated C or runtime C source is compiled.
"""

import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import textwrap
import unittest


ROOT = Path(__file__).resolve().parents[1]
COMPILER = "build/compiler.exe" if os.name == "nt" else "build/compiler"
CC = None
COMPILE_TIMEOUT = 120
RUN_TIMEOUT = 5


def resolve_executable(value):
    path = Path(value).expanduser()
    if path.is_absolute():
        return str(path) if path.is_file() else None
    project_path = ROOT / path
    if project_path.is_file():
        return str(project_path.resolve())
    return shutil.which(str(value))


def default_cc():
    for candidate in ("build/toolchain/ziglang/zig.exe",
                      "build/toolchain/ziglang/zig", "cc", "gcc", "clang"):
        found = resolve_executable(candidate)
        if found:
            return found
    return None


def main_program(declarations="", statements="", definitions=""):
    return "\n".join(("_main", definitions, declarations, statements,
                      "return;", "end", ""))


PAIR = """
record #pair
    type int : count;
    type real : value;
endrecord
"""
NESTED = PAIR + """
record #nested
    type #pair : inner;
    type int : extra;
endrecord
"""
VARIANT = """
union #choice
    type int : whole;
    type real : fraction;
endunion
record #variant
    type #choice : payload;
    type int : tagvalue;
endrecord
"""
FUNCTION = """
_identity input parameter list [int b2] output parameter list [int c2];
    c2 <--- b2;
    return [c2];
end
"""

# name, complete source, stdin, expected output lines (numbers compare by value)
SUCCESS_CASES = [
    ("empty", main_program(), "", []),
    ("arithmetic_associativity", main_program("type int : b2;", """
        b2 <--- 20 - 5 - 3; write(b2);
        b2 <--- 48 / 4 / 3; write(b2);
        b2 <--- 2 + 3 * 4; write(b2);
        b2 <--- (2 + 3) * 4; write(b2);
        b2 <--- 0 - 7; b2 <--- b2 / 2; write(b2);
    """), "", [12, 4, 14, 20, -3]),
    ("real_arithmetic", main_program("type real : b2;", """
        b2 <--- 10.00 - 2.00 - 1.00; write(b2);
        b2 <--- 24.00 / 2.00 / 3.00; write(b2);
        b2 <--- 1.25E+02 + 2.50; write(b2);
        b2 <--- 1.25E-02; write(b2);
    """), "", [7.0, 4.0, 127.5, 0.0125]),
    ("control_flow", main_program("type int : b2; type int : c2;", """
        b2 <--- 0; c2 <--- 0;
        while (b2 < 5)
            b2 <--- b2 + 1;
            if (b2 == 3) then
                c2 <--- c2 + 10;
            else
                c2 <--- c2 + b2;
            endif
        endwhile
        write(b2); write(c2);
        if ((b2 >= 5) &&& (c2 != 0)) then write(1); else write(0); endif
        if ((b2 < 0) @@@ (c2 <= 22)) then write(2); else write(0); endif
        if (~(b2 > 5)) then write(3); else write(0); endif
        while (b2 < 0) write(999); endwhile
    """), "", [5, 22, 1, 2, 3]),
    ("branch_initialization", main_program("type int : b2;", """
        if (1 == 1) then b2 <--- 6; else b2 <--- 7; endif
        write(b2);
    """), "", [6]),
    ("primitive_io", main_program("type int : b2; type real : c2;", """
        read(b2); read(c2); write(b2); write(c2);
    """), " -7\n2.125\n", [-7, 2.125]),
    ("integer_boundaries", main_program("type int : b2;", """
        read(b2); write(b2); b2 <--- 9223372036854775807; write(b2);
    """), "-9223372036854775808\n", [-9223372036854775808, 9223372036854775807]),
    ("multiple_outputs_value_and_aliasing", """
        _split input parameter list [int b2, int c2]
            output parameter list [int d2, int d3];
            b2 <--- b2 + 10;
            d2 <--- b2;
            d3 <--- c2 + 20;
            return [d2, d3];
        end
        _main
            type int : b2; type int : c2; type int : d2;
            b2 <--- 1; c2 <--- 2;
            [c2, d2] <--- call _split with parameters [b2, c2];
            write(b2); write(c2); write(d2);
            [b2, b2] <--- call _split with parameters [b2, b2];
            write(b2);
            return;
        end
    """, "", [1, 11, 22, 21]),
    ("earlier_function_and_no_outputs", FUNCTION + """
        _print input parameter list [int b2];
            type int : c2;
            [c2] <--- call _identity with parameters [b2];
            write(c2);
            return;
        end
        _main
            type int : b2;
            b2 <--- 9;
            call _print with parameters [b2];
            return;
        end
    """, "", [9]),
    ("globals", """
        _bump input parameter list [int b2];
            c2 <--- c2 + b2;
            return;
        end
        _main
            type int : c2 : global;
            type int : b2;
            write(c2); b2 <--- 4;
            call _bump with parameters [b2];
            call _bump with parameters [b2];
            write(c2);
            return;
        end
    """, "", [0, 8]),
    ("nested_record_arithmetic", main_program("""
        type #nested : b2; type #nested : c2; type #nested : d2;
    """, """
        read(b2); read(c2);
        d2 <--- b2 + c2; write(d2);
        d2 <--- d2 - c2; write(d2);
        d2 <--- 2 * b2; write(d2);
        d2 <--- b2 * 1.50; write(d2);
        d2 <--- b2 / 2; write(d2);
        d2 <--- b2 / 2.00; write(d2);
        write(b2.inner.value);
    """, NESTED), "3 2.50 5  1 1.50 3\n",
     [4, 4.0, 8, 3, 2.5, 5, 6, 5.0, 10, 4, 3.75, 7,
      1, 1.25, 2, 1, 1.25, 2, 2.5]),
    ("record_scalar_truncates_toward_zero", main_program("type #pair : b2;", """
        b2.count <--- 0 - 3; b2.value <--- 0.00 - 2.50;
        b2 <--- b2 * 1.50; write(b2);
    """, PAIR), "", [-4, -3.75]),
    ("record_aliases_and_value_parameters", """
        _change input parameter list [record #alias b2]
            output parameter list [#pair c2];
            b2.count <--- b2.count + 1;
            c2 <--- b2;
            return [c2];
        end
    """ + main_program("type #pair : b2; type #second : c2;", """
        read(b2);
        [c2] <--- call _change with parameters [b2];
        write(b2); write(c2);
        [b2] <--- call _change with parameters [b2]; write(b2);
    """, PAIR + "definetype record #pair as #alias\n"
        "definetype record #alias as #second\n"), "4 1.25\n",
     [4, 1.25, 5, 1.25, 5, 1.25]),
    ("union_switch_and_copy", main_program("type #variant : b2; type #variant : c2;", """
        b2.tagvalue <--- 1; b2.payload.whole <--- 7; write(b2);
        c2 <--- b2; write(c2.payload.whole);
        b2.tagvalue <--- 2; b2.payload.fraction <--- 2.50; write(b2);
        write(c2);
        c2.tagvalue <--- 2; c2.payload <--- b2.payload; write(c2);
    """, VARIANT), "", [1, 7, 7, 2, 2.5, 1, 7, 2, 2.5]),
    ("union_tag_first_io", main_program("type #variant : b2;", "read(b2); write(b2);", VARIANT),
     "2 4.25\n", [2, 4.25]),
    ("union_alias_real_tag", main_program("type #variant : b2;", """
        b2.tagvalue <--- 1.00; b2.payload.whole <--- 8; write(b2);
        b2.tagvalue <--- 2.00; b2.payload.fraction <--- 1.25; write(b2);
    """, VARIANT.replace("record #variant", "definetype union #choice as #alias\nrecord #variant")
        .replace("type #choice : payload", "type #alias : payload")
        .replace("type int : tagvalue", "type real : tagvalue")), "", [1.0, 8, 2.0, 1.25]),
    ("union_short_circuit", main_program("type #variant : b2;", """
        b2.tagvalue <--- 1; b2.payload.whole <--- 7;
        if ((1 == 1) @@@ (b2.payload.fraction > 0.00)) then write(1); else write(0); endif
        if ((1 == 0) &&& (b2.payload.fraction > 0.00)) then write(0); else write(2); endif
    """, VARIANT), "", [1, 2]),
]

# Semantic fixtures are grammar-valid: errors must not merely be parser errors.
# Diagnostic fragments deliberately avoid requiring an exact sentence/line.
REJECT_CASES = [
    ("mixed_arithmetic", main_program("type real : b2;", "b2 <--- 1 + 2.00;"), "primitive"),
    ("assignment_type", main_program("type int : b2;", "b2 <--- 1.00;"), "type"),
    ("comparison_type", main_program(statements="if (1 < 2.00) then write(1); endif"), "primitive"),
    ("duplicate_local", main_program("type int : b2; type int : b2;"), "duplicate"),
    ("duplicate_global", main_program("type int : b2 : global; type int : b2 : global;"), "duplicate"),
    ("global_shadow", FUNCTION + main_program("type int : b2 : global;"), "global"),
    ("duplicate_parameter", FUNCTION.replace("[int b2]", "[int b2, int b2]") + main_program(), "duplicate"),
    ("duplicate_function", FUNCTION + FUNCTION + main_program(), "duplicate"),
    ("duplicate_type", main_program(definitions=PAIR + PAIR), "duplicate"),
    ("duplicate_field", main_program(definitions=PAIR.replace("value;", "count;")), "duplicate"),
    ("duplicate_alias", main_program(definitions=PAIR + "definetype record #pair as #pair"), "duplicate"),
    ("undefined_variable", main_program(statements="write(b2);"), "undeclared"),
    ("undefined_type", main_program("type #missing : b2;"), "unknown"),
    ("undefined_function", main_program("type int : b2;", "b2 <--- 1; call _missing with parameters [b2];"), "function"),
    ("undefined_field", main_program("type #pair : b2;", "b2.missing <--- 1;", PAIR), "field"),
    ("primitive_field", main_program("type int : b2;", "b2.count <--- 1;"), "primitive"),
    ("alias_cycle", main_program(definitions="definetype record #first as #second\ndefinetype record #second as #first"), "cycl"),
    ("record_cycle", main_program(definitions="record #first type #first : self; type int : value; endrecord"), "cycl"),
    ("mutual_record_cycle", main_program(definitions="record #first type #second : other; type int : value; endrecord\nrecord #second type #first : other; type int : value; endrecord"), "cycl"),
    ("alias_kind", main_program(definitions=PAIR + "definetype union #pair as #alias"), "union type"),
    ("nominal_record_types", main_program("type #pair : b2; type #other : c2;", "read(b2); c2 <--- b2;",
        PAIR + PAIR.replace("#pair", "#other")), "type"),
    ("record_multiplication", main_program("type #pair : b2;", "read(b2); b2 <--- b2 * b2;", PAIR), "operand"),
    ("forward_call", "_first input parameter list [int b2]; call _identity with parameters [b2]; return; end\n" + FUNCTION + main_program(), "earlier"),
    ("recursion", FUNCTION.replace("c2 <--- b2;", "[c2] <--- call _identity with parameters [b2];") + main_program(), "recursion"),
    ("call_input_arity", FUNCTION + main_program("type int : b2; type int : c2;", "b2 <--- 1; [c2] <--- call _identity with parameters [b2, b2];"), "arity"),
    ("call_output_arity", FUNCTION + main_program("type int : b2;", "b2 <--- 1; call _identity with parameters [b2];"), "arity"),
    ("call_input_type", FUNCTION + main_program("type real : b2; type int : c2;", "b2 <--- 1.00; [c2] <--- call _identity with parameters [b2];"), "type"),
    ("call_output_type", FUNCTION + main_program("type int : b2; type real : c2;", "b2 <--- 1; [c2] <--- call _identity with parameters [b2];"), "type"),
    ("return_arity", FUNCTION.replace("return [c2];", "return;") + main_program(), "return"),
    ("return_type", FUNCTION.replace("return [c2];", "return [d2];").replace("c2 <--- b2;", "type real : d2; c2 <--- b2; d2 <--- 1.00;") + main_program(), "return"),
    ("main_return_value", main_program("type int : b2;", "b2 <--- 1;").replace("return;", "return [b2];"), "return"),
    ("uninitialized_output", FUNCTION.replace("c2 <--- b2;", "") + main_program(), "initializ"),
    ("output_not_assigned", FUNCTION.replace("c2 <--- b2;", "").replace("return [c2];", "return [b2];") + main_program(), "initializ"),
    ("uninitialized_local", main_program("type int : b2;", "write(b2);"), "initializ"),
    ("uninitialized_self", main_program("type int : b2;", "b2 <--- b2 + 1;"), "initializ"),
    ("uninitialized_branch", main_program("type int : b2;", "if (1 == 1) then b2 <--- 1; endif write(b2);"), "initializ"),
    ("uninitialized_loop", main_program("type int : b2;", "while (1 < 0) b2 <--- 1; endwhile write(b2);"), "initializ"),
    ("uninitialized_record", main_program("type #pair : b2;", "b2.count <--- 1; write(b2);", PAIR), "initializ"),
    ("uninitialized_argument", FUNCTION + main_program("type int : b2; type int : c2;", "[c2] <--- call _identity with parameters [b2];"), "initializ"),
    ("read_literal", main_program(statements="read(1);"), "destination"),
    ("read_real_literal", main_program(statements="read(1.00);"), "destination"),
    ("integer_literal_range", main_program(statements="write(9223372036854775808);"), "range"),
    ("standalone_union", main_program("type union #choice : b2;", definitions=VARIANT), "standalone"),
    ("union_missing_tag", main_program(definitions=VARIANT.replace("tagvalue", "other")), "tagvalue"),
    ("union_multiple_fields", main_program(definitions=VARIANT.replace("type int : tagvalue;", "type int : tagvalue; type #choice : extra;")), "multiple"),
    ("union_inside_union", main_program(definitions=VARIANT + "union #outer type #variant : nested; type int : other; endunion"), "union"),
    ("variant_arithmetic", main_program("type #variant : b2;", "read(b2); b2 <--- b2 + b2;", VARIANT), "variant"),
    ("uninitialized_tag", main_program("type #variant : b2;", "b2.payload.whole <--- 1;", VARIANT), "tagvalue"),
]

SYNTAX_REJECT_CASES = [
    ("missing_return", "_main\nend\n", None),
    ("literal_call_argument", FUNCTION + main_program("type int : b2;", "[b2] <--- call _identity with parameters [1];"), None),
    ("record_terminator_semicolon", main_program(definitions=PAIR + ";"), None),
    ("alias_terminator_semicolon", main_program(definitions=PAIR + "definetype record #pair as #alias;"), None),
    ("declaration_missing_colon", main_program("type int b2;"), None),
]

# name, source, input, runtime diagnostic fragment
RUNTIME_CASES = [
    ("integer_division_zero", main_program("type int : b2;", "b2 <--- 1 / 0; write(b2);"), "", "division by zero"),
    ("real_division_zero", main_program("type real : b2;", "b2 <--- 1.00 / 0.00; write(b2);"), "", "division by zero"),
    ("integer_add_overflow", main_program("type int : b2;", "b2 <--- 9223372036854775807 + 1;"), "", "overflow"),
    ("integer_sub_overflow", main_program("type int : b2;", "read(b2); b2 <--- b2 - 1;"), "-9223372036854775808", "overflow"),
    ("integer_mul_overflow", main_program("type int : b2;", "b2 <--- 9223372036854775807 * 2;"), "", "overflow"),
    ("integer_div_overflow", main_program("type int : b2; type int : c2;", "read(b2); c2 <--- 0 - 1; b2 <--- b2 / c2;"), "-9223372036854775808", "overflow"),
    ("real_overflow", main_program("type real : b2;", "read(b2); b2 <--- b2 * b2;"), "1e308", "nonfinite"),
    ("record_scalar_range", main_program("type #pair : b2;", "read(b2); b2 <--- b2 * 2.00;", PAIR), "9223372036854775807 1.0", "range"),
    ("union_inactive_read", main_program("type #variant : b2;", "b2.tagvalue <--- 1; b2.payload.whole <--- 7; write(b2.payload.fraction);", VARIANT), "", "inactive"),
    ("union_inactive_write", main_program("type #variant : b2;", "b2.tagvalue <--- 1; b2.payload.fraction <--- 1.00;", VARIANT), "", "inactive"),
    ("union_stale_tag", main_program("type #variant : b2;", "b2.tagvalue <--- 1; b2.payload.whole <--- 7; b2.tagvalue <--- 2; write(b2);", VARIANT), "", "initialized"),
    ("union_bad_tag_zero", main_program("type #variant : b2;", "read(b2);", VARIANT), "0 1", "tagvalue"),
    ("union_bad_tag_high", main_program("type #variant : b2;", "read(b2);", VARIANT), "3 1", "tagvalue"),
    ("union_fractional_tag", main_program("type #variant : b2;", "read(b2);", VARIANT.replace("type int : tagvalue", "type real : tagvalue")), "1.5 1", "tagvalue"),
    ("union_copy_destination_tag", main_program("type #variant : b2; type #variant : c2;", "read(b2); c2.tagvalue <--- 2; c2.payload <--- b2.payload;", VARIANT), "1 7", "initialized"),
    ("union_copy_source_tag", main_program("type #variant : b2; type #variant : c2;", "read(b2); b2.tagvalue <--- 2; c2.tagvalue <--- 2; c2.payload <--- b2.payload;", VARIANT), "1 7", "initialized"),
]


class CompilerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        compiler = resolve_executable(COMPILER)
        cc = resolve_executable(CC) if CC else default_cc()
        if not compiler:
            raise RuntimeError(f"Compiler not found: {COMPILER}. Build the CLI first or pass --compiler.")
        if not cc:
            raise RuntimeError("Assembler/linker toolchain not found. Pass --cc or install the project Zig toolchain.")
        cls.compiler = compiler
        cls.cc = cc
        cls.cache_temporary = tempfile.TemporaryDirectory(prefix="compiler-e2e-cache-")
        cls.addClassCleanup(cls.cache_temporary.cleanup)
        cls.cache_work = Path(cls.cache_temporary.name)

    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="compiler-e2e-")
        self.addCleanup(self.temporary.cleanup)
        self.work = Path(self.temporary.name)
        # Spaces exercise argument handling without shell quoting assumptions.
        self.source = self.work / "source program.txt"
        self.binary = self.work / ("compiled program.exe" if os.name == "nt" else "compiled program")
        self.generated = self.work / "generated program.s"
        self.environment = os.environ.copy()
        self.environment.pop("RUNTIME", None)
        # Share expensive toolchain builds, not sources or test outputs.
        self.environment["ZIG_GLOBAL_CACHE_DIR"] = str(self.cache_work / "zig-global-cache")
        self.environment["ZIG_LOCAL_CACHE_DIR"] = str(self.cache_work / "zig-local-cache")
        self.environment["TMP"] = self.environment["TEMP"] = self.environment["TMPDIR"] = str(self.work)

    def invoke(self, arguments, *, stdin="", timeout=COMPILE_TIMEOUT):
        command = [str(argument) for argument in arguments]
        try:
            return subprocess.run(command, input=stdin, capture_output=True,
                                  text=True, encoding="utf-8", errors="replace",
                                  cwd=self.work, env=self.environment, timeout=timeout)
        except subprocess.TimeoutExpired as error:
            self.fail(f"Timed out after {timeout}s: {command!r}\n"
                      f"stdout: {error.stdout!r}\nstderr: {error.stderr!r}")
        except OSError as error:
            self.fail(f"Cannot run {command!r}: {error}")

    def assert_success(self, result):
        self.assertEqual(result.returncode, 0,
                         f"Command: {result.args!r}\nstdout:\n{result.stdout}\nstderr:\n{result.stderr}")

    def write_source(self, source):
        self.source.write_text(textwrap.dedent(source).strip() + "\n", encoding="utf-8")

    def compile_source(self, source):
        self.write_source(source)
        self.assert_success(self.invoke([self.compiler, "--check", self.source]))
        self.assert_success(self.invoke([self.compiler, self.source, "-o", self.binary, "--cc", self.cc]))
        self.assertTrue(self.binary.is_file(), "Successful compilation did not create a binary")

    def assert_output(self, result, expected):
        self.assert_success(result)
        self.assertEqual(result.stderr, "", "Successful program emitted diagnostics")
        lines = result.stdout.splitlines()
        self.assertEqual(len(lines), len(expected), f"Unexpected output: {result.stdout!r}")
        for actual, wanted in zip(lines, expected):
            if isinstance(wanted, int):
                self.assertEqual(actual, str(wanted))
            else:
                self.assertAlmostEqual(float(actual), wanted, delta=max(1e-12, abs(wanted) * 1e-12))

    def check_success_case(self, source, stdin, expected):
        self.compile_source(source)
        self.assert_output(self.invoke([self.binary], stdin=stdin, timeout=RUN_TIMEOUT), expected)

    def check_rejection(self, source, diagnostic):
        self.write_source(source)
        for mode in ("--check", "--emit-asm"):
            with self.subTest(mode=mode):
                command = [self.compiler, mode, self.source]
                if mode == "--emit-asm":
                    command += ["-o", self.generated]
                result = self.invoke(command)
                self.assertNotEqual(result.returncode, 0, "Invalid program was accepted")
                self.assertTrue(result.stderr.strip(), "Failure must produce a diagnostic on stderr")
                if diagnostic:
                    self.assertIn(diagnostic, result.stderr.lower())
                if self.generated.exists():
                    self.assertEqual(self.generated.stat().st_size, 0, "Semantic failure emitted partial assembly")

    def check_runtime_case(self, source, stdin, diagnostic):
        self.compile_source(source)
        result = self.invoke([self.binary], stdin=stdin, timeout=RUN_TIMEOUT)
        self.assertNotEqual(result.returncode, 0, "Expected a checked runtime failure")
        self.assertIn("runtime error", result.stderr.lower())
        self.assertIn(diagnostic, result.stderr.lower())

    def test_demo(self):
        source = (ROOT / "tests/programs/demo.txt").read_text(encoding="utf-8")
        self.check_success_case(source, "3 1.50\n", [3, 3, 3.0])

    def test_emit_asm_round_trip(self):
        self.write_source(main_program("type int : b2;", "read(b2); b2 <--- b2 * 3; write(b2);"))
        result = self.invoke([self.compiler, "--emit-asm", self.source, "-o", self.generated])
        self.assert_success(result)
        self.assertTrue(self.generated.is_file())
        assembly = self.generated.read_text(encoding="utf-8")
        self.assertIn(".intel_syntax", assembly)
        self.assertRegex(assembly, r"(?m)^\s*\.text\b")
        self.assertRegex(assembly, r"(?mi)^\s*(?:mov|push|sub|imul|call)\s+[^\n]+")
        if os.name == "nt":
            for directive in (".seh_proc", ".seh_pushreg", ".seh_stackalloc",
                              ".seh_endprologue", ".seh_endproc"):
                self.assertIn(directive, assembly)
        self.assertNotIn("#include", assembly)
        self.assertNotRegex(assembly, r"\bint\s+main\s*\(")
        command = [self.cc]
        if Path(self.cc).name.lower() in ("zig", "zig.exe"):
            command.append("cc")
        command += ["-x", "assembler", str(self.generated), "-x", "none",
                    str(ROOT / "build/runtime.o"), "-lm", "-o", str(self.binary)]
        self.assert_success(self.invoke(command))
        self.assert_output(self.invoke([self.binary], stdin="14\n", timeout=RUN_TIMEOUT), [42])

    def test_multipage_callee_stack_frame(self):
        source = """_large input parameter list [int b2] output parameter list [int c2];
        """ + "\n".join("b2 <--- b2 + 1;" for _ in range(800)) + """
        c2 <--- b2; return [c2]; end
        _main
        type int : b2;
        b2 <--- 9;
        [b2] <--- call _large with parameters [b2];
        write(b2); return; end
        """
        self.check_success_case(source, "", [809])
        self.assert_success(self.invoke([self.compiler, "--emit-asm", self.source, "-o", self.generated]))
        assembly = self.generated.read_text(encoding="utf-8")
        self.assertIn("sub r10, 4096", assembly)
        self.assertIn("mov BYTE PTR [r10], 0", assembly)

    def test_check_needs_no_runtime_or_toolchain(self):
        self.write_source(main_program(statements="write(1);"))
        self.environment["PATH"] = str(self.work / "nonexistent-path")
        self.environment["CC"] = str(self.work / "missing-toolchain")
        missing_runtime = self.work / "missing runtime.o"
        self.environment["RUNTIME"] = str(missing_runtime)
        self.assert_success(self.invoke([self.compiler, "--check", self.source,
                                         "--runtime", missing_runtime]))
        self.assert_work_files(self.source)

    def test_help(self):
        result = self.invoke([self.compiler, "--help"], timeout=RUN_TIMEOUT)
        self.assert_success(result)
        self.assertEqual(result.stderr, "")
        help_text = result.stdout.lower()
        for option in ("--emit-asm", "--check", "--legacy", "--cc", "--runtime"):
            with self.subTest(option=option):
                self.assertIn(option, help_text)
        self.assertIn("usage", help_text)
        self.assertNotIn("--emit-c", help_text)
        self.assert_work_files()

    def test_missing_runtime_preserves_output(self):
        self.write_source(main_program(statements="write(42);"))
        missing_runtime = self.work / "missing runtime.o"
        sentinel = b"existing executable must survive\x00\xff\n"
        for existing_output in (False, True):
            with self.subTest(existing_output=existing_output):
                if existing_output:
                    self.binary.write_bytes(sentinel)
                result = self.invoke([self.compiler, self.source, "-o", self.binary,
                                      "--cc", self.cc, "--runtime", missing_runtime])
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("runtime", result.stderr.lower())
                if existing_output:
                    self.assertEqual(self.binary.read_bytes(), sentinel)
                    self.assert_work_files(self.source, self.binary)
                else:
                    self.assert_work_files(self.source)

    def test_runtime_option_overrides_default(self):
        self.write_source(main_program(statements="write(42);"))
        runtime = self.work / "custom runtime.o"
        shutil.copyfile(ROOT / "build/runtime.o", runtime)
        self.environment["RUNTIME"] = str(self.work / "missing environment runtime.o")
        self.assert_success(self.invoke([self.compiler, self.source, "-o", self.binary,
                                         "--cc", self.cc, "--runtime", runtime]))
        self.assert_output(self.invoke([self.binary], timeout=RUN_TIMEOUT), [42])
        self.assert_work_files(self.source, runtime, self.binary)

    @unittest.skipIf(os.name == "nt", "Executable Python toolchain shim requires POSIX exec")
    def test_native_toolchain_only_assembles_and_links(self):
        if any(character.isspace() for character in sys.executable):
            self.skipTest("Python interpreter path cannot contain whitespace in a POSIX shebang")
        self.write_source(main_program("type int : b2;", "read(b2); b2 <--- b2 + 2; write(b2);"))
        shim = self.work / "assembler only toolchain"
        log = self.work / "toolchain calls.jsonl"
        command = [self.cc]
        if Path(self.cc).name.lower() in ("zig", "zig.exe"):
            command.append("cc")
        shim.write_text(f"#!{sys.executable}\n" + textwrap.dedent(f"""\
            import json
            from pathlib import Path
            import subprocess
            import sys

            arguments = sys.argv[1:]
            with open({str(log)!r}, "a", encoding="utf-8") as output:
                output.write(json.dumps(arguments) + "\\n")
            for index, argument in enumerate(arguments):
                language = (arguments[index + 1] if argument == "-x" and index + 1 < len(arguments)
                            else argument[2:] if argument.startswith("-x") else "")
                if language and language not in ("assembler", "assembler-with-cpp", "none"):
                    sys.exit("C compilation is forbidden: " + language)
                if Path(argument).suffix.lower() in (".c", ".i", ".cc", ".cpp", ".cxx"):
                    sys.exit("C input is forbidden: " + argument)
            sys.exit(subprocess.run({command!r} + arguments, timeout={COMPILE_TIMEOUT - 5}).returncode)
            """), encoding="utf-8")
        shim.chmod(0o755)
        self.assert_success(self.invoke([self.compiler, self.source, "-o", self.binary, "--cc", shim]))
        self.assert_output(self.invoke([self.binary], stdin="40\n", timeout=RUN_TIMEOUT), [42])
        calls = [json.loads(line) for line in log.read_text(encoding="utf-8").splitlines()]
        self.assertTrue(calls, "Native build bypassed the selected toolchain")
        self.assertTrue(any("-xassembler" in call or
                            any(call[index:index + 2] == ["-x", "assembler"]
                                for index in range(len(call) - 1)) for call in calls),
                        f"No explicit assembly input mode: {calls!r}")
        self.assertTrue(any(Path(argument).suffix == ".s" for call in calls for argument in call),
                        f"No emitted .s input: {calls!r}")
        self.assertTrue(any(Path(argument).resolve() == (ROOT / "build/runtime.o").resolve()
                            for call in calls for argument in call if not argument.startswith("-")),
                        f"Prebuilt runtime was not linked: {calls!r}")
        self.assert_work_files(self.source, shim, log, self.binary)

    def test_missing_source(self):
        result = self.invoke([self.compiler, "--check", self.work / "missing.txt"])
        self.assertNotEqual(result.returncode, 0)
        self.assertTrue(result.stderr.strip())

    def test_missing_toolchain(self):
        self.write_source(main_program())
        result = self.invoke([self.compiler, self.source, "-o", self.binary,
                              "--cc", self.work / "missing-toolchain.exe"])
        self.assertNotEqual(result.returncode, 0)
        self.assertTrue(result.stderr.strip())
        self.assertFalse(self.binary.exists())

    def assert_work_files(self, *expected):
        self.assertEqual(set(self.work.iterdir()), set(expected),
                         "Unexpected artifacts remain in the test directory")

    def test_invalid_arguments(self):
        self.write_source(main_program())
        cases = [
            [], ["--unknown"], ["--check"], ["--emit-asm"], ["--legacy"],
            ["--check", self.source, "-o", self.generated],
            ["--check", self.source, "--cc", self.cc],
            ["--emit-asm", self.source],
            ["--emit-asm", self.source, "-o", self.generated, "--cc", self.cc],
            ["--check", "--emit-asm", self.source],
            ["--check", "--check", self.source],
            [self.source, "-o"], [self.source, "-o", ""],
            [self.source, "--cc"], [self.source, "--cc", ""],
            [self.source, "--runtime"], [self.source, "--runtime", ""],
            [self.source, "--runtime", ROOT / "build/runtime.o",
             "--runtime", ROOT / "build/runtime.o"],
            [self.source, "-o", self.binary, "-o", self.generated],
            [self.source, "--cc", self.cc, "--cc", self.cc],
            [self.source, "-o", self.binary, self.source],
        ]
        for arguments in cases:
            with self.subTest(arguments=arguments):
                result = self.invoke([self.compiler, *arguments], timeout=RUN_TIMEOUT)
                self.assertNotEqual(result.returncode, 0, "Invalid arguments were accepted")
                self.assertIn("usage", result.stderr.lower())
                self.assert_work_files(self.source)

    def test_existing_output_preserved_on_semantic_failure(self):
        self.write_source(main_program("type int : b2;", "write(b2);"))
        sentinel = b"existing output must survive\x00\xff\n"
        for mode, output in (("--emit-asm", self.generated), ("native", self.binary)):
            with self.subTest(mode=mode):
                output.write_bytes(sentinel)
                arguments = (["--emit-asm", self.source, "-o", output] if mode == "--emit-asm"
                             else [self.source, "-o", output, "--cc", self.cc])
                result = self.invoke([self.compiler, *arguments])
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("initializ", result.stderr.lower())
                self.assertEqual(output.read_bytes(), sentinel)
                self.assert_work_files(self.source, output)
                output.unlink()

    def test_existing_output_preserved_on_native_failure(self):
        self.write_source(main_program(statements="write(42);"))
        sentinel = b"existing executable must survive\x00\xff\n"
        self.binary.write_bytes(sentinel)
        # Python starts successfully but rejects the assembler/linker flags.
        result = self.invoke([self.compiler, self.source, "-o", self.binary,
                              "--cc", sys.executable])
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("native compilation failed", result.stderr.lower())
        self.assertEqual(self.binary.read_bytes(), sentinel)
        self.assert_work_files(self.source, self.binary)

    def test_failed_compilation_leaves_no_artifacts(self):
        for failure in ("semantic", "native"):
            with self.subTest(failure=failure):
                source = (main_program("type int : b2;", "write(b2);") if failure == "semantic"
                          else main_program(statements="write(42);"))
                self.write_source(source)
                result = self.invoke([self.compiler, self.source, "-o", self.binary,
                                      "--cc", sys.executable])
                self.assertNotEqual(result.returncode, 0)
                self.assertTrue(result.stderr.strip())
                self.assert_work_files(self.source)
        self.write_source(main_program("type int : b2;", "write(b2);"))
        result = self.invoke([self.compiler, "--emit-asm", self.source, "-o", self.generated])
        self.assertNotEqual(result.returncode, 0)
        self.assert_work_files(self.source)

    def test_successful_emit_asm_replaces_output_without_artifacts(self):
        self.write_source(main_program(statements="write(42);"))
        sentinel = b"old assembly output\x00\xff"
        self.generated.write_bytes(sentinel)
        self.assert_success(self.invoke([self.compiler, "--emit-asm", self.source, "-o", self.generated]))
        self.assertNotEqual(self.generated.read_bytes(), sentinel)
        self.assertGreater(self.generated.stat().st_size, len(sentinel))
        self.assert_work_files(self.source, self.generated)

    def test_successful_native_replaces_output_without_artifacts(self):
        self.binary.write_bytes(b"old executable\x00\xff")
        self.compile_source(main_program(statements="write(42);"))
        self.assert_output(self.invoke([self.binary], timeout=RUN_TIMEOUT), [42])
        self.assert_work_files(self.source, self.binary)

    def test_cc_option_overrides_environment(self):
        self.environment["CC"] = str(self.work / "missing-environment-compiler")
        self.compile_source(main_program(statements="write(42);"))
        self.assert_output(self.invoke([self.binary], timeout=RUN_TIMEOUT), [42])
        self.assert_work_files(self.source, self.binary)

    def test_default_build_time_cc(self):
        self.environment.pop("CC", None)
        self.write_source(main_program(statements="write(42);"))
        self.assert_success(self.invoke([self.compiler, self.source, "-o", self.binary]))
        self.assert_output(self.invoke([self.binary], timeout=RUN_TIMEOUT), [42])
        self.assert_work_files(self.source, self.binary)

    def test_source_output_same_file_rejected(self):
        self.write_source(main_program(statements="write(42);"))
        original = self.source.read_bytes()
        for mode in ("--emit-asm", "native"):
            with self.subTest(mode=mode):
                arguments = (["--emit-asm", self.source, "-o", self.source] if mode == "--emit-asm"
                             else [self.source, "-o", self.source, "--cc", self.cc])
                result = self.invoke([self.compiler, *arguments])
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("different files", result.stderr.lower())
                self.assertEqual(self.source.read_bytes(), original)
                self.assert_work_files(self.source)

    def test_source_output_aliases_rejected(self):
        self.write_source(main_program(statements="write(42);"))
        original = self.source.read_bytes()
        hardlink = self.work / "source alias.txt"
        os.link(self.source, hardlink)
        aliases = [str(hardlink)]
        if os.name == "nt":
            aliases.append("\\\\?\\" + str(self.source.resolve()))
        for alias in aliases:
            with self.subTest(alias=alias):
                result = self.invoke([self.compiler, "--emit-asm", self.source, "-o", alias])
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("different files", result.stderr.lower())
                self.assertEqual(self.source.read_bytes(), original)
        self.assert_work_files(self.source, hardlink)

    def test_option_like_output_paths_are_literal(self):
        self.write_source(main_program(statements="write(42);"))
        outputs = []
        for prefix in ("-", "@"):
            with self.subTest(prefix=prefix):
                name = prefix + ("program.exe" if os.name == "nt" else "program")
                self.assert_success(self.invoke([self.compiler, self.source, "-o", name, "--cc", self.cc]))
                binary = self.work / name
                outputs.append(binary)
                self.assert_output(self.invoke([binary], timeout=RUN_TIMEOUT), [42])
        self.assert_work_files(self.source, *outputs)

    def test_shell_metacharacter_paths_are_literal(self):
        self.source = self.work / "source & echo INJECTED ; literal ' % ! (test).txt"
        suffix = ".exe" if os.name == "nt" else ""
        self.binary = self.work / ("program & echo INJECTED ; literal ' % ! (test)" + suffix)
        self.generated = self.work / "output & echo INJECTED ; literal ' % ! (test).s"
        self.compile_source(main_program(statements="write(42);"))
        self.assert_output(self.invoke([self.binary], timeout=RUN_TIMEOUT), [42])
        self.assert_success(self.invoke([self.compiler, "--emit-asm", self.source, "-o", self.generated]))
        self.assertGreater(self.generated.stat().st_size, 0)
        self.assert_work_files(self.source, self.binary, self.generated)

    def test_legacy_menu(self):
        self.write_source(main_program(statements="write(1);"))
        tree = self.work / "legacy tree.txt"
        result = self.invoke([self.compiler, "--legacy", self.source, tree], stdin="3\n0\n", timeout=15)
        self.assert_success(result)
        self.assertTrue(tree.is_file(), "Legacy parse-menu option did not write its tree")
        self.assertGreater(tree.stat().st_size, 0)

    def test_invalid_integer_input(self):
        self.compile_source(main_program("type int : b2;", "read(b2); write(b2);"))
        for value in ("", "abc", "1.5", "12junk", "9223372036854775808",
                      "-9223372036854775809", "9" * 600, "1\x00x"):
            with self.subTest(value=value[:40]):
                result = self.invoke([self.binary], stdin=value, timeout=RUN_TIMEOUT)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("runtime error", result.stderr.lower())
                self.assertIn("input", result.stderr.lower())

    def test_invalid_real_input(self):
        self.compile_source(main_program("type real : b2;", "read(b2); write(b2);"))
        for value in ("", "abc", "1.2junk", "nan", "inf", "-inf", "1e9999", "1e-9999"):
            with self.subTest(value=value):
                result = self.invoke([self.binary], stdin=value, timeout=RUN_TIMEOUT)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("runtime error", result.stderr.lower())
                self.assertIn("input", result.stderr.lower())


def install_cases():
    # Separate unittest methods give failures stable names and allow selection.
    for prefix, cases, checker in (
        ("run", SUCCESS_CASES, "check_success_case"),
        ("reject", REJECT_CASES + SYNTAX_REJECT_CASES, "check_rejection"),
        ("runtime", RUNTIME_CASES, "check_runtime_case"),
    ):
        for name, *arguments in cases:
            def test(self, arguments=tuple(arguments), checker=checker):
                getattr(self, checker)(*arguments)
            test.__name__ = f"test_{prefix}_{name}"
            setattr(CompilerTests, test.__name__, test)


install_cases()


def main():
    global COMPILER, CC
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--compiler", default=COMPILER, help=f"Compiler CLI executable (default: {COMPILER})")
    parser.add_argument("--cc", help="Assembler/linker toolchain executable (default: project Zig, then PATH)")
    options, unittest_arguments = parser.parse_known_args()
    COMPILER, CC = options.compiler, options.cc
    if not resolve_executable(COMPILER):
        parser.error(f"compiler not found: {COMPILER}; build the CLI first or pass --compiler")
    if not (resolve_executable(CC) if CC else default_cc()):
        parser.error("Assembler/linker toolchain not found; pass --cc")
    unittest.main(argv=[sys.argv[0]] + unittest_arguments)


if __name__ == "__main__":
    main()
