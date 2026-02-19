//
// Created by felix on 2/19/26.
//

#ifndef NYX_NMATH_TEST_INTERNALS_H
#define NYX_NMATH_TEST_INTERNALS_H

#include "nmath/nmath.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

inline void err_proc(enum NMath_Error_Info e)
{
  if (e != nmath_okay)
  {
    const char *estr = nmath_explain_err(e);
    printf("NLayout error: %s\n", estr);
    fflush(stdout);
    exit(EXIT_FAILURE);
  }
}

inline double get_ms(const struct timespec start, const struct timespec end)
{
  return (double)(end.tv_sec - start.tv_sec) * 1000.0 +
         (double)(end.tv_nsec - start.tv_nsec) / 1000000.0;
}

#endif // NYX_NMATH_TEST_INTERNALS_H
