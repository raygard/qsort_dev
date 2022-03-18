#define qsort rg91ss

#include <stddef.h>

static int ssort(char *, unsigned, unsigned, int (*comp)());

void qsort(void *base, size_t nmemb, size_t size,
        int (*compar)(const void *, const void *))
{
    (void)ssort(base, nmemb, size, compar);
}

#include "qsorts/rdg/rg_ssort.c"
