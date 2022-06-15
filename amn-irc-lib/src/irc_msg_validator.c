#include "irc_msg_validator.h"

#include <stddef.h>
#include <stdlib.h>

struct IrcMsgValidator
{
	const Logger* log;
};

IrcMsgValidator* IrcMsgValidator_New(const Logger* logger)
{
	IrcMsgValidator* self = malloc(sizeof(IrcMsgValidator));
	if (self == NULL)
	{
		return NULL;
	}

	self->log = logger;

	return self;
}

void IrcMsgValidator_Delete(IrcMsgValidator* self)
{
	free(self);
}

bool IrcMsgValidator_ValidateOrigin(const IrcMsgValidator* self,
		const char* origin, const char* originEnd)
{
	(void) self, (void) origin, (void) originEnd;
	return true;
}

bool IrcMsgValidator_ValidateNick(const IrcMsgValidator* self, const char* nick);

bool IrcMsgValidator_ValidateUser(const IrcMsgValidator* self,
		const char* user, const char* userEnd)
{
	(void) self, (void) user, (void) userEnd;
	return true;
}

bool IrcMsgValidator_ValidateHost(const IrcMsgValidator* self,
		const char* host, const char* hostEnd)
{
	(void) self, (void) host, (void) hostEnd;
	return true;
}

bool IrcMsgValidator_ValidateCommand(const IrcMsgValidator* self,
		const char* command, const char* commandEnd)
{
	(void) self, (void) command, (void) commandEnd;
	return true;
}

bool IrcMsgValidator_ValidateMiddleParam(const IrcMsgValidator* self,
		const char* param, const char* paramEnd)
{
	(void) self, (void) param, (void) paramEnd;
	return true;
}

bool IrcMsgValidator_ValidateTrailingParam(const IrcMsgValidator* self,
		const char* param, const char* paramEnd)
{
	(void) self, (void) param, (void) paramEnd;
	return true;
}

bool IrcMsgValidator_ValidateChannel(const IrcMsgValidator* self, const char* channel);

bool IrcMsgValidator_ValidateMask(const IrcMsgValidator* self, const char* mask);

bool IrcMsgValidator_ValidateChstring(const IrcMsgValidator* self, const char* chstring);


