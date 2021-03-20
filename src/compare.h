#ifndef COMPARE_H_
#define COMPARE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"

#include "data_structure.h"
#include "readCSV.h"
#include "algorithmRectangle.h"
#include "tube.h"
#include "tubeSize.h"
#include "mkdir_p.h"

#define MAX 100

/*
*   Descriptor of the file used for logging the numerical processing errors
*   (all other errors like memory, file access, bad argument...
*   are still output to stderr.)
*/
FILE *log_file;

/*
 * Function: compareAndReport
 * -----------------------
 *   This function does the actual computations. It is introduced so that it
 *   can be called from Python in which case the argument parsing of main
 *   is not needed.
 */
int compareAndReport(
  const double* tReference,
  const double* yReference,
  const size_t nReference,
  const double* tTest,
  const double* yTest,
  const size_t nTest,
  const char * outputDirectory,
  const double atolx,
  const double atoly,
  const double rtolx,
  const double rtoly,
  const double ltolx,
  const double ltoly
);

#endif /* COMPARE_H_ */
