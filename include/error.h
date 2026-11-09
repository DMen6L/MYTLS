#ifndef MYTLS_ERROR
#define MYTLS_ERROR

#define ERROR_TEXT_LENGTH 512

typedef struct {
    char message[ERROR_TEXT_LENGTH];
} Error;

#endif
