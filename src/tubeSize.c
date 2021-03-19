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
 *   tube_size_calc : calculate tube size (half-width and half-height of rectangle)
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
 * Function: set_data
 * -----------------
 *   find maximum values and value ranges in x and y of reference data
 *
 *   refData: CSV data which will be used as reference
 *
 *   return : a data_char struct
 */

struct data_char {
  double range_x;  /* Range of x */
  double range_y;  /* Range of y */
  double mag_x;    /* Magnitude of x */
  double mag_y;    /* Magnitude of y */
};

struct data_char set_data(struct data refData) {

  double maxX = maxValue(refData.x, refData.n);
  double minX = minValue(refData.x, refData.n);

  double maxY = maxValue(refData.y, refData.n);
  double minY = minValue(refData.y, refData.n);

  struct data_char d = {
    .range_x=maxX - minX,
    .range_y=maxY - minY,
    .mag_x=max(maxX, fabs(minX)),
    .mag_y = max(maxY, fabs(minY))
  };

  return d;
}

/*
 * Function: tube_size_calc
 * ------------------
 *   calculate tube size (half-width and half-height of rectangle)
 *
 *   refData: CSV data which will be used as reference
 *   tol    : data structure containing absolute tolerance (atolx, atoly)
 *            and relative tolerance (rtolx, rtoly)
 *
 *   return : a tube_size struct
 */
struct tube_size tube_size_calc(struct data refData, struct tolerances tol) {
  double dx, dy;
  struct data_char d = set_data(refData);
  struct tube_size tube_s;

  if ((equ(tol.atolx,0) && equ(tol.rtolx, 0)) || (equ(tol.atoly,0) && equ(tol.rtoly, 0))) {
    fputs("Error: At least one tolerance has to be set for both x and y.\n", stderr);
    exit(1);
  }

  if (equ(d.range_x, 0)) {
    dx = max(tol.atolx, tol.rtolx * d.mag_x);
  } else {
    dx = max(tol.atolx, tol.rtolx * d.range_x);
  }

  if (equ(d.range_y, 0)) {
    dy = max(tol.atoly, tol.rtoly * d.mag_y);
  } else {
    dy = max(tol.atoly, tol.rtoly * d.range_y);
  }

  tube_s.dx = dx;
  tube_s.dy = dy;
  tube_s.range_x = d.range_x;
  tube_s.range_y = d.range_y;

  return tube_s;
}
