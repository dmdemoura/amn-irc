#ifndef AMN_USER_OUTPUT_QUEUE_H
#define AMN_USER_OUTPUT_QUEUE_H


#include <stdbool.h>
#include <stddef.h>

typedef struct UserOutputQueue UserOutputQueue;

typedef enum UserOutputQueue_TryPopResult 
{
	UserOutputQueue_TryPopResult_Ok,
	UserOutputQueue_TryPopResult_Empty,
	UserOutputQueue_TryPopResult_Error,
}
UserOutputQueue_TryPopResult;

UserOutputQueue* UserOutputQueue_New(size_t capacity);
void UserOutputQueue_Delete(UserOutputQueue* self);

bool UserOutputQueue_Push(UserOutputQueue* self, char* line);
char* UserOutputQueue_Pop(UserOutputQueue* self);
UserOutputQueue_TryPopResult UserOutputQueue_TryPop(UserOutputQueue* self, char** line);



#endif // AMN_USER_OUTPUT_QUEUE_H
