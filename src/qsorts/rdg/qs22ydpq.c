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
#include <stddef.h>
#include <stdint.h>

#define INSORTTHRESH    27          // if n < this use insertion sort

typedef uint64_t WORD;

// if no uintptr_t, use Bentley-McIlroy trick (undefined behavior)
//#define ptr_to_int(p) ((char *)p - (char *)0)
#define ptr_to_int(p) ((uintptr_t)(void *)p)

#define ASWAP(a, b, t) ((void)(t = a, a = b, b = t))

#if COUNTSWAPS
#define SWAP(a, b) if (swap_type) swapfunc(a, b, size, swap_type);\
                    else do {tot_swaps += sizeof(WORD);\
                    ASWAP(*(WORD*)(a), *(WORD*)(b), temp_word);} while (0)
#else
#define SWAP(a, b) if (swap_type) swapfunc(a, b, size, swap_type);\
                    else ASWAP(*(WORD*)(a), *(WORD*)(b), temp_word)
#endif

#define  COMP(a, b)  ((*compar)((void *)(a), (void *)(b)))

static void swapfunc(char *a, char *b, size_t size, int swap_type)
{
    WORD temp_word;
    char temp_char;
#if COUNTSWAPS
    tot_swaps += size;
#endif
    if (swap_type < 2) {    // aligned, WORD sized
        do {
            ASWAP(*(WORD*)a, *(WORD*)b, temp_word);
            a += sizeof(WORD);
            b += sizeof(WORD);
        } while (size -= sizeof(WORD));
    } else {                // unaligned or not multiple of WORD size
        do {
            ASWAP(*a, *b, temp_char);
            a++;
            b++;
        } while (--size);
    }
}

static void dpq(char *left, char *right, size_t size, int swap_type,
                             int (*compar)(const void *, const void *), int div)
{
    WORD temp_word;

    if (left >= right)
        return;
    size_t len = (right - left) / size;
    if (len < INSORTTHRESH) {
        for (char *i = left + size; i <= right; i += size) {
            for (char *j = i; j != left && COMP(j - size, j) > 0; j -= size) {
                SWAP(j - size, j);
            }
        }
    } else {
        size_t third = (len / div) * size;

        // "medians"
        char *m1 = left + third, *m2 = right - third;
        if (m1 <= left)
            m1 = left + size;
        if (m2 >= right)
            m2 = right - size;
        if (COMP(m1, m2) < 0) {
            SWAP(m1, left);
            SWAP(m2, right);
        } else {
            SWAP(m1, right);
            SWAP(m2, left);
        }
        // pivots
        char *p1 = left, *p2 = right;

        // pointers
        char *less = left + size, *great = right - size;

        // sorting (partitioning)
        for (char *k = less; k <= great; k += size) {
            if (COMP(k, p1) < 0) {
                SWAP(k, less);
                less += size;
            } else if (COMP(k, p2) > 0) {
                while (k < great && COMP(great, p2) > 0) {
                    great -= size;
                }
                SWAP(k, great);
                great -= size;
                if (COMP(k, p1) < 0) {
                    SWAP(k, less);
                    less += size;
                }
            }
        }

        // swaps
        size_t dist = less > great ? 0 : great - less;

        if (dist < 13)
            div++;
        p1 = less - size;
        p2 = great + size;
        SWAP(p1, left);
        SWAP(p2, right);

        // subarrays
        if (less > left + 2 * size)
            dpq(left, less - 2 * size, size, swap_type, compar, div);
        if (right > great + 2 * size)
            dpq(great + 2 * size, right, size, swap_type, compar, div);

        // equal elements
        if (dist > (len - 13) && COMP(p1, p2) != 0) {
            for (char *k = less; k <= great; k += size) {
                if (COMP(k, p1) == 0) {
                    SWAP(k, less);
                    less += size;
                } else if (COMP(k, p2) == 0) {
                    SWAP(k, great);
                    great -= size;
                    if (COMP(k, p1) == 0) {
                        SWAP(k, less);
                        less += size;
                    }
                }
            }
        }

        // subarray
        if (COMP(p1, p2) < 0) {
            dpq(less, great, size, swap_type, compar, div);
        }
    }
}

void qsort(void *base, size_t nelems, size_t size,
                                     int (*compar)(const void *, const void *))
{
    int swap_type;
    if ((ptr_to_int(base) | size) % sizeof(WORD)) {
        swap_type = 2;  // unaligned or not multple of WORD size; swap bytes
    } else if (size > sizeof(WORD)) {
        swap_type = 1;  // aligned and multiple of WORD size
    } else {
        swap_type = 0;  // aligned and single WORD
    }
    dpq((char *)base, base + (nelems - 1) * size, size, swap_type, compar, 3);
}
