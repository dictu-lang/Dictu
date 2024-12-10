#include "term.h"

#ifdef _WIN32
#define isatty(fd) _isatty(fd)
#endif

static Value termIsattyNative(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "isatty() takes 1 argument (%d given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError(vm, "arg passed to isatty must be a number.");
        return EMPTY_VAL;
    }

    if (isatty(AS_NUMBER(args[0]))) {
        return BOOL_VAL(true);
    }

    return BOOL_VAL(false);
}

static Value getSizeNative(DictuVM *vm, int argCount, Value *args) {
    UNUSED(args);

    if (argCount != 0) {
        runtimeError(vm, "getSize() doesn't take any arguments (%d given).", argCount);
        return EMPTY_VAL;
    }

    ObjDict *terminalSizeDict = newDict(vm);
    push(vm, OBJ_VAL(terminalSizeDict));

    ObjString *lines = copyString(vm, "rows", 4);
    push(vm, OBJ_VAL(lines));

    ObjString *columns = copyString(vm, "columns", 7);
    push(vm, OBJ_VAL(columns));

#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    
    int height = (int)(csbi.srWindow.Bottom-csbi.srWindow.Top+1);
    int width = (int)(csbi.srWindow.Right-csbi.srWindow.Left+1);

    dictSet(vm, terminalSizeDict, OBJ_VAL(lines), NUMBER_VAL(height));
    pop(vm);

    dictSet(vm, terminalSizeDict, OBJ_VAL(columns), NUMBER_VAL(width));
    pop(vm);
#else
    ObjString *horizontalPixesl = copyString(vm, "horizontal_pixels", 17);
    push(vm, OBJ_VAL(horizontalPixesl));

    ObjString *verticalPixels = copyString(vm, "vertical_pixels", 15);
    push(vm, OBJ_VAL(verticalPixels));

    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    dictSet(vm, terminalSizeDict, OBJ_VAL(lines), NUMBER_VAL(w.ws_row));
    pop(vm);

    dictSet(vm, terminalSizeDict, OBJ_VAL(columns), NUMBER_VAL(w.ws_col));
    pop(vm);

    dictSet(vm, terminalSizeDict, OBJ_VAL(horizontalPixesl), NUMBER_VAL(w.ws_col));
    pop(vm);

    dictSet(vm, terminalSizeDict, OBJ_VAL(verticalPixels), NUMBER_VAL(w.ws_col));
    pop(vm);
#endif

    pop(vm);
    
    return OBJ_VAL(terminalSizeDict);
}

Value createTermModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Term", 4);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    /**
     * Define Term methods
     */
    defineNative(vm, &module->values, "isatty", termIsattyNative);
    defineNative(vm, &module->values, "getSize", getSizeNative);

    /**
     * Define Term properties
     */

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}
