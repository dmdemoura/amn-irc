cmake_minimum_required(VERSION 3.18)
project(amn-irc-client)

find_package(Curses REQUIRED)

add_executable(${PROJECT_NAME}
	"src/main.c"
	"src/tui.h"
	"src/tui.c"
	"src/tui_input.h"
	"src/tui_input.c"
	"src/msg_sender_task.h"
	"src/msg_sender_task.c"
	"src/slash_cmd.h"
	"src/slash_cmd.c"
	"src/slash_cmd_map.h"
	"src/slash_cmd_map.c"
	"src/slash_cmd_parser.h"
	"src/slash_cmd_parser.c"
	"src/user_input_queue.h"
	"src/user_input_queue.c"
	"src/user_output_queue.h"
	"src/user_output_queue.c"
)

target_compile_features(${PROJECT_NAME} PUBLIC c_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES
	C_STANDARD 17
	C_STANDARD_REQUIRED YES
	C_EXTENSIONS ON)

target_include_directories(${PROJECT_NAME}
	PRIVATE
		"src/"
		"${CURSES_INCLUDE_DIR}"
)

target_link_libraries(${PROJECT_NAME} PRIVATE amn-irc-lib ${CURSES_LIBRARY}) 

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
