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
 *   The tolerance is 1e-10 times the larger operand, and 1e-10 for values at
 *   or below magnitude one.
 *
 *   For parameters and data ranges only (tubeSize.c, compare.c). The tube
 *   construction compares exactly: see algorithmRectangle.c.
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
