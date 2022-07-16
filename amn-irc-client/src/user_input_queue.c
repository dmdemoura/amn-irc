#include "user_input_queue.h"

#include "queue.h"

UserInputQueue* UserInputQueue_New(size_t capacity, int32_t shutdownTimeout)
{
	return (UserInputQueue*) Queue_New(capacity, shutdownTimeout, sizeof(char*)); 
}

void UserInputQueue_Delete(UserInputQueue* self)
{
	Queue_Delete((Queue*) self);
}

bool UserInputQueue_Push(UserInputQueue* self, char* line)
{
	return Queue_Push((Queue*) self, &line, sizeof(char*));
}

char* UserInputQueue_Pop(UserInputQueue* self)
{
	char* line;

	if (!Queue_Pop((Queue*) self, &line, sizeof(char*)))
	{
		return false;
	}

	return line;
}
