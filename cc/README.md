# c compiler

- llvm
- mlir
- tcc
- qbe
- cranelift
- 8cc
- booth
- cuik
- https://codeberg.org/thesalt/lance
- github.com/hexratcc/rat

## things to look into

- ssa, sea of nodes
- llvm ir, mlir, qbe

## links

- csmith and yarpgen

## features I would add

- `#run func_call()`, like jai, convert to bytecode and run at compile time.
- methods
- simple stack trace in debug mode with a static error object.
- typesafe enums, but type inferred too. like .VALUE
- how hard is it to know if value is undefined? at compile time, or force mark as undefined like zig?
- defer
- default params
- really nice inline assembly. maybe look at odin?
- int x = x in a lower scope should be banned.
- whats a nice way to printf with compile time type checking and customizeable based on the struct.
