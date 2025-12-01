#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>
#include "grader.h"
#include "util.h"
#include "files.h"

void grader_run_loop(shared_mem_t *shm, char **exam_files, int exam_count, const char *rubric_file) {
    pid_t mypid = getpid();

    srand((unsigned)(time(NULL) ^ (mypid << 8)));

    while (1) {
        // check for complete condition
        if (strcmp(shm->student_id, "9999") == 0) {
            util_log("[TA %d] Detected sentinel 9999, exiting.", mypid);
            _exit(0);
        }

        // loop through rubric and randomly correct 
        for (int r = 0; r < RUBRIC_ENTRIES; ++r) {
            util_rand_sleep_range(0.5, 1.0);
            double p = (double)rand() / (double)RAND_MAX;
            if (p < 0.30) {
                char old = shm->rubric[r];
                char nw = (char)(old + 1);
                shm->rubric[r] = nw;
                if (files_write_rubric(rubric_file, shm->rubric) == 0) {
                    util_log("[TA %d] Corrected rubric Q%d: %c -> %c (saved)", mypid, r+1, old, nw);
                } else {
                    util_log("[TA %d] Corrected rubric Q%d: %c -> %c (failed save)", mypid, r+1, old, nw);
                }
            }
        }

        // finding an unmarked question 
        int found = 0;
        for (int q = 0; q < RUBRIC_ENTRIES; ++q) {
            if (shm->marked[q] == 0) {
                //claim question
                shm->marked[q] = 1;
                found = 1;
                util_log("[TA %d] Claiming student %s question %d", mypid, shm->student_id, q+1);
                util_rand_sleep_range(1.0, 2.0);
                util_log("[TA %d] Marked student %s question %d (file: %s)", mypid, shm->student_id, q+1, shm->current_exam_filename);
                break;
            }
        }

        if (!found) {
            //attempt to load next exam 
            if (shm->exam_index + 1 >= shm->total_exams) {
                util_log("[TA %d] All exams processed. Setting sentinel and exiting.", mypid);
                strncpy(shm->student_id, "9999", STUDENT_ID_LEN);
                shm->student_id[STUDENT_ID_LEN - 1] = '\0';
                _exit(0);
            } else {
                if (shm->loading == 0) {
                    // mark as loading 
                    shm->loading = 1;
                    int next_idx = shm->exam_index + 1;
                    if (next_idx >= exam_count) {
                        shm->loading = 0;
                        continue;
                    }
                    const char *next_file = exam_files[next_idx];
                    char next_student[STUDENT_ID_LEN];
                    if (files_read_exam_studentid(next_file, next_student) != 0) {
                        util_log("[TA %d] failed to read %s, skipping", mypid, next_file);
                        shm->exam_index = next_idx;
                        strncpy(shm->current_exam_filename, next_file, MAX_FILENAME_LEN - 1);
                        shm->current_exam_filename[MAX_FILENAME_LEN - 1] = '\0';
                        for (int i = 0; i < RUBRIC_ENTRIES; ++i) shm->marked[i] = 0;
                        shm->loading = 0;
                        continue;
                    }
                    shm->exam_index = next_idx;
                    strncpy(shm->current_exam_filename, next_file, MAX_FILENAME_LEN - 1);
                    shm->current_exam_filename[MAX_FILENAME_LEN - 1] = '\0';
                    strncpy(shm->student_id, next_student, STUDENT_ID_LEN - 1);
                    shm->student_id[STUDENT_ID_LEN - 1] = '\0';
                    for (int i = 0; i < RUBRIC_ENTRIES; ++i) shm->marked[i] = 0;
                    util_log("[TA %d] Loaded next exam index %d file %s (student %s)", mypid, next_idx, shm->current_exam_filename, shm->student_id);
                    shm->loading = 0;
                } else {
                    //small pause
                    struct timespec ts1 = {0, 100 * 1000 * 1000}; 
                    nanosleep(&ts1, NULL);
                }
            }
        }

        // small pause to simulate waiting
        struct timespec ts2 = {0, 50 * 1000 * 1000}; 
        nanosleep(&ts2, NULL);
    }
}
