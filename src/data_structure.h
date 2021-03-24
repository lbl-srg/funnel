/*
 * data_structure.h
 *
 *  Created on: Apr 4, 2018
 *      Author: jianjun
 */

#ifndef DATA_STRUCTURE_H_
#define DATA_STRUCTURE_H_

#include <sys/types.h>

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
