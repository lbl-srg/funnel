/*
 * algorithmRectangle.c
 *
 * Created on: Apr 5, 2018
 * Author: jianjun
 *
 * Functions:
 * ----------
 *   createNode: creates new linked list node
 *   addNode: add node to a existing linked list at end
 *   listLen: find length of linked list
 *   getNth: find value of specific node in linked list
 *   getListValues: find values at all the nodes of linked list
 *   lastNodeDeletion: delete last node of the linked list
 *   calculateLower: find the data set of lower tube curve
 *   calculateUpper: find the data set of upper tube curve
 *   removeLoop: remove points and add intersection points in case of backward order
 *   removeRange: remove a range of elements from array
 *   removeAt: remove element at the specified index from array
 *   insertAt: insert element into the array at the specified index
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stdbool.h"

#include "data_structure.h"
#include "algorithmRectangle.h"
#include "tubeSize.h"

#ifndef sign
#define sign(a) (((a)>0) ? 1 : (((a)<0) ? -1 : 0))
#endif

#ifndef equ
#define equ(a,b) (fabs((a)-(b)) < 1e-10 ? true : false)  /* (b) required by Win32 compiler for <0 values */
#endif

#if defined(_MSC_VER)
#define inline __inline
#endif


/*
 * Function: createNode
 * --------------------
 *   creates new linked list node
 */
node_t * createNode(void) {
  node_t* temp;
  temp = malloc(sizeof(node_t));
  if (temp == NULL){
	  fputs("Error: Failed to allocate memory for temp.\n", stderr);
    exit(1);
  }
  temp->next = NULL;
  return temp;
}

/*
 * Function: addNode
 * -----------------
 *   add node to a existing linked list at end
 *
 *   head: original linked list to add additional node at end
 *   newVal: value of the additional node
 *
 *   return: new linked list that have an additional node
 */
node_t * addNode(node_t* head, double newVal) {
  node_t* temp;
  node_t* p;
  // create additional node
  temp = createNode();
  temp->val = newVal;

  if (head == NULL) {
    head = temp;
  } else {
    p = head;
    while (p->next != NULL) {
      p = p->next;
    }
    p->next = temp;
  }
  return head;
}

/*
 * Function: listLen
 * -----------------
 *   find length of linked list
 *
 *   head: linked list
 *
 *   return: number of nodes in the linked list
 */
int listLen(node_t* head) {
  int count = 0;
  node_t* current = head;  // Initialize current
  while (current != NULL) {
    count = count+1;
    current = current->next;
  }
  return count;
}

/*
 * Function: getNth
 * ----------------
 *   find value of specific node in linked list
 *
 *   head: linked list
 *   index: zero-based index at which node its value will be output
 *
 *   return: value of Nth node of lined list
 */
double getNth(node_t* head, int index) {
    node_t* current = head;
    int count = 0; // the index of the node we're currently looking at
    while (current != NULL) {
       if (count == index)
         break;
       count = count+1;
       current = current->next;
    }
    return current->val;
}

/*
 * Function: getListValues
 * -----------------------
 *   find values at all the nodes of linked list
 *
 *   head: linked list
 *
 *   return: data array storing values of linked list
 */
double * getListValues(node_t* head) {
  int size = 1;
  double *value = malloc(sizeof(double) * size);
  if (value == NULL){
	  fputs("Error: Failed to allocate memory for value.\n", stderr);
    exit(1);
  }
  memset(value,0,sizeof(double)*size);

  node_t* current = head;
  int count = 0;
  while (current != NULL) {
    value[count] = current->val;
    if (count+1 == size) {
      /* need more space */
      size += 10;
      double *value_tmp = realloc(value, sizeof(double)*size);
      if (value_tmp == NULL) {
        fputs("Fatal error -- out of memory!\n", stderr);
        exit(1);
      }
      value = value_tmp;
    }
    count = count+1;
    current = current->next;
  }
  return value;
}

/*
 * Function: lastNodeDeletion
 * --------------------------
 *   delete last node of the linked list
 *
 *   head: linked list
 */
void lastNodeDeletion(node_t* head) {
  node_t* toDelLast;
  node_t* preNode;
    if(head == NULL) {
        printf("There is no element in the list.");
    } else {
      toDelLast = head;
        preNode = head;
        /* Traverse to the last node of the list */
        while(toDelLast->next != NULL) {
            preNode = toDelLast;
            toDelLast = toDelLast->next;
        }
        if(toDelLast == head) {
          /* If there is only one item in the list, remove it */
            head = NULL;
        } else {
            /* Disconnects the link of second last node with last node */
            preNode->next = NULL;
        }
        /* Delete the last node */
        if (toDelLast != NULL) free(toDelLast);
    }
}

/* Normalize variable array by variable magnitude */
void normalize(double *var, size_t length, double var_mag) {
  for (size_t i = 0; i < length; i++) {
    if (var_mag > 1E-5) {
      var[i] = var[i] / var_mag;
    } else {
      var[i] = var[i];
    }
  }
}

/* Denormalize variable array by variable magnitude */
void denormalize(double *var, size_t length, double var_mag) {
  for (size_t i = 0; i < length; i++) {
    if (var_mag > 1E-5) {
      var[i] = var[i] * var_mag;
    } else {
      var[i] = var[i];
    }
  }
}

/*
 * Function: setLower
 * ------------------------
 *   find the data set of lower tube curve
 *
 *   reference: pointer to reference data struct
 *   tube_size: pointer to tube_size struct
 *
 *   return : data struct defining lower curve of the tube
 */
struct data getLower(struct data *reference, struct data *tube_size) {
  struct data lower;
  node_t *lx = NULL;
  node_t *ly = NULL;
  size_t i, b;

  /* Normalize values and tube size in x direction.
   * This was introduced in https://github.com/lbl-srg/funnel/pull/30
   * to guard against vanishing derivatives (dy/dx) for x values with a large order of magnitude.
   */

  struct data_char dat_char = get_data_char(reference);                   // Data characteristics
  double *x_norm = (double *)malloc(sizeof(double) * reference->n);       // Normalized x values
  double *tube_x_norm = (double *)malloc(sizeof(double) * tube_size->n);  // Normalized tube size in x direction
  if ((x_norm == NULL) || (tube_x_norm == NULL)){
	  fputs("Error: Failed to allocate memory for x_norm or tube_x_norm.\n", stderr);
    exit(1);
  }
  memcpy(x_norm, reference->x, sizeof(double) * reference->n);
  memcpy(tube_x_norm, tube_size->x, sizeof(double) * tube_size->n);
  normalize(x_norm, reference->n, dat_char.mag_x);
  normalize(tube_x_norm, tube_size->n, dat_char.mag_x);

  // ===== 1. add corner points of the rectangle =====
  double m0, m1; // slopes before and after point i of reference curve
  double s0, s1; // sign of slopes of reference curve: 1 - increasing, 0 - constant, -1 - decreasing

  // ----- 1.1 Start: rectangle with center (x,y) = (reference->x[0], reference->y[0]) -----
  // ignore identical point at the beginning
  b = 0;
  while ((b+1 < reference->n) && equ(x_norm[b], x_norm[b+1]) && (equ(reference->y[b], reference->y[b+1])))
  {
    b = b+1;
  }

  // add down left point
  lx = addNode(lx,(x_norm[b] - tube_x_norm[b]));
  ly = addNode(ly, (reference->y[b] - tube_size->y[b]));

  if (b+1 < reference->n) {
  	  // slopes of reference curve (initialization)
  	  s0 = sign(reference->y[b+1] - reference->y[b]);
  	  if (!equ(x_norm[b+1], x_norm[b])) {
  		  m0 = (reference->y[b+1] - reference->y[b]) / (x_norm[b+1] - x_norm[b]);
  	  } else {
  		  m0 = (s0>0) ? 1e+15 : -1e+15;
  	  }
  	  if equ(s0, 1) {
  		  // add down right point
  		  lx = addNode(lx,(x_norm[b] + tube_x_norm[b]));
  		  ly = addNode(ly, (reference->y[b] - tube_size->y[b]));
  	  }

  	  // ----- 1.2 Iteration: rectangle with center (x,y) = (reference->x[i], reference->y[i]) -----
  	  for (i = b+1; i < reference->n-1; i++) {
  		  // ignore identical points
  		  if (equ(x_norm[i], x_norm[i+1]) && equ(reference->y[i], reference->y[i+1]))
  			  continue;

  		  // slopes of reference curve
  		  s1 = sign(reference->y[i+1] - reference->y[i]);
  		  if (!equ(x_norm[i+1], x_norm[i])) {
  			  m1 = (reference->y[i+1] - reference->y[i]) / (x_norm[i+1] - x_norm[i]);
  		  } else {
  			  m1 = (s1>0) ? (1e+15) : (-1e+15);
  		  }

  		  // add no point for equal slopes of reference curve
  		  if (!equ(m0, m1)) {
  			  if (!equ(s0, -1) && !equ(s1, -1)) {
  				  // add down right point
  				  lx = addNode(lx, (x_norm[i] + tube_x_norm[i]));
  				  ly = addNode(ly, (reference->y[i] - tube_size->y[i]));
  			  } else if (!equ(s0, 1) && !equ(s1, 1)) {
  				  // add down left point
  				  lx = addNode(lx, (x_norm[i] - tube_x_norm[i]));
  				  ly = addNode(ly, (reference->y[i] - tube_size->y[i]));
  			  } else if (equ(s0, -1) && equ(s1, 1)) {
  				  // add down left point
  				  lx = addNode(lx, (x_norm[i] - tube_x_norm[i]));
  				  ly = addNode(ly, (reference->y[i] - tube_size->y[i]));
  				  // add down right point
  				  lx = addNode(lx, (x_norm[i] + tube_x_norm[i]));
  				  ly = addNode(ly, (reference->y[i] - tube_size->y[i]));
  			  } else if (equ(s0, 1) && equ(s1, -1)) {
  				  // add down right point
  				  lx = addNode(lx, (x_norm[i] + tube_x_norm[i]));
  				  ly = addNode(ly, (reference->y[i] - tube_size->y[i]));
  				  // add down left point
  				  lx = addNode(lx, (x_norm[i] - tube_x_norm[i]));
  				  ly = addNode(ly, (reference->y[i] - tube_size->y[i]));
  			  }

  			  int len = listLen(ly);
  			  double lastY = getNth(ly, len-1);
  			  // remove the last added points in case of zero slope of tube curve
  			  if equ((reference->y[i+1] - tube_size->y[i+1]), lastY) {
  				  if (equ(s0 * s1, -1) && equ(getNth(ly, len-3), lastY)) {
  					  // remove two points, if two points were added at last
  					  // ((len-1) - 2 >= 0, because start point + two added points)
  					  lastNodeDeletion(lx);
  					  lastNodeDeletion(ly);
  					  lastNodeDeletion(lx);
  					  lastNodeDeletion(ly);
  				  } else if (!equ(s0 * s1, -1) && equ(getNth(ly, len-2), lastY)) {
  					  // remove one point, if one point was added at last
  					  // ((len-1) - 1 >= 0, because start point + one added point)
  					  lastNodeDeletion(lx);
  					  lastNodeDeletion(ly);
  				  }
  			  }
  		  }
  		  s0 = s1;
  		  m0 = m1;
  	  }
  	  // ----- 1.3. End: Rectangle with center (x,y) = (reference->x[reference->n - 1], reference->y[reference->n - 1]) -----
  	  if equ(s0, -1) {
  		  // add down left point
  		  lx = addNode(lx, (x_norm[reference->n-1] - tube_x_norm[reference->n-1]));
  		  ly = addNode(ly, (reference->y[reference->n-1] - tube_size->y[reference->n-1]));
  	  }
  }
  // add down right point
  lx = addNode(lx, (x_norm[reference->n-1] + tube_x_norm[reference->n-1]));
  ly = addNode(ly, (reference->y[reference->n-1] - tube_size->y[reference->n-1]));

  // ===== 2. Remove points and add intersection points in case of backward order =====
  int lisLen = listLen(ly);
  double* tempLX = malloc(lisLen * sizeof(double));
  if (tempLX == NULL){
  	  fputs("Error: Failed to allocate memory for tempLX.\n", stderr);
      exit(1);
  }
  double* tempLY = malloc(lisLen * sizeof(double));
  if (tempLY == NULL){
  	  fputs("Error: Failed to allocate memory for tempLY.\n", stderr);
      exit(1);
  }

  tempLX = getListValues(lx);
  tempLY = getListValues(ly);
  lower = removeLoop(tempLX, tempLY, lisLen, -1);
  denormalize(lower.x, lower.n, dat_char.mag_x);

  // Free the memory.
  if (x_norm != NULL) free(x_norm);
  if (tube_x_norm != NULL) free(tube_x_norm);

  return lower;
}


/*
 * Function: setUpper
 * ------------------------
 *   find the data set of upper tube curve
 *
 *   reference: reference data curve
 *   tube_size: struct specifying tube size
 *
 *   return : data set defining upper curve of the tube
 */
struct data getUpper(struct data *reference, struct data *tube_size) {
  struct data upper;
  node_t *ux = NULL;
  node_t *uy = NULL;
  size_t i, b;

  /* Normalize values and tube size in x direction.
   * This was introduced in https://github.com/lbl-srg/funnel/pull/30
   * to guard against vanishing derivatives (dy/dx) for x values with a large order of magnitude.
   */

  struct data_char dat_char = get_data_char(reference);                   // Data characteristics
  double *x_norm = (double *)malloc(sizeof(double) * reference->n);       // Normalized x values
  double *tube_x_norm = (double *)malloc(sizeof(double) * tube_size->n);  // Normalized tube size in x direction
  if ((x_norm == NULL) || (tube_x_norm == NULL)){
	  fputs("Error: Failed to allocate memory for x_norm or tube_x_norm.\n", stderr);
    exit(1);
  }
  memcpy(x_norm, reference->x, sizeof(double) * reference->n);
  memcpy(tube_x_norm, tube_size->x, sizeof(double) * tube_size->n);
  normalize(x_norm, reference->n, dat_char.mag_x);
  normalize(tube_x_norm, tube_size->n, dat_char.mag_x);

  // ===== 1. add corner points of the rectangle =====
  double m0, m1; // slopes before and after point i of reference curve
  double s0, s1; // sign of slopes of reference curve: 1 - increasing, 0 - constant, -1 - decreasing

  // ----- 1.1 Start: rectangle with center (x,y) = (reference->x[0], reference->y[0]) -----
  // ignore identical point at the beginning
  b = 0;
  while (((b+1)< reference->n) && equ(x_norm[b], x_norm[b+1]) && equ(reference->y[b], reference->y[b+1]))
  {
    b = b+1;
  }
  // add top left point
  ux = addNode(ux,(x_norm[b] - tube_x_norm[b]));
  uy = addNode(uy, (reference->y[b] + tube_size->y[b]));

  if (b+1 < reference->n) {
	  // slopes of reference curve (initialization)
	  s0 = sign(reference->y[b+1] - reference->y[b]);
	  if (!equ(x_norm[b+1], x_norm[b])) {
		  m0 = (reference->y[b+1] - reference->y[b]) / (x_norm[b+1] - x_norm[b]);
	  } else {
		  m0 = (s0>0) ? 1e+15 : -1e+15;
	  }
	  if equ(s0, -1) {
		  // add top right point
		  ux = addNode(ux, (x_norm[b] + tube_x_norm[b]));
		  uy = addNode(uy, (reference->y[b] + tube_size->y[b]));
	  }

	  // ----- 1.2 Iteration: rectangle with center (x,y) = (x_norm[i], reference->y[i]) -----
	  for (i = b+1; i < reference->n-1; i++) {
		  // ignore identical points
		  if (equ(x_norm[i], x_norm[i+1]) && equ(reference->y[i], reference->y[i+1]))
			  continue;

		  // slopes of reference curve
		  s1 = sign(reference->y[i+1] - reference->y[i]);
		  if (!equ(x_norm[i+1], x_norm[i])) {
			  m1 = (reference->y[i+1] - reference->y[i]) / (x_norm[i+1] - x_norm[i]);
		  } else {
			  m1 = (s1>0) ? (1e+15) : (-1e+15);
		  }

		  // add no point for equal slopes of reference curve
		  if (!equ(m0, m1)) {
			  if (!equ(s0, -1) && !equ(s1, -1)) {
				  // add top left point
				  ux = addNode(ux, (x_norm[i] - tube_x_norm[i]));
				  uy = addNode(uy, (reference->y[i] + tube_size->y[i]));
			  } else if (!equ(s0, 1) && !equ(s1, 1)) {
				  // add top right point
				  ux = addNode(ux, (x_norm[i] + tube_x_norm[i]));
				  uy = addNode(uy, (reference->y[i] + tube_size->y[i]));
			  } else if (equ(s0, 1) && equ(s1, -1)) {
				  // add top left point
				  ux = addNode(ux, (x_norm[i] - tube_x_norm[i]));
				  uy = addNode(uy, (reference->y[i] + tube_size->y[i]));
				  // add top right point
				  ux = addNode(ux, (x_norm[i] + tube_x_norm[i]));
				  uy = addNode(uy, (reference->y[i] + tube_size->y[i]));
			  } else if (equ(s0, -1) && equ(s1, 1)) {
				  // add top right point
				  ux = addNode(ux, (x_norm[i] + tube_x_norm[i]));
				  uy = addNode(uy, (reference->y[i] + tube_size->y[i]));
				  // add top left point
				  ux = addNode(ux, (x_norm[i] - tube_x_norm[i]));
				  uy = addNode(uy, (reference->y[i] + tube_size->y[i]));
			  }

			  int len = listLen(uy);
			  double lastY = getNth(uy, len-1);
			  // remove the last added points in case of zero slope of tube curve
			  if equ((reference->y[i+1] + tube_size->y[i+1]), lastY) {
				  if (equ(s0 * s1, -1) && equ(getNth(uy, len-3), lastY)) {
					  // remove two points, if two points were added at last
					  // ((len-1) - 2 >= 0, because start point + two added points)
					  lastNodeDeletion(ux);
					  lastNodeDeletion(uy);
					  lastNodeDeletion(ux);
					  lastNodeDeletion(uy);
				  } else if (!equ(s0 * s1, -1) && equ(getNth(uy, len-2), lastY)) {
					  // remove one point, if one point was added at last
					  // ((len-1) - 1 >= 0, because start point + one added point)
					  lastNodeDeletion(ux);
					  lastNodeDeletion(uy);
				  }
			  }
		  }
		  s0 = s1;
		  m0 = m1;
	  }
	  // ----- 1.3. End: Rectangle with center (x,y) = (x_norm[reference->n - 1], reference->y[reference->n - 1]) -----
	  if equ(s0, 1) {
		  // add top left point
		  ux = addNode(ux, (x_norm[reference->n-1] - tube_x_norm[reference->n-1]));
		  uy = addNode(uy, (reference->y[reference->n-1] + tube_size->y[reference->n-1]));
	  }
  }
  // add top right point
  ux = addNode(ux, (x_norm[reference->n-1] + tube_x_norm[reference->n-1]));
  uy = addNode(uy, (reference->y[reference->n-1] + tube_size->y[reference->n-1]));

  // ===== 2. Remove points and add intersection points in case of backward order =====
  int lisLen = listLen(uy);
  double* tempUX = malloc(lisLen * sizeof(double));
  if (tempUX == NULL){
	  fputs("Error: Failed to allocate memory for tempUX.\n", stderr);
      exit(1);
  }
  double* tempUY = malloc(lisLen * sizeof(double));
  if (tempUY == NULL){
  	  fputs("Error: Failed to allocate memory for tempUY.\n", stderr);
      exit(1);
  }

  tempUX = getListValues(ux);
  tempUY = getListValues(uy);
  upper = removeLoop(tempUX, tempUY, lisLen, 1);
  denormalize(upper.x, upper.n, dat_char.mag_x);

  // Free the memory.
  if (x_norm != NULL) free(x_norm);
  if (tube_x_norm != NULL) free(tube_x_norm);

  return upper;
}

 /*
  * Function: removeLoop
  * --------------------
  *   remove points and add intersection points in case of backward order
  *
  *   X: x values of curve
  *   Y: y values of curve
  *   size: size of curve array
  *   curInd: if equals to 1, algorithms for upper tube curve is used,
  *           if equals to -1, algorithms for lower tube curve is used
  *
  *   return: data structure including updated curve data sets (X, Y, size)
  */
 struct data removeLoop(double* X, double* Y, int size, int curInd) {
   struct data output;
   int j = 1;
   int countLoops = 0;
   int re_size = size;

   while (j < re_size -2) {
     // Find backward segment (j, j+1)
     if (X[j+1] < X[j]) {

       countLoops = countLoops + 1;
       // ===== 1. Find i, k, such that i <= j<j+1 <= k-1 and segment (i-1, i) intersect segment (k-1, k) =====
       int i, k, iPrevious;
       double y;
       // for calculation and adding of intersection point
       bool addPoint = true;
       double ix = 0;
       double iy = 0;
       int kMax;

       i = j;
       iPrevious = i;

       // Find initial value for i = i_s, such that X[i_s-1]  <= X[j+1] < X[i_s]
       // it holds: i element of interval (i_s, j)
       while (X[j+1] < X[i-1])
         i = i-1;
       // j+1 < k <= kMax
       kMax = j+1;
       while (X[kMax] < X[j] && kMax < re_size-1)  //=============================re_size
         kMax = kMax+1;

       // initial value for k
       k = j+1;
       y = Y[i-1];

       // Find k
       while (((curInd==-1 && y < Y[k]) || (curInd==1 && Y[k] < y))
           && k < kMax) {
         iPrevious = i;
         k = k+1;
         while ((X[i] < X[k]
                  || (curInd==-1 && equ(X[i], X[k]) && Y[i] < Y[k] && !(k + 1 < re_size && equ(X[k], X[k + 1]) && Y[k + 1] < Y[k]))
                  || (curInd==1 && equ(X[i], X[k]) && Y[i] > Y[k] && !(k + 1 < re_size && equ(X[k], X[k + 1]) && Y[k + 1] > Y[k])))
             && i < j)
           i = i+1;
         // it holds X[i - 1] < X[k] <= X[i], particularly X[i] != X[i - 1]
         // for i < j and X[i - 1] < X[k] it holds X[i - 1] < X[k] <= X[i], particularly X[i] != X[i - 1]
         // linear interpolation of (x, y) = (X[k], y) on segment (i - 1, i)
         if (!equ(X[i], X[i - 1]))
           y = (Y[i] - Y[i - 1]) / (X[i] - X[i - 1]) * (X[k] - X[i - 1]) + Y[i - 1];
         else
           y = Y[i];
       }

       // k located: intersection point is on segment (k - 1, k)
       // i approximately located: intersection point is on polygonal line (iPrevoius - 1, i)
       // Regular case
       if (iPrevious > 1)
         i = iPrevious - 1;
       // Special case handling: assure, that i - 1 >= 0
       else
         i = iPrevious;
       if (!equ(X[k], X[k - 1]))
           // linear interpolation of (x, y) = (X[i], y) on segment (k - 1, k)
         y = (Y[k] - Y[k - 1]) / (X[k] - X[k - 1]) * (X[i] - X[k - 1]) + Y[k - 1];
       // it holds Y[i] = Y[iPrevious - 1] < Y[k - 1]
       // Find i
       while ((!equ(X[k], X[k - 1])
                   && ((curInd==-1 && Y[i] < y) || (curInd==1 && y < Y[i])))
           || (equ(X[k], X[k - 1]) && X[i] < X[k]))
       {
         i = i+1;
           if (!equ(X[k], X[k - 1]))
             // linear interpolation of (x, y) = (X[i], y) on segment (k - 1, k)
               y = (Y[k] - Y[k - 1]) / (X[k] - X[k - 1]) * (X[i] - X[k - 1]) + Y[k - 1];
       }

       // ===== 2. Calculate intersection point (ix, iy) of segments (i - 1, i) and (k - 1, k) =====
       double a1 = 0;
       double a2 = 0;

       // both branches vertical
       if (equ(X[i], X[i - 1]) && equ(X[k], X[k - 1]))
         // add no point; check if case occur: slopes have different signs
         addPoint = false;
       // case i-branch vertical
       else if equ(X[i], X[i - 1]) {
         ix = X[i];
         iy = Y[k - 1] + ((X[i] - X[k - 1]) * (Y[k] - Y[k - 1])) / (X[k] - X[k - 1]);
       }
       // case k-branch vertical
       else if equ(X[k], X[k - 1]) {
         ix = X[k];
         iy = Y[i - 1] + ((X[k] - X[i - 1]) * (Y[i] - Y[i - 1])) / (X[i] - X[i - 1]);
       }
       // common case
       else {
         a1 = (Y[i] - Y[i - 1]) / (X[i] - X[i - 1]); // slope of segment (i - 1, i)
         a2 = (Y[k] - Y[k - 1]) / (X[k] - X[k - 1]); // slope of segment (k - 1, k)
         // common case: no equal slopes
         if (!equ(a1, a2)) {
           ix = (a1 * X[i - 1] - a2 * X[k - 1] - Y[i - 1] + Y[k - 1]) / (a1 - a2);
           if (fabs(a1) > fabs(a2))
             // calculate y on segment (k - 1, k)
             iy = a2 * (ix - X[k - 1]) + Y[k - 1];
           else
             // calculate y on segment (i - 1, i)
             iy = a1 * (ix - X[i - 1]) + Y[i - 1];
         }
         else
           // case equal slopes: add no point
           addPoint = false;
       }

       // ===== 3. Delete points i until (including) k-1 =====
       int count = k-i;
       double* XX = malloc((re_size-count) * sizeof(double));
       if (XX == NULL){
    	   fputs("Error: Failed to allocate memory for XX.\n", stderr);
           exit(1);
       }
       XX = removeRange(X, re_size, i, count);
       double* YY = malloc((size-count) * sizeof(double));
       if (YY == NULL){
           fputs("Error: Failed to allocate memory for YY.\n", stderr);
           exit(1);
       }
       YY = removeRange(Y, re_size, i, count);
       re_size = re_size-count;
       // ===== 4. Add intersection point =====
       // add intersection point, if it isn't already there
       if (addPoint && (!equ(XX[i], ix) || !equ(YY[i], iy))) {
         re_size = re_size+1;
         double *X_temp = malloc(re_size * sizeof(double));
         if (X_temp == NULL){
        	 fputs("Error: Failed to allocate memory for X_temp.\n", stderr);
             exit(1);
         }
         double *Y_temp = malloc(re_size * sizeof(double));
         if (Y_temp == NULL){
        	 fputs("Error: Failed to allocate memory for Y_temp.\n", stderr);
        	 exit(1);
         }
         X_temp = insertAt(XX, re_size-1, i, ix);
         Y_temp = insertAt(YY, re_size-1, i, iy);

         XX = realloc(XX, sizeof(double)*re_size);
         if (XX == NULL){
        	 fputs("Error: Failed to reallocate memory for XX.\n", stderr);
        	 exit(1);
         }
         YY = realloc(YY, sizeof(double)*re_size);
         if (YY == NULL){
        	 fputs("Error: Failed to reallocate memory for YY.\n", stderr);
        	 exit(1);
         }
         XX = X_temp;
         YY = Y_temp;
       }

       // ===== 5. set j = i =====
       j = i;

       // ===== 6. Delete points that are doubled =====
       if (equ(XX[i-1], XX[i]) && equ(YY[i-1], YY[i])) {
         re_size = re_size-1;
         double *X_temp = malloc(re_size * sizeof(double));
         if (X_temp == NULL){
        	 fputs("Error: Failed to allocate memory for X_temp.\n", stderr);
        	 exit(1);
         }
         double *Y_temp = malloc(re_size * sizeof(double));
         if (Y_temp == NULL){
        	 fputs("Error: Failed to allocate memory for Y_temp.\n", stderr);
        	 exit(1);
         }
         X_temp = removeAt(XX, re_size+1, i);
         Y_temp = removeAt(YY, re_size+1, i);

         XX = realloc(XX, sizeof(double)*re_size);
         if (XX == NULL){
        	 fputs("Error: Failed to reallocate memory for XX.\n", stderr);
        	 exit(1);
         }
         YY = realloc(YY, sizeof(double)*re_size);
         if (YY == NULL){
        	 fputs("Error: Failed to reallocate memory for YY.\n", stderr);
        	 exit(1);
         }
         XX = X_temp;
         YY = Y_temp;
         j = i - 1;
       }
       X = realloc(X, sizeof(double)*re_size);
       if (X == NULL){
    	   fputs("Error: Failed to reallocate memory for X.\n", stderr);
    	   exit(1);
       }
       Y = realloc(Y, sizeof(double)*re_size);
       if (Y == NULL){
    	   fputs("Error: Failed to reallocate memory for Y.\n", stderr);
    	   exit(1);
       }
       X = XX;
       Y = YY;
     }
     j=j+1;
   }
   output.x = X;
   output.y = Y;
   output.n = re_size;
   return output;
 }


/*
 * Function: removeRange
 * ---------------------
 *   remove a range of elements from array
 *
 *   array: original array
 *   size: original array size
 *   staInd: zero-based starting index of the range of elements to remove
 *   count: the number of elements to remove
 *
 *   return: updated array with specified length of elements being removed
 */
double * removeRange(double* array, int size, int staInd, int count) {
  int i;
  double* updArr = malloc((size-count) * sizeof(double));
  if (updArr == NULL){
	  fputs("Error: Failed to allocate memory for updArr.\n", stderr);
	  exit(1);
  }
  if (!((staInd+count) <= size)) {
    fputs("Deletion not possible!\n", stderr);
    exit(1);
  } else {
    for (i=0; i<staInd; i++) {
      updArr[i] = array[i];
    }
    if (staInd+count != size) {
      for (i=staInd; i<(size-count); i++) {
        updArr[i] = array[i+count];
      }
    }
  }
  return updArr;
}


/*
 * Function: removeAt
 * ------------------
 *   remove element at the specified index from array
 *
 *   array: original array
 *   size: original array size
 *   ind: zero-based index of the element to remove
 *
 *   return: updated array with specified element being removed
 */
double * removeAt(double* array, int size, int ind) {
  int i;
  double* updArr = malloc((size-1) * sizeof(double));
  if (updArr == NULL){
  	  fputs("Error: Failed to allocate memory for updArr.\n", stderr);
  	  exit(1);
  }
  if (ind > size-1) {
    fputs("Deletion not possible!\n", stderr);
    exit(1);
  } else {
    if (ind == size-1) {
      for (i=0; i<size-1; i++) {
        updArr[i] = array[i];
      }
    } else {
      for (i=0; i<ind; i++) {
        updArr[i] = array[i];
      }
      for (i=ind; i<size-1; i++) {
        updArr[i] = array[i+1];
      }
    }
  }
  return updArr;
}


/*
 * Function: insertAt
 * ------------------
 *   insert element into the array at the specified index
 *
 *   array: original array
 *   size: original array size
 *   index: zero-based index at which item should be inserted
 *   item: the object to insert
 *
 *   return: updated array with new element being added at specified position
 */
double * insertAt(double* array, int size, int index, double item) {
  int i;
  double* updArr = malloc((size+1) * sizeof(double));
  if (updArr == NULL){
	  fputs("Error: Failed to allocate memory for updArr.\n", stderr);
	  exit(1);
  }
  if (index > size) {
    fputs("Insert not possible!\n", stderr);
    exit(1);
  } else {
    if (index == size) {
      for (i=0; i<size; i++) {
        updArr[i] = array[i];
      }
      updArr[size] = item;
    } else {
      for (i=0; i<index; i++) {
        updArr[i] = array[i];
      }
      updArr[index] = item;
      for (i=index; i<size; i++) {
        updArr[i+1] = array[i];
      }
    }
  }
  return updArr;
}
