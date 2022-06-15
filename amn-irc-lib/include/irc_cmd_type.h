#ifndef AMN_IRC_CMD_TYPE_H
#define AMN_IRC_CMD_TYPE_H

typedef enum IrcCmdType
{
	// Not an actual command, just means the variable is empty.
	IrcCmdType_Null = 0,

	// Connection Registration
	// https://datatracker.ietf.org/doc/html/rfc1459#section-4.1
	IrcCmdType_Pass,
	IrcCmdType_Nick,
	IrcCmdType_User,
	IrcCmdType_Server,
	IrcCmdType_Operator,
	IrcCmdType_Quit,
	IrcCmdType_ServerQuit,
	// Channel Operations
	// https://datatracker.ietf.org/doc/html/rfc1459#section-4.2
	IrcCmdType_Join,
	IrcCmdType_Part,
	IrcCmdType_Mode,
	IrcCmdType_Topic,
	IrcCmdType_Names,
	IrcCmdType_List,
	IrcCmdType_Invite,
	IrcCmdType_Kick,
	// Server Queries
	// https://datatracker.ietf.org/doc/html/rfc1459#section-4.3
	IrcCmdType_Version,
	IrcCmdType_Stats,
	IrcCmdType_Links,
	IrcCmdType_Time,
	IrcCmdType_Connect,
	IrcCmdType_Trace,
	IrcCmdType_Admin,
	IrcCmdType_Info,
	// Chat Messages
	// https://datatracker.ietf.org/doc/html/rfc1459#section-4.4
	IrcCmdType_PrivMsg,
	IrcCmdType_Notice,
	// User Queries
	// https://datatracker.ietf.org/doc/html/rfc1459#section-4.5
	IrcCmdType_Who,
	IrcCmdType_Whois,
	IrcCmdType_Whowas,
	// Misc Messages
	// https://datatracker.ietf.org/doc/html/rfc1459#section-4.6
	IrcCmdType_Kill,
	IrcCmdType_Ping,
	IrcCmdType_Pong,
	IrcCmdType_Error,
	// Optional Messages
	// https://datatracker.ietf.org/doc/html/rfc1459#section-5
	IrcCmdType_Away,
	IrcCmdType_Rehash,
	IrcCmdType_Restart,
	IrcCmdType_Summon,
	IrcCmdType_Users,
	IrcCmdType_WallOps,
	IrcCmdType_UserHost,
	IrcCmdType_IsOn,

	// Error Replies
	// https://datatracker.ietf.org/doc/html/rfc1459#section-6.1
	IrcCmdType_Err_NoSuchNick,
	IrcCmdType_Err_NoSuchServer,
	IrcCmdType_Err_NoSuchChannel,
	// TODO: Finish replies

} IrcCmdType;

#endif // AMN_IRC_CMD_TYPE_H

