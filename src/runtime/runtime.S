/* Hand-written x86-64 runtime. Assemble with gcc/clang/zig cc (normal CRT).
 * Public ABI: all arguments are host integer arguments; reads return bits in
 * RAX, including doubles. Writes take (bits, line); reads take (line).
 * cc_error(line, reason) does not return. cc_finish() diagnoses at line zero.
 * No locale is installed: token whitespace is the six C-locale ASCII bytes.
 */
#if !defined(__x86_64__) && !defined(_M_X64)
#error "The runtime requires x86-64"
#endif
#if !defined(_WIN32) && !defined(__linux__)
#error "The runtime supports Windows COFF and Linux ELF only"
#endif

.intel_syntax noprefix
#ifdef _WIN32
#define A1 rcx
#define A1D ecx
#define A2 rdx
#define A2D edx
#define A3 r8
#define A3D r8d
#define A4 r9
.section .rdata,"dr"
#else
#define A1 rdi
#define A1D edi
#define A2 rsi
#define A2D esi
#define A3 rdx
#define A3D edx
#define A4 rcx
.section .rodata
#endif
.Lint_format:     .asciz "%lld\n"
.Lreal_format:    .asciz "%.17g\n"
.Lerror_format:   .asciz "runtime error at line %d: %s\n"
.Linteger_input:  .asciz "invalid integer input"
.Lreal_input:     .asciz "invalid real input"
.Lempty_input:    .asciz "unexpected end of input"
.Lread_failure:   .asciz "input read failed"
.Llong_input:     .asciz "input token too long"
.Lnul_input:      .asciz "NUL byte in input"
.Lwrite_error:    .asciz "output write failed"
.Lflush_error:    .asciz "output flush failed"

.text
/* Every frame includes Win64's 32-byte outgoing shadow area. Only RBP is
 * nonvolatile among the registers used here; no nonvolatile XMM is touched.
 * Entry RSP is 8 mod 16, so push RBP plus a multiple of 16 aligns every call.
 */
.macro FUNCTION name
#ifdef _WIN32
    .seh_proc \name
#else
    .type \name, @function
#endif
\name:
.endm
.macro END_FUNCTION name
#ifdef _WIN32
    .seh_endproc
#else
    .size \name, .-\name
#endif
.endm
.macro ENTER bytes
    push rbp
#ifdef _WIN32
    .seh_pushreg rbp
#endif
    mov rbp, rsp
    sub rsp, \bytes
#ifdef _WIN32
    .seh_stackalloc \bytes
    .seh_endprologue
#endif
.endm
.macro LEAVE bytes
    add rsp, \bytes
    pop rbp
    ret
.endm
.macro ERRNO_ADDRESS
#ifdef _WIN32
    call _errno
#else
    call __errno_location
#endif
.endm
/* UCRT and Zig's MinGW CRT expose __acrt_iob_func; unlike __iob_func,
 * this does not depend on the CRT-specific size/layout of FILE.
 */
.macro STREAM index, symbol
#ifdef _WIN32
    mov ecx, \index
    call __acrt_iob_func
    mov A1, rax
#else
    mov rax, QWORD PTR [rip + \symbol@GOTPCREL]
    mov A1, QWORD PTR [rax]
#endif
.endm

.globl cc_read_int
FUNCTION cc_read_int
    xor eax, eax
    jmp .Lread_common
END_FUNCTION cc_read_int

.globl cc_read_real
FUNCTION cc_read_real
    mov eax, 1
    jmp .Lread_common
END_FUNCTION cc_read_real

FUNCTION .Lread_common
    /* Locals: line -4, real flag -8, length -16, errno pointer -24,
     * end pointer -32, result -40, buffer [-576,-65] (512 bytes).
     * The lowest 32 bytes of the 608-byte frame are outgoing shadow space.
     */
    ENTER 608
    mov DWORD PTR [rbp - 4], A1D
    mov DWORD PTR [rbp - 8], eax
    mov QWORD PTR [rbp - 16], 0
.Lskip_space:
    call getchar
    cmp eax, -1
    je .Lread_eof
    cmp eax, 32
    je .Lskip_space
    cmp eax, 9
    jb .Ltoken_byte
    cmp eax, 13
    jbe .Lskip_space
.Ltoken_byte:
    test eax, eax
    je .Lread_nul
    mov rdx, QWORD PTR [rbp - 16]
    cmp rdx, 511
    jae .Lread_long
    mov BYTE PTR [rbp + rdx - 576], al
    inc rdx
    mov QWORD PTR [rbp - 16], rdx
    call getchar
    cmp eax, -1
    je .Lread_eof
    cmp eax, 32
    je .Ltoken_end
    cmp eax, 9
    jb .Ltoken_byte
    cmp eax, 13
    ja .Ltoken_byte
.Ltoken_end:
    mov rdx, QWORD PTR [rbp - 16]
    mov BYTE PTR [rbp + rdx - 576], 0
    ERRNO_ADDRESS
    mov QWORD PTR [rbp - 24], rax
    mov DWORD PTR [rax], 0
    lea A1, [rbp - 576]
    lea A2, [rbp - 32]
    cmp DWORD PTR [rbp - 8], 0
    jne .Lparse_real
    mov A3D, 10
    call strtoll
    jmp .Lparsed
.Lparse_real:
    call strtod
    movq rax, xmm0
.Lparsed:
    mov QWORD PTR [rbp - 40], rax
    mov rdx, QWORD PTR [rbp - 24]
    cmp DWORD PTR [rdx], 0
    jne .Lread_invalid
    mov rdx, QWORD PTR [rbp - 32]
    lea rcx, [rbp - 576]
    cmp rdx, rcx
    je .Lread_invalid
    cmp BYTE PTR [rdx], 0
    jne .Lread_invalid
    cmp DWORD PTR [rbp - 8], 0
    je .Lread_done
    /* An all-ones exponent rejects both infinity and every NaN payload. */
    mov rdx, rax
    shr rdx, 52
    and edx, 2047
    cmp edx, 2047
    je .Lread_invalid
.Lread_done:
    mov rax, QWORD PTR [rbp - 40]
    LEAVE 608
.Lread_invalid:
    lea A2, [rip + .Linteger_input]
    cmp DWORD PTR [rbp - 8], 0
    je .Lread_error
    lea A2, [rip + .Lreal_input]
    jmp .Lread_error
.Lread_eof:
    /* getchar uses EOF for both end-of-file and read failure. A token ending
     * at clean EOF is valid, but a failed read must not accept a partial token.
     */
    STREAM 0, stdin
    call ferror
    test eax, eax
    jne .Lread_failed
    cmp QWORD PTR [rbp - 16], 0
    jne .Ltoken_end
.Lread_empty:
    lea A2, [rip + .Lempty_input]
    jmp .Lread_error
.Lread_failed:
    lea A2, [rip + .Lread_failure]
    jmp .Lread_error
.Lread_long:
    lea A2, [rip + .Llong_input]
    jmp .Lread_error
.Lread_nul:
    lea A2, [rip + .Lnul_input]
.Lread_error:
    mov A1D, DWORD PTR [rbp - 4]
    call cc_error
    ud2
END_FUNCTION .Lread_common

.globl cc_write_int
FUNCTION cc_write_int
    ENTER 48
    mov DWORD PTR [rbp - 4], A2D
    mov A2, A1
    lea A1, [rip + .Lint_format]
#ifndef _WIN32
    xor eax, eax
#endif
    call printf
    test eax, eax
    js .Lint_write_error
    LEAVE 48
.Lint_write_error:
    mov A1D, DWORD PTR [rbp - 4]
    lea A2, [rip + .Lwrite_error]
    call cc_error
    ud2
END_FUNCTION cc_write_int

.globl cc_write_real
FUNCTION cc_write_real
    ENTER 48
    mov DWORD PTR [rbp - 4], A2D
#ifdef _WIN32
    /* Win64 varargs duplicate a double into its positional integer slot. */
    mov rdx, rcx
    movq xmm1, rdx
#else
    movq xmm0, rdi
    mov eax, 1
#endif
    lea A1, [rip + .Lreal_format]
    call printf
    test eax, eax
    js .Lreal_write_error
    LEAVE 48
.Lreal_write_error:
    mov A1D, DWORD PTR [rbp - 4]
    lea A2, [rip + .Lwrite_error]
    call cc_error
    ud2
END_FUNCTION cc_write_real

.globl cc_error
FUNCTION cc_error
    ENTER 48
    mov DWORD PTR [rbp - 4], A1D
    mov QWORD PTR [rbp - 16], A2
    STREAM 2, stderr
    lea A2, [rip + .Lerror_format]
    mov A3D, DWORD PTR [rbp - 4]
    mov A4, QWORD PTR [rbp - 16]
#ifndef _WIN32
    xor eax, eax
#endif
    call fprintf
    mov A1D, 1
    call exit
    ud2
END_FUNCTION cc_error

.globl cc_finish
FUNCTION cc_finish
    ENTER 32
    STREAM 1, stdout
    call fflush
    test eax, eax
    jne .Lfinish_error
    LEAVE 32
.Lfinish_error:
    xor A1D, A1D
    lea A2, [rip + .Lflush_error]
    call cc_error
    ud2
END_FUNCTION cc_finish

#ifndef _WIN32
.section .note.GNU-stack,"",@progbits
#endif
