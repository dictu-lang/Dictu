#include "sets.h"

bool setMethods(char *method, int argCount) {
    if (strcmp(method, "add") == 0) {
        // TODO: Add method for sets
    } else if (strcmp(method, "remove") == 0) {
        // TODO: Remove method for sets
    } else if (strcmp(method, "contains") == 0) {
        // TODO: Contains method for sets
    }

    runtimeError("Set has no method %s()", method);
    return false;
}
