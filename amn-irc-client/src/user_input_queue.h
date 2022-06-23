#ifndef AMN_USER_INPUT_QUEUE_H
#define AMN_USER_INPUT_QUEUE_H

#include <stdbool.h>
#include <stddef.h>

typedef struct UserInputQueue UserInputQueue;


UserInputQueue* UserInputQueue_New(size_t capacity);
void UserInputQueue_Delete(UserInputQueue* self);

bool UserInputQueue_Push(UserInputQueue* self, char* line);
char* UserInputQueue_Pop(UserInputQueue* self);

#endif // AMN_USER_INPUT_QUEUE_H
