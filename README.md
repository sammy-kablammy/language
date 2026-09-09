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
