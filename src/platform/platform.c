#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#endif

int sameFile(const char *source, const char *output)
{
#ifdef _WIN32
    BY_HANDLE_FILE_INFORMATION a, b;
    DWORD share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    HANDLE first = CreateFileA(source, 0, share, NULL, OPEN_EXISTING,
                               FILE_FLAG_BACKUP_SEMANTICS, NULL);
    HANDLE second = CreateFileA(output, 0, share, NULL, OPEN_EXISTING,
                                FILE_FLAG_BACKUP_SEMANTICS, NULL);
    int same = first != INVALID_HANDLE_VALUE && second != INVALID_HANDLE_VALUE &&
        GetFileInformationByHandle(first, &a) && GetFileInformationByHandle(second, &b) &&
        a.dwVolumeSerialNumber == b.dwVolumeSerialNumber &&
        a.nFileIndexHigh == b.nFileIndexHigh && a.nFileIndexLow == b.nFileIndexLow;
    if (first != INVALID_HANDLE_VALUE) CloseHandle(first);
    if (second != INVALID_HANDLE_VALUE) CloseHandle(second);
    return same;
#else
    struct stat a, b;
    return stat(source, &a) == 0 && stat(output, &b) == 0 &&
           a.st_dev == b.st_dev && a.st_ino == b.st_ino;
#endif
}

int publishFile(const char *from, const char *to)
{
#ifdef _WIN32
    if (MoveFileExA(from, to, MOVEFILE_REPLACE_EXISTING)) return 0;
    fprintf(stderr, "compiler: cannot publish output '%s' (Windows error %lu)\n", to,
            (unsigned long)GetLastError());
#else
    if (rename(from, to) == 0) return 0;
    fprintf(stderr, "compiler: cannot publish output '%s': %s\n", to, strerror(errno));
#endif
    return 1;
}

#ifdef _WIN32
/* The Windows CRT joins spawn arguments without escaping them. Quote for the
 * child's CRT parser, not for a shell (which is never invoked). */
static char *quoteArgument(const char *arg)
{
    char *quoted = malloc(strlen(arg) * 2 + 3), *out;
    if (!quoted) return NULL;
    out = quoted; *out++ = '"';
    while (*arg) {
        size_t slashes = 0;
        while (*arg == '\\') { ++slashes; ++arg; }
        if (*arg == '"' || !*arg) slashes *= 2;
        while (slashes--) *out++ = '\\';
        if (*arg == '"') *out++ = '\\';
        if (*arg) *out++ = *arg++;
    }
    *out++ = '"'; *out = '\0';
    return quoted;
}
#endif

int executeProgram(const char *executable, const char *const *args)
{
#ifdef _WIN32
    size_t count = 0, i;
    char **quoted;
    intptr_t status;
    while (args[count]) ++count;
    quoted = calloc(count + 1, sizeof(*quoted));
    if (!quoted) { fputs("compiler: out of memory\n", stderr); return 1; }
    for (i = 0; i < count; ++i) {
        quoted[i] = quoteArgument(args[i]);
        if (!quoted[i]) break;
    }
    status = i == count ? _spawnvp(_P_WAIT, executable, (const char *const *)quoted) : -1;
    if (status == -1)
        fprintf(stderr, "compiler: cannot launch C compiler '%s': %s\n", executable, strerror(errno));
    for (i = 0; i < count; ++i) free(quoted[i]);
    free(quoted);
    return status != 0;
#else
    pid_t pid = fork();
    if (pid < 0) { perror("compiler: fork"); return 1; }
    if (pid == 0) {
        execvp(executable, (char *const *)args);
        fprintf(stderr, "compiler: cannot launch C compiler '%s': %s\n", executable, strerror(errno));
        _exit(127);
    }
    int status;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) { perror("compiler: waitpid"); return 1; }
    }
    return !WIFEXITED(status) || WEXITSTATUS(status) != 0;
#endif
}
