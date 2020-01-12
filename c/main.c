#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"
#include "util.h"

#include "linenoise.h"

#define VERSION "Dictu Version: 0.1.7\n"


static bool replCountBraces(char *line) {
    int leftBraces = 0;
    int rightBraces = 0;

    for (int i = 0; line[i]; i++) {
        if (line[i] == '{') {
            leftBraces++;
        } else if (line[i] == '}') {
            rightBraces++;
        }
    }

    return leftBraces == rightBraces;
}

static void repl() {
    printf(VERSION);
    char *line;
    char *fullLine;

    linenoiseHistoryLoad("history.txt");

    while((line = linenoise(">>> ")) != NULL) {
        fullLine = malloc(sizeof(char) * (strlen(line) + 1));
        snprintf(fullLine, strlen(line) + 1, "%s", line);

        linenoiseHistoryAdd(line);
        linenoiseHistorySave("history.txt");

        while (!replCountBraces(fullLine)) {
            free(line);
            line = linenoise("... ");

            if (line == NULL) {
                return;
            }

            fullLine = realloc(fullLine, strlen(fullLine) + strlen(line) + 1);
            snprintf(fullLine, strlen(fullLine) + strlen(line) + 1, "%s%s", fullLine, line);

            linenoiseHistoryAdd(line);
            linenoiseHistorySave("history.txt");
        }

        interpret(fullLine);

        free(line);
        free(fullLine);
    }
}

static void runFile(const char *path) {
    char *source = readFile(path);
    InterpretResult result = interpret(source);
    free(source); // [owner]

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char *argv[]) {
    initVM(argc == 1, argc == 2 ? argv[1] : "repl");

    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: dictu [path]\n");
        exit(64);
    }

    freeVM();
    return 0;
}
