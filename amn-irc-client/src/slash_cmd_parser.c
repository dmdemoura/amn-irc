#include "slash_cmd_parser.h"

#include "slash_cmd_map.h"
#include "str_utils.h"

#include <stdlib.h>
#include <string.h>

struct SlashCmdParser
{
	const Logger* log;
};

SlashCmdParser* SlashCmdParser_New(const Logger* log)
{
	SlashCmdParser* self = malloc(sizeof(SlashCmdParser));
	if (self == NULL)
	{
		LOG_ERROR(log, "Failed to allocated SlashCmdParser");
		return NULL;
	}

	*self = (SlashCmdParser) {
		.log = log
	};

	return self;
}

void SlashCmdParser_Delete(SlashCmdParser* self)
{
	free(self);
}


SlashCmd* SlashCmdParser_Parse(SlashCmdParser* self, const char* line)
{
	if (*line != '/')
	{
		LOG_ERROR(self->log, "Expected '/' at position: 0");
		return NULL;
	}

	line++;

	if (*line == '\0')
	{
		LOG_ERROR(self->log, "Expected <command> after '/' at position: 1");
		return NULL;
	}

	const char* cmdEnd = StrUtils_FindFirst(line, " ");

	if (cmdEnd == NULL)
	{
		LOG_ERROR(self->log, "Expected <SPACE> after <command>");
		return NULL;
	}

	LOG_DEBUG(self->log, "Got command: '%.*s'", (size_t) (cmdEnd - line), line); 
	
	SlashCmdType cmdType = SlashCmdType_FromStr(line, (size_t) (cmdEnd - line));
	if (cmdType == SlashCmdType_Null)
	{
		LOG_ERROR(self->log, "Invalid command");
		return NULL;
	}

	line = cmdEnd;
	while(*line == ' ')
	{
		line += 1;
	}

	switch (cmdType)
	{
		case SlashCmdType_Quit:
		case SlashCmdType_Ping:
			if (*line != '\0')
			{
				LOG_ERROR(self->log, "Didn't expect parameter for command");
				return NULL;
			}
			break;
		case SlashCmdType_Connect:
		case SlashCmdType_Join:
		case SlashCmdType_Nickname:
		case SlashCmdType_Kick:
		case SlashCmdType_Mute:
		case SlashCmdType_Unmute:
		case SlashCmdType_Whois:
			if (*line == '\0')
			{
				LOG_ERROR(self->log, "Expected parameter for command");
				return NULL;
			}
			break;
		case SlashCmdType_Null:
			LOG_ERROR(self->log, "Invalid command");
			return NULL;
	}

	SlashCmd* cmd = malloc(sizeof(SlashCmd));
	if (cmd == NULL)
	{
		LOG_ERROR(self->log, "Failed to allocate slash cmd");
		return NULL;
	}
	
	*cmd = (SlashCmd) {
		.type = cmdType,
		.param = StrUtils_Clone(line)
	};

	if (cmd->param == NULL)
	{
		LOG_ERROR(self->log, "Failed to clone slash cmd parameter");
		return NULL;
	}

	return cmd;
}
