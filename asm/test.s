# This should parse correctly.

;;
        addi x0, zero, 1 ;; lw x1, 0x2(x1)
# 2
_label0:addi x1, ra, 0x00;
        lw x2, 0x40(sp)
        addi x3, gp, 5;
        sra x4, tp, tp
        addi x5, t0, 7;
# 7
l_abel1:
        addi x6, t1, 8;
        beq x7, t2, label3_ # 8
        addi x8, s0, -1;
        sra x8, fp, s0;
        addi x9, s1, -2
        sh x10, -3(a0);
        slli x11, a1, -4 # this is another comment
        auipc a2, -5
        slli x13, a3, -6;
        addi x14, a4, 0x7fF;
        ecall
  # another comment
        beq x15, a5, l_abel1; # 17
        addi x16, a6, -9
        addi x17, a7, -10;addi x18, s2, -11
        addi x19, s3, 10 # this is some comment
        slli x20, s4, 2047
        addi x21, s5, -2048; addi x22, s6, -100
# 25
label3_: # last comment
        addi x23, s7, -10;
        beq x24, s8, label4 # 26
        jal x25, label4;
        sra x26, s10, s10;
# 29
label4:  addi x27, s11, 3 ;addi x28, t3, 4;
        addi x29, t4, 4
        beq x30, t5, _label0; # 32
        lw x31, 4(t6)
