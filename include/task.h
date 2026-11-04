#ifndef TASK_H
#define TASK_H

#include <stddef.h>

#define PAGE_SIZE 50
#define TITLE_SIZE 256

typedef struct {
    int id;
    char title[TITLE_SIZE];
    int done;
} Task;

typedef struct {
    Task *items;
    size_t count;
} TaskList;

#endif // !TASK_H
