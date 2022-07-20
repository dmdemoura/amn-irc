#ifndef AMN_TUI_INPUT_H
#define AMN_TUI_INPUT_H

#include "log.h"
#include "task.h"
#include "user_input_queue.h"

typedef struct TuiInput TuiInput;

TuiInput* TuiInput_New(
		const Logger* log,
		UserInputQueue* userInput,
		int height,
		int width,
		int posY,
		int posX);

void TuiInput_Delete(TuiInput* self);

TaskStatus TuiInput_Run(TuiInput* self);

#endif // AMN_TUI_INPUT_H
