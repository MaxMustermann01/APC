/***********************************************************************
*                                                                      *
* (C) 2005, Mondrian Nuessle, Computer Architecture Group,               *
* University of Mannheim, Germany                                      *
*                                                                      *
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
*                                                                      *
* This program is distributed in the hope that it will be useful,      *
* but WITHOUT ANY WARRANTY; without even the implied warranty of       *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
* GNU General Public License for more details.                         *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with this program; if not, write to the Free Software          *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 *
* USA                                                                  *
*                                                                      *
* For informations regarding this file contact nuessle@uni-mannheim.de *
*                                                                      *
***********************************************************************/

#ifndef UTILS_H
#define UTILS_H

//AMD64 variante
#ifdef __x86_64__
#define serialize __asm__ __volatile__ ("cpuid" : : : "ax", "bx", "cx", "dx");
#else
//iA32 variante
#define serialize() asm volatile("lock ; addl $0,(%esp)")
#endif

#ifdef __POWERPC__
#define rdtsc(val) do { \
    unsigned long int __lower, __upper, __temp; \
    asm volatile ( \
        "0: mftbu %0\n" \
        "   mftb  %1\n" \
        "   mftbu %2\n" \
        "   cmpw  %0,%2\n" \
        "   bne-  0b\n" \
        : "=r" (__temp), "=r" (__lower), "=r" (__upper)); \
    (val) = ((((unsigned long long int)__upper)<<32)|((unsigned long long int)__lower)); \
} while(0)
#else
/* 
 * Workaround for the #define .. do{ .. } while(0) statement.
 * Otherwise it won't compile with gcc. (Günther Schindler)
 */
static __inline__ unsigned long long rdtsc(void){
     unsigned int __a,__d;
      asm volatile("rdtsc" : "=a" (__a), "=d" (__d));
     return ((unsigned long long)__a) | (((unsigned long long)__d)<<32);
}
#endif
#endif
