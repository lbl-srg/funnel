/*
 * data_structure.h
 *
 *  Created on: Apr 4, 2018
 *      Author: jianjun
 */

#ifndef DATA_STRUCTURE_H_
#define DATA_STRUCTURE_H_

#include <math.h>
#include <stdbool.h>
#include <sys/types.h>

/*
 * Function: equ
 * -------------
 *   compare two doubles for equality, relative to their magnitude
 *
 *   This was an absolute 1e-10, defined four times over in four translation
 *   units -- twice with the parentheses that the other two carry a comment
 *   about needing. An absolute tolerance is unusable above magnitude one: at
 *   3.15e7, a year in seconds, a double's ulp is about 7.5e-9, so no two
 *   distinct values can satisfy it and values equal to within rounding read as
 *   different. getLower and getUpper normalise x precisely to work around
 *   that, but y, the slopes computed from it, and the endpoint check in
 *   compareAndReport are all left at their natural magnitude.
 *
 *   Scaling by the larger operand holds the tolerance at a constant ~1e-10
 *   relative, which is what the absolute form already meant for values of
 *   order one. At or below magnitude one the result is unchanged, which is why
 *   the stored numerics results are unaffected.
 */
static inline bool equ(double a, double b) {
  double mag_a = fabs(a);
  double mag_b = fabs(b);
  double mag = (mag_a > mag_b) ? mag_a : mag_b;
  return fabs(a - b) <= 1e-10 * ((mag > 1.0) ? mag : 1.0);
}

struct data {
  double *x;
  double *y;
  size_t n;
};

struct data_char {
  double range_x;  /* Range of x */
  double range_y;  /* Range of y */
  double mag_x;    /* Magnitude of x */
  double mag_y;    /* Magnitude of y */
};

struct errorReport {
  struct data original;
  struct data diff;
};

struct reports {
  struct errorReport errors;
};

struct tolerances {
	double atolx;  /* Absolute tolerance in x */
	double atoly;  /* Absolute tolerance in y */
	double ltolx;  /* Relative tolerance in x (relatively to local value) */
	double ltoly;  /* Relative tolerance in y (relatively to local value) */
	double rtolx;  /* Relative tolerance in x (relatively to range */
	double rtoly;  /* Relative tolerance in y (relatively to range) */
};

#endif /* DATA_STRUCTURE_H_ */
