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
	{"axes", 'x', "AXES", 0, "Check if the tolerance value is set for the half-width (X) or half-height (Y) of the rectangle to generate tube, default=Y"},
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
 * Function: write_to_file
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
int write_to_file(struct sumData data, char const *outDir, char const *refData, char const *testData, char const *lowerData, char const *upperData, char const *report) {
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
    fputs("Error: Failed to open file in write_to_file.\n", stderr);
    return -1;
  }
	int i = 0;
	while (i < data.refData.size) {
		fprintf(f1, "%lf,%lf\n", data.refData.X[i],data.refData.Y[i]);
		i++;
	}
	int j = 0;
	while (j < data.testData.size) {
		fprintf(f2, "%lf,%lf\n", data.testData.X[j], data.testData.Y[j]);
		j++;
	}
	int k = 0;
	while (k < data.lowerCurve.size) {
		fprintf(f3, "%lf,%lf\n", data.lowerCurve.X[k],data.lowerCurve.Y[k]);
		k++;
	}
	int l = 0;
	while (l < data.upperCurve.size) {
		fprintf(f4, "%lf,%lf\n", data.upperCurve.X[l],data.upperCurve.Y[l]);
		l++;
	}
	fprintf(f5, "The test result is %s.\n", data.validateReport.valid);
	if (data.validateReport.errors.size != 0) {
		fprintf(f5, "There are errors at %d points.\n", data.validateReport.errors.size);
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


int main(int argc, char *argv[]) {
  int exiVal; // Exit value

	struct arguments arguments;
	// Default values
	arguments.toleranceFlag = false;
	arguments.tolerance = 0.002;
	arguments.axes = 'Y';
	arguments.useRelativeTolerance = true;
	arguments.outputFlag = false;

	// Parse arguments
	argp_parse (&argp, argc, argv, 0, 0, &arguments);

	// Read CSV files into data structure
	struct data testCSV = readCSV(arguments.testFile, 1);
	struct data baseCSV = readCSV(arguments.baseFile, 1);

	// Calculate tube size (half-width and half-height of rectangle)
  //printf("useRelative=%d\n", arguments.useRelativeTolerance);
	double* tube = tubeSize(baseCSV, arguments.tolerance, arguments.axes, 0, 0, arguments.useRelativeTolerance);

	// Calculate the data set of lower and upper curve around base
	struct data lowerCurve = calculateLower(baseCSV, tube);
	struct data upperCurve = calculateUpper(baseCSV, tube);

	// Validate test curve and generate error report
	struct reports validateReport = validate(lowerCurve, upperCurve, testCSV);

	// Summarize the results
	struct sumData sumReport;
	sumReport.refData = baseCSV;
	sumReport.lowerCurve = lowerCurve;
	sumReport.upperCurve = upperCurve;
	sumReport.testData = testCSV;
	sumReport.validateReport = validateReport;

	exiVal = write_to_file(sumReport, arguments.output, "refData.csv", "testData.csv", "lowerData.csv", "upperData.csv", "report.csv");
  return exiVal;
}
