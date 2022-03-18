/* Copied memswap() from memswap.c */
/*
 * memswap()
 *
 * Swaps the contents of two nonoverlapping memory areas.
 * This really could be done faster...
 */

//#include <string.h>
#include <stddef.h>

void memswap(void *m1, void *m2, size_t n)
{
	char *p = m1;
	char *q = m2;
	char tmp;

	while (n--) {
		tmp = *p;
		*p = *q;
		*q = tmp;

		p++;
		q++;
	}
}
/* END Copied memswap() from memswap.c */

#define qsort klibc
#include "libs/klibc-2.0.10/usr/klibc/qsort.c"
