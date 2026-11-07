#ifndef COMPILER_PLATFORM_H
#define COMPILER_PLATFORM_H

/* Separate translation unit avoids Windows typedefs colliding with tokens. */
int sameFile(const char *source, const char *output);
int publishFile(const char *from, const char *to);
int executeProgram(const char *executable, const char *const *args);

#endif
