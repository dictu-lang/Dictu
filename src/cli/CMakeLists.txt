set(DICTU_CLI_SRC main.c linenoise.c linenoise.h)
set(DISABLE_LINENOISE OFF CACHE BOOL "Determines if the REPL uses linenoise. Linenoise requires termios.")

if(DISABLE_LINENOISE)
    add_compile_definitions(DISABLE_LINENOISE)
endif()

add_executable(Dictu ${DICTU_CLI_SRC})
target_include_directories(Dictu PUBLIC ${INCLUDE_DIR})
target_link_libraries(Dictu dictu_api_s)