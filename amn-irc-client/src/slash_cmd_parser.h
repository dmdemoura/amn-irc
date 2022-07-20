#ifndef AMN_SLASH_CMD_PARSER_H
#define AMN_SLASH_CMD_PARSER_H

#include "log.h"
#include "slash_cmd.h"

typedef struct SlashCmdParser SlashCmdParser;

SlashCmdParser* SlashCmdParser_New(const Logger* log);

void SlashCmdParser_Delete(SlashCmdParser* self);

SlashCmd* SlashCmdParser_Parse(SlashCmdParser* self, const char* line);


#endif // AMN_SLASH_CMD_PARSER_H 
