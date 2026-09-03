/*
 * algorithmRectangle.h
 *
 *  Created on: May 10, 2018
 *      Author: jianjun
 */

#ifndef ALGORITHMRECTANGLE_H_
#define ALGORITHMRECTANGLE_H_

struct data getLower(struct data *reference, struct data *tube_size);

struct data getUpper(struct data *reference, struct data *tube_size);

struct data removeLoop(double* x, double* y, int size, int curInd);

double * removeRange(double* array, int size, int staInd, int count);

double * removeAt(double* array, int size, int ind);

double * insertAt(double* array, int size, int index, double item);

#endif /* ALGORITHMRECTANGLE_H_ */
