#include "log.h"
#include "tui.h"
#include "task.h"
#include "task_runner.h"
#include "task_queue.h"
#include "user_input_queue.h"
#include "msg_sender_task.h"

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define RUNNER_COUNT 10
#define PROTOCOL_IP 0
#define SERVER_PORT "6667"
#define TIMEOUT 10

int main()
{
	int result = EXIT_FAILURE;

	FILE* logFile = NULL;
	Logger* log = NULL;
	TaskQueue* tasks = NULL;
	UserInputQueue* userInput = NULL;
	UserOutputQueue* userOutput = NULL;
	TaskRunner* runners[RUNNER_COUNT] = {0};
	Tui* tui = NULL;

	logFile = fopen("amn-irc-client.log", "w");
	if (logFile == NULL)
	{
		fprintf(stderr, "Failed to create logfile amn-irc-client.log!");
		return EXIT_FAILURE;
	}

	log = Logger_Create(&logFile, 1); 
	if (log == NULL)
	{
		fprintf(stderr, "Failed to create logger!");
		goto cleanup;
	}

	LOG_INFO(log, "Starting amn-irc-client...");

	tasks = TaskQueue_New(10, TIMEOUT);
	if (tasks == NULL)
		goto cleanup;

	userInput = UserInputQueue_New(50, TIMEOUT);
	if (userInput == NULL)
		goto cleanup;

	userOutput = UserOutputQueue_New(200, TIMEOUT);
	if (userOutput == NULL)
		goto cleanup;

	tui = Tui_Create(log, userInput, userOutput);
	if (tui == NULL)
	{
		LOG_ERROR(log, "Failed to create TUI.");
		goto cleanup;
	}

	for (size_t i = 0; i < RUNNER_COUNT; i++)
	{
		runners[i] = TaskRunner_New(log, tasks);
		if (runners[i] == NULL)
		{
			LOG_ERROR(log, "Failed to create runner %zu.", i);
			goto cleanup;
		}
	}

	if (!Tui_Run(tui))
	{
		LOG_ERROR(log, "Unexpected error while running TUI");
		goto cleanup;
	}

	result = EXIT_SUCCESS;

cleanup:	

	for (size_t i = 0; i < RUNNER_COUNT; i++)
	{
		TaskRunner_Delete(runners[i]);
	}

	Tui_Delete(tui);
	UserInputQueue_Delete(userInput);
	TaskQueue_Delete(tasks);
	Logger_Destroy(log);
	fclose(logFile);

	return result;
}

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

	int connectSocket = socket(AF_INET6, SOCK_STREAM, PROTOCOL_IP);
	if (connectSocket == -1)
	{
		LOG_ERROR(log, "Failed to create socket");
		return -1;
	}

	LOG_DEBUG(log, "Connecting socket");

	if(connect(connectSocket, address->ai_addr, address->ai_addrlen) == -1)
	{
		LOG_ERROR(log, "Failed to connect to address");
		return -1;
	}

	return connectSocket;
}

bool StartServer(const Logger* log, TaskQueue* tasks, UserInputQueue* userInput)
{
	struct addrinfo* address = getServerAddress(log);
	if(address == NULL)
		return false;

	int clientSocket = setupSocket(log, address);
	freeaddrinfo(address);
	if(clientSocket == -1)
		return false;

	Task* msgSenderTask = MsgSenderTask_New(log, userInput, clientSocket);
	if (msgSenderTask == NULL)
	{
		LOG_ERROR(log, "Failed to create task to send messages.");
		if (close(clientSocket) != 0)
		{
			LOG_ERROR(log, "Failed to close client socket.");
		}
		return false;
	}

	if (!TaskQueue_Push(tasks, msgSenderTask)) 
	{
		LOG_ERROR(log, "Failed to push msg sender task to queue.");
		return false;
	}

	return true;
}
