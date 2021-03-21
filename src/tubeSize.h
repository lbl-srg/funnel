/*
 * tubeSize.h
 *
 *  Created on: Apr 5, 2018
 *      Author: jianjun
 */

#include "stdbool.h"

#ifndef TUBESIZE_H_
#define TUBESIZE_H_

void set_tube_size(struct data *tube_size, struct data *refData, struct tolerances tol);

struct data_char get_data_char(struct data *dat);

double minValue(double* array, size_t size);

double maxValue(double* array, size_t size);

#endif /* TUBESIZE_H_ */
