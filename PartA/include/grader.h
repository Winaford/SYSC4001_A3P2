#ifndef GRADER_H
#define GRADER_H

#include "shared_mem.h"

// loop inside each TA process
void grader_run_loop(shared_mem_t *shm, char **exam_files, int exam_count, const char *rubric_file);

#endif 

