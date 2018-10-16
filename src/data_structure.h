/*
 * data_structure.h
 *
 *  Created on: Apr 4, 2018
 *      Author: jianjun
 */

#ifndef DATA_STRUCTURE_H_
#define DATA_STRUCTURE_H_

struct data {
	double *X;
	double *Y;
	int size;
};

struct errReport {
	double *X;
	double *Y;
	int size;
	double *diffX;
	double *diffY;
	int diffSize;
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
