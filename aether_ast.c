#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TOKEN_PRINT,
    TOKEN_INT,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_IF,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMICOLON,
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef enum {
    AST_PRINT,
    AST_INT_DECL,
    AST_EXPR,
    AST_BLOCK,
    AST_IF
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char *value;                // e.g. literal text or variable name
    struct ASTNode **children;  // array of child nodes
    int child_count;
} ASTNode;

/* create_ast_node
   Creates a new AST node with the given type and value.
*/
ASTNode* create_ast_node(ASTNodeType type, const char *value) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = type;
    node->value = value ? strdup(value) : NULL;
    node->children = NULL;
    node->child_count = 0;
    return node;
}

/* add_child
   Adds a child node to the parent AST node.
*/
void add_child(ASTNode *parent, ASTNode *child) {
    parent->child_count++;
    parent->children = (ASTNode **)realloc(parent->children, parent->child_count * sizeof(ASTNode *));
    parent->children[parent->child_count - 1] = child;
}

/* print_ast
   Recursively prints the AST structure with indentation.
*/
void print_ast(ASTNode *node, int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
    switch (node->type) {
        case AST_PRINT:
            printf("PRINT: %s\n", node->value);
            break;
        case AST_INT_DECL:
            printf("INT_DECL: %s\n", node->value);
            break;
        case AST_IF:
            printf("IF STATEMENT\n");
            break;
        case AST_BLOCK:
            printf("BLOCK\n");
            break;
        case AST_EXPR:
            printf("EXPR: %s\n", node->value ? node->value : "");
            break;
        default:
            printf("AST_NODE\n");
            break;
    }
    for (int i = 0; i < node->child_count; i++) {
        print_ast(node->children[i], indent + 1);
    }
}

int main() {

    
    ASTNode *root = create_ast_node(AST_BLOCK, NULL);
    
    ASTNode *intDecl = create_ast_node(AST_INT_DECL, "x = 5");
    add_child(root, intDecl);
    
    ASTNode *printNode = create_ast_node(AST_PRINT, "Hello, world!");
    add_child(root, printNode);
    
    ASTNode *ifNode = create_ast_node(AST_IF, "x > 0");
    ASTNode *ifBlock = create_ast_node(AST_BLOCK, NULL);
    ASTNode *ifPrint = create_ast_node(AST_PRINT, "Positive");
    add_child(ifBlock, ifPrint);
    add_child(ifNode, ifBlock);
    add_child(root, ifNode);
    
    print_ast(root, 0);
    
    return 0;
}
