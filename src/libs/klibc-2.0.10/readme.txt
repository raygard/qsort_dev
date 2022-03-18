source of klibc qsort.c:
https://git.kernel.org/pub/scm/libs/klibc/klibc.git/snapshot/klibc-2.0.10.tar.gz
klibc-2.0.10\usr\klibc\qsort.c
Claim is
 * This is actually combsort.  It's an O(n log n) algorithm with
 * simplicity/small code size being its main virtue.


No way. Per Wikipedia, it's O(n^2). Best is n log n; average is n^2 / 2^p; p --
number of increments.

"Comb sort improves on bubble sort in the same way that Shellsort improves on
insertion sort." -- Wikipedia

Outperformed by Shell sort.
