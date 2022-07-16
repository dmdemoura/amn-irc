#include "application.h"
#include "log.h"
#include "irc_msg.h"
#include "irc_msg_parser.h"
#include "irc_msg_validator.h"
#include "task_queue.h"
#include "task_runner.h"
#include "accept_conn_task.h"
#include "irc_cmd_executor_task.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define RUNNER_COUNT 10

const int PROTOCOL_IP = 0;
const char* SERVER_PORT = "6667";
const int32_t TIMEOUT = 10;

struct addrinfo* getServerAddress(const Logger* log)
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

	if(getaddrinfo(NULL, SERVER_PORT, &hints, &result) == 0)
	{
		return result;
	}
	else
	{
		LOG_ERROR(log, "Failed to get addrinfo.");
		return NULL;
	}
}

int setupSocket(const Logger* log, struct addrinfo* address)
{
	LOG_DEBUG(log, "Creating socket");

	int listenSocket = socket(AF_INET6, SOCK_STREAM, PROTOCOL_IP);
	if (listenSocket == -1)
	{
		LOG_ERROR(log, "Failed to create socket");
		return -1;
	}

	struct timeval timeout = { .tv_sec = TIMEOUT };

	if (setsockopt(listenSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval))
			== -1)
	{
		LOG_ERROR(log, "Failed to set socket receive timeout");
		return -1;
	}

	LOG_DEBUG(log, "Binding socket");

	if(bind(listenSocket, address->ai_addr, address->ai_addrlen) == -1)
	{
		LOG_ERROR(log, "Failed to bind to address");
		return -1;
	}

	LOG_DEBUG(log, "Setting socket to listen");
	if(listen(listenSocket, 10) == -1) {
		LOG_ERROR(log, "Failed to listen socket.");
		return -1;
	}

	return listenSocket;
}

bool StartServer(const Logger* log, TaskQueue* tasks, IrcCmdQueue* cmds)
{
	struct addrinfo* address = getServerAddress(log);
	if(address == NULL)
		return false;

	int listenSocket = setupSocket(log, address);
	freeaddrinfo(address);
	if(listenSocket == -1)
		return false;

	Task* acceptConnTask = AcceptConnTask_New(log, tasks, cmds, listenSocket);
	if (acceptConnTask == NULL)
	{
		LOG_ERROR(log, "Failed to create task to accept connections.");
		if (close(listenSocket) != 0)
		{
			LOG_ERROR(log, "Failed to close listen socket.");
		}
		return false;
	}

	if (!TaskQueue_Push(tasks, acceptConnTask)) 
	{
		LOG_ERROR(log, "Failed to push accept task to queue.");
		Task_Delete(acceptConnTask);
		return false;
	}

	return true;
}

void signalHandler(int signum)
{
	switch (signum)
	{
		case SIGINT:
		case SIGTERM:
		case SIGQUIT:
			Application_StartShutdown();
		break;
	}
}

bool setupSignals(const Logger* log) {

	struct sigaction act =
	{
		.sa_handler = signalHandler 
	};

	if (sigemptyset(&act.sa_mask) != 0)
	{
		LOG_ERROR(log, "Failed to set empty signal mask!");
		return false;
	}

	if (sigaction(SIGINT, &act, NULL) != 0)
	{
		LOG_ERROR(log, "Failed to register SIGINT handler!");
		return false;
	}

	if (sigaction(SIGTERM, &act, NULL) != 0)
	{
		LOG_ERROR(log, "Failed to register SIGTERM signal handler!");
		return false;
	}

	if (sigaction(SIGQUIT, &act, NULL) != 0)
	{
		LOG_ERROR(log, "Failed to register SIGQUIT handler!");
		return false;
	}

	return true;
}

int main()
{
	int returnCode = EXIT_FAILURE;
	Logger* log = Logger_Create(&stdout, 1);
	TaskQueue* tasks = NULL;
	TaskRunner* runners[RUNNER_COUNT] = {0};
	IrcCmdQueue* cmds = NULL;

	LOG_INFO(log, "Server starting");

	if (!setupSignals(log))
		goto cleanup;

	tasks = TaskQueue_New(256, TIMEOUT);
	if (tasks == NULL)
		goto cleanup;

	cmds = IrcCmdQueue_New(1024, TIMEOUT);
	if (cmds == NULL)
		goto cleanup;

	for (size_t i = 0; i < RUNNER_COUNT; i++)
	{
		runners[i] = TaskRunner_New(log, tasks);
		if (runners[i] == NULL)
		{
			LOG_ERROR(log, "Failed to create runner %zu.", i);
			goto cleanup;
		}
	}

	Task* cmdExecutorTask = IrcCmdExecutorTask_New(log, tasks, cmds);
	if (cmdExecutorTask == NULL)
	{
		LOG_ERROR(log, "Failed to create command executor task.");
		goto cleanup;
	}

	if (!TaskQueue_Push(tasks, cmdExecutorTask)) 
	{
		LOG_ERROR(log, "Failed to push command executor task to queue.");
		Task_Delete(cmdExecutorTask);
		goto cleanup;
	}

	if(!StartServer(log, tasks, cmds))
		goto cleanup;


	returnCode = EXIT_SUCCESS;

cleanup:

	for (size_t i = 0; i < RUNNER_COUNT; i++)
	{
		TaskRunner_Delete(runners[i]);
	}

	IrcCmdQueue_Delete(cmds);

	TaskQueue_Delete(tasks);
	Logger_Destroy(log);

	return returnCode;
}
