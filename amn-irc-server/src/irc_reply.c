#include "irc_reply.h"

#include "irc_cmd_type.h"
#include "str_utils.h"

#include <stdlib.h>
#include <string.h>


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

static char* WriteChannel(IrcChannelType channelType, const char* channelName)
{
	size_t len = strlen(channelName) + 1;

	char* param = malloc(sizeof(char) * (len + 1));
	if (param == NULL)
	{
		return NULL;
	}

	switch (channelType)
	{
		case IrcChannelType_Local:
			param[0] = '&';
			break;
		case IrcChannelType_Distributed:
			param[0] = '#';
			break;
	}
	memcpy(param + 1, channelName, len - 1);
	param[len] = '\0';

	return param;
}

IrcMsg* IrcReply_RplTopic(
		const char* servername, IrcChannelType channelType, const char* channel,
		const char* topic)
{
	IrcMsg* self = IrcReply_Base(servername);
	if (self == NULL)
	{
		return NULL;
	}

	self->replyNumber = 332;
	self->paramCount = 2;

	self->params[0] = WriteChannel(channelType, channel);
	if (self->params[0] == NULL)
	{
		return NULL;
	}

	self->params[1] = StrUtils_Clone(topic);
	if (self->params[1] == NULL)
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

IrcMsg* IrcReply_ErrChannelIsFull(
		const char* servername, IrcChannelType channelType, const char* channelName)
{
	IrcMsg* self = IrcReply_Base(servername);
	if (self == NULL)
	{
		return NULL;
	}
	
	self->replyNumber = 471;
	self->paramCount = 2;

	self->params[0] = WriteChannel(channelType, channelName);
	if (self->params[0] == NULL)
	{
		return NULL;
	}

	self->params[1] = StrUtils_Clone("Cannot join channel (+l)");
	if (self->params[1] == NULL)
	{
		return NULL;
	}

	return self;
}

IrcMsg* IrcReply_ErrInviteOnlyChan(
		const char* servername, IrcChannelType channelType, const char* channelName)
{
	IrcMsg* self = IrcReply_Base(servername);
	if (self == NULL)
	{
		return NULL;
	}
	
	self->replyNumber = 473;
	self->paramCount = 2;

	self->params[0] = WriteChannel(channelType, channelName);
	if (self->params[0] == NULL)
	{
		return NULL;
	}

	self->params[1] = StrUtils_Clone("Cannot join channel (+i)");
	if (self->params[1] == NULL)
	{
		return NULL;
	}

	return self;
}

IrcMsg* IrcReply_ErrBannedFromChan(
		const char* servername, IrcChannelType channelType, const char* channelName)
{
	IrcMsg* self = IrcReply_Base(servername);
	if (self == NULL)
	{
		return NULL;
	}
	
	self->replyNumber = 474;
	self->paramCount = 2;

	self->params[0] = WriteChannel(channelType, channelName);
	if (self->params[0] == NULL)
	{
		return NULL;
	}

	self->params[1] = StrUtils_Clone("Cannot join channel (+b)");
	if (self->params[1] == NULL)
	{
		return NULL;
	}

	return self;
}

IrcMsg* IrcReply_ErrBadChannelKey(
		const char* servername, IrcChannelType channelType, const char* channelName)
{
	IrcMsg* self = IrcReply_Base(servername);
	if (self == NULL)
	{
		return NULL;
	}
	
	self->replyNumber = 475;
	self->paramCount = 2;

	self->params[0] = WriteChannel(channelType, channelName);
	if (self->params[0] == NULL)
	{
		return NULL;
	}

	self->params[1] = StrUtils_Clone("Cannot join channel (+k)");
	if (self->params[1] == NULL)
	{
		return NULL;
	}

	return self;
}
