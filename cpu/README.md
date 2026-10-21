# cpu

Current goal is a scalar, pipelined rv32i. in vhdl for now.

## the pipeline

1. instruction fetch
2. decode
3. register file
4. ALU
5. writeback

## notes

- [RV32I Base Integer Instruction Set, Version 2.1](docs.riscv.org/reference/isa/v20260120/unpriv/rv32.html)
- [Architectures of Processor Systems](https://mpsu.github.io/APS/en/Introduction/index.html)
- [Comprehensive guide to 32-bit risc-v assembly programming](https://gist.github.com/robert-saramet/1b9ef3cac5a8345c90d84ac1ac4a8d2b)
- [Chapter 2 RV32I base integer instruction set](https://www.cse.cuhk.edu.hk/~byu/CENG3420/2025Spring/doc/RV32-reference-2.pdf)

### risc-v

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

There are two more variants for handling immediates: B (branch) and J (jump and link).

- B [31 imm12 31] [30 imm10-5 25] [24 rs2   20] [19 rs1 15] [14 funct3 12] [11 rd 7] [6 op 0]
- J [31 imm20 31] [30 imm10-1 21] [20 imm11 20] [19 imm19-12           12] [11 rd 7] [6 op 0]

Some Notes:

- the sign bit for every immediate is bit[31], to help make fan-out hardware with sign-extension simpler.
- To differentiate between compressed and non-compressed, the two least significant bits will be 11 for non-compressed, and anything else tells the processor its a compressed 16bit instruction.
- slli, srli and srai use a slightly modified I encoding format. 5 bits are needed to encode 31, which is the maximum before the same result occurs. For these instructions, bits 31-25 are treated as the funct7 field.

#### bus

- wishbone master/slave bus
- axi (look at zipcpu)

### Hardware testing

- self testing
- scan chains
- jtag tap
- formal verification

TODO: have a testgen library for generating inputs and expected outputs, and have a generic test vector controller harness. This would be specifically for smaller parts of the chip, parts that I can't verify with spike. It would be useful for it to be recoverable also, so that i can see all the bugs at the same time.

TODO: Once the first version of the core is done, must make sure to write benchmark gathering scripts.

Have "debug" hardware that shows on waveforms what instruction is executing at a certain stage/unit. Have a decent, generated waveform setup so you don't have to go picking around at signals trying to find stuff.

Maybe, have in_signal, out_signal, debug_signal (these don't get read from, so get optimized out. just for looking at waveforms)

#### links

- [Spike](https://github.com/riscv-software-src/riscv-isa-sim)

#### verification reading

- Chen, Challenges and Trends in Modern SoC Design Verification,
- Wilcox, Professional Verification
- Andrews, Co-Verification of Hardware and Software for ARM SoC Design
- Piziali, Functional Verification, Coverage Measurement and Analysis
- Tasiran, Coverage Metrics for the Functional Validation of Hardware
- Foster, Assertion-Based Design

For testing, write a testbench for each entity in python + cocotb.
Use spike to verify the core.
