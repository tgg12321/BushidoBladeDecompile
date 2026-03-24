glabel func_8007E11C
    /* 6E91C 8007E11C 00F08448 */  mtc2       $a0, $30 /* handwritten instruction */
    /* 6E920 8007E120 00000000 */  nop
    /* 6E924 8007E124 00000000 */  nop
    /* 6E928 8007E128 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6E92C 8007E12C 20000124 */  addiu      $at, $zero, 0x20
    /* 6E930 8007E130 19004110 */  beq        $v0, $at, .L8007E198
    /* 6E934 8007E134 00000000 */   nop
    /* 6E938 8007E138 01004830 */  andi       $t0, $v0, 0x1
    /* 6E93C 8007E13C FEFF0A24 */  addiu      $t2, $zero, -0x2
    /* 6E940 8007E140 24504A00 */  and        $t2, $v0, $t2
    /* 6E944 8007E144 1F000924 */  addiu      $t1, $zero, 0x1F
    /* 6E948 8007E148 22482A01 */  sub        $t1, $t1, $t2 /* handwritten instruction */
    /* 6E94C 8007E14C 43480900 */  sra        $t1, $t1, 1
    /* 6E950 8007E150 E8FF4B21 */  addi       $t3, $t2, -0x18 /* handwritten instruction */
    /* 6E954 8007E154 03006005 */  bltz       $t3, .L8007E164
    /* 6E958 8007E158 00000000 */   nop
    /* 6E95C 8007E15C 04606401 */  sllv       $t4, $a0, $t3
    /* 6E960 8007E160 03000010 */  b          .L8007E170
  .L8007E164:
    /* 6E964 8007E164 18000B24 */   addiu     $t3, $zero, 0x18
    /* 6E968 8007E168 22586A01 */  sub        $t3, $t3, $t2 /* handwritten instruction */
    /* 6E96C 8007E16C 07606401 */  srav       $t4, $a0, $t3
  .L8007E170:
    /* 6E970 8007E170 C0FF8C21 */  addi       $t4, $t4, -0x40 /* handwritten instruction */
    /* 6E974 8007E174 40600C00 */  sll        $t4, $t4, 1
    /* 6E978 8007E178 0A800D3C */  lui        $t5, %hi(D_8009C7A8)
    /* 6E97C 8007E17C 2168AC01 */  addu       $t5, $t5, $t4
    /* 6E980 8007E180 A8C7AD85 */  lh         $t5, %lo(D_8009C7A8)($t5)
    /* 6E984 8007E184 00000000 */  nop
    /* 6E988 8007E188 04682D01 */  sllv       $t5, $t5, $t1
    /* 6E98C 8007E18C 02130D00 */  srl        $v0, $t5, 12
    /* 6E990 8007E190 0800E003 */  jr         $ra
    /* 6E994 8007E194 00000000 */   nop
  .L8007E198:
    /* 6E998 8007E198 0800E003 */  jr         $ra
    /* 6E99C 8007E19C 00000224 */   addiu     $v0, $zero, 0x0
endlabel func_8007E11C
