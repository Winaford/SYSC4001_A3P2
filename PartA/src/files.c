#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include "files.h"

static int cmpstr(const void *a, const void *b) {
    return strcmp(*(char * const *)a, *(char * const *)b);
}

// List all exams alphabetically 
int files_list_exam_dir(const char *dirpath, char ***out_files) {
    DIR *d = opendir(dirpath);
    if (!d) {
        perror("opendir");
        return -1;
    }

    struct dirent *ent;
    int capacity = 64;
    int count = 0;
    char **list = malloc(sizeof(char*) * capacity);
    if (!list) {
        closedir(d);
        return -1;
    }

    while ((ent = readdir(d)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        char filepath[PATH_MAX];
        if (snprintf(filepath, sizeof(filepath), "%s/%s", dirpath, ent->d_name) >= (int)sizeof(filepath)) {
            continue;
        }

        struct stat st;
        if (stat(filepath, &st) == -1) {
            continue;
        }

        if (!S_ISREG(st.st_mode))
            continue;

        if (count >= capacity) {
            int newcap = capacity * 2;
            char **tmp = realloc(list, sizeof(char*) * newcap);
            if (!tmp) {
                for (int i = 0; i < count; ++i) free(list[i]);
                free(list);
                closedir(d);
                return -1;
            }
            list = tmp;
            capacity = newcap;
        }

        list[count] = malloc(MAX_FILENAME_LEN);
        if (!list[count]) {
            for (int i = 0; i < count; ++i) free(list[i]);
            free(list);
            closedir(d);
            return -1;
        }
        strncpy(list[count], filepath, MAX_FILENAME_LEN - 1);
        list[count][MAX_FILENAME_LEN - 1] = '\0';
        count++;
    }

    closedir(d);

    if (count == 0) {
        free(list);
        *out_files = NULL;
        return 0;
    }

    qsort(list, count, sizeof(char*), cmpstr);
    *out_files = list;
    return count;
}

// Free the list of exams 
void files_free_list(char **files, int count) {
    if (!files) return;
    for (int i = 0; i < count; ++i) {
        free(files[i]);
    }
    free(files);
}

// read the rubric file into an array
int files_read_rubric(const char *rubric_file, char rubric[RUBRIC_ENTRIES]) {
    FILE *f = fopen(rubric_file, "r");
    if (!f) return -1;

    char line[256];
    int i = 0;
    while (i < RUBRIC_ENTRIES && fgets(line, sizeof(line), f)) {
        char *comma = strchr(line, ',');
        if (!comma) {
            rubric[i] = 'A' + i;
            i++;
            continue;
        }
        char *p = comma + 1;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '\n')
            rubric[i] = 'A' + i;
        else
            rubric[i] = *p;
        i++;
    }

    fclose(f);

    for (; i < RUBRIC_ENTRIES; ++i)
        rubric[i] = 'A' + i;

    return 0;
}

// write rubric array back to file
int files_write_rubric(const char *rubric_file, char rubric[RUBRIC_ENTRIES]) {
    char tmpname[PATH_MAX];
    pid_t pid = getpid();
    int ret = snprintf(tmpname, sizeof(tmpname), "%s.tmp.%d", rubric_file, (int)pid);
    if (ret < 0 || ret >= (int)sizeof(tmpname)) {
        errno = ENAMETOOLONG;
        return -1;
    }

    FILE *f = fopen(tmpname, "w");
    if (!f) {
        perror("fopen tmp rubric");
        return -1;
    }

    for (int i = 0; i < RUBRIC_ENTRIES; ++i) {
        if (fprintf(f, "%d, %c\n", i+1, rubric[i]) < 0) {
            fclose(f);
            unlink(tmpname);
            return -1;
        }
    }
    fflush(f);
    if (fclose(f) != 0) {
        unlink(tmpname);
        return -1;
    }

    if (rename(tmpname, rubric_file) != 0) {
        perror("rename rubric");
        unlink(tmpname);
        return -1;
    }
    return 0;
}

// read the first student ID 
int files_read_exam_studentid(const char *path, char out_id[STUDENT_ID_LEN]) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char line[512];
    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return -1;
    }

    char digits[STUDENT_ID_LEN];
    int idx = 0;
    char *p = line;

    //strip and clean 
    
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
    
    //get 4 digits
    while (*p && idx < (STUDENT_ID_LEN - 1)) {
        if (*p >= '0' && *p <= '9') {
            digits[idx++] = *p++;
        } else {
            if (idx > 0) break;
            p++;
        }
    }
    digits[idx] = '\0';

    if (idx == 0) {
        strncpy(out_id, "0000", STUDENT_ID_LEN);
        out_id[STUDENT_ID_LEN - 1] = '\0';
    } else {
        strncpy(out_id, digits, STUDENT_ID_LEN);
        out_id[STUDENT_ID_LEN - 1] = '\0';
    }

    fclose(f);
    return 0;
}
