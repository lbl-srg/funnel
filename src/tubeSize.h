/*
 * tubeSize.h
 *
 *  Created on: Apr 5, 2018
 *      Author: jianjun
 */

#include "stdbool.h"

#ifndef TUBESIZE_H_
#define TUBESIZE_H_

void tube_size_calc(struct data *refData, struct data *tube_size, struct tolerances tol);

double minValue(double* array, int size);

double maxValue(double* array, int size);

#endif /* TUBESIZE_H_ */
