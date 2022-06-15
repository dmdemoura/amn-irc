#ifndef AMN_IRC_CMD_MAP_H
#define AMN_IRC_CMD_MAP_H

#include "irc_cmd_type.h"

#include <stddef.h>

IrcCmdType IrcCmdType_FromStr(const char* cmd, size_t len);

#endif // AMN_IRC_CMD_MAP_H

