# Swap out for explicit files
file(GLOB_RECURSE sources "*.c")
file(GLOB_RECURSE headers "*.h")
set(libraries)

set(INCLUDE_DIR include/)

# Remove CLI files
list(FILTER sources EXCLUDE REGEX "(main|stringbuf|linenoise(-win32)?|utf8).c")
list(FILTER headers EXCLUDE REGEX "(linenoise|stringbuf|utf8).h")

find_library(SQLITE_LIB SQLite3)
set(THREADS)

if(DISABLE_HTTP)
    list(FILTER sources EXCLUDE REGEX "http.c")
    list(FILTER headers EXCLUDE REGEX "http.h")
    add_compile_definitions(DISABLE_HTTP)
else()
    if (ENABLE_VCPKG)
        find_package(CURL REQUIRED)
        list(APPEND libraries CURL::libcurl)
        include_directories(${CURL_INCLUDE_DIRS})
    else()
        list(APPEND libraries curl)
    endif()
endif()

if(DISABLE_UUID)
    list(FILTER sources EXCLUDE REGEX "uuid.c")
    list(FILTER headers EXCLUDE REGEX "uuid.h")
    add_compile_definitions(DISABLE_UUID)
else()
    if(NOT APPLE)
        list(APPEND libraries uuid)
    endif()
endif()

if(DISABLE_SODIUM)
    add_compile_definitions(DISABLED_SODIUM)
else()
    message(YES)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
    find_package(sodium REQUIRED)
    list(APPEND libraries sodium)
endif()

if(NOT SQLITE_LIB)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    set(THREADS Threads::Threads)
else()
    list(FILTER sources EXCLUDE REGEX "sqlite3.c")
    list(FILTER headers EXCLUDE REGEX "sqlite3.h")
    list(APPEND libraries ${SQLITE_LIB})
    add_compile_definitions(INCLUDE_SQLITE_LIB)
endif()

if(WIN32)
    # ws2_32 is required for winsock2.h to work correctly
    list(APPEND libraries ws2_32 bcrypt)
else()
    list(FILTER sources EXCLUDE REGEX "linenoise-win32.c")
    list(APPEND libraries m)
endif()

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    # An issue on CI/CD with winbase.h producing C5105 warnings
    if(NOT CICD)
        set(CMAKE_C_FLAGS "/WX /W1")
    endif()
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

add_library(objlib OBJECT ${sources} ${headers})

# shared libraries need PIC
set_property(TARGET objlib PROPERTY POSITION_INDEPENDENT_CODE 1)

# shared and static libraries built from the same object files
add_library(${PROJECT_NAME}_static STATIC $<TARGET_OBJECTS:objlib>)
add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:objlib>)

target_link_libraries(${PROJECT_NAME}_static ${libraries} ${CMAKE_DL_LIBS} ${THREADS})
target_link_libraries(${PROJECT_NAME} ${libraries} ${CMAKE_DL_LIBS} ${THREADS})