int evaluate_leaves(int left, int right, token operator);

int simulate_arithmetic(expression_node *e) {
	int left = 0;
	int right = 0;

    if (e->kind == INT) {
        return e->value.integer;
    }

	if (e->value.infix.left->kind == INFIX) {
		left = simulate_arithmetic(e->value.infix.left);
	}
	else if (e->value.infix.left->kind == INT) {
		left = e->value.infix.left->value.integer;
	}
    else {
        panic("Unrecognized expression kind in simulate");
    }

	if (e->value.infix.right->kind == INFIX) {
		right = simulate_arithmetic(e->value.infix.right);
	}
	else if (e->value.infix.right->kind == INT) {
		right = e->value.infix.right->value.integer;
	}
    else {
        panic("Unrecognized expression kind in simulate");
    }

	return evaluate_leaves(left, right, e->value.infix.operator);
}

int evaluate_leaves(int left, int right, token operator) {
	switch(operator.kind) {
		case TOKEN_PLUS:
			printf("%d + %d\n", left, right);
			return left + right;
		case TOKEN_MINUS:
			printf("%d - %d\n", left, right);
			return left - right;
		case TOKEN_STAR:
			printf("%d * %d\n", left, right);
			return left * right;
		case TOKEN_SLASH:
			printf("%d / %d\n", left, right);
			return left / right;
		default:
			panic("what happened\n");
	}
}
