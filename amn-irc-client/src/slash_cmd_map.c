/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf amn-irc-client/slash_cmds.gperf  */
/* Computed positions: -k'1' */

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

#line 1 "amn-irc-client/slash_cmds.gperf"


#include "slash_cmd_map.h"

struct SlashCmdTypeMapping
{
	const char* name;
	SlashCmdType cmd;
};

const struct SlashCmdTypeMapping* in_word_set(const char* cmd, size_t len);

SlashCmdType SlashCmdType_FromStr(const char* cmd, size_t len)
{
	const struct SlashCmdTypeMapping* mapping =  in_word_set(cmd, len);

	return mapping != NULL ? mapping->cmd : SlashCmdType_Null;
}


#line 28 "amn-irc-client/slash_cmds.gperf"
struct SlashCmdTypeMapping;
#include <string.h>
/* maximum key range = 16, duplicates = 0 */

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
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20,  0,
      20, 20, 20, 20, 20, 20,  9, 15, 20, 10,
       0, 20,  5,  0, 20, 20, 20,  0, 20,  0,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20
    };
  return (unsigned int) len + asso_values[(unsigned char)str[0]];
}

const struct SlashCmdTypeMapping *
in_word_set (register const char *str, register size_t len)
{
  enum
    {
      TOTAL_KEYWORDS = 9,
      MIN_WORD_LENGTH = 4,
      MAX_WORD_LENGTH = 8,
      MIN_HASH_VALUE = 4,
      MAX_HASH_VALUE = 19
    };

  static const struct SlashCmdTypeMapping wordlist[] =
    {
      {"", SlashCmdType_Null}, {"", SlashCmdType_Null}, {"", SlashCmdType_Null}, {"", SlashCmdType_Null},
#line 31 "amn-irc-client/slash_cmds.gperf"
      {"quit", SlashCmdType_Quit},
#line 38 "amn-irc-client/slash_cmds.gperf"
      {"whois", SlashCmdType_Whois},
#line 37 "amn-irc-client/slash_cmds.gperf"
      {"unmute", SlashCmdType_Unmute},
#line 30 "amn-irc-client/slash_cmds.gperf"
      {"connect", SlashCmdType_Connect},
#line 33 "amn-irc-client/slash_cmds.gperf"
      {"nickname", SlashCmdType_Nickname},
#line 32 "amn-irc-client/slash_cmds.gperf"
      {"ping", SlashCmdType_Ping},
      {"", SlashCmdType_Null}, {"", SlashCmdType_Null}, {"", SlashCmdType_Null},
#line 34 "amn-irc-client/slash_cmds.gperf"
      {"join", SlashCmdType_Join},
#line 36 "amn-irc-client/slash_cmds.gperf"
      {"mute", SlashCmdType_Mute},
      {"", SlashCmdType_Null}, {"", SlashCmdType_Null}, {"", SlashCmdType_Null}, {"", SlashCmdType_Null},
#line 35 "amn-irc-client/slash_cmds.gperf"
      {"kick", SlashCmdType_Kick}
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
