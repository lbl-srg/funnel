/*
 * tubeSize.h
 *
 *  Created on: Apr 5, 2018
 *      Author: jianjun
 */

#include "stdbool.h"

#ifndef TUBESIZE_H_
#define TUBESIZE_H_

struct tube_size tube_size_calc(struct data refData, struct tolerances tol);

double minValue(double* array, int size);

double maxValue(double* array, int size);

#endif /* TUBESIZE_H_ */
