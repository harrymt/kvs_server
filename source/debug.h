
#ifndef _debug_h_
#define _debug_h_

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

  /*
   * Debug print statements, taken from here: http://stackoverflow.com/a/1941337
   *
   * Example:
   *
   *  DEBUG_PRINT(("Hi %d", 1));
   *
   * Note: The extra parentheses are necessary, because some older C compilers don't support var-args in macros.
   */
        //   #define DEBUG
  #ifdef DEBUG
  # define DEBUG_PRINT(x) printf x
  #else
  # define DEBUG_PRINT(x)
  #endif


/**
 * Macro to print out the line number of the file that created the error number, then exit.
 */
#define perror_exit(x) { fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, x, strerror(errno)); fflush(stdout); exit(1); }

#endif
