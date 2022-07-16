#include "irc_msg_reader.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <unistd.h>

#include "irc_msg.h"

#define BUF_SIZE IRC_MSG_SIZE

struct IrcMsgReader
{
	const Logger* log;
	int socket;

	// Temporary buffer to read data from socket.
	uint8_t readBuffer[BUF_SIZE];

	// Buffer to assemble message.
	// A reference is returned to the caller after each Read
	// call, and is valid until the next Read or Delete call.
	char msgBuffer[IRC_MSG_SIZE + 1];

	// Position on the readBuffer where the next message starts.
	// SIZE_MAX if the next message is not available.
	size_t nextMsgStart;
	// Length of next message data in the readBuffer.
	size_t nextMsgLen;
};

static ssize_t FindMessageEnd(const uint8_t* buffer, ssize_t len);


IrcMsgReader* IrcMsgReader_New(const Logger* log, int socket)
{
	IrcMsgReader* self = malloc(sizeof(IrcMsgReader));
	if (self == NULL)
	{
		return NULL;
	}

	self->log = log;
	self->socket = socket;
	self->nextMsgStart = SIZE_MAX;
	self->nextMsgLen = 0;

	return self;
}


void IrcMsgReader_Delete(IrcMsgReader* self)
{
	free(self);
}


const char* IrcMsgReader_Read(IrcMsgReader* self)
{
	if (self->nextMsgStart != SIZE_MAX)
	{
		// Copy start of next message
		memcpy(self->msgBuffer, self->readBuffer + self->nextMsgStart, self->nextMsgLen); 
	}

	size_t msgLen = 0;
	ssize_t msgEnd = -1;
	ssize_t readLen = 0;
	do
	{
		LOG_DEBUG(self->log, "Waiting for mesage");

		readLen = read(self->socket, self->readBuffer, IRC_MSG_SIZE);

		if (readLen == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
		{
			return NULL;
		}

		LOG_DEBUG(self->log, "Received %zd bytes", readLen);

		if(readLen == 0)
		{
			LOG_INFO(self->log, "EOF. Unexpected client disconnect.");
			return NULL;
		}
		else if (readLen == -1)
		{
			LOG_ERROR(self->log, "Failed to read message.");
			return NULL;
		}


		// Check if we have a CRLF in the readBuffer
		ssize_t msgEnd = FindMessageEnd(self->readBuffer, readLen);
		// Copy until the CRLF or the read length otherwise.
		size_t copyLen = (size_t) (msgEnd != -1 ? msgEnd + 1 : readLen);

		if (msgLen + copyLen <= IRC_MSG_SIZE)
		{
			memcpy(self->msgBuffer + msgLen, self->readBuffer, copyLen);
			msgLen += copyLen;
		}
		else
		{
			LOG_ERROR(self->log, "Received message exceeds expected size");

			// We will keep receiving the "too long message" bytes,
			// and ignoring them, until we get a CRLF and can continue to
			// the next message.
			if (msgEnd != -1)
			{
				// Recover from too long message: Discard long message, and start
				// assembling the next one.
				size_t nextMsgStart = (size_t) msgEnd + 1;
				size_t nextMsgLen = (size_t) readLen - nextMsgStart;

				memcpy(self->msgBuffer, self->readBuffer + nextMsgStart, nextMsgLen);

				// Clear msgEnd flag, as we discarded that message.
				msgEnd = -1;
			}
		}
	}
	while (msgEnd != -1);

	// There may still be bytes from the next message on the readBuffer.
	// Store the information needed to assemble that next call.
	self->nextMsgStart = (size_t) msgEnd + 1;
	self->nextMsgLen = (size_t) readLen - self->nextMsgStart;

	// Set delimiter. There's one extra byte in the array size for it if needed.
	self->msgBuffer[msgLen] = '\0';
	// Return message, this will be valid until the next Read or Delete call.
	return self->msgBuffer;
}


static ssize_t FindMessageEnd(const uint8_t* buffer, ssize_t len)
{
	for (ssize_t i = 0; i + 1 < len; i++)
	{
		if (buffer[i] == '\r' && buffer[i + 1] == '\n')
		{
			return i + 1;
		}
	}

	return -1;
}
