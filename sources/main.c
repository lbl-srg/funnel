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

/* Used by main to communicate with parse_opt */
struct arguments {
    bool toleranceFlag;
    double tolerance;
    char axes;
    bool absoluteFlag;
    bool outputFlag;
    char *output;
    char *testFile;
    char *baseFile;
};

/* Input options */
static struct argp_option options[] = {
    /* name, key, argname, flags, doc, group */
    {"tolerance", 't', "TOLERANCE", 0, "Tolerance to generate data tube"},
	{"axes", 'x', "AXES", 0, "Check if the tolerance value is set for half-width or half-height of tube"},
	{"absolute", 'a', 0, 0, "Set to absolute tolerance"},
	{"outputFile", 'o', "DIR", 0, "Output directory"},
	{"compareFile", 'c', "FILE_PATH", 0, "Test CSV file path"},
	{"baseFile", 'b', "FILE_PATH", 0, "Base CSV file path"},
    { 0 }
};

/* Parse a single option */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	struct arguments *arguments = state->input; // get input argument from argp_parse
	switch(key) {
	case 'a':
		arguments->absoluteFlag = true;
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
 *   refData: file name for storing base CSV data
 *   testData: file name for storing test CSV data
 *   lowerData: file name for storing tube lower curve data
 *   upperData: file name for storing tube upper curve data
 *   report: file name for validate report
 */
int write_to_file(struct sumData data, char const *refData, char const *testData, char const *lowerData, char const *upperData, char const *report) {
	FILE *f1 = fopen(refData, "w");
	FILE *f2 = fopen(testData, "w");
	FILE *f3 = fopen(lowerData, "w");
	FILE *f4 = fopen(upperData, "w");
	FILE *f5 = fopen(report, "w");

	if (f1 == NULL || f2 == NULL || f3 == NULL || f4 == NULL || f5 == NULL) return -1;
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
	struct arguments arguments;
	// Default values
	arguments.toleranceFlag = false;
	arguments.tolerance = 0.002;
	arguments.axes = 'Y';
	arguments.absoluteFlag = false;
	arguments.outputFlag = false;

	// Parse arguments; every option seen by parse_opt will be reflected in arguments
	argp_parse (&argp, argc, argv, 0, 0, &arguments);

	// Read CSV files into data structure
	struct data testCSV = readCSV(arguments.testFile, 1);
	struct data baseCSV = readCSV(arguments.baseFile, 1);

	char* absolute = (arguments.absoluteFlag) ? "true" : "false";
	// Calculate tube size (half-width and half-height of rectangle)
	double* tube = tubeSize(baseCSV, arguments.tolerance, arguments.axes, 0, 0, !absolute);

	printf("tolerance = %lf, axes = %c, absolute = %s; \n",
					arguments.tolerance, arguments.axes, absolute);
	printf("half-width = %lf; half-height = %lf; \n", tube[0], tube[1]);

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


	write_to_file(sumReport, "results/0_refData.csv", "results/0_testData.csv", "results/0_lowerData.csv", "results/0_upperData.csv", "results/0_report.csv");

	return 0;
}




