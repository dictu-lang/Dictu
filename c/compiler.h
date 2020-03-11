#ifndef dictu_compiler_h
#define dictu_compiler_h

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
    PREC_PREFIX,      // ++ --
    PREC_CALL,        // . () []
    PREC_PRIMARY
} Precedence;

typedef struct {
    // The name of the local variable.
    Token name;

    // The depth in the scope chain that this variable was declared at.
    // Zero is the outermost scope--parameters for a method, or the first
    // local block in top level code. One is the scope within that, etc.
    int depth;

    // True if this local variable is captured as an upvalue by a
    // function.
    bool isUpvalue;
} Local;

typedef struct {
    // The index of the local variable or upvalue being captured from the
    // enclosing function.
    uint8_t index;

    // Whether the captured variable is a local or upvalue in the
    // enclosing function.
    bool isLocal;
} Upvalue;

typedef enum {
    TYPE_FUNCTION,
    TYPE_INITIALIZER,
    TYPE_METHOD,
    TYPE_STATIC,
    TYPE_TOP_LEVEL
} FunctionType;

typedef struct ClassCompiler {
    struct ClassCompiler *enclosing;
    Token name;
    bool hasSuperclass;
    bool staticMethod;
} ClassCompiler;

typedef struct Loop {
    struct Loop *enclosing;
    int start;
    int scopeDepth;
} Loop;

typedef struct Compiler {
    struct Compiler *enclosing;

    ObjFunction *function;
    FunctionType type;

    Local locals[UINT8_COUNT];

    int localCount;
    Upvalue upvalues[UINT8_COUNT];

    int scopeDepth;
} Compiler;

typedef struct {
    VM *vm;
    Compiler *compiler;
    ClassCompiler *class;
    Loop *loop;
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

typedef void (*ParseFn)(Parser *parser, bool canAssign);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

ObjFunction *compile(VM *vm, const char *source);

void grayCompilerRoots(VM *vm);

#endif
