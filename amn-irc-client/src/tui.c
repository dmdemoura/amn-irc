#include "tui.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <curses.h>

#define INPUT_WIN_HEIGHT 3

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
	WINDOW* inputWin;
	UserInputQueue* userInput;
	UserOutputQueue* userOutput;
};

static bool Tui_InputWinMove(Tui* self, int pos);
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

	self->inputWin = newwin(INPUT_WIN_HEIGHT, COLS - 2, LINES - INPUT_WIN_HEIGHT - 1, 1);
	if (self->inputWin == NULL)
	{
		LOG_ERROR(log, "Failed to create input window!");
		Tui_Delete(self);
		return NULL;
	}

	if (keypad(self->inputWin, true) == ERR)
	{
		LOG_ERROR(log, "Failed to enable input window keypad!");
		Tui_Delete(self);
		return NULL;
	}

	// if (scrollok(self->inputWin, true) == ERR)
	// {
	// 	LOG_ERROR(log, "Failed to enable input window scrollok!");
	// 	Tui_Delete(self);
	// 	return NULL;
	// }

	wtimeout(self->inputWin, 100);

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

	char msgBuffer[4097] = {0};
	int len = 0;
	int pos = 0;

	while (true)
	{
		errno = 0;
		int input = wgetch(self->inputWin);
		// LOG_DEBUG(self->log, "Got input: %d", input);
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

		int cols = getmaxx(self->inputWin);
		if (cols == ERR)
		{
			LOG_ERROR(self->log, "Failed to get input window max x.");
			return false;
		}

		int lines = getmaxy(self->inputWin);
		if (lines == ERR)
		{
			LOG_ERROR(self->log, "Failed to get input window max y.");
			return false;
		}

		switch(input)
		{
			case KEY_LEFT:
				pos = pos > 0 ? pos - 1 : 0;
				goto move;
			case KEY_RIGHT:
				pos = pos + 1 < len ? pos + 1 : len;
				goto move;
			case KEY_UP:
				pos = pos >= cols ? pos - cols : 0;
				goto move;
			case KEY_DOWN:
				pos = pos + cols < len ? pos + cols : len;
				goto move;
			move:
				if(!Tui_InputWinMove(self, pos))
				{
					return false;
				}
				break;
			case KEY_BACKSPACE:
				if (len > 0 && pos > 0) 
				{
					memmove(msgBuffer + pos - 1, msgBuffer + pos, (size_t) (len - pos));
					msgBuffer[len - 1] = '\0';
					pos--;
					len--;
				}
				break;
			case KEY_DC:
				if (len > 0 && pos < len) 
				{
					memmove(msgBuffer + pos, msgBuffer + pos + 1, (size_t) (len - pos - 1));
					msgBuffer[len - 1] = '\0';
					len--;
				}
				break;
			case '\r':
			case '\n':
				msgBuffer[0] = '\0';
				pos = 0;
				len = 0;
				break;
			case '\0':
				continue;
			default:
				if (input & KEY_CODE_YES || pos >= 4096 || len >= 4096)
				{
					continue;
				}
				else if (pos < len) 
				{
					memmove(msgBuffer + pos + 1, msgBuffer + pos, (size_t) (len - pos));
				}
				msgBuffer[pos] = (char) input;
				msgBuffer[len + 1] = '\0';
				pos++;
				len++;
		}
		
		LOG_DEBUG(self->log, "Pos: %d | Len: %d\nMsg Buffer: %s", pos, len, msgBuffer);

		if (wclear(self->inputWin) == ERR)
		{
			LOG_ERROR(self->log, "Failed to clear input window!");
			return false;
		}

		int max_chars = lines * cols - 1;
		int offset = pos / max_chars * max_chars;
		LOG_DEBUG(self->log, "Max chars: %d | Pos: %d | Offset: %d", max_chars, pos, offset);

		if (mvwaddnstr(self->inputWin, 0, 0, msgBuffer + offset, max_chars) == ERR)
		{
			LOG_ERROR(self->log, "Failed to write input text!");
			return false;
		}

		if(!Tui_InputWinMove(self, pos))
		{
			return false;
		}
	}

	return true;
}

static bool Tui_InputWinMove(Tui* self, int pos)
{
	int cols = getmaxx(self->inputWin);
	if (cols == ERR)
	{
		LOG_ERROR(self->log, "Failed to get input window max x.");
		return false;
	}

	int lines = getmaxy(self->inputWin);
	if (lines == ERR)
	{
		LOG_ERROR(self->log, "Failed to get input window max y.");
		return false;
	}

	int max_chars = lines * cols - 1;
	int offset = pos / max_chars * max_chars;
	int wpos = pos - offset;

	int y = wpos / cols;
	int x = wpos % cols;

	if (wmove(self->inputWin, y, x) == ERR)
	{
		LOG_ERROR(self->log, "Failed to move input window cursor");
		return false;
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
