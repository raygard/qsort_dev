#define qsort nlopt
#include "libs/nlopt/qsort_r.c"

// wrapping a qsort_r - type function

//void qsort_r_fallback(void *a, size_t n, size_t es, void *thunk, cmp_t *cmp)
//typedef int		 cmp_t(void *, const void *, const void *);

typedef int (*comp)(const void *, const void *);

int cmp(void *compar, const void *a, const void *b)
{
    return ((comp)compar)(a, b);
}

void qsort(void *base, size_t nmemb, size_t size,
        int (*compar)(const void *, const void *))
{
    //qsort_r_fallback(base, nmemb, size, compar, cmp);
    nlopt_qsort_r(base, nmemb, size, compar, cmp);
}
