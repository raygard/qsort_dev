#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

static inline void fswap(void *a, void *b, size_t size) {
  // unaligned accesses on x86_64 are cheap
  uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
  switch (size) {
    case 1:  u8 = *( uint8_t*)a; *( uint8_t*)a = *( uint8_t*)b; *( uint8_t*)b =  u8; return;
    case 2: u16 = *(uint16_t*)a; *(uint16_t*)a = *(uint16_t*)b; *(uint16_t*)b = u16; return;
    case 4: u32 = *(uint32_t*)a; *(uint32_t*)a = *(uint32_t*)b; *(uint32_t*)b = u32; return;
    case 8: u64 = *(uint64_t*)a; *(uint64_t*)a = *(uint64_t*)b; *(uint64_t*)b = u64; return;
  }
#define min(a, b) ((a) < (b) ? (a) : (b))
  for (char tmp[256]; size; size -= min(size, sizeof(tmp))) {
    memcpy(tmp, a, min(size, sizeof(tmp)));
    memcpy(a,   b, min(size, sizeof(tmp)));
    memcpy(b, tmp, min(size, sizeof(tmp)));
    a = (char *) a + min(size, sizeof(tmp));
    b = (char *) b + min(size, sizeof(tmp));
  }
}

#define swap(a, b) fswap(a, b, size)
#define vecswap(a, b, n) 	if ((n) > 0) fswap(a, b, n)

#define cmp(a, b) (f1 ? f1(a, b) : f2(a, b, arg))
static inline size_t med3(void *base, size_t size, size_t a, size_t b, size_t c,
    int (*f1)(const void *, const void *),
    int (*f2)(const void *, const void *, void *),
    void *arg) {

  typedef char type[size];
  type *array = base;

	return cmp(array[a], array[b]) < 0 ?
	       (cmp(array[b], array[c]) < 0 ? b : (cmp(array[a], array[c]) < 0 ? c : a ))
         :(cmp(array[b], array[c]) > 0 ? b : (cmp(array[a], array[c]) < 0 ? a : c ));
}
#define med3(a, b, c) med3(array, size, a, b, c, f1, f2, arg)

static void sift_down(void *base, size_t size, size_t top, size_t bottom,
    int (*f1)(const void *, const void *),
    int (*f2)(const void *, const void *, void *),
    void *arg) {

  typedef char type[size];
  type *array = base;

  for (size_t child; top * 2 + 1 <= bottom; top = child) {
    child = top * 2 + 1;
    if (child + 1 <= bottom && cmp(array[child], array[child+1]) < 0) child++;
    if (cmp(array[child], array[top]) < 0) return;
    swap(array[child], array[top]);
  }
}

static void actual_qsort(void *base, size_t nmemb, size_t size, size_t recur,
    int (*f1)(const void *, const void *),
    int (*f2)(const void *, const void *, void *),
    void *arg) {
	int cmp_result;

  typedef char type[size];
  type *array = base;
  size_t a, b, c, d, s;

  // introsort
  while (nmemb > 10) {

    // switch to heap sort if recursion is too deep
    if (!recur) {
#define sift_down(arr, top, bottom) sift_down(arr, size, top, bottom, f1, f2, arg)
      for (ssize_t top = (nmemb-2) / 2; top >= 0; top--) // signed
        sift_down(array, top, nmemb-1);

      for (size_t i = nmemb-1; i > 0; sift_down(array, 0, --i))
        swap(array[0], array[i]);
#undef sift_down
      return;
    }

    // recursive 3 way quicksort
    // largely adapted from "engineering a sort function"
    size_t l = 0, m = nmemb / 2, n = nmemb - 1, med;
    if (nmemb > 40) {
      s = nmemb / 8;
      l = med3(l, l + s, l + 2 * s);
      m = med3(m - s, m, m + s);
      n = med3(n - 2 * s, n - s, n);
    }

    if ((med = med3(l, m, n))) swap(array[0], array[med]);

    size_t swaps = 0;
    for (a = b = 1, c = d = nmemb - 1; ; ) {
      while (b <= c && (cmp_result = cmp(array[b], array[0])) <= 0) {
        if (cmp_result == 0) swap(array[a++], array[b]), swaps = 1;
        b++;
      }
      while (b <= c && (cmp_result = cmp(array[c], array[0])) >= 0) {
        if (cmp_result == 0) swap(array[c], array[d--]), swaps = 1;
        c--;
      }
      if (b > c) break;
      swap(array[b++], array[c--]);
      swaps = 1;
    }

    // this is a cool idea from freebsd, but limit it to avoid O(n^2) time
    if (swaps == 0 && nmemb < 256) break;
  
    s = min(a, b - a);             vecswap(array[0], array[b - s], s*size);
    s = min(d - c, nmemb - d - 1); vecswap(array[b], array[nmemb - s], s*size);
    recur--;
    if ((s = b - a) > 1) actual_qsort(array, s, size, recur, f1, f2, arg);
    if ((s = d - c) > 1) {
      array = &array[nmemb - s];
      nmemb = s;
    }
    else break;
  }

  // final pass with small arrays
  for (size_t i = 1; i < nmemb; i++)
    for (size_t j = i; j > 0 && cmp(array[j - 1], array[j]) > 0; j--)
      swap(array[j], array[j-1]);
}

void my_qsort(void *base, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *)) {
  size_t depth = nmemb, log2 = 0;
  while (depth >>= 1) log2++;
  actual_qsort(base, nmemb, size, 2*log2, compar, NULL, NULL);
}

#undef swap
#undef cmp
#undef min
#undef med3
#undef vecswap
