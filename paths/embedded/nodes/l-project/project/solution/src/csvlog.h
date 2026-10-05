#ifndef CSVLOG_H
#define CSVLOG_H

#include <limits.h>

/* <dir>/sensord.csv, rotated to sensord.csv.1 … .keep when it's full. */
struct csvlog {
    int fd;
    char path[PATH_MAX];
    int rows;              /* data rows in the current file */
    int rotate_lines, keep;
    unsigned long rotations;
};

/* Creates dir if needed and opens the log for appending. An existing file's
 * rows count towards rotate_lines. 0, or -1 with errno set. */
int csvlog_open(struct csvlog *log, const char *dir, int rotate_lines, int keep);
/* Appends one row (without the newline), rotating first if the file is full. */
int csvlog_write(struct csvlog *log, const char *row);
void csvlog_close(struct csvlog *log);

#endif
