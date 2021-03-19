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
  double* x;
  double* y;
  size_t n;
};

struct errorReport {
  struct data original;
  struct data diff;
};

struct reports {
  struct errorReport errors;
};

struct tolerances {
	double atolx;
	double atoly;
	double rtolx;
	double rtoly;
};

struct tube_size {
  double dx;       /* Half width of rectangle  */
  double dy;       /* Half height of rectangle  */
  double range_x;  /* Range of x */
  double range_y;  /* Range of y */
};

#endif /* DATA_STRUCTURE_H_ */
