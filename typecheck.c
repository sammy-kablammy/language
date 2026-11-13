struct type_info;
typedef uint32_t type_info_id;
const type_info_id TYPE_INFO_ID_INVALID = 0;

typedef struct {
    // there could be 0, 1, or many argument or return types
    type_info_id argument_types;
    type_info_id return_types;
} function_type_info;

typedef struct {
    enum {
        TYPE_INVALID = 0,

        // primitives
        TYPE_INT,
        TYPE_BOOL,

        // compound types
        TYPE_FUNCTION,
        // TYPE_STRUCT,

        // meta
        TYPE_TYPE,
    } kind;
    union {
        // void *primitive; // Primitive types don't have any other info
        function_type_info function;
        // struct_type_info structure;
        type_info_id id_of_underlying_type; // Only used for TYPE_TYPE (this is so meta)
    } value;
    // type_info_id next; // to be used for function arguments
} type_info;

typedef array type_info_array;
// Return the ID of whatever was just appended
type_info_id type_info_array_append(type_info_array *arr, type_info info) {
    array_append(arr, &info);
    return arr->length - 1;
}
type_info type_info_array_get(type_info_array *arr, type_info_id id) {
    void *element = array_get(arr, id);
    return *(type_info *)element;
}
type_info_array type_info_array_alloc() {
    return array_alloc(sizeof(type_info));
}
void type_info_array_free_contents(type_info_array *arr) {
    array_free_contents(arr);
}

// Symbol table maps identifiers to types for a given scope
typedef struct symbol_table {
    struct symbol_table *parent;
    hash_table table;
} symbol_table;

symbol_table symbol_table_alloc() {
    return (symbol_table){
        .parent = NULL,
        .table = hash_table_alloc(sizeof(type_info)),
    };
}

void symbol_table_free(symbol_table symbols) {
    hash_table_free(symbols.table);
}

// TODO this should be the same as hash_table_lookup (using an outparam)
type_info symbol_table_lookup(symbol_table *symbols, string identifier) {
    while (symbols != NULL) {
        type_info out;
        if (hash_table_lookup(&symbols->table, identifier, (void *)&out)) {
            return out;
        }
        symbols = symbols->parent;
    }
    type_info invalid = { 0 };
    return invalid;
}

void symbol_table_insert(symbol_table *symbols, string identifier, type_info info) {
    hash_table_insert(&symbols->table, identifier, (void *)&info);
}

typedef struct {
    symbol_table *symbols;
    type_info_array *type_info_registry;
    bool print_symbols_when_exiting_scope; // (only used for testing)
} typecheck_state;

// TODO(sam) should this be a deep equals? depends whether we want duck typing i 'pose
bool type_equals(type_info t1, type_info t2) {
    if (t1.kind != t2.kind) {
        return false;
    }
    switch (t1.kind) {
        // Primitive types
        case TYPE_INT: // fall through
        case TYPE_BOOL:
            return t1.kind == t2.kind;
        // Compound types
        case TYPE_FUNCTION:
            if (t1.value.function.argument_types != TYPE_INFO_ID_INVALID || t2.value.function.argument_types != TYPE_INFO_ID_INVALID) {
                panic("haven't implemented type checking of function arguments");
            }
            if (t1.value.function.return_types != TYPE_INFO_ID_INVALID || t2.value.function.return_types != TYPE_INFO_ID_INVALID) {
                panic("haven't implemented type checking of function return types");
            }
            return true;
        default:
            panic("haven't implemented type conversion for these type_info kinds (%d %d)", t1.kind, t2.kind);
    }
}

type_info typecheck_expression(typecheck_state *state, expression_node *expression);
void typecheck_statement(typecheck_state *state, statement_node *statement);
void print_symbol_table(symbol_table *symbols, const char *name);

// TODO: ensure operator matches types. for example '123 * true' should cause a type error
type_info typecheck_infix_expression(typecheck_state *state, infix_node infix) {
    type_info lhs_type = typecheck_expression(state, infix.left);
    type_info rhs_type = typecheck_expression(state, infix.right);

    if (!type_equals(lhs_type, rhs_type)) {
        panic("type error: infix expression lhs type (%d) doesn't match rhs type (%d)", lhs_type.kind, rhs_type.kind);
    }
    return lhs_type;
}

type_info typecheck_function_declaration(typecheck_state *state, function_node function) {
    type_info type = { .kind = TYPE_FUNCTION };
    // TODO typecheck arguments (functions can't have arguments yet)
    type.value.function.argument_types = TYPE_INFO_ID_INVALID;
    // TODO typecheck return expressions (functions can't have return types yet)
    type.value.function.return_types = TYPE_INFO_ID_INVALID;



    // save
    symbol_table child_scope_symbols = symbol_table_alloc();
    child_scope_symbols.parent = state->symbols;
    state->symbols = &child_scope_symbols;

    statement_node *statement = function.body;
    while (statement != NULL) {
        typecheck_statement(state, statement);
        statement = statement->next;
    }
    if (state->print_symbols_when_exiting_scope) {
        print_symbol_table(&child_scope_symbols, "symbols for function declaration");
    }

    // restore
    state->symbols = child_scope_symbols.parent;
    symbol_table_free(child_scope_symbols);



    return type;
}

type_info typecheck_expression(typecheck_state *state, expression_node *expression) {
    type_info type;
    switch (expression->kind) {
        case INT:
            return (type_info){ .kind = TYPE_INT };
        case BOOL:
            return (type_info){ .kind = TYPE_BOOL };
        case IDENTIFIER:
            type = symbol_table_lookup(state->symbols, expression->value.identifier);
            if (type.kind != TYPE_INVALID) {
                return type;
            } else {
                printf("Could not find type of (\"");
                printf("\") in scope");
                print_string(expression->value.identifier);
                panic("Could not find identifier");
            }
            break;
        case INFIX:
            return typecheck_infix_expression(state, expression->value.infix);
        case INLINE_FUNCTION:
            return typecheck_function_declaration(state, expression->value.function_declaration);
        default:
            panic("unhandled expression kind in typecheck_expression (%d)", expression->kind);
    }
}

void typecheck_declaration(typecheck_state *state, declaration_node declaration) {
    type_info lhs_type = symbol_table_lookup(state->symbols, declaration.identifier);
    if (lhs_type.kind != TYPE_INVALID) {
        // TODO consider child<->parent scope shadowing (same-scope shadowing likely banned)
        panic("shadowing is not supported (at least not currently)");
    }

    type_info info_of_desired_type = symbol_table_lookup(state->symbols, declaration.data_type);
    if (info_of_desired_type.kind != TYPE_TYPE) {
        panic("type error: expected type in declaration");
    }
    type_info desired_type = type_info_array_get(state->global_type_infos, info_of_desired_type.value.id_of_underlying_type);
    if (desired_type.kind == TYPE_TYPE) {
        panic("first-class types not yet supported");
    }

    type_info expression_type = typecheck_expression(state, declaration.expression);

    if (!type_equals(desired_type, expression_type)) {
        panic("type error: desired type (%d) does not match expression type (%d)", desired_type.kind, expression_type.kind);
    }

    symbol_table_insert(state->symbols, declaration.identifier, desired_type);
}

void typecheck_assignment(typecheck_state *state, assignment_node assignment) {
    type_info lhs_type = symbol_table_lookup(state->symbols, assignment.lhs);
    if (lhs_type.kind == TYPE_INVALID) {
        printf("\ntype error: lhs of identifier (\"");
        print_string(assignment.lhs);
        printf("\") not found in current or parent scope\n");
        panic("assignment typecheck failed");
    }

    type_info rhs_type = typecheck_expression(state, assignment.expression);
    if (!type_equals(lhs_type, rhs_type)) {
        panic("type error: Assignment's RHS type (%d) does not match LHS type (%d)", rhs_type.kind, lhs_type.kind);
    }
}

void typecheck_statement(typecheck_state *state, statement_node *statement) {
    switch (statement->kind) {
        case DECLARATION:
            typecheck_declaration(state, statement->value.declaration);
            break;
        case ASSIGNMENT:
            typecheck_assignment(state, statement->value.assignment);
            break;
        case RETURN:
            // TODO(sam) need a way to pass current function's type_info to typecheck_return
            break;
        default:
            panic("typecheck_statement: unhandled statement kind (%d)", statement->kind);
    }
}

void typecheck_top_level_statement(statement_node *statement, bool debug_print_symbols) {

    // populate builtin types
    type_info_array type_info_registry = type_info_array_alloc(sizeof(type_info));
    type_info_array_append(&type_info_registry, (type_info){ .kind = TYPE_INVALID }); // (TYPE_INFO_ID_INVALID)
    // const type_info_id type_id = type_info_array_append(&type_info_registry, (type_info){
    //     .kind = TYPE_TYPE,
    //     // TYPE_TYPE id must be defined first so it has a known id. this forms a
    //     // cycle (the underlying type of "type" is itself)
    //     .value.id_of_underlying_type = 1,
    // });
    const type_info_id int_id = type_info_array_append(&type_info_registry, (type_info){ .kind = TYPE_INT });
    const type_info_id bool_id = type_info_array_append(&type_info_registry, (type_info){ .kind = TYPE_BOOL });

    // populate builtin symbols
    symbol_table symbols = symbol_table_alloc();
    symbol_table_insert(&symbols, string_from_cstring("int"), (type_info) {
            .kind = TYPE_TYPE,
            .value.id_of_underlying_type = int_id,
    });
    symbol_table_insert(&symbols, string_from_cstring("bool"), (type_info) {
            .kind = TYPE_TYPE,
            .value.id_of_underlying_type = bool_id,
    });
    // symbol_table_insert(&symbols, string_from_cstring("type"), (type_info) {
    //         .kind = TYPE_TYPE,
    //         value.id_of_underlying_type = type_id,
    // });
    // HACK: 'main' is predefined because currently functions can only be ASSIGNED, not DECLARED.
    //        This can go away once 'main :: func() { // ...' is parseable.
    symbol_table_insert(&symbols, string_from_cstring("main"), (type_info){
        .kind = TYPE_FUNCTION,
        .value.function = { .argument_types = TYPE_INFO_ID_INVALID, .return_types = TYPE_INFO_ID_INVALID },
    });
    // This is where type_infos for other top-level declarations would be inserted

    typecheck_state state = {
        .symbols = &symbols,
        .type_info_registry = &type_info_registry,
        .print_symbols_when_exiting_scope = debug_print_symbols,
    };

    typecheck_statement(&state, statement);

    if (debug_print_symbols) {
        print_symbol_table(&symbols, "symbols for top-level scope");
    }

    symbol_table_free(symbols);
    type_info_array_free_contents(&type_info_registry);
}

void print_type_info(void *info) {
    type_info *t = (type_info *)info;
    switch (t->kind) {
        case TYPE_INVALID:
            cprintf("(invalid type, this shouldn't be here)");
            break;
        case TYPE_INT:
            cprintf("int");
            break;
        case TYPE_BOOL:
            cprintf("bool");
            break;
        case TYPE_FUNCTION:
            cprintf("function");
            // print argument types here
            // print return types here
            break;
        case TYPE_TYPE:
            cprintf("type whose underlying ID is (%d)", t->value.id_of_underlying_type);
            break;
    }
}

// does not print parent table
void print_symbol_table(symbol_table *symbols, const char *name) {
    hash_table_print(symbols->table, name, print_type_info);
}
