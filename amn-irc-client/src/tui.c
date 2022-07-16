#include "tui.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#include <curses.h>

#define INPUT_WIN_HEIGHT 3

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
	WINDOW* inputWin;
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

	self->mainWin = win;

	if (noecho() == ERR)
	{
		LOG_ERROR(log, "Failed to disable ncurses echo!");
		Tui_Delete(self);
		return NULL;
	}

	self->chatWin = newwin(LINES - INPUT_WIN_HEIGHT, COLS, 0, 0);
	if (self->chatWin == NULL)
	{
		LOG_ERROR(log, "Failed to create chat window!");
		Tui_Delete(self);
		return NULL;
	}

	self->inputWin = newwin(INPUT_WIN_HEIGHT, COLS, LINES - INPUT_WIN_HEIGHT, 0);
	if (self->inputWin == NULL)
	{
		LOG_ERROR(log, "Failed to create input window!");
		Tui_Delete(self);
		return NULL;
	}

	if (keypad(self->inputWin, TRUE) == ERR)
	{
		LOG_ERROR(log, "Failed to enable ncurses keypad!");
		Tui_Delete(self);
		return NULL;
	}

	wtimeout(self->inputWin, 100);

	if (box(self->inputWin, 0, 0) == ERR)
	{
		LOG_ERROR(log, "Failed to create input window border!");
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

	if (delwin(self->chatWin) == ERR)
	{
		LOG_ERROR(self->log, "Failed to free chat window!");
	}

	if (delwin(self->inputWin) == ERR)
	{
		LOG_ERROR(self->log, "Failed to free input window!");
	}

	if (endwin() == ERR)
	{
		LOG_ERROR(self->log, "Failed to cleanup ncurses state!");
	}

	free(self);
}

bool Tui_Run(Tui* self)
{
	
	if (wprintw(self->chatWin, "Hello world!") == ERR)
	{
		return false;
	}

	if (wrefresh(self->chatWin) == ERR)
	{
		return false;
	}

	if (wmove(self->inputWin, 1, 1) == ERR)
	{
		return false;
	}

	while (true)
	{
		errno = 0;
		int input = wgetch(self->inputWin);
		LOG_DEBUG(self->log, "Got input: %d", input);
		if (input == ERR)
		{
			if (errno == EINTR)
			{
				LOG_ERROR(self->log, "Interrupted");
				return false;
			}

			Tui_TryUpdateChat(self);
			continue;
		}

		switch(input)
		{
			case KEY_RIGHT:
				break;
			case KEY_LEFT:
				break;
		}


		waddch(self->inputWin, (chtype) input);
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
