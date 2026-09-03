#include <math.h>
#include "compare.h"

/*
*   Descriptor of the file used for logging the numerical processing errors
*   (all other errors like memory, file access, bad argument...
*   are still output to stderr.)
*/
FILE *log_file;

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
    perror("Error: Failed to allocate memory for fname in buildPath.");
    return NULL;
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
  if (fname == NULL) return NULL;
  FILE *fil = fopen(fname, "w+");
  free(fname);

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
  if (fname == NULL) return -1;
  FILE *fil = fopen(fname, "w+");

  if (fil == NULL){
    /* fname used to be freed before it was printed here. */
    fprintf(log_file, "Error: Failed to open '%s' in writeToFile.\n", fname);
    free(fname);
    return -1;
  }
  free(fname);

  fprintf(fil, "%s\n", "x,y");
  for (i = 0; i < data->n; i++) {
    fprintf(fil, "%.16g,%.16g\n", data->x[i], data->y[i]);
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
    fputs("Error: Failed to allocate memory for data.\n", stderr);
    return NULL;
  }
  // Try to allocate vector data, free structure if fail.

  retVal->x = malloc(n * sizeof(double));
  if (retVal->x == NULL) {
    fputs("Error: Failed to allocate memory for data.x.\n", stderr);
    free (retVal);
    return NULL;
  }

  retVal->y = malloc(n * sizeof(double));
  if (retVal->y == NULL) {
    fputs("Error: Failed to allocate memory for data.y.\n", stderr);
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
  if (dat != NULL) {
    memcpy(dat->x, x, sizeof(double) * dat->n);
    memcpy(dat->y, y, sizeof(double) * dat->n);
  } else {
    fputs("Error: Cannot set data for unallocated struct.\n", log_file);
  }
}

void freeData(struct data *dat) {
  if (dat == NULL) return;
  if (dat->x != NULL) free (dat->x);
  if (dat->y != NULL) free (dat->y);
  free (dat);
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
  /* Declared here, and zeroed, so that the cleanup at `end` can free them
     whichever `goto end` was taken. */
  struct data lowerCurve = {NULL, NULL, 0};
  struct data upperCurve = {NULL, NULL, 0};
  struct reports validateReport = {{{NULL, NULL, 0}, {NULL, NULL, 0}}};
  if (baseCSV == NULL || testCSV == NULL || tube_size == NULL) {
    fputs("Error: Failed to allocate memory for the input data.\n", stderr);
    freeData(baseCSV);
    freeData(testCSV);
    freeData(tube_size);
    return -1;
  }
  setData(baseCSV, tReference, yReference);
  setData(testCSV, tTest, yTest);

  if (rc_mkdir != 0) {
    fprintf(stderr, "Error: Failed to create directory: %s\n", outputDirectory);
    return -1;
  }
  log_file = init_log(outputDirectory, "c_funnel.log");
  if (log_file == NULL) {
    /* Losing the log is not a reason to lose the comparison, but every
       later fprintf() would dereference NULL. */
    fputs("Error: Failed to open the log file; logging to stderr.\n", stderr);
    log_file = stderr;
  }

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
  lowerCurve = getLower(baseCSV, tube_size);
  upperCurve = getUpper(baseCSV, tube_size);

  // Validate test curve and generate error report
  if (lowerCurve.n == 0 || upperCurve.n == 0){
    fputs("Error: lower or upper curve has 0 elements.\n", log_file);
    retVal = 1;
    goto end;
  }
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
    /* getLower/getUpper and validate hand back heap arrays that nothing else
       owns; without these the whole per-call working set stayed allocated for
       the lifetime of the process that loaded the library. */
    if (lowerCurve.x != NULL) free(lowerCurve.x);
    if (lowerCurve.y != NULL) free(lowerCurve.y);
    if (upperCurve.x != NULL) free(upperCurve.x);
    if (upperCurve.y != NULL) free(upperCurve.y);
    if (validateReport.errors.original.x != NULL) free(validateReport.errors.original.x);
    if (validateReport.errors.original.y != NULL) free(validateReport.errors.original.y);
    if (validateReport.errors.diff.x != NULL) free(validateReport.errors.diff.x);
    if (validateReport.errors.diff.y != NULL) free(validateReport.errors.diff.y);
    if (log_file != stderr) fclose(log_file);
    return retVal;
}
