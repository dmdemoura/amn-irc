#ifndef AMN_USER_INPUT_QUEUE_H
#define AMN_USER_INPUT_QUEUE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct UserInputQueue UserInputQueue;


UserInputQueue* UserInputQueue_New(size_t capacity, int32_t shutdownTimeout);
void UserInputQueue_Delete(UserInputQueue* self);

bool UserInputQueue_Push(UserInputQueue* self, char* line);
char* UserInputQueue_Pop(UserInputQueue* self);

#endif // AMN_USER_INPUT_QUEUE_H
