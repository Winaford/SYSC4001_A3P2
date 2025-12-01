#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>
#include "grader.h"
#include "util.h"
#include "files.h"
#include "semaphores.h"

void grader_run_loop(shared_mem_t *shm, char **exam_files, int exam_count, const char *rubric_file) {
    pid_t mypid = getpid();

    srand((unsigned)(time(NULL) ^ (mypid << 8)));

    while (1) {
        // termination check 
        if (strcmp(shm->student_id, "9999") == 0) {
            util_log("[TA %d] Detected sentinel 9999, exiting.", mypid);
            _exit(0);
        }

        //loop rubric entries, randomly decide to correct answer
        for (int r = 0; r < RUBRIC_ENTRIES; ++r) {
            util_rand_sleep_range(0.5, 1.0);
            double p = (double)rand() / (double)RAND_MAX;
            if (p < 0.30) {
                // lock rubric for update
                if (sem_wait(sem_rubric) == -1) {
                    if (errno == EINTR) { --r; continue; } 
                    util_log("[TA %d] sem_wait(sem_rubric) failed: %s", mypid, strerror(errno));
                } else {
                    char old = shm->rubric[r];
                    char nw = (char)(old + 1);
                    shm->rubric[r] = nw;
                    if (files_write_rubric(rubric_file, shm->rubric) == 0) {
                        util_log("[TA %d] Corrected rubric Q%d: %c -> %c (saved)", mypid, r+1, old, nw);
                    } else {
                        util_log("[TA %d] Corrected rubric Q%d: %c -> %c (failed save)", mypid, r+1, old, nw);
                    }
                    sem_post(sem_rubric);
                }
            }
        }

        // try to find an unmarked question
        int found = 0;
        if (sem_wait(sem_mark) == -1) {
            if (errno == EINTR) {}
        } else {
            for (int q = 0; q < RUBRIC_ENTRIES; ++q) {
                if (shm->marked[q] == 0) {
                    //claim question
                    shm->marked[q] = 1;
                    found = 1;
                    util_log("[TA %d] Claiming student %s question %d", mypid, shm->student_id, q+1);
                    break;
                }
            }
            sem_post(sem_mark);
        }

        if (found) {
            util_rand_sleep_range(1.0, 2.0);
            util_log("[TA %d] Marked student %s question (file: %s)", mypid, shm->student_id, shm->current_exam_filename);
        } else {
            // exam finished try to load next exam */
            if (sem_wait(sem_load) == -1) {
                if (errno == EINTR) {}
            } else {
                // check if at end
                if (shm->exam_index + 1 >= shm->total_exams) {
                    util_log("[TA %d] All exams processed. Setting sentinel and exiting.", mypid);
                    strncpy(shm->student_id, "9999", STUDENT_ID_LEN);
                    shm->student_id[STUDENT_ID_LEN - 1] = '\0';
                    sem_post(sem_load);
                    _exit(0);
                } else {
                    int next_idx = shm->exam_index + 1;
                    if (next_idx >= exam_count) {
                        sem_post(sem_load);
                    } else {
                        const char *next_file = exam_files[next_idx];
                        char next_student[STUDENT_ID_LEN];

                        if (files_read_exam_studentid(next_file, next_student) != 0) {
                            util_log("[TA %d] failed to read %s, skipping", mypid, next_file);

                            // advance exam index 
                            shm->exam_index = next_idx;
                            strncpy(shm->current_exam_filename, next_file, MAX_FILENAME_LEN - 1);
                            shm->current_exam_filename[MAX_FILENAME_LEN - 1] = '\0';
                            for (int i = 0; i < RUBRIC_ENTRIES; ++i) shm->marked[i] = 0;
                            sem_post(sem_load);

                        } else {
                            // update shared memory atomically 
                            shm->exam_index = next_idx;
                            strncpy(shm->current_exam_filename, next_file, MAX_FILENAME_LEN - 1);
                            shm->current_exam_filename[MAX_FILENAME_LEN - 1] = '\0';
                            strncpy(shm->student_id, next_student, STUDENT_ID_LEN - 1);
                            shm->student_id[STUDENT_ID_LEN - 1] = '\0';

                            for (int i = 0; i < RUBRIC_ENTRIES; ++i) shm->marked[i] = 0;
                            util_log("[TA %d] Loaded next exam index %d file %s (student %s)", mypid, next_idx, shm->current_exam_filename, shm->student_id);
                            sem_post(sem_load);
                        }
                    }
                }
            }
        }

        // small simulated pause 
        struct timespec ts2 = {0, 50 * 1000 * 1000}; //50ms
        nanosleep(&ts2, NULL);
    }
}
