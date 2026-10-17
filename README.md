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

language features:
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

future
- typecheck
- bytecode generation
- bytecode interpreter?? 👀😎
- optimization
- machine code output
