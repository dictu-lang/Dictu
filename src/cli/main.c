#include <errno.h>
#if defined(__linux__) || defined(_WIN32)
#include <limits.h> 
#elif defined(__APPLE__) || defined(__FreeBSD__)
#include <sys/syslimits.h> 
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include "../optionals/windowsapi.h"
#define PATH_MAX MAX_PATH
#else
#include <unistd.h>
#endif

#define UNUSED(__x__) (void) __x__

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

#include "../include/dictu_include.h"
#include "argparse.h"

#include "linenoise/linenoise.h"

#define DICTU_HOME "/.dictu"
#define DICTU_HIST "/history.txt"

static int matchStringLiteral(char* line, int i) {
    char quote = line[i];
    
    if (quote != '\'' && quote != '"') {
        return i; // not at beginning of string, return same index
    }
    
    while (line[++i]) {
        if (line[i] == '\\') {
            char skipped = line[++i];
            
            if (skipped == '\0') {
                return -1; // string not closed
            }
        } else if (line[i] == quote) {
            return i; // return index of last character of string
        }
    }
    
    return -1; // string not closed
}

static bool matchBraces(char *line) {
    int braceLevel = 0;

    for (int i = 0; line[i]; i++) {
        i = matchStringLiteral(line, i);
        
        if (i == -1) {
            return false;
        }
        
        if (line[i] == '\0') {
            break;
        } else if (line[i] == '{') {
            braceLevel++;
        } else if (line[i] == '}') {
            braceLevel--;
        }
        
        if (braceLevel < 0) {
            return true; // closed brace before opening, end line now
        }
    }

    return braceLevel == 0;
}

static void memcpyAndAppendNul(char* dst, char* src, int len) {
    memcpy(dst, src, len);
    dst[len] = '\0';
}

// getDictuPath sets up the path to be used for saving history
// amongst other things. The returned string needs to be freed
// by the caller.
char *getDictuPath() {
    char *dictuPath = calloc(PATH_MAX, sizeof(char) * PATH_MAX);
    char *tmp;

    if ((tmp = getenv("DICTU_PATH")) != NULL) {
        strncat(dictuPath, tmp, strlen(tmp));
    } else {
        const char *home = getenv("HOME");
        strncpy(dictuPath, home, strlen(home));
        strncat(dictuPath, DICTU_HOME, strlen(DICTU_HOME));
    }

    return dictuPath;
}

static void repl(DictuVM *vm) {
    printf(DICTU_STRING_VERSION);
    char *line;

    char *dictuPath = getDictuPath();

    if (mkdir(dictuPath, 0700) == -1 && errno != EEXIST) {
        fprintf(stderr, "Cannot create directory %s - %s\n", dictuPath, strerror(errno));
        free(dictuPath);
        exit(75);
    }

    strncat(dictuPath, DICTU_HIST, strlen(DICTU_HIST));

    linenoiseHistoryLoad(dictuPath);

    while((line = linenoise(">>> ")) != NULL) {
        int statementLength = strlen(line);
        char *statement = malloc(sizeof(char) * (statementLength + 1));
        memcpyAndAppendNul(statement, line, statementLength);

        linenoiseHistoryAdd(line);
        linenoiseHistorySave(dictuPath);

        while (!matchBraces(statement)) {
            free(line);
            line = linenoise("... ");

            if (line == NULL) {
                return;
            }

            int lineLength = strlen(line);
            statement[statementLength++] = '\n'; // keep newline characters between lines
            char *temp = realloc(statement, sizeof(char) * (statementLength + lineLength + 1));

            if (temp == NULL) {
                printf("Unable to allocate memory\n");
                exit(71);
            }

            statement = temp;
            memcpyAndAppendNul(statement + statementLength, line, lineLength);
            statementLength += lineLength;

            linenoiseHistoryAdd(line);
            linenoiseHistorySave(dictuPath);
        }

        dictuInterpret(vm, "repl", statement);

        free(line);
        free(statement);
    }

    free(dictuPath);
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
    argparse_init(&argparse, options, usage, ARGPARSE_STOP_AT_NON_OPTION);
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
