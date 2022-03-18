#define qsort uclibcng

#define attribute_hidden
#define libc_hidden_def(x)
#define __restrict

typedef int (*__compar_fn_t) (const void *, const void *);
typedef int (*__compar_d_fn_t) (const void *, const void *, void *);

#include <stddef.h>
static void qsort_r(void *a, size_t n, size_t es, __compar_d_fn_t cmp, void *thunk);

#define L_qsort_r
#include "libs/uclibc-ng/stdlib.c"

static int comp_wrapper(const void *a, const void *b, void *compar)
{
    return ((__compar_fn_t)compar)(a, b);
}

void qsort(void *base, size_t nmemb, size_t size, __compar_fn_t compar)
{
    qsort_r(base, nmemb, size, comp_wrapper, compar);
}
/*
 * NOTE 
 * #include <stdint.h>
 * appears twice in stdlib.c
 */
