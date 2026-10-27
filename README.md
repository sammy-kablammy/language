# todo

    file -> token_array -> list of syntax tree nodes -> pretty_print(syntax_tree_node)
                           ^
         one syntax tree per top-level declaration
       this does mean top level declarations are unordered / must be const

sample thing we should be able to pretty print by the end of this:
```text
add :: func(a: int, b: int) {
    result: int = a + b
    return result
}

main :: func(argc: int) {
    x: int = argc
    y: int = sum(x, 10)
    z: int = y * 2
    return z
}
```

- [X] some proper test cases
- [X] CI / github actions
  - [x] compiler should compile
  - [X] tests should pass

the following TODOs are organized by compiler phase

language features to parse:
- [ ] string literal expressions
- [ ] more binary operators
  - [ ] logical `&&`, `||`
  - [ ] dot operator
- [ ] unary operators
  - [ ] logical `!`
  - [ ] negation `-`
- [ ] comments
- [ ] import keyword like `import "filename"`
- [ ] correctly parse operator precedence
  - [x] infix operators
  - [ ] handle parenthesized expressions like `a * (b + c)`
- [ ] function call expressions
  - [ ] array subscript expressions (use `[` instead of `(` for this)
- [ ] if statements
- [ ] while loops
  - [ ] consider for-loop
- [ ] function declaration expressions
- [ ] struct declarations

typechecking phase:
- [X] add builtin type (say bool), so there are at least two types to work with
- [X] typed declarations `a = b` vs `a: int = b`
- [ ] basic "does LHS match RHS"
- [ ] figure out numeric types. signed vs unsigned, floats. see go's untyped
  integer literals and zig's `comptime_int` for reference
- [ ] string literals
- [ ] type inference `who_knows := some_expression_here`
- [ ] first class types, builtin type called `type`
- [ ] casting, for inspiration see https://www.gingerbill.org/article/2026/02/23/designing-odins-casting-syntax/

future
- bytecode generation
- bytecode interpreter?? 👀😎
- optimization
- machine code output
