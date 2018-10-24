#include "compare.h"

#include "mkdir_p.h"

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
  int i = 0;

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

  for (i = 0; i < data.refData.n; i++) {
    fprintf(f1, "%lf,%lf\n", data.refData.x[i],data.refData.y[i]);
  }
  for (i = 0; i < data.testData.n; i++) {
    fprintf(f2, "%lf,%lf\n", data.testData.x[i], data.testData.y[i]);
  }
  for (i = 0; i < data.lowerCurve.n; i++) {
    fprintf(f3, "%lf,%lf\n", data.lowerCurve.x[i],data.lowerCurve.y[i]);
  }
  for (i = 0; i < data.upperCurve.n; i++) {
    fprintf(f4, "%lf,%lf\n", data.upperCurve.x[i],data.upperCurve.y[i]);
  }
  if (data.validateReport.errors.diff.n != 0) {
    fprintf(f5, "The test result is invalid.\n");
    fprintf(f5, "There are errors at %zu points.\n", data.validateReport.errors.diff.n);
    for (i =0; i < data.validateReport.errors.diff.n; i++){
      fprintf(f5, "%lf,%lf\n",
          data.validateReport.errors.diff.x[i],
          data.validateReport.errors.diff.y[i]);
    }
  }
  else
    fprintf(f5, "The test result is valid.\n");

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
  const double* tReference,
  const double* yReference,
  const size_t nReference,
  const double* tTest,
  const double* yTest,
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
  if (lowerCurve.n == 0 || upperCurve.n == 0){
    fputs("Error: lower or upper curve has 0 elements.\n", stderr);
    return 1;
  }
  struct reports validateReport;
  retVal = validate(lowerCurve, upperCurve, *testCSV, &validateReport.errors);
  if (retVal != 0)
    return retVal;

  // Summarize the results
  struct sumData sumReport;
  sumReport.refData = *baseCSV;
  sumReport.lowerCurve = lowerCurve;
  sumReport.upperCurve = upperCurve;
  sumReport.testData = *testCSV;
  sumReport.validateReport = validateReport;
  retVal = writeToFile(sumReport, outputDirectory,
    "refData.csv", "testData.csv", "lowerData.csv", "upperData.csv", "report.csv");
  freeData(baseCSV);
  freeData(testCSV);

  return retVal;
}
