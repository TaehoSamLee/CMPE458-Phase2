/* parser.h */
#ifndef PARSER_H
#define PARSER_H

#include "tokens.h"

// Basic node types for AST
typedef enum {
    AST_PROGRAM,        // Program node
    AST_VARDECL,        // Variable declaration (int x)
    AST_ASSIGN,         // Assignment (x = 5)
    AST_PRINT,          // Print statement
    AST_NUMBER,         // Number literal
    AST_IDENTIFIER,     // Variable name
    AST_BINOP,          // For binary operations
    AST_IF,             // For if statements
    AST_ELSE,           // For else branch (or attach to AST_IF)
    AST_WHILE,          // For while loops
    AST_REPEAT,         // For repeat-until loops
    AST_BLOCK,          // For block statements
    AST_FUNC_CALL       // For function calls, e.g., factorial
    // TODO: Add more node types as needed
} ASTNodeType;

typedef enum {
    PARSE_ERROR_NONE,
    PARSE_ERROR_UNEXPECTED_TOKEN,
    PARSE_ERROR_MISSING_SEMICOLON,
    PARSE_ERROR_MISSING_IDENTIFIER,
    PARSE_ERROR_MISSING_EQUALS,
    PARSE_ERROR_INVALID_EXPRESSION,
    PARSE_ERROR_MISSING_LPAREN,       // New: Missing left parenthesis
    PARSE_ERROR_MISSING_RPAREN,       // New: Missing right parenthesis
    PARSE_ERROR_MISSING_BLOCK,        // New: Missing '{' or '}'
    PARSE_ERROR_INVALID_OPERATOR,     // New: Unsupported operator usage
    PARSE_ERROR_FUNCTION_CALL_ERROR   // New: Function call issues (e.g., missing arguments)
} ParseError;

// AST Node structure
typedef struct ASTNode {
    ASTNodeType type;           // Type of node
    Token token;               // Token associated with this node
    struct ASTNode* left;      // Left child
    struct ASTNode* right;     // Right child
    // TODO: Add more fields if needed
} ASTNode;

// Parser functions
void parser_init(const char* input);
ASTNode* parse(void);
void print_ast(ASTNode* node, int level);
void free_ast(ASTNode* node);

#endif /* PARSER_H */