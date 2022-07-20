#ifndef AMN_SLASH_CMD_H
#define AMN_SLASH_CMD_H

typedef enum SlashCmdType
{
	SlashCmdType_Null,
	SlashCmdType_Connect,
	SlashCmdType_Quit,
	SlashCmdType_Ping,
	SlashCmdType_Join,
	SlashCmdType_Nickname,
	SlashCmdType_Kick,
	SlashCmdType_Mute,
	SlashCmdType_Unmute,
	SlashCmdType_Whois,
}
SlashCmdType;

typedef struct SlashCmd
{
	SlashCmdType type;
	char* param;
}
SlashCmd;

void SlashCmd_Delete(SlashCmd* self);

#endif
