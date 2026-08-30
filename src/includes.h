#ifndef _INCLUDES_H
#define _INCLUDES_H

/* 2*pi as the original spelled it -- 3 ULP short of the real value
 * (matches the asm literal pool constant H'40C90FD8). */
#define TWO_PI 6.283184f

#ifdef UNIT_TESTING
#define STATIC
#else
#define STATIC static
#endif

/* Like STATIC, but also expands to nothing in a matching build. A unit compiled
   from C into that build (Makefile.matching SRCS) must emit each function as the
   original did -- `static` would let SHC drop an uncalled one or inline it and
   break the byte match. Use for private functions in those units. */
#if defined(UNIT_TESTING) || defined(MATCHING)
#define NM_STATIC
#else
#define NM_STATIC static
#endif

#endif // _INCLUDES_H
