//#define KISS64_TEST
/* From George Marsaglia's KISS64 posting
 * See: https://groups.google.com/g/comp.lang.fortran/c/qFv18ql_WlU
 */

#include "kiss64.h"
/*
 * Setting seeds: Use of KISS or KISS() as a general 64-bit RNG requires
 * specifying 3*64+58=250 bits for seeds, 64 bits each for x,y,z and 58 for c,
 * resulting in a composite sequence with period around 2^250.
 */

void seed_KISS64(KISS64_state *s, _ULL x, _ULL y, _ULL z, _ULL c)
{
    s->x = x; s->y = y; s->z = z; s->c = c;
}

void seed_KISS64_default(KISS64_state *s)
{
    s->x=1234567890987654321ULL;
    s->c=123456123456123456ULL;
    s->y=362436362436362436ULL;
    s->z=1066149217761810ULL;
}

_ULL KISS64(KISS64_state *s)
{
#define MWC (s->t=(s->x<<58)+s->c, s->c=(s->x>>6), s->x+=s->t, s->c+=(s->x<s->t), s->x)
#define XSH (s->y^=(s->y<<13), s->y^=(s->y>>17), s->y^=(s->y<<43))
#define CNG (s->z=6906969069LL*s->z+1234567)
#define KISS (MWC+XSH+CNG)
    return KISS;
}
#undef MWC
#undef XSH
#undef CNG

#ifdef KISS64_TEST
#include <stdio.h>

int main(void)
{
    int i;
    KISS64_state s;
    seed_KISS64_default(&s);
    for(i = 0; i < 100000000; i++) s.t = KISS64(&s);
    (s.t == 1666297717051644203ULL) ?
        printf("%s\n", "100 million uses of KISS OK"):
        printf("%s\n", "Fail");
    return 0;
}
#endif
#if 0
static unsigned long long
x=1234567890987654321ULL, c=123456123456123456ULL,
y=362436362436362436ULL, z=1066149217761810ULL, t;

#define MWC (t=(x<<58)+c, c=(x>>6), x+=t, c+=(x<t), x)
#define XSH ( y^=(y<<13), y^=(y>>17), y^=(y<<43) )
#define CNG ( z=6906969069LL*z+1234567 )
#define KISS (MWC+XSH+CNG)

int main(void)
{
    int i;
    for(i=0; i<100000000; i++) t=KISS;
    (t==1666297717051644203ULL) ?
        printf("100 million uses of KISS OK"):
        printf("Fail");
}
#endif
