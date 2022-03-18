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

// heapsort -- adapted from Knuth, The Art of Computer Programming, Vol 3 (1973 ed), algorithm H
void qsort(void *base, size_t n, size_t size, int (*compar)(const void *, const void *))
{
    int l, r, i, j;
    char *h;
    char *v = base;
    if (n < 2)
        return;
    l = (n / 2);
    r = n - 1;
    for (;;) {
        if (l > 0) {
            l--;
        } else {
            swapn(v + r * size, v, size);
            r--;
            if (r == 0)
                return;
        }
        j = l;
        h = v + l * size;
        for (;;) {
            i = j;
            j = j * 2 + 1;
            if (j <= r) {
                if (j < r && compar(v + j * size, v + j * size + size) < 0)
                    j++;
                if (compar(h, v + j * size) < 0) {
                    swapn(v + i * size, v + j * size, size);
                    h = v + j * size;
                } else
                    break;
            } else
                break;
        }
    }
}
