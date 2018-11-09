/*
 * tubeSize.c
 *
 * Created on: Apr 4, 2018
 * Author: jianjun
 *
 * Functions:
 * ----------
 *   minValue : find minimum value of an array
 *   maxValue : find maximum value of an array
 *   setStandardBaseAndRatio : calculate standard values for baseX, baseY and ratio
 *   setFormerBaseAndRatio : calculate former standard values for baseX, baseY and ratio
 *   tubeSize : calculate tubeSize (half-width and half-height of rectangle)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "data_structure.h"
#include "tubeSize.h"

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef equ
#define equ(a,b) (fabs(a-b) < 1e-10 ? true : false)
#endif


/*
 * Function: minValue
 * ------------------
 *   find minimum value of an array
 *
 *   array: data array
 *   size: data array size
 *
 *   return: minimum value of the data array
 */
double minValue(double* array, int size) {
  int i;
  double min;
  min = array[0];
  for (i=0; i<size; i++) {
    if (array[i] < min) {
      min = array[i];
    }
  }
  return min;
}

/*
 * Function: maxValue
 * ------------------
 *   find maximum value of an array
 *
 *   array: data array
 *   size: data array size
 *
 *   return: maximum value of the data array
 */
double maxValue(double* array, int size) {
  int i;
  double max;
  max = array[0];
  for (i=0; i<size; i++) {
    if (array[i] > max) {
      max = array[i];
    }
  }
  return max;
}

/*
 * Function: setStandardBaseAndRatio
 * ---------------------------------
 *   calculate standard values for base of relative values in x and y direction, and their ratio
 *
 *   refData: reference data set
 *
 *   return: data structure including base values and their ratio
 */
double * setStandardBaseAndRatio(struct data refData) {
  double baseX, baseY, ratio;
  double* staBasRat = malloc(3 * sizeof(double));

  double maxX = maxValue(refData.x, refData.n);
  double minX = minValue(refData.x, refData.n);

  double maxY = maxValue(refData.y, refData.n);
  double minY = minValue(refData.y, refData.n);

  // set baseX
  baseX = maxX - minX;
  if (equ(baseX, 0)) {
    baseX = fabs(maxX);
  }
  if (equ(baseX, 0)) {
    baseX = 1;
  }
  // set baseY
  baseY = maxY - minY;
  if (equ(baseY, 0)) {
    baseY = fabs(maxY);
  }
  if (equ(baseY, 0)) {
    baseY = 0.0000000000000001;
  }
  // set ratio
  if (!equ(baseX, 0)) {
    ratio = baseY / baseX;
  } else {
    ratio = 0;
  }

  staBasRat[0] = baseX;
  staBasRat[1] = baseY;
  staBasRat[2] = ratio;

  return staBasRat;
}

/*
 * Function: setFormerBaseAndRatio
 * -------------------------------
 *   calculate former standard values for baseX, baseY and ratio
 *
 *   refData: reference data set
 *
 *   return: structure of data set including base values and their ratio
 */
double * setFormerBaseAndRatio(struct data refData) {
  double baseX, baseY, ratio;
  double* staBasRat = malloc(3 * sizeof(double));

  double maxX = maxValue(refData.x,refData.n);
  double minX = minValue(refData.x,refData.n);

  double maxY = maxValue(refData.y,refData.n);
  double minY = minValue(refData.y,refData.n);

  baseX = maxX - minX + fabs(minX);
  baseY = maxY - minY + fabs(minY);

  if (!equ(maxX, minX)) {
    ratio = max(0.0004, ((maxY - minY + fabs(minY)) / (maxX - minX)));
  } else {
    ratio = 0;
  }

  staBasRat[0] = baseX;
  staBasRat[1] = baseY;
  staBasRat[2] = ratio;
  return staBasRat;
}


/*
 * Function: tubeSize
 * ------------------
 *   calculate tubeSize (half-width and half-height of rectangle)
 *
 *   refData: CSV data which will be used as reference
 *   singleValue: single value (relative or absolute ) that will be used for defining size.
 *                If it is non-zero, the size will be defined based on this single value
 *   axes: when single value is used (non-zero), it sets whether the single value is for half-width (x) or half-height (y)
 *   valueX: when singleValue is zero, it defines half-width (relative or absolute)
 *   valueY: when singleValue is zero, it defines half-height (relative or absolute)
 *   relative : define if it is relative value
 *
 *   return : an array "tubeSize" that includes:
 *                   x -- half width of rectangle
 *                   y -- half height of rectangle
 *               baseX -- base of relative value is x direction
 *               baseY -- base of relative value is y direction
 *               ratio -- ratio y / x
 */
double * tubeSize(struct data refData, double singleValue, char axes, double valueX, double valueY, bool relative) {
  double x, y, baseX, baseY, ratio;
  double* tubeSize = malloc(5 * sizeof(double));

  int i = 2; // 1: SetFormerBaseAndRatio; 2: SetStandardBaseAndRatio;
  double *standValue;

  if (i == 1) {
    standValue = setFormerBaseAndRatio(refData);
  } else {
    standValue = setStandardBaseAndRatio(refData);
  }
  baseX = standValue[0];
  baseY = standValue[1];
  ratio = standValue[2];

  // Specify single value to define half-width (x) or half-height (y) of rectangle
  if (singleValue != 0) {
    // If non-zero ratio
    if (ratio > 0) {
      // If relative value
      if (relative) {
        if ((singleValue < 0) || (singleValue > 1)) {
          fputs("Relative value is out of expected range [0, 1].\n", stderr);
          exit(1);
        }
        // If value is relative to half-height (axes = y)
        if ((axes == 'y') && (baseY > 0)) {
          y = singleValue * baseY;
          x = y / ratio;
        // If value is relative to half-width (axes = x)
        } else if ((axes == 'x') && (baseX > 0)) {
          x = singleValue * baseX;
          y = ratio * x;
        }
      // If absolute value
      } else if (!relative) {
        // If value is half-height
        if (axes == 'y') {
          y = singleValue;
          x = singleValue / ratio;
        // If value is half-width
        } else if (axes == 'x') {
          x = singleValue;
          y = ratio * singleValue;
        }
      }
    }
  // Specify both height and width values to define half-width (x) and half-height (y) of rectangle
  } else {
    // If relative value
    if (relative && (baseX != 0) && (baseY != 0)) {
      if ((valueX < 0) || (valueX > 1))
        fputs("Relative x value is out of expected range [0, 1].", stderr);
      if ((valueY < 0) || (valueY > 1))
        fputs("Relative y value is out of expected range [0, 1].", stderr);
      x = valueX * baseX;
      y = valueY * baseY;
    // If absolute value
    } else if (!relative) {
      x = valueX;
      y = valueY;
    }

  }

  tubeSize[0] = x;
  tubeSize[1] = y;
  tubeSize[2] = baseX;
  tubeSize[3] = baseY;
  tubeSize[4] = ratio;

  return tubeSize;
}
