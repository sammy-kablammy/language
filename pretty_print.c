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
        case BOOL:
            if (expression->value.bool == true) {
                printf("true");
            } else if (expression->value.bool == false) {
                printf("false");
            } else {
                panic("Somehow a bool expression got a nonboolean value");
            }
            break;
        default:
            panic("unhandled expression kind (%d) to print", expression->kind);
    }
}

void pretty_print_statement(statement_node *statement) {
    switch (statement->kind) {
        case DECLARATION:
            print_string(statement->value.declaration.identifier);
            printf(": ");
            print_string(statement->value.declaration.data_type);
            printf(" = ");
            pretty_print_expression(statement->value.declaration.expression);
            break;
        case ASSIGNMENT:
            print_string(statement->value.assignment.lhs);
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

void pretty_print_entire_file(statement_node *statement) {
    while (statement != NULL) {
        pretty_print_statement(statement);
        statement = statement->next;
    }
}
