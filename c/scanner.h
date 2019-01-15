//> Scanning on Demand scanner-h
#ifndef dictu_scanner_h
#define dictu_scanner_h
//> token-type

typedef enum {
    // Single-character tokens.
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET, TOKEN_RIGHT_BRACKET,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_INCREMENT, TOKEN_DECREMENT,
    TOKEN_PLUS_EQUALS,
    TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,
    TOKEN_PERCENT,

    // One or two character tokens.
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,

    // Literals.
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

    // Keywords.
    TOKEN_CLASS, TOKEN_STATIC, TOKEN_THIS, TOKEN_SUPER, TOKEN_DEF,
    TOKEN_IF, TOKEN_AND, TOKEN_ELSE, TOKEN_OR,
    TOKEN_VAR, TOKEN_TRUE, TOKEN_FALSE, TOKEN_NIL,
    TOKEN_FOR, TOKEN_WHILE, TOKEN_BREAK,
    TOKEN_RETURN,
    TOKEN_WITH, TOKEN_EOF,
    TOKEN_ERROR

} TokenType;
//< token-type
//> token-struct

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;
//< token-struct

void initScanner(const char* source);
//> scan-token-h
Token scanToken();
//< scan-token-h

#endif
