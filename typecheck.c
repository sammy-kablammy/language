// There is a notion of type *equality* (these types are literally the same) and
// type *castability* (for example if i do "myint: type = int" i should then be
// able to do "x: int = (int)some_myint" but not "x: int = some_myint"
// (casting syntax not finalized)

// The convention in this file is that typecheck_*_expression functions will
// return the type that they got. Unless they find a type error.

// Symbol table maps identifiers to types.
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
char *builtin_types[] = {
    "int",
    "bool",
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
        // TODO(sam) likely ban same-scope shadowing but what about child<->parent scope shadowing?
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

void typecheck_entire_program(statement_node *first_statement) {
    symbol_table global_symbols = symbol_table_alloc();

    data_type next_available_type_id = 1; // Start at 1 as 0 represents TYPE_INVALID

    for (data_type i = 0; i < NUM_BUILTIN_TYPES; i++) {
        string builtin_type = string_from_cstring(builtin_types[i]);
        symbol_table_insert(&global_symbols, builtin_type, i + 1);
        next_available_type_id += 1;
    }

    statement_node *current_statement = first_statement;
    while (current_statement != NULL) {
        typecheck_statement(&global_symbols, current_statement);
        current_statement = current_statement->next;
    }

    hash_table_print(global_symbols.table);

    symbol_table_free(global_symbols);
}
