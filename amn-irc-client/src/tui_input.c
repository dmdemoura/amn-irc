#include "tui_input.h"

#include "str_utils.h"

#include <stdlib.h>
#include <string.h>

#include <curses.h>

#define MAX_MSG_SIZE 4096
#define INPUT_BUF_SIZE MAX_MSG_SIZE + 1

struct TuiInput
{
	const Logger* log;
	UserInputQueue* userInput;

	WINDOW* win;
	int width;
	int height;

	char inputBuf[INPUT_BUF_SIZE];
	int len;
	int pos;

	bool success;
};

static void Type(TuiInput* self, char c);
static void Backspace(TuiInput* self);
static void Delete(TuiInput* self);
static void Submit(TuiInput* self);
static void MoveCursor(TuiInput* self, int posOffset);
static bool RefreshPreview(TuiInput* self);
static bool RefreshCursor(TuiInput* self);

TuiInput* TuiInput_New(
		const Logger* log,
		UserInputQueue* userInput,
		int height,
		int width,
		int posY,
		int posX)
{
	TuiInput* self = malloc(sizeof(TuiInput));
	if (self == NULL)
	{
		return NULL;
	}

	*self = (TuiInput) {
		.log = log,
		.userInput = userInput,
		.width = width,
		.height = height,
		.len = 0,
		.pos = 0
	};

	self->win = newwin(height, width, posY, posX);
	if (self->win == NULL)
	{
		LOG_ERROR(log, "Failed to create window!");
		goto error;
	}

	if (keypad(self->win, true) == ERR)
	{
		LOG_ERROR(log, "Failed to enable input window keypad!");
		goto error;
	}

	wtimeout(self->win, 100);

	return self;

error:
	TuiInput_Delete(self);
	return NULL;
}

void TuiInput_Delete(TuiInput* self)
{
	if (self == NULL)
	{
		return;
	}

	if (delwin(self->win) == ERR)
	{
		LOG_ERROR(self->log, "Failed to free window!");
	}

	free(self);
}

TaskStatus TuiInput_Run(TuiInput* self)
{
	self->success = true;

	int input = wgetch(self->win);
	if (input == ERR)
	{
		return TaskStatus_Yield;
	}

	// LOG_DEBUG(self->log, "Input %d", input);

	switch(input)
	{
		case KEY_LEFT:
			MoveCursor(self, -1);
			break;
		case KEY_RIGHT:
			MoveCursor(self, +1);
			break;
		case KEY_UP:
			MoveCursor(self, -self->width);
			break;
		case KEY_DOWN:
			MoveCursor(self, +self->width);
			break;
		case KEY_BACKSPACE:
			Backspace(self);
			break;
		case KEY_DC:
			Delete(self);
			break;
		case '\r':
		case '\n':
			Submit(self);
			break;
		case '\0':
			return TaskStatus_Yield; 
		default:
			if (input & KEY_CODE_YES || self->pos >= 4096 || self->len >= 4096)
			{
				return TaskStatus_Yield; 
			}
			else
			{
				Type(self, (char) input);
			}
	}

	if (!self->success)
	{
		return TaskStatus_Failed;
	}
	
	// LOG_DEBUG(self->log, "Pos: %d | Len: %d\nMsg Buffer: %s", self->pos, self->len, self->inputBuf);

	if (wclear(self->win) == ERR)
	{
		LOG_ERROR(self->log, "Failed to clear input window!");
		return TaskStatus_Failed;
	}

	if (!RefreshPreview(self))
	{
		return TaskStatus_Failed;
	}

	if(!RefreshCursor(self))
	{
		return TaskStatus_Failed;
	}

	return TaskStatus_Yield; 
}


static void Type(TuiInput* self, char c)
{
	if (self->pos < self->len) 
	{
		char* copyTo	= self->inputBuf + self->pos + 1;
		char* copyFrom	= self->inputBuf + self->pos;
		size_t len		= (size_t) (self->len - self->pos);

		memmove(copyTo, copyFrom, len);
	}

	self->inputBuf[self->pos] = c;
	self->inputBuf[self->len + 1] = '\0';
	self->pos++;
	self->len++;
}

static void Backspace(TuiInput* self)
{
	if (self->len > 0 && self->pos > 0) 
	{
		char* copyTo	= self->inputBuf + self->pos - 1;
		char* copyFrom	= self->inputBuf + self->pos;
		size_t len		= (size_t) (self->len - self->pos);

		memmove(copyTo, copyFrom, len);

		self->inputBuf[self->len - 1] = '\0';
		self->pos--;
		self->len--;
	}
}

static void Delete(TuiInput* self)
{
	if (self->len > 0 && self->pos < self->len) 
	{
		char* copyTo	= self->inputBuf + self->pos;
		char* copyFrom	= self->inputBuf + self->pos + 1;
		size_t len		= (size_t) (self->len - self->pos - 1);

		memmove(copyTo, copyFrom, len);

		self->inputBuf[self->len - 1] = '\0';
		self->len--;
	}
}

static void Submit(TuiInput* self)
{
	char* line = StrUtils_CloneRange(self->inputBuf, self->inputBuf + self->len);

	self->inputBuf[0] = '\0';
	self->pos = 0;
	self->len = 0;

	if (line == NULL)
	{
		LOG_ERROR(self->log, "Failed to clone submitted line.");
		self->success = false;
		return;
	}

	LOG_DEBUG(self->log, "Submitting line: %s", line);

	if (!UserInputQueue_Push(self->userInput, line))
	{
		LOG_ERROR(self->log, "Failed to push submitted line into queue.");
		self->success = false;
		return;
	}
}

static void MoveCursor(TuiInput* self, int posOffset)
{
	if (posOffset > 0)
	{
		if (posOffset < self->pos)
		{
			self->pos -= posOffset;
		}
		else
		{
			self->pos = 0;
		}
	}
	else
	{
		if (self->pos + posOffset < self->len)
		{
			self->pos += posOffset;
		}
		else
		{
			self->pos = self->len;
		}
	}
}

static bool RefreshPreview(TuiInput* self)
{
	int max_chars = self->height * self->width - 1;
	int offset = self->pos / max_chars * max_chars;

	// LOG_DEBUG(self->log, "Max chars: %d | Pos: %d | Offset: %d", max_chars, self->pos, offset);

	if (mvwaddnstr(self->win, 0, 0, self->inputBuf + offset, max_chars) == ERR)
	{
		LOG_ERROR(self->log, "Failed to write input text!");
		return false;
	}
	
	return true;
}

static bool RefreshCursor(TuiInput* self)
{
	int max_chars = self->width * self->height - 1;
	int offset = self->pos / max_chars * max_chars;

	int posInView = self->pos - offset;

	int y = posInView / self->width;
	int x = posInView % self->width;

	if (wmove(self->win, y, x) == ERR)
	{
		LOG_ERROR(self->log, "Failed to move input window cursor");
		return false;
	}

	return true;
}
