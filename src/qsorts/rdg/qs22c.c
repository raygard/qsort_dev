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

#define INSORTTHRESH    10          // if n < this use insertion sort
                                    // MUST be >= 2
#define MIDTHRESH       35          // < this use middle as pivot
#define MEDOF3THRESH    40          // < this use median-of-3 as pivot
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
            char *pivot = left + (nmemb / 2) * size;
            if (nmemb >= MIDTHRESH) {
                if (nmemb < MEDOF3THRESH) {
                    pivot = med3(left + size, pivot, right, compar);
                } else {
                    size_t k = (nmemb / 8) * size;
                    pivot = med3(
                        med3(left + size, left + k, left + k * 2, compar),
                        med3(pivot - k, pivot, pivot + k, compar),
                        med3(limit - k * 2, limit - k, right, compar), compar);
                }
            }
            SWAP(left, pivot);

            i = ii = left;                  // i scans left to right
            j = jj = limit;                 // j scans right to left
            for (;;) {
                while ((kj=COMP(j -= size, left)) > 0)
                    ;
                while (i < j && (ki=COMP(i += size, left)) < 0)
                    ;
                if (i >= j)                 // if pointers crossed
                    break;                  //   break loop
                if (ki || kj)
                    SWAP(i, j);
                if (kj == 0 && i!=j) {      // *i is eq pivot
                    ii += size;
                    if (i != ii)
                        SWAP(i, ii);
                }
                if (ki == 0) {              // *j is eq pivot
                    jj -= size;
                    if (j != jj)
                        SWAP(j, jj);
                }
            }
            SWAP(left, j);                  // move pivot into correct place

            ptrdiff_t lessthan = (j > ii + size) ? j - (size + ii) : 0;
            size_t k = min(ii - left, lessthan);
            vecswap(left + size, j - k, k);
            ptrdiff_t morethan = jj - size - j;
            k = min(morethan, limit - jj);
            vecswap(j + size, limit - k, k);

            if (size + lessthan > (size_t)morethan) {
                sp[0] = left;
                sp[1] = left + size + lessthan;
                left = limit - morethan;
            } else {
                sp[0] = limit - morethan;
                sp[1] = limit;
                limit = left + size + lessthan;
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
