//  License: 0BSD
//
//  Copyright 2022 Raymond Gardner
//
//  Permission to use, copy, modify, and/or distribute this software for any
//  purpose with or without fee is hereby granted.
//
//  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
//  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
//  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
//  SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
//  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
//  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
//  IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
#include <stddef.h>

inline static void swapn(char *a, char *b, size_t size)
{
    char tmp;
    do {
        tmp = *b; *b++ = *a; *a++ = tmp;
    } while (--size);
}

#define comp(i, j) (compar(base + (i - 1) * size, base + (j - 1) * size))

#define swap(i, j) (swapn(base + (i - 1) * size, base + (j - 1) * size, size))

inline static void siftup(size_t i, size_t n, void  *base, size_t size,
        int (*compar)(const void *, const void *))
{
    while (2 * i <= n) {
        size_t j = 2 * i;
        if (j < n) {
            if (comp(j, j+1) < 0)
                j++;
        }
        if (comp(i, j) < 0) {
            swap(i, j);
            i = j;
        } else {
            i = n;
        }
    }
}

// heapsort -- Floyd's treesort3 -- CACM Dec. 1964
void qsort(void  *base, size_t n, size_t size,
        int (*compar)(const void *, const void *))
{
    size_t i;
    for (i = n / 2; i >= 1; i--)
        siftup(i, n, base, size, compar);
    for (i = n; i >= 2; i--) {
        swap(1, i);
        siftup(1, i-1, base, size, compar);
    }
}
