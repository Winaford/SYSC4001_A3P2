#ifndef SHARED_MEM_H
#define SHARED_MEM_H

#include <limits.h>

#define RUBRIC_ENTRIES 5
#define STUDENT_ID_LEN 5 //4 digits + null terminator
#define MAX_FILENAME_LEN 1024

typedef struct {
    char student_id[STUDENT_ID_LEN];
    char rubric[RUBRIC_ENTRIES];
    int marked[RUBRIC_ENTRIES];
    int exam_index;
    int total_exams;
    char current_exam_filename[MAX_FILENAME_LEN];
    int loading;
    char pad[64];
} shared_mem_t;

int shm_create_and_map(const char *shm_name, shared_mem_t **out_shm);
int shm_unmap_and_unlink(const char *shm_name, shared_mem_t *shm);

#endif 
