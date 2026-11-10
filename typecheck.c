struct type_info;

typedef struct {
    struct type_info *argument_types;
    struct type_info *return_types;
} function_type_info;

typedef struct {
    // TODO
    struct type_info *members;
} struct_type_info;

typedef struct type_info {
    enum {
        TYPE_INVALID = 0,
        // primitives
        TYPE_INT,
        TYPE_BOOL,
        // compound types
        TYPE_FUNCTION,
        TYPE_STRUCT,
        // TYPE_STATIC_ARRAY, // could have 'kind' and 'size' for example { TYPE_INT, 100 }
        // meta
        TYPE_TYPE,
        // Note there is no concept of "safe range" for user defined types. User
        // defined types are always kind=TYPE_TYPE and point to a particular
        // underlying type. We're not generating *new* types anywhere, only
        // providing wrappers for existing types.
    } kind;
    union {
        // void *primitive; // Primitive types don't have any other info
        function_type_info function;
        struct_type_info structure;
        struct type_info *underlying_type; // Only used for TYPE_TYPE (this is so meta)
    } value;
    // bool is_constant; // TODO(sam) differentiate between constants and variables
                         // Actually, constness is really an aspect of a
                         // particular declaration, not of the type itself.
    struct type_info *next; // type_infos can be made into an intrusive list, for function arguments and stuff
} type_info;

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

// typedef uint32_t data_type;
// const data_type TYPE_INVALID = 0;
// string builtin_types[] = {
//     // I don't like this but eh
//     { .buf = "int", .length = sizeof("int") - 1 },
//     { .buf = "bool", .length = sizeof("bool") - 1 },
// };
// const uint32_t NUM_BUILTIN_TYPES = sizeof(builtin_types) / sizeof(builtin_types[0]);
// // User-defined types will counting after all the builtins

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
    // TODO(sam) should check here whether identifier is already taken
    // if (hash_table_lookup()) {
    // panic()
    // }
    // cprintf("Inserting '%S' into hash table\n", identifier);
    hash_table_insert(&symbols->table, identifier, (void *)&info);
    // cprintf("Inserted '%S'\n", identifier);
}

// TODO(sam) should this be a deep equals? depends whether we want duck typing i 'pose
bool type_equals(type_info t1, type_info t2) {
    if (t1.kind != t2.kind) {
        return false;
    }
    if (t1.kind == TYPE_INT || t1.kind == TYPE_BOOL) {
        return true;
    }
    switch (t1.kind) {
        // Primitive types
        case TYPE_INT: // fall through
        case TYPE_BOOL:
            return t1.kind == t2.kind;
        // Compound types
        case TYPE_FUNCTION:
            if (t1.value.function.argument_types != NULL || t2.value.function.argument_types != NULL) {
                panic("haven't implemented type checking of function arguments");
            }
            if (t1.value.function.return_types != NULL || t2.value.function.return_types != NULL) {
                panic("haven't implemented type checking of function return types");
            }
            return true;
            // TODO
        // Meta
            // TODO
        default:
            panic("haven't implemented type conversion for these type_info kinds (%d %d)", t1.kind, t2.kind);
    }
}

type_info typecheck_expression(symbol_table *symbols, expression_node *expression);
void typecheck_statement(symbol_table *symbols, statement_node *statement);

// TODO: ensure operator matches types. for example '123 * true' should cause a type error
type_info typecheck_infix_expression(symbol_table *symbols, infix_node infix) {
    type_info lhs_type = typecheck_expression(symbols, infix.left);
    type_info rhs_type = typecheck_expression(symbols, infix.right);
    if (!type_equals(lhs_type, rhs_type)) {
        panic("type error: infix expression lhs type (%d) doesn't match rhs type (%d)", lhs_type.kind, rhs_type.kind);
    }
    return lhs_type;
}

type_info typecheck_function_declaration(symbol_table *symbols, function_node function) {

    type_info type = { .kind = TYPE_FUNCTION };
    // TODO typecheck arguments (functions can't have arguments yet)
    // TODO typecheck return expressions (functions can't have return types yet)
    type.value.function.argument_types = NULL;
    type.value.function.return_types = NULL;



    symbol_table child_scope_symbols = symbol_table_alloc();
    child_scope_symbols.parent = symbols;

    statement_node *statement = function.body;
    while (statement != NULL) {
        typecheck_statement(&child_scope_symbols, statement);
        statement = statement->next;
    }

    symbol_table_free(child_scope_symbols);



    return type;
}

type_info typecheck_expression(symbol_table *symbols, expression_node *expression) {
    type_info type;
    switch (expression->kind) {
        case INT:
            return (type_info){ .kind = TYPE_INT };
        case BOOL:
            return (type_info){ .kind = TYPE_BOOL };
        case IDENTIFIER:
            type = symbol_table_lookup(symbols, expression->value.identifier);
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
            return typecheck_infix_expression(symbols, expression->value.infix);
        case INLINE_FUNCTION:
            return typecheck_function_declaration(symbols, expression->value.function_declaration);
        default:
            panic("unhandled expression kind in typecheck_expression (%d)", expression->kind);
    }
}

void typecheck_declaration(symbol_table *symbols, declaration_node declaration) {
    type_info lhs_type = symbol_table_lookup(symbols, declaration.identifier);
    if (lhs_type.kind != TYPE_INVALID) {
        // TODO consider child<->parent scope shadowing (same-scope shadowing likely banned)
        panic("shadowing is not supported (at least not currently)");
    }

    type_info desired_type = symbol_table_lookup(symbols, declaration.data_type);
    if (desired_type.kind == TYPE_INVALID) {
        printf("type error: attempted to use unknown type '");
        print_string(declaration.data_type);
        printf("' in declaration\n");
        panic("(make sure that type is defined prior to use)");
    }
    type_info expression_type = typecheck_expression(symbols, declaration.expression);
    if (!type_equals(desired_type, expression_type)) {
        panic("type error: desired type (%d) does not match expression type (%d)", desired_type.kind, expression_type.kind);
    }

    symbol_table_insert(symbols, declaration.identifier, desired_type);
}

void typecheck_assignment(symbol_table *symbols, assignment_node assignment) {
    type_info lhs_type = symbol_table_lookup(symbols, assignment.lhs);
    if (lhs_type.kind == TYPE_INVALID) {
        printf("\ntype error: lhs of identifier (\"");
        print_string(assignment.lhs);
        printf("\") not found in current or parent scope\n");
        panic("assignment typecheck failed");
    }

    type_info rhs_type = typecheck_expression(symbols, assignment.expression);
    if (!type_equals(lhs_type, rhs_type)) {
        panic("type error: Assignment's RHS type (%d) does not match LHS type (%d)", rhs_type.kind, lhs_type.kind);
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
        case RETURN:
            // TODO(sam) need a way to pass current function's type_info to typecheck_return
            break;
        default:
            panic("typecheck_statement: unhandled statement kind (%d)", statement->kind);
    }
}

#define from_cstr string_from_cstring

void print_symbol_table(symbol_table *symbols, char *name);

void typecheck_entire_program(symbol_table *global_symbols, statement_node *first_statement) {
    // Start counting types at 1, since 0 represents TYPE_INVALID
    // data_type next_available_type_id = 1;
    // // Predefined types
    // for (uint32_t i = 0; i < NUM_BUILTIN_TYPES; i++) {
    //     symbol_table_insert(global_types, builtin_types[i], i + 1);
    //     next_available_type_id += 1;
    // }
    // // Predefined symbols
    // // TODO(sam) what number is this?
    // // symbol_table_insert(global_symbols, string_from_cstring("main"), 67);
    // // next_available_type_id += 1;



    // Predefined types
    symbol_table_insert(global_symbols, from_cstr("int"), (type_info){ .kind = TYPE_INT });
    //     ("int" is one possible name for the underlying concept of intness. Other aliases ("myint") may refer to the same underlying type)
    symbol_table_insert(global_symbols, from_cstr("bool"), (type_info){ .kind = TYPE_BOOL });
    // Predefined symbols (if any). for now this is a hack because functions
    // can only be ASSIGNED, not DECLARED
    symbol_table_insert(global_symbols, from_cstr("main"), (type_info){
        .kind = TYPE_FUNCTION,
        .value.function = { .argument_types = NULL, .return_types = NULL },
    });

    print_symbol_table(global_symbols, "sam");



    statement_node *current_statement = first_statement;
    while (current_statement != NULL) {
        typecheck_statement(global_symbols, current_statement);
        current_statement = current_statement->next;
    }
}

void print_symbol_table(symbol_table *symbols, char *name) {
    hash_table_print(symbols->table,name);
}
