// Convert top level statement into snippets of a bytecode program, then
// possibly stitch them together

typedef uint64_t reg;
const reg REGISTER_INVALID = 0;

typedef uint32_t instruction_id;

typedef struct immediate_s32_instr {
    reg dest;
    int32_t the_s32;
} immediate_s32_instr;

typedef struct instruction {
    enum {
        IMMEDIATE_S32,
    } kind; // perhaps should be "opcode"
    union {
        immediate_s32_instr immediate_s32;
    } value;
} instruction;

typedef array bytecode_snippet;
instruction get_instruction(bytecode_snippet *snippet, int i) {
    return *(instruction *)array_get(snippet, i);
}
void append_instruction(bytecode_snippet *snippet, instruction instr) {
    array_append(snippet, &instr);
}

// NOTE reg will be passed around now. no more state
// reg new_reg(bytecode_snippet *snippet) {
//     snippet->last_used_reg += 1;
//     return snippet->last_used_reg;
// }

void debug_print_bytecode_snippet(bytecode_snippet *snippet) {
    cprintf("::: Printing snippet\n");
    for (uint64_t i = 0; i < snippet->length; i++) {
        instruction instr = get_instruction(snippet, i);
        switch (instr.kind) {
            case IMMEDIATE_S32:
                printf("%ld  |  t%ld = %d\n", i, instr.value.immediate_s32.dest, instr.value.immediate_s32.the_s32);
            break;
        }
    }
    cprintf("::: end print snippet\n");
}

////////////////////////////////////////////////////////////////////////////////

// Each bytecodify_* function generates some intermediate registers, and stores
// the final result in a register. Return the result register so the caller
// knows where the result is.

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
        // case INFIX:
        //     break;
        default:
            panic("unhandled expression kind=%d in bytecodify_expression", expression->kind);
    }

    return REGISTER_INVALID;
}

bytecode_snippet bytecodify_top_level_declaration(declaration_node declaration) {
    if (!string_equals_cstring(declaration.data_type, "int")) {
        panic("currently assuming TLDs are ints that are evaluable at comptime");
    }

    bytecode_snippet snippet = { 0 };
    array_init(&snippet, sizeof(instruction));

    reg result_reg = bytecodify_expression(&snippet, REGISTER_INVALID, declaration.expression);
    compiler_printf("bytecodified a TLD ('%S'), its result is in t%d\n", declaration.identifier, result_reg);

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

        bytecode_snippet snippet = bytecodify_top_level_declaration(current->value.declaration);
        debug_print_bytecode_snippet(&snippet);
        // NOTE(sam) Eventually will store bytecode snippets
        // (but for now only printing)

        current = current->next;
    }
}
