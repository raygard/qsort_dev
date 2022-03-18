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
#define SWAP_INTS
// qsort from snippets 1991-10, modified 
//**********************************************************************
// qsort.c  --  ANSI Quicksort function                                 
//                                                                      
// Public domain by Raymond Gardner, Englewood CO  February 1991        
// 2021-01-26 rdg  revised but should be funtionally the same           
//                                                                      
// Usage:                                                               
//     qsort(base, nbr_elements, width_bytes, compare_function);        
//        void *base;                                                   
//        size_t nbr_elements, width_bytes;                             
//        int (*compare_function)(const void *, const void *);          
//                                                                      
// Sorts an array starting at base, of length nbr_elements, each        
// element of size width_bytes, ordered via compare_function, which is  
// called as (*compare_function)(ptr_to_element1, ptr_to_element2) and  
// returns < 0 if element1 < element2, 0 if element1 = element2, > 0    
// if element1 > element2. Most refinements are due to R. Sedgewick.    
// See "Implementing Quicksort Programs", Comm. ACM, Oct. 1978, and     
// Corrigendum, Comm.  ACM, June 1979, p. 368.                          
//**********************************************************************

#include <stddef.h>                         // for size_t definition    

/*
** Compile with -DSWAP_INTS if your machine can access an int at an
** arbitrary location with reasonable efficiency.  (Some machines
** cannot access an int at an odd address at all, so be careful.)
*/

//  swap n bytes between a and b 
static void swap_bytes(char *a, char *b, size_t n)
{
#if COUNTSWAPS
    tot_swaps += n;
#endif
    char tmp;
    do {
        tmp = *a; *a++ = *b; *b++ = tmp;
    } while (--n);
}

#ifdef   SWAP_INTS
//  swap n ints between a and b 
static void swap_ints(char *ap, char *bp, size_t n)
{
#if COUNTSWAPS
    tot_swaps += sizeof(int) * n;
#endif
    int *a = (int *)ap, *b = (int *)bp;
    int tmp;
    do {
        tmp = *a; *a++ = *b; *b++ = tmp;
    } while (--n);
}

#define  SWAP(a, b)  (swap_func((char *)(a), (char *)(b), width))
#else
#define  SWAP(a, b)  (swap_bytes((char *)(a), (char *)(b), size))
#endif

#define  COMP(a, b)  ((*comp)((void *)(a), (void *)(b)))

#define  T          15          // subfiles of T or fewer elements will 
                                // be sorted by a simple insertion sort 
                                // Note!  T must be at least 3          

void qsort(void *basep, size_t nelems, size_t size,
                int (*comp)(const void *, const void *))
{
    char *stack[2*8*sizeof(size_t)], **sp; // stack and stack pointer   
    char *i, *j, *limit;                // scan and limit pointers      
    char *base;                         // base pointer as char *       
#ifdef   SWAP_INTS
    size_t width;                       // width of array element       
    void (*swap_func)(char *, char *, size_t);  // swap function pointer

    width = size;                       // save size for swap routine   
    swap_func = swap_bytes;             // choose swap function         
    if (size % sizeof(int) == 0) {      // size is multiple of ints     
        width /= sizeof(int);           // set width in ints            
        swap_func = swap_ints;          // use int swap function        
    }
#endif
    base = (char *)basep;               // set up char * base pointer   
    sp = stack;                         // init stack pointer           
    limit = base + nelems * size;       // pointer past end of array    
    for (;;) {
        if ((size_t)(limit - base) / size > T) {    // if > T elements  
                                            //   swap base with middle  
            SWAP((((limit - base) / size) / 2) * size + base, base);
            i = base + size;                // i scans left to right    
            j = limit - size;               // j scans right to left    
            if (COMP(i, j) > 0)             // Sedgewick's              
                SWAP(i, j);                 //    three-element sort    
            if (COMP(base, j) > 0)          //        sets things up    
                SWAP(base, j);              //            so that       
            if (COMP(i, base) > 0)          //      *i <= *base <= *j   
                SWAP(i, base);              // *base is pivot element   
            for (;;) {
                while (COMP(i += size, base) < 0) // move i right until 
                    ;                       //          *i >= pivot     
                while (COMP(j -= size, base) > 0) // move j left until  
                    ;                       //          *j <= pivot     
                if (i > j)                  // if pointers crossed      
                    break;                  //     break loop           
                SWAP(i, j);         // else swap elements, keep scanning
            }
            SWAP(base, j);          // move pivot into correct place    
            if (j - base > limit - i) {     // if left subfile larger   
                sp[0] = base;               // stack left subfile base  
                sp[1] = j;                  //    and limit             
                base = i;                   // sort the right subfile   
            } else {                        // else right subfile larger
                sp[0] = i;                  // stack right subfile base 
                sp[1] = limit;              //    and limit             
                limit = j;                  // sort the left subfile    
            }
            sp += 2;                        // increment stack pointer  
        } else {        // else subfile is small, use insertion sort    
            for (i = base + size; i < limit; i += size)
                for (j = i; j != base && COMP(j - size, j) > 0; j -= size)
                    SWAP(j - size, j);
            if (sp != stack) {              // if any entries on stack  
                sp -= 2;                    // pop the base and limit   
                base = sp[0];
                limit = sp[1];
            } else                          // else stack empty, done   
                break;
        }
    }
}
