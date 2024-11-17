#ifndef dictu_compiler_h
#define dictu_compiler_h

#include <math.h>

#include "object.h"
#include "scanner.h"

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_BITWISE_OR,  // bitwise or
    PREC_BITWISE_XOR, // bitwise xor
    PREC_BITWISE_AND, // bitwise and
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_INDICES,     // **
    PREC_UNARY,       // ! -
    PREC_CHAIN,       // ?.
    PREC_CALL,        // . () []
    PREC_PRIMARY
} Precedence;

typedef struct {
    // The name of the local variable.
    LangToken name;

    // The depth in the scope chain that this variable was declared at.
    // Zero is the outermost scope--parameters for a method, or the first
    // local block in top level code. One is the scope within that, etc.
    int depth;

    // True if this local variable is captured as an upvalue by a
    // function.
    bool isUpvalue;

    // True if it's a constant value.
    bool constant;

    bool isFile;
} Local;

typedef struct {
    // The index of the local variable or upvalue being captured from the
    // enclosing function.
    uint8_t index;

    // Whether the captured variable is a local or upvalue in the
    // enclosing function.
    bool isLocal;

    // Whether the upvalue was defined as constant
    // Rather than having to walk a compiler chain (nested closures)
    bool constant;
} Upvalue;

typedef struct ClassCompiler {
    struct ClassCompiler *enclosing;
    LangToken name;
    bool hasSuperclass;
    bool staticMethod;
    bool abstractClass;
    Table privateVariables;
} ClassCompiler;

typedef struct Loop {
    struct Loop *enclosing;
    int start;
    int body;
    int end;
    int scopeDepth;
} Loop;

typedef struct {
    DictuVM *vm;
    Scanner scanner;
    LangToken current;
    LangToken previous;
    bool hadError;
    bool panicMode;
    ObjModule *module;
} Parser;

typedef struct Compiler {
    Parser *parser;
    Table stringConstants;

    struct Compiler *enclosing;
    ClassCompiler *class;
    Loop *loop;

    ObjFunction *function;
    FunctionType type;

    Local locals[UINT8_COUNT];

    int localCount;
    Upvalue upvalues[UINT8_COUNT];

    int scopeDepth;
    ObjDict *classAnnotations;
    ObjDict *methodAnnotations;
    ObjDict *fieldAnnotations;
} Compiler;

typedef void (*ParsePrefixFn)(Compiler *compiler, bool canAssign);
typedef void (*ParseInfixFn)(Compiler *compiler, LangToken previousToken, bool canAssign);

typedef struct {
    ParsePrefixFn prefix;
    ParseInfixFn infix;
    Precedence precedence;
} ParseRule;

ObjFunction *compile(DictuVM *vm, ObjModule *module, const char *source);

void grayCompilerRoots(DictuVM *vm);

#endif
