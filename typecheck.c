
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
    TYPE_BYTE
    TYPE_FLOAT,
    TYPE_STRING,

    // User defined data types start at the end and go up
    USER_DEFINED_TYPE_START,
} data_type;


bool typecheck_infix_expression() {
}

bool typecheck_expression() {
    switch (expression->kind) {
        case INFIX:
            break;
        default:
            panic("unhandled expression kind in typecheck_expression");
    }
}

bool typecheck_assignment() {

    type left_side_type = xxx;
    type right_side_type = xxx;

    if (!type_equals(left_side_type, right_side_type)) {
        type_error("Assignment expression doesn't match LHS type");
        return false;
    }

    return true;

}

bool typecheck_statement() {
    switch (statement.kind) {
    }
}
