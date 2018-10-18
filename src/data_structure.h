/*
 * data_structure.h
 *
 *  Created on: Apr 4, 2018
 *      Author: jianjun
 */

#ifndef DATA_STRUCTURE_H_
#define DATA_STRUCTURE_H_

struct data {
  double *x;
  double *y;
  size_t n;
};

struct errReport {
  double *x;
  double *y;
  size_t n;
  double *diffX;
  double *diffY;
  size_t diffSize;
};

struct reports {
  struct data test;
  struct errReport errors;
  char *valid;
};

struct sumData {
  struct data refData;
  struct data testData;
  struct data lowerCurve;
  struct data upperCurve;
  struct reports validateReport;
};

#endif /* DATA_STRUCTURE_H_ */
