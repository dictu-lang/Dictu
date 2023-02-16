set(DICTU_CLI_SRC main.c linenoise/linenoise.c linenoise/linenoise.h linenoise/stringbuf.c linenoise/stringbuf.h linenoise/utf8.c linenoise/utf8.h)
set(DISABLE_LINENOISE OFF CACHE BOOL "Determines if the REPL uses linenoise. Linenoise requires termios.")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

if((NOT WIN32) OR DISABLE_LINENOISE)
    list(FILTER DICTU_CLI_SRC EXCLUDE REGEX "linenoise-win32.c")
endif()

if(DISABLE_LINENOISE)
    list(FILTER DICTU_CLI_SRC EXCLUDE REGEX "(linenoise|stringbuf|utf8)\.(c|h)")

    add_compile_definitions(DISABLE_LINENOISE)
else()
    add_compile_definitions(USE_UTF8)
endif()

add_executable(dictu ${DICTU_CLI_SRC})
target_include_directories(dictu PUBLIC ${INCLUDE_DIR})
target_link_libraries(dictu dictu_api_static)
