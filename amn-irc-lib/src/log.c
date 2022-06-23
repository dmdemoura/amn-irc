#include "log.h"

#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


const char* TIME_PATTERN = "%H:%M:%S";
const size_t MAX_TIME_BYTES = 128;

const size_t MAX_MSG_BYTES = 1024;

const char* LOG_PATTERN = "[%s][%s] %s:%"PRIu32" - %s - %s\n%s%s";

const char* LOG_PATTERN_WITH_ERRNO = "[%s][%s] %s:%"PRIu32" - %s - %s\n\tErrno: %s";

const char* LOG_LEVEL_STRS[] = {
	"\033[38;2;⟨164⟩;⟨164⟩;⟨164⟩mDEBUG\033[0m",
	"INFO",
	"\033[38;2;⟨255⟩;⟨200⟩;⟨0⟩mWARN\033[0m",
	"\033[38;2;⟨240⟩;⟨20⟩;⟨20⟩mERROR\033[0m" };

const size_t ERRNO_MSG_BYTES = 256;

const char* SRC_PATH_PREFIX = "amn-irc-";


static const char* LogLevel_ToString(LogLevel level)
{
	return LOG_LEVEL_STRS[level];
}

struct Logger {
	FILE** logFiles;
	size_t logFileCount;
};

Logger* Logger_Create(FILE* *const logFiles, size_t logFileCount)
{
	Logger* self = malloc(sizeof(Logger));
	if (self == NULL)
	{
		return NULL;
	}

	self->logFiles = malloc(sizeof(FILE*) * logFileCount);
	if (self->logFiles == NULL)
	{
		free(self);
		return NULL;
	}

	memcpy(self->logFiles, logFiles, sizeof(FILE*) * logFileCount);
	self->logFileCount = logFileCount;
	
	return self;
}

void Logger_Destroy(Logger* self)
{
	if (self == NULL)
	{
		return;
	}

	free(self->logFiles);
	free(self);
}

static const char* getRelativePathForLog(const char* file)
{
	const char* substr = strstr(file, SRC_PATH_PREFIX);
	
	return substr != NULL ? substr : file;
}

void Logger_Log(
	const Logger* self,
	LogLevel level, 
	const char* function,
	const char* file,
	uint32_t line,
	const char* format,
	...)
{
   (void) self;

	bool hasErrno = errno != 0;
	// Format errno
	char errorMsg[ERRNO_MSG_BYTES];
	bool errnoOk = !hasErrno || strerror_r(errno, errorMsg, ERRNO_MSG_BYTES) == 0;

	// Format message
	va_list args;
	va_start(args, format);

	char message[MAX_MSG_BYTES];
	bool messageOk = vsnprintf(message, MAX_MSG_BYTES, format, args) >= 0;

	va_end(args);

	// Format time
	time_t currentTime = time(NULL);
	struct tm localTime;
	bool timeOk = localtime_r(&currentTime, &localTime) != NULL;

	char timeString[MAX_TIME_BYTES];
	timeOk = timeOk && strftime(timeString, MAX_TIME_BYTES, TIME_PATTERN, &localTime) != 0;

	for (size_t i = 0; i < self->logFileCount; i++)
	{
		fprintf(self->logFiles[i], LOG_PATTERN,
				timeOk ? timeString : "time fmt err",
				LogLevel_ToString(level),
				getRelativePathForLog(file),
				line, function,
				messageOk ? message : "msg fmt err",
				hasErrno ? "\tErrno: " : "",
				hasErrno ? (errnoOk ? errorMsg : "errno fmt err") : "");
	}

	errno = 0;
}
