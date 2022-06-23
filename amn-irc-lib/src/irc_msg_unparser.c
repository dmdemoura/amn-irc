#include "irc_msg_unparser.h"

#include <stdlib.h>
#include <string.h>

struct IrcMsgUnparser
{
	const Logger* log;

	char buffer[IRC_MSG_SIZE + 1];
	size_t msgLen;
	const IrcMsg* msg;	
};

IrcMsgUnparser* IrcMsgUnparser_New(const Logger* log)
{
	IrcMsgUnparser* self = malloc(sizeof(IrcMsgUnparser));
	if (self == NULL)
	{
		LOG_ERROR(log, "Failed to allocate IrcMsgUnparser");
		return NULL;
	}

	self->log = log;

	return self;
}
void IrcMsgUnparser_Delete(IrcMsgUnparser* self)
{
	free(self);
}

static bool WriteChar(IrcMsgUnparser* self, char character);
static bool WriteString(IrcMsgUnparser* self, const char* string);

static bool UnparsePrefix(IrcMsgUnparser* self);
static bool UnparsePrefixOrigin(IrcMsgUnparser* self);
static bool UnparsePrefixUsername(IrcMsgUnparser* self);
static bool UnparsePrefixHostname(IrcMsgUnparser* self);
static bool UnparseCommand(IrcMsgUnparser* self);
static bool UnparseParams(IrcMsgUnparser* self);
static bool UnparseCrlf(IrcMsgUnparser* self);

const char* IrcMsgUnparser_Unparse(IrcMsgUnparser* self, const IrcMsg* msg)
{
	self->msg = msg;
	self->msgLen = 0;

	if (!UnparsePrefix(self))
	{
		return false;
	}

	if (!UnparseCommand(self))
	{
		return false;
	}

	if (!UnparseParams(self))
	{
		return false;
	}

	if (!UnparseCrlf(self))
	{
		return false;
	}

	self->buffer[self->msgLen] = '\0';

	return self->buffer;
}


static bool WriteChar(IrcMsgUnparser* self, char character)
{
	if (self->msgLen + 1 > IRC_MSG_SIZE)
	{
		LOG_WARN(self->log, "Message exceeds size limit. Limit: %zu", IRC_MSG_SIZE);
		return false;
	}

	self->buffer[self->msgLen] = character;
	self->msgLen += 1;

	return true;
}

static bool WriteString(IrcMsgUnparser* self, const char* string)
{
	size_t len = strlen(string);

	if (self->msgLen + len > IRC_MSG_SIZE)
	{
		LOG_WARN(self->log, "Message exceeds size limit. Limit: %zu", IRC_MSG_SIZE);
		return false;
	}

	memcpy(self->buffer + self->msgLen, string, len);
	self->msgLen += len;

	return true;
}

static bool UnparsePrefix(IrcMsgUnparser* self)
{
	if (self->msg->prefix.origin == NULL)
	{
		return true;
	}

	if (!UnparsePrefixOrigin(self))
	{
		return false;
	}

	if (!UnparsePrefixUsername(self))
	{
		return false;
	}

	if (!UnparsePrefixHostname(self))
	{
		return false;
	}

	if (!WriteChar(self, ' '))
	{
		return false;
	}

	return true;
}

static bool UnparsePrefixOrigin(IrcMsgUnparser* self)
{
	return WriteString(self, self->msg->prefix.origin);
}

static bool UnparsePrefixUsername(IrcMsgUnparser* self)
{
	if (self->msg->prefix.username == NULL)
	{
		return true;
	}

	if (!WriteChar(self, '!'))
	{
		return false;
	}

	if (!WriteString(self, self->msg->prefix.username))
	{
		return false;
	}

	return true;
}

static bool UnparsePrefixHostname(IrcMsgUnparser* self)
{
	if (self->msg->prefix.hostname == NULL)
	{
		return true;
	}

	if (!WriteChar(self, '@'))
	{
		return false;
	}

	if (!WriteString(self, self->msg->prefix.hostname))
	{
		return false;
	}

	return true;
}

static bool UnparseCommand(IrcMsgUnparser* self)
{
	const char* cmdStr = IRC_CMD_TYPE_STRS[self->msg->cmd];

	if (!WriteString(self, cmdStr))
	{
		return false;
	}

	return true;
}

static bool UnparseParams(IrcMsgUnparser* self)
{
	if (!WriteChar(self, ' '))
	{
		return false;
	}

	for (size_t i = 0; i < self->msg->paramCount; i++)
	{
		if (i == self->msg->paramCount - 1 && !WriteChar(self, ':'))
		{
			return false;	
		}

		if (!WriteString(self, self->msg->params[i]))
		{
			return false;
		}
	}

	return true;
}

static bool UnparseCrlf(IrcMsgUnparser* self)
{
	return WriteChar(self, '\r') && WriteChar(self, '\n');
}
