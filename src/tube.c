/*
 * tube.c
 *
 * Created on: Apr 5, 2018
 * Author: jianjun
 *
 * Functions:
 * ----------
 *   interpolate: interpolate source data points, with a choice of the value at a vertical edge
 *   interpolateValues: interpolate source data points
 *   compare: compare test value with tube
 *   validate: validate test curve and generate error report
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "data_structure.h"
#include "tubeSize.h"
#include "tube.h"

/* Exact comparisons of x values are intended here, see algorithmRectangle.c. */
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

/*
 * Function: interpolate
 * ---------------------
 *   interpolate source data points onto target x values,
 *                                 sourceY[j] - sourceY[j-1]
 *     targetY[i] = sourceY[j-1] + ------------------------- * (targetX[i] - sourceX[j-1])
 *                                 sourceX[j] - sourceX[j-1]
 *
 *   A target x equal to a source x takes the source y as is. The source may
 *   hold several points at that x: a tube curve has a vertical edge wherever
 *   the reference jumps by more than the tube height within the x tolerance.
 *   `side` selects the value at such an edge:
 *      0: the first point at that x
 *     -1: the lowest one (lower tube curve)
 *      1: the highest one (upper tube curve)
 *   The tube is a closed set, so its edge belongs to it: with side = -1 or 1,
 *   a test point at exactly the x tolerance from a jump of the reference is
 *   inside the tube.
 *
 *   x values are compared exactly (see algorithmRectangle.c).
 */
static double * interpolate(double* sourceX, double* sourceY, int sourceLength,
  double* targetX, int targetLength, int side) {
  if (sourceY == NULL || sourceLength == 0) {
    return sourceY;
  }
  int i;
  double* targetY = malloc(targetLength * sizeof(double));
  if (targetY == NULL){
  	  fputs("Error: Failed to allocate memory for targetY.\n", stderr);
  	  return NULL;
  }
  if (sourceLength == 1) {
    /* A single point holds no segment to interpolate on. */
    for (i=0; i<targetLength; i++) {
      targetY[i] = sourceY[0];
    }
    return targetY;
  }
  int j = 1;
  double x, x0, x1, y0, y1;

  for (i=0; i<targetLength; i++) {
    /* Prevent extrapolating: hold the last source value for the remainder.
       Shrinking the buffer here was unsafe -- compare() is handed the target
       length regardless and read past the end -- and realloc(p, 0) at i == 0
       may legitimately return NULL, which was then treated as a failure and
       terminated the process. */
    if (targetX[i] > sourceX[sourceLength-1]) {
      int k;
      for (k = i; k < targetLength; k++) {
        targetY[k] = sourceY[sourceLength-1];
      }
      break;
    }

    x = targetX[i];
    x1 = sourceX[j];
    y1 = sourceY[j];

    // Step sourceX to current targetX
    while ((x1<x) && (j+1 < sourceLength)) {
      j++;
      x1 = sourceX[j];
      y1 = sourceY[j];
    }

    x0 = sourceX[j-1];
    y0 = sourceY[j-1];

    if (x == x1 || x == x0) {
      /* On a source point: no interpolation. Source points j-1, j, j+1, ...
         may share this x. */
      int k = (x == x0) ? j-1 : j;
      double y = sourceY[k];
      if (side != 0) {
        for (k = k+1; k < sourceLength && sourceX[k] == x; k++) {
          if ((side < 0 && sourceY[k] < y) || (side > 0 && sourceY[k] > y)) {
            y = sourceY[k];
          }
        }
      }
      targetY[i] = y;
    } else if (x1 != x0) {
      targetY[i] = y0 + (((y1 - y0) / (x1 - x0)) * (x - x0));
    } else {
      targetY[i] = y0;
    }
  }

  return targetY;
}

/*
 * Function: interpolateValues
 * ---------------------------
 *   interpolate source data points onto target x values
 *
 *   sourceX: source data x value
 *   sourceY: source data y value
 *   sourceLength: total source data points
 *   targetX: target data x value
 *   targetLength: total target data points
 *
 *   return: targetY -- target data y value
 */
double * interpolateValues(double* sourceX, double* sourceY, int sourceLength, double* targetX, int targetLength) {
  return interpolate(sourceX, sourceY, sourceLength, targetX, targetLength, 0);
}

/*
 * Function: compare
 * -------------------
 *   compare test value with data tube
 *
 *   lower: lower tube curve value
 *   upper: upper tube curve value
 *   refLen: total data points in tube curve
 *   testY: test curve value
 *   testX: test curve time value
 *   testLen: total data points in test curve
 *
 *   return: errorReport, data structure which includes;
 *              err->x -- time when there is error
 *              err->y -- error value
 *           err->n -- total time moment when there is error
 */
int compare(double* lower, double* upper, int refLen,
  double* testY, double* testX, int testLen,
  struct errorReport* err) {
  size_t i;
  size_t errArrSize = 1;
  err->original.n = 0;
  err->original.x = malloc(errArrSize * sizeof(double));
  if (err->original.x == NULL){
    fputs("Error: Failed to allocate memory for err->original.x.\n", stderr);
    return -1;
  }
  err->original.y = malloc(errArrSize * sizeof(double));
  if (err->original.y == NULL){
    fputs("Error: Failed to allocate memory for err->original.y.\n", stderr);
    return -1;
  }

  err->diff.n = min(testLen, refLen);
  err->diff.x = malloc(err->diff.n * sizeof(double));
  if (err->diff.x == NULL){
    fputs("Error: Failed to allocate memory for err->diff.x.\n", stderr);
    return -1;
  }
  err->diff.y = malloc(err->diff.n * sizeof(double));
  if (err->diff.y == NULL){
    fputs("Error: Failed to allocate memory for err->diff.y.\n", stderr);
    return -1;
  }

  for (i=0; i < err->diff.n; i++) {
    if (testY[i] < lower[i] || testY[i] > upper[i]) {
      err->original.x[err->original.n] = testX[i];
      if (testY[i] < lower[i]) {
        err->original.y[err->original.n] = lower[i]-testY[i];
      } else {
        err->original.y[err->original.n] = testY[i]-upper[i];
      }
      err->diff.y[i] = err->original.y[err->original.n];
      err->original.n++;
    } else {
      err->diff.y[i] = 0.0;
    }
    err->diff.x[i] = testX[i];
    // resize error arrays
    if (err->original.n == errArrSize) {
      errArrSize += 10;
      err->original.x = realloc(err->original.x, sizeof(double)*errArrSize);
      if (err->original.x == NULL){
        fputs("Error: Failed to reallocate memory for err->original.x.\n", stderr);
        return -1;
      }

      err->original.y = realloc(err->original.y, sizeof(double)*errArrSize);
      if (err->original.y == NULL){
        fputs("Error: Failed to reallocate memory for err->original.y.\n", stderr);
        return -1;
      }
    }
  }
  return 0;
}


/*
 * Function: validate
 * ------------------
 *   validate test curve and generate error report
 *
 *   lower: data structure for lower curve
 *   upper: data structure for upper curve
 *   test: data structure for test curve
 *
 *   return: 0 if there was success
 */
int validate(
  const struct data lower,
  const struct data upper,
  const struct data test,
  struct errorReport* err) {
    /* At a vertical edge of the tube, take its outer end: the edge is inside. */
    double *newLower = interpolate(lower.x, lower.y, lower.n, test.x, test.n, -1);
    double *newUpper = interpolate(upper.x, upper.y, upper.n, test.x, test.n, 1);
    if (newLower == NULL || newUpper == NULL) {
      fputs("Error: Failed to interpolate the tube curves onto the test grid.\n", stderr);
      if (newLower != lower.y) free(newLower);
      if (newUpper != upper.y) free(newUpper);
      return -1;
    }
    int retVal = compare(newLower, newUpper, test.n, test.y, test.x, test.n, err);
    /* interpolateValues returns its own buffer, except in the degenerate case
       where it hands back the source pointer -- which the caller still owns. */
    if (newLower != lower.y) free(newLower);
    if (newUpper != upper.y) free(newUpper);
    return retVal;
}
