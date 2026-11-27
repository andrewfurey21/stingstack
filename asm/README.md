# rv32i assembler

directives:
- file "file_name" (just one for now.)
- .option (rcv/norvc, pic/nopic, relax/norelax, push/pop, arch,+ext/arch, -ext/arch, =ISA, csr-check/no-csr-check)
- (string, asciz, ascii) "string"
- .attribute <tag>, <value>
- zero expression (also dword, word, quad etc)
- section name, "flags", @type, entsize (bss, data, text, rodata)
- align expression
- type label, @type (function, object, look at elf.h 597)
- size label, expression
- global/globl label
- word <possible label/something?>
- ident "string"

other stuff:
- %hi(label), %lo(label)
- . is the location counter, returns bytes from start of current section

labels:
- [a-zA-z$_.]*

TOKENS:
- instruction
- register
- integer literal
- string literal
- operator (+ - * / << >> & | ^ ~)
- punctuation (, :, ;, ())
- newline
- label
- . location counter
- @ident
- %ident

parser:
1. build up sections. store location of labels when you reach a label.
2. evaluate expressions with label addresses fixed from 1st pass.
3. output each section into an elf.

When parsing, may need a function that takes variable length tokens to match.

## other notes

- .section .data and .data are the same
- anywhere a number can be, an expression can be.
- the same section can be split up in the assembly file.

## links I read

- [fasm](https://flatassembler.net/download.php)
- [RISC-V Assembly Programmers Manual](https://zju-os.github.io/doc/spec/riscv-asm.pdf)
- [Baremetal x86 assembler](https://github.com/hexratcc/baremetal)
