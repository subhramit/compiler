@echo off
setlocal
rem Build the C implementation and assemble the runtime without Python or make.
set "ROOT=%~dp0"
set "ROOT=%ROOT:\=/%"
set "TOOL=%~1"
if not defined TOOL if defined CC set "TOOL=%CC%"
if not defined TOOL if exist "%ROOT%build/toolchain/ziglang/zig.exe" set "TOOL=%ROOT%build/toolchain/ziglang/zig.exe"
if not defined TOOL set "TOOL=gcc"
set "SUBCOMMAND="
for %%I in ("%TOOL%") do if /i "%%~nI"=="zig" set "SUBCOMMAND=cc"
set "TOOL_C=%TOOL:\=/%"
if not exist "%ROOT%build" mkdir "%ROOT%build"
if errorlevel 1 exit /b 1
"%TOOL%" %SUBCOMMAND% -c -x assembler-with-cpp "%ROOT%src/runtime/runtime.S" -o "%ROOT%build/runtime.o"
if errorlevel 1 exit /b 1
"%TOOL%" %SUBCOMMAND% -std=c11 -O2 -Wall -Wextra "-DCOMPILER_ROOT=\"%ROOT%\"" "-DDEFAULT_CC=\"%TOOL_C%\"" "-DDEFAULT_RUNTIME=\"%ROOT%build/runtime.o\"" "%ROOT%src/lexer/lexer.c" "%ROOT%src/parser/parser.c" "%ROOT%src/compiler/compiler.c" "%ROOT%src/platform/platform.c" "%ROOT%driver.c" -lm -o "%ROOT%build/compiler.exe"
if errorlevel 1 exit /b 1
echo Built "%ROOT%build/compiler.exe"
echo Runtime: "%ROOT%build/runtime.o"
exit /b 0
