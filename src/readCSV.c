/*
 * readCSV.c
 *
 * Created on: Apr 4, 2018
 * Author: jianjun
 *
 * Functions:
 * ----------
 *   readCSV : reads in CSV file and returns data structure
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "data_structure.h"
#include "readCSV.h"


/*
 * Function: file_exist
 * -----------------
 *  Test if file is in file system (with no dependency to unistd.h (access()): not available on Windows).
 *  
 *  filename: path to file
 *  
 *  returns: TRUE if exists
 */
int file_exist (const char *filename)
{
  struct stat buffer;   
  return (stat(filename, &buffer) == 0);
}

/*
 * Function: readCSV
 * -----------------
 *   read in CSV file and returns data structure. The CSV file should be two columns, delimited by comma or semicolon.
 *
 *   filename: path to the CSV file
 *   skipLines: number of head lines to be skipped
 *
 *   returns: the data structure "inputs", which includes fist and second data set, and the number of rows
 */
struct data readCSV(const char * filename, int skipLines) {
  int i;
  struct data inputs;
  double *time;
  double *value;
  int arraySize = 1;
  int rowCount = 0;
  char buf[100];

  FILE *fp;

  if (!file_exist(filename)){
    fprintf(stderr, "No such file: %s\n", filename);
    exit(1);
  }
  fp = fopen(filename, "r");
  if (!(fp)){
    fprintf(stderr, "Cannot open file: %s\n", filename);
    exit(1);
  }
  
  time = malloc(sizeof(double) * arraySize);
  if (time == NULL){
    fputs("Error: Failed to allocate memory for time.\n", stderr);
    exit(1);
  }
  value = malloc(sizeof(double) * arraySize);
  if (value == NULL){
	  fputs("Error: Failed to allocate memory for value.\n", stderr);
	  exit(1);
  }

  memset(time,0,sizeof(double)*arraySize);
  memset(value,0,sizeof(double)*arraySize);

  for (i=0; i<skipLines; i++) {
    // skip the first "skipLines" lines
    if (fgets(buf,100,fp) == NULL){
      if (ferror(fp)) {
        fprintf(stderr, "Error: Failed to skip the first %d lines in file %s\n", skipLines, filename);
      } else {
        fprintf(stderr, "End-of-File reached.\n");
      }
      exit(1);
    }
  }

  while (fscanf(fp, "%lf%*[,;]%lf\n", &time[rowCount], &value[rowCount]) == 2) {
    if (rowCount == arraySize ) {
      // need more space
      arraySize += 5;
      double *time_tmp = realloc(time, sizeof(double)*(arraySize+1));
      double *value_tmp = realloc(value, sizeof(double)*(arraySize+1));
      if (time_tmp == NULL || value_tmp == NULL) {
        fputs("Fatal error -- out of memory!\n", stderr);
        exit(1);
      }
      time = time_tmp;
      value = value_tmp;
    }
    rowCount++;
  }
  fclose(fp);

  inputs.x = time;
  inputs.y = value;
  inputs.n = rowCount;

  return inputs;
}
