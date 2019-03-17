#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "memory.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_PREFIX,      // ++ --
    PREC_CALL,        // . () []
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(bool canAssign);


typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

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

typedef struct Compiler {
    struct Compiler *enclosing;

    ObjFunction *function;
    FunctionType type;

    Local locals[UINT8_COUNT];

    int localCount;
    Upvalue upvalues[UINT8_COUNT];

    int scopeDepth;

    int loopDepth;
} Compiler;

typedef struct ClassCompiler {
    struct ClassCompiler *enclosing;
    Token name;
    bool hasSuperclass;
} ClassCompiler;

Parser parser;

Compiler *current = NULL;

ClassCompiler *currentClass = NULL;

bool staticMethod = false;

static Chunk *currentChunk() {
    return &current->function->chunk;
}

static void errorAt(Token *token, const char *message) {
    if (parser.panicMode) return;
    parser.panicMode = true;

    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing.
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void error(const char *message) {
    errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char *message) {
    errorAt(&parser.current, message);
}

static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char *message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static bool check(TokenType type) {
    return parser.current.type == type;
}

static bool match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

static void emitLoop(int loopStart) {
    emitByte(OP_LOOP);

    int offset = currentChunk()->count - loopStart + 2;
    if (offset > UINT16_MAX) error("Loop body too large.");

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

// Emits [instruction] followed by a placeholder for a jump offset. The
// placeholder can be patched by calling [jumpPatch]. Returns the index
// of the placeholder.
static int emitJump(uint8_t instruction) {
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk()->count - 2;
}

static void emitReturn() {
    // An initializer automatically returns "this".
    if (current->type == TYPE_INITIALIZER) {
        emitBytes(OP_GET_LOCAL, 0);
    } else {
        emitByte(OP_NIL);
    }

    emitByte(OP_RETURN);
}

static uint8_t makeConstant(Value value) {
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t) constant;
}

static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

// Replaces the placeholder argument for a previous CODE_JUMP or
// CODE_JUMP_IF instruction with an offset that jumps to the current
// end of bytecode.
static void patchJump(int offset) {
    // -2 to adjust for the bytecode for the jump offset itself.
    int jump = currentChunk()->count - offset - 2;

    if (jump > UINT16_MAX) {
        error("Too much code to jump over.");
    }

    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset + 1] = jump & 0xff;
}

static void initCompiler(Compiler *compiler, int scopeDepth,
                         FunctionType type) {
    compiler->enclosing = current;
    compiler->function = NULL;
    compiler->type = type;
    compiler->localCount = 0;
    compiler->scopeDepth = scopeDepth;
    compiler->loopDepth = 0;
    compiler->function = newFunction(type == TYPE_STATIC);
    current = compiler;

    switch (type) {
        case TYPE_INITIALIZER:
        case TYPE_METHOD:
        case TYPE_STATIC:
        case TYPE_FUNCTION:
            current->function->name = copyString(parser.previous.start,
                                                 parser.previous.length);
            break;
        case TYPE_TOP_LEVEL:
            current->function->name = NULL;
            break;
    }

    Local *local = &current->locals[current->localCount++];
    local->depth = current->scopeDepth;
    local->isUpvalue = false;
    if (type != TYPE_FUNCTION && type != TYPE_STATIC) {
        // In a method, it holds the receiver, "this".
        local->name.start = "this";
        local->name.length = 4;
    } else {
        // In a function, it holds the function, but cannot be referenced,
        // so has no name.
        local->name.start = "";
        local->name.length = 0;
    }
}

static ObjFunction *endCompiler() {
    emitReturn();

    ObjFunction *function = current->function;
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {

      disassembleChunk(currentChunk(),
          function->name != NULL ? function->name->chars : "<top>");
    }
#endif

    current = current->enclosing;

    return function;
}

static void beginScope() {
    current->scopeDepth++;
}

static void endScope() {
    current->scopeDepth--;

    while (current->localCount > 0 &&
           current->locals[current->localCount - 1].depth >
           current->scopeDepth) {

        if (current->locals[current->localCount - 1].isUpvalue) {
            emitByte(OP_CLOSE_UPVALUE);
        } else {
            emitByte(OP_POP);
        }
        current->localCount--;
    }
}

static void expression();

static void statement();

static void declaration();

static ParseRule *getRule(TokenType type);

static void parsePrecedence(Precedence precedence);

static uint8_t identifierConstant(Token *name) {
    return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

static bool identifiersEqual(Token *a, Token *b) {
    if (a->length != b->length) return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

static int resolveLocal(Compiler *compiler, Token *name,
                        bool inFunction) {
    // Look it up in the local scopes. Look in reverse order so that the
    // most nested variable is found first and shadows outer ones.
    for (int i = compiler->localCount - 1; i >= 0; i--) {
        Local *local = &compiler->locals[i];
        if (identifiersEqual(name, &local->name)) {
            if (!inFunction && local->depth == -1) {
                error("Cannot read local variable in its own initializer.");
            }
            return i;
        }
    }

    return -1;
}

// Adds an upvalue to [compiler]'s function with the given properties.
// Does not add one if an upvalue for that variable is already in the
// list. Returns the index of the upvalue.
static int addUpvalue(Compiler *compiler, uint8_t index, bool isLocal) {
    // Look for an existing one.
    int upvalueCount = compiler->function->upvalueCount;
    for (int i = 0; i < upvalueCount; i++) {
        Upvalue *upvalue = &compiler->upvalues[i];
        if (upvalue->index == index && upvalue->isLocal == isLocal) {
            return i;
        }
    }

    // If we got here, it's a new upvalue.
    if (upvalueCount == UINT8_COUNT) {
        error("Too many closure variables in function.");
        return 0;
    }

    compiler->upvalues[upvalueCount].isLocal = isLocal;
    compiler->upvalues[upvalueCount].index = index;
    return compiler->function->upvalueCount++;
}

// Attempts to look up [name] in the functions enclosing the one being
// compiled by [compiler]. If found, it adds an upvalue for it to this
// compiler's list of upvalues (unless it's already in there) and
// returns its index. If not found, returns -1.
//
// If the name is found outside of the immediately enclosing function,
// this will flatten the closure and add upvalues to all of the
// intermediate functions so that it gets walked down to this one.
static int resolveUpvalue(Compiler *compiler, Token *name) {
    // If we are at the top level, we didn't find it.
    if (compiler->enclosing == NULL) return -1;

    // See if it's a local variable in the immediately enclosing function.
    int local = resolveLocal(compiler->enclosing, name, true);
    if (local != -1) {
        // Mark the local as an upvalue so we know to close it when it goes
        // out of scope.
        compiler->enclosing->locals[local].isUpvalue = true;
        return addUpvalue(compiler, (uint8_t) local, true);
    }

    // See if it's an upvalue in the immediately enclosing function. In
    // other words, if it's a local variable in a non-immediately
    // enclosing function. This "flattens" closures automatically: it
    // adds upvalues to all of the intermediate functions to get from the
    // function where a local is declared all the way into the possibly
    // deeply nested function that is closing over it.
    int upvalue = resolveUpvalue(compiler->enclosing, name);
    if (upvalue != -1) {
        return addUpvalue(compiler, (uint8_t) upvalue, false);
    }

    // If we got here, we walked all the way up the parent chain and
    // couldn't find it.
    return -1;
}

static void addLocal(Token name) {
    if (current->localCount == UINT8_COUNT) {
        error("Too many local variables in function.");
        return;
    }

    Local *local = &current->locals[current->localCount];
    local->name = name;

    // The local is declared but not yet defined.
    local->depth = -1;
    local->isUpvalue = false;
    current->localCount++;
}

// Allocates a local slot for the value currently on the stack, if
// we're in a local scope.
static void declareVariable() {
    // Global variables are implicitly declared.
    if (current->scopeDepth == 0) return;

    // See if a local variable with this name is already declared in this
    // scope.
    Token *name = &parser.previous;
    for (int i = current->localCount - 1; i >= 0; i--) {
        Local *local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scopeDepth) break;
        if (identifiersEqual(name, &local->name)) {
            error("Variable with this name already declared in this scope.");
        }
    }

    addLocal(*name);
}

static uint8_t parseVariable(const char *errorMessage) {
    consume(TOKEN_IDENTIFIER, errorMessage);

    // If it's a global variable, create a string constant for it.
    if (current->scopeDepth == 0) {
        return identifierConstant(&parser.previous);
    }

    declareVariable();
    return 0;
}

static void defineVariable(uint8_t global) {
    if (current->scopeDepth == 0) {
        emitBytes(OP_DEFINE_GLOBAL, global);
    } else {
        // Mark the local as defined now.
        current->locals[current->localCount - 1].depth =
                current->scopeDepth;
    }
}

static uint8_t argumentList() {
    uint8_t argCount = 0;
    if (!check(TOKEN_RIGHT_PAREN)) {
        do {
            expression();
            argCount++;

            if (argCount > 32) {
                error("Cannot have more than 32 arguments.");
            }
        } while (match(TOKEN_COMMA));
    }

    consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");

    return argCount;
}

static void and_(bool canAssign) {
    // left operand...
    // OP_JUMP_IF       ------.
    // OP_POP // left operand |
    // right operand...       |
    //   <--------------------'
    // ...

    // Short circuit if the left operand is false.
    int endJump = emitJump(OP_JUMP_IF_FALSE);

    // Compile the right operand.
    emitByte(OP_POP); // Left operand.
    parsePrecedence(PREC_AND);

    patchJump(endJump);
}

static void binary(bool canAssign) {
    TokenType operatorType = parser.previous.type;

    ParseRule *rule = getRule(operatorType);
    parsePrecedence((Precedence) (rule->precedence + 1));

    switch (operatorType) {
        case TOKEN_BANG_EQUAL:
            emitBytes(OP_EQUAL, OP_NOT);
            break;
        case TOKEN_EQUAL_EQUAL:
            emitByte(OP_EQUAL);
            break;
        case TOKEN_GREATER:
            emitByte(OP_GREATER);
            break;
        case TOKEN_GREATER_EQUAL:
            emitBytes(OP_LESS, OP_NOT);
            break;
        case TOKEN_LESS:
            emitByte(OP_LESS);
            break;
        case TOKEN_LESS_EQUAL:
            emitBytes(OP_GREATER, OP_NOT);
            break;
        case TOKEN_PLUS:
            emitByte(OP_ADD);
            break;
        case TOKEN_MINUS:
            emitByte(OP_SUBTRACT);
            break;
        case TOKEN_STAR:
            emitByte(OP_MULTIPLY);
            break;
        case TOKEN_SLASH:
            emitByte(OP_DIVIDE);
            break;
        case TOKEN_PERCENT:
            emitByte(OP_MOD);
            break;
        default:
            return;
    }
}

static void call(bool canAssign) {
    uint8_t argCount = argumentList();
    emitByte(OP_CALL_0 + argCount);
}

static void dot(bool canAssign) {
    consume(TOKEN_IDENTIFIER, "Expect property name after '.'.");
    uint8_t name = identifierConstant(&parser.previous);

    if (canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitBytes(OP_SET_PROPERTY, name);
    } else if (match(TOKEN_LEFT_PAREN)) {
        uint8_t argCount = argumentList();
        emitBytes(OP_INVOKE_0 + argCount, name);
    } else {
        emitBytes(OP_GET_PROPERTY, name);
    }
}

static void literal(bool canAssign) {
    switch (parser.previous.type) {
        case TOKEN_FALSE:
            emitByte(OP_FALSE);
            break;
        case TOKEN_NIL:
            emitByte(OP_NIL);
            break;
        case TOKEN_TRUE:
            emitByte(OP_TRUE);
            break;
        default:
            return; // Unreachable.
    }
}

static void grouping(bool canAssign) {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number(bool canAssign) {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

static void or_(bool canAssign) {
    // left operand...
    // OP_JUMP_IF       ---.
    // OP_JUMP          ---+--.
    //   <-----------------'  |
    // OP_POP // left operand |
    // right operand...       |
    //   <--------------------'
    // ...

    // If the operand is *true* we want to keep it, so when it's false,
    // jump to the code to evaluate the right operand.
    int elseJump = emitJump(OP_JUMP_IF_FALSE);

    // If we get here, the operand is true, so jump to the end to keep it.
    int endJump = emitJump(OP_JUMP);

    // Compile the right operand.
    patchJump(elseJump);
    emitByte(OP_POP); // Left operand.

    parsePrecedence(PREC_OR);
    patchJump(endJump);
}

static void string(bool canAssign) {
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1,
                                    parser.previous.length - 2)));
}

static void list(bool canAssign) {
    emitByte(OP_NEW_LIST);

    do {
        if (check(TOKEN_RIGHT_BRACKET))
            break;

        expression();
        emitByte(OP_ADD_LIST);
    } while (match(TOKEN_COMMA));

    consume(TOKEN_RIGHT_BRACKET, "Expected closing ']'");
}

static void dict(bool canAssign) {
    emitByte(OP_NEW_DICT);

    do {
        if (check(TOKEN_RIGHT_BRACE))
            break;

        expression();
        consume(TOKEN_COLON, "Expected ':'");
        expression();
        emitByte(OP_ADD_DICT);
    } while (match(TOKEN_COMMA));

    consume(TOKEN_RIGHT_BRACE, "Expected closing '}'");
}

static void subscript(bool canAssign) {
    expression();

    TokenType type;
    if (parser.previous.type == TOKEN_NUMBER) {
        type = TOKEN_NUMBER;
    } else {
        type = TOKEN_STRING;
    }

    consume(TOKEN_RIGHT_BRACKET, "Expected closing ']'");

    // Number means its a list subscript
    if (type == TOKEN_NUMBER) {
        if (match(TOKEN_EQUAL)) {
            expression();
            emitByte(OP_SUBSCRIPT_ASSIGN);
        } else {
            emitByte(OP_SUBSCRIPT);
        }
    } else {
        // Dict subscript
        if (match(TOKEN_EQUAL)) {
            expression();
            emitByte(OP_SUBSCRIPT_DICT_ASSIGN);
        } else {
            emitByte(OP_SUBSCRIPT_DICT);
        }
    }
}

static void namedVariable(Token name, bool canAssign) {
    uint8_t getOp, setOp;
    int arg = resolveLocal(current, &name, false);
    if (arg != -1) {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else if ((arg = resolveUpvalue(current, &name)) != -1) {
        getOp = OP_GET_UPVALUE;
        setOp = OP_SET_UPVALUE;
    } else {
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitBytes(setOp, (uint8_t) arg);
    } else {
        emitBytes(getOp, (uint8_t) arg);
    }
}

static void variable(bool canAssign) {
    if (parser.current.type == TOKEN_IDENTIFIER || parser.current.type == TOKEN_EQUAL) {
        namedVariable(parser.previous, canAssign);
    } else {
        Token name = parser.previous;
        uint8_t setOp;
        namedVariable(name, false);

        if (match(TOKEN_PLUS_EQUALS)) {
            expression();
            emitByte(OP_ADD);
        } else if (match(TOKEN_MINUS_EQUALS)) {
            expression();
            emitByte(OP_SUBTRACT);
        } else if (match(TOKEN_MULTIPLY_EQUALS)) {
            expression();
            emitByte(OP_MULTIPLY);
        } else if (match(TOKEN_DIVIDE_EQUALS)) {
            expression();
            emitByte(OP_DIVIDE);
        }

        int arg = resolveLocal(current, &name, false);
        if (arg != -1) {
            setOp = OP_SET_LOCAL;
        } else if ((arg = resolveUpvalue(current, &name)) != -1) {
            setOp = OP_SET_UPVALUE;
        } else {
            arg = identifierConstant(&name);
            setOp = OP_SET_GLOBAL;
        }

        emitBytes(setOp, (uint8_t) arg);
    }
}

static Token syntheticToken(const char *text) {
    Token token;
    token.start = text;
    token.length = (int) strlen(text);
    return token;
}

static void pushSuperclass() {
    if (currentClass == NULL) return;
    namedVariable(syntheticToken("super"), false);
}

static void super_(bool canAssign) {
    if (currentClass == NULL) {
        error("Cannot use 'super' outside of a class.");
    } else if (!currentClass->hasSuperclass) {
        error("Cannot use 'super' in a class with no superclass.");
    }

    consume(TOKEN_DOT, "Expect '.' after 'super'.");
    consume(TOKEN_IDENTIFIER, "Expect superclass method name.");
    uint8_t name = identifierConstant(&parser.previous);

    // Push the receiver.
    namedVariable(syntheticToken("this"), false);

    if (match(TOKEN_LEFT_PAREN)) {
        uint8_t argCount = argumentList();

        pushSuperclass();
        emitBytes(OP_SUPER_0 + argCount, name);
    } else {
        pushSuperclass();
        emitBytes(OP_GET_SUPER, name);
    }
}

static void this_(bool canAssign) {
    if (currentClass == NULL) {
        error("Cannot use 'this' outside of a class.");
    } else if (staticMethod) {
        error("Cannot use 'this' inside a static method.");
    } else {
        variable(false);
    }
}

static void static_(bool canAssign) {
    if (currentClass == NULL) {
        error("Cannot use 'static' outside of a class.");
    }
}

static void unary(bool canAssign) {
    TokenType operatorType = parser.previous.type;

    parsePrecedence(PREC_UNARY);

    switch (operatorType) {
        case TOKEN_BANG:
            emitByte(OP_NOT);
            break;
        case TOKEN_MINUS:
            emitByte(OP_NEGATE);
            break;
        default:
            return;
    }
}

static void prefix(bool canAssign) {
    TokenType operatorType = parser.previous.type;
    parsePrecedence(PREC_PREFIX);

    switch (operatorType) {
        case TOKEN_INCREMENT: {
            emitByte(OP_INCREMENT);
            break;
        }
        case TOKEN_DECREMENT:
            emitByte(OP_DECREMENT);
            break;
        default:
            return;
    }

    uint8_t setOp;
    Token name = parser.previous;

    int arg = resolveLocal(current, &name, false);
    if (arg != -1) {
        setOp = OP_SET_LOCAL;
    } else if ((arg = resolveUpvalue(current, &name)) != -1) {
        setOp = OP_SET_UPVALUE;
    } else {
        arg = identifierConstant(&name);
        setOp = OP_SET_GLOBAL;
    }

    emitBytes(setOp, (uint8_t) arg);
}

ParseRule rules[] = {
        {grouping, call,         PREC_CALL},               // TOKEN_LEFT_PAREN
        {NULL,     NULL,         PREC_NONE},               // TOKEN_RIGHT_PAREN
        {dict,     NULL,         PREC_NONE},               // TOKEN_LEFT_BRACE [big]
        {NULL,     NULL,         PREC_NONE},               // TOKEN_RIGHT_BRACE
        {list,     subscript,    PREC_CALL},               // TOKEN_LEFT_BRACKET
        {NULL,     NULL,         PREC_NONE},               // TOKEN_RIGHT_BRACKET
        {NULL,     NULL,         PREC_NONE},               // TOKEN_COMMA
        {NULL,     dot,          PREC_CALL},               // TOKEN_DOT
        {unary,    binary,       PREC_TERM},               // TOKEN_MINUS
        {NULL,     binary,       PREC_TERM},               // TOKEN_PLUS
        {prefix,   NULL,         PREC_PREFIX},             // TOKEN_INCREMENT
        {prefix,   NULL,         PREC_PREFIX},             // TOKEN_DECREMENT
        {NULL,     NULL,         PREC_NONE},               // TOKEN_PLUS_EQUALS
        {NULL,     NULL,         PREC_NONE},               // TOKEN_MINUS_EQUALS
        {NULL,     NULL,         PREC_NONE},               // TOKEN_MULTIPLY_EQUALS
        {NULL,     NULL,         PREC_NONE},               // TOKEN_DIVIDE_EQUALS
        {NULL,     NULL,         PREC_NONE},               // TOKEN_SEMICOLON
        {NULL,     NULL,         PREC_NONE},               // TOKEN_COLON
        {NULL,     binary,       PREC_FACTOR},             // TOKEN_SLASH
        {NULL,     binary,       PREC_FACTOR},             // TOKEN_STAR
        {NULL,     binary,       PREC_FACTOR},             // TOKEN_PERCENT
        {unary,    NULL,         PREC_NONE},               // TOKEN_BANG
        {NULL,     binary,       PREC_EQUALITY},           // TOKEN_BANG_EQUAL
        {NULL,     NULL,         PREC_NONE},               // TOKEN_EQUAL
        {NULL,     binary,       PREC_EQUALITY},           // TOKEN_EQUAL_EQUAL
        {NULL,     binary,       PREC_COMPARISON},         // TOKEN_GREATER
        {NULL,     binary,       PREC_COMPARISON},         // TOKEN_GREATER_EQUAL
        {NULL,     binary,       PREC_COMPARISON},         // TOKEN_LESS
        {NULL,     binary,       PREC_COMPARISON},         // TOKEN_LESS_EQUAL
        {variable, NULL,         PREC_NONE},               // TOKEN_IDENTIFIER
        {string,   NULL,         PREC_NONE},               // TOKEN_STRING
        {number,   NULL,         PREC_NONE},               // TOKEN_NUMBER
        {NULL,     NULL,         PREC_NONE},               // TOKEN_CLASS
        {static_,  NULL,         PREC_NONE},               // TOKEN_STATIC
        {this_,    NULL,         PREC_NONE},               // TOKEN_THIS
        {super_,   NULL,         PREC_NONE},               // TOKEN_SUPER
        {NULL,     NULL,         PREC_NONE},               // TOKEN_DEF
        {NULL,     NULL,         PREC_NONE},               // TOKEN_IF
        {NULL,     and_,         PREC_AND},                // TOKEN_AND
        {NULL,     NULL,         PREC_NONE},               // TOKEN_ELSE
        {NULL,     or_,          PREC_OR},                 // TOKEN_OR
        {NULL,     NULL,         PREC_NONE},               // TOKEN_VAR
        {literal,  NULL,         PREC_NONE},               // TOKEN_TRUE
        {literal,  NULL,         PREC_NONE},               // TOKEN_FALSE
        {literal,  NULL,         PREC_NONE},               // TOKEN_NIL
        {NULL,     NULL,         PREC_NONE},               // TOKEN_FOR
        {NULL,     NULL,         PREC_NONE},               // TOKEN_WHILE
        {NULL,     NULL,         PREC_NONE},               // TOKEN_BREAK
        {NULL,     NULL,         PREC_NONE},               // TOKEN_RETURN
        {NULL,     NULL,         PREC_NONE},               // TOKEN_WITH
        {NULL,     NULL,         PREC_NONE},               // TOKEN_EOF
        {NULL,     NULL,         PREC_NONE},               // TOKEN_IMPORT
        {NULL,     NULL,         PREC_NONE},               // TOKEN_ERROR
};

static void parsePrecedence(Precedence precedence) {
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL) {
        error("Expect expression.");
        return;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule(canAssign);
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        // If we get here, we didn't parse the "=" even though we could
        // have, so the LHS must not be a valid lvalue.
        error("Invalid assignment target.");
        expression();
    }
}

static ParseRule *getRule(TokenType type) {
    return &rules[type];
}

void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

static void block() {
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void function(FunctionType type) {
    Compiler compiler;
    initCompiler(&compiler, 1, type);

    // Compile the parameter list.
    consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");

    if (!check(TOKEN_RIGHT_PAREN)) {
        do {
            uint8_t paramConstant = parseVariable("Expect parameter name.");
            defineVariable(paramConstant);

            current->function->arity++;
            if (current->function->arity > 32) {
                error("Cannot have more than 32 parameters.");
            }
        } while (match(TOKEN_COMMA));
    }

    consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");

    // The body.
    consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    block();

    // Create the function object.
    endScope();

    if (type == TYPE_STATIC) {
        // Reset 'staticMethod' for next function
        staticMethod = false;
    }

    ObjFunction *function = endCompiler();

    // Capture the upvalues in the new closure object.
    emitBytes(OP_CLOSURE, makeConstant(OBJ_VAL(function)));

    // Emit arguments for each upvalue to know whether to capture a local
    // or an upvalue.
    for (int i = 0; i < function->upvalueCount; i++) {
        emitByte(compiler.upvalues[i].isLocal ? 1 : 0);
        emitByte(compiler.upvalues[i].index);
    }
}

static void method() {
    FunctionType type;

    if (check(TOKEN_STATIC)) {
        type = TYPE_STATIC;
        consume(TOKEN_STATIC, "Expect static.");
        staticMethod = true;
    } else {
        type = TYPE_METHOD;
    }

    consume(TOKEN_IDENTIFIER, "Expect method name.");
    uint8_t constant = identifierConstant(&parser.previous);

    // If the method is named "init", it's an initializer.
    if (parser.previous.length == 4 &&
        memcmp(parser.previous.start, "init", 4) == 0) {
        type = TYPE_INITIALIZER;
    }

    function(type);

    emitBytes(OP_METHOD, constant);
}

static void classDeclaration() {
    consume(TOKEN_IDENTIFIER, "Expect class name.");
    uint8_t nameConstant = identifierConstant(&parser.previous);
    declareVariable();

    ClassCompiler classCompiler;
    classCompiler.name = parser.previous;
    classCompiler.hasSuperclass = false;
    classCompiler.enclosing = currentClass;
    currentClass = &classCompiler;

    if (match(TOKEN_LESS)) {
        consume(TOKEN_IDENTIFIER, "Expect superclass name.");
        classCompiler.hasSuperclass = true;

        beginScope();

        // Store the superclass in a local variable named "super".
        variable(false);
        addLocal(syntheticToken("super"));

        emitBytes(OP_SUBCLASS, nameConstant);
    } else {
        emitBytes(OP_CLASS, nameConstant);
    }

    consume(TOKEN_LEFT_BRACE, "Expect '{' before class body.");

    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        method();
    }
    consume(TOKEN_RIGHT_BRACE, "Expect '}' after class body.");

    if (classCompiler.hasSuperclass) {
        endScope();
    }

    defineVariable(nameConstant);

    currentClass = currentClass->enclosing;
}

static void funDeclaration() {
    uint8_t global = parseVariable("Expect function name.");
    function(TYPE_FUNCTION);
    defineVariable(global);
}

static void varDeclaration() {
    uint8_t global = parseVariable("Expect variable name.");

    if (match(TOKEN_EQUAL)) {
        // Compile the initializer.
        expression();
    } else {
        // Default to nil.
        emitByte(OP_NIL);
    }
    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    defineVariable(global);
}

static void expressionStatement() {
    expression();
    emitByte(OP_POP_REPL);
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
}

/*
static void forStatement() {
    // for (var i = 0; i < 10; i = i + 1) print i;
    //
    //   var i = 0;
    // start:                      <--.
    //   if (i < 10) goto exit;  --.  |
    //   goto body;  -----------.  |  |
    // increment:            <--+--+--+--.
    //   i = i + 1;             |  |  |  |
    //   goto start;  ----------+--+--'  |
    // body:                 <--'  |     |
    //   print i;                  |     |
    //   goto increment;  ---------+-----'
    // exit:                    <--'
    // for (2;10;1)

    // Create a scope for the loop variable.
    beginScope();

    // The initialization clause.
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
    int start = 0, start1 = 0, start2 = 0;

    if (match(TOKEN_NUMBER)) {
        //Get number substring
        char *t = strndup(parser.previous.start, parser.previous.length);
        //Convert string to int
        start = strtol(t, NULL, 10);
        consume(TOKEN_SEMICOLON, "Expect ';' after 'number'");
    }

    loopVarDeclaration(start);

    int loopStart = currentChunk()->count;
    int exitJump = -1;

    // The exit condition.
    //match(TOKEN_MINUS);

    if (match(TOKEN_NUMBER)) {
        //Get number substring
        char *t1 = strndup(parser.previous.start, parser.previous.length);
        //Convert string to int
        start1 = strtol(t1, NULL, 10);
        consume(TOKEN_SEMICOLON, "Expect ';' after 'number'");
    }

    //if (match(TOKEN_MINUS)) {
    //    consume(TOKEN_MINUS, "Expect '-' before 'number'");
    //}

    if (match(TOKEN_NUMBER)) {
        //Get number substring
        char *t2 = strndup(parser.previous.start, parser.previous.length);
        start2 = strtod(t2, NULL);
        printf("%d\n", start2);
        printf("%d\n", -10);
    }

    //emitByte(OP_ADD);

    // Compile the body.
    statement();

    // Jump back to the beginning (or the increment).
    //emitLoop(loopStart);

    if (exitJump != -1) {
        printf("%d", loopStart);
        patchJump(exitJump);
        emitByte(OP_POP); // Condition.
    }

    endScope(); // Loop variable.
}
 */


static void forStatement() {
    // for (var i = 0; i < 10; i = i + 1) print i;
    //
    //   var i = 0;
    // start:                      <--.
    //   if (i < 10) goto exit;  --.  |
    //   goto body;  -----------.  |  |
    // increment:            <--+--+--+--.
    //   i = i + 1;             |  |  |  |
    //   goto start;  ----------+--+--'  |
    // body:                 <--'  |     |
    //   print i;                  |     |
    //   goto increment;  ---------+-----'
    // exit:                    <--'

    // Create a scope for the loop variable.
    beginScope();
    current->loopDepth++;

    // The initialization clause.
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
    if (match(TOKEN_VAR)) {
        varDeclaration();
    } else if (match(TOKEN_SEMICOLON)) {
        // No initializer.
    } else {
        expressionStatement();
    }

    int loopStart = currentChunk()->count;

    // The exit condition.
    int exitJump = -1;

    if (!match(TOKEN_SEMICOLON)) {
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        // Jump out of the loop if the condition is false.
        exitJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP); // Condition.
    }

    // Increment step.
    if (!match(TOKEN_RIGHT_PAREN)) {
        // We don't want to execute the increment before the body, so jump
        // over it.
        int bodyJump = emitJump(OP_JUMP);

        int incrementStart = currentChunk()->count;
        expression();
        emitByte(OP_POP);
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        // After the increment, start the whole loop over.
        emitLoop(loopStart);

        // At the end of the body, we want to jump to the increment, not
        // the top of the loop.
        loopStart = incrementStart;

        patchJump(bodyJump);
    }

    // Compile the body.
    statement();

    // Jump back to the beginning (or the increment).
    emitLoop(loopStart);

    if (exitJump != -1) {
        patchJump(exitJump);
        emitByte(OP_POP); // Condition.
    }

    endScope(); // Loop variable.
    current->loopDepth--;
}


static void ifStatement() {
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    // Jump to the else branch if the condition is false.
    int elseJump = emitJump(OP_JUMP_IF_FALSE);

    // Compile the then branch.
    emitByte(OP_POP); // Condition.
    statement();

    // Jump over the else branch when the if branch is taken.
    int endJump = emitJump(OP_JUMP);

    // Compile the else branch.
    patchJump(elseJump);
    emitByte(OP_POP); // Condition.

    if (match(TOKEN_ELSE)) statement();

    patchJump(endJump);
}

static void withStatement() {
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'with'.");
    expression();
    consume(TOKEN_COMMA, "Expect comma");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after 'with'.");

    beginScope();

    Local *local = &current->locals[current->localCount++];
    local->depth = current->scopeDepth;
    local->isUpvalue = false;
    local->name = syntheticToken("file");

    emitByte(OP_OPEN_FILE);

    statement();
    endScope();
}

static void returnStatement() {
    if (current->type == TYPE_TOP_LEVEL) {
        error("Cannot return from top-level code.");
    }

    if (match(TOKEN_SEMICOLON)) {
        emitReturn();
    } else {
        if (current->type == TYPE_INITIALIZER) {
            error("Cannot return a value from an initializer.");
        }

        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
        emitByte(OP_RETURN);
    }
}

static void importStatement() {
    consume(TOKEN_STRING, "Expect string after import.");
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1,
                                    parser.previous.length - 2)));
    consume(TOKEN_SEMICOLON, "Expect ';' after import.");

    emitByte(OP_IMPORT);
}

static void breakStatement() {
    if (current->loopDepth == 0) {
        error("Cannot use 'break' outside of a loop.");
        return;
    }

    consume(TOKEN_SEMICOLON, "Expected semicolon after break");
    emitByte(OP_BREAK);
}

static void whileStatement() {
    current->loopDepth++;

    int loopStart = currentChunk()->count;

    if (check(TOKEN_LEFT_BRACE)) {
        emitByte(OP_TRUE);

        //emitByte(OP_POP);
    } else {
        consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
        expression();
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
    }

    // Jump out of the loop if the condition is false.
    int exitJump = emitJump(OP_JUMP_IF_FALSE);

    if (vm.repl)
        emitByte(OP_NO_PRINT);

    // Compile the body.
    emitByte(OP_POP); // Condition.
    statement();

    // Loop back to the start.
    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OP_POP); // Condition.

    current->loopDepth--;
}

static void synchronize() {
    parser.panicMode = false;

    while (parser.current.type != TOKEN_EOF) {
        if (parser.previous.type == TOKEN_SEMICOLON) return;

        switch (parser.current.type) {
            case TOKEN_CLASS:
            case TOKEN_DEF:
            case TOKEN_STATIC:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_BREAK:
            case TOKEN_RETURN:
            case TOKEN_IMPORT:
                return;

            default:
                // Do nothing.
                ;
        }

        advance();
    }
}

static void declaration() {
    if (match(TOKEN_CLASS)) {
        classDeclaration();
    } else if (match(TOKEN_DEF)) {
        funDeclaration();
    } else if (match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        statement();
    }

    if (parser.panicMode) synchronize();
}

static void statement() {
    if (match(TOKEN_FOR)) {
        forStatement();
    } else if (match(TOKEN_IF)) {
        ifStatement();
    } else if (match(TOKEN_RETURN)) {
        returnStatement();
    } else if (match(TOKEN_WITH)) {
        withStatement();
    } else if (match(TOKEN_IMPORT)){
        importStatement();
    } else if (match(TOKEN_BREAK)) {
        breakStatement();
    } else if (match(TOKEN_WHILE)) {
        whileStatement();
    } else if (match(TOKEN_LEFT_BRACE)) {
        Token previous = parser.previous;
        Token current = parser.current;
        if (check(TOKEN_STRING)) {
            for (int i = 0; i < parser.current.length - parser.previous.length + 1; ++i) {
                backTrack();
            }

            parser.current = previous;
            expressionStatement();
            return;
        } else if (check(TOKEN_RIGHT_BRACE)) {
            advance();
            if (check(TOKEN_SEMICOLON)) {
                backTrack();
                backTrack();
                parser.current = previous;
                expressionStatement();
                return;
            }
        }
        parser.current = current;

        beginScope();
        block();
        endScope();
    } else {
        expressionStatement();
    }
}

ObjFunction *compile(const char *source) {
    initScanner(source);
    Compiler mainCompiler;
    initCompiler(&mainCompiler, 0, TYPE_TOP_LEVEL);
    parser.hadError = false;
    parser.panicMode = false;

    advance();

    if (!match(TOKEN_EOF)) {
        do {
            declaration();
        } while (!match(TOKEN_EOF));
    }

    ObjFunction *function = endCompiler();

    // If there was a compile error, the code is not valid, so don't
    // create a function.
    return parser.hadError ? NULL : function;
}

void grayCompilerRoots() {
    Compiler *compiler = current;
    while (compiler != NULL) {
        grayObject((Obj *) compiler->function);
        compiler = compiler->enclosing;
    }
}
