#include "irc_reply.h"

#include "irc_cmd_type.h"
#include "str_utils.h"

#include <stdlib.h>


static IrcMsg* IrcReply_Base(const char* servername)
{
	IrcMsg* self = malloc(sizeof(IrcMsg));
	if (self == NULL)
	{
		return NULL;
	}

	*self = (IrcMsg) { 0 };

	self->prefix.origin = StrUtils_Clone(servername);
	if (self->prefix.origin == NULL)
	{
		return NULL;
	}

	return self;
}

IrcMsg* IrcReply_ErrNoSuchNick(const char* servername, const char* nickname)
{
	IrcMsg* self = IrcReply_Base(servername);
	if (self == NULL)
	{
		return NULL;
	}

	self->replyNumber = 401;
	self->paramCount = 2;

	self->params[0] = StrUtils_Clone(nickname);
	if (self->params[0] == NULL)
	{
		return NULL;
	}

	self->params[1] = StrUtils_Clone("No such nick/channel");
	if (self->params[1] == NULL)
	{
		return NULL;
	};

	return self;
}

IrcMsg* IrcReply_ErrNickCollision(const char* servername, const char* nickname)
{
	IrcMsg* self = IrcReply_Base(servername);
	if (self == NULL)
	{
		return NULL;
	}

	self->replyNumber = 436;
	self->paramCount = 2;

	self->params[0] = StrUtils_Clone(nickname);
	if (self->params[0] == NULL)
	{
		return NULL;
	}

	self->params[1] = StrUtils_Clone("Nickname collision KILL");
	if (self->params[1] == NULL)
	{
		return NULL;
	};

	return self;
}

IrcMsg* IrcReply_ErrNotRegistered(const char* servername)
{
	IrcMsg* self = IrcReply_Base(servername);
	if (self == NULL)
	{
		return NULL;
	}

	self->replyNumber = 451;
	self->paramCount = 1;

	self->params[0] = StrUtils_Clone("You have not registered");
	if (self->params[0] == NULL)
	{
		return NULL;
	}

	return self;
}

IrcMsg* IrcReply_ErrNeedMoreParams(const char* servername, IrcCmdType cmd)
{
	IrcMsg* self = IrcReply_Base(servername);
	if (self == NULL)
	{
		return NULL;
	}

	self->replyNumber = 461;
	self->paramCount = 2;

	self->params[0] = StrUtils_Clone(IRC_CMD_TYPE_STRS[cmd]);
	if (self->params[0] == NULL)
	{
		return NULL;
	}

	self->params[1] = StrUtils_Clone("Not enough parameters");
	if (self->params[1] == NULL)
	{
		return NULL;
	};

	return self;
}

IrcMsg* IrcReply_ErrAlreadyRegistered(const char* servername)
{
	IrcMsg* self = IrcReply_Base(servername);
	if (self == NULL)
	{
		return NULL;
	}

	self->replyNumber = 462;
	self->paramCount = 1;

	self->params[0] = StrUtils_Clone("You may not reregister");
	if (self->params[0] == NULL)
	{
		return NULL;
	}

	return self;
}

