#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int num_tas;
    const char *exams_dir;
    const char *rubric_file;
} program_config_t;

int config_parse_args(int argc, char *argv[], program_config_t *out);

#endif 
