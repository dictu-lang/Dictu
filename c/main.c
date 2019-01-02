//> Chunks of Bytecode main-c
//> Scanning on Demand main-includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//< Scanning on Demand main-includes
#include "common.h"
//> main-include-chunk
#include "chunk.h"
//< main-include-chunk
//> main-include-debug
#include "debug.h"
//< main-include-debug
//> A Virtual Machine main-include-vm
#include "vm.h"

#define VERSION "Dictu Version: 0.0.1\n"

//< A Virtual Machine main-include-vm
//> Scanning on Demand repl
static void repl() {
    printf(VERSION);
    char line[1024];
    for (;;) {
        printf(">>> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret(line);
    }
}

//< Scanning on Demand repl
//> Scanning on Demand read-file
static char *readFile(const char *path) {
    FILE *file = fopen(path, "rb");
//> no-file
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }
//< no-file

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char *buffer = (char *) malloc(fileSize + 1);
//> no-buffer
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

//< no-buffer
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
//> no-read
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

//< no-read
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

//< Scanning on Demand read-file
//> Scanning on Demand run-file
static void runFile(const char *path) {
    char *source = readFile(path);
    InterpretResult result = interpret(source);
    free(source); // [owner]

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}
//< Scanning on Demand run-file

int main(int argc, const char *argv[]) {
//> A Virtual Machine main-init-vm
    initVM(argc == 1);

//> Scanning on Demand args
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: dictu [path]\n");
        exit(64);
    }

    freeVM();
//< Scanning on Demand args
/* A Virtual Machine main-free-vm < Scanning on Demand args
  freeVM();
*/
/* Chunks of Bytecode main-chunk < Scanning on Demand args
  freeChunk(&chunk);
*/
    return 0;
}
