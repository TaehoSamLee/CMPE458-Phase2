/* parser.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/lexer/lexer.c"
#include "../../include/parser.h"
#include "../../include/lexer.h"
#include "../../include/tokens.h"

typedef struct Symbol {
    char name[100];
    struct Symbol *next;
} Symbol;

static Symbol *current_scope = NULL;

void push_scope() {
    // Create a new empty scope by pushing a marker onto the stack.
    Symbol *new_scope = malloc(sizeof(Symbol));
    new_scope->name[0] = '\0';  // marker (empty name)
    new_scope->next = current_scope;
    current_scope = new_scope;
}

void pop_scope() {
    if (current_scope) {
        Symbol *old = current_scope;
        current_scope = current_scope->next;
        free(old);
    }
}

void add_symbol(const char *name) {
    Symbol *sym = malloc(sizeof(Symbol));
    strcpy(sym->name, name);
    sym->next = current_scope;
    current_scope = sym;
}

// TODO 1: Add more parsing function declarations for:
// - if statements: if (condition) { ... }
static ASTNode *parse_if_statement(void);
// - while loops: while (condition) { ... }
static ASTNode *parse_while_statement(void);
// - repeat-until: repeat { ... } until (condition)
static ASTNode *parse_repeat_statement(void);
// - print statements: print x;
static ASTNode *parse_print_statement(void);
// - blocks: { statement1; statement2; }
static ASTNode *parse_block(void);
// - factorial function: factorial(x)
static ASTNode *parse_function_call(ASTNode *identifierNode);

static ASTNode *parse_bool_expression(void);
static ASTNode *parse_factor(void);
static ASTNode *parse_term(void);
static ASTNode *parse_expression(void);

static int is_comparison_operator(const char *op) {
    return (strcmp(op, "<") == 0 ||
            strcmp(op, ">") == 0 ||
            strcmp(op, "==") == 0 ||
            strcmp(op, "!=") == 0);
}


// Current token being processed
static Token current_token;
static int position = 0;
static const char *source;


static void parse_error(ParseError error, Token token) {
    // TODO 2: Add more error types for:
    // - Missing parentheses
    // - Missing condition
    // - Missing block braces
    // - Invalid operator
    // - Function call errors

    printf("Parse Error at line %d: ", token.line);
    switch (error) {
        case PARSE_ERROR_UNEXPECTED_TOKEN:
            printf("Unexpected token '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_SEMICOLON:
            printf("Missing semicolon after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_IDENTIFIER:
            printf("Expected identifier after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_EQUALS:
            printf("Expected '=' after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_INVALID_EXPRESSION:
            printf("Invalid expression after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_LPAREN:
            printf("Missing '(' near '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_RPAREN:
            printf("Missing ')' near '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_BLOCK:
            printf("Missing block braces near '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_INVALID_OPERATOR:
            printf("Invalid operator '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_FUNCTION_CALL_ERROR:
            printf("Function call error near '%s'\n", token.lexeme);
            break;
        default:
            printf("Unknown error\n");
    }
}

// Get next token
static void advance(void) {
    current_token = get_next_token(source, &position);
}

// Create a new AST node
static ASTNode *create_node(ASTNodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (node) {
        node->type = type;
        node->token = current_token;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

// Match current token with expected type
static int match(TokenType type) {
    return current_token.type == type;
}

//For error recovery
static void synchronize() {
    // Skip tokens until we reach a likely statement boundary
    while (!match(TOKEN_SEMICOLON) && !match(TOKEN_RBRACE) && current_token.type != TOKEN_EOF) {
        advance();
    }
    // If we ended on a semicolon, advance past it.
    if (match(TOKEN_SEMICOLON)) {
        advance();
    }
}

// Expect a token type or error
static void expect(TokenType type) {
    if (match(type)) {
        advance();
    } else {
        parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
        synchronize();
    }
}

// Forward declarations
static ASTNode *parse_statement(void);

// TODO 3: Add parsing functions for each new statement type
// static ASTNode* parse_if_statement(void) { ... }
static ASTNode *parse_if_statement(void) {
    // Create an AST node for the if statement.
    ASTNode *node = create_node(AST_IF);
    advance();  // consume 'if'
    
    if (!match(TOKEN_LPAREN)) {
        printf("Parse Error at line %d: Expected '(' after 'if', but found '%s'\n", current_token.line, current_token.lexeme); // CHANGED
        synchronize();
        exit(1);
    }
    advance(); // consume '('

    node->left = parse_bool_expression();  
    if (!match(TOKEN_RPAREN)) {
        printf("Parse Error at line %d: Expected ')' after if condition, but found '%s'\n", current_token.line, current_token.lexeme); // CHANGED
        synchronize();
        exit(1);
    }
    advance(); // consume ')'
    
    // Parse the "then" block.
    ASTNode *thenBlock = parse_block();
    
    // Create a node to hold the if statement (with then/else branches).
    node->right = thenBlock;
    
    // Check for an optional else clause.
    if (match(TOKEN_ELSE)) {
        advance(); // consume 'else'
        ASTNode *elseBlock = parse_block();
        thenBlock->right = elseBlock;
    }
    return node;
}

// static ASTNode* parse_while_statement(void) { ... }
static ASTNode *parse_while_statement(void) {
    ASTNode *node = create_node(AST_WHILE);
    advance();  // consume 'while'
    if (!match(TOKEN_LPAREN)) {
        printf("Parse Error at line %d: Expected '(' after 'while', but found '%s'\n", current_token.line, current_token.lexeme); // CHANGED
        synchronize();
        exit(1);
    }
    advance(); // consume '('
    node->left = parse_bool_expression(); // condition
    if (!match(TOKEN_RPAREN)) {
        printf("Parse Error at line %d: Expected ')' after while condition, but found '%s'\n", current_token.line, current_token.lexeme); // CHANGED
        synchronize();
        exit(1);
    }
    advance(); // consume ')'
    node->right = parse_block();     // loop body
    return node;
}

// static ASTNode* parse_repeat_statement(void) { ... }
static ASTNode *parse_repeat_statement(void) {
    ASTNode *node = create_node(AST_REPEAT);
    advance(); // consume 'repeat'
    node->left = parse_block(); // the block of statements
    if (!match(TOKEN_UNTIL)) {
        printf("Parse Error at line %d: Expected 'until' after repeat block, but found '%s'\n", current_token.line, current_token.lexeme);
        exit(1);
    }
    advance(); // consume 'until'
    if (!match(TOKEN_LPAREN)) {
        printf("Parse Error at line %d: Expected '(' after 'until', but found '%s'\n", current_token.line, current_token.lexeme);
        synchronize();
        exit(1);
    }
    advance();
    ASTNode *condition = parse_bool_expression(); // condition after until
    node->right = condition;
    if (!match(TOKEN_RPAREN)) {
        printf("Parse Error at line %d: Expected ')' after repeat condition, but found '%s'\n", current_token.line, current_token.lexeme);
        synchronize();
        exit(1);
    }
    advance();
    if (!match(TOKEN_SEMICOLON)) {
        printf("Parse Error at line %d: Expected ';' after repeat statement, but found '%s'\n", current_token.line, current_token.lexeme);
        exit(1);
    }
    advance(); // consume ';'
    return node;
}

// static ASTNode* parse_print_statement(void) { ... }
static ASTNode *parse_print_statement(void) {
    ASTNode *node = create_node(AST_PRINT);
    advance(); // consume 'print'
    node->left = parse_expression();
    if (!match(TOKEN_SEMICOLON)) {
        printf("Parse Error at line %d: Expected ';' after print statement, but found '%s'\n", current_token.line, current_token.lexeme);
        exit(1);
    }
    advance(); // consume ';'
    return node;
}

// static ASTNode* parse_block(void) { ... }
static ASTNode *parse_block(void) {
    push_scope();
    ASTNode *node = create_node(AST_BLOCK);
    expect(TOKEN_LBRACE);
    ASTNode *firstStmt = NULL;
    ASTNode *currentStmt = NULL;
    while (!match(TOKEN_RBRACE) && !match(TOKEN_EOF)) {
        ASTNode *stmt = parse_statement();
        if (firstStmt == NULL) {
            firstStmt = stmt;
            currentStmt = stmt;
        } else {
            currentStmt->right = stmt;
            currentStmt = stmt;
        }
    }
    expect(TOKEN_RBRACE);
    node->left = firstStmt;
    pop_scope();
    return node;
}

// static ASTNode* parse_factorial(void) { ... }
static ASTNode *parse_function_call(ASTNode *identifierNode) {
    ASTNode *node = create_node(AST_FUNC_CALL);
    node->left = identifierNode;  // function name stored in identifierNode
    expect(TOKEN_LPAREN);
    node->right = parse_expression(); // single argument (extend as needed)
    expect(TOKEN_RPAREN);
    return node;
}

static ASTNode *parse_expression(void);

// Parse variable declaration: int x;
static ASTNode *parse_declaration(void) {
    ASTNode *node = create_node(AST_VARDECL);
    advance(); // consume 'int'

    if (!match(TOKEN_IDENTIFIER)) {
        printf("Parse Error at line %d: Expected identifier after 'int', but found '%s'\n", current_token.line, current_token.lexeme);
        exit(1);
    }
    
    node->token = current_token;
    add_symbol(current_token.lexeme);
    advance();

    if (match(TOKEN_EQUALS)) {
        advance(); // consume '='
        ASTNode *initExpr = parse_expression();
        node->right = initExpr;
    }

    if (!match(TOKEN_SEMICOLON)) {
        printf("Parse Error at line %d: Expected ';' at end of declaration, but found '%s'\n", current_token.line, current_token.lexeme);
        exit(1);
    }
    
    advance();
    return node;
}

static ASTNode *parse_assignment(void) {
    ASTNode *node = create_node(AST_ASSIGN);
    node->left = create_node(AST_IDENTIFIER);
    node->left->token = current_token;
    advance();

    if (!match(TOKEN_EQUALS)) {
        printf("Parse Error at line %d: Expected '=' after identifier in assignment, but found '%s'\n", current_token.line, current_token.lexeme);
        exit(1);
    }
    advance();

    node->right = parse_expression();

    if (!match(TOKEN_SEMICOLON)) {
        printf("Parse Error at line %d: Expected ';' after assignment, but found '%s'\n", current_token.line, current_token.lexeme);
        exit(1);
    }
    advance();
    return node;
}

// Parse statement
static ASTNode *parse_statement(void) {
    if (match(TOKEN_INT)) {
        return parse_declaration();
    } else if (match(TOKEN_IDENTIFIER)) {
        return parse_assignment();
    } else if (match(TOKEN_IF)) {
        return parse_if_statement();
    } else if (match(TOKEN_WHILE)) {
        return parse_while_statement();
    } else if (match(TOKEN_REPEAT)) {
        return parse_repeat_statement();
    } else if (match(TOKEN_PRINT)) {
        return parse_print_statement();
    } else if (match(TOKEN_LBRACE)) {
        return parse_block();
    }

    // TODO 4: Add cases for new statement types
    // else if (match(TOKEN_IF)) return parse_if_statement();
    // else if (match(TOKEN_WHILE)) return parse_while_statement();
    // else if (match(TOKEN_REPEAT)) return parse_repeat_statement();
    // else if (match(TOKEN_PRINT)) return parse_print_statement();
    // ...

    printf("Syntax Error: Unexpected token '%s' at line %d\n", current_token.lexeme, current_token.line);
    exit(1);
}

// Parse expression (currently only handles numbers and identifiers)
// TODO 5: Implement expression parsing
// Current expression parsing is basic. Need to implement:
// - Binary operations (+-*/)
// - Comparison operators (<, >, ==, etc.)
// - Operator precedence
// - Parentheses grouping
// - Function calls

static ASTNode *parse_factor(void) {
    ASTNode *node = NULL;
    if (match(TOKEN_NUMBER)) {
        node = create_node(AST_NUMBER);
        advance();
    } else if (match(TOKEN_IDENTIFIER)) {
        node = create_node(AST_IDENTIFIER);
        advance();
        // Check if this is a function call (e.g., factorial(x))
        if (match(TOKEN_LPAREN)) {
            node = parse_function_call(node);
        }
    } else if (match(TOKEN_LPAREN)) {
        advance();
        node = parse_expression();
        expect(TOKEN_RPAREN);
    } else {
        printf("Parse Error at line %d: Expected number, identifier, or '(' in expression, but found '%s'\n", current_token.line, current_token.lexeme);
        exit(1);
    }
    return node;
}

static ASTNode *parse_term(void) {
    ASTNode *node = parse_factor();
    while (match(TOKEN_OPERATOR) &&
          (current_token.lexeme[0] == '*' || current_token.lexeme[0] == '/')) {
        ASTNode *binOpNode = create_node(AST_BINOP);
        binOpNode->token = current_token;
        advance();
        binOpNode->left = node;
        binOpNode->right = parse_factor();
        node = binOpNode;
    }
    return node;
}

static ASTNode *parse_expression(void) {
    ASTNode *node = parse_term();
    while (match(TOKEN_OPERATOR) &&
          (current_token.lexeme[0] == '+' || current_token.lexeme[0] == '-')) {
        ASTNode *binOpNode = create_node(AST_BINOP);
        binOpNode->token = current_token;
        advance();
        binOpNode->left = node;
        binOpNode->right = parse_term();
        node = binOpNode;
    }
    return node;
}

static ASTNode *parse_bool_expression(void) {
    ASTNode *node = parse_expression();
    while (match(TOKEN_OPERATOR) && is_comparison_operator(current_token.lexeme)) {
        ASTNode *binOpNode = create_node(AST_BINOP);
        binOpNode->token = current_token;
        advance();  // consume comparison operator
        binOpNode->left = node;
        binOpNode->right = parse_expression();
        node = binOpNode;
    }
    return node;
}

// Parse program (multiple statements)
static ASTNode *parse_program(void) {
    ASTNode *program = create_node(AST_PROGRAM);
    ASTNode *current = program;

    while (!match(TOKEN_EOF)) {
        current->left = parse_statement();
        if (!match(TOKEN_EOF)) {
            current->right = create_node(AST_PROGRAM);
            current = current->right;
        }
    }

    return program;
}

// Initialize parser
void parser_init(const char *input) {
    source = input;
    position = 0;
    advance(); // Get first token
}

// Main parse function
ASTNode *parse(void) {
    return parse_program();
}

// Print AST (for debugging)
void print_ast(ASTNode *node, int level) {
    if (!node) return;

    // Indent based on level
    for (int i = 0; i < level; i++) printf("  ");

    // Print node info
    switch (node->type) {
        case AST_PROGRAM:
            printf("Program\n");
            break;
        case AST_VARDECL:
            printf("VarDecl: %s\n", node->token.lexeme);
            break;
        case AST_ASSIGN:
            printf("Assign\n");
            break;
        case AST_NUMBER:
            printf("Number: %s\n", node->token.lexeme);
            break;
        case AST_IDENTIFIER:
            printf("Identifier: %s\n", node->token.lexeme);
            break;

        // TODO 6: Add cases for new node types
        // case AST_IF: printf("If\n"); break;
        case AST_IF:
            printf("If Statement\n");
            break;
        // case AST_WHILE: printf("While\n"); break;
        case AST_WHILE:
            printf("While Loop\n");
            break;
        // case AST_REPEAT: printf("Repeat-Until\n"); break;
        case AST_REPEAT:
            printf("Repeat-Until Loop\n");
            break;
        // case AST_BLOCK: printf("Block\n"); break;
        case AST_BLOCK:
            printf("Block\n");
            break;
        // case AST_BINOP: printf("BinaryOp: %s\n", node->token.lexeme); break;
        case AST_BINOP:
            printf("BinaryOp: %s\n", node->token.lexeme);
            break;
        case AST_PRINT:
            printf("Print Statement\n");
            break;
        case AST_FUNC_CALL:
            printf("Function Call: %s\n", node->left->token.lexeme);
            break;
        default:
            printf("Unknown node type\n");
    }

    // Print children
    print_ast(node->left, level + 1);
    print_ast(node->right, level + 1);
}

// Free AST memory
void free_ast(ASTNode *node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

double evaluate_ast(ASTNode *node) {
    if (!node) return 0;
    switch (node->type) {
        case AST_NUMBER:
            return atof(node->token.lexeme);
        case AST_BINOP: {
            double left = evaluate_ast(node->left);
            double right = evaluate_ast(node->right);
            // Check for division by zero
            if (node->token.lexeme[0] == '/') {
                if (right == 0) {
                    printf("Runtime Error: Division by zero at line %d, column %d\n", node->token.line, node->token.column);
                    exit(1);
                }
                return left / right;
            } else if (node->token.lexeme[0] == '+') {
                return left + right;
            } else if (node->token.lexeme[0] == '-') {
                return left - right;
            } else if (node->token.lexeme[0] == '*') {
                return left * right;
            }
            // For comparison operators, you could return 1 (true) or 0 (false)
            else if (strcmp(node->token.lexeme, "<") == 0) {
                return left < right ? 1 : 0;
            } else if (strcmp(node->token.lexeme, ">") == 0) {
                return left > right ? 1 : 0;
            } else if (strcmp(node->token.lexeme, "==") == 0) {
                return left == right ? 1 : 0;
            } else if (strcmp(node->token.lexeme, "!=") == 0) {
                return left != right ? 1 : 0;
            }
            break;
        }
        default:
            return 0;
    }
    return 0;
}


int main() {
    // Change the filename below to test different inputs.
    //Correct input
    //const char *filename = "./test/input_correct_par.txt";
    //Incorrect input
    const char *filename = "./test/input_incorrect_par_3.txt";
    
    FILE *file = fopen(filename, "r");
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    char *input = malloc(file_size + 1);
    
    fread(input, 1, file_size, file);
    input[file_size] = '\0';
    fclose(file);
    
    printf("Parsing file: %s\n", filename);
    parser_init(input);
    ASTNode *ast = parse();
    
    printf("\nAbstract Syntax Tree:\n");
    print_ast(ast, 0);
    
    free_ast(ast);
    free(input);
    
    return 0;
}


// Main function for testing
// int main() {
//     // Test with both valid and invalid inputs
//     const char *input = "int x;\n" // Valid declaration
//         "x = 42;\n" // Valid assignment;
//         "int z;\n"
//         "int z = 100;\n"
//         "z = z / 2;\n"
//         "if (x > 0) { print x;} else {print 0;}\n"
//         "while (x < 100) {   x = x + 1;}repeat {x = x - 1;} until (x == 0);\n"
//         "print (x + 1);\n";
//     // TODO 8: Add more test cases and read from a file:
//     const char *invalid_input = "int x;\n"
//         "x = 42;\n"
//         "int ;\n"
//         "if (x > ) { print x; } else { print 0; }\n"
//         "while x < 100) { x = x + 1; } repeat { x = x - 1; } until (x == 0\n"
//         "{ int y;\n y = 10;print y;\n";
//     printf("Parsing input:\n%s\n", invalid_input);
//     parser_init(invalid_input);

//     // printf("Parsing input:\n%s\n", input);
//     // parser_init(input);

//     ASTNode *ast = parse();

//     printf("\nAbstract Syntax Tree:\n");
//     print_ast(ast, 0);

//     free_ast(ast);
//     return 0;
// }
