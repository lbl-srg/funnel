/*
 * tube.h
 *
 *  Created on: Apr 5, 2018
 *      Author: jianjun
 */

#ifndef TUBE_H_
#define TUBE_H_

double * interpolateValues(double* sourceX, double* sourceY, int sourceLength, double* targetX, int targetLength);

struct errReport compare(double* lower, double* upper, int refLen, double* testY, double* testX, int testLen);

struct reports validate(struct data lower, struct data upper, struct data test);


#endif /* TUBE_H_ */
