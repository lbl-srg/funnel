
#include "commons.h"

int init_log(void) {
    if (log_file == NULL) {
        log_file = fopen("c_funnel.log", "w+");
        if (log_file == NULL) {
            perror("Failed to open log file.\n");
            return -1;
        }
    }
    return 0;
}

int close_log(void) {
    if (log_file != NULL){
        fclose(log_file);
    }
    return 0;
}