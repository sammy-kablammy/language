typedef uint32_t type_info_id;
const type_info_id TYPE_INFO_ID_INVALID = 0;

typedef struct {
    // there could be 0, 1, or many argument types
    type_info_id argument_types;
    // there could be 0 or 1 return type
    type_info_id return_type;
} function_type_info;

typedef struct type_info {
    enum {
        TYPE_INVALID = 0,

        // primitives
        TYPE_PRIMITIVE,

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
        type_info_id underlying_type; // Only used for TYPE_TYPE (this is so meta)
    } value;
    type_info_id next; // to be used for function arguments
} type_info;

struct symbol_table;

typedef struct {
    struct symbol_table *symbols;
    array *type_infos;
    bool print_symbols_when_exiting_scope; // (only used for testing)
} typecheck_state;

type_info_id type_info_alloc(typecheck_state *state, type_info info) {
    return array_append(state->type_infos, (void *)&info);
}

type_info type_info_from_id(typecheck_state *state, type_info_id id) {
    if (id == TYPE_INFO_ID_INVALID) {
        panic("cannot look up TYPE_INFO_ID_INVALID");
    }
    type_info out = { 0 };
    array_get(state->type_infos, id, &out);
    return out;
}

// Symbol table maps identifiers to types for a given scope
typedef struct symbol_table {
    struct symbol_table *parent;
    hash_table table;
} symbol_table;

symbol_table symbol_table_alloc() {
    return (symbol_table){
        .parent = NULL,
        .table = hash_table_alloc(sizeof(type_info_id)),
    };
}

void symbol_table_free(symbol_table symbols) {
    hash_table_free(symbols.table);
}

bool symbol_table_lookup(typecheck_state *state, string identifier, type_info_id *out) {
    symbol_table *symbols = state->symbols;
    while (symbols != NULL) {
        if (hash_table_lookup(&symbols->table, identifier, out)) {
            return true;
        }
        symbols = symbols->parent;
    }
    return false;
}

void symbol_table_insert(symbol_table *symbols, string identifier, type_info_id id) {
    hash_table_insert(&symbols->table, identifier, &id);
}

bool are_types_compatible(typecheck_state *state, type_info_id id1, type_info_id id2) {
    if (id1 == id2) {
        // Direct comparison of primitive types, for example:
        //   "5 + 7"
        //   "true || false"
        return true;
    }

    type_info t1 = type_info_from_id(state, id1);
    type_info t2 = type_info_from_id(state, id2);

    if (t1.kind != t2.kind) {
        // Category error, for example:
        //   "5 + mystruct"
        //   "mystruct - myfunction"
        //   "bool + true"
        return false;
    }

    switch (t1.kind) { // aka t2.kind
        // Primitive types are compared *literally*
        case TYPE_PRIMITIVE:
            // TODO (sam) how is this different from the above check? (this comment may desync, if so ignore)
            return id1 == id2;
        // Compound types are compared *structurally*
        case TYPE_FUNCTION:
            if (t1.value.function.argument_types != TYPE_INFO_ID_INVALID || t2.value.function.argument_types != TYPE_INFO_ID_INVALID) {
                panic("haven't implemented type checking of function arguments");
            }
            if (t1.value.function.return_type != TYPE_INFO_ID_INVALID || t2.value.function.return_type != TYPE_INFO_ID_INVALID) {
                panic("haven't implemented type checking of function return types");
            }
            return true;
        default:
            panic("haven't implemented type conversion for these type_info kinds (%d %d)", t1.kind, t2.kind);
    }
}

type_info_id typecheck_expression(typecheck_state *state, expression_node *expression);
void typecheck_statement(typecheck_state *state, statement_node *statement);
void print_symbol_table(typecheck_state *state, const char *name);

// TODO: ensure operator matches types. for example '123 * true' should cause a type error
type_info_id typecheck_infix_expression(typecheck_state *state, infix_node infix) {
    type_info_id lhs_type_id = typecheck_expression(state, infix.left);
    type_info_id rhs_type_id = typecheck_expression(state, infix.right);
    type_info lhs_type = type_info_from_id(state, lhs_type_id);
    type_info rhs_type = type_info_from_id(state, rhs_type_id);

    if (!are_types_compatible(state, lhs_type_id, rhs_type_id)) {
        panic("type error: infix expression lhs type (%d) doesn't match rhs type (%d)", lhs_type.kind, rhs_type.kind);
    }
    return lhs_type_id;
}

type_info_id typecheck_function_declaration(typecheck_state *state, function_node function) {
    type_info type = { .kind = TYPE_FUNCTION };
    // TODO typecheck arguments (functions can't have arguments yet)
    type.value.function.argument_types = TYPE_INFO_ID_INVALID;
    // TODO typecheck return expressions (functions can't have return types yet)
    type.value.function.return_type = TYPE_INFO_ID_INVALID;

    type_info_id id = type_info_alloc(state, type);


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
        print_symbol_table(state, "symbols for function declaration");
    }

    // restore
    state->symbols = child_scope_symbols.parent;
    symbol_table_free(child_scope_symbols);



    return id;
}

type_info_id typecheck_expression(typecheck_state *state, expression_node *expression) {
    type_info_id id = TYPE_INFO_ID_INVALID;
    switch (expression->kind) {
        case INT:
            // TODO find a way to dedupe this
            assert(symbol_table_lookup(state, string_from_cstring("int"), &id));
            assert(type_info_from_id(state, id).kind == TYPE_TYPE);
            return type_info_from_id(state, id).value.underlying_type;
        case BOOL:
            assert(symbol_table_lookup(state, string_from_cstring("bool"), &id));
            assert(type_info_from_id(state, id).kind == TYPE_TYPE);
            return type_info_from_id(state, id).value.underlying_type;
        case IDENTIFIER:
            if (symbol_table_lookup(state, expression->value.identifier, &id)) {
                return id;
            } else {
                cprintf("Could not find type of (\"%S\") in scope\n", expression->value.identifier);
                panic("type error");
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
    type_info_id id_of_lhs_type;
    if (symbol_table_lookup(state, declaration.identifier, &id_of_lhs_type)) {
        cfprintf(stderr, "type error: this identifier (\"%S\") has already been used.", declaration.identifier);
        panic("type error: shadowing is not supported.");
    }

    type_info_id id_of_desired_type;
    if (!symbol_table_lookup(state, declaration.data_type, &id_of_desired_type)) {
        panic("type error: declaration used unknown type 'thing: HERE = expression'");
    }
    type_info info_of_desired_type = type_info_from_id(state, id_of_desired_type);
    if (info_of_desired_type.kind != TYPE_TYPE) {
        panic("type error: declaration used non-type symbol 'thing: HERE = expression'");
    }

    type_info_id id_of_expression_type = typecheck_expression(state, declaration.expression);
    type_info expression_type = type_info_from_id(state, id_of_expression_type);

    type_info desired_type = type_info_from_id(state, info_of_desired_type.value.underlying_type);
    if (desired_type.kind == TYPE_TYPE) {
        if (expression_type.kind != TYPE_TYPE) {
            panic("type error: expected rhs of to be a type expression");
        }

        type_info_id new_type_id = type_info_alloc(state, (type_info){
            .kind = TYPE_TYPE,
            .value.underlying_type = expression_type.value.underlying_type,
        });
        symbol_table_insert(state->symbols, declaration.identifier, new_type_id);
        return;
    } else {
        if (!are_types_compatible(state, info_of_desired_type.value.underlying_type, id_of_expression_type)) {
            panic(
                "type error: desired type (id=%d kind=%d) does not match expression type (id=%d kind=%d)",
                info_of_desired_type.value.underlying_type,
                desired_type.kind,
                id_of_expression_type,
                expression_type.kind
            );
        }
        symbol_table_insert(state->symbols, declaration.identifier, info_of_desired_type.value.underlying_type);
    }
}

void typecheck_assignment(typecheck_state *state, assignment_node assignment) {

    type_info_id lhs_type_id;
    if (!symbol_table_lookup(state, assignment.lhs, &lhs_type_id)) {
        cprintf("\ntype error: lhs of identifier (\"%S\") not found in current or parent scope\n", assignment.lhs);
        panic("type error");
    }
    type_info lhs_type = type_info_from_id(state, lhs_type_id);

    type_info_id rhs_type_id = typecheck_expression(state, assignment.expression);
    type_info rhs_type = type_info_from_id(state, rhs_type_id);

    if (!are_types_compatible(state, lhs_type_id, rhs_type_id)) {
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
            // panic("returns are not yet typechecked");
            break;
        default:
            panic("typecheck_statement: unhandled statement kind (%d)", statement->kind);
    }
}

void typecheck_top_level_statement(statement_node *statement, bool debug_print_symbols) {

    symbol_table symbols = symbol_table_alloc();
    array type_infos = array_alloc(sizeof(type_info));
    typecheck_state state = {
        .symbols = &symbols,
        .type_infos = &type_infos,
        .print_symbols_when_exiting_scope = debug_print_symbols,
    };

    // populate builtin types
    type_info_alloc(&state, (type_info) { .kind = TYPE_INVALID }); // invalid needs to be 0 so this goes here, unused
    // type_type forms a cycle (its underlying type is itself). must go first so that it gets ID 1.
    const type_info_id type_type_id = type_info_alloc(&state, (type_info) { .kind = TYPE_TYPE, .value.underlying_type = 1 });
    assert(type_type_id == 1);
    // the rest of the primitive types go after invalid and the metatype
    // Each type gets a primitive type, and a "type" so it is accessible by name
    const type_info_id int_id = type_info_alloc(&state, (type_info) { .kind = TYPE_PRIMITIVE });
    const type_info_id bool_id = type_info_alloc(&state, (type_info) { .kind = TYPE_PRIMITIVE });
    const type_info_id int_type = type_info_alloc(&state, (type_info) { .kind = TYPE_TYPE, .value.underlying_type = int_id });
    const type_info_id bool_type = type_info_alloc(&state, (type_info) { .kind = TYPE_TYPE, .value.underlying_type = bool_id });

    // populate builtin symbols
    symbol_table_insert(state.symbols, string_from_cstring("type"), type_type_id);
    symbol_table_insert(state.symbols, string_from_cstring("int"), int_type);
    symbol_table_insert(state.symbols, string_from_cstring("bool"), bool_type);
    // HACK: 'main' is predefined because currently functions can only be ASSIGNED, not DECLARED.
    //        This can go away once 'main :: func() { // ...' is parseable.
    const type_info_id secret_main_type = type_info_alloc(&state, (type_info) {
        .kind = TYPE_FUNCTION,
        .value.function = { .argument_types = TYPE_INFO_ID_INVALID, .return_type = TYPE_INFO_ID_INVALID },
    });
    symbol_table_insert(state.symbols, string_from_cstring("main"), secret_main_type);
    // This is where type_infos for other top-level declarations would be inserted

    typecheck_statement(&state, statement);

    if (debug_print_symbols) {
        print_symbol_table(&state, "top level symbols");
    }

    symbol_table_free(symbols);
    array_free_contents(&type_infos);
}

void print_type_info(typecheck_state *state, type_info_id id) {
    type_info info = type_info_from_id(state, id);
    switch (info.kind) {
        case TYPE_INVALID:
            panic("invalid type, this shouldn't be here");
            break;
        case TYPE_PRIMITIVE:
            cprintf("primitive whose id is %u", id);
            break;
        case TYPE_FUNCTION:
            cprintf("function (todo print argument/return types)");
            break;
        case TYPE_TYPE:
            cprintf("type whose underlying ID is %u", info.value.underlying_type);
            break;
    }
}

void _print_symbol_table_callback(string key, void *value, void *userdata) {
    type_info_id id = *(type_info_id *)value;
    typecheck_state *state = userdata;

    cprintf("| \"%S\" | ", key);
    print_type_info(state, id);
    cprintf("\n");
}

void print_symbol_table(typecheck_state *state, const char *name) {
    symbol_table *symbols = state->symbols;
    printf("================= %s =================\n", name);
    hash_table_foreach(symbols->table, _print_symbol_table_callback, state);
    symbols = symbols->parent;
    while (symbols != NULL) {
        printf("=== and now for its parent symbol table\n");
        hash_table_foreach(symbols->table, _print_symbol_table_callback, state);
        symbols = symbols->parent;
    }
    printf("=============== END %s ===============\n", name);
}
