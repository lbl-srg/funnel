#include <math.h>
#include "compare.h"

#ifndef equ
#define equ(a,b) (fabs((a)-(b)) < 1e-10 ? true : false)  /* (b) required by Win32 compiler for <0 values */
#endif

/*
 * Function: buildPath
 * -----------------------
 *   constructs a file path
 *
 *   outDir: directory of file
 *   fileName: file name
 */

char *buildPath(
  const char *outDir,
  const char *fileName
) {
  const char lastChar = outDir[(strlen(outDir)-1)];
  #ifdef _WIN32
  /* Windows supports forward and backward slash */
    bool addSlash = (lastChar == '/' || lastChar == '\\') ? false : true;
  #else
    bool addSlash = (lastChar == '/') ? false : true;
  #endif

  char *fname = NULL;
  if (addSlash)
    fname = (char*)malloc((strlen(outDir) + strlen(fileName) + 2) * sizeof(char));
  else
    fname = (char*)malloc((strlen(outDir) + strlen(fileName) + 1) * sizeof(char));

  if (fname == NULL){
    perror("Error: Failed to allocate memory for fname in writeToFile.");
  }

  strcpy(fname, outDir);

  #ifdef _WIN32
  if (addSlash)
    strcat(fname, "\\");
  #else
  if (addSlash)
    strcat(fname, "/");
  #endif

  strcat(fname, fileName);

  return fname;
}

/*
 * Function: init_log
 * -----------------------
 *   opens a file for logging the numerical processing errors
 *   (all other errors like memory, file access, bad argument...
 *   are still output to stderr.)
 *
 *   outDir: directory of logging file
 *   fileName: logging file name
 */

FILE *init_log(
  const char *outDir,
  const char *fileName
) {
  char *fname = buildPath(outDir, fileName);
  FILE *fil = fopen(fname, "w+");
  if (fname != NULL) free(fname);

  if (fil == NULL){
    perror("Error: Failed to open log.\n");
  }

  return fil;
}

/*
 * Function: writeToFile
 * -----------------------
 *   write input data structure to files
 *
 *   outDir: directory to save the output files
 *   fileName: file name for storing base CSV data
 *   data: data to be written
 */

int writeToFile(
  const char *outDir,
  const char *fileName,
  struct data *data
) {
  size_t i = 0;

  char *fname = buildPath(outDir, fileName);
  FILE *fil = fopen(fname, "w+");
  if (fname != NULL) free(fname);

  if (fil == NULL){
    fprintf(log_file, "Error: Failed to open '%s' in writeToFile.\n", fname);
    return -1;
  }

  fprintf(fil, "%s\n", "x,y");
  for (i = 0; i < data->n; i++) {
    fprintf(fil, "%lf,%lf\n", data->x[i], data->y[i]);
  }

  fclose(fil);

  return 0;
}

struct data *newData(
  size_t n
) {
  struct data *retVal = malloc(sizeof(struct data));
  if (retVal == NULL)
  {
    fputs("Error: Failed to allocate memory for data.\n", log_file);
    return NULL;
  }
  // Try to allocate vector data, free structure if fail.

  retVal->x = malloc(n * sizeof(double));
  if (retVal->x == NULL) {
    fputs("Error: Failed to allocate memory for data.x.\n", log_file);
    free (retVal);
    return NULL;
  }

  retVal->y = malloc(n * sizeof(double));
  if (retVal->y == NULL) {
    fputs("Error: Failed to allocate memory for data.y.\n", log_file);
    free (retVal->x);
    free (retVal);
    return NULL;
  }

  // Set size and return.
  retVal->n = n;
  return retVal;
}

void setData(
  struct data *dat,
  const double x[],
  const double y[]
) {
  // size_t lentgh_x = sizeof(x) / sizeof(x[0]);
  // size_t lentgh_y = sizeof(y) / sizeof(y[0]);
  // if ((dat->n != lentgh_x) || (dat->n != lentgh_y)) {
  //   fputs("Error: data struct size and array lengths are different.\n", log_file);
  // }
  if (dat != NULL) {
    memcpy(dat->x, x, sizeof(double) * dat->n);
    memcpy(dat->y, y, sizeof(double) * dat->n);
  } else {
    fputs("Error: Cannot set data for unallocated struct.\n", log_file);
  }
}

void freeData(struct data *dat) {
  if (dat != NULL) {
    if (dat->x != NULL) free (dat->x);
    if (dat->y != NULL) free (dat->y);
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
  const double *tReference,
  const double *yReference,
  const size_t nReference,
  const double *tTest,
  const double *yTest,
  const size_t nTest,
  const char *outputDirectory,
  const double atolx,
  const double atoly,
  const double ltolx,
  const double ltoly,
  const double rtolx,
  const double rtoly
) {
  int retVal;
  int rc_mkdir = mkdir_p(outputDirectory);
  struct data *baseCSV = newData(nReference);
  struct data *testCSV = newData(nTest);
  struct data *tube_size = newData(nReference);
  setData(baseCSV, tReference, yReference);
  setData(testCSV, tTest, yTest);

  if (rc_mkdir != 0) {
    fprintf(stderr, "Error: Failed to create directory: %s\n", outputDirectory);
    return -1;
  }
  log_file = init_log(outputDirectory, "c_funnel.log");

  if (!equ(baseCSV->x[0], testCSV->x[0])){
    fprintf(log_file, "Error: Reference and test data minimum x values are different.\n");
    retVal = 1;
    goto end;
  }
  if (!equ(baseCSV->x[baseCSV->n - 1], testCSV->x[testCSV->n - 1])){
    fprintf(log_file, "Error: Reference and test data maximum x values are different.\n");
    retVal = 1;
    goto end;
  }

  struct tolerances tolerances = {
    .atolx = atolx,
    .atoly = atoly,
    .ltolx = ltolx,
    .ltoly = ltoly,
    .rtolx = rtolx,
    .rtoly = rtoly,
  };
  // Compute tube size.
  set_tube_size(tube_size, baseCSV, tolerances);

  // Calculate values of lower and upper curve around base
  struct data lowerCurve = getLower(baseCSV, tube_size);
  struct data upperCurve = getUpper(baseCSV, tube_size);

  // Validate test curve and generate error report
  if (lowerCurve.n == 0 || lowerCurve.n == 0){
    fputs("Error: lower or upper curve has 0 elements.\n", log_file);
    retVal = 1;
    goto end;
  }
  struct reports validateReport;

  retVal = validate(lowerCurve, upperCurve, *testCSV, &validateReport.errors);
  if (retVal != 0){
    fputs("Error: Failed to run validate function.\n", log_file);
    goto end;
  }

  /* Write data to files */
  retVal = writeToFile(outputDirectory, "reference.csv", baseCSV);
  if (retVal != 0){
    fputs("Error: Failed to write reference.csv in output directory.\n", log_file);
    goto end;
  }
  retVal = writeToFile(outputDirectory, "lowerBound.csv", &lowerCurve);
  if (retVal != 0){
    fputs("Error: Failed to write lowerBound.csv in output directory.\n", log_file);
    goto end;
  }
  retVal = writeToFile(outputDirectory, "upperBound.csv", &upperCurve);
  if (retVal != 0){
    fputs("Error: Failed to write upperBound.csv in output directory.\n", log_file);
    goto end;
  }
  retVal = writeToFile(outputDirectory, "test.csv", testCSV);
  if (retVal != 0){
    fputs("Error: Failed to write test.csv in output directory.\n", log_file);
    goto end;
  }
  retVal = writeToFile(outputDirectory, "errors.csv", &validateReport.errors.diff);
  if (retVal != 0){
    fputs("Error: Failed to write errors.csv in output directory.\n", log_file);
    goto end;
  }

  end:
    freeData(baseCSV);
    freeData(testCSV);
    freeData(tube_size);
    fclose(log_file);
    return retVal;
}
