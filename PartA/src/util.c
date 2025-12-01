#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include "util.h"

// sleep for random double
void util_rand_sleep_range(double a, double b) {
    if (b < a) {
        double tmp = a; a = b; b = tmp;
    }
    double r = (double)rand() / (double)RAND_MAX;
    double t = a + r * (b - a);
    time_t sec = (time_t)t;
    long nsec = (long)((t - (double)sec) * 1e9);
    struct timespec ts;
    ts.tv_sec = sec;
    ts.tv_nsec = nsec;
    nanosleep(&ts, NULL);
}

// logging
void util_log(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    printf("\n");
    fflush(stdout);
    va_end(ap);
}
