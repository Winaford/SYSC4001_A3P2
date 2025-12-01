#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

int config_parse_args(int argc, char *argv[], program_config_t *out) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <num_TAs> <exams_dir> <rubric_file>\n", argv[0]);
        return 0;
    }
    int n = atoi(argv[1]);
    if (n < 2) {
        fprintf(stderr, "Please provide at least 2 TAs (num_TAs >= 2)\n");
        return 0;
    }
    out->num_tas = n;
    out->exams_dir = argv[2];
    out->rubric_file = argv[3];
    return 1;
}
