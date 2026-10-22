// There is a notion of type *equality* (these types are literally the same) and
// type *castability* (for example if i do "myint: type = int" i should then be
// able to do "x: int = (int)some_myint" but not "x: int = some_myint"
// (casting syntax not finalized)

// Symbol table maps identifiers to types.
typedef struct symbol_table {
    struct symbol_table *parent;
    hash_table table;
} symbol_table;

symbol_table symbol_table_alloc() {
    return (symbol_table){
        .parent = NULL,
        // 💢grrrrr what is this, RAII? find a way to not do this
        .table = hash_table_alloc(),
    };
}

void symbol_table_free(symbol_table symbols) {
    hash_table_free(symbols.table);
}

typedef uint32_t data_type;
// NOTE C99 apparently doesn't let you choose the size of an enum
typedef enum {
    // TODO(sam) meaningful zero value here?
    TYPE_INVALID = 0,

    // Primitive / builtin types go here
    TYPE_INT = 1,
    TYPE_BOOL,
    // TYPE_FLOAT,
    // TYPE_STRING,
    // ...

    // User defined data types start at the end and go up
    USER_DEFINED_TYPE_START,
} builtin_types;
const uint32_t FIRST_BUILTIN_TYPE = 1;

// keep this list in sync
char * builtin_type_strings[] = {
    "<invalidtype>",
    "int",
    "bool",
};

// Still unsure whether to use a bool + outparam, or to only use a data_type
// with a meaningful zero value
bool symbol_table_lookup(symbol_table *symbols, string identifier, data_type *out) {
    while (symbols != NULL) {
        if (hash_table_lookup(&symbols->table, identifier, out)) {
            return true;
        }
        symbols = symbols->parent;
    }
    return false;
}

void symbol_table_insert(symbol_table *symbols, string identifier, data_type type) {
    printf("symbol table insert: '");
    print_string(identifier);
    printf("' data type id=%d\n", type);
    hash_table_insert(&symbols->table, identifier, type);
}

// The convention in this file is that typecheck_*_expression functions will
// return the type that they got. Unless they find a type error.

data_type typecheck_expression(symbol_table *symbols, expression_node *expression);

data_type typecheck_infix_expression(symbol_table *symbols, infix_node infix) {
    data_type lhs_type = typecheck_expression(symbols, infix.left);
    data_type rhs_type = typecheck_expression(symbols, infix.right);
    if (lhs_type != rhs_type) {
        panic("infix expression lhs type (%d) doesn't match rhs type (%d)", lhs_type, rhs_type);
    }
    return lhs_type;
}

data_type typecheck_expression(symbol_table *symbols, expression_node *expression) {
    data_type out;
    switch (expression->kind) {
        case INT:
            return TYPE_INT;
        case BOOL:
            return TYPE_BOOL;
        case IDENTIFIER:
            if (symbol_table_lookup(symbols, expression->value.identifier, &out)) {
                return out;
            } else {
                printf("Could not find type of (\"");
                printf("\") in scope");
                print_string(expression->value.identifier);
                panic("Could not find identifier");
            }
            break;
        case INFIX:
            return typecheck_infix_expression(symbols, expression->value.infix);
        default:
            panic("unhandled expression kind in typecheck_expression (%d)", expression->kind);
    }
}

void typecheck_declaration(symbol_table *symbols, declaration_node declaration) {
    data_type lhs_type;
    if (symbol_table_lookup(symbols, declaration.lhs, &lhs_type)) {
        // TODO(sam) likely ban same-scope shadowing but what about child<->parent scope shadowing?
        panic("shadowing is not supported (at least not currently)");
    }

    data_type desired_type;
    if (!symbol_table_lookup(symbols, declaration.data_type, &desired_type)) {
        print_string(declaration.data_type);
        panic("attempted to use unknown type in declaration");
    }
    data_type expression_type = typecheck_expression(symbols, declaration.expression);
    if (desired_type != expression_type) {
        panic("desired type (%d) does not match expression type (%d)", desired_type, expression_type);
    }

    symbol_table_insert(symbols, declaration.lhs, desired_type);
}

void typecheck_assignment(symbol_table *symbols, assignment_node assignment) {

    // TODO(sam) decide whether to do lookup(&data_type)->bool or drop the
    // outparam and return a zero-valued DATA_TYPE_INVALID instead
    data_type lhs_type;
    if (!symbol_table_lookup(symbols, assignment.lhs, &lhs_type)) {
        printf("\nlhs of identifier (\"");
        print_string(assignment.lhs);
        printf("\") not found in current or parent scope\n");
        panic("assignment typecheck failed");
    }

    data_type rhs_type = typecheck_expression(symbols, assignment.expression);
    if (lhs_type != rhs_type) {
        panic("Assignment's RHS type does not match LHS type");
    }
}

void typecheck_statement(symbol_table *symbols, statement_node *statement) {
    switch (statement->kind) {
        case DECLARATION:
            typecheck_declaration(symbols, statement->value.declaration);
            break;
        case ASSIGNMENT:
            typecheck_assignment(symbols, statement->value.assignment);
            break;
        default:
            panic("typecheck_statement: unhandled statement kind (%d)", statement->kind);
    }
}

void typecheck_entire_program(statement_node *first_statement) {
    symbol_table global_symbols = symbol_table_alloc();

    for (uint32_t builtin_type = FIRST_BUILTIN_TYPE; builtin_type < USER_DEFINED_TYPE_START; builtin_type++) {
        string builtin_type_string = string_from_cstring(builtin_type_strings[builtin_type]);
        symbol_table_insert(&global_symbols, builtin_type_string, builtin_type);

        // data_type unused;
        // bool res = symbol_table_lookup(&global_symbols, builtin_type_string, &unused);
        // printf("res=%d\n", res);
    }

    // first_statement = first_statement;

    statement_node *current_statement = first_statement;
    while (current_statement != NULL) {
        typecheck_statement(&global_symbols, current_statement);
        current_statement = current_statement->next;
    }

}
