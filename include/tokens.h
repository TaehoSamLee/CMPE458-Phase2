/* tokens.h */
#ifndef TOKENS_H
#define TOKENS_H

typedef enum {
    TOKEN_EOF,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_IDENTIFIER,
    TOKEN_EQUALS,
    TOKEN_SEMICOLON,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_IF,
    TOKEN_ELSE,      // Added for if-else
    TOKEN_INT,
    TOKEN_PRINT,
    TOKEN_WHILE,     // Added for while loops
    TOKEN_REPEAT,    // Added for repeat-until loops
    TOKEN_UNTIL,     // Added for repeat-until loops
    TOKEN_READ,      // (optional) for input operations
    TOKEN_ERROR
} TokenType;

typedef enum {
    ERROR_NONE,
    ERROR_INVALID_CHAR,
    ERROR_INVALID_NUMBER,
    ERROR_CONSECUTIVE_OPERATORS,
    ERROR_INVALID_IDENTIFIER,
    ERROR_UNEXPECTED_TOKEN
} ErrorType;

typedef struct {
    TokenType type;
    char lexeme[100];   // Actual text of the token
    int line;           // Line number in source file
    int column;
    ErrorType error;    // Error type if any
} Token;

#endif /* TOKENS_H */