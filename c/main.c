#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#elif defined(_WIN32)
#include "windowsapi.h"

void cleanupSockets(void) {
    // Calls WSACleanup until an error occurs.
    // Avoids issues if WSAStartup is called multiple times.
    while (!WSACleanup());
}
#endif

#include "common.h"
#include "vm.h"
#include "util.h"

#define VERSION "Dictu Version: 0.11.0\n"

#ifndef DISABLE_LINENOISE
#include "linenoise.h"

static bool replCountBraces(char *line) {
    int leftBraces = 0;
    int rightBraces = 0;
    bool inString = false;

    for (int i = 0; line[i]; i++) {
        if (line[i] == '\'' || line[i] == '"') {
            inString = !inString;
        }

        if (inString) {
            continue;
        }

        if (line[i] == '{') {
            leftBraces++;
        } else if (line[i] == '}') {
            rightBraces++;
        }
    }

    return leftBraces == rightBraces;
}

static bool replCountQuotes(char *line) {
    int singleQuotes = 0;
    int doubleQuotes = 0;
    char quote = '\0';

    for (int i = 0; line[i]; i++) {
        if (line[i] == '\'' && quote != '"') {
            singleQuotes++;

            if (quote == '\0') {
                quote = '\'';
            }
        } else if (line[i] == '"' && quote != '\'') {
            doubleQuotes++;
            if (quote == '\0') {
                quote = '"';
            }
        } else if (line[i] == '\\') {
            line++;
        }
    }

    return singleQuotes % 2 == 0 && doubleQuotes % 2 == 0;
}
#endif

static void repl(VM *vm, int argc, const char *argv[]) {
    UNUSED(argc); UNUSED(argv);

    printf(VERSION);
    char *line;

    #ifndef DISABLE_LINENOISE
    linenoiseHistoryLoad("history.txt");

    while((line = linenoise(">>> ")) != NULL) {
        char *fullLine = malloc(sizeof(char) * (strlen(line) + 1));
        snprintf(fullLine, strlen(line) + 1, "%s", line);

        linenoiseHistoryAdd(line);
        linenoiseHistorySave("history.txt");

        while (!replCountBraces(fullLine) || !replCountQuotes(fullLine)) {
            free(line);
            line = linenoise("... ");

            if (line == NULL) {
                return;
            }

            char *temp = realloc(fullLine, strlen(fullLine) + strlen(line) + 1);

            if (temp == NULL) {
                printf("Unable to allocate memory\n");
                exit(71);
            }

            fullLine = temp;
            memcpy(fullLine + strlen(fullLine), line, strlen(line) + 1);

            linenoiseHistoryAdd(line);
            linenoiseHistorySave("history.txt");
        }

        interpret(vm, fullLine);

        free(line);
        free(fullLine);
    }
    #else
    #define BUFFER_SIZE 8
    line = calloc(BUFFER_SIZE, sizeof(char));
    if (line == NULL) {
        printf("Unable to allocate memory\n");
        exit(71);
    }
    size_t lineLength = 0;
    size_t lineMemory = BUFFER_SIZE;

    while (true) {
        printf(">>> ");

        char buffer[BUFFER_SIZE];
        while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
            while (lineLength + BUFFER_SIZE > lineMemory) {
                lineMemory *= 2;
                line = realloc(line, lineMemory);
                if (line == NULL) {
                    printf("Unable to allocate memory\n");
                    exit(71);
                }
            }
            strcat(line, buffer);
            lineLength += BUFFER_SIZE;
            if (strlen(buffer) != BUFFER_SIZE - 1 || buffer[BUFFER_SIZE-2] == '\n') {
                break;
            }
        }

        interpret(vm, line);
        lineLength = 0;
        line[0] = '\0';
    }

    #undef BUFFER_SIZE
    free(line);
    #endif
}

static void runFile(VM *vm, int argc, const char *argv[]) {
    UNUSED(argc);

    char *source = readFile(argv[1]);
    InterpretResult result = interpret(vm, source);
    free(source); // [owner]

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char *argv[]) {
    #ifdef _WIN32
    atexit(cleanupSockets);
    WORD versionWanted = MAKEWORD(2, 2);
    WSADATA wsaData;
    WSAStartup(versionWanted, &wsaData);
    #endif

    VM *vm = initVM(argc == 1, argc >= 2 ? argv[1] : "repl", argc, argv);

    if (argc == 1) {
        repl(vm, argc, argv);
    } else if (argc >= 2) {
        runFile(vm, argc, argv);
    } else {
        fprintf(stderr, "Usage: dictu [path] [args]\n");
        exit(64);
    }

    freeVM(vm);
    return 0;
}
