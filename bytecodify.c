// Convert top level statement into snippets of a bytecode program, then
// possibly stitch them together

typedef uint64_t reg;
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
instruction get_instruction(bytecode_snippet *snippet, int i) {
    return *(instruction *)array_get(snippet, i);
}
void append_instruction(bytecode_snippet *snippet, instruction instr) {
    array_append(snippet, &instr);
}

void debug_print_bytecode_snippet(bytecode_snippet *snippet) {
    for (uint64_t i = 0; i < snippet->length; i++) {
        instruction instr = get_instruction(snippet, i);
        switch (instr.kind) {
            case IMMEDIATE_S32:
                printf("%ld  |  t%ld = %d\n", i, instr.value.immediate_s32.dest, instr.value.immediate_s32.the_s32);
                break;
            case ADD_S32:
                printf("%ld  |  t%ld = add_s32 t%ld t%ld\n", i, instr.value.reg_op_reg.dest, instr.value.reg_op_reg.op1, instr.value.reg_op_reg.op2);
                break;
            case SUBTRACT_S32:
                printf("%ld  |  t%ld = subtract_s32 t%ld t%ld\n", i, instr.value.reg_op_reg.dest, instr.value.reg_op_reg.op1, instr.value.reg_op_reg.op2);
                break;
            case MULTIPLY_S32:
                printf("%ld  |  t%ld = multiply_s32 t%ld t%ld\n", i, instr.value.reg_op_reg.dest, instr.value.reg_op_reg.op1, instr.value.reg_op_reg.op2);
                break;
            case DIVIDE_S32:
                printf("%ld  |  t%ld = divide_s32 t%ld t%ld\n", i, instr.value.reg_op_reg.dest, instr.value.reg_op_reg.op1, instr.value.reg_op_reg.op2);
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

reg bytecodify_infix_expression(bytecode_snippet *snippet, reg last_used_reg, infix_node infix) {

    reg lhs_result = bytecodify_expression(snippet, last_used_reg, infix.left);
    reg rhs_result = bytecodify_expression(snippet, lhs_result, infix.right);

    reg_op_reg_instr instr = {
        .op1 = lhs_result,
        .op2 = rhs_result,
        .dest = rhs_result + 1,
    };

    // TODO this currently assumes that every operator is directly mappable to
    // single instruction. Perhaps an operator (say struct field access)
    // requires add then load.
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

bytecode_snippet bytecodify_top_level_declaration(declaration_node declaration) {
    if (!string_equals_cstring(declaration.data_type, "int")) {
        panic("currently assuming TLDs are ints");
    }

    bytecode_snippet snippet = { 0 };
    array_init(&snippet, sizeof(instruction));

    reg result_reg = bytecodify_expression(&snippet, REGISTER_INVALID, declaration.expression);
    // compiler_printf("bytecodified a TLD ('%S'), its result is in t%d\n", declaration.identifier, result_reg);
    (void)result_reg;

    return snippet;
}

// NOTE(sam) Placeholder until out-of-order compilation exists
void bytecodify_entire_program(statement_node *first_statement) {

    // NOTE(sam) Eventually will store bytecode snippets
    // bytecode_program program;

    statement_node *current = first_statement;
    while (current != NULL) {
        if (current->kind != DECLARATION) {
            panic("currently assuming every top level statement is a declaration, got %d", current->kind);
        }

        cprintf("\n");
        pretty_print_statement(0, current);
        cprintf("=========================");
        cprintf(" -> BYTECODE -> ");
        cprintf("=========================\n");

        bytecode_snippet snippet = bytecodify_top_level_declaration(current->value.declaration);
        debug_print_bytecode_snippet(&snippet);
        // NOTE(sam) Eventually will store bytecode snippets
        // (but for now only printing)

        current = current->next;
    }
}
