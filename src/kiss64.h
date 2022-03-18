/* From George Marsaglia's KISS64 posting
 * See: https://groups.google.com/g/comp.lang.fortran/c/qFv18ql_WlU
 */

typedef unsigned long long _ULL;

typedef struct {
    _ULL x, y, z, c, t;
} KISS64_state;

void seed_KISS64(KISS64_state *s, _ULL x, _ULL y, _ULL z, _ULL c);

void seed_KISS64_default(KISS64_state *s);

_ULL KISS64(KISS64_state *s);
