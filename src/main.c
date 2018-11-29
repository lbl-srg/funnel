/*
 * main.c
 *
 *  Created on: Apr 4, 2018
 *      Author: jianjun
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "compare.h"
#include "data_structure.h"

/* Flag set by '--verbose'. */
static int verbose_flag;

int main(int argc, char **argv) {
  int exiVal; // Exit value

  int c;

  double atolx = 0, atoly = 0, rtolx = 0.002, rtoly = 0.002;
  char *output = NULL, *testFile = NULL, *baseFile = NULL;
  char *help =
		  "Usage: funnel [OPTION...] \n"
		  "--test             Path of CSV file to be tested \n"
		  "--reference        Path of CSV file to be used as base \n"
		  "--output           Directory path to save output results \n"
		  "--atolx            Absolute tolerance in x direction, default=0 \n"
		  "--atoly            Absolute tolerance in y direction, default=0 \n"
		  "--rtolx            Relative tolerance in x direction, default=0.002 \n"
		  "--rtoly            Relative tolerance in y direction, default=0.002 \n"
		  "--help             Give this help list\n";

  struct tolerances tolerances;

  while (1)
  {
	  static struct option long_options[] =
	  {
		{"verbose",    no_argument,        &verbose_flag,   1},
		{"brief",      no_argument,        &verbose_flag,   0},
		{"test",       required_argument,  0,              't'},
		{"reference",  required_argument,  0,              'r'},
		{"atolx",      required_argument,  0,              'X'},
		{"atoly",      required_argument,  0,              'Y'},
		{"rtolx",      required_argument,  0,              'x'},
		{"rtoly",      required_argument,  0,              'y'},
		{"output",     required_argument,  0,              'o'},
		{"help",       no_argument,        0,              'h'},
		{0, 0, 0, 0}
	  };

	  /* getopt_long stores the option index here. */
	  int option_index = 0;

	  c = getopt_long_only (argc, argv, "f:b:X:Y:x:y:o:h",
                          long_options, &option_index);

	  /* Detect the end of the options. */
	  if (c == -1)
		  break;

	  switch (c) {
	  case 0:
		  /* If this option set a flag, do nothing else now. */
		  if (long_options[option_index].flag != 0)
			  break;
		  printf ("option %s", long_options[option_index].name);
		  if (optarg)
			  printf (" with arg %s", optarg);
		  printf ("\n");
		  break;
	  case 'X':
		  atolx = atof(optarg);
		  break;
	  case 'Y':
		  atoly = atof(optarg);
	  	  break;
	  case 'x':
		  rtolx = atof(optarg);
		  break;
	  case 'y':
		  rtoly = atof(optarg);
		  break;
	  case 't':
		  testFile = optarg;
		  break;
	  case 'r':
		  baseFile = optarg;
		  break;
	  case 'o':
		  output = optarg;
		  break;
	  case 'h':
		  printf("use it as following: \n %s", help);
		  exit (0);
	  case '?':
		  break;
	  default:
		  abort ();
	  }
  }

  tolerances.atolx = atolx;
  tolerances.atoly = atoly;
  tolerances.rtolx = rtolx;
  tolerances.rtoly = rtoly;

  struct data baseCSV = readCSV(baseFile, 1);
  struct data testCSV = readCSV(testFile, 1);

  exiVal = compareAndReport(
    baseCSV.x,
    baseCSV.y,
    baseCSV.n,
    testCSV.x,
    testCSV.y,
    testCSV.n,
    output,
    tolerances);

  return exiVal;
}
