#include "tui.h"

#include "application.h"
#include "tui_input.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <curses.h>

#define INPUT_WIN_HEIGHT 3
#define INPUT_WIN_WIDTH COLS - 2
#define INPUT_WIN_Y LINES - INPUT_WIN_HEIGHT - 1
#define INPUT_WIN_X 1

#define MIN_LINES INPUT_WIN_HEIGHT + 4
#define MIN_COLS 3

/**
 * Limits how many lines we pull from the queue at max, per update interval.
 * When the client receives a high volume of messages, this limits the
 * speed of message displaying so that it doesn't block the ui from 
 * reading user input.
 */
#define MAX_LINES_PER_UPDATE 100

struct Tui
{
	const Logger* log;
	WINDOW* mainWin;
	WINDOW* chatWin;
	TuiInput* tuiInput;
	UserInputQueue* userInput;
	UserOutputQueue* userOutput;
};

static bool Tui_TryUpdateChat(Tui* self);
static bool Tui_DisplayChatLine(Tui* self, char* line);

Tui* Tui_Create(const Logger* log, UserInputQueue* userInput, UserOutputQueue* userOutput)
{
	Tui* self = malloc(sizeof(Tui));
	if (self == NULL)
	{
		return NULL;
	}

	// Default initialization
	*self = (Tui) {0};
	self->log = log;
	self->userInput = userInput;
	self->userOutput = userOutput;

	WINDOW* win = initscr();
	if (win == NULL)
	{
		LOG_ERROR(log, "Failed to initialize ncurses!");
		Tui_Delete(self);
		return NULL;
	}

	if (LINES < MIN_LINES)
	{
		LOG_ERROR(log, "Terminal doesn't have enough lines");
		Tui_Delete(self);
		return NULL;
	}

	if (COLS < MIN_COLS)
	{
		LOG_ERROR(log, "Terminal doesn't have enough columns");
		Tui_Delete(self);
		return NULL;
	}

	self->mainWin = win;

	if (box(self->mainWin, 0, 0) == ERR)
	{
		LOG_ERROR(log, "Failed to create main window border!");
		Tui_Delete(self);
		return NULL;
	}

	if (mvwhline(self->mainWin, LINES - INPUT_WIN_HEIGHT - 2, 1, 0, COLS - 2))
	{
		LOG_ERROR(log, "Failed to create main window separator!");
		Tui_Delete(self);
		return NULL;
	}

	if (noecho() == ERR)
	{
		LOG_ERROR(log, "Failed to disable ncurses echo!");
		Tui_Delete(self);
		return NULL;
	}

	self->chatWin = newwin(LINES - INPUT_WIN_HEIGHT - 3, COLS - 2, 1, 1);
	if (self->chatWin == NULL)
	{
		LOG_ERROR(log, "Failed to create chat window!");
		Tui_Delete(self);
		return NULL;
	}

	self->tuiInput = TuiInput_New(
			log, userInput, INPUT_WIN_HEIGHT, INPUT_WIN_WIDTH, INPUT_WIN_Y, INPUT_WIN_X);
	if (self->tuiInput == NULL)
	{
		LOG_ERROR(log, "Failed to create input window!");
		Tui_Delete(self);
		return NULL;
	}

	return self;
}

void Tui_Delete(Tui* self)
{
	if (self == NULL)
	{
		return;
	}

	TuiInput_Delete(self->tuiInput);

	if (delwin(self->chatWin) == ERR)
	{
		LOG_ERROR(self->log, "Failed to free chat window!");
	}

	if (delwin(self->mainWin) == ERR)
	{
		LOG_ERROR(self->log, "Failed to free main window!");
	}

	if (endwin() == ERR)
	{
		LOG_ERROR(self->log, "Failed to cleanup ncurses state!");
	}

	free(self);
}

bool Tui_Run(Tui* self)
{
	if (wrefresh(self->mainWin) == ERR)
	{
		return false;
	}
	
	if (wprintw(self->chatWin, "Hello world!") == ERR)
	{
		return false;
	}

	if (wrefresh(self->chatWin) == ERR)
	{
		return false;
	}
	
	while (!Application_ShouldShutdown())
	{
		switch (TuiInput_Run(self->tuiInput))
		{
			case TaskStatus_Yield:
				break;
			case TaskStatus_Done:
				return true;
			case TaskStatus_Failed:
				return false;
		}

		if (!Tui_TryUpdateChat(self))
		{
			return false;
		}
	}

	return true;
}

static bool Tui_TryUpdateChat(Tui* self)
{
	for(int i = 0; i < MAX_LINES_PER_UPDATE; i++) 
	{
		char* line = NULL;
		switch (UserOutputQueue_TryPop(self->userOutput, &line))
		{
			case Queue_TryPopResult_Ok:
				if (!Tui_DisplayChatLine(self, line))
				{
					LOG_ERROR(self->log, "Failed to display chat line!");
					return false;
				}
				break;
			case Queue_TryPopResult_Empty:
				// Nothing to do for now.
				return true;
			case Queue_TryPopResult_Error:
				LOG_ERROR(self->log, "Failed to pop user output from queue!");
				return false;
		}
	}

	return true;
}

static bool Tui_DisplayChatLine(Tui* self, char* line)
{
	if (wprintw(self->chatWin, "%s", line) == ERR)
	{
		return false;
	}

	return true;
}
