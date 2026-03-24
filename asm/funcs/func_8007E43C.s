glabel func_8007E43C
    /* 6EC3C 8007E43C 00F08448 */  mtc2       $a0, $30 /* handwritten instruction */
    /* 6EC40 8007E440 00000000 */  nop
    /* 6EC44 8007E444 00000000 */  nop
    /* 6EC48 8007E448 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6EC4C 8007E44C 20000124 */  addiu      $at, $zero, 0x20
    /* 6EC50 8007E450 1D004110 */  beq        $v0, $at, .L8007E4C8
    /* 6EC54 8007E454 00000000 */   nop
    /* 6EC58 8007E458 01004830 */  andi       $t0, $v0, 0x1
    /* 6EC5C 8007E45C FEFF0A24 */  addiu      $t2, $zero, -0x2
    /* 6EC60 8007E460 24504A00 */  and        $t2, $v0, $t2
    /* 6EC64 8007E464 13000924 */  addiu      $t1, $zero, 0x13
    /* 6EC68 8007E468 22482A01 */  sub        $t1, $t1, $t2 /* handwritten instruction */
    /* 6EC6C 8007E46C 43480900 */  sra        $t1, $t1, 1
    /* 6EC70 8007E470 E8FF4B21 */  addi       $t3, $t2, -0x18 /* handwritten instruction */
    /* 6EC74 8007E474 03006005 */  bltz       $t3, .L8007E484
    /* 6EC78 8007E478 00000000 */   nop
    /* 6EC7C 8007E47C 04606401 */  sllv       $t4, $a0, $t3
    /* 6EC80 8007E480 03000010 */  b          .L8007E490
  .L8007E484:
    /* 6EC84 8007E484 18000B24 */   addiu     $t3, $zero, 0x18
    /* 6EC88 8007E488 22586A01 */  sub        $t3, $t3, $t2 /* handwritten instruction */
    /* 6EC8C 8007E48C 07606401 */  srav       $t4, $a0, $t3
  .L8007E490:
    /* 6EC90 8007E490 C0FF8C21 */  addi       $t4, $t4, -0x40 /* handwritten instruction */
    /* 6EC94 8007E494 40600C00 */  sll        $t4, $t4, 1
    /* 6EC98 8007E498 0A800D3C */  lui        $t5, %hi(D_8009C7A8)
    /* 6EC9C 8007E49C 2168AC01 */  addu       $t5, $t5, $t4
    /* 6ECA0 8007E4A0 A8C7AD85 */  lh         $t5, %lo(D_8009C7A8)($t5)
    /* 6ECA4 8007E4A4 00000000 */  nop
    /* 6ECA8 8007E4A8 04002005 */  bltz       $t1, .L8007E4BC
    /* 6ECAC 8007E4AC 00000000 */   nop
    /* 6ECB0 8007E4B0 04102D01 */  sllv       $v0, $t5, $t1
    /* 6ECB4 8007E4B4 0800E003 */  jr         $ra
    /* 6ECB8 8007E4B8 00000000 */   nop
  .L8007E4BC:
    /* 6ECBC 8007E4BC 22480900 */  neg        $t1, $t1 /* handwritten instruction */
    /* 6ECC0 8007E4C0 0800E003 */  jr         $ra
    /* 6ECC4 8007E4C4 06102D01 */   srlv      $v0, $t5, $t1
  .L8007E4C8:
    /* 6ECC8 8007E4C8 0800E003 */  jr         $ra
    /* 6ECCC 8007E4CC 00000224 */   addiu     $v0, $zero, 0x0
endlabel func_8007E43C
