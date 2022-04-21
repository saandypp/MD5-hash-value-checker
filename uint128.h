/* 
 * Operating Systems (2INC0) Practical Assignment
 *
 * Contains definitions for 128-bit integers (which are not present in <stdint.h>)
 *
 */


#ifndef _UINT128_H_
#define _UINT128_H_

#include <stdint.h>		// for uint64_t

// gcc offers __int128, but not the unsigned version
typedef unsigned __int128	uint128_t;


// create a 128-bit constant integer out of two 64-bit constant integers
#define UINT128(hi,lo)		((((uint128_t) (hi)) << 64) + ((uint128_t) (lo)))


// printf() cannot handle a 128-bit integer, 
// therefore: print the HIgh and a LOw part (both are 64-bit integers) separately
#define HI(a)	((uint64_t) ((a) >> 64))
#define LO(a)	((uint64_t) (a))

#endif

