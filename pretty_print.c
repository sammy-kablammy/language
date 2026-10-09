void pretty_print_expression(expression_node *expression) {
    switch (expression->kind) {
        case INFIX:
            pretty_print_expression(expression->value.infix.left);
            printf(" ");
            print_token(expression->value.infix.operator);
            // Note: print_token currently prints a space
            pretty_print_expression(expression->value.infix.right);
            break;
        case INT:
            printf("%d", expression->value.integer);
            break;
        case IDENTIFIER:
            print_string(expression->value.identifier);
            break;
    }
}

void pretty_print_statement(statement_node *statement) {
    switch (statement->kind) {
        case ASSIGNMENT:
            print_string(statement->value.assignment.identifier);
            printf(" = ");
            pretty_print_expression(statement->value.assignment.expression);
            break;
        case RETURN:
            printf("return");
            if (statement->value.maybe_return_expression != NULL) {
                printf(" ");
                pretty_print_expression(statement->value.maybe_return_expression);
            }
            break;
        default:
            panic("unhandled statement kind (%d) to print", statement->kind);
    }
    printf("\n");
}
