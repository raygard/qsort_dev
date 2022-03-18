#define qsort netbsd_old

//#define _DIAGASSERT(x)

typedef unsigned char u_char;

#if 0
static void insertion_sort(), quick_sort();


void insertion_sort(void *bot, nmemb, size, compar)
	void *bot;
	size_t nmemb, size;
	int (*compar) __P((const void *, const void *));
#endif

static void
insertion_sort(
	char *bot,
	int nmemb, 
	register int size,
    int (*compar)());

static void
quick_sort(
	char *bot,
	int nmemb, 
	register int size,
    int (*compar)());

#define __P(x) x

#include "libs/netbsd/qsort_1_1.c"
