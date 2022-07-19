#include "irc_msg_writer.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>

struct IrcMsgWriter
{
	const Logger* log;
	int socket;
};

IrcMsgWriter* IrcMsgWriter_New(const Logger* log, int socket)
{
	IrcMsgWriter* self = malloc(sizeof(IrcMsgWriter));
	if (self == NULL) {
		LOG_ERROR(log, "Failure to allocate IrcMsgWriter");
		return NULL;
	}

	self->log = log;
	self->socket = socket;

	return self;
}

void IrcMsgWriter_Delete(IrcMsgWriter* self)
{
	free(self);
}

bool IrcMsgWriter_Write(IrcMsgWriter* self, const char* msg)
{
	size_t msgLen = strlen(msg);

	size_t totalBytesWritten = 0;
	do
	{
		const char* sendStart = msg + totalBytesWritten;
		size_t sendLen = msgLen - totalBytesWritten;

		ssize_t bytesWritten = send(self->socket, sendStart, sendLen, 0); 

		if (bytesWritten < 0)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
			{
				LOG_ERROR(self->log, "Failure while writing message to socket");
			}

			return false;
		}

		totalBytesWritten += (size_t) bytesWritten;
	}
	while (totalBytesWritten < msgLen);

	return true;
}
