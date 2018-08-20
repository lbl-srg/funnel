/*
 * tube.c
 *
 * Created on: Apr 5, 2018
 * Author: jianjun
 *
 * Functions:
 * ----------
 *   interpolateValues: interpolate sources data points
 *   compare: compare test value with tube
 *   validate: validate test curve and generate error report
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data_structure.h"
#include "tubeSize.h"
#include "tube.h"

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

/*
 * Function: interpolateValues
 * ---------------------------
 *   interpolate sources data points,
 *                                 sourceY[j] - sourceY[j-1]
 *     targetY[i] = sourceY[j-1] + ------------------------- * (targetX[i] - sourceX[j-1])
 *                                 sourceX[j] - sourceX[j-1]
 *
 *   sourceX: source data X value
 *   sourceY: source data Y value
 *   sourceLength: total source data points
 *   targetX: target data X value
 *   targetLength: total target data points
 *
 *   return: targetY -- target data Y value
 */
double * interpolateValues(double* sourceX, double* sourceY, int sourceLength, double* targetX, int targetLength) {
	if (sourceY == NULL || sourceLength == 0) {
		return sourceY;
	}

	double* targetY = malloc(targetLength * sizeof(double));
	int j = 1;
	double x, x0, x1, y0, y1;

	for (int i=0; i<targetLength; i++) {
		// Prevent extrapolating
		if (targetX[i] > sourceX[sourceLength-1]) {
			double *tmp = realloc(targetY, sizeof(double)*i);
			targetY = tmp;
			break;
		}

		x = targetX[i];
		x1 = sourceX[j];
		y1 = sourceY[j];

		// Step sourceX to current targetX
		while ((x1<x) && (j+1 < sourceLength)) {
			j++;
			x1 = sourceX[j];
			y1 = sourceY[j];
		}

		x0 = sourceX[j-1];
		y0 = sourceY[j-1];

		// Prevent NaN -> division by zero
		if (((x1-x0)*(x-x0)) != 0) {
			targetY[i] = y0 + (((y1 - y0) / (x1 - x0)) * (x - x0));
		} else {
			targetY[i] = y0;
		}
	}

	return targetY;
}

/*
 * Function: errReport
 * -------------------
 *   compare test value with data tube
 *
 *   lower: lower tube curve value
 *   upper: upper tube curve value
 *   refLen: total data points in tube curve
 *   testY: test curve value
 *   testX: test curve time value
 *   testLen: total data points in test curve
 *
 *   return: errReport, data structure which includes;
 *              err.X -- time when there is error
 *              err.Y -- error value
 *           err.size -- total time moment when there is error
 */
struct errReport compare(double* lower, double* upper, int refLen, double* testY, double* testX, int testLen) {
	struct errReport err;
	int errArrSize = 1;
	int errCount = 0;
	double* errX = malloc(errArrSize * sizeof(double));
	double* errY = malloc(errArrSize * sizeof(double));

	int minSize = min(testLen,refLen);

	double* diffX = malloc(minSize * sizeof(double));
	double* diffY = malloc(minSize * sizeof(double));

	for (int i=0; i<minSize; i++) {
		if (testY[i] < lower[i] || testY[i] > upper[i]) {
			errX[errCount] = testX[i];
			if (testY[i] < lower[i]) {
				errY[errCount] = lower[i]-testY[i];
			} else {
				errY[errCount] = testY[i]-upper[i];
			}
			diffY[i] = errY[errCount];
			errCount++;
		} else {
			diffY[i] = 0.0;
		}
		diffX[i] = testX[i];
		// resize error arrays
		if (errCount == errArrSize) {
			errArrSize += 10;
			double *tmpX = realloc(errX, sizeof(double)*errArrSize);
			double *tmpY = realloc(errY, sizeof(double)*errArrSize);
			errX = tmpX;
			errY = tmpY;
		}
	}

	err.X = errX;
	err.Y = errY;
	err.size = errCount;
	err.diffX = diffX;
	err.diffY = diffY;
	err.diffSize = minSize;

	return err;
}


/*
 * Function: validate
 * ------------------
 *   validate test curve and generate error report
 *
 *   lower: data structure for lower curve
 *   upper: data structure for upper curve
 *   test: data structure for test curve
 *
 *   return: data structure which includes:
 *            report.test -- test curve
 *          report.errors -- error report
 *           report.valid -- validity
 */
struct reports validate(struct data lower, struct data upper, struct data test) {
	struct reports report;

	double* lowerX = lower.X;
	double* lowerY = lower.Y;
	int lowerSize = lower.size;

	double* upperX = upper.X;
	double* upperY = upper.Y;
	int upperSize = upper.size;

	double* testX = test.X;
	double* testY = test.Y;
	int testSize = test.size;

	if (lowerSize != 0 && testSize != 0) {
		double* newLower = interpolateValues(lowerX, lowerY, lowerSize, testX, testSize);
		double* newUpper = interpolateValues(upperX, upperY, upperSize, testX, testSize);

		report.test = test;
		report.errors = compare(newLower, newUpper, lowerSize, testY, testX, testSize);
		if (report.errors.size == 0) {
			report.valid = "Valid";
		} else {
			report.valid = "Invalid";
		}
	} else {
		report.valid = "Undefined";
	}

	return report;
}





