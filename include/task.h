#ifndef TASK_H
#define TASK_H

#include <stddef.h>
#include "error.h"

#define TITLE_SIZE 256

typedef struct {
    int id;
    char title[TITLE_SIZE];
    int progress;
    int goal;
} Task;

typedef struct {
    Task *items;
    size_t count;
} TaskList;

typedef enum {
    VALUE_INT,
    VALUE_TEXT,
} ValueType;

/* Return 0 on success, 1 on failure; leave dest unchanged on failure.
 * error must be valid. dest_size is the writable destination size in bytes.
 * value must point to an int for VALUE_INT or a NUL-terminated string for
 * VALUE_TEXT. Callers are responsible for matching pointers to their type. */
int asgn_task_val(void *dest, size_t dest_size, const void *value,
                  ValueType type, Error *error);

#endif // !TASK_H
