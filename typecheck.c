// There is a notion of type *equality* (these types are literally the same) and
// type *castability* (for example if i do "myint: type = int" i should then be
// able to do "x: int = (int)some_myint" but not "x: int = some_myint"
// (casting syntax not finalized)

// Symbol table maps identifiers to types.
typedef struct symbol_table {
    struct symbol_table *parent;
    hash_table table;
} symbol_table;

typedef uint32_t data_type;
// NOTE C99 apparently doesn't let you choose the size of an enum
typedef enum {
    // TODO(sam) meaningful zero value here?

    // Primitive / builtin types go here
    TYPE_INT,
    TYPE_BOOL,
    // TYPE_FLOAT,
    // TYPE_STRING,

    // User defined data types start at the end and go up
    USER_DEFINED_TYPE_START,
};

data_type symbol_table_lookup(symbol_table *symbols, string identifier) {
    data_type out;
    bool exists = hash_table_lookup(all_symbols, identifier, out)
}

// The convention in this file is that typecheck_*_expression functions will
// return the type that they got. Unless they find a type error.

data_type typecheck_infix_expression() {
}

data_type typecheck_expression(expression_node *e) {
    switch (expression->kind) {
        case INFIX:
            return typecheck_infix_expression(e->value.infix);
        default:
            panic("unhandled expression kind in typecheck_expression");
    }
}

data_type typecheck_assignment(symbol_table symbols, assignment_node assignment) {

    // a = b when 'a' doesn't exist yet should yield a type error
    // a = b when 'a' does exist is fine, so long as b matches type of a
    // a: t = b is fine when b matches t

    if (assignment.has_data_type) {
    }

    data_type left_side_type = xxx;
    data_type right_side_type = typecheck_expression(assignment.expression);

    if (left_side_type != right_side_type) {
        panic("Assignment's RHS type does not match LHS type");
        return false;
    }

    return true;

}

// TODO(sam) figure out whether top-level statements are treated separately
// from regular statements

data_type typecheck_statement(symbol_table symbols, statement_node *statement) {
    switch (statement->kind) {
        case ASSIGNMENT:
            data_type idk = typecheck_assignment(symbols, statement->value.assignment);
            break;
        default:
            panic("typecheck_statement: unhandled statement kind (%d)", statement->kind);
    }
}
