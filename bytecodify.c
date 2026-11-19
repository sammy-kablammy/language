typedef uint32_t reg;
const reg REGISTER_INVALID = 0;

typedef uint32_t instruction_id;

typedef struct {
    reg dest;
    int32_t the_s32;
} immediate_s32_instr;

typedef struct {
    reg dest;
    reg op1;
    reg op2;
} reg_op_reg_instr;

typedef struct instruction {
    enum {
        INSTRUCTION_INVALID = 0,
        IMMEDIATE_S32,
        ADD_S32,
        SUBTRACT_S32,
        MULTIPLY_S32,
        DIVIDE_S32,
    } kind;
    union {
        immediate_s32_instr immediate_s32;
        reg_op_reg_instr reg_op_reg;
    } value;
} instruction;

typedef array bytecode_snippet;
bytecode_snippet bytecode_snippet_alloc() {
    return array_alloc(sizeof(bytecode_snippet));
}
instruction get_instruction(bytecode_snippet *snippet, int i) {
    instruction out;
    array_get(snippet, i, &out);
    return out;
}
void append_instruction(bytecode_snippet *snippet, instruction instr) {
    array_append(snippet, &instr);
}

void debug_print_bytecode_snippet(bytecode_snippet *snippet) {
    for (uint64_t i = 0; i < snippet->length; i++) {
        instruction instr = get_instruction(snippet, i);
        switch (instr.kind) {
            case IMMEDIATE_S32:
                printf("%ld  |  t%u = %d\n", i, instr.value.immediate_s32.dest, instr.value.immediate_s32.the_s32);
                break;
            case ADD_S32:
                printf("%ld  |  t%u = add_s32 t%u t%u\n", i, instr.value.reg_op_reg.dest, instr.value.reg_op_reg.op1, instr.value.reg_op_reg.op2);
                break;
            case SUBTRACT_S32:
                printf("%ld  |  t%u = subtract_s32 t%u t%u\n", i, instr.value.reg_op_reg.dest, instr.value.reg_op_reg.op1, instr.value.reg_op_reg.op2);
                break;
            case MULTIPLY_S32:
                printf("%ld  |  t%u = multiply_s32 t%u t%u\n", i, instr.value.reg_op_reg.dest, instr.value.reg_op_reg.op1, instr.value.reg_op_reg.op2);
                break;
            case DIVIDE_S32:
                printf("%ld  |  t%u = divide_s32 t%u t%u\n", i, instr.value.reg_op_reg.dest, instr.value.reg_op_reg.op1, instr.value.reg_op_reg.op2);
                break;
            case INSTRUCTION_INVALID:
                panic("you forgot to initialize an instruction kind somewhere");
        }
    }
}

typedef struct {
    bytecode_snippet *snippet;
    reg last_used_reg;
    symbol_table *symbols;
} bytecode_state;

reg generate_register(bytecode_state *state) {
    state->last_used_reg += 1;
    return state->last_used_reg;
}

// Each bytecodify_* function generates some intermediate registers, and stores
// the final result in a register. Return the result register so the caller
// knows where the result is.

void bytecodify_expression(bytecode_state *state, expression_node *expression);
void bytecodify_statement(bytecode_state *state, statement_node *statement);

void bytecodify_infix_expression(bytecode_state *state, infix_node infix) {

    bytecodify_expression(state, infix.left);
    reg lhs_result = state->last_used_reg;
    bytecodify_expression(state, infix.right);
    reg rhs_result = state->last_used_reg;

    reg_op_reg_instr instr = {
        .op1 = lhs_result,
        .op2 = rhs_result,
        .dest = generate_register(state),
    };

    uint64_t kind = INSTRUCTION_INVALID;
    switch (infix.operator.kind) {
        case TOKEN_PLUS:
            kind = ADD_S32;
            break;
        case TOKEN_MINUS:
            kind = SUBTRACT_S32;
            break;
        case TOKEN_STAR:
            kind = MULTIPLY_S32;
            break;
        case TOKEN_SLASH:
            kind = DIVIDE_S32;
            break;
        default:
            panic("Unhandled operator (%d) in bytecodify_infix_expression\n", infix.operator.kind);
    }
    append_instruction(state->snippet, (instruction){ .kind = kind, .value.reg_op_reg = instr });

}

bytecode_snippet bytecodify_function_declaration(symbol_table *parent_symbols, function_node function){
    bytecode_snippet snippet = bytecode_snippet_alloc();
    bytecode_state state = {
        .snippet = &snippet,
        .last_used_reg = REGISTER_INVALID,
        .symbols = parent_symbols,
    };

    statement_node *statement = function.body;
    while (statement != NULL) {
        bytecodify_statement(&state, statement);
        statement = statement->next;
    }

    return snippet;
}

void bytecodify_expression(bytecode_state *state, expression_node *expression) {
    switch (expression->kind) {
        case INT:
            append_instruction(state->snippet, (instruction){
                .kind = IMMEDIATE_S32,
                .value.immediate_s32 = {
                    .dest = generate_register(state),
                    .the_s32 = expression->value.integer,
                },
            });
            break;
        case INFIX:
            bytecodify_infix_expression(state, expression->value.infix);
            break;
        default:
            panic("unhandled expression kind=%d in bytecodify_expression", expression->kind);
    }
}

void bytecodify_statement(bytecode_state *state, statement_node *statement) {
    switch (statement->kind) {
        case DECLARATION:
            bytecodify_expression(state, statement->value.declaration.expression);
            symbol_table_insert(state->symbols, statement->value.declaration.identifier, state->last_used_reg);
            break;
        case ASSIGNMENT:
            bytecodify_expression(state, statement->value.assignment.expression);
            symbol_table_insert(state->symbols, statement->value.assignment.lhs, state->last_used_reg);
            break;
        default:
            panic("todo need to implement bytecodify_statement");
    }
}

void _print_bytecode_symbol_table_callback(string key, uint32_t value, void *userdata) {
    (void)userdata;
    cprintf("The most recent value of variable \"%S\" is in t%u\n", key, value);
}
void print_bytecode_symbol_table(symbol_table *symbols) {
    hash_table_foreach(symbols->table, _print_bytecode_symbol_table_callback, NULL);
}
