#include "application.h"
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
#define TIMEOUT 10

static bool StartClient(const Logger* log, TaskQueue* tasks, UserInputQueue* userInput);
static bool SetupSignals(const Logger* log);

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

	SetupSignals(log);

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

	if (!StartClient(log, tasks, userInput))
	{
		LOG_ERROR(log, "Failed to start client!");
		goto cleanup;
	}

	if (!Tui_Run(tui))
	{
		LOG_ERROR(log, "Unexpected error while running TUI");
		goto cleanup;
	}

	result = EXIT_SUCCESS;
	Application_ShouldShutdown();

cleanup:	
	fflush(logFile);

	for (size_t i = 0; i < RUNNER_COUNT; i++)
	{
		TaskRunner_Delete(runners[i]);
	}

	Tui_Delete(tui);
	UserInputQueue_Delete(userInput);
	UserOutputQueue_Delete(userOutput);
	TaskQueue_Delete(tasks);
	Logger_Destroy(log);
	fclose(logFile);

	return result;
}

static bool StartClient(const Logger* log, TaskQueue* tasks, UserInputQueue* userInput)
{
	Task* msgSenderTask = MsgSenderTask_New(log, userInput);
	if (msgSenderTask == NULL)
	{
		LOG_ERROR(log, "Failed to create task to send messages.");
		return false;
	}

	if (!TaskQueue_Push(tasks, msgSenderTask)) 
	{
		LOG_ERROR(log, "Failed to push msg sender task to queue.");
		return false;
	}

	return true;
}

void signalHandler(int signum)
{
	switch (signum)
	{
		case SIGINT:
			break; // Ignore SIGINT
		case SIGTERM:
		case SIGQUIT:
			Application_StartShutdown();
		break;
	}
}

bool SetupSignals(const Logger* log)
{

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
