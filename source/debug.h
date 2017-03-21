
#ifndef _debug_h_
#define _debug_h_

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

  /*
   * Debug printf statements.
   * Taken from here: http://stackoverflow.com/a/1941337
   * Called like
   *  DEBUG_PRINT(("Hi %d", 1));
   * Note: The extra parentheses are necessary, because some older C compilers don't support var-args in macros.
   */
  /* 1: enables extra print statements, 0: disable */
  #define DEBUG 1
  #ifdef DEBUG
  # define DEBUG_PRINT(x) printf x
  #else
  # define DEBUG_PRINT(x) do {} while (0)
  #endif


#define perror_line(x) { fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, x, strerror(errno)); fflush(stdout); exit(1); }

#endif
