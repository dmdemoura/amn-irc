cmake_minimum_required(VERSION 3.18)
project(amn-irc-server)

add_executable(${PROJECT_NAME}
	"src/task.h"
	"src/task.c"
	"src/task_queue.h"
	"src/task_queue.c"
	"src/task_runner.h"
	"src/task_runner.c"
	"src/accept_conn_task.h"
	"src/accept_conn_task.c"
	"src/receive_msg_task.h"
	"src/receive_msg_task.c"
	"src/main.c"
)

target_compile_features(${PROJECT_NAME} PUBLIC c_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES
	C_STANDARD 17
	C_STANDARD_REQUIRED YES
	C_EXTENSIONS ON)

target_link_libraries(${PROJECT_NAME} PRIVATE amn-irc-lib) 

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
