#include "irc_cmd_queue.h"

#include "queue.h"


IrcCmdQueue* IrcCmdQueue_New(size_t capacity, int32_t shutdownTimeout)
{
	return (IrcCmdQueue*) Queue_New(capacity, shutdownTimeout, sizeof(IrcCmd*)); 
}

void IrcCmdQueue_Delete(IrcCmdQueue* self)
{
	Queue_Delete((Queue*) self);
}

bool IrcCmdQueue_Push(IrcCmdQueue* self, IrcCmd* ircCmd)
{
	return Queue_Push((Queue*) self, &ircCmd, sizeof(IrcCmd*));
}

IrcCmd* IrcCmdQueue_Pop(IrcCmdQueue* self)
{
	IrcCmd* ircCmd;

	if (!Queue_Pop((Queue*) self, &ircCmd, sizeof(IrcCmd*)))
	{
		return false;
	}

	return ircCmd;
}
