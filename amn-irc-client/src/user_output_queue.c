#include "user_output_queue.h"

#include <stdlib.h>


UserOutputQueue* UserOutputQueue_New(size_t capacity, int32_t shutdownTimeout)
{
	return (UserOutputQueue*) Queue_New(capacity, shutdownTimeout, sizeof(char*)); 
}

void UserOutputQueue_Delete(UserOutputQueue* self)
{
	Queue_Delete((Queue*) self, free, sizeof(char*));
}

bool UserOutputQueue_Push(UserOutputQueue* self, char* line)
{
	return Queue_Push((Queue*) self, &line, sizeof(char*));
}

char* UserOutputQueue_Pop(UserOutputQueue* self)
{
	char* line;

	if (!Queue_Pop((Queue*) self, &line, sizeof(char*)))
	{
		return false;
	}

	return line;
}

Queue_TryPopResult UserOutputQueue_TryPop(UserOutputQueue* self, char** line)
{
	return Queue_TryPop((Queue*) self, line, sizeof(char*));
}
