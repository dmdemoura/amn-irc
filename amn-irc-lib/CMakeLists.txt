cmake_minimum_required(VERSION 3.18)
project(amn-irc-lib)

add_library(${PROJECT_NAME}
	"include/log.h"
	"src/log.c"
	"include/str_utils.h"
	"src/str_utils.c"
	"include/array_list.h"
	"src/array_list.c"
	"include/application.h"
	"src/application.c"
	"include/queue.h"
	"src/queue.c"

	"include/irc_msg.h"
	"src/irc_msg.c"

	"include/irc_cmd.h"
	"src/irc_cmd.c"
	"include/irc_cmd_type.h"
	"src/irc_cmd_type.c"
	"src/irc_cmd_map.h"
	"src/irc_cmd_map.c"

	"include/irc_msg_validator.h"
	"src/irc_msg_validator.c"

	"include/irc_msg_reader.h"
	"src/irc_msg_reader.c"
	"include/irc_msg_parser.h"
	"src/irc_msg_parser.c"
	"include/irc_cmd_parser.h"
	"src/irc_cmd_parser.c"

	"include/irc_msg_writer.h"
	"src/irc_msg_writer.c"
	"include/irc_msg_unparser.h"
	"src/irc_msg_unparser.c"
	"include/irc_cmd_unparser.h"
	"src/irc_cmd_unparser.c"

	"include/task.h"
	"src/task.c"
	"include/task_queue.h"
	"src/task_queue.c"
	"include/task_runner.h"
	"src/task_runner.c"
)

target_compile_features(${PROJECT_NAME} PUBLIC c_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES
	C_STANDARD 17
	C_STANDARD_REQUIRED YES
	C_EXTENSIONS ON)

target_include_directories(${PROJECT_NAME}
	PUBLIC
		"include/"
	PRIVATE
		"src/"
)

target_compile_options(${PROJECT_NAME}
	PRIVATE
	$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
		-Werror				# Treat warnings as errors.
		-Wall				# Enables many warning but despite the name not all.
		-Wextra				# More warnings.
		-Wconversion		# Warn on implicit conversion that might alter a value.
		-Wsign-conversion	# Warn also about implict conversion between signed and unsigned
							# types.
		-pedantic-errors	# Error on language extensions.
	>
	$<$<CXX_COMPILER_ID:MSVC>:
		/WX		# Treat warnings as errors.
		/W4		# Warning level 4.
	>
)
