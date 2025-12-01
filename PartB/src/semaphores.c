#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>          
#include <sys/stat.h>        
#include <unistd.h>
#include <errno.h>
#include "semaphores.h"

static const char *SEM_NAME_RUBRIC = "/grading_sem_rubric_v1";
static const char *SEM_NAME_LOAD   = "/grading_sem_load_v1";
static const char *SEM_NAME_MARK   = "/grading_sem_mark_v1";

sem_t *sem_rubric = NULL;
sem_t *sem_load   = NULL;
sem_t *sem_mark   = NULL;

// initialize semaphores

int sems_init(void) {
    // unlink old names  
    sem_unlink(SEM_NAME_RUBRIC);
    sem_unlink(SEM_NAME_LOAD);
    sem_unlink(SEM_NAME_MARK);

    sem_rubric = sem_open(SEM_NAME_RUBRIC, O_CREAT | O_EXCL, 0666, 1);
    if (sem_rubric == SEM_FAILED) {
        perror("sem_open rubric");
        sem_rubric = NULL;
        goto fail;
    }

    sem_load = sem_open(SEM_NAME_LOAD, O_CREAT | O_EXCL, 0666, 1);
    if (sem_load == SEM_FAILED) {
        perror("sem_open load");
        sem_load = NULL;
        goto fail;
    }

    sem_mark = sem_open(SEM_NAME_MARK, O_CREAT | O_EXCL, 0666, 1);
    if (sem_mark == SEM_FAILED) {
        perror("sem_open mark");
        sem_mark = NULL;
        goto fail;
    }

    return 0;

fail:
    if (sem_rubric && sem_rubric != SEM_FAILED) { sem_close(sem_rubric); sem_unlink(SEM_NAME_RUBRIC); sem_rubric = NULL; }
    if (sem_load   && sem_load   != SEM_FAILED) { sem_close(sem_load);   sem_unlink(SEM_NAME_LOAD);   sem_load = NULL; }
    if (sem_mark   && sem_mark   != SEM_FAILED) { sem_close(sem_mark);   sem_unlink(SEM_NAME_MARK);   sem_mark = NULL; }
    return -1;
}

// close semaphores 
int sems_destroy(void) {
    int rc = 0;
    if (sem_rubric) {
        if (sem_close(sem_rubric) == -1) rc = -1;
        if (sem_unlink(SEM_NAME_RUBRIC) == -1) rc = -1;
        sem_rubric = NULL;
    }
    if (sem_load) {
        if (sem_close(sem_load) == -1) rc = -1;
        if (sem_unlink(SEM_NAME_LOAD) == -1) rc = -1;
        sem_load = NULL;
    }
    if (sem_mark) {
        if (sem_close(sem_mark) == -1) rc = -1;
        if (sem_unlink(SEM_NAME_MARK) == -1) rc = -1;
        sem_mark = NULL;
    }
    return rc;
}
