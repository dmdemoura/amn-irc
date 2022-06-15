/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf commands.gperf  */
/* Computed positions: -k'1-3' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "commands.gperf"


#include "irc_cmd_map.h"

struct IrcCmdMapping
{
	const char* name;
	IrcCmdType cmd;
};

const struct IrcCmdMapping* in_word_set(const char* cmd, size_t len);

IrcCmdType IrcCmdType_FromStr(const char* cmd, size_t len)
{
	const struct IrcCmdMapping* mapping =  in_word_set(cmd, len);

	return mapping != NULL ? mapping->cmd : IrcCmdType_Null;
}


#line 26 "commands.gperf"
struct IrcCmdMapping;
#include <string.h>
/* maximum key range = 71, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (register const char *str, register size_t len)
{
  static const unsigned char asso_values[] =
    {
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75,  0, 75, 15, 30,  5,
      55, 75,  5,  5,  6, 25, 35, 30, 10,  0,
      10, 15,  5,  0, 20, 30,  5,  0, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75
    };
  return (unsigned int) len + asso_values[(unsigned char)str[2]] + asso_values[(unsigned char)str[1]] + asso_values[(unsigned char)str[0]];
}

const struct IrcCmdMapping *
in_word_set (register const char *str, register size_t len)
{
  enum
    {
      TOTAL_KEYWORDS = 40,
      MIN_WORD_LENGTH = 3,
      MAX_WORD_LENGTH = 8,
      MIN_HASH_VALUE = 4,
      MAX_HASH_VALUE = 74
    };

  static const struct IrcCmdMapping wordlist[] =
    {
      {"", IrcCmdType_Null}, {"", IrcCmdType_Null}, {"", IrcCmdType_Null}, {"", IrcCmdType_Null},
#line 60 "commands.gperf"
      {"AWAY", IrcCmdType_Away},
      {"", IrcCmdType_Null}, {"", IrcCmdType_Null}, {"", IrcCmdType_Null},
#line 53 "commands.gperf"
      {"WHO", IrcCmdType_Who},
#line 67 "commands.gperf"
      {"ISON", IrcCmdType_IsOn},
#line 54 "commands.gperf"
      {"WHOIS", IrcCmdType_Whois},
#line 55 "commands.gperf"
      {"WHOWAS", IrcCmdType_Whowas},
      {"", IrcCmdType_Null}, {"", IrcCmdType_Null},
#line 28 "commands.gperf"
      {"PASS", IrcCmdType_Pass},
#line 35 "commands.gperf"
      {"JOIN", IrcCmdType_Join},
#line 31 "commands.gperf"
      {"SERVER", IrcCmdType_Server},
#line 62 "commands.gperf"
      {"RESTART", IrcCmdType_Restart},
      {"", IrcCmdType_Null},
#line 36 "commands.gperf"
      {"PART", IrcCmdType_Part},
#line 59 "commands.gperf"
      {"ERROR", IrcCmdType_Error},
#line 61 "commands.gperf"
      {"REHASH", IrcCmdType_Rehash},
#line 43 "commands.gperf"
      {"VERSION", IrcCmdType_Version},
#line 32 "commands.gperf"
      {"OPERATOR", IrcCmdType_Operator},
#line 58 "commands.gperf"
      {"PONG", IrcCmdType_Pong},
#line 44 "commands.gperf"
      {"STATS", IrcCmdType_Stats},
#line 41 "commands.gperf"
      {"INVITE", IrcCmdType_Invite},
#line 51 "commands.gperf"
      {"PRIVMSG", IrcCmdType_PrivMsg},
      {"", IrcCmdType_Null},
#line 57 "commands.gperf"
      {"PING", IrcCmdType_Ping},
#line 48 "commands.gperf"
      {"TRACE", IrcCmdType_Trace},
      {"", IrcCmdType_Null},
#line 47 "commands.gperf"
      {"CONNECT", IrcCmdType_Connect},
      {"", IrcCmdType_Null},
#line 29 "commands.gperf"
      {"NICK", IrcCmdType_Nick},
#line 38 "commands.gperf"
      {"TOPIC", IrcCmdType_Topic},
#line 52 "commands.gperf"
      {"NOTICE", IrcCmdType_Notice},
      {"", IrcCmdType_Null}, {"", IrcCmdType_Null},
#line 30 "commands.gperf"
      {"USER", IrcCmdType_User},
#line 64 "commands.gperf"
      {"USERS", IrcCmdType_Users},
      {"", IrcCmdType_Null},
#line 65 "commands.gperf"
      {"WALLOPS", IrcCmdType_WallOps},
#line 66 "commands.gperf"
      {"USERHOST", IrcCmdType_UserHost},
#line 40 "commands.gperf"
      {"LIST", IrcCmdType_List},
#line 39 "commands.gperf"
      {"NAMES", IrcCmdType_Names},
      {"", IrcCmdType_Null}, {"", IrcCmdType_Null}, {"", IrcCmdType_Null},
#line 42 "commands.gperf"
      {"KICK", IrcCmdType_Kick},
#line 34 "commands.gperf"
      {"SQUIT", IrcCmdType_ServerQuit},
      {"", IrcCmdType_Null}, {"", IrcCmdType_Null}, {"", IrcCmdType_Null},
#line 33 "commands.gperf"
      {"QUIT", IrcCmdType_Quit},
#line 45 "commands.gperf"
      {"LINKS", IrcCmdType_Links},
      {"", IrcCmdType_Null}, {"", IrcCmdType_Null}, {"", IrcCmdType_Null},
#line 46 "commands.gperf"
      {"TIME", IrcCmdType_Time},
      {"", IrcCmdType_Null}, {"", IrcCmdType_Null}, {"", IrcCmdType_Null}, {"", IrcCmdType_Null},
#line 37 "commands.gperf"
      {"MODE", IrcCmdType_Mode},
#line 49 "commands.gperf"
      {"ADMIN", IrcCmdType_Admin},
#line 63 "commands.gperf"
      {"SUMMON", IrcCmdType_Summon},
      {"", IrcCmdType_Null}, {"", IrcCmdType_Null},
#line 56 "commands.gperf"
      {"KILL", IrcCmdType_Kill},
      {"", IrcCmdType_Null}, {"", IrcCmdType_Null}, {"", IrcCmdType_Null}, {"", IrcCmdType_Null},
#line 50 "commands.gperf"
      {"INFO", IrcCmdType_Info}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
            return &wordlist[key];
        }
    }
  return 0;
}
