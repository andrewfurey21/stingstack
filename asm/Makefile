all:
	gcc main.c -o asm -g -O0 -std=c11

test:
	gcc main.c -S -o main.s -O3 -fno-asynchronous-unwind-tables -fno-unwind-tables -masm=intel

riscv:
	riscv64-linux-gnu-gcc main.c -S -o main.s -O3 -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-stack-protector -fno-builtin -fno-pic -fno-pie -march=rv64g

# bare:
# 	riscv64-unknown-elf-gcc -O3 -fno-asynchronous-unwind-tables -fno-unwind-tables -march=rv32i -mabi=ilp32 -static main.c -S -o main.s

clean:
	rm asm

.PHONY: all clean
