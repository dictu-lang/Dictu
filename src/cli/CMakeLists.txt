set(DICTU_CLI_SRC main.c linenoise.c linenoise.h encodings/utf8.c encodings/utf8.h)
set(DISABLE_LINENOISE OFF CACHE BOOL "Determines if the REPL uses linenoise. Linenoise requires termios.")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

if(DISABLE_LINENOISE)
    list(FILTER DICTU_CLI_SRC EXCLUDE REGEX "(linenoise|utf8)\.(c|h)")

    add_compile_definitions(DISABLE_LINENOISE)
endif()

add_executable(dictu ${DICTU_CLI_SRC})
target_include_directories(dictu PUBLIC ${INCLUDE_DIR})
target_link_libraries(dictu dictu_api_static)