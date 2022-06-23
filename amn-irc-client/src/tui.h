#ifndef AMN_TUI_H
#define AMN_TUI_H

#include "log.h"
#include "user_input_queue.h"
#include "user_output_queue.h"

#include <stdbool.h>

typedef struct Tui Tui;

Tui* Tui_Create(const Logger* log, UserInputQueue* userInput, UserOutputQueue* userOutput);
void Tui_Delete(Tui* self);

bool Tui_Run(Tui* self);

#endif // AMN_TUI_H
