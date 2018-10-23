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

#include "compare.h"

/* Used to communicate with parse_opt */
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
