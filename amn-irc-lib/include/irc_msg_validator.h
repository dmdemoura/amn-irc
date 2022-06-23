#ifndef AMN_IRC_MSG_VALIDATOR_H
#define AMN_IRC_MSG_VALIDATOR_H

#include "log.h"

#include <stdbool.h>


typedef struct IrcMsgValidator IrcMsgValidator;

IrcMsgValidator* IrcMsgValidator_New(const Logger* logger);
void IrcMsgValidator_Delete(IrcMsgValidator* self);

bool IrcMsgValidator_ValidateOrigin(const IrcMsgValidator* self,
		const char* origin, const char* originEnd);

bool IrcMsgValidator_ValidateNick(const IrcMsgValidator* self,
		const char* nick, const char* nickEnd);

bool IrcMsgValidator_ValidateUser(const IrcMsgValidator* self,
		const char* user, const char* userEnd);

bool IrcMsgValidator_ValidateHost(const IrcMsgValidator* self,
		const char* host, const char* hostEnd);

bool IrcMsgValidator_ValidateServer(const IrcMsgValidator* self,
		const char* server, const char* serverEnd);

bool IrcMsgValidator_ValidateCommand(const IrcMsgValidator* self,
		const char* command, const char* commandEnd);

bool IrcMsgValidator_ValidateMiddleParam(const IrcMsgValidator* self,
		const char* param, const char* paramEnd);

bool IrcMsgValidator_ValidateTrailingParam(const IrcMsgValidator* self,
		const char* param, const char* paramEnd);

bool IrcMsgValidator_ValidateChannel(const IrcMsgValidator* self, const char* channel);

bool IrcMsgValidator_ValidateMask(const IrcMsgValidator* self, const char* mask);

bool IrcMsgValidator_ValidateChstring(const IrcMsgValidator* self, const char* chstring);


#endif // AMN_IRC_MSG_VALIDATOR_H
