const uint32_t INDENT_AMOUNT = 4;

void pretty_print_statement(uint32_t num_spaces, statement_node *statement);
void pretty_print_block(uint32_t num_spaces, statement_node *statement);

void print_spaces(uint32_t num_spaces) {
    for (uint32_t i = 0; i < num_spaces; i++) {
        printf(" ");
    }
}

void pretty_print_expression(uint32_t num_spaces, expression_node *expression) {
    switch (expression->kind) {
        case INFIX:
            pretty_print_expression(num_spaces, expression->value.infix.left);
            compiler_printf(" %T ", expression->value.infix.operator);
            pretty_print_expression(num_spaces, expression->value.infix.right);
            break;
        case INT:
            printf("%d", expression->value.integer);
            break;
        case IDENTIFIER:
            print_string(expression->value.identifier);
            break;
        case BOOL:
            if (expression->value.bool == true) {
                printf("true");
            } else if (expression->value.bool == false) {
                printf("false");
            } else {
                panic("Somehow a bool expression got a nonboolean value");
            }
            break;
        case INLINE_FUNCTION:
            printf("func(");
            // print arguments here
            printf(") ");
            // print return type here
            printf("{\n");
            // print function body here
            pretty_print_block(num_spaces + INDENT_AMOUNT, expression->value.function_declaration.body);
            printf("}");
            break;
        default:
            panic("unhandled expression kind (%d) to print", expression->kind);
    }
}

void pretty_print_block(uint32_t num_spaces, statement_node *statement) {
    while (statement != NULL) {
        pretty_print_statement(num_spaces, statement);
        statement = statement->next;
    }
}

void pretty_print_statement(uint32_t num_spaces, statement_node *statement) {
    print_spaces(num_spaces);
    switch (statement->kind) {
        case DECLARATION:
            print_string(statement->value.declaration.identifier);
            printf(": ");
            print_string(statement->value.declaration.data_type);
            printf(" = ");
            pretty_print_expression(num_spaces, statement->value.declaration.expression);
            break;
        case ASSIGNMENT:
            print_string(statement->value.assignment.lhs);
            printf(" = ");
            pretty_print_expression(num_spaces, statement->value.assignment.expression);
            break;
        case RETURN:
            printf("return");
            if (statement->value.maybe_return_expression != NULL) {
                printf(" ");
                pretty_print_expression(num_spaces, statement->value.maybe_return_expression);
            }
            break;
        case WHILE_LOOP:
            printf("while ");
            pretty_print_expression(num_spaces, statement->value.while_loop.condition);
            printf(" {\n");
            pretty_print_block(num_spaces + INDENT_AMOUNT, statement->value.while_loop.body);
            print_spaces(num_spaces);
            printf("}");
            break;
        default:
            panic("unhandled statement kind (%d) to print", statement->kind);
    }
    printf("\n");
}

void pretty_print_entire_file(statement_node *statement) {
    pretty_print_block(0, statement);
}
