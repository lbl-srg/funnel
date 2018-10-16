/*
 * tubeSize.h
 *
 *  Created on: Apr 5, 2018
 *      Author: jianjun
 */

#include "stdbool.h"

#ifndef TUBESIZE_H_
#define TUBESIZE_H_

double minValue(double* array, int size);

double maxValue(double* array, int size);

double * tubeSize(struct data refData, double singleValue, char axes, double valueX, double valueY, bool relativity);

#endif /* TUBESIZE_H_ */
