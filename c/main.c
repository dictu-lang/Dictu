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

static void repl() {
    printf(VERSION);
    char *line;

    linenoiseHistoryLoad("history.txt");

    while((line = linenoise(">>> ")) != NULL) {
        interpret(line);
        linenoiseHistoryAdd(line);
        linenoiseHistorySave("history.txt");
        free(line);
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
