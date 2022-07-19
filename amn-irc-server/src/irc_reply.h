#ifndef AMN_IRC_REPLY_H
#define AMN_IRC_REPLY_H

#include "irc_cmd.h"
#include "irc_msg.h"

/*
 * 332	 RPL_TOPIC
 * 				"<channel> :<topic>"

 * 		- When sending a TOPIC message to determine the
 * 		  channel topic, one of two replies is sent.  If
 * 		  the topic is set, RPL_TOPIC is sent back else
 * 		  RPL_NOTOPIC.
 */
IrcMsg* IrcReply_RplTopic(
		const char* servername, IrcChannelType channelType, const char* channel,
		const char* topic);

/*
 * 401	 ERR_NOSUCHNICK
 * 				"<nickname> :No such nick/channel"

 * 		- Used to indicate the nickname parameter supplied to a
 * 		  command is currently unused.
 */
IrcMsg* IrcReply_ErrNoSuchNick(const char* servername, const char* nickname);

/*
 * 436	 ERR_NICKCOLLISION
 * 				"<nick> :Nickname collision KILL"

 * 		- Returned by a server to a client when it detects a
 * 		  nickname collision (registered of a NICK that
 * 		  already exists by another server).
 */
IrcMsg* IrcReply_ErrNickCollision(const char* servername, const char* nickname);

/*
 * 451	 ERR_NOTREGISTERED
 * 				":You have not registered"

 * 		- Returned by the server to indicate that the client
 * 		  must be registered before the server will allow it
 * 		  to be parsed in detail.
 */
IrcMsg* IrcReply_ErrNotRegistered(const char* servername);

/*
 * 461	 ERR_NEEDMOREPARAMS
 * 				"<command> :Not enough parameters"

 * 		- Returned by the server by numerous commands to
 * 		  indicate to the client that it didn't supply enough
 * 		  parameters.
 */
IrcMsg* IrcReply_ErrNeedMoreParams(const char* servername, IrcCmdType cmd);

/*
 * 462	 ERR_ALREADYREGISTRED
 * 				":You may not reregister"

 * 		- Returned by the server to any link which tries to
 * 		  change part of the registered details (such as
 * 		  password or user details from second USER message).
 */
IrcMsg* IrcReply_ErrAlreadyRegistered(const char* servername);

/*
 * 471	 ERR_CHANNELISFULL
 * 				"<channel> :Cannot join channel (+l)"
 */
IrcMsg* IrcReply_ErrChannelIsFull(
		const char* servername, IrcChannelType channelType, const char* channelName);

/*
 * 473	 ERR_INVITEONLYCHAN
 * 				"<channel> :Cannot join channel (+i)"
 */
IrcMsg* IrcReply_ErrInviteOnlyChan(
		const char* servername, IrcChannelType channelType, const char* channelName);

/*
 * 474	 ERR_BANNEDFROMCHAN
 * 				"<channel> :Cannot join channel (+b)"
 */
IrcMsg* IrcReply_ErrBannedFromChan(
		const char* servername, IrcChannelType channelType, const char* channelName);

/*
 * 475	 ERR_BADCHANNELKEY
 * 				"<channel> :Cannot join channel (+k)"
 */
IrcMsg* IrcReply_ErrBadChannelKey(
		const char* servername, IrcChannelType channelType, const char* channelName);

#endif // AMN_IRC_REPLY_H
