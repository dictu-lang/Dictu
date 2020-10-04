cmake_minimum_required(VERSION 3.18.0)
project(Dictu C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(DISABLE_HTTP OFF CACHE BOOL "Determines if HTTPS based features are compiled. HTTPS based features require cURL.")
set(DISABLE_LINENOISE OFF CACHE BOOL "Determines if the REPL uses linenoise. Linenoise requires termios.")

file(GLOB_RECURSE sources "c/*.c")
file(GLOB_RECURSE headers "c/*.h")
set(libraries)

if(DISABLE_HTTP)
    list(FILTER sources EXCLUDE REGEX "http.c")
    list(FILTER headers EXCLUDE REGEX "http.h")
    add_compile_definitions(DISABLE_HTTP)
else()
    list(APPEND libraries curl)
endif()

if(DISABLE_LINENOISE)
    add_compile_definitions(DISABLE_LINENOISE)
endif()

if(WIN32)
    # ws2_32 is required for winsock2.h to work correctly
    list(APPEND libraries ws2_32)
else()
    list(APPEND libraries m)
endif()

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_C_FLAGS "/WX /W1")
    set(CMAKE_C_FLAGS_DEBUG "/Zi")
    set(CMAKE_C_FLAGS_RELEASE "/O2")
else()
    set(CMAKE_C_FLAGS "-Wall -Wextra -Werror -Wshadow -Wunused-function -Wunused-macros")
    set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
    set(CMAKE_C_FLAGS_RELEASE "-O3 -flto")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(DEBUG DEBUG_STRESS_GC DEBUG_FINAL_MEM)
endif()

add_executable(${PROJECT_NAME} ${sources} ${headers})

target_link_libraries(${PROJECT_NAME} ${libraries})
