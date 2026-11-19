// Convert top level statement into snippets of a bytecode program, then
// possibly stitch them together

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
    } kind; // perhaps should be "opcode"
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

////////////////////////////////////////////////////////////////////////////////

// Each bytecodify_* function generates some intermediate registers, and stores
// the final result in a register. Return the result register so the caller
// knows where the result is.

reg bytecodify_expression(bytecode_snippet *snippet, reg last_used_reg, expression_node *expression);
reg bytecodify_statement(bytecode_snippet *snippet, reg last_used_reg, symbol_table *symbols, statement_node *statement);

reg bytecodify_infix_expression(bytecode_snippet *snippet, reg last_used_reg, infix_node infix) {

    reg lhs_result = bytecodify_expression(snippet, last_used_reg, infix.left);
    reg rhs_result = bytecodify_expression(snippet, lhs_result, infix.right);

    reg_op_reg_instr instr = {
        .op1 = lhs_result,
        .op2 = rhs_result,
        .dest = rhs_result + 1,
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
    append_instruction(snippet, (instruction){ .kind = kind, .value.reg_op_reg = instr });

    return instr.dest;
}

bytecode_snippet bytecodify_function_declaration(symbol_table *parent_symbols, function_node function){
    bytecode_snippet snippet = bytecode_snippet_alloc();
    reg current_reg = REGISTER_INVALID;

    statement_node *statement = function.body;
    while (statement != NULL) {
        current_reg = bytecodify_statement(&snippet, current_reg, parent_symbols, statement);
        statement = statement->next;
    }

    return snippet;
}

reg bytecodify_expression(bytecode_snippet *snippet, reg last_used_reg, expression_node *expression) {
    switch (expression->kind) {
        case INT:
            append_instruction(snippet, (instruction){
                .kind = IMMEDIATE_S32,
                .value.immediate_s32 = {
                    .dest = ++last_used_reg,
                    .the_s32 = expression->value.integer,
                },
            });
            return last_used_reg;
        case INFIX:
            return bytecodify_infix_expression(snippet, last_used_reg, expression->value.infix);
            break;
        default:
            panic("unhandled expression kind=%d in bytecodify_expression", expression->kind);
    }

    return REGISTER_INVALID;
}

reg bytecodify_assignment(bytecode_snippet *snippet, reg last_used_reg, symbol_table *symbols, assignment_node assignment) {
    reg rhs = bytecodify_expression(snippet, last_used_reg, assignment.expression);
    symbol_table_insert(symbols, assignment.lhs, rhs);
    return rhs;
}
reg bytecodify_declaration(bytecode_snippet *snippet, reg last_used_reg, symbol_table *symbols, declaration_node declaration) {
    reg rhs = bytecodify_expression(snippet, last_used_reg, declaration.expression);
    symbol_table_insert(symbols, declaration.identifier, rhs);
    return rhs;
}

reg bytecodify_statement(bytecode_snippet *snippet, reg last_used_reg, symbol_table *symbols, statement_node *statement) {
    switch (statement->kind) {
        case DECLARATION:
            return bytecodify_declaration(snippet, last_used_reg, symbols, statement->value.declaration);
            break;
        case ASSIGNMENT:
            return bytecodify_assignment(snippet, last_used_reg, symbols, statement->value.assignment);
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
