// test_sorts.c -- Test qsort() functions
// Ray Gardner, Centennial, CO, USA
// February 2022
// License: 0BSD
// See NOTES at end, following main().

#if defined(__unix__) || defined(linux)
    #define OS_Windows  0
#elif   defined(_WIN32) || defined(_WIN64)
    #define OS_Windows  1
#else
    #error Cannot determine platform or compiler support.
#endif

#if OS_Windows
#  define __USE_MINGW_ANSI_STDIO    1
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <math.h>

#include <unistd.h>

#include "kiss64.h"


static char *usage[] = {
"test_sorts -- Test qsort() functions",
"    Ray Gardner (github.com/raygard)",
"",
"Modeled on pseudocode in \"Engineering a Sort Function\",",
"    J. Bentley and M.D. McIlroy,",
"    Software Practice and Experience, November 1993",
"    and https://github.com/izabera/qsortbench by Isabella Bosia.",
"    Report format modeled on qsortbench.",
"",
"Usage: test_sorts [num] [-h -i -d -p -s -z -c]",
"    -h  (or --help)  display usage and quit",
"    num number of elements to sort (default 10000)",
"    -i  test C int values",
"    -d  test C double values",
"    -p  test pointers to strings",
"    -s  test array of structs",
"    -z  run izabera tests",
"    -c  check for excess compares",
"    -v  no tests on front or back half reversed",
"    -m  run small arrays test only (sanity test)",
"    -r num   number of reps for each test",
"",
"    Default is to test all datatypes on Bentley-McIlroy data patterns.",
"    One or more of -i, -d, -p, -s may be specified.",
"    -z runs tests taken from the qsortbench test of Isabella Bosia",
"        (github.com/izabera), plus a couple of my own.",
"    -c reports compares in excess of 1.2 n lg n (!!Compares) or",
"        1.5 n lg n (!!!Compares); lg is log base 2.",
"    -r num will repeat each test on all sorts 'num' times; default 1",
NULL,
};

// #define NO_REVERSE_HALF 1 to suppress reversing front and back half of distros.
// (Otherwise some qsorts (mostly BSD-derived) take O(N**2) time.)
#ifndef NO_REVERSE_HALF
#define NO_REVERSE_HALF  0
#endif
// #define SMALL_ARRAYS 1 to test on various small arrays (sanity test).
#ifndef SMALL_ARRAYS
#define SMALL_ARRAYS    0
#endif

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

typedef unsigned long long ULL;
typedef unsigned long UL;

typedef struct {char t; char *str;} tagged_string_list_t;

typedef struct {
    char pad[210];
    char s[20];
} stest;

typedef void qsort_t(void *base, size_t nmemb, size_t size,
        int (*compar)(const void *, const void *));

typedef struct {
    qsort_t *func;
    char *name;
    ULL tot_time;
    ULL tot_compares;
    ULL tot_swaps;
    UL time;
    UL compares;
    UL swaps;
    int time_rank;
    int compares_rank;
} qstbl;

qsort_t bentley_mcilroy;
qsort_t bentley_mcilroy2;
qsort_t bentley_mcilroy_pre;
qsort_t bentley_mcilroy_shell;
qsort_t bentley_mcilroy_pre_shell;
qsort_t bentmcil;
qsort_t illumos;
qsort_t rg91;
qsort_t rg91mod;
qsort_t quadsort;

qsort_t qs22a;
qsort_t qs22b;
qsort_t qs22bk;
qsort_t qs22c;
qsort_t qs22ck;
qsort_t qs22f;
qsort_t qs22h;
qsort_t qs22i;
qsort_t qs22j;
qsort_t qs22k;

qsort_t rg91ss;
qsort_t qs22ss;
qsort_t qs22ssb;
qsort_t qs22heap1;
qsort_t qs22heap2;
qsort_t qs22heap3;
qsort_t qs22ydpq;

qsort_t mccaughan;

qsort_t klibc;
qsort_t nlopt;
qsort_t newlib;
qsort_t musl;
qsort_t sortix;

qsort_t freebsd;
qsort_t freebsd_nopt;
qsort_t freebsd_pre;
qsort_t freebsd_shell;
qsort_t freebsd_pre_shell;
qsort_t openbsd;
qsort_t netbsd;
qsort_t bionic;
qsort_t bionic_nopt;
qsort_t bionic_pre;
qsort_t bionic_shell;
qsort_t bionic_pre_shell;
qsort_t reactos;
qsort_t reactos_nopt;
qsort_t reactos_pre;
qsort_t reactos_shell;
qsort_t reactos_pre_shell;

qsort_t dietlibc;
qsort_t uboot;

qsort_t uClibc;
qsort_t uclibcng;
qsort_t picolibc;

qsort_t netbsd_old;
qsort_t emacs;

qsort_t netbsd_1_4;
qsort_t plan9;
qsort_t plan9x;
qsort_t izabera;
qsort_t izabera_mini;

#define tblentry(f) {f, #f, 0, 0, 0, 0, 0, 0, 0, 0},

static qstbl qsorts[] = {
#if ! OS_Windows
#if 1
    // Windows qsort can go quadratic
    {qsort, "system", 0, 0, 0, 0, 0, 0, 0, 0},
#endif
#endif
#if 0
    // Both emacs and netbsd_old are very slow. They came in last or
    // near-last in almost every test.
    // They apparently go quadratic in many cases.
    tblentry(emacs)         // 1987 -- came in 4.4BSD-Lite.tar.gz
    tblentry(netbsd_old)    // from 19930321, slight mod to make it compile
#endif
#if 0
    // All these have the swap_cnt problem; can go quadratic.
    tblentry(netbsd_1_4)    // first based on Bentley-McIlroy; has swap_cnt
    tblentry(newlib)            // can go quadratic
    tblentry(nlopt)             // can go quadratic
    tblentry(picolibc)          // can go quadratic
    tblentry(freebsd)           // can go quadratic
    tblentry(bionic)            // can go quadratic
    tblentry(reactos)           // can go quadratic
#endif

#if 1
    tblentry(bionic_nopt)       // no swap_cnt "optimization"
    tblentry(bionic_pre)        // precheck for sorted data
    tblentry(bionic_shell)  // Shell sort instead of insertion on swap_cnt==0
    tblentry(bionic_pre_shell)  // sorted precheck plus Shell if swap_cnt==0
#endif
#if 1
    tblentry(reactos_nopt)      // no swap_cnt "optimization"
    tblentry(reactos_pre)       // precheck for sorted data
    tblentry(reactos_shell) // Shell sort instead of insertion on swap_cnt==0
    tblentry(reactos_pre_shell) // sorted precheck plus Shell if swap_cnt==0
#endif
#if 1
    tblentry(freebsd_nopt)
    tblentry(freebsd_pre)
    tblentry(freebsd_shell)
    tblentry(freebsd_pre_shell)
#endif
#if 1
    tblentry(bentley_mcilroy_pre)
    tblentry(bentley_mcilroy_shell)
    tblentry(bentley_mcilroy_pre_shell)
#endif
#if 1
    tblentry(bentmcil)
    tblentry(bentley_mcilroy)
    tblentry(bentley_mcilroy2)
    tblentry(illumos)
    tblentry(qs22b)
    tblentry(qs22bk)
    tblentry(qs22c)
    tblentry(qs22ck)
    tblentry(qs22f)
    tblentry(qs22h)
    tblentry(qs22i)
    tblentry(qs22j)
    tblentry(qs22k)
#endif
#if 0
    tblentry(quadsort)
#endif
#if 1
    tblentry(openbsd)
    tblentry(netbsd)    // netbsd qsort 1.23
    tblentry(dietlibc)
#endif
#if 1
    tblentry(plan9)
    tblentry(plan9x)
    tblentry(izabera)
    tblentry(mccaughan)
#endif
#if 0
    tblentry(rg91)
#endif
#if 1
    tblentry(rg91mod)
#endif
#if 1
    tblentry(qs22a)
#endif
#if 1
    tblentry(qs22ydpq)  // Yaroslavskiy's dual-pivot quicksort
#endif
#if 1
    tblentry(izabera_mini)  // Shell sort
#endif
#if 1
    tblentry(uboot)         // Shell sort
    //tblentry(uClibc)        // Shell sort
    tblentry(uclibcng)      // Shell sort
#endif
#if 1
    tblentry(qs22ssb)       // Newer Shell sort variant by rdg
    tblentry(qs22ss)        // Newer Shell sort variant by rdg
#endif
#if 0
    tblentry(rg91ss)        // Old Shell sort variant by rdg
#endif
#if 1
    tblentry(klibc)         // combsort (improved? bubble sort) very slow!
#endif
#if 1
    tblentry(qs22heap1)     // heapsort
    tblentry(qs22heap2)     // heapsort
    tblentry(qs22heap3)     // heapsort
#endif
#if 1
    tblentry(sortix)        // heapsort -- very slow
#endif
#if 1
    tblentry(musl)          // smoothsort (Dijkstra var. of heapsort) very slow!
#endif
};

ULL tot_swaps;  // This is updated by qsorts modified to count swapped bytes.

static ULL tot_time;
static ULL tot_compares;

// datatypes[] must correspond with dtypes[]
#define maxdatatypes 10     // must be > len(datatypes)
static char datatypes[] = "idps";   // int, double, ptr to string, struct

static tagged_string_list_t dtypes[] = {
    {'i', "int"},
    {'d', "double"},
    {'p', "stringptr"},
    {'s', "struct"},
    {0, NULL}
    };


/////////////////////////////// Timer stuff ////////////////////////
typedef long long ticks_t;

static ticks_t ticks_per_second;

#if OS_Windows

static ticks_t get_timer_resolution()
{
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq)
            || !freq.QuadPart) {
        printf("Error: cannot get Windows timer resolution.\n");
        exit(123);
    }
    return freq.QuadPart;
}

inline static ticks_t get_ticks()
{
    LARGE_INTEGER ticks;
    if (!QueryPerformanceCounter(&ticks)) {
        printf("Error: cannot get Windows timer count.\n");
        exit(123);
    }
    return ticks.QuadPart;
}
#else       // NOT WINDOWS, must be Linux / Unix
static ticks_t get_timer_resolution()
{
    return 10000000;
}

inline static ticks_t get_ticks()
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return (long long)t.tv_sec * 10000000 + (t.tv_nsec + 50) / 100;
}

#if 0
static void Sleep(long millisecs)
{
    struct timespec t = {millisecs / 1000, (millisecs % 1000) * 1000};
    struct timespec tr;
    nanosleep(&t, &tr);
}
#endif
#endif
//////////////////////// END Timer stuff ////////////////////////

static void *mcalloc(size_t num, size_t size)
{
    void *p = calloc(num, size);
    assert(p);
    return p;
}

static KISS64_state random_state;

static void seed_random31() { seed_KISS64_default(&random_state); }

static unsigned int random31()
{
    return (unsigned)(KISS64(&random_state) & 0x7FFFFFFF);
}

static int compare_int(const void *a, const void *b)
{
    tot_compares++;
    if (*(const int *)a < *(const int *)b)
        return -1;
    else if (*(const int *)a > *(const int *)b)
        return 1;
    return 0;
}

#if 0
static int compare_long(const void *a, const void *b)
{
    tot_compares++;
    if (*(const long *)a < *(const long *)b)
        return -1;
    else if (*(const long *)a > *(const long *)b)
        return 1;
    return 0;
}
#endif

static int compare_double(const void *a, const void *b)
{
    tot_compares++;
    if (*(const double *)a < *(const double *)b)
        return -1;
    else if (*(const double *)a > *(const double *)b)
        return 1;
    return 0;
}

#if 0
static int compare_ptr_to_str(const void *a, const void *b)
{
    tot_compares++;
    return strcmp(*(char **)a, *(char **)b);
}
#endif

static void xstrrev(char *s)
{
    if (s && *s) {
        char *e = s + strlen(s) - 1;
        while (s < e) {
            char t = *s;
            *s++ = *e;
            *e-- = t;
        }
    }
}

static int compare_ptr_to_str(const void *a, const void *b)
{
    tot_compares++;
    char *aa = *(char **)a, *bb = *(char **)b;
    xstrrev(aa);
    xstrrev(aa);
    return strcmp(aa, bb);
}

static int compare_struct(const void *a, const void *b)
{
    tot_compares++;
    return strcmp(((stest *)a)->s, ((stest *)b)->s);
}

static UL sum(int *v, size_t n)
{
    UL tot = 0;
    for (size_t i = 0; i < n; ++i) {
        tot += (UL)v[i];
        if ((UL)v[i] == 0) {
            tot += 42;  // arbitrary non-0
        }
    }
    return tot;
}

static int is_sorted(int *v, size_t n) {
    for (size_t i = 1; i < n; ++i) {
        if (v[i-1] > v[i]) {
            return 0;
        }
    }
    return 1;
}

#if 0
static void showtime(ticks_t nticks)
{
    if ( nticks < 0 ) {
        printf("***Timer error: %lld cannot be negative?", nticks);
        return;
    }
    nticks = nticks * 1000000 / ticks_per_second;   // convert to microsecs
    if (nticks < 10000)
        //printf("%8lldμs", nticks);
        printf("%8lld us", nticks);
    else if (nticks < 1000000)
        //printf("%8lldms", nticks / 1000);
        printf("%4lld.%03lld ms",
            nticks/1000, (nticks%1000));
    else
        printf("%4lld.%03lld s ",
            nticks/1000000, (nticks%1000000)/1000);
    //printf(" (%lld)", nticks);
}
#endif
#if 1
static void showtime(ticks_t nticks)
{
    if ( nticks < 0 ) {
        printf("***Timer error: %lld cannot be negative?", nticks);
        return;
    }
    nticks = nticks * 1000000 / ticks_per_second;   // convert to microsecs
    if (nticks < 1000000)
        //printf("%8lldms", nticks / 1000);
        printf("%4lld.%03lld ms",
            nticks/1000, (nticks%1000));
    else
        printf("%4lld.%03lld s ",
            nticks/1000000, (nticks%1000000)/1000);
    //printf(" (%lld)", nticks);
}
#endif

static int *copy(int *x, size_t n)
{
    //printf("copy %d values\n", n);
    int *v = mcalloc(n, sizeof(int));
    memcpy(v, x, n * sizeof(int));
    return v;
}

static int *reverse(int *x, size_t n, size_t lo, size_t lim)
{
    //printf("copy/reverse %lu values; lo: %lu lim: %lu\n", (UL)n, (UL)lo, (UL)lim);
    int *v = copy(x, n);
    //printf("copied!\n");
    if (lim)
    for (size_t left = lo, right = lim-1; left < right; left++, right--) {
        int temp = v[left];
        v[left] = v[right];
        v[right] = temp;
    }
    //printf("reversed!\n");
    return v;
}

static int *sort(int *x, size_t n)
{
    //printf("copy/sort %d values\n", n);
    int *v = copy(x, n);
    long long save_tot_compares = tot_compares;
    long long save_tot_swaps = tot_swaps;
    bentley_mcilroy(v, n, sizeof(int), compare_int);
    tot_compares = save_tot_compares;
    tot_swaps = save_tot_swaps;
    return v;
}

static int *dither(int *x, size_t n)
{
    //printf("copy/dither %d values\n", n);
    int *v = copy(x, n);
    for (size_t i = 0; i < n; i++)
        v[i] += i % 5;
    return v;
}

static void sort_data(qstbl *q, int *data, size_t n, int datatype,
        int distribution, int modification, int modulus,
        int check_excess_compares)
{
#if DEBUG
printf("Running sort %s on %lld elements (type %c) distro %c modif %c mod %lld\n",
        q->name, (ULL)n, datatype, distribution, modification, (ULL)modulus);
#endif
    UL cksum = sum(data, n);
    double *ddata = NULL;
    char **pdata = NULL;
    stest *sdata = NULL;
    switch (datatype) {
        case 'i':
            break;
        case 'd':
            ddata = mcalloc(n, sizeof *ddata);
            for (size_t kk = 0; kk < n; kk++)
                ddata[kk] = data[kk];
            break;
        case 'p':
            pdata = mcalloc(n, sizeof *pdata);
            for (size_t kk = 0; kk < n; kk++) {
                pdata[kk] = mcalloc(20, 1);
                sprintf(pdata[kk], "%12.12d", data[kk]);
            }
            break;
        case 's':
            sdata = mcalloc(n, sizeof(*sdata));
            for (size_t kk = 0; kk < n; kk++) {
                sprintf(sdata[kk].s, "%12.12d", data[kk]);
            }
            break;
    }
    ULL test_compares = tot_compares;
    ticks_t nticks = get_ticks();
    switch (datatype) {
        case 'i':
            q->func(data, n, sizeof *data, compare_int);
            nticks = get_ticks() - nticks;
            break;
        case 'd':
            q->func(ddata, n, sizeof *ddata, compare_double);
            nticks = get_ticks() - nticks;
            for (size_t kk = 0; kk < n; kk++) data[kk] = ddata[kk];
            free(ddata);
            break;
        case 'p':
            q->func(pdata, n, sizeof *pdata, compare_ptr_to_str);
            nticks = get_ticks() - nticks;
            for (size_t kk = 0; kk < n; kk++) {
                data[kk] = strtoul(pdata[kk], NULL, 10);
                free(pdata[kk]);
            }
            free(pdata);
            break;
        case 's':
            q->func(sdata, n, sizeof *sdata, compare_struct);
            nticks = get_ticks() - nticks;
            for (size_t kk = 0; kk < n; kk++) {
                data[kk] = strtoul(sdata[kk].s, NULL, 10);
            }
            free(sdata);
            break;
    }
    tot_time += nticks;
    test_compares = tot_compares - test_compares;
    assert(is_sorted(data, n));
    assert(sum(data, n) == cksum);
    if (check_excess_compares) {
        if (test_compares > 1.5 * n * log(n) / log(2))
            printf("!!!Compares: running sort %s on %lld elements (type %c)"
                    " distro %c modif %c mod %lld %lld %5.3g\n",
                    q->name, (ULL)n, datatype, distribution, modification,
                    (ULL)modulus, test_compares,
                    (double)test_compares / ((double)n * log(n) / log(2)));
        else if (test_compares > 1.2 * n * log(n) / log(2))
            printf("!!Compares: running sort %s on %lld elements (type %c)"
                    " distro %c modif %c mod %lld %lld %5.3g\n",
                    q->name, (ULL)n, datatype, distribution, modification,
                    (ULL)modulus, test_compares,
                    (double)test_compares / ((double)n * log(n) / log(2)));
    }
}

static tagged_string_list_t iztests[] = {
    {'s', "sorted"},
    {'r', "random"},
    {'w', "few_unique"},
    //{'s', "sorted"},
    {'v', "reverse"},
    {'p', "one_percent"},
    {'1', "1% the nsz way"},
    {'5', "5% the nsz way"},    // Added by Ray G
    {'f', "50% sorted"},
    {'t', "triangle"},
    {'q', "all_equal"},
    {'d', "dutch_flag"},
    {'e', "even_odd"},
    {'o', "reverse_even_odd"},
    {'g', "pipe_organ"},
    {'x', "push_front"},
    {'y', "push_middle"},
#if ! NO_REVERSE_HALF
    {'z', "reverse_front"},     // Added by Ray G
    {'b', "reverse_back"},      // Added by Ray G
#endif
    {0, NULL}
    };

static void run_izabera_tests(qstbl *q, int *x, size_t n, int datatype,
        int distribution, int check_excess_compares)
{
    //printf("izabera run sort %s on datatype %c elements %lu x %p\n", q->name,
    //datatype, (UL)n, x);
    seed_random31();
    switch (distribution) {
        case 'r':
            for (size_t i = 0; i < n; i++)
                x[i] = random31();
            break;
        case 'w':
            for (size_t i = 0; i < n; i++)
                x[i] = random31() % (n / 100 + 1);
            break;
        case 's':
            for (size_t i = 0; i < n; i++)
                x[i] = i;
            break;
        case 'v':
            for (size_t i = 0; i < n; i++)
                x[i] = n - i;
            break;
        case 'p':
            for (size_t i = 0; i < n; i++)
                x[i] = i;
            for (size_t i = 0; i < n / 100; i++)
                x[random31() % n] = random31();
            break;
        // "1% the nsz way"
        // Per izabera, "nsz refers to Szabolcs Nagy, a long time musl contributor".
        // (github.com/izabera is Isabella Bosia.)
        case '1':
            for (size_t i = 0; i < n; i++)
                x[i] = i;
            for (size_t i = 0; i < n / 200; i++) {
                size_t a = random31() % n;
                size_t b = random31() % n;
                int tmp = x[a]; x[a] = x[b]; x[b] = tmp;
            }
            break;
        // "5% the nsz way" Added by Ray G
        case '5':
            for (size_t i = 0; i < n; i++)
                x[i] = i;
            for (size_t i = 0; i < n / 40; i++) {
                size_t a = random31() % n;
                size_t b = random31() % n;
                int tmp = x[a]; x[a] = x[b]; x[b] = tmp;
            }
            break;
        case 'f':
            for (size_t i = 0; i < n / 2; i++) {
                x[2 * i] = i;
                x[2 * i + 1] = random31();
            }
            break;
        case 't':
            for (size_t i = 0; i < n; i++)
                x[i] = i % ((n % 100) + 1);
            break;
        case 'q':
            for (size_t i = 0; i < n; i++)
                x[i] = 123456789;
            break;
        case 'd':
            for (size_t i = 0; i < n; i++)
                x[i] = i % 3;
            break;
        case 'e':
            for (size_t i = 0; i < n / 2; i++)
                x[i] = 2 * i;
            for (size_t i = n / 2; i < n; i++)
                x[i] = 2 * (i - n / 2) + 1;
            break;
        case 'o':
            for (size_t i = 0; i < n / 2; i++)
                x[i] = n - 2 * i;
            for (size_t i = n / 2; i < n; i++)
                x[i] = n - 2 * (i - n / 2) + 1;
            break;
        case 'g':
            for (size_t i = 0; i < n / 2; i++)
                x[i] = i;
            for (size_t i = n / 2; i < n; i++)
                x[i] = n - 1 - i;
            break;
        case 'x':
            for (size_t i = 0; i < n - 1; i++)
                x[i] = i + 1;
            x[n - 1] = 0;
            break;
        case 'y':
            for (size_t i = 0; i < n - 1; i++)
                x[i] = i + 1;
            x[n - 1] = n / 2;
            break;
        case 'z':
            for (size_t i = 0; i < n / 2; i++)
                x[i] = n / 2 - 1 - i;
            for (size_t i = n / 2; i < n; i++)
                x[i] = i;
            break;
        case 'b':
            for (size_t i = 0; i < n / 2; i++)
                x[i] = i;
            for (size_t i = n / 2; i < n; i++)
                x[i] = (3 * n) / 2 - 1 - i;
            break;
    }
    //printf("iz: %c\n", distribution);
    sort_data(q, x, n, datatype, distribution, 'z', 0, check_excess_compares);
}

static tagged_string_list_t b_m_tests[] = {
    {'w', "sawtooth"}, {'r', "random"}, {'s', "stagger"},
    {'p', "plateau"}, {'f', "shuffle"}, {0, NULL}
    };

// copy, reverse, rev-front-half, rev-back-half, sorted, dither
static tagged_string_list_t modifs[] = {
    {'c', "copy"},
    {'r', "reverse"},
    {'f', "reverse_front"},
    {'b', "reverse_back"},
    {'s', "sorted"},
    {'d', "dither"},
    {0, NULL}
    };

static void run_a_sort(qstbl *q, int *x, size_t n, int datatype,
        int use_izabera_tests, int distribution, int modification,
        int check_excess_compares)
{
    if (use_izabera_tests) {
        run_izabera_tests(q, x, n, datatype, distribution,
                check_excess_compares);
        return;
    }

#if 0
    printf("run sort %s on datatype %c distro %c modif %c elements %lu x %p\n",
            q->name, datatype, distribution, modification, (UL)n, x);
#endif
    // Change m = 1; to m = n; to run only on final modulus.
    for (size_t m = 1; m < n * 2; m *= 2) {
        //printf("modulus %lu n %lu\n", (UL)m, (UL)n);
        seed_random31();
        for (size_t i = 0, j = 0, k = 1; i < n; i++) {
            switch (distribution) {
                case 'w': x[i] = i % m;                     // sawtooth
                          break;
                case 'r': x[i] = random31() % m;            // rand
                          break;
                case 't': x[i] = (i * m + i) % n;           // stagger
                          break;
                case 'p': x[i] = min(i, m);                 // plateau
                          break;
                // shuffle:
                case 'f': x[i] = random31() % m ?  (j += 2) : (k += 2);
                          break;
            }
        }

        // 'c', 'r', 'f', 'b', 's', 'd':
        // copy, reverse, rev-front-half, rev-back-half, sorted, dither
        int *data = NULL;
        switch (modification) {
            case 'c':
                data = copy(x, n);
                break;
            case 'r':
                data = reverse(x, n, 0, n);       // on a reversed copy
                break;
            case 'f':
                data = reverse(x, n, 0, n / 2);
                break;
            case 'b':
                data = reverse(x, n, n / 2, n);
                break;
            case 's':
                data = sort(x, n);  // on an ordered copy
                break;
            case 'd':
                data = dither(x, n);
                break;
        }
        sort_data(q, data, n, datatype, distribution, modification, m,
                check_excess_compares);
        free(data);
    }
}

// Small sorts. Includes sizes in Bentley-McIlroy paper.
static int small_arrays[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 100, 1023, 1024, 1025, -1};

static void test_sort(qstbl *q, int datatype, int use_izabera_tests,
        int distribution, int modification, size_t num,
        int check_excess_compares, int opt_small_arrays)
{
#if 0
    printf("run test on %s datatype %c distro %c modif %c num %lu\n",
            q->name, datatype, distribution, modification, (UL)num);
#endif
    tot_swaps = tot_compares = tot_time = 0;
    if (opt_small_arrays) {
        int sm = sizeof small_arrays / sizeof small_arrays[0];
        int xsize = small_arrays[sm - 2];
        int *x = mcalloc(xsize + 1, sizeof(int));
        int n;
        for (int nn = 0; (n = small_arrays[nn]) >= 0; nn++) {
            x[n] = 0xdefaced;
            run_a_sort(q, x, n, datatype, use_izabera_tests, distribution,
                    modification, check_excess_compares);
            assert(x[n] == 0xdefaced);
        }
        free(x);
    } else {
        int *x = mcalloc(num + 1, sizeof(int));
        x[num] = 0xdefaced;
        run_a_sort(q, x, num, datatype, use_izabera_tests, distribution,
                modification, check_excess_compares);
        assert(x[num] == 0xdefaced);
        free(x);
    }
    // Adjust tot_swaps (which is now number of bytes swapped) to actual number
    // of swaps.
    size_t datasize = datatype == 'i' ? sizeof(int)
        : datatype == 'd' ? sizeof(double)
        : datatype == 'p' ? sizeof(char *)
        : datatype == 's' ? sizeof(stest)
        : 0;
    assert(tot_swaps % datasize == 0);
    tot_swaps /= datasize;
    q->tot_time += tot_time;
    q->time += tot_time;
    q->tot_compares += tot_compares;
    q->compares += tot_compares;
    q->tot_swaps += tot_swaps;
    q->swaps += tot_swaps;
}

static int compare_compares(const void *a, const void *b)
{
    qstbl *aa = *(qstbl **)a, *bb = *(qstbl **)b;
    //printf("%s %s\n", aa->name, bb->name);
    if (aa->compares < bb->compares)
        return -1;
    if (aa->compares > bb->compares)
        return 1;
    if (aa < bb)
        return -1;
    return 1;
}

static int compare_times(const void *a, const void *b)
{
    qstbl *aa = *(qstbl **)a, *bb = *(qstbl **)b;
    //printf("%s %s\n", aa->name, bb->name);
    if (aa->time < bb->time)
        return -1;
    if (aa->time > bb->time)
        return 1;
    return strcmp(aa->name, bb->name);
#if 0
    if (aa < bb)
        return -1;
    return 1;
#endif
}

static int compare_time_rank(const void *a, const void *b)
{
    qstbl *aa = *(qstbl **)a, *bb = *(qstbl **)b;
    if (aa->time_rank < bb->time_rank)
        return -1;
    if (aa->time_rank > bb->time_rank)
        return 1;
    if (aa->time < bb->time)
        return -1;
    if (aa->time > bb->time)
        return 1;
    return strcmp(aa->name, bb->name);
}

static int compare_compares_rank(const void *a, const void *b)
{
    qstbl *aa = *(qstbl **)a, *bb = *(qstbl **)b;
    if (aa->compares_rank < bb->compares_rank)
        return -1;
    if (aa->compares_rank > bb->compares_rank)
        return 1;
    if (aa->compares < bb->compares)
        return -1;
    if (aa->compares > bb->compares)
        return 1;
    return strcmp(aa->name, bb->name);
}

static void run_tests(char *test_datatypes, size_t num, int use_izabera_tests,
        int check_excess_compares, int opt_no_half_reversed,
        int opt_small_arrays, int nreps)
{
    tagged_string_list_t *tests_set = use_izabera_tests ? iztests : b_m_tests;
    int num_sorts = sizeof qsorts / sizeof qsorts[0];
    qstbl *qq[sizeof qsorts / sizeof qsorts[0]];
    for (int i = 0; i < num_sorts; i++) {
        qq[i] = &qsorts[i];
        qsorts[i].tot_time = 0;
        qsorts[i].tot_compares = 0;
        qsorts[i].tot_swaps = 0;
        qsorts[i].time = 0;
        qsorts[i].compares = 0;
        qsorts[i].swaps = 0;
        qsorts[i].compares_rank = 0;
        qsorts[i].time_rank = 0;
    }
    printf("%lu elements %d sorts\n", (UL)num, num_sorts);
    for (int dt = 0; dtypes[dt].t; dt++) {
            if (! strchr(test_datatypes, dtypes[dt].t))
                continue;
        for (int dp = 0; tests_set[dp].t; dp++) {
            if (use_izabera_tests && opt_no_half_reversed &&
                    (tests_set[dp].t == 'z' || tests_set[dp].t == 'b'))
                continue;
            for (int mt = 0; modifs[mt].t; mt++) {
                if (opt_no_half_reversed &&
                        (modifs[mt].t == 'f' || modifs[mt].t == 'b'))
                    continue;
                //printf("!!! type: %c  distro: %c   modifier: %c\n",
                //    dtypes[dt].t, tests_set[dp], modifs[mt].t);
                printf("Testing %lu %s elements %s %s:\n", (UL)num,
                        dtypes[dt].str, tests_set[dp].str,
                        use_izabera_tests ? "(izaberra)" : modifs[mt].str);
                for (int i = 0; i < num_sorts; i++) {
                    qsorts[i].time = 0;
                    qsorts[i].compares = 0;
                    qsorts[i].swaps = 0;
                }
                for (int repcnt = 0; repcnt < nreps; repcnt++) {
                    for (int qn = 0; qn < num_sorts; qn++) {
                        test_sort(&qsorts[qn], dtypes[dt].t, use_izabera_tests,
                            tests_set[dp].t, modifs[mt].t, num,
                            check_excess_compares, opt_small_arrays);
                    }
                }
                // Report on one data distribution.
                qsort(qq, num_sorts, sizeof(qstbl *), compare_times);
                for (int i = 0; i < num_sorts; i++) {
#if COUNTSWAPS
                    printf("%12lu ", qq[i]->swaps);
#endif
                    printf("%12lu ", qq[i]->compares);
                    showtime(qq[i]->time);
                    printf(" %6.3f %s\n", (double)qq[i]->time / qq[0]->time,
                            qq[i]->name);
                }

                printf("Comps:");
                qsort(qq, num_sorts, sizeof(qstbl *), compare_compares);
                // rank update handles ties.
                for (int rank = 1, i = 0; i < num_sorts; i++) {
                    if (i && qq[i]->compares != qq[i-1]->compares)
                        rank = i+1;
                    printf(" %d. %s", rank, qq[i]->name);
                    qq[i]->compares_rank += rank;
                }
                printf("\n");
                printf("Times:");
                qsort(qq, num_sorts, sizeof(qstbl *), compare_times);
                for (int rank = 1, i = 0; i < num_sorts; i++) {
                    if (i && qq[i]->time != qq[i-1]->time)
                        rank = i+1;
                    printf(" %d. %s", rank, qq[i]->name);
                    qq[i]->time_rank += rank;
                }
                printf("\n");
                if (use_izabera_tests)
                    break;
            }
        }
    }

    // Final summary report
    qsort(qq, num_sorts, sizeof(qstbl *), compare_compares_rank);
    printf("Best by rankings on compares:\n");
#if COUNTSWAPS
    printf("   Tot.rank     Swaps     Compares      Time   Ratio Implementation\n");
#else
    printf("   Tot.rank  Compares      Time   Ratio Implementation\n");
#endif
    for (int i = 0; i < num_sorts; i++) {
        printf("%2d. %4d", i + 1, qq[i]->compares_rank);
#if COUNTSWAPS
        printf(" %12llu", qq[i]->tot_swaps);
#endif
        printf(" %12llu", qq[i]->tot_compares);
        showtime(qq[i]->tot_time);
        printf(" %6.3f %s\n",
                (double)qq[i]->tot_compares / qq[0]->tot_compares,
                qq[i]->name);
    }

    qsort(qq, num_sorts, sizeof(qstbl *), compare_time_rank);
    printf("Best by rankings on time:\n");
#if COUNTSWAPS
    printf("   Tot.rank     Swaps     Compares      Time   Ratio Implementation\n");
#else
    printf("   Tot.rank  Compares      Time   Ratio Implementation\n");
#endif
    for (int i = 0; i < num_sorts; i++) {
        printf("%2d. %4d", i + 1, qq[i]->time_rank);
#if COUNTSWAPS
        printf(" %12llu", qq[i]->tot_swaps);
#endif
        printf(" %12llu", qq[i]->tot_compares);
        showtime(qq[i]->tot_time);
        printf(" %6.3f %s\n",
                (double)qq[i]->tot_time / qq[0]->tot_time,
                qq[i]->name);
    }
}

static void show_usage()
{
    for ( char **p = usage; *p; p++ )
        printf("%s\n", *p);
}

int main(int argc, char **argv)
{
    int check_excess_compares = 0;
    size_t num = 10000;
    int use_izabera_tests = 0;
    int opt_no_half_reversed = 0, opt_small_arrays = 0;
    char test_datatypes[maxdatatypes+1] = "";
    memset(test_datatypes, 0, maxdatatypes+1);
    ticks_per_second = get_timer_resolution();
    //printf("%lld ticks per second\n", ticks_per_second);
    setvbuf(stdout, NULL, _IOLBF, 0);
    //printf("%s\n", datatypes);
    int nreps = 1;
    int c;
    while ((c = getopt(argc, argv, "+hidpszcvmr:n:")) != -1) {
        switch (c) {
            case 'h':
                show_usage();
                return 0;
            case 'i':
            case 'd':
            case 'p':
            case 's':
                if (strlen(test_datatypes) >= maxdatatypes) {
                    printf("Too many datatypes requested.\n");
                    return 1;
                }
                test_datatypes[strlen(test_datatypes)+1] = 0;
                test_datatypes[strlen(test_datatypes)] = c;
                break;
            case 'z':
                use_izabera_tests = 1;
                break;
            case 'c':
                check_excess_compares = 1;
                break;
            case 'v':
                opt_no_half_reversed = 1;
                break;
            case 'm':
                opt_small_arrays = 1;
                break;
            case 'r':
                nreps = strtoul(optarg, NULL, 10);
                break;
            case 'n':
                num = strtoul(optarg, NULL, 10);
                break;
            default:
                abort();
        }
    }
    if (optind < argc) {
        num = strtoul(argv[optind], NULL, 10);
    }
    if (opt_small_arrays)
        num = 0;
    if (! test_datatypes[0])
        strcpy(test_datatypes, datatypes);
    printf("Testing types %s with %lu elements %d times\n", test_datatypes, (UL)num, nreps);
    run_tests(test_datatypes, num, use_izabera_tests, check_excess_compares,
            opt_no_half_reversed, opt_small_arrays, nreps);
    return 0;
}

//////// NOTES ////////
//
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
//////////////
//
//  The report follows the approach of that in github.com/izabera/qsortbench.
//  Each run of tests against several sorts measures time and number of
//  comparisons and then ranks the sorts on each measure. At the end of all the
//  tests, a summary is given showing total times and total compares, but
//  ranking them in the order of the sums of the ranks from the individual
//  tests.
//
//  The number of swaps is measured for selected sorts that have been
//  instrumented to provide that measure, and the report will show the swap
//  count but does not rank the sorts on swap count. (Feel free to add that
//  feature.) Note that the qsort() functions actually accumulate total bytes
//  swapped into global tot_swaps, which is then converted to actual swap count
//  in test_sort().
//
//  I originally based the program on the "certification" pseudocode (shown
//  below) from the Bentley & McIlroy paper. I later found the qsortbench
//  program and hacked most of the same tests into this program. The hack is
//  not as seamless as I'd like. The Bentley-McIlroy certification program was
//  really intended more as a torture test to reveal performance problems with
//  qsort implementations, rather than as a performance benchmark. A benchmark
//  should IMO ideally run presumably "typical" workloads as well as outlier
//  cases, but I don't know what qsort is "typically" run on, if there is such
//  a thing. So I think adding the izabera tests is also useful.
//
//  You can adjust the cost of swaps by changing the padding in struct stest,
//  and cost of compares by making more or less work in compare_ptr_to_str().
//
// Pseudocode for qsort certification program
// From Bentley & McIlroy: Engineering a Sort Function
//
//  for n in { 100, 1023, 1024, 1025 }
//      for (m = 1; m < 2*n; m *= 2)
//          for dist in { sawtooth, rand, stagger, plateau, shuffle }
//              for (i = j = 0, k = 1; i < n; i++)
//                  switch (dist)
//                      case sawtooth: x[i] = i % m
//                      case rand: x[i] = rand() % m
//                      case stagger: x[i] = (i*m + i) % n
//                      case plateau: x[i] = min(i, m)
//                      case shuffle: x[i] = rand()%m? (j+=2): (k+=2)
//              for type in { int, double }
//                  test(n, copy(x, n));            /* work on a copy of x */
//                  test(n, reverse(x, 0, n));      /* on a reversed copy */
//                  test(n, reverse(x, 0, n / 2));  /* front half reversed */
//                  test(n, reverse(x, n / 2, n));  /* back half reversed */
//                  test(n, sort(x, n));            /* an ordered copy */
//                  test(n, dither(x, n));          /* add i%5 to x[i] */
