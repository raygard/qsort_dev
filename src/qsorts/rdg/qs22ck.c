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
#include <string.h>

#define INSORTTHRESH    10          // if n < this use insertion sort
                                    // MUST be >= 2
#define MIDTHRESH       35          // < this use middle as pivot
#define MEDOF3THRESH    40          // < this use median-of-3 as pivot
                                    // larger subfiles use med-of-3-medians

#define min(a,b) (((a) < (b)) ? (a) : (b))

typedef int32_t WORD;
typedef int64_t DWORD;
typedef void *pref_typ;

// if no uintptr_t, use Bentley-McIlroy trick (undefined behavior)
//#define ptr_to_int(p) (p-(char*)0)
#define ptr_to_int(p) ((uintptr_t)(void *)p)

#define ASWAP(a, b, t) ((void)(t = a, a = b, b = t))

#if COUNTSWAPS
#define SWAP(a, b) if (swap_type) swapf(a, b, size);\
    else do {tot_swaps += sizeof(pref_typ);\
            pref_typ t; ASWAP(*(pref_typ*)(a), *(pref_typ*)(b), t);} while (0)
#else
#define SWAP(a, b) if (swap_type) swapf(a, b, size);\
    else do {pref_typ t; ASWAP(*(pref_typ*)(a), *(pref_typ*)(b), t);} while (0)
#endif

#define  COMP(a, b)  ((*compar)((void *)(a), (void *)(b)))

static void swapbytes(void *a0, void *b0, size_t n)
{
#if COUNTSWAPS
    tot_swaps += n;
#endif
    char *a = a0, *b = b0, t;
    do {ASWAP(*a, *b, t); a++; b++;} while (--n);
}

static void swapdword(void *a0, void *b0, size_t n)
{
#if COUNTSWAPS
    tot_swaps += sizeof(DWORD);
#endif
    char *a = a0, *b = b0, t[8];
    memcpy(t, a, 8);
    memcpy(a, b, 8);
    memcpy(b, t, 8);
    (void)n;
}

static void swapdwords(void *a0, void *b0, size_t n)
{
#if COUNTSWAPS
    //tot_swaps += n * sizeof(DWORD);
    tot_swaps += n;
#endif
    char *a = a0, *b = b0, t[8];
    do {
        memcpy(t, a, 8);
        memcpy(a, b, 8);
        memcpy(b, t, 8);
        a += 8;
        b += 8;
        n -= 8;
    } while (n);
}

static void swapword(void *a0, void *b0, size_t n)
{
#if COUNTSWAPS
    tot_swaps += sizeof(WORD);
#endif
    char *a = a0, *b = b0, t[4];
    memcpy(t, a, 4);
    memcpy(a, b, 4);
    memcpy(b, t, 4);
    (void)n;
}

static void swapwords(void *a0, void *b0, size_t n)
{
#if COUNTSWAPS
    //tot_swaps += n * sizeof(WORD);
    tot_swaps += n;
#endif
    char *a = a0, *b = b0, t[4];
    do {
        memcpy(t, a, 4);
        memcpy(a, b, 4);
        memcpy(b, t, 4);
        a += 4;
        b += 4;
        n -= 4;
    } while (n);
}

typedef void (*swapf_typ)(void *, void *, size_t);

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
    char *left = base;                     // set up char * base pointer
    char *limit = left + nmemb * size;    // pointer past end of array
    char *i, *ii, *j, *jj;                  // scan pointers
    int ki = 0, kj = 0;
    int swap_type = 1;
    swapf_typ swapf, vecswapf;

    vecswapf = swapf = swapbytes;
    if ((ptr_to_int(left) | size) % sizeof(WORD)) {
        ;  // unaligned or not multple of WORD size; swap bytes
    } else if (size == sizeof(DWORD)) {
        swapf = swapdword;
        vecswapf = swapdwords;
        if (size == sizeof(pref_typ))
            swap_type = 0;
    } else if (size == sizeof(WORD)) {
        swapf = swapword;
        vecswapf = swapwords;
        if (size == sizeof(pref_typ))
            swap_type = 0;
    } else if ((size % sizeof(DWORD)) == 0) {
        swapf = vecswapf = swapdwords;
    } else if ((size % sizeof(WORD)) == 0) {
        swapf = vecswapf = swapwords;
    }

    for (;;) {
        nmemb = (limit - left) / size;
        for (i = left + size; i < limit && COMP(i - size, i) <= 0; i += size)
            ;
        if (i == limit)                     // if already in order
            goto pop;
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

            i = ii = left;                // i scans left to right
            j = jj = limit;               // j scans right to left
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
            if (k)
                vecswapf(left + size, j - k, k);
            ptrdiff_t morethan = jj - size - j;
            k = min(morethan, limit - jj);
            if (k)
                vecswapf(j + size, limit - k, k);

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
pop:
            if (sp != stack) {              // if any entries on stack
                sp -= 2;                    // pop the left and limit
                left = sp[0];
                limit = sp[1];
            } else                          // else stack empty, done
                break;
        }
    }
}
