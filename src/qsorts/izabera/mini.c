#include <string.h>
#include <stdint.h>
#include <sys/types.h>

static void swap(void *a_, void *b_, size_t size) {
  char *a = a_, *b = b_, tmp;
  for (size_t i = 0; i < size; i++) {
    tmp = a[i];
    a[i] = b[i];
    b[i] = tmp;
  }
}

void mini_qsort(void *base, size_t nmemb, size_t size, int (*cmp)(const void *, const void *)) {
  const static unsigned short gaps[] = {
#if 0
    1, 4, 10, 23, 57, 132, 301, 701, // this is the original sequence from ciura
    1750, // this was in ciura's paper too, but he kinda stopped caring because of diminishing returns
    3938, 8861, 19938, 44861 // gaps[i] = gaps[i-1] * 2.25
#endif
    44861, 19938, 8861, 3938, 1750, 701, 301, 132, 57, 23, 10, 4, 1
  };

  typedef char type[size];
  type *array = base;

  for (const unsigned short *gap = gaps; gap < gaps + sizeof gaps / sizeof *gaps; gap++) {
    for (size_t i = *gap; i < nmemb; i++)
      for (size_t j = i; j >= *gap && cmp(array[j - *gap], array[j]) > 0; j -= *gap)
        swap(array[j], array[j - *gap], size);
  }
}
