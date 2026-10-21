
/*

Just one file!

*/

#include "../utils/utils.h"

/*

instructions seperated by new lines. comments with ;

1. map text to pseudoinstructions
2. map pseudoinstructions to 32bits instructions

- R [31 funct7 25] [24 rs2 20] [19 rs1 15] [14 funct3 12] [11 rd  7] [6 op 0]
- I [31 imm                20] [19 rs1 15] [14 funct3 12] [11 rd  7] [6 op 0]
- S [31 imm    25] [24 rs2 20] [19 rs1 15] [14 funct3 12] [11 imm 7] [6 op 0]
- U [31 imm                                           12] [11 rd  7] [6 op 0]
- B [31 imm12 31] [30 imm10-5 25] [24 rs2   20] [19 rs1 15] [14 funct3 12] [11 rd 7] [6 op 0]
- J [31 imm20 31] [30 imm10-1 21] [20 imm11 20] [19 imm19-12           12] [11 rd 7] [6 op 0]

*/

struct Instruction {

  enum Type {
    NOP,
    ADDI
  } type;

  // registers, immediate
};

i32 main() {
  const u32 size = 32 KB;
  u8 *memory = os_page_alloc(size);

  os_file_open("test.s", )



  os_page_dealloc(memory, size);
}
