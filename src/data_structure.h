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

#endif /* DATA_STRUCTURE_H_ */
