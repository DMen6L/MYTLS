#include "task.h"
#include <stdio.h>
#include <string.h>

int asgn_task_val(void *dest, size_t dest_size, const void *value,
                  ValueType type, Error *error) {
    error->message[0] = '\0';

    if (dest == NULL || value == NULL) {
        snprintf(error->message, sizeof(error->message),
                 "asgn_task_val: destination and value are required");
        return 1;
    }

    switch (type) {
    case VALUE_INT:
        if (dest_size != sizeof(int)) {
            snprintf(
                error->message, sizeof(error->message),
                "asgn_task_val: integer destination must be sizeof(int) bytes");
            return 1;
        }
        memmove(dest, value, sizeof(int));
        return 0;
    case VALUE_TEXT: {
        size_t length = strlen(value);
        if (length >= dest_size) {
            snprintf(error->message, sizeof(error->message),
                     "asgn_task_val: text does not fit destination including "
                     "terminator");
            return 1;
        }
        memmove(dest, value, length + 1);
        return 0;
    }
    default:
        snprintf(error->message, sizeof(error->message),
                 "asgn_task_val: unsupported value type");
        return 1;
    }
}
