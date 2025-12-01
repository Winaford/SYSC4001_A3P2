#ifndef FILES_H
#define FILES_H

#include "shared_mem.h"

// Read the exam filenames from dir 
int files_list_exam_dir(const char *dirpath, char ***out_files);

// free the array returned by listing exam dir 
void files_free_list(char **files, int count);

// read rubric file into char array 
int files_read_rubric(const char *rubric_file, char rubric[RUBRIC_ENTRIES]);

// write rubric array to file 
int files_write_rubric(const char *rubric_file, char rubric[RUBRIC_ENTRIES]);

// read first line of exam file and extract student ID 
int files_read_exam_studentid(const char *path, char out_id[STUDENT_ID_LEN]);

#endif 
