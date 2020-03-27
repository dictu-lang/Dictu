#ifndef dictu_instance_h
#define dictu_instance_h

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "copy.h"
#include "../util.h"

bool instanceMethods(VM *vm, char *method, int argCount);

void declareInstanceMethods(VM *vm);

#endif //dictu_instance_h
