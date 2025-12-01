#ifndef PROCESS_H
#define PROCESS_H

#include "shared_mem.h"
#include "config.h"

// create TA processes 
int process_create_tas(const program_config_t *cfg, shared_mem_t *shm, char **exam_files, int exam_count);

#endif