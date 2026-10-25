// Symbol table maps identifiers to types for a given scope
typedef struct symbol_table {
    struct symbol_table *parent;
    hash_table table;
} symbol_table;

symbol_table symbol_table_alloc() {
    return (symbol_table){
        .parent = NULL,
        .table = hash_table_alloc(),
    };
}

void symbol_table_free(symbol_table symbols) {
    hash_table_free(symbols.table);
}

typedef uint32_t data_type;
const data_type TYPE_INVALID = 0;
string builtin_types[] = {
    // I don't like this but eh
    { .buf = "int", .length = sizeof("int") - 1 },
    { .buf = "bool", .length = sizeof("bool") - 1 },
};
const uint32_t NUM_BUILTIN_TYPES = sizeof(builtin_types) / sizeof(builtin_types[0]);
// User-defined types will counting after all the builtins

data_type symbol_table_lookup(symbol_table *symbols, string identifier) {
    while (symbols != NULL) {
        data_type out;
        if (hash_table_lookup(&symbols->table, identifier, &out)) {
            return out;
        }
        symbols = symbols->parent;
    }
    return TYPE_INVALID;
}

void symbol_table_insert(symbol_table *symbols, string identifier, data_type type) {
    hash_table_insert(&symbols->table, identifier, type);
}

data_type typecheck_expression(symbol_table *symbols, expression_node *expression);

// TODO: ensure operator matches types. for example '123 * true' should cause a type error
data_type typecheck_infix_expression(symbol_table *symbols, infix_node infix) {
    data_type lhs_type = typecheck_expression(symbols, infix.left);
    data_type rhs_type = typecheck_expression(symbols, infix.right);
    if (lhs_type != rhs_type) {
        panic("infix expression lhs type (%d) doesn't match rhs type (%d)", lhs_type, rhs_type);
    }
    return lhs_type;
}

data_type typecheck_expression(symbol_table *symbols, expression_node *expression) {
    data_type type;
    switch (expression->kind) {
        case INT:
            return symbol_table_lookup(symbols, string_from_cstring("int"));
        case BOOL:
            return symbol_table_lookup(symbols, string_from_cstring("bool"));
        case IDENTIFIER:
            type = symbol_table_lookup(symbols, expression->value.identifier);
            if (type != TYPE_INVALID) {
                return type;
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
    data_type lhs_type = symbol_table_lookup(symbols, declaration.identifier);
    if (lhs_type != TYPE_INVALID) {
        // TODO consider child<->parent scope shadowing (same-scope shadowing likely banned)
        panic("shadowing is not supported (at least not currently)");
    }

    data_type desired_type = symbol_table_lookup(symbols, declaration.data_type);
    if (desired_type == TYPE_INVALID) {
        print_string(declaration.data_type);
        panic("attempted to use unknown type in declaration");
    }
    data_type expression_type = typecheck_expression(symbols, declaration.expression);
    if (desired_type != expression_type) {
        panic("desired type (%d) does not match expression type (%d)", desired_type, expression_type);
    }

    symbol_table_insert(symbols, declaration.identifier, desired_type);
}

void typecheck_assignment(symbol_table *symbols, assignment_node assignment) {
    data_type lhs_type = symbol_table_lookup(symbols, assignment.lhs);
    if (lhs_type == TYPE_INVALID) {
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

void typecheck_entire_program(symbol_table *global_symbols, statement_node *first_statement) {
    // Start counting types at 1, since 0 represents TYPE_INVALID
    data_type next_available_type_id = 1;

    for (uint32_t i = 0; i < NUM_BUILTIN_TYPES; i++) {
        symbol_table_insert(global_symbols, builtin_types[i], i + 1);
        next_available_type_id += 1;
    }

    statement_node *current_statement = first_statement;
    while (current_statement != NULL) {
        typecheck_statement(global_symbols, current_statement);
        current_statement = current_statement->next;
    }
}

void print_symbol_table(symbol_table *symbols) {
    hash_table_print(symbols->table);
}
