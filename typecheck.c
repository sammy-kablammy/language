// There is a notion of type *equality* (these types are literally the same) and
// type *castability* (for example if i do "myint: type = int" i should then be
// able to do "x: int = (int)some_myint" but not "x: int = some_myint"
// (casting syntax not finalized)

// Symbol table maps identifiers to types. Like 
typedef struct symbol_table {
    struct symbol_table *parent;
} symbol_table;

typedef enum data_type {
    // TODO(sam) meaningful zero value here?

    // Primitive / builtin types go here
    TYPE_INT,
    TYPE_BOOL,
    // TYPE_FLOAT,
    // TYPE_STRING,

    // User defined data types start at the end and go up
    USER_DEFINED_TYPE_START,
} data_type;

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

data_type typecheck_assignment(assignment_node assignment) {

    data_type left_side_type = xxx;
    data_type right_side_type = typecheck_expression(assignment.expression);

    if (left_side_type != right_side_type) {
        panic("Assignment's RHS type does not match LHS type");
        return false;
    }

    return true;

}

data_type typecheck_statement(statement_node *statement) {
    switch (statement->kind) {
        case ASSIGNMENT:
            break;
        default:
            panic("typecheck_statement: unhandled statement kind (%d)", statement->kind);
    }
}
