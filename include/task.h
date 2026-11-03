#ifndef TASK_H
#define TASK_H

#define PAGE_SIZE 50
#define TITLE_SIZE 256

typedef struct {
    int id;
    char title[TITLE_SIZE];
    int done;
} Task;

#endif // !TASK_H
