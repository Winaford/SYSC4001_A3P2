#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "shared_mem.h"

static const char *DEFAULT_SHM_NAME = "/grading_sim_shm_v1";

int shm_create_and_map(const char *shm_name, shared_mem_t **out_shm) {
    const char *name = shm_name ? shm_name : DEFAULT_SHM_NAME;
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0) {
        perror("shm_open");
        return -1;
    }
    size_t shm_size = sizeof(shared_mem_t);
    if (ftruncate(shm_fd, shm_size) == -1) {
        perror("ftruncate");
        close(shm_fd);
        return -1;
    }
    shared_mem_t *shm = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        return -1;
    }
    close(shm_fd);
    *out_shm = shm;
    return 0;
}

int shm_unmap_and_unlink(const char *shm_name, shared_mem_t *shm) {
    const char *name = shm_name ? shm_name : DEFAULT_SHM_NAME;
    if (shm) {
        munmap(shm, sizeof(shared_mem_t));
    }
    if (shm_unlink(name) == -1) {
        
    }
    return 0;
}
