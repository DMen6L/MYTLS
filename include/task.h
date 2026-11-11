#ifndef TASK_H
#define TASK_H

#include <stddef.h>

#define TITLE_SIZE 256

typedef struct {
    int id;
    char title[TITLE_SIZE];
    int progress;
    int goal;
} Task;

typedef enum {
    VALUE_INT,
    VALUE_TEXT,
} ValueType;

typedef struct {
    // Tasks
    Task *items;
    // For comparing if types are valid
    ValueType *types;
    // Size
    size_t count;
} TaskList;

#endif // !TASK_H
