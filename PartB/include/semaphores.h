#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include <semaphore.h>

extern sem_t *sem_rubric; // protects rubric array
extern sem_t *sem_load;   // protects loading next exam
extern sem_t *sem_mark;   // protects checking and marking 

int sems_init(void);      // create semaphores 
int sems_destroy(void);   // close and unlink semaphores 

#endif 
