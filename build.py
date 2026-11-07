"""Build the C compiler implementation and assemble its handwritten runtime."""

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parent


def resolve_tool(value):
    path = Path(value).expanduser()
    if path.is_file():
        return str(path.resolve())
    if (ROOT / path).is_file():
        return str((ROOT / path).resolve())
    return shutil.which(value)


def find_tool(explicit=None):
    if explicit or os.environ.get("CC"):
        value = explicit or os.environ["CC"]
        found = resolve_tool(value)
        if not found:
            raise RuntimeError(f"C compiler not found: {value!r} (use one executable path)")
        return found
    for candidate in ("build/toolchain/ziglang/zig.exe", "build/toolchain/ziglang/zig",
                      "cc", "gcc", "clang", "zig"):
        found = resolve_tool(candidate)
        if found:
            return found
    raise RuntimeError("No C compiler found. Install GCC/Clang, or run:\n"
                       "  python -m pip install --target build/toolchain ziglang")


def c_string(value):
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cc", help="GCC/Clang/Zig executable path (not a shell command)")
    parser.add_argument("--output", type=Path, help="output executable path")
    args = parser.parse_args()
    try:
        tool = find_tool(args.cc)
        output = (args.output or ROOT / "build" / ("compiler.exe" if os.name == "nt" else "compiler")).resolve()
        output.parent.mkdir(parents=True, exist_ok=True)
        runtime = output.parent / "runtime.o"
        tool_command = [tool]
        if Path(tool).stem.lower() == "zig":
            tool_command.append("cc")
        subprocess.run(tool_command + ["-c", "-x", "assembler-with-cpp",
                       str(ROOT / "src/runtime/runtime.S"), "-o", str(runtime)],
                       check=True, timeout=120, cwd=ROOT)
        command = tool_command + ["-std=c11", "-O2", "-Wall", "-Wextra",
                    "-DCOMPILER_ROOT=" + c_string(ROOT.as_posix()),
                    "-DDEFAULT_CC=" + c_string(Path(tool).as_posix()),
                    "-DDEFAULT_RUNTIME=" + c_string(runtime.as_posix()),
                    str(ROOT / "src/lexer/lexer.c"), str(ROOT / "src/parser/parser.c"),
                    str(ROOT / "src/compiler/compiler.c"), str(ROOT / "src/platform/platform.c"),
                    str(ROOT / "driver.c"),
                    "-lm", "-o", str(output)]
        subprocess.run(command, check=True, timeout=180, cwd=ROOT)
        print(f"Built {output}\nAssembly runtime: {runtime}\nAssembler/linker driver: {tool}")
        return 0
    except (RuntimeError, OSError, subprocess.SubprocessError) as error:
        print(f"Build failed: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
