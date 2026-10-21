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

typedef uint32_t data_type;
// NOTE C99 apparently doesn't let you choose the size of an enum
enum {
    // TODO(sam) meaningful zero value here?
    // TYPE_INVALID = 0,

    // Primitive / builtin types go here
    TYPE_INT,
    TYPE_BOOL,
    // TYPE_FLOAT,
    // TYPE_STRING,
    // ...

    // User defined data types start at the end and go up
    USER_DEFINED_TYPE_START,
};

// Still unsure whether to use a bool + outparam, or to only use a data_type
// with a meaningful zero value
bool symbol_table_lookup(symbol_table *symbols, string identifier, data_type *out) {
    while (symbols != NULL) {
        if (hash_table_lookup(symbols->, identifier, out)) {
            return true;
        }
        symbols = symbols->parent;
    }
    return false;
}

data_type symbol_table_insert(symbol_table *symbols, string identifier, data_type type) {
    hash_table_insert(symbols, identifier, type);
}

// The convention in this file is that typecheck_*_expression functions will
// return the type that they got. Unless they find a type error.

data_type typecheck_infix_expression(infix_node infix) {
    data_type lhs_type = typecheck_expression(infix.left);
    data_type rhs_type = typecheck_expression(infix.right);
    if (lhs_type != rhs_type) {
        type_error();
    }
    return lhs_type;
}

data_type typecheck_expression(expression_node *expression) {
    switch (expression->kind) {
        case INFIX:
            return typecheck_infix_expression(expression->value.infix);
        default:
            panic("unhandled expression kind in typecheck_expression");
    }
}

data_type typecheck_declaration(symbol_table *symbols, declaration_node declaration) {
    data_type lhs_type;
    if (!symbol_table_lookup(symbols, declaration.lhs, &lhs_type)) {
        // TODO(sam) likely ban same-scope shadowing but what about child<->parent scope shadowing?
        type_error("shadowing not supported (at least not currently)");
    }

    data_type expression_type = typecheck_expression(declaration.expression);
    if (declaration.data_type != expression_type) {
        type_error("stated type does not match expression type");
    }

    symbol_table_insert(symbols, declaration.lhs, declaration.data_type);

    return declaration.data_type;
}

data_type typecheck_assignment(symbol_table *symbols, assignment_node assignment) {

    // TODO(sam) decide whether to do lookup(&data_type)->bool or drop the
    // outparam and return a zero-valued DATA_TYPE_INVALID instead
    data_type lhs_type;
    if (!symbol_table_lookup(symbols, assignment.lhs, &lhs_type)) {
        type_error("Not found");
    }

    data_type rhs_type = typecheck_expression(assignment.expression);
    if (lhs_type != rhs_type) {
        panic("Assignment's RHS type does not match LHS type");
    }

    return lhs_type;
}

data_type typecheck_statement(symbol_table *symbols, statement_node *statement) {
    switch (statement->kind) {
        case DECLARATION:
            data_type idk = typecheck_declaration(symbols, statement->value.declaration);
            break;
        case ASSIGNMENT:
            data_type idk = typecheck_assignment(symbols, statement->value.assignment);
            break;
        default:
            panic("typecheck_statement: unhandled statement kind (%d)", statement->kind);
    }
}

void typecheck_entire_program(statement_node *first_statement) {
    symbol_table global_symbols = symbol_table_init();

    statement_node *current_statement = first_statement;
    while (current_statement != NULL) {
        typecheck_statement(&global_symbols, current_statement);
        current_statement = current_statement->next;
    }
}
