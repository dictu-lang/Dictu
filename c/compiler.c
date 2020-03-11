#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "memory.h"
#include "vm.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

static Chunk *currentChunk(Parser *parser) {
    return &parser->compiler->function->chunk;
}

static void errorAt(Parser *parser, Token *token, const char *message) {
    if (parser->panicMode) return;
    parser->panicMode = true;

    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing.
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser->hadError = true;
}

static void error(Parser *parser, const char *message) {
    errorAt(parser, &parser->previous, message);
}

static void errorAtCurrent(Parser *parser, const char *message) {
    errorAt(parser, &parser->current, message);
}

static void advance(Parser *parser) {
    parser->previous = parser->current;

    for (;;) {
        parser->current = scanToken();
        if (parser->current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser, parser->current.start);
    }
}

static void consume(Parser *parser, TokenType type, const char *message) {
    if (parser->current.type == type) {
        advance(parser);
        return;
    }

    errorAtCurrent(parser, message);
}

static bool check(Parser *parser, TokenType type) {
    return parser->current.type == type;
}

static bool match(Parser *parser, TokenType type) {
    if (!check(parser, type)) return false;
    advance(parser);
    return true;
}

static void emitByte(Parser *parser, uint8_t byte) {
    writeChunk(parser->vm, currentChunk(parser), byte, parser->previous.line);
}

static void emitBytes(Parser *parser, uint8_t byte1, uint8_t byte2) {
    emitByte(parser, byte1);
    emitByte(parser, byte2);
}

static void emitLoop(Parser *parser, int loopStart) {
    emitByte(parser, OP_LOOP);

    int offset = currentChunk(parser)->count - loopStart + 2;
    if (offset > UINT16_MAX) error(parser, "Loop body too large.");

    emitByte(parser, (offset >> 8) & 0xff);
    emitByte(parser, offset & 0xff);
}

// Emits [instruction] followed by a placeholder for a jump offset. The
// placeholder can be patched by calling [jumpPatch]. Returns the index
// of the placeholder.
static int emitJump(Parser *parser, uint8_t instruction) {
    emitByte(parser, instruction);
    emitByte(parser, 0xff);
    emitByte(parser, 0xff);
    return currentChunk(parser)->count - 2;
}

static void emitReturn(Parser *parser) {
    // An initializer automatically returns "this".
    if (parser->compiler->type == TYPE_INITIALIZER) {
        emitBytes(parser, OP_GET_LOCAL, 0);
    } else {
        emitByte(parser, OP_NIL);
    }

    emitByte(parser, OP_RETURN);
}

static uint8_t makeConstant(Parser *parser, Value value) {
    int constant = addConstant(parser->vm, currentChunk(parser), value);
    if (constant > UINT8_MAX) {
        error(parser, "Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t) constant;
}

static void emitConstant(Parser *parser, Value value) {
    emitBytes(parser, OP_CONSTANT, makeConstant(parser, value));
}

// Replaces the placeholder argument for a previous CODE_JUMP or
// CODE_JUMP_IF instruction with an offset that jumps to the current
// end of bytecode.
static void patchJump(Parser *parser, int offset) {
    // -2 to adjust for the bytecode for the jump offset itself.
    int jump = currentChunk(parser)->count - offset - 2;

    if (jump > UINT16_MAX) {
        error(parser, "Too much code to jump over.");
    }

    currentChunk(parser)->code[offset] = (jump >> 8) & 0xff;
    currentChunk(parser)->code[offset + 1] = jump & 0xff;
}

static void initCompiler(Parser *parser, Compiler *compiler, FunctionType type) {
    compiler->enclosing = parser->compiler;
    compiler->function = NULL;
    compiler->type = type;
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    compiler->function = newFunction(parser->vm, type == TYPE_STATIC);
    parser->compiler = compiler;
    parser->vm->compiler = compiler;

    switch (type) {
        case TYPE_INITIALIZER:
        case TYPE_METHOD:
        case TYPE_STATIC:
        case TYPE_FUNCTION:
            parser->compiler->function->name = copyString(
                    parser->vm,
                    parser->previous.start,
                    parser->previous.length
            );
            break;
        case TYPE_TOP_LEVEL: {
            parser->compiler->function->name = NULL;
            break;
        }
    }

    Local *local = &compiler->locals[compiler->localCount++];
    local->depth = compiler->scopeDepth;
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

static ObjFunction *endCompiler(Parser *parser) {
    emitReturn(parser);

    ObjFunction *function = parser->compiler->function;
#ifdef DEBUG_PRINT_CODE
    if (!parser->hadError) {

      disassembleChunk(currentChunk(parser),
          function->name != NULL ? function->name->chars : "<top>");
    }
#endif

    parser->compiler = parser->compiler->enclosing;

    return function;
}

static void beginScope(Parser *parser) {
    parser->compiler->scopeDepth++;
}

static void endScope(Parser *parser) {
    parser->compiler->scopeDepth--;

    while (parser->compiler->localCount > 0 &&
           parser->compiler->locals[parser->compiler->localCount - 1].depth >
           parser->compiler->scopeDepth) {

        if (parser->compiler->locals[parser->compiler->localCount - 1].isUpvalue) {
            emitByte(parser, OP_CLOSE_UPVALUE);
        } else {
            emitByte(parser, OP_POP);
        }
        parser->compiler->localCount--;
    }
}

static void expression(Parser *parser);

static void statement(Parser *parser);

static void declaration(Parser *parser);

static ParseRule *getRule(TokenType type);

static void parsePrecedence(Parser *parser, Precedence precedence);

static uint8_t identifierConstant(Parser *parser, Token *name) {
    return makeConstant(parser, OBJ_VAL(copyString(parser->vm, name->start, name->length)));
}

static bool identifiersEqual(Token *a, Token *b) {
    if (a->length != b->length) return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

static int resolveLocal(Parser *parser, Compiler *compiler, Token *name,
                        bool inFunction) {
    // Look it up in the local scopes. Look in reverse order so that the
    // most nested variable is found first and shadows outer ones.
    for (int i = compiler->localCount - 1; i >= 0; i--) {
        Local *local = &compiler->locals[i];
        if (identifiersEqual(name, &local->name)) {
            if (!inFunction && local->depth == -1) {
                error(parser, "Cannot read local variable in its own initializer.");
            }
            return i;
        }
    }

    return -1;
}

// Adds an upvalue to [compiler]'s function with the given properties.
// Does not add one if an upvalue for that variable is already in the
// list. Returns the index of the upvalue.
static int addUpvalue(Parser *parser, Compiler *compiler, uint8_t index, bool isLocal) {
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
        error(parser, "Too many closure variables in function.");
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
static int resolveUpvalue(Parser *parser, Compiler *compiler, Token *name) {
    // If we are at the top level, we didn't find it.
    if (compiler->enclosing == NULL) return -1;

    // See if it's a local variable in the immediately enclosing function.
    int local = resolveLocal(parser, compiler->enclosing, name, true);
    if (local != -1) {
        // Mark the local as an upvalue so we know to close it when it goes
        // out of scope.
        compiler->enclosing->locals[local].isUpvalue = true;
        return addUpvalue(parser, compiler, (uint8_t) local, true);
    }

    // See if it's an upvalue in the immediately enclosing function. In
    // other words, if it's a local variable in a non-immediately
    // enclosing function. This "flattens" closures automatically: it
    // adds upvalues to all of the intermediate functions to get from the
    // function where a local is declared all the way into the possibly
    // deeply nested function that is closing over it.
    int upvalue = resolveUpvalue(parser, compiler->enclosing, name);
    if (upvalue != -1) {
        return addUpvalue(parser, compiler, (uint8_t) upvalue, false);
    }

    // If we got here, we walked all the way up the parent chain and
    // couldn't find it.
    return -1;
}

static void addLocal(Parser *parser, Token name) {
    if (parser->compiler->localCount == UINT8_COUNT) {
        error(parser, "Too many local variables in function.");
        return;
    }

    Local *local = &parser->compiler->locals[parser->compiler->localCount];
    local->name = name;

    // The local is declared but not yet defined.
    local->depth = -1;
    local->isUpvalue = false;
    parser->compiler->localCount++;
}

// Allocates a local slot for the value currently on the stack, if
// we're in a local scope.
static void declareVariable(Parser *parser) {
    // Global variables are implicitly declared.
    if (parser->compiler->scopeDepth == 0) return;

    // See if a local variable with this name is already declared in this
    // scope.
    Token *name = &parser->previous;
    for (int i = parser->compiler->localCount - 1; i >= 0; i--) {
        Local *local = &parser->compiler->locals[i];
        if (local->depth != -1 && local->depth < parser->compiler->scopeDepth) break;
        if (identifiersEqual(name, &local->name)) {
            error(parser, "Variable with this name already declared in this scope.");
        }
    }

    addLocal(parser, *name);
}

static uint8_t parseVariable(Parser *parser, const char *errorMessage) {
    consume(parser, TOKEN_IDENTIFIER, errorMessage);

    // If it's a global variable, create a string constant for it.
    if (parser->compiler->scopeDepth == 0) {
        return identifierConstant(parser, &parser->previous);
    }

    declareVariable(parser);
    return 0;
}

static void defineVariable(Parser *parser, uint8_t global) {
    if (parser->compiler->scopeDepth == 0) {
        emitBytes(parser, OP_DEFINE_GLOBAL, global);
    } else {
        // Mark the local as defined now.
        parser->compiler->locals[parser->compiler->localCount - 1].depth =
                parser->compiler->scopeDepth;
    }
}

static int argumentList(Parser *parser) {
    int argCount = 0;
    if (!check(parser, TOKEN_RIGHT_PAREN)) {
        do {
            expression(parser);
            argCount++;

            if (argCount > 255) {
                error(parser, "Cannot have more than 255 arguments.");
            }
        } while (match(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");

    return argCount;
}

static void and_(Parser *parser, bool canAssign) {
    // left operand...
    // OP_JUMP_IF       ------.
    // OP_POP // left operand |
    // right operand...       |
    //   <--------------------'
    // ...

    // Short circuit if the left operand is false.
    int endJump = emitJump(parser, OP_JUMP_IF_FALSE);

    // Compile the right operand.
    emitByte(parser, OP_POP); // Left operand.
    parsePrecedence(parser, PREC_AND);

    patchJump(parser, endJump);
}

static void binary(Parser *parser, bool canAssign) {
    TokenType operatorType = parser->previous.type;

    ParseRule *rule = getRule(operatorType);
    parsePrecedence(parser, (Precedence) (rule->precedence + 1));

    switch (operatorType) {
        case TOKEN_BANG_EQUAL:
            emitBytes(parser, OP_EQUAL, OP_NOT);
            break;
        case TOKEN_EQUAL_EQUAL:
            emitByte(parser, OP_EQUAL);
            break;
        case TOKEN_GREATER:
            emitByte(parser, OP_GREATER);
            break;
        case TOKEN_GREATER_EQUAL:
            emitBytes(parser, OP_LESS, OP_NOT);
            break;
        case TOKEN_LESS:
            emitByte(parser, OP_LESS);
            break;
        case TOKEN_LESS_EQUAL:
            emitBytes(parser, OP_GREATER, OP_NOT);
            break;
        case TOKEN_PLUS:
            emitByte(parser, OP_ADD);
            break;
        case TOKEN_MINUS:
            emitBytes(parser, OP_NEGATE, OP_ADD);
            break;
        case TOKEN_STAR:
            emitByte(parser, OP_MULTIPLY);
            break;
        case TOKEN_STAR_STAR:
            emitByte(parser, OP_POW);
            break;
        case TOKEN_SLASH:
            emitByte(parser, OP_DIVIDE);
            break;
        case TOKEN_PERCENT:
            emitByte(parser, OP_MOD);
            break;
        case TOKEN_AMPERSAND:
            emitByte(parser, OP_BITWISE_AND);
            break;
        case TOKEN_CARET:
            emitByte(parser, OP_BITWISE_XOR);
            break;
        case TOKEN_PIPE:
            emitByte(parser, OP_BITWISE_OR);
            break;
        default:
            return;
    }
}

static void call(Parser *parser, bool canAssign) {
    int argCount = argumentList(parser);
    emitBytes(parser, OP_CALL, argCount);
}

static void dot(Parser *parser, bool canAssign) {
    consume(parser, TOKEN_IDENTIFIER, "Expect property name after '.'.");
    uint8_t name = identifierConstant(parser, &parser->previous);

    if (canAssign && match(parser, TOKEN_EQUAL)) {
        expression(parser);
        emitBytes(parser, OP_SET_PROPERTY, name);
    } else if (match(parser, TOKEN_LEFT_PAREN)) {
        int argCount = argumentList(parser);
        emitBytes(parser, OP_INVOKE, argCount);
        emitByte(parser, name);
    } else if (canAssign && match(parser, TOKEN_PLUS_EQUALS)) {
        emitBytes(parser, OP_GET_PROPERTY_NO_POP, name);
        expression(parser);
        emitByte(parser, OP_ADD);
        emitBytes(parser, OP_SET_PROPERTY, name);
    } else if (canAssign && match(parser, TOKEN_MINUS_EQUALS)) {
        emitBytes(parser, OP_GET_PROPERTY_NO_POP, name);
        expression(parser);
        emitBytes(parser, OP_NEGATE, OP_ADD);
        emitBytes(parser, OP_SET_PROPERTY, name);
    } else if (canAssign && match(parser, TOKEN_MULTIPLY_EQUALS)) {
        emitBytes(parser, OP_GET_PROPERTY_NO_POP, name);
        expression(parser);
        emitByte(parser, OP_MULTIPLY);
        emitBytes(parser, OP_SET_PROPERTY, name);
    } else if (canAssign && match(parser, TOKEN_DIVIDE_EQUALS)) {
        emitBytes(parser, OP_GET_PROPERTY_NO_POP, name);
        expression(parser);
        emitByte(parser, OP_DIVIDE);
        emitBytes(parser, OP_SET_PROPERTY, name);
    } else if (canAssign && match(parser, TOKEN_AMPERSAND_EQUALS)) {
        emitBytes(parser, OP_GET_PROPERTY_NO_POP, name);
        expression(parser);
        emitByte(parser, OP_BITWISE_AND);
        emitBytes(parser, OP_SET_PROPERTY, name);
    } else if (canAssign && match(parser, TOKEN_CARET_EQUALS)) {
        emitBytes(parser, OP_GET_PROPERTY_NO_POP, name);
        expression(parser);
        emitByte(parser, OP_BITWISE_XOR);
        emitBytes(parser, OP_SET_PROPERTY, name);
    } else if (canAssign && match(parser, TOKEN_PIPE_EQUALS)) {
        emitBytes(parser, OP_GET_PROPERTY_NO_POP, name);
        expression(parser);
        emitByte(parser, OP_BITWISE_OR);
        emitBytes(parser, OP_SET_PROPERTY, name);
    } else {
        emitBytes(parser, OP_GET_PROPERTY, name);
    }
}

static void literal(Parser *parser, bool canAssign) {
    switch (parser->previous.type) {
        case TOKEN_FALSE:
            emitByte(parser, OP_FALSE);
            break;
        case TOKEN_NIL:
            emitByte(parser, OP_NIL);
            break;
        case TOKEN_TRUE:
            emitByte(parser, OP_TRUE);
            break;
        default:
            return; // Unreachable.
    }
}

static void grouping(Parser *parser, bool canAssign) {
    expression(parser);
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number(Parser *parser, bool canAssign) {
    double value = strtod(parser->previous.start, NULL);
    emitConstant(parser, NUMBER_VAL(value));
}

static void or_(Parser *parser, bool canAssign) {
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
    int elseJump = emitJump(parser, OP_JUMP_IF_FALSE);

    // If we get here, the operand is true, so jump to the end to keep it.
    int endJump = emitJump(parser, OP_JUMP);

    // Compile the right operand.
    patchJump(parser, elseJump);
    emitByte(parser, OP_POP); // Left operand.

    parsePrecedence(parser, PREC_OR);
    patchJump(parser, endJump);
}

int parseString(char *string, int length) {
    for (int i = 0; i < length - 1; i++) {
        if (string[i] == '\\') {
            switch (string[i + 1]) {
                case 'n': {
                    string[i + 1] = '\n';
                    break;
                }
                case 't': {
                    string[i + 1] = '\t';
                    break;
                }
                case 'r': {
                    string[i + 1] = '\r';
                    break;
                }
                case 'v': {
                    string[i + 1] = '\v';
                    break;
                }
                case '\'':
                case '"': {
                    break;
                }
                default: {
                    continue;
                }
            }
            memmove(&string[i], &string[i + 1], length - i);
            length -= 1;
        }
    }

    return length;
}

static void string(Parser *parser, bool canAssign) {
    char *string = malloc(sizeof(char) * parser->previous.length - 1);
    memcpy(string, parser->previous.start + 1, parser->previous.length - 2);
    int length = parseString(string, parser->previous.length - 2);
    string[length] = '\0';

    emitConstant(parser, OBJ_VAL(copyString(parser->vm, string, length)));

    free(string);
}

static void list(Parser *parser, bool canAssign) {
    emitByte(parser, OP_NEW_LIST);

    do {
        if (check(parser, TOKEN_RIGHT_BRACKET))
            break;

        expression(parser);
        emitByte(parser, OP_ADD_LIST);
    } while (match(parser, TOKEN_COMMA));

    consume(parser, TOKEN_RIGHT_BRACKET, "Expected closing ']'");
}

static void dict(Parser *parser, bool canAssign) {
    emitByte(parser, OP_NEW_DICT);

    do {
        if (check(parser, TOKEN_RIGHT_BRACE))
            break;

        expression(parser);
        consume(parser, TOKEN_COLON, "Expected ':'");
        expression(parser);
        emitByte(parser, OP_ADD_DICT);
    } while (match(parser, TOKEN_COMMA));

    consume(parser, TOKEN_RIGHT_BRACE, "Expected closing '}'");
}

static void subscript(Parser *parser, bool canAssign) {
    // slice with no initial index [1, 2, 3][:100]
    if (match(parser, TOKEN_COLON)) {
        emitByte(parser, OP_NIL);
        expression(parser);
        emitByte(parser, OP_SLICE);
        consume(parser, TOKEN_RIGHT_BRACKET, "Expected closing ']'");
        return;
    }

    expression(parser);

    if (match(parser, TOKEN_COLON)) {
        // If we slice with no "ending" push NIL so we know
        // To go to the end of the iterable
        // i.e [1, 2, 3][1:]
        if (check(parser, TOKEN_RIGHT_BRACKET)) {
            emitByte(parser, OP_NIL);
        } else {
            expression(parser);
        }
        emitByte(parser, OP_SLICE);
        consume(parser, TOKEN_RIGHT_BRACKET, "Expected closing ']'");
        return;
    }

    consume(parser, TOKEN_RIGHT_BRACKET, "Expected closing ']'");

    if (canAssign && match(parser, TOKEN_EQUAL)) {
        expression(parser);
        emitByte(parser, OP_SUBSCRIPT_ASSIGN);
    } else if (canAssign && match(parser, TOKEN_PLUS_EQUALS)) {
        expression(parser);
        emitBytes(parser, OP_PUSH, OP_ADD);
        emitByte(parser, OP_SUBSCRIPT_ASSIGN);
    } else if (canAssign && match(parser, TOKEN_MINUS_EQUALS)) {
        expression(parser);
        emitByte(parser, OP_PUSH);
        emitBytes(parser, OP_NEGATE, OP_ADD);
        emitByte(parser, OP_SUBSCRIPT_ASSIGN);
    } else if (canAssign && match(parser, TOKEN_MULTIPLY_EQUALS)) {
        expression(parser);
        emitBytes(parser, OP_PUSH, OP_MULTIPLY);
        emitByte(parser, OP_SUBSCRIPT_ASSIGN);
    } else if (canAssign && match(parser, TOKEN_DIVIDE_EQUALS)) {
        expression(parser);
        emitBytes(parser, OP_PUSH, OP_DIVIDE);
        emitByte(parser, OP_SUBSCRIPT_ASSIGN);
    } else if (canAssign && match(parser, TOKEN_AMPERSAND_EQUALS)) {
        expression(parser);
        emitBytes(parser, OP_PUSH, OP_BITWISE_AND);
        emitByte(parser, OP_SUBSCRIPT_ASSIGN);
    } else if (canAssign && match(parser, TOKEN_CARET_EQUALS)) {
        expression(parser);
        emitBytes(parser, OP_PUSH, OP_BITWISE_XOR);
        emitByte(parser, OP_SUBSCRIPT_ASSIGN);
    } else if (canAssign && match(parser, TOKEN_PIPE_EQUALS)) {
        expression(parser);
        emitBytes(parser, OP_PUSH, OP_BITWISE_OR);
        emitByte(parser, OP_SUBSCRIPT_ASSIGN);
    } else {
        emitByte(parser, OP_SUBSCRIPT);
    }
}

static void namedVariable(Parser *parser, Token name, bool canAssign) {
    uint8_t getOp, setOp;
    int arg = resolveLocal(parser, parser->compiler, &name, false);
    if (arg != -1) {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else if ((arg = resolveUpvalue(parser, parser->compiler, &name)) != -1) {
        getOp = OP_GET_UPVALUE;
        setOp = OP_SET_UPVALUE;
    } else {
        arg = identifierConstant(parser, &name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign && match(parser, TOKEN_EQUAL)) {
        expression(parser);
        emitBytes(parser, setOp, (uint8_t) arg);
    } else if (canAssign && match(parser, TOKEN_PLUS_EQUALS)) {
        namedVariable(parser, name, false);
        expression(parser);
        emitByte(parser, OP_ADD);
        emitBytes(parser, setOp, (uint8_t) arg);
    } else if (canAssign && match(parser, TOKEN_MINUS_EQUALS)) {
        namedVariable(parser, name, false);
        expression(parser);
        emitBytes(parser, OP_NEGATE, OP_ADD);
        emitBytes(parser, setOp, (uint8_t) arg);
    } else if (canAssign && match(parser, TOKEN_MULTIPLY_EQUALS)) {
        namedVariable(parser, name, false);
        expression(parser);
        emitByte(parser, OP_MULTIPLY);
        emitBytes(parser, setOp, (uint8_t) arg);
    } else if (canAssign && match(parser, TOKEN_DIVIDE_EQUALS)) {
        namedVariable(parser, name, false);
        expression(parser);
        emitByte(parser, OP_DIVIDE);
        emitBytes(parser, setOp, (uint8_t) arg);
    } else if (canAssign && match(parser, TOKEN_AMPERSAND_EQUALS)) {
        namedVariable(parser, name, false);
        expression(parser);
        emitByte(parser, OP_BITWISE_AND);
        emitBytes(parser, setOp, (uint8_t) arg);
    } else if (canAssign && match(parser, TOKEN_CARET_EQUALS)) {
        namedVariable(parser, name, false);
        expression(parser);
        emitByte(parser, OP_BITWISE_XOR);
        emitBytes(parser, setOp, (uint8_t) arg);
    } else if (canAssign && match(parser, TOKEN_PIPE_EQUALS)) {
        namedVariable(parser, name, false);
        expression(parser);
        emitByte(parser, OP_BITWISE_OR);
        emitBytes(parser, setOp, (uint8_t) arg);
    }  else {
        emitBytes(parser, getOp, (uint8_t) arg);
    }
}

static void variable(Parser *parser, bool canAssign) {
    namedVariable(parser, parser->previous, canAssign);
}

static Token syntheticToken(const char *text) {
    Token token;
    token.start = text;
    token.length = (int) strlen(text);
    return token;
}

static void pushSuperclass(Parser *parser) {
    if (parser->class == NULL) return;
    namedVariable(parser, syntheticToken("super"), false);
}

static void super_(Parser *parser, bool canAssign) {
    if (parser->class == NULL) {
        error(parser, "Cannot utilise 'super' outside of a class.");
    } else if (!parser->class->hasSuperclass) {
        error(parser, "Cannot utilise 'super' in a class with no superclass.");
    }

    consume(parser, TOKEN_DOT, "Expect '.' after 'super'.");
    consume(parser, TOKEN_IDENTIFIER, "Expect superclass method name.");
    uint8_t name = identifierConstant(parser, &parser->previous);

    // Push the receiver.
    namedVariable(parser, syntheticToken("this"), false);

    if (match(parser, TOKEN_LEFT_PAREN)) {
        int argCount = argumentList(parser);

        pushSuperclass(parser);
        emitBytes(parser, OP_SUPER, argCount);
        emitByte(parser, name);
    } else {
        pushSuperclass(parser);
        emitBytes(parser, OP_GET_SUPER, name);
    }
}

static void this_(Parser *parser, bool canAssign) {
    if (parser->class == NULL) {
        error(parser, "Cannot utilise 'this' outside of a class.");
    } else if (parser->class->staticMethod) {
        error(parser, "Cannot utilise 'this' inside a static method.");
    } else {
        variable(parser, false);
    }
}

static void static_(Parser *parser, bool canAssign) {
    if (parser->class == NULL) {
        error(parser, "Cannot utilise 'static' outside of a class.");
    }
}

static void useStatement(Parser *parser) {
    if (parser->class == NULL) {
        error(parser, "Cannot utilise 'use' outside of a class.");
    }

    do {
        consume(parser, TOKEN_IDENTIFIER, "Expect trait name after use statement.");
        namedVariable(parser, parser->previous, false);
        emitByte(parser, OP_USE);
    } while (match(parser, TOKEN_COMMA));

    consume(parser, TOKEN_SEMICOLON, "Expect ';' after use statement.");
}

static void unary(Parser *parser, bool canAssign) {
    TokenType operatorType = parser->previous.type;

    parsePrecedence(parser, PREC_UNARY);

    switch (operatorType) {
        case TOKEN_BANG:
            emitByte(parser, OP_NOT);
            break;
        case TOKEN_MINUS:
            emitByte(parser, OP_NEGATE);
            break;
        default:
            return;
    }
}

static void prefix(Parser *parser, bool canAssign) {
    TokenType operatorType = parser->previous.type;
    Token cur = parser->current;
    consume(parser, TOKEN_IDENTIFIER, "Expected variable");
    namedVariable(parser, parser->previous, true);

    int arg;
    bool instance = false;

    if (match(parser, TOKEN_DOT)) {
        consume(parser, TOKEN_IDENTIFIER, "Expect property name after '.'.");
        arg = identifierConstant(parser, &parser->previous);
        emitBytes(parser, OP_GET_PROPERTY_NO_POP, arg);
        instance = true;
    }

    switch (operatorType) {
        case TOKEN_PLUS_PLUS: {
            emitByte(parser, OP_INCREMENT);
            break;
        }
        case TOKEN_MINUS_MINUS:
            emitByte(parser, OP_DECREMENT);
            break;
        default:
            return;
    }

    if (instance) {
        emitBytes(parser, OP_SET_PROPERTY, arg);
    } else {
        uint8_t setOp;
        arg = resolveLocal(parser, parser->compiler, &cur, false);
        if (arg != -1) {
            setOp = OP_SET_LOCAL;
        } else if ((arg = resolveUpvalue(parser, parser->compiler, &cur)) != -1) {
            setOp = OP_SET_UPVALUE;
        } else {
            arg = identifierConstant(parser, &cur);
            setOp = OP_SET_GLOBAL;
        }

        emitBytes(parser, setOp, (uint8_t) arg);
    }
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
        {prefix,   NULL,         PREC_NONE},               // TOKEN_PLUS_PLUS
        {prefix,   NULL,         PREC_NONE},               // TOKEN_MINUS_MINUS
        {NULL,     NULL,         PREC_NONE},               // TOKEN_PLUS_EQUALS
        {NULL,     NULL,         PREC_NONE},               // TOKEN_MINUS_EQUALS
        {NULL,     NULL,         PREC_NONE},               // TOKEN_MULTIPLY_EQUALS
        {NULL,     NULL,         PREC_NONE},               // TOKEN_DIVIDE_EQUALS
        {NULL,     NULL,         PREC_NONE},               // TOKEN_SEMICOLON
        {NULL,     NULL,         PREC_NONE},               // TOKEN_COLON
        {NULL,     binary,       PREC_FACTOR},             // TOKEN_SLASH
        {NULL,     binary,       PREC_FACTOR},             // TOKEN_STAR
        {NULL,     binary,       PREC_INDICES},            // TOKEN_STAR_STAR
        {NULL,     binary,       PREC_FACTOR},             // TOKEN_PERCENT
        {NULL,     binary,       PREC_BITWISE_AND},        // TOKEN_AMPERSAND
        {NULL,     binary,       PREC_BITWISE_XOR},        // TOKEN_CARET
        {NULL,     binary,       PREC_BITWISE_OR},         // TOKEN_PIPE
        {NULL,     NULL,         PREC_NONE},               // TOKEN_AMPERSAND_EQUALS
        {NULL,     NULL,         PREC_NONE},               // TOKEN_CARET_EQUALS
        {NULL,     NULL,         PREC_NONE},               // TOKEN_PIPE_EQUALS
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
        {NULL,     NULL,         PREC_NONE},               // TOKEN_TRAIT
        {NULL,     NULL,         PREC_NONE},               // TOKEN_USE
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
        {NULL,     NULL,         PREC_NONE},               // TOKEN_CONTINUE
        {NULL,     NULL,         PREC_NONE},               // TOKEN_WITH
        {NULL,     NULL,         PREC_NONE},               // TOKEN_EOF
        {NULL,     NULL,         PREC_NONE},               // TOKEN_IMPORT
        {NULL,     NULL,         PREC_NONE},               // TOKEN_ERROR
};

static void parsePrecedence(Parser *parser, Precedence precedence) {
    advance(parser);
    ParseFn prefixRule = getRule(parser->previous.type)->prefix;
    if (prefixRule == NULL) {
        error(parser, "Expect expression.");
        return;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(parser, canAssign);

    while (precedence <= getRule(parser->current.type)->precedence) {
        advance(parser);
        ParseFn infixRule = getRule(parser->previous.type)->infix;
        infixRule(parser, canAssign);
    }

    if (canAssign && match(parser, TOKEN_EQUAL)) {
        // If we get here, we didn't parse the "=" even though we could
        // have, so the LHS must not be a valid lvalue.
        error(parser, "Invalid assignment target.");
    }
}

static ParseRule *getRule(TokenType type) {
    return &rules[type];
}

void expression(Parser *parser) {
    parsePrecedence(parser, PREC_ASSIGNMENT);
}

static void block(Parser *parser) {
    while (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF)) {
        declaration(parser);
    }

    consume(parser, TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void function(Parser *parser, FunctionType type) {
    Compiler compiler;
    initCompiler(parser, &compiler, type);
    beginScope(parser);

    // Compile the parameter list.
    consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after function name.");

    if (!check(parser, TOKEN_RIGHT_PAREN)) {
        bool optional = false;
        do {
            uint8_t paramConstant = parseVariable(parser, "Expect parameter name.");
            defineVariable(parser, paramConstant);

            if (match(parser, TOKEN_EQUAL)) {
                parser->compiler->function->arityOptional++;
                optional = true;
                expression(parser);
            } else {
                parser->compiler->function->arity++;

                if (optional) {
                    error(parser, "Cannot have non-optional parameter after optional.");
                }
            }

            if (parser->compiler->function->arity + parser->compiler->function->arityOptional > 255) {
                error(parser, "Cannot have more than 255 parameters.");
            }
        } while (match(parser, TOKEN_COMMA));

        if (parser->compiler->function->arityOptional > 0) {
            emitByte(parser, OP_DEFINE_OPTIONAL);
        }
    }

    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");

    // The body.
    consume(parser, TOKEN_LEFT_BRACE, "Expect '{' before function body.");

    block(parser);

    /**
     * No need to explicitly reduce the scope here as endCompiler does
     * it for us.
     **/
    ObjFunction *function = endCompiler(parser);

    // Capture the upvalues in the new closure object.
    emitBytes(parser, OP_CLOSURE, makeConstant(parser, OBJ_VAL(function)));

    // Emit arguments for each upvalue to know whether to capture a local
    // or an upvalue.
    for (int i = 0; i < function->upvalueCount; i++) {
        emitByte(parser, compiler.upvalues[i].isLocal ? 1 : 0);
        emitByte(parser, compiler.upvalues[i].index);
    }
}

static void method(Parser *parser, bool trait) {
    FunctionType type;

    if (check(parser, TOKEN_STATIC)) {
        type = TYPE_STATIC;
        consume(parser, TOKEN_STATIC, "Expect static.");
        parser->class->staticMethod = true;
    } else {
        type = TYPE_METHOD;
        parser->class->staticMethod = false;
    }

    consume(parser, TOKEN_IDENTIFIER, "Expect method name.");
    uint8_t constant = identifierConstant(parser, &parser->previous);

    // If the method is named "init", it's an initializer.
    if (parser->previous.length == 4 &&
        memcmp(parser->previous.start, "init", 4) == 0) {
        type = TYPE_INITIALIZER;
    }

    function(parser, type);

    if (trait) {
        emitBytes(parser, OP_TRAIT_METHOD, constant);
    } else {
        emitBytes(parser, OP_METHOD, constant);
    }
}

static void classDeclaration(Parser *parser) {
    consume(parser, TOKEN_IDENTIFIER, "Expect class name.");
    uint8_t nameConstant = identifierConstant(parser, &parser->previous);
    declareVariable(parser);

    ClassCompiler classCompiler;
    classCompiler.name = parser->previous;
    classCompiler.hasSuperclass = false;
    classCompiler.enclosing = parser->class;
    classCompiler.staticMethod = false;
    parser->class = &classCompiler;

    if (match(parser, TOKEN_LESS)) {
        consume(parser, TOKEN_IDENTIFIER, "Expect superclass name.");
        classCompiler.hasSuperclass = true;

        beginScope(parser);

        // Store the superclass in a local variable named "super".
        variable(parser, false);
        addLocal(parser, syntheticToken("super"));

        emitBytes(parser, OP_SUBCLASS, nameConstant);
    } else {
        emitBytes(parser, OP_CLASS, nameConstant);
    }

    consume(parser, TOKEN_LEFT_BRACE, "Expect '{' before class body.");

    while (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF)) {
        if (match(parser, TOKEN_USE)) {
            useStatement(parser);
        } else {
            method(parser, false);
        }
    }
    consume(parser, TOKEN_RIGHT_BRACE, "Expect '}' after class body.");

    if (classCompiler.hasSuperclass) {
        endScope(parser);
    }

    defineVariable(parser, nameConstant);

    parser->class = parser->class->enclosing;
}

static void traitDeclaration(Parser *parser) {
    consume(parser, TOKEN_IDENTIFIER, "Expect trait name.");
    uint8_t nameConstant = identifierConstant(parser, &parser->previous);
    declareVariable(parser);

    ClassCompiler classCompiler;
    classCompiler.name = parser->previous;
    classCompiler.hasSuperclass = false;
    classCompiler.enclosing = parser->class;
    classCompiler.staticMethod = false;
    parser->class = &classCompiler;

    emitBytes(parser, OP_TRAIT, nameConstant);

    consume(parser, TOKEN_LEFT_BRACE, "Expect '{' before trait body.");
    while (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF)) {
        method(parser, true);
    }
    consume(parser, TOKEN_RIGHT_BRACE, "Expect '}' after trait body.");

    defineVariable(parser, nameConstant);

    parser->class = parser->class->enclosing;
}

static void funDeclaration(Parser *parser) {
    uint8_t global = parseVariable(parser, "Expect function name.");
    function(parser, TYPE_FUNCTION);
    defineVariable(parser, global);
}

static void varDeclaration(Parser *parser) {
    uint8_t global = parseVariable(parser, "Expect variable name.");

    if (match(parser, TOKEN_EQUAL)) {
        // Compile the initializer.
        expression(parser);
    } else {
        // Default to nil.
        emitByte(parser, OP_NIL);
    }
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    defineVariable(parser, global);
}

static void expressionStatement(Parser *parser) {
    expression(parser);
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after expression.");
    if (parser->vm->repl) {
        emitByte(parser, OP_POP_REPL);
    } else {
        emitByte(parser, OP_POP);
    }
}

/*
static void forstatement(parser) {
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
    beginScope(parser);

    // The initialization clause.
    consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
    int start = 0, start1 = 0, start2 = 0;

    if (match(parser, TOKEN_NUMBER)) {
        //Get number substring
        char *t = strndup(parser->previous.start, parser->previous.length);
        //Convert string to int
        start = strtol(t, NULL, 10);
        consume(parser, TOKEN_SEMICOLON, "Expect ';' after 'number'");
    }

    loopVarDeclaration(start);

    int loopStart = currentChunk(parser)->count;
    int exitJump = -1;

    // The exit condition.
    //match(parser, TOKEN_MINUS);

    if (match(parser, TOKEN_NUMBER)) {
        //Get number substring
        char *t1 = strndup(parser->previous.start, parser->previous.length);
        //Convert string to int
        start1 = strtol(t1, NULL, 10);
        consume(parser, TOKEN_SEMICOLON, "Expect ';' after 'number'");
    }

    //if (match(parser, TOKEN_MINUS)) {
    //    consume(parser, TOKEN_MINUS, "Expect '-' before 'number'");
    //}

    if (match(parser, TOKEN_NUMBER)) {
        //Get number substring
        char *t2 = strndup(parser->previous.start, parser->previous.length);
        start2 = strtod(t2, NULL);
        printf("%d\n", start2);
        printf("%d\n", -10);
    }

    //emitByte(parser, OP_ADD);

    // Compile the body.
    statement(parser);

    // Jump back to the beginning (or the increment).
    //emitLoop(loopStart);

    if (exitJump != -1) {
        printf("%d", loopStart);
        patchJump(parser, exitJump);
        emitByte(parser, OP_POP); // Condition.
    }

    endScope(parser); // Loop variable.
}
 */


static void forStatement(Parser *parser) {
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
    beginScope(parser);

    // The initialization clause.
    consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
    if (match(parser, TOKEN_VAR)) {
        varDeclaration(parser);
    } else if (match(parser, TOKEN_SEMICOLON)) {
        // No initializer.
    } else {
        expressionStatement(parser);
    }

    Loop loop;
    loop.start = currentChunk(parser)->count;
    loop.scopeDepth = parser->compiler->scopeDepth;
    loop.enclosing = parser->loop;
    parser->loop = &loop;

    // The exit condition.
    int exitJump = -1;

    if (!match(parser, TOKEN_SEMICOLON)) {
        expression(parser);
        consume(parser, TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        // Jump out of the loop if the condition is false.
        exitJump = emitJump(parser, OP_JUMP_IF_FALSE);
        emitByte(parser, OP_POP); // Condition.
    }

    // Increment step.
    if (!match(parser, TOKEN_RIGHT_PAREN)) {
        // We don't want to execute the increment before the body, so jump
        // over it.
        int bodyJump = emitJump(parser, OP_JUMP);

        int incrementStart = currentChunk(parser)->count;
        expression(parser);
        emitByte(parser, OP_POP);
        consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(parser, parser->loop->start);
        parser->loop->start = incrementStart;

        patchJump(parser, bodyJump);
    }

    // Compile the body.
    statement(parser);

    // Jump back to the beginning (or the increment).
    emitLoop(parser, parser->loop->start);

    if (exitJump != -1) {
        patchJump(parser, exitJump);
        emitByte(parser, OP_POP); // Condition.
    }

    parser->loop = parser->loop->enclosing;

    endScope(parser); // Loop variable.
}

static void continueStatement(Parser *parser) {
    if (parser->loop == NULL) {
        error(parser, "Cannot utilise 'continue' outside of a loop.");
    }

    consume(parser, TOKEN_SEMICOLON, "Expect ';' after 'continue'.");

    // Discard any locals created inside the loop.
    for (int i = parser->compiler->localCount - 1;
         i >= 0 && parser->compiler->locals[i].depth > parser->loop->scopeDepth;
         i--) {
        emitByte(parser, OP_POP);
    }

    // Jump to top of current innermost loop.
    emitLoop(parser, parser->loop->start);
}

static void ifStatement(Parser *parser) {
    consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression(parser);
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    // Jump to the else branch if the condition is false.
    int elseJump = emitJump(parser, OP_JUMP_IF_FALSE);

    // Compile the then branch.
    emitByte(parser, OP_POP); // Condition.
    statement(parser);

    // Jump over the else branch when the if branch is taken.
    int endJump = emitJump(parser, OP_JUMP);

    // Compile the else branch.
    patchJump(parser, elseJump);
    emitByte(parser, OP_POP); // Condition.

    if (match(parser, TOKEN_ELSE)) statement(parser);

    patchJump(parser, endJump);
}

static void withStatement(Parser *parser) {
    consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'with'.");
    expression(parser);
    consume(parser, TOKEN_COMMA, "Expect comma");
    expression(parser);
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after 'with'.");

    beginScope(parser);

    Local *local = &parser->compiler->locals[parser->compiler->localCount++];
    local->depth = parser->compiler->scopeDepth;
    local->isUpvalue = false;
    local->name = syntheticToken("file");

    emitByte(parser, OP_OPEN_FILE);

    statement(parser);
    endScope(parser);
}

static void returnStatement(Parser *parser) {
    if (parser->compiler->type == TYPE_TOP_LEVEL) {
        error(parser, "Cannot return from top-level code.");
    }

    if (match(parser, TOKEN_SEMICOLON)) {
        emitReturn(parser);
    } else {
        if (parser->compiler->type == TYPE_INITIALIZER) {
            error(parser, "Cannot return a value from an initializer.");
        }

        expression(parser);
        consume(parser, TOKEN_SEMICOLON, "Expect ';' after return value.");
        emitByte(parser, OP_RETURN);
    }
}

static void importStatement(Parser *parser) {
    consume(parser, TOKEN_STRING, "Expect string after import.");
    emitConstant(parser, OBJ_VAL(copyString(
            parser->vm,
            parser->previous.start + 1,
            parser->previous.length - 2))
    );
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after import.");

    emitByte(parser, OP_IMPORT);
}

static void breakStatement(Parser *parser) {
    if (parser->loop == NULL) {
        error(parser, "Cannot utilise 'break' outside of a loop.");
        return;
    }

    consume(parser, TOKEN_SEMICOLON, "Expected semicolon after break");
    emitByte(parser, OP_BREAK);
}

static void whileStatement(Parser *parser) {
    Loop loop;
    loop.start = currentChunk(parser)->count;
    loop.scopeDepth = parser->compiler->scopeDepth;
    loop.enclosing = parser->loop;
    parser->loop = &loop;

    if (check(parser, TOKEN_LEFT_BRACE)) {
        emitByte(parser, OP_TRUE);
    } else {
        consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
        expression(parser);
        consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
    }

    // Jump out of the loop if the condition is false.
    int exitJump = emitJump(parser, OP_JUMP_IF_FALSE);

    // Compile the body.
    emitByte(parser, OP_POP); // Condition.
    statement(parser);

    // Loop back to the start.
    emitLoop(parser, loop.start);

    patchJump(parser, exitJump);
    emitByte(parser, OP_POP); // Condition.

    parser->loop = parser->loop->enclosing;
}

static void synchronize(Parser *parser) {
    parser->panicMode = false;

    while (parser->current.type != TOKEN_EOF) {
        if (parser->previous.type == TOKEN_SEMICOLON) return;

        switch (parser->current.type) {
            case TOKEN_CLASS:
            case TOKEN_TRAIT:
            case TOKEN_DEF:
            case TOKEN_STATIC:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_BREAK:
            case TOKEN_RETURN:
            case TOKEN_IMPORT:
            case TOKEN_WITH:
                return;

            default:
                // Do nothing.
                ;
        }

        advance(parser);
    }
}

static void declaration(Parser *parser) {
    if (match(parser, TOKEN_CLASS)) {
        classDeclaration(parser);
    } else if (match(parser, TOKEN_TRAIT)) {
        traitDeclaration(parser);
    } else if (match(parser, TOKEN_DEF)) {
        funDeclaration(parser);
    } else if (match(parser, TOKEN_VAR)) {
        varDeclaration(parser);
    } else {
        statement(parser);
    }

    if (parser->panicMode) synchronize(parser);
}

static void statement(Parser *parser) {
    if (match(parser, TOKEN_FOR)) {
        forStatement(parser);
    } else if (match(parser, TOKEN_IF)) {
        ifStatement(parser);
    } else if (match(parser, TOKEN_RETURN)) {
        returnStatement(parser);
    } else if (match(parser, TOKEN_WITH)) {
        withStatement(parser);
    } else if (match(parser, TOKEN_IMPORT)){
        importStatement(parser);
    } else if (match(parser, TOKEN_BREAK)) {
        breakStatement(parser);
    } else if (match(parser, TOKEN_WHILE)) {
        whileStatement(parser);
    } else if (match(parser, TOKEN_LEFT_BRACE)) {
        Token previous = parser->previous;
        Token curtok = parser->current;

        // Advance the parser to the next token
        advance(parser);

        if (check(parser, TOKEN_RIGHT_BRACE)) {
            if (check(parser, TOKEN_SEMICOLON)) {
                backTrack();
                backTrack();
                parser->current = previous;
                expressionStatement(parser);
                return;
            }
        }

        if (check(parser, TOKEN_COLON)) {
            for (int i = 0; i < parser->current.length + parser->previous.length; ++i) {
                backTrack();
            }

            parser->current = previous;
            expressionStatement(parser);
            return;
        }

        // Reset the scanner to the previous position
        for (int i = 0; i < parser->current.length; ++i) {
            backTrack();
        }

        // Reset the parser
        parser->previous = previous;
        parser->current = curtok;

        beginScope(parser);
        block(parser);
        endScope(parser);
    }  else if (match(parser, TOKEN_CONTINUE)) {
        continueStatement(parser);
    } else {
        expressionStatement(parser);
    }
}

ObjFunction *compile(VM *vm, const char *source) {
    Parser parser;
    parser.vm = vm;
    parser.compiler = NULL;
    parser.class = NULL;
    parser.loop = NULL;
    parser.hadError = false;
    parser.panicMode = false;

    initScanner(source);
    Compiler mainCompiler;
    initCompiler(&parser, &mainCompiler, TYPE_TOP_LEVEL);

    advance(&parser);

    if (!match(&parser, TOKEN_EOF)) {
        do {
            declaration(&parser);
        } while (!match(&parser, TOKEN_EOF));
    }

    ObjFunction *function = endCompiler(&parser);

    // If there was a compile error, the code is not valid, so don't
    // create a function.
    return parser.hadError ? NULL : function;
}

void grayCompilerRoots(VM *vm) {
    Compiler *compiler = vm->compiler;
    while (compiler != NULL) {
        grayObject(vm, (Obj *) compiler->function);
        compiler = compiler->enclosing;
    }
}
