#include "msg_sender_task.h"

#include "irc_msg_writer.h"
#include "user_input_queue.h"

#include <stdlib.h>

typedef struct MsgSenderContext
{
	const Logger* log;
	UserInputQueue* userInput;
	int socket;
	
	IrcMsgWriter* writer;
}
MsgSenderContext;

static MsgSenderContext* MsgSenderContext_New(const Logger* log, UserInputQueue* userInput, int socket);
static void MsgSenderContext_Run(void* ctx);

Task* MsgSenderTask_New(const Logger* log, UserInputQueue* userInput, int socket)
{
	MsgSenderContext* ctx = MsgSenderContext_New(log, userInput, socket);
	if (ctx == NULL)
	{
		LOG_ERROR(log, "Failed to create MsgSenderContext");
		return NULL;
	}

	Task* self = Task_Create(MsgSenderContext_Run, ctx, free); 
	if (self == NULL)
	{
		free(ctx);
		return NULL;
	}

	return self;
}

static MsgSenderContext* MsgSenderContext_New(const Logger* log, UserInputQueue* userInput, int socket)
{
	MsgSenderContext* self = malloc(sizeof(MsgSenderContext));
	if (self == NULL)
	{
		LOG_ERROR(log, "Failed to allocate MsgSenderContext");
		return NULL;
	}
	
	self->userInput = userInput;
	self->socket = socket;

	self->writer = IrcMsgWriter_New(log, socket);
	if (self->writer == NULL)
	{
		LOG_ERROR(log, "Failed to create IrcMsgWriter.");
		free(self);
		return NULL;
	}

	return self;
}

static void MsgSenderContext_Run(void* arg)
{
	MsgSenderContext* ctx = (MsgSenderContext*) arg;	

	char* userInput = UserInputQueue_Pop(ctx->userInput);
	if (userInput == NULL)
	{
		LOG_ERROR(ctx->log, "Failed to pop user input from queue!");
		return;
	}


	if (!IrcMsgWriter_Write(ctx->writer, userInput))
	{
		LOG_ERROR(ctx->log, "Failed to write message to socket!");
		return;
	}
}
