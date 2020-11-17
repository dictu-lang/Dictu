set(DICTU_CLI_SRC main.c linenoise.c linenoise.h)

if(DISABLE_LINENOISE)
    add_compile_definitions(DISABLE_LINENOISE)
endif()

add_executable(Dictu ${DICTU_CLI_SRC})
target_link_libraries(Dictu dictu_api)