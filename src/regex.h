#ifndef _REGEX_H
#define _REGEX_H
#include <sys/types.h>

/* simplify regex struct for compability with other C compilers */
typedef struct {
    size_t re_nsub;
    char __padding[128];
} regex_t;

typedef int regoff_t;
typedef struct { regoff_t rm_so; regoff_t rm_eo; } regmatch_t;

int regcomp(regex_t *, const char *, int);
int regexec(const regex_t *, const char *, size_t, regmatch_t *, int);
void regfree(regex_t *);

#define REG_EXTENDED 1
#define REG_ICASE    2
#define REG_NOSUB    4
#define REG_NEWLINE  8
#endif