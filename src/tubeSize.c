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

struct data_char set_data(struct data *refData) {

  double maxX = maxValue(refData->x, refData->n);
  double minX = minValue(refData->x, refData->n);

  double maxY = maxValue(refData->y, refData->n);
  double minY = minValue(refData->y, refData->n);

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
 *   refData: pointer to struct with the reference data
 *   tube_size : pointer to struct with the tube size
 *   tol    : struct with tolerance values
 *
 *   return : void (modifies tube_size in place)
 */
void tube_size_calc(struct data *refData, struct data *tube_size, struct tolerances tol) {
  int i;
  struct data_char d = set_data(refData);

  if (((equ(tol.atolx,0) && equ(tol.rtolx, 0)) && equ(tol.ltolx, 0)) ||
      ((equ(tol.atoly,0) && equ(tol.rtoly, 0)) && equ(tol.ltoly, 0))) {
    fputs("Error: At least one of the possible tolerance parameters \
           (atol, ltol, or rtol) must be defined for each axis..\n", stderr);
    exit(1);
  }

  for (i=0; i<refData->n; i++)
  {
    tube_size->x[i] = max(max(tol.atolx, tol.rtolx * d.range_x), tol.ltolx * refData->x[i]);
    tube_size->y[i] = max(max(tol.atoly, tol.rtoly * d.range_y), tol.ltoly * refData->y[i]);

    if (equ(tube_size->x[i], 0))
    {
      if (!equ(tol.rtolx, 0))
      /* This is for consistency with csv compare. */
      {
        tube_size->x[i] = max(tube_size->x[i], tol.rtolx * d.mag_x);
      }
      if (equ(tube_size->x[i], 0))
      /* Still possible if magnitude is 0 or rtol is 0 or ltol is 0 or local value is 0.
         Then we consider both rtol and ltol as absolute.
      */
      {
        tube_size->x[i] = max(tube_size->x[i], max(tol.rtolx, tol.ltolx));
      }
    }
    /* Same logic for y variable. */
    if (equ(tube_size->y[i], 0))
    {
      if (!equ(tol.rtoly, 0))
      /* This is for consistency with csv compare: we use the magnitude if the range is 0. */
      {
        tube_size->y[i] = max(tube_size->y[i], tol.rtoly * d.mag_y);
      }
      if (equ(tube_size->y[i], 0))
      /* Still possible if magnitude is 0 or rtol is 0 or ltol is 0 or local value is 0.
         Then we consider both rtol and ltol as absolute.
      */
      {
        tube_size->y[i] = max(tube_size->y[i], max(tol.rtoly, tol.ltoly));
      }
    }
  }
}
