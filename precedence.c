// How to parse infix expressions with correct operator precedence
// (this file in progress)

// (For example we want * to be lower in the tree than + because * should be
// evaluated first.)

// First consider two possible implementations of parse expression...

////////////////////////////////////////////////////////////////////////////////

parse_expression() {
    left = parse_leaf(); // "leaf" is identifier or integer or something

    if current token is not operator {
        // turns out this "expression" was only a single leaf :/
        return left;
    }
    // there is more than a single leaf

    operator = current token;
    current token position += 1;

    right = parse_expression();
}

parse_expression() {
    expression left = parse_leaf(); // "leaf" is identifier or integer or something

    while current token is operator {
        operator = current token;
        current token position += 1;

        expression right = parse_leaf();

        // The expression as parsed up until now gets pushed "down" one level in the tree
        infix_expression parent = { left, operator, right };
        left = parent;
    }

    return left
}

////////////////////////////////////////////////////////////////////////////////

// The first/recursive implementation gives right-leaning trees:
//     expr op (expr op (expr op (expr op ...)))
//
// When an expression consists of increasing precedence like a + b * c,
// right-leaning parses correctly:
//     a + b * c    gets parsed with this precedence: a + (b * c)
//                                                    L   RRRRRRR
//     a * b + c    gets parsed with this precedence: a * (b + c)
//                                                    L   RRRRRRR


// The second/iterative implementation gives left-leaning trees:
//     (((... op expr) op expr) op expr) op expr
//
// When an expression consists of decreasing precedence like a * b + c,
// left-leaning parses correctly:
//     a + b * c    gets parsed with this precedence: (a + b) * c
//                                                    LLLLLLL   R
//     a * b + c    gets parsed with this precedence: (a * b) + c
//                                                    LLLLLLL   R


// (When an expression consists of the same precedence like a + a + a,
// either strategy is correct.)

////////////////////////////////////////////////////////////////////////////////

// Also see Sam H Smith parsing video, iirc it's basically this same explanation
// https://www.youtube.com/watch?v=WdlXBDHXqAs
