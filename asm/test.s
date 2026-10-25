# This should parse correctly.
        addi x0, zero, 1 ;; addi x1, x1, 0x2
_label0:addi x1, ra, 0x00;
        addi x2, sp, 0x40
        addi x3, gp, 5;
        addi x4, tp, 6
        addi x5, t0, 7;
l_abel1:
        addi x6, t1, 8;
        addi x7, t2, 9
        addi x8, s0, -1;
        addi x8, fp, -1;
        addi x9, s1, -2
        addi x10, a0, -3;
        addi x11, a1, -4 # this is another comment
        addi x12, a2, -5
        addi x13, a3, -6;
        addi x14, a4, 0x7fF;
  # another comment
        addi x15, a5, -8;
        addi x16, a6, -9
        addi x17, a7, -10;addi x18, s2, -11
        addi x19, s3, 10 # this is some comment
        addi x20, s4, 2047
        addi x21, s5, -2048; addi x22, s6, -100
label3_: # last comment
        addi x23, s7, -10;
        addi x24, s8, -1
        addi x25, s9, 1;
        addi x26, s10, 2;
label4:  addi x27, s11, 3 ;addi x28, t3, 4;
        addi x29, t4, 4
        addi x30, t5, 4;
        addi x31, t6, 4
