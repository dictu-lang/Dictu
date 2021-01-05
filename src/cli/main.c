#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(__x__) (void) __x__

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

#include "../include/dictu_include.h"
#include "argparse.h"

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

static void repl(DictuVM *vm) {
    printf(DICTU_STRING_VERSION);
    char *line;

    #ifndef DISABLE_LINENOISE
    linenoiseHistoryLoad("history.txt");

    while((line = linenoise(">>> ")) != NULL) {
        int fullLineLength = strlen(line);
        char *fullLine = malloc(sizeof(char) * (fullLineLength + 1));
        snprintf(fullLine, fullLineLength + 1, "%s", line);

        linenoiseHistoryAdd(line);
        linenoiseHistorySave("history.txt");

        while (!replCountBraces(fullLine) || !replCountQuotes(fullLine)) {
            free(line);
            line = linenoise("... ");
            int lineLength = strlen(line);

            if (line == NULL) {
                return;
            }

            char *temp = realloc(fullLine, sizeof(char) * fullLineLength + lineLength);

            if (temp == NULL) {
                printf("Unable to allocate memory\n");
                exit(71);
            }

            fullLine = temp;
            memcpy(fullLine + fullLineLength, line, lineLength);
            fullLineLength += lineLength;

            linenoiseHistoryAdd(line);
            linenoiseHistorySave("history.txt");
        }

        dictuInterpret(vm, "repl", fullLine);

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

        dictuInterpret(vm, "repl", line);
        lineLength = 0;
        line[0] = '\0';
    }

    #undef BUFFER_SIZE
    free(line);
    #endif
}

static char *readFile(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char *buffer = malloc(sizeof(char) * (fileSize + 1));
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

static void runFile(DictuVM *vm, char *filename) {
    char *source = readFile(filename);

    if (source == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", filename);
        exit(74);
    }

    DictuInterpretResult result = dictuInterpret(vm, filename, source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

static const char *const usage[] = {
        "dictu [options] [[--] args]",
        "dictu [options]",
        NULL,
};

int main(int argc, char *argv[]) {
    int version = 0;
    char *cmd = NULL;

    struct argparse_option options[] = {
            OPT_HELP(),
            OPT_BOOLEAN('v', "version", &version, "Display Dictu version"),
            OPT_STRING('c', "cmd", &cmd, "Run program passed in as string"),
            OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usage, 0);
    argc = argparse_parse(&argparse, argc, (const char **)argv);

    if (version) {
        printf(DICTU_STRING_VERSION);
        return 0;
    }

    DictuVM *vm = dictuInitVM(argc == 0, argc, argv);

    if (cmd != NULL) {
        DictuInterpretResult result = dictuInterpret(vm, "repl", cmd);
        if (result == INTERPRET_COMPILE_ERROR) exit(65);
        if (result == INTERPRET_RUNTIME_ERROR) exit(70);
        dictuFreeVM(vm);
        return 0;
    }

    if (argc == 0) {
        repl(vm);
        dictuFreeVM(vm);
        return 0;
    }

    runFile(vm, argv[0]);
    dictuFreeVM(vm);
    return 0;
}
