## Coding convention

### Naming
- Class names are nouns using [Pascal Case](https://pl.wikipedia.org/wiki/PascalCase) and give information about its intention
- Function names are verbs using [Camel Case](https://pl.wikipedia.org/wiki/CamelCase) and give information about its intention
- Variables names are using [Camel Case](https://pl.wikipedia.org/wiki/CamelCase) and give information about its intention
- Headers placed in `include/` and its subdirectories (for specific modules), with `.hpp` extension
- Source files placed in `src/` and same subdirectories as corresponding headers, with `.cpp` extension

### Namespaces
- It would be better not to use `using namespace` at all, but we can all agree on `using namespace std`
- Don't use `using...` for any other namespaces
  - to avoid naming conflicts
  - to give user a clear information where does particular function, class or variable come from

### Misc
- Lines should have at most 100 columns, then we break lines with proper indentation
- Opening `{` and closing `}` braces for scopes are stand-alone in separate lines
