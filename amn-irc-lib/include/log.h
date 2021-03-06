#ifndef AMN_LOG_H
#define AMN_LOG_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

typedef struct Logger Logger;

typedef enum LogLevel {
	LogLevel_Debug,
	LogLevel_Info,
	LogLevel_Warn,
	LogLevel_Error,
} LogLevel;

/**
  * @param logFiles			An array of files to log to, it must live as long
  *							as the logger does.
  *	@param logFileCount		How many log files to use.
  * @return A new logger, or null if failure occurs. 
  */
Logger* Logger_Create(FILE* *const logFiles, size_t logFileCount);

/**
 * @param self Pointer to a logger, must not be null.
 */
void Logger_Destroy(Logger* self);

void Logger_Log(
	const Logger* self,
	LogLevel level, 
	const char* function,
	const char* file,
	uint32_t line,
	const char* format,
	...);

#define LOG_DEBUG(logger, ...) \
	Logger_Log(logger, LogLevel_Debug, __func__, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(logger, ...) \
	Logger_Log(logger, LogLevel_Info, __func__, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(logger, ...) \
	Logger_Log(logger, LogLevel_Warn, __func__, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(logger, ...) \
	Logger_Log(logger, LogLevel_Error, __func__, __FILE__, __LINE__, __VA_ARGS__)

#endif //AMN_LOG_H

