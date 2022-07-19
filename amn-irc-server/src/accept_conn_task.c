#include "accept_conn_task.h"

#include "receive_msg_task.h"

#include <errno.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <unistd.h>

typedef struct AcceptConnContext
{
	const Logger* log;
	TaskQueue* tasks;
	IrcCmdQueue* cmds;
	int socket;
}
AcceptConnContext;

static TaskStatus WaitForConnections(void* context);
static void DeleteContext(void* context);

Task* AcceptConnTask_New(const Logger* log, TaskQueue* tasks, IrcCmdQueue* cmds, int socket)
{
	AcceptConnContext* context = malloc(sizeof(AcceptConnContext));
	if (context == NULL)
	{
		return NULL;
	}

	context->log = log;
	context->tasks = tasks;
	context->cmds = cmds;
	context->socket = socket;

	Task* self = Task_Create(WaitForConnections, context, DeleteContext);
	if (self == NULL)
	{
		free(context);
		return NULL;
	}

	return self;
}

static void DeleteContext(void* arg)
{
	AcceptConnContext* ctx = (AcceptConnContext*) arg;

	if (close(ctx->socket) != 0)
	{
		LOG_ERROR(ctx->log, "Failed to close listen socket.");
	}
	
	free(ctx);
}

static TaskStatus WaitForConnections(void* arg)
{
	AcceptConnContext* ctx = (AcceptConnContext*) arg;

	// LOG_DEBUG(ctx->log, "Waiting for connection");

	int clientSocket = accept(ctx->socket, NULL, NULL);
	if (clientSocket == -1 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR))
	{
		// Socket timeout
		// LOG_DEBUG(ctx->log, "Timeout while accepting connections.");
		errno = 0;
		return TaskStatus_Yield;
	}
	else if (clientSocket == -1)
	{
		LOG_ERROR(ctx->log, "Failed to accept connection.");
		return TaskStatus_Failed;
	}

	Task* receiveTask = ReceiveMsgTask_New(
			ctx->log, ctx->tasks, ctx->cmds, clientSocket);	
	if (receiveTask == NULL)
	{
		LOG_ERROR(ctx->log, "Failed to create ReceiveMsgTask.");

		if (close(ctx->socket) != 0)
		{
			LOG_ERROR(ctx->log, "Failed to close client socket.");
		}

		return TaskStatus_Failed;
	}

	if (!TaskQueue_Push(ctx->tasks, receiveTask))
	{
		LOG_ERROR(ctx->log, "Failed to add task to queue.");
		return TaskStatus_Failed;
	}

	return TaskStatus_Yield;
}
