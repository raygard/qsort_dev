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

#define INSORTTHRESH    5           // if n < this use insertion sort
                                    // MUST be >= 2
#define MIDTHRESH       20          // < this use middle as pivot
#define MEDOF3THRESH    50          // < this use median-of-3 as pivot
                                    // larger subfiles use med-of-3-medians

#define min(a,b) (((a) < (b)) ? (a) : (b))

typedef uint64_t WORD;

// if no uintptr_t, use Bentley-McIlroy trick (undefined behavior)
//#define ptr_to_int(p) (p-(char*)0)
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

#define vecswap(a, b, n) if ((n) > 0) swapfunc(a, b, n, swap_type)

static char *med3(char *a, char *b, char *c, int (*compar)(const void *, const void *))
{
    return COMP(a, b) < 0 ?
        (COMP(b, c) < 0 ? b : COMP(a, c) < 0 ? c : a) :
        (COMP(b, c) > 0 ? b : COMP(a, c) > 0 ? c : a);
}

void qsort(void *base, size_t nmemb, size_t size,
                                     int (*compar)(const void *, const void *))
{
    char *stack[2*8*sizeof(size_t)], **sp = stack; // stack and stack pointer
    char *left = base;                      // set up char * base pointer
    char *limit = left + nmemb * size;      // pointer past end of array
    char *i, *ii, *j, *jj;                  // scan pointers
    int ki = 0, kj = 0;

    WORD temp_word;
    int swap_type;
    if ((ptr_to_int(left) | size) % sizeof(WORD)) {
        swap_type = 2;  // unaligned or not multple of WORD size; swap bytes
    } else if (size > sizeof(WORD)) {
        swap_type = 1;  // aligned and multiple of WORD size
    } else {
        swap_type = 0;  // aligned and single WORD
    }

    for (;;) {
        nmemb = (limit - left) / size;
        if (nmemb >= INSORTTHRESH) {        // otherwise use insertion sort
            char *right = limit - size;
            char *p = left + (nmemb / 2) * size;
            if (nmemb >= MIDTHRESH) {
                if (nmemb < MEDOF3THRESH) {
                    size_t k = ((nmemb - 1) / 2) * size;
                    p = med3(left, left + k, left + k * 2, compar);
                } else {
                    size_t k = ((nmemb - 1) / 8) * size;
                    p = med3(
                        med3(left, left + k, left + k * 2, compar),
                        med3(left + k * 3, left + k * 4, left + k * 5, compar),
                        med3(left + k * 6, left + k * 7, left + k * 8, compar), compar);
                }
            }

            i = ii = left;                  // i scans left to right
            j = jj = right;                 // j scans right to left
            for (;;) {

                while (i <= j) {
                    if (i == p || ((ki = COMP(i, p)) < 0))
                        i += size;
                    else if (ki)
                        break;
                    else {
                        if (ii == p)
                            p = i;
                        else if (i != ii)
                            SWAP(i, ii);
                        ii += size;
                        i += size;
                    }
                }

                while (i < j) {
                    if (j == p || ((kj = COMP(j, p)) > 0))
                        j -= size;
                    else if (kj)
                        break;
                    else {
                        if (jj == p)
                            p = j;
                        else if (j != jj)
                            SWAP(j, jj);
                        jj -= size;
                        j -= size;
                    }
                }
                if (i >= j)
                    break;
                SWAP(i, j);
                i += size;
                j -= size;
            }

            if (p < i)
                i -= size;
            if (p != i)
                SWAP(p, i);

            ptrdiff_t lessthan = i - ii;
            size_t k = min(lessthan, ii - left);
            vecswap(left, i - k, k);
            ptrdiff_t morethan = jj - i;
            k = min(morethan, right - jj);
            vecswap(i + size, limit - k, k);

            if (lessthan > morethan) {
                sp[0] = left;
                sp[1] = left + lessthan;
                left = limit - morethan;
            } else {
                sp[0] = limit - morethan;
                sp[1] = limit;
                limit = left + lessthan;
            }
            sp += 2;                        // increment stack pointer
        } else {                // else subfile is small, use insertion sort
            for (i = left + size; i < limit; i += size) {
                for (j = i; j != left && COMP(j - size, j) > 0; j -= size) {
                    SWAP(j - size, j);
                }
            }
            if (sp != stack) {              // if any entries on stack
                sp -= 2;                    // pop the left and limit
                left = sp[0];
                limit = sp[1];
            } else                          // else stack empty, done
                break;
        }
    }
}
