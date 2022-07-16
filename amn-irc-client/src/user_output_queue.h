#ifndef AMN_USER_OUTPUT_QUEUE_H
#define AMN_USER_OUTPUT_QUEUE_H


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "queue.h"


typedef struct UserOutputQueue UserOutputQueue;

UserOutputQueue* UserOutputQueue_New(size_t capacity, int32_t shutdownTimeout);
void UserOutputQueue_Delete(UserOutputQueue* self);

bool UserOutputQueue_Push(UserOutputQueue* self, char* line);
char* UserOutputQueue_Pop(UserOutputQueue* self);
Queue_TryPopResult UserOutputQueue_TryPop(UserOutputQueue* self, char** line);



#endif // AMN_USER_OUTPUT_QUEUE_H
