#include "irc_msg.h"

#include "str_utils.h"

#include <stdlib.h>

bool IrcMsgPrefix_Clone(const IrcMsgPrefix* self, IrcMsgPrefix* clone)
{
	if (self->hostname != NULL)
	{
		clone->hostname = StrUtils_Clone(self->hostname);
		if (clone->hostname == NULL)
		{
			return false;	
		}
	}
	
	if (self->origin != NULL)
	{
		clone->origin = StrUtils_Clone(self->origin);
		if (clone->origin == NULL)
		{
			return false;	
		}
	}

	if (self->username != NULL)
	{
		clone->username = StrUtils_Clone(self->origin);
		if (clone->username == NULL)
		{
			return false;	
		}
	}

	return true;
}

void IrcMsg_Delete(IrcMsg* self)
{
	if (self == NULL)
	{
		return;
	}

	free(self->prefix.origin);
	free(self->prefix.username);
	free(self->prefix.hostname);

	for (size_t i = 0; i < self->paramCount; i++)
	{
		free(self->params[i]);
	}


	free(self);
}
