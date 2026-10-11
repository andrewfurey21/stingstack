# cpu

Current goal is a scalar, pipelined rv32i. in vhdl for now.

## current pipeline

1. instruction fetch
2. decode
3. register file
4. ALU
5. writeback

## notes

- [Architectures of Processor Systems](https://mpsu.github.io/APS/en/Introduction/index.html)
- [Comprehensive guide to 32-bit risc-v assembly programming](https://gist.github.com/robert-saramet/1b9ef3cac5a8345c90d84ac1ac4a8d2b)
- [Chapter 2 RV32I base integer instruction set](https://www.cse.cuhk.edu.hk/~byu/CENG3420/2025Spring/doc/RV32-reference-2.pdf)

### risc-v instructions

There are 32 registers x0-x31 (32 in total, except x0 is hardcoded to 0).
There is no fixed stack pointer or return address register in rv32i.
The standard calling convention uses x1 as the return address and x2 for
the stack pointer. x5 can be used as an alternative return address.

The program counter is a seperate register that is not directly accessible.
Modify it with branches and `auipc`.

#### rv32i encodings

There are four core formats: R (register), I (immediate), S (store), U (upper immediate).
32-bits and must be aligned on a 4-byte boundary. If incorrectly aligned, an instruction-address-misaligned exception is generated.

rs1, rs2 and rd register are at the same position in all formats. Immediates
except for CSR instructions are sign extended.

- R [31 funct7 25] [24 rs2 20] [19 rs1 15] [14 funct3 12] [11 rd  7] [6 op 0]
- I [31 imm                20] [19 rs1 15] [14 funct3 12] [11 rd  7] [6 op 0]
- S [31 imm    25] [24 rs2 20] [19 rs1 15] [14 funct3 12] [11 imm 7] [6 op 0]
- U [31 imm                                           12] [11 rd  7] [6 op 0]

funct7 and funct3 choose the specific operation to perform depending on op.

There are two more varaints for handling immediates: B (branch) and J (jump and link).

- B [31 imm12 31] [30 imm10-5 25] [24 rs2   20] [19 rs1 15] [14 funct3 12] [11 rd 7] [6 op 0]
- J [31 imm20 31] [30 imm10-1 21] [20 imm11 20] [19 imm19-12           12] [11 rd 7] [6 op 0]

Some Notes:

- the sign bit for every immediate is bit[31], to help with sign-extension.
- To differentiate between compressed and non-compressed, the two least significant bits will be 11 for non-compressed, and anything else tells the processor its a compressed 16bit instruction.
- slli, srli and srai use a slightly modified I encoding format. 5 bits are needed to encode 31, which is the maximum before the same result occurs. For these instructions, bits 31-25 are treated as the funct7 field.
