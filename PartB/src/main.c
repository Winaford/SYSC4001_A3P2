#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "config.h"
#include "shared_mem.h"
#include "files.h"
#include "process.h"
#include "util.h"
#include "semaphores.h"

int main(int argc, char *argv[]) {
    program_config_t cfg;
    if (!config_parse_args(argc, argv, &cfg)) {
        return EXIT_FAILURE;
    }

    srand(time(NULL) ^ getpid());

    char **exam_files = NULL;
    int exam_count = files_list_exam_dir(cfg.exams_dir, &exam_files);
    if (exam_count <= 0) {
        fprintf(stderr, "No exam files found in %s\n", cfg.exams_dir);
        return EXIT_FAILURE;
    }

    // initialize semaphores before fork 
    if (sems_init() != 0) {
        fprintf(stderr, "Failed to initialize semaphores\n");
        files_free_list(exam_files, exam_count);
        return EXIT_FAILURE;
    }

    // make shared memory 
    shared_mem_t *shm = NULL;
    if (shm_create_and_map(NULL, &shm) != 0) {
        fprintf(stderr, "Failed to create shared memory\n");
        files_free_list(exam_files, exam_count);
        sems_destroy();
        return EXIT_FAILURE;
    }

    // init shared memory
    memset(shm, 0, sizeof(shared_mem_t));
    shm->exam_index = 0;
    shm->total_exams = exam_count;
    shm->loading = 0;
    strncpy(shm->current_exam_filename, exam_files[0], MAX_FILENAME_LEN - 1);
    shm->current_exam_filename[MAX_FILENAME_LEN - 1] = '\0';

    // load rubric into shared mem
    if (files_read_rubric(cfg.rubric_file, shm->rubric) != 0) {
        fprintf(stderr, "Warning: could not read rubric file '%s'. Creating default rubric.\n", cfg.rubric_file);
        for (int i = 0; i < RUBRIC_ENTRIES; ++i) shm->rubric[i] = 'A' + i;
        files_write_rubric(cfg.rubric_file, shm->rubric);
    }

    // load first exam
    if (files_read_exam_studentid(shm->current_exam_filename, shm->student_id) != 0) {
        fprintf(stderr, "Warning: could not read first exam file '%s'. Using 0000\n", shm->current_exam_filename);
        strncpy(shm->student_id, "0000", STUDENT_ID_LEN);
        shm->student_id[STUDENT_ID_LEN - 1] = '\0';
    }
    for (int i = 0; i < RUBRIC_ENTRIES; ++i) shm->marked[i] = 0;

    util_log("Parent: Loaded %d exam files. Initial exam: %s (student %s)", exam_count, shm->current_exam_filename, shm->student_id);

    // create TA's
    process_create_tas(&cfg, shm, exam_files, exam_count);

    shm_unmap_and_unlink(NULL, shm);
    files_free_list(exam_files, exam_count);

    // destroy semaphores
    sems_destroy();

    util_log("Parent: All TAs finished. Cleaning up.");

    return EXIT_SUCCESS;
}
