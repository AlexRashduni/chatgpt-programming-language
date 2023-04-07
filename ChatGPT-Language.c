#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_FUNCTION_ARGS 32
#define MAX_TOKENS 1024

// Token types
typedef enum {
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_IDENTIFIER,
    TOKEN_ASSIGN,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_KEYWORD_FUNC,
    TOKEN_KEYWORD_RETURN,
    TOKEN_RETURN,
    TOKEN_EOF,
    TOKEN_LBRACE,
    TOKEN_RBRACE
} token_type_t;

// Token structure
typedef struct {
    token_type_t type;
    char* value;
} token_t;

// Lexer function
void lexer(const char* input, token_t* tokens, int* num_tokens) {
    int i = 0;
    *num_tokens = 0;
    while (input[i] != '\0') {
        if (isdigit(input[i])) {
            int j = i;
            while (isdigit(input[j])) j++;
            if (input[j] == '.') j++;
            while (isdigit(input[j])) j++;
            int len = j - i;
            char* value = (char*)malloc((len + 1) * sizeof(char));
            strncpy(value, input + i, len);
            value[len] = '\0';
            tokens[*num_tokens].type = strchr(value, '.') ? TOKEN_FLOAT : TOKEN_INTEGER;
            tokens[*num_tokens].value = value;
            (*num_tokens)++;
            i = j;
        } else if (isalpha(input[i])) {
            int j = i;
            while (isalnum(input[j])) j++;
            int len = j - i;
            char* value = (char*)malloc((len + 1) * sizeof(char));
            strncpy(value, input + i, len);
            value[len] = '\0';
            if (strcmp(value, "func") == 0) {
                tokens[*num_tokens].type = TOKEN_KEYWORD_FUNC;
            } else if (strcmp(value, "return") == 0) {
                tokens[*num_tokens].type = TOKEN_KEYWORD_RETURN;
            } else {
                tokens[*num_tokens].type = TOKEN_IDENTIFIER;
            }
            tokens[*num_tokens].value = value;
            (*num_tokens)++;
            i = j;
        } else if (input[i] == '+') {
            tokens[*num_tokens].type = TOKEN_PLUS;
            tokens[*num_tokens].value = "+";
            (*num_tokens)++;
            i++;
        } else if (input[i] == '-') {
            tokens[*num_tokens].type = TOKEN_MINUS;
            tokens[*num_tokens].value = "-";
            (*num_tokens)++;
            i++;
        } else if (input[i] == '*') {
            tokens[*num_tokens].type = TOKEN_MUL;
            tokens[*num_tokens].value = "*";
            (*num_tokens)++;
            i++;
        } else if (input[i] == '/') {
            tokens[*num_tokens].type = TOKEN_DIV;
            tokens[*num_tokens].value = "/";
            (*num_tokens)++;
            i++;
        } else if (input[i] == '(') {
            tokens[*num_tokens].type = TOKEN_LPAREN;
            tokens[*num_tokens].value = "(";
            (*num_tokens)++;
            i++;
        } else if (input[i] == ')') {
            tokens[*num_tokens].type = TOKEN_RPAREN;
            tokens[*num_tokens].value = ")";
            (*num_tokens)++;
            i++;
        } else if (input[i] == '=') {
            tokens[*num_tokens].type = TOKEN_ASSIGN;
            tokens[*num_tokens].value = "=";
            (*num_tokens)++;
            i++;
        } else if (input[i] == ',') {
            tokens[*num_tokens].type = TOKEN_COMMA;
            tokens[*num_tokens].value = ",";
            (*num_tokens)++;
            i++;
        } else if (input[i] == ';') {
            tokens[*num_tokens].type = TOKEN_SEMICOLON;
            tokens[*num_tokens].value = ";";
            (*num_tokens)++;
            i++;
        } else if (isspace(input[i])) {
            i++;
        } else {
            fprintf(stderr, "Unexpected character: %c\n", input[i]);
            exit(EXIT_FAILURE);
        }
    }
}

// AST node types
typedef enum {
    NODE_INTEGER,
    NODE_FLOAT,
    NODE_BINARY_OP,
    NODE_ASSIGNMENT,
    NODE_VARIABLE,
    NODE_FUNCTION_CALL,
    NODE_FUNCTION_DEFINITION,
    NODE_RETURN_STATEMENT,
    NODE_BLOCK,
    NODE_PROGRAM
} node_type_t;

// AST node structure
typedef struct node_s {
    node_type_t type;
    union {
        int int_value;
        float float_value;
        struct {
            struct node_s* left;
            struct node_s* right;
            token_type_t op;
        } binary_op;
        struct {
            char* name;
            struct node_s* value;
        } assignment;
        char* variable_name;
        struct {
            char* name;
            struct node_s** arguments;
            int num_arguments;
        } function_call;
        struct {
            char* name;
            char** parameters;
            int num_parameters;
            struct node_s* body;
        } function_definition;
        struct node_s* return_value;
        struct {
            struct node_s** statements;
            int num_statements;
        } block;
        struct {
            struct node_s** statements;
            int num_statements;
        } program;
    } data;
} node_t;

// Parser function
node_t* parse_expression(token_t* tokens, int* current_token);
node_t* parse_primary(token_t* tokens, int* current_token);

node_t* parse_expression(token_t* tokens, int* current_token) {
    node_t* left = parse_primary(tokens, current_token);
    while (tokens[*current_token].type == TOKEN_PLUS ||
    tokens[*current_token].type == TOKEN_MINUS ||
    tokens[*current_token].type == TOKEN_MUL ||
    tokens[*current_token].type == TOKEN_DIV) {
        token_type_t op = tokens[*current_token].type;
        (current_token)++;
        node_t* right = parse_primary(tokens, current_token);
        node_t* new_node = (node_t*)malloc(sizeof(node_t));
        new_node->type = NODE_BINARY_OP;
        new_node->data.binary_op.left = left;
        new_node->data.binary_op.right = right;
        new_node->data.binary_op.op = op;
        left = new_node;
    }
    return left;
}

node_t* parse_primary(token_t* tokens, int* current_token) {
    node_t* new_node = NULL;
    if (tokens[*current_token].type == TOKEN_INTEGER) {
        new_node = (node_t*)malloc(sizeof(node_t));
        new_node->type = NODE_INTEGER;
        new_node->data.int_value = atoi(tokens[*current_token].value);
        (*current_token)++;
    } else if (tokens[*current_token].type == TOKEN_FLOAT) {
        new_node = (node_t*)malloc(sizeof(node_t));
        new_node->type = NODE_FLOAT;
        new_node->data.float_value = atof(tokens[*current_token].value);
        (*current_token)++;
    } else if (tokens[*current_token].type == TOKEN_IDENTIFIER &&
            tokens[*current_token + 1].type == TOKEN_LPAREN) {
        char* name = tokens[*current_token].value;
        (*current_token) += 2; // skip identifier and LPAREN tokens
        node_t** arguments = NULL;
        int num_arguments = 0;
        if (tokens[*current_token].type != TOKEN_RPAREN) {
            arguments = (node_t**)malloc(sizeof(node_t*) * MAX_FUNCTION_ARGS);
            arguments[num_arguments] = parse_expression(tokens, current_token);
            num_arguments++;
            while (tokens[*current_token].type == TOKEN_COMMA) {
                (*current_token)++;
                arguments[num_arguments] = parse_expression(tokens, current_token);
                num_arguments++;
            }
        }
        if (tokens[*current_token].type != TOKEN_RPAREN) {
            fprintf(stderr, "Expected ')' after function call arguments\n");
            exit(EXIT_FAILURE);
        }
        (*current_token)++; // skip RPAREN token
        new_node = (node_t*)malloc(sizeof(node_t));
        new_node->type = NODE_FUNCTION_CALL;
        new_node->data.function_call.name = name;
        new_node->data.function_call.arguments = arguments;
        new_node->data.function_call.num_arguments = num_arguments;
    } else if (tokens[*current_token].type == TOKEN_IDENTIFIER) {
        char* name = tokens[*current_token].value;
        (*current_token)++;
        if (tokens[*current_token].type == TOKEN_ASSIGN) {
            (*current_token)++;
            node_t* value = parse_expression(tokens, current_token);
            new_node = (node_t*)malloc(sizeof(node_t));
            new_node->type = NODE_ASSIGNMENT;
            new_node->data.assignment.name = name;
            new_node->data.assignment.value = value;
        } else {
            new_node = (node_t*)malloc(sizeof(node_t));
            new_node->type = NODE_VARIABLE;
            new_node->data.variable_name = name;
        }
    } else if (tokens[*current_token].type == TOKEN_LPAREN) {
        (*current_token)++;
        new_node = parse_expression(tokens, current_token);
        if (tokens[*current_token].type != TOKEN_RPAREN) {
            fprintf(stderr, "Expected ')' after expression\n");
            exit(EXIT_FAILURE);
        }
        (*current_token)++;
    } else {
        fprintf(stderr, "Unexpected token: %s\n", tokens[*current_token].value);
        exit(EXIT_FAILURE);
    }
    return new_node;
}

node_t* parse_statement(token_t* tokens, int* current_token) {
    if (tokens[*current_token].type == TOKEN_IDENTIFIER &&
        tokens[*current_token + 1].type == TOKEN_LPAREN) {
        return parse_expression(tokens, current_token);
    } else if (tokens[*current_token].type == TOKEN_RETURN) {
        (current_token)++;
        node_t* value = parse_expression(tokens, current_token);
        node_t* new_node = (node_t*)malloc(sizeof(node_t));
        new_node->type = NODE_RETURN_STATEMENT;
        new_node->data.return_value = value;
        return new_node;
    } else {
        fprintf(stderr, "Unexpected token: %s\n", tokens[*current_token].value);
        exit(EXIT_FAILURE);
    }
}

node_t* parse_block(token_t* tokens, int* current_token) {
    if (tokens[*current_token].type != TOKEN_LBRACE) {
        fprintf(stderr, "Expected '{' to start block\n");
        exit(EXIT_FAILURE);
    }
    (current_token)++; // skip LBRACE token
    node_t** statements = NULL;
    int num_statements = 0;
    while (tokens[*current_token].type != TOKEN_RBRACE) {
        node_t* statement = parse_statement(tokens, current_token);
        num_statements++;
        statements = (node_t**)realloc(statements, sizeof(node_t*) * num_statements);
        statements[num_statements - 1] = statement;
    }
    (*current_token)++; // skip RBRACE token
    node_t* new_node = (node_t*)malloc(sizeof(node_t));
    new_node->type = NODE_BLOCK;
    new_node->data.block.statements = statements;
    new_node->data.block.num_statements = num_statements;
    return new_node;
}

node_t* parse_function_definition(token_t* tokens, int* current_token) {
    (*current_token)++; // skip "def" token
    char* name = tokens[*current_token].value;(*current_token)++; // skip function name token
    if (tokens[*current_token].type != TOKEN_LPAREN) {
        fprintf(stderr, "Expected '(' after function name\n");
        exit(EXIT_FAILURE);
    }
    (*current_token)++; // skip LPAREN token
    char** parameters = NULL;
    int num_parameters = 0;
    if (tokens[*current_token].type != TOKEN_RPAREN) {
        parameters = (char**)malloc(sizeof(char*) * MAX_FUNCTION_ARGS);
        parameters[num_parameters] = tokens[*current_token].value;
        num_parameters++;
        (*current_token)++;
        while (tokens[*current_token].type == TOKEN_COMMA) {
            (*current_token)++;
            parameters[num_parameters] = tokens[*current_token].value;
            num_parameters++;
            (*current_token)++;
        }
    }
    if (tokens[*current_token].type != TOKEN_RPAREN) {
        fprintf(stderr, "Expected ')' after function parameters\n");
        exit(EXIT_FAILURE);
    }
    (*current_token)++; // skip RPAREN token
    node_t* body = parse_block(tokens, current_token);
    node_t* new_node = (node_t*)malloc(sizeof(node_t));
    new_node->type = NODE_FUNCTION_DEFINITION;
    new_node->data.function_definition.name = name;
    new_node->data.function_definition.parameters = parameters;
    new_node->data.function_definition.num_parameters = num_parameters;
    new_node->data.function_definition.body = body;
    return new_node;
}

node_t* parse_program(token_t* tokens) {
    int current_token = 0;
    node_t** statements = NULL;
    int num_statements = 0;
    while (tokens[current_token].type != TOKEN_EOF) {
        node_t* statement = parse_statement(tokens, &current_token);
        num_statements++;
        statements = (node_t**)realloc(statements, sizeof(node_t*) * num_statements);
        statements[num_statements - 1] = statement;
    }
    node_t* new_node = (node_t*)malloc(sizeof(node_t));
    new_node->type = NODE_PROGRAM;
    new_node->data.program.statements = statements;
    new_node->data.program.num_statements = num_statements;
    return new_node;
}



typedef struct {
    int num_bindings;
    int capacity;
    char** variable_names;
    node_t** values;
} environment_t;

environment_t* create_environment() {
    environment_t* env = (environment_t*)malloc(sizeof(environment_t));
    env->num_bindings = 0;
    env->capacity = 10;
    env->variable_names = (char**)malloc(env->capacity * sizeof(char*));
    env->values = (node_t**)malloc(env->capacity * sizeof(node_t*));
    return env;
}

node_t** hashtable_lookup(environment_t* env, char* key) {
    unsigned int hash = hash_function(key) % env->capacity;
    char** keys = env->variable_names;
    node_t*** values = env->values;

    for (int i = 0; i < env->capacity; i++) {
        unsigned int index = (hash + i) % env->capacity;
        if (keys[index] == NULL) {
            return NULL;
        }
        if (strcmp(keys[index], key) == 0) {
            return values[index];
        }
    }
    return NULL;
}

void hashtable_insert(environment_t* env, char* key, node_t** value) {
    char** keys = env->variable_names;
    unsigned int hash = hash_function(key) % env->capacity;

    for (int i = 0; i < env->capacity; i++) {
        unsigned int index = (hash + i) % env->capacity;
        if (keys[index] == NULL) {
            keys[index] = strdup(key);
            keys[index] = value;
            return;
        }
        if (strcmp(keys[index], key) == 0) {
            keys[index] = value;
            return;
        }
    }
    fprintf(stderr, "Hashtable full, cannot insert key-value pair.\n");
    exit(EXIT_FAILURE);
}

void environment_bind(environment_t* env, char* var_name, node_t* value) {
    // Check if the variable already exists in the environment
    node_t** p_value = hashtable_lookup(env->variable_names, var_name);

    if (p_value != NULL) {
        // If the variable already exists, update its value
        *p_value = value;
    } else {
        // If the variable doesn't exist, add it to the environment
        node_t** new_value = malloc(sizeof(node_t*));
        *new_value = value;
        hashtable_insert(env->variable_names, var_name, new_value);
    }
}

void destroy_environment(environment_t* env) {
    for (int i = 0; i < env->num_bindings; i++) {
        free(env->variable_names[i]);
        destroy_node(env->values[i]);
    }
    free(env->variable_names);
    free(env->values);
    free(env);
}



int execute_binary_op(node_t* node) {
    int left = execute_node(node->data.binary_op.left);
    int right = execute_node(node->data.binary_op.right);

    switch (node->data.binary_op.op) {
        case TOKEN_PLUS:
            return left + right;
        case TOKEN_MINUS:
            return left - right;
        case TOKEN_MUL:
            return left * right;
        case TOKEN_DIV:
            return left / right;
        default:
            fprintf(stderr, "Error: Invalid binary operator");
            return 0;
    }
}

node_t* execute_function_call(node_t* node, environment_t* env) {
    // Find the function definition in the environment
    node_t* func_def = environment_lookup(env, node->data.function_call.name);

    // Create a new environment for the function call
    environment_t* func_env = create_environment();

    // Bind the function arguments to their values in the new environment
    for (int i = 0; i < node->data.function_call.num_arguments; i++) {
        char* var_name = func_def->data.function_definition.parameters[i];
        node_t* var_value = execute_node(node->data.function_call.arguments[i], env);
        environment_bind(func_env, var_name, var_value);
    }

    // Execute the function body in the new environment
    node_t* result = execute_node(func_def->data.function_definition.body, func_env);

    // Destroy the function environment
    destroy_environment(func_env);

    return result;
}

void execute_assignment(node_t* node, environment_t* env) {
    // Evaluate the value expression
    node_t* value = execute_node(node->data.assignment.value, env);

    // Bind the variable to the value in the environment
    environment_bind(env, node->data.assignment.name, value);
}




void interpret(node_t* node) {
    environment_t* env = create_environment();
    switch (node->type) {
        case NODE_INTEGER:
        case NODE_FLOAT:
        case NODE_VARIABLE:
        case NODE_RETURN_STATEMENT:
            // These types of nodes have no effect on the program's execution
            break;
        case NODE_BINARY_OP:
            execute_binary_op(node);
            break;
        case NODE_ASSIGNMENT:
            execute_assignment(node, env);
            break;
        case NODE_FUNCTION_DEFINITION:
            // Function definitions have no effect on the program's execution
            break;
        case NODE_FUNCTION_CALL:
            execute_function_call(node, env);
            break;
        case NODE_BLOCK:
        case NODE_PROGRAM:
            // Execute each statement in the block or program
            for (int i = 0; i < node->data.block.num_statements; i++) {
                interpret(node->data.block.statements[i]);
            }
            break;
        default:
            fprintf(stderr, "Error: Unknown node type %d\n", node->type);
            exit(EXIT_FAILURE);
    }
    destroy_env(env);
}

int main() {
    char* input = "def add(a, b) { return a + b; }";
    int num_tokens = 0;
    token_t tokens[MAX_TOKENS];
    lexer(input, tokens, &num_tokens);
    node_t* program = parse_program(tokens);
    interpret(program);
    return 0;
}
