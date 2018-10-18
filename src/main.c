/*
 * main.c
 *
 *  Created on: Apr 4, 2018
 *      Author: jianjun
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>
#include "stdbool.h"

#include "data_structure.h"
#include "readCSV.h"
#include "algorithmRectangle.h"
#include "tube.h"
#include "tubeSize.h"
#include "mkdir_p.h"

#define MAX 100

/* Used by main to communicate with parse_opt */
struct arguments {
    bool toleranceFlag;
    double tolerance;
    char axes;
    bool useRelativeTolerance;
    bool outputFlag;
    char *output;
    char *testFile;
    char *baseFile;
};

/* Input options */
static struct argp_option options[] = {
  /* name, key, argname, flags, doc, group */
  {"tolerance", 't', "TOLERANCE", 0, "Tolerance to generate data tube, default=0.002"},
  {"axes", 'x', "AXES", 0, "Check if the tolerance value is set for the half-width (x) or half-height (y) of the rectangle to generate tube, default=y"},
  {"absolute", 'a', 0, 0, "If specified, absolute tolerance is used."},
  {"outputFile", 'o', "DIR", 0, "Directory to save outputs."},
  {"compareFile", 'c', "PATH", 0, "Name of CSV file to be tested."},
  {"baseFile", 'b', "PATH", 0, "Name of CSV file to be used as the reference."},
  { 0 }
};

/* Parse a single option */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input; // get input argument from argp_parse
  switch(key) {
  case 'a':
    arguments->useRelativeTolerance = false;
    break;
  case 'b':
    arguments->baseFile = arg;
    break;
  case 'c':
    arguments->testFile = arg;
    break;
  case 'o':
    arguments->outputFlag = true;
    arguments->output = arg;
    break;
  case 't':
    arguments->toleranceFlag = true;
    arguments->tolerance = atof(arg);
    break;
  case 'x':
    arguments->axes = arg[0];
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

/* Argp parser */
static struct argp argp = { options, parse_opt, 0, 0 };

/*
 * Function: writeToFile
 * -----------------------
 *   write input data structure to files
 *
 *   data: input data structure containing original CSV data, tube curve data, and validate report
 *   outDir: directory to save the output files
 *   refData: file name for storing base CSV data
 *   testData: file name for storing test CSV data
 *   lowerData: file name for storing tube lower curve data
 *   upperData: file name for storing tube upper curve data
 *   report: file name for validate report
 */
int writeToFile(
  struct sumData data,
  const char *outDir,
  const char *refData,
  const char *testData,
  const char *lowerData,
  const char *upperData,
  const char *report) {

  mkdir_p(outDir);

  char refDataFile[MAX], testDataFile[MAX], lowDataFile[MAX], upperDataFile[MAX], reportFile[MAX];

  strcpy(refDataFile, outDir);
  strcpy(testDataFile, outDir);
  strcpy(lowDataFile, outDir);
  strcpy(upperDataFile, outDir);
  strcpy(reportFile, outDir);
  strcat(refDataFile, refData);
  strcat(testDataFile, testData);
  strcat(lowDataFile, lowerData);
  strcat(upperDataFile, upperData);
  strcat(reportFile, report);

  FILE *f1 = fopen(refDataFile, "w+");
  FILE *f2 = fopen(testDataFile, "w+");
  FILE *f3 = fopen(lowDataFile, "w+");
  FILE *f4 = fopen(upperDataFile, "w+");
  FILE *f5 = fopen(reportFile, "w+");

  if (f1 == NULL || f2 == NULL || f3 == NULL || f4 == NULL || f5 == NULL){
    fputs("Error: Failed to open file in writeToFile.\n", stderr);
    return -1;
  }
  int i = 0;
  while (i < data.refData.n) {
    fprintf(f1, "%lf,%lf\n", data.refData.x[i],data.refData.y[i]);
    i++;
  }
  int j = 0;
  while (j < data.testData.n) {
    fprintf(f2, "%lf,%lf\n", data.testData.x[j], data.testData.y[j]);
    j++;
  }
  int k = 0;
  while (k < data.lowerCurve.n) {
    fprintf(f3, "%lf,%lf\n", data.lowerCurve.x[k],data.lowerCurve.y[k]);
    k++;
  }
  int l = 0;
  while (l < data.upperCurve.n) {
    fprintf(f4, "%lf,%lf\n", data.upperCurve.x[l],data.upperCurve.y[l]);
    l++;
  }
  fprintf(f5, "The test result is %s.\n", data.validateReport.valid);
  if (data.validateReport.errors.n != 0) {
    fprintf(f5, "There are errors at %zu points.\n", data.validateReport.errors.n);
    int m = 0;
    while (m < data.validateReport.errors.diffSize) {
      fprintf(f5, "%lf,%lf\n",
          data.validateReport.errors.diffX[m],data.validateReport.errors.diffY[m]);
      m++;
    }
  }
  fclose(f1);
  fclose(f2);
  fclose(f3);
  fclose(f4);
  fclose(f5);
  return 0;
}

struct data *newData(
  const double x[],
  const double y[],
  size_t n) {

  struct data *retVal = malloc (sizeof (struct data));
  if (retVal == NULL){
    fputs("Error: Failed to allocate memory for data.\n", stderr);
    return NULL;
  }
  // Try to allocate vector data, free structure if fail.

  retVal->x = malloc (n * sizeof (double));
  if (retVal->x == NULL) {
    fputs("Error: Failed to allocate memory for data.x.\n", stderr);
    free (retVal);
    return NULL;
  }
  memcpy(retVal->x, x, sizeof(double)*n);

  retVal->y = malloc (n * sizeof (double));
  if (retVal->y == NULL) {
    fputs("Error: Failed to allocate memory for data.y.\n", stderr);
    free (retVal->x);
    free (retVal);
    return NULL;
  }
  memcpy(retVal->y, y, sizeof(double)*n);

  // Set size and return.
  retVal->n = n;
  return retVal;
}

void freeData (struct data *dat) {
  if (dat != NULL) {
      free (dat->x);
      free (dat->y);
      free (dat);
  }
}

/*
 * Function: compareAndReport
 * -----------------------
 *   This function does the actual computations. It is introduced so that it
 *   can be called from Python in which case the argument parsing of main
 *   is not needed.
 */
int compareAndReport(
  const double tReference[],
  const double yReference[],
  const size_t nReference,
  const double tTest[],
  const double yTest[],
  const size_t nTest,
  const char * outputDirectory,
  const double tolerance,
  const char axes,
  const bool useRelativeTolerance){

  int retVal;

  struct data * baseCSV = newData(tReference, yReference, nReference);
  struct data * testCSV = newData(tTest, yTest, nTest);

  // Calculate tube size (half-width and half-height of rectangle)
  //printf("useRelative=%d\n", arguments.useRelativeTolerance);
  double* tube = tubeSize(*baseCSV, tolerance, axes, 0, 0, useRelativeTolerance);

  // Calculate the data set of lower and upper curve around base
  struct data lowerCurve = calculateLower(*baseCSV, tube);
  struct data upperCurve = calculateUpper(*baseCSV, tube);

  // Validate test curve and generate error report
  struct reports validateReport = validate(lowerCurve, upperCurve, *testCSV);

  // Summarize the results
  struct sumData sumReport;
  sumReport.refData = *baseCSV;
  sumReport.lowerCurve = lowerCurve;
  sumReport.upperCurve = upperCurve;
  sumReport.testData = *testCSV;
  sumReport.validateReport = validateReport;

  retVal = writeToFile(sumReport, outputDirectory, "refData.csv", "testData.csv", "lowerData.csv", "upperData.csv", "report.csv");
  freeData(baseCSV);
  freeData(testCSV);

  return retVal;
}

int main(int argc, char *argv[]) {
  int exiVal; // Exit value

  struct arguments arguments;
  // Default values
  arguments.toleranceFlag = false;
  arguments.tolerance = 0.002;
  arguments.axes = 'y';
  arguments.useRelativeTolerance = true;
  arguments.outputFlag = false;

  // Parse arguments
  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  struct data baseCSV = readCSV(arguments.baseFile, 1);
  struct data testCSV = readCSV(arguments.testFile, 1);

  exiVal = compareAndReport(
    baseCSV.x,
    baseCSV.y,
    baseCSV.n,
    testCSV.x,
    testCSV.y,
    testCSV.n,
    arguments.output,
    arguments.tolerance,
    arguments.axes,
    arguments.useRelativeTolerance);

  return exiVal;
}
