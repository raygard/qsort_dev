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

    for (gap = 4; gap < nmemb; gap = 3 * gap + 1)
        ;
    while (gap /= 3) {
        for (i = (char *)base + gap * size; i < limit; i += size) {
            for (j = i; j >= (char *)base + gap * size && (compar)(j - gap * size, j) > 0; j -= gap * size) {
                size_t k = size;
                char temp, *a = j, *b = j - gap * size;
                do { temp = *a; *a++ = *b; *b++ = temp; } while (--k);
            }
        }
    }
}
