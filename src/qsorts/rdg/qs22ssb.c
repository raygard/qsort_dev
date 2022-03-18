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
#if COUNTSWAPS
extern unsigned long long tot_swaps;
#endif
// ssort()  --  Fast, small, qsort()-compatible Shell sort
#include <stddef.h>

void
ssort(void *base,
        size_t nmemb,
        size_t size,
        int (*compar)(const void *, const void *))
{
    char *limit = (char *)base + nmemb * size;
    size_t gap;
    char *i, *j;
    int k;

    /* Tokuda: h[k] = ceil((9 * (9 / 4) ** k - 4) / 5); (9/4) * h[k] < N
     * Ciura: h[] = {1, 4, 10, 23, 57, 132, 301, 701, 1750};
     * Tokuda's sequence is roughly h[k]/h[k-1] = 2.25 after 10 elements.
     * These are not his but are a continuation of Ciura at that ratio
     */
    size_t h[] = {1698794540, 755019795, 335564353, 149139712, 66284316, 29459696,
        13093198, 5819199, 2586310, 1149471, 510876, 227056, 100913, 44850, 19933, 8859, 3937,
        // Ciura's sequence
        1750, 701, 301, 132, 57, 23, 10, 4, 1, 0};
    for (k = 0; h[k] > (nmemb + 4) / 3; k++)
        ;
    for (gap = h[k]; gap > 0; gap = h[++k]) {
        for (i = (char *)base + gap * size; i < limit; i += size) {
            for (j = i; j >= (char *)base + gap * size && (compar)(j - gap * size, j) > 0; j -= gap * size) {
#if COUNTSWAPS
tot_swaps += size;
#endif
                size_t k = size;
                char temp, *a = j, *b = j - gap * size;
                do { temp = *a; *a++ = *b; *b++ = temp; } while (--k);
            }
        }
    }
}
