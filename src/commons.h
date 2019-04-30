#ifndef COMMONS_H_
#define COMMONS_H_

#include <stdio.h>
#include <stdlib.h>

extern FILE *log_file;

int init_log(void);

int close_log(void);

#endif /* COMMONS_H_ */
