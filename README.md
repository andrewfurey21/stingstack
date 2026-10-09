# full stack project

* rv32ima cpu + mmu (boot linux)
* c compiler: lexer, recursive descent parser, semantic checks, SSA, codegen
* assembler
* linker
* libc
* os
* shell over uart + renderer
* text editor

Goal: render a shell over uart, edit and execute programs on an rv32i chip synthesized onto an fpga.

## links

- werwolv.net/posts/linux_bringup
- csmith and yarpgen

## features I would add

- f strings would be handy.
- namespaces
- zig comptime or similar for some sort of generic support
- methods, but just syntax sugar maybe. idk.
