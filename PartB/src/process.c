#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "process.h"
#include "grader.h"

// create TA's, each child will run grading loop 
int process_create_tas(const program_config_t *cfg, shared_mem_t *shm, char **exam_files, int exam_count) {
    pid_t *pids = calloc(cfg->num_tas, sizeof(pid_t));
    if (!pids) return -1;
    for (int i = 0; i < cfg->num_tas; ++i) pids[i] = -1;

    int created = 0;
    for (int t = 0; t < cfg->num_tas; ++t) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        }
        if (pid == 0) {
            // child
            grader_run_loop(shm, exam_files, exam_count, cfg->rubric_file);
            _exit(0);
        } else {
            pids[t] = pid;
            created++;
        }
    }

    //parent waits for all children to finish 
    int reaped = 0;
    int status;
    while (reaped < created) {
        pid_t w = waitpid(-1, &status, 0);
        if (w > 0) {
            reaped++;
        } else {
            if (errno == ECHILD) break;
            if (errno == EINTR) continue;
            break;
        }
    }

    free(pids);
    return 0;
}
