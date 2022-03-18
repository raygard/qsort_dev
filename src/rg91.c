#define qsort rg91
#define SWAP_INTS
#include <stddef.h>

static void swap_chars(char *a, char *b, size_t nbytes);
static void swap_ints(char *ap, char *bp, size_t nints);

#include "qsorts/rdg/rg_qsort.c"
