#include "msg_sender_task.h"

#include "application.h"
#include "irc_cmd_unparser.h"
#include "irc_msg_unparser.h"
#include "irc_msg_writer.h"
#include "slash_cmd_parser.h"
#include "str_utils.h"
#include "task.h"
#include "user_input_queue.h"

#include <errno.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

#define PROTOCOL_IP 0
#define SERVER_PORT "6667"


typedef struct MsgSenderContext
{
	const Logger* log;
	UserInputQueue* userInput;
	
	int socket;
	SlashCmdParser* slashCmdParser;
	IrcMsgValidator* msgValidator;
	IrcCmdUnparser* cmdUnparser;
	IrcMsgUnparser* msgUnparser;
	IrcMsgWriter* writer;

	bool success;
}
MsgSenderContext;

static MsgSenderContext* MsgSenderContext_New(const Logger* log, UserInputQueue* userInput);
static TaskStatus MsgSenderContext_Run(void* ctx);
static void MsgSenderTaskContext_Delete(void* ctx);

static void HandleCommmand(MsgSenderContext* ctx, char* cmdLine);

Task* MsgSenderTask_New(const Logger* log, UserInputQueue* userInput)
{
	MsgSenderContext* ctx = MsgSenderContext_New(log, userInput);
	if (ctx == NULL)
	{
		LOG_ERROR(log, "Failed to create MsgSenderContext");
		return NULL;
	}

	Task* self = Task_Create(MsgSenderContext_Run, ctx, MsgSenderTaskContext_Delete); 
	if (self == NULL)
	{
		free(ctx);
		return NULL;
	}

	return self;
}

static MsgSenderContext* MsgSenderContext_New(const Logger* log, UserInputQueue* userInput)
{
	MsgSenderContext* self = malloc(sizeof(MsgSenderContext));
	if (self == NULL)
	{
		LOG_ERROR(log, "Failed to allocate MsgSenderContext");
		return NULL;
	}
	*self = (MsgSenderContext) {
		.log = log,
		.userInput = userInput,
		.socket = -1,
		.writer = NULL
	};

	self->slashCmdParser = SlashCmdParser_New(log);
	if (self->slashCmdParser == NULL)
	{
		LOG_ERROR(log, "Failed to create SlashCmdParser");
		goto error;
	}

	self->msgValidator = IrcMsgValidator_New(log);
	if (self->msgValidator == NULL)
	{
		LOG_ERROR(log, "Failed to create IrcMsgValidator.");
		goto error;
	}

	self->cmdUnparser = IrcCmdUnparser_New(log, self->msgValidator);
	if (self->cmdUnparser == NULL)
	{
		LOG_ERROR(log, "Failed to create IrcCmdUnparser.");
		goto error;
	}

	self->msgUnparser = IrcMsgUnparser_New(log);
	if (self->msgUnparser == NULL)
	{
		LOG_ERROR(log, "Failed to create IrcCmdUnparser.");
		goto error;
	}

	return self;

error:
	MsgSenderTaskContext_Delete(self);
	return NULL;
}

static void MsgSenderTaskContext_Delete(void* arg)
{
	MsgSenderContext* ctx = (MsgSenderContext*) arg;	

	IrcMsgWriter_Delete(ctx->writer);
	IrcMsgUnparser_Delete(ctx->msgUnparser);
	IrcCmdUnparser_Delete(ctx->cmdUnparser);
	IrcMsgValidator_Delete(ctx->msgValidator);
	SlashCmdParser_Delete(ctx->slashCmdParser);

	if (ctx->socket != -1)
	{
		if (close(ctx->socket) != 0)
		{
			LOG_ERROR(ctx->log, "Failed to close socket!");
		}
	}

	free(ctx);
}

static TaskStatus MsgSenderContext_Run(void* arg)
{
	MsgSenderContext* ctx = (MsgSenderContext*) arg;	

	ctx->success = true;

	char* line = UserInputQueue_Pop(ctx->userInput);
	if (line == NULL && errno == EAGAIN)
	{
		return TaskStatus_Yield;
	}
	else if (line == NULL)
	{
		LOG_ERROR(ctx->log, "Failed to pop user input from queue!");
		return TaskStatus_Failed;
	}

	if (line[0] == '/')
	{
		LOG_DEBUG(ctx->log, "Got cmd to execute: %s", line);
		HandleCommmand(ctx, line);
	}
	else
	{
		LOG_DEBUG(ctx->log, "Got message to send: %s", line);
	}

	if (!ctx->success)
	{
		return TaskStatus_Failed;
	}
	else
	{
		return TaskStatus_Yield;
	}
}

static struct addrinfo* ResolveServer(MsgSenderContext* ctx, char* address)
{
	struct addrinfo hints = {
		// Find an IPv6 address
		.ai_family = AF_INET6,
		// for stream socket
		.ai_socktype = SOCK_STREAM,
		// to bind a socket to accept connections
		.ai_flags = AI_PASSIVE | AI_NUMERICSERV,
		// using TCP/IP
		.ai_protocol = IPPROTO_TCP,
	};

	struct addrinfo* result = NULL;

	if(getaddrinfo(address, SERVER_PORT, &hints, &result) == 0)
	{
		return result;
	}
	else
	{
		LOG_ERROR(ctx->log, "Failed to get addrinfo.");
		ctx->success = false;
		return NULL;
	}
}

static void SetupSocket(MsgSenderContext* ctx, struct addrinfo* address)
{
	LOG_DEBUG(ctx->log, "Creating socket");

	ctx->socket = socket(AF_INET6, SOCK_STREAM, PROTOCOL_IP);
	if (ctx->socket == -1)
	{
		LOG_ERROR(ctx->log, "Failed to create socket");
		ctx->success = false;
		return;
	}

	LOG_DEBUG(ctx->log, "Connecting socket");

	if(connect(ctx->socket, address->ai_addr, address->ai_addrlen) == -1)
	{
		LOG_ERROR(ctx->log, "Failed to connect to address");
		ctx->success = false;
		return;
	}
}

static void RegisterNick(MsgSenderContext* ctx)
{
	IrcCmd nickCmd = {
		.prefix = { 0 },
		.type = IrcCmdType_Nick,
		.nick = {
			.nickname = "fixme" // FIXME
		}
	};

	IrcMsg* msg = IrcCmdUnparser_Unparse(ctx->cmdUnparser, &nickCmd);
	if (msg == NULL)
	{
		LOG_ERROR(ctx->log, "Failed to unparse command");
		ctx->success = false;
		return;
	}

	const char* rawMsg = IrcMsgUnparser_Unparse(ctx->msgUnparser, msg);
	IrcMsg_Delete(msg);
	if (rawMsg == NULL)
	{
		LOG_ERROR(ctx->log, "Failed to unparse message");
		ctx->success = false;
		return;
	}

	if (!IrcMsgWriter_Write(ctx->writer, rawMsg))
	{
		LOG_ERROR(ctx->log, "Failed to send message");
		ctx->success = false;
		return;
	}
}

static void RegisterUser(MsgSenderContext* ctx)
{
	long loginNameMax = sysconf(_SC_LOGIN_NAME_MAX);
	if (loginNameMax < 0)
	{
		LOG_ERROR(ctx->log, "Failed to determine max username length");
		ctx->success = false;
		return;
	}

	char username[loginNameMax];
	if (getlogin_r(username, (size_t) loginNameMax) != 0)
	{
		LOG_ERROR(ctx->log, "Failed to get username");
		ctx->success = false;
		return;
	}

	long hostNameMax = sysconf(_SC_HOST_NAME_MAX);
	if (hostNameMax < 0)
	{
		LOG_ERROR(ctx->log, "Failed to determine max hostname length");
		ctx->success = false;
		return;
	}

	char hostname[hostNameMax];
	if (gethostname(hostname, (size_t) hostNameMax) != 0)
	{
		LOG_ERROR(ctx->log, "Failed to get hostname");
		ctx->success = false;
		return;
	}
	hostname[hostNameMax - 1] = '\0'; // Name might be truncated and missing the terminator

	IrcCmd userCmd = {
		.prefix = { 0 },
		.type = IrcCmdType_User,
		.user = {
			.username = username,
			.hostname = hostname,
			.servername = "servername", // Server must ignore this from a client anyway
			.realname = username, // FIXME
		}
	};

	IrcMsg* msg = IrcCmdUnparser_Unparse(ctx->cmdUnparser, &userCmd);
	if (msg == NULL)
	{
		LOG_ERROR(ctx->log, "Failed to unparse command");
		ctx->success = false;
		return;
	}

	const char* rawMsg = IrcMsgUnparser_Unparse(ctx->msgUnparser, msg);
	IrcMsg_Delete(msg);
	if (rawMsg == NULL)
	{
		LOG_ERROR(ctx->log, "Failed to unparse message");
		ctx->success = false;
		return;
	}

	if (!IrcMsgWriter_Write(ctx->writer, rawMsg))
	{
		LOG_ERROR(ctx->log, "Failed to send message");
		ctx->success = false;
		return;
	}
}

static void ExecuteCmdConnect(MsgSenderContext* ctx, SlashCmd* cmd)
{
	if (ctx->socket != -1)
	{
		LOG_WARN(ctx->log, "Already connected to a server");
		return;
	}

	struct addrinfo* address = ResolveServer(ctx, cmd->param);
	if(address == NULL)
	{
		return;
	}

	SetupSocket(ctx, address);
	freeaddrinfo(address);
	if(!ctx->success)
	{
		return;
	}

	ctx->writer = IrcMsgWriter_New(ctx->log, ctx->socket);
	if (ctx->writer == NULL)
	{
		LOG_ERROR(ctx->log, "Failed to create IrcMsgWriter.");
		ctx->success = false;
		return;
	}

	RegisterNick(ctx);
	if(!ctx->success)
	{
		return;
	}

	RegisterUser(ctx);
	if(!ctx->success)
	{
		return;
	}
}

static void HandleCommmand(MsgSenderContext* ctx, char* cmdLine)
{
	SlashCmd* slashCmd = SlashCmdParser_Parse(ctx->slashCmdParser, cmdLine);
	if (slashCmd == NULL)
	{
		LOG_ERROR(ctx->log, "Failed to unparse slash cmd!");
		ctx->success = false;
		return;
	}

	switch (slashCmd->type)
	{
		case SlashCmdType_Connect:
			ExecuteCmdConnect(ctx, slashCmd);
			break;
		case SlashCmdType_Quit:
		case SlashCmdType_Ping:
		case SlashCmdType_Join:
		case SlashCmdType_Nickname:
		case SlashCmdType_Kick:
		case SlashCmdType_Mute:
		case SlashCmdType_Unmute:
		case SlashCmdType_Whois:
			LOG_ERROR(ctx->log, "Unimplemented command.");
			return;
		case SlashCmdType_Null:
			LOG_ERROR(ctx->log, "Invalid command.");
			ctx->success = false;
			return;
	}
}
