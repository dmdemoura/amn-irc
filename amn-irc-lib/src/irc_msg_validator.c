#include "irc_msg_validator.h"

#include <stddef.h>
#include <stdlib.h>

struct IrcMsgValidator
{
	const Logger* log;
};

static bool InBounds(const char* start, const char* end, size_t i)
{
	if (end != NULL)
	{
		return start + i != end;
	}
	else
	{
		return start[i] != '\0';
	}
}

static bool IsLetter(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool IsNumber(char c)
{
	return c >= '0' && c <= '9';
}

static bool IsSpecial(char c)
{
	switch (c)
	{
		case '-':
		case '[':
		case ']':
		case '\\':
		case '`':
		case '^':
		case '{':
		case '}':
			return true;
		default:
			return false;
	}
}

static bool IsNonWhite(char c)
{
	switch (c)
	{
		case ' ':
		case '\0':
		case '\r':
		case '\n':
			return false;
		default:
			return true;
	}
}

static bool IsChString(char c)
{
	switch (c)
	{
		case ' ':
		case '\a': // BELL
		case '\0':
		case '\r':
		case '\n':
		case ',':
			return false;
		default:
			return true;
	}
}

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
	return IrcMsgValidator_ValidateHost(self, origin, originEnd)
		|| IrcMsgValidator_ValidateNick(self, origin, originEnd);
}

bool IrcMsgValidator_ValidateNick(const IrcMsgValidator* self,
		const char* nick, const char* nickEnd)
{
	if (!IsLetter(nick[0]))
	{
		LOG_DEBUG(self->log, "Expected letter at pos 0, got: %c.", nick[0]);
		return false;
	}

	for (size_t i = 1; InBounds(nick, nickEnd, i); i++)
	{
		if (!IsLetter(nick[i]) && !IsNumber(nick[i]) && !IsSpecial(nick[i]))
		{
			LOG_DEBUG(self->log, "Expected letter, number or special at pos %zu, got: %c.",
					i, nick[i]);
			return false;
		}
	}

	return true;
}

bool IrcMsgValidator_ValidateUser(const IrcMsgValidator* self,
		const char* user, const char* userEnd)
{
	if (!IsNonWhite(user[0]))
	{
		LOG_DEBUG(self->log, "Expected nonwhite at pos 0, got: %c.", user[0]);
		return false;
	}

	for (size_t i = 1; InBounds(user, userEnd, i); i++)
	{
		if (!IsNonWhite(user[i]))
		{
			LOG_DEBUG(self->log, "Expected nonwhite at pos %zu, got: %c.",
					i, user[i]);
			return false;
		}
	}

	return true;
}

bool IrcMsgValidator_ValidateHost(const IrcMsgValidator* self,
		const char* host, const char* hostEnd)
{
	if (!IsLetter(host[0]))
	{
		LOG_DEBUG(self->log, "Expected letter at pos 0, got: %c.", host[0]);
		return false;
	}
	
	for (size_t i = 1; InBounds(host, hostEnd, i); i++)
	{
		if (host[i] == '.')
		{
			if (!IsLetter(host[i + 1]))
			{
				LOG_DEBUG(self->log, "Expected letter at pos %zu, got: %c.",
						i + 1, host[i + 1]);
				return false;
			}
		}
		else if (host[i + 1] == '.')
		{
			if (!IsLetter(host[i]) && !IsNumber(host[i]))
			{
				LOG_DEBUG(self->log, "Expected letter or digit at pos %zu, got: %c.",
						i, host[i]);
				return false;
			}
		}
		else if (!IsLetter(host[i]) && !IsNumber(host[i]) && host[i] != '-')
		{
			LOG_DEBUG(self->log, "Expected letter, digit or hyphen at pos %zu, got: %c.",
					i, host[i]);
			return false;
		}
	}

	return true;
}

bool IrcMsgValidator_ValidateServer(const IrcMsgValidator* self,
		const char* server, const char* serverEnd)
{
	return IrcMsgValidator_ValidateHost(self, server, serverEnd);
}

bool IrcMsgValidator_ValidateCommand(const IrcMsgValidator* self,
		const char* command, const char* commandEnd)
{
	if (IsLetter(command[0]))
	{
		for (size_t i = 1; InBounds(command, commandEnd, i); i++)
		{
			if (!IsLetter(command[i]))
			{
				LOG_DEBUG(self->log, "Expected letter at pos %zu, got: %c.",
						i, command[i]);
				return false;
			}
		}
		return true;
	}
	else if (IsNumber(command[0]) && IsNumber(command[1]) && IsNumber(command[3]))
	{
		return true;
	}
	else
	{
		LOG_DEBUG(self->log, "Expected letter or number at pos 0, got: %c.", command[0]);
		return false;
	}
}

bool IrcMsgValidator_ValidateMiddleParam(const IrcMsgValidator* self,
		const char* param, const char* paramEnd)
{
	if (param[0] == ':' || !IsNonWhite(param[0]))
	{
		LOG_DEBUG(self->log, "Expected nonwhite and non ':' at pos 0, got: %c.", param[0]);
		return false;
	}

	for (size_t i = 1; InBounds(param, paramEnd, i); i++)
	{
		if (!IsNonWhite(param[i]))
		{
			LOG_DEBUG(self->log, "Expected nonwhite at pos %zu, got: %c.",
					i, param[i]);
			return false;
		}
	}

	return true;
}

bool IrcMsgValidator_ValidateTrailingParam(const IrcMsgValidator* self,
		const char* param, const char* paramEnd)
{
	for (size_t i = 0; InBounds(param, paramEnd, i); i++)
	{
		if (param[i] == '\0' || param[i] == '\r' || param[i] == '\n')
		{
			LOG_DEBUG(self->log, "Expected non NUL, CR or LF at pos %zu, got: %c.",
					i, param[i]);
			return false;
		}
	}

	return true;
}


bool IrcMsgValidator_ValidateChannel(
		const IrcMsgValidator* self, const char* channel, const char* channelEnd)
{
	return (channel[0] == '#' || channel[0] == '&')
		&& IrcMsgValidator_ValidateChstring(self, channel + 1, channelEnd);
}

bool IrcMsgValidator_ValidateMask(
		const IrcMsgValidator* self, const char* mask, const char* maskEnd)
{
	return (mask[0] == '#' || mask[0] == '$')
		&& IrcMsgValidator_ValidateChstring(self, mask + 1, maskEnd);
}

bool IrcMsgValidator_ValidateChstring(
		const IrcMsgValidator* self, const char* chstring, const char* chstringEnd)
{
	for (size_t i = 0; InBounds(chstring, chstringEnd, i); i++)
	{
		if (!IsChString(chstring[i]))
		{
			LOG_DEBUG(self->log, "Expected chstring at pos %zu, got: %c.",
					i, chstring[i]);
			return false;
		}
	}
	return true;
}

