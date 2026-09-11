# todo

## phase 1

    file -> token_array -> syntax_tree_node[] -> pretty_print(syntax_tree_node)
                           ^
         one syntax tree per top-level declaration
       this does mean top level declarations are unordered / must be const

- [x] basic lexing of entire file
- [ ] moar lexing
- [ ] parsing
- [ ] pretty printing
  - i think a roundtrip of source -> syntax tree -> pretty print is a decent way
    to test the parser early days

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

- [ ] correctly parse operator precedence. example:
```text
main :: func(argc: int) {
    thing1: int = 10 + 20 * 30
    thing2: int = 40 * 50 - 60
    thing3: int = 2 * (1 + 3 * (4 + 5))
}
```
