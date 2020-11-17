file(GLOB_RECURSE sources "*.c")
file(GLOB_RECURSE headers "*.h")
set(libraries)

if(DISABLE_HTTP)
    list(FILTER sources EXCLUDE REGEX "http.c")
    list(FILTER headers EXCLUDE REGEX "http.h")
    add_compile_definitions(DISABLE_HTTP)
else()
    list(APPEND libraries curl)
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

add_library(${PROJECT_NAME}_s STATIC ${sources} ${headers})
add_library(${PROJECT_NAME} SHARED ${sources} ${headers})

target_link_libraries(${PROJECT_NAME}_s ${libraries})
target_link_libraries(${PROJECT_NAME} ${libraries})