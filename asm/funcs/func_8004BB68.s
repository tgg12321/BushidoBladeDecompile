glabel func_8004BB68
    /* 3C368 8004BB68 21008014 */  bnez       $a0, .L8004BBF0
    /* 3C36C 8004BB6C 00000000 */   nop
    /* 3C370 8004BB70 080060CA */  lwc2       $0, 0x8($s3)
    /* 3C374 8004BB74 0C0061CA */  lwc2       $1, 0xC($s3)
    /* 3C378 8004BB78 100062CA */  lwc2       $2, 0x10($s3)
    /* 3C37C 8004BB7C 140063CA */  lwc2       $3, 0x14($s3)
    /* 3C380 8004BB80 180064CA */  lwc2       $4, 0x18($s3)
    /* 3C384 8004BB84 1C0065CA */  lwc2       $5, 0x1C($s3)
    /* 3C388 8004BB88 00000000 */  nop
    /* 3C38C 8004BB8C 00000000 */  nop
    /* 3C390 8004BB90 3000284A */  rtpt
    /* 3C394 8004BB94 CC01EC96 */  lhu        $t4, 0x1CC($s7)
    /* 3C398 8004BB98 CE01ED96 */  lhu        $t5, 0x1CE($s7)
    /* 3C39C 8004BB9C 0E006896 */  lhu        $t0, 0xE($s3)
    /* 3C3A0 8004BBA0 16006996 */  lhu        $t1, 0x16($s3)
    /* 3C3A4 8004BBA4 1E006A96 */  lhu        $t2, 0x1E($s3)
    /* 3C3A8 8004BBA8 00640C00 */  sll        $t4, $t4, 16
    /* 3C3AC 8004BBAC 006C0D00 */  sll        $t5, $t5, 16
    /* 3C3B0 8004BBB0 25400C01 */  or         $t0, $t0, $t4
    /* 3C3B4 8004BBB4 25482D01 */  or         $t1, $t1, $t5
    /* 3C3B8 8004BBB8 0C0088AE */  sw         $t0, 0xC($s4)
    /* 3C3BC 8004BBBC 140089AE */  sw         $t1, 0x14($s4)
    /* 3C3C0 8004BBC0 1C008AA6 */  sh         $t2, 0x1C($s4)
    /* 3C3C4 8004BBC4 08008CEA */  swc2       $12, 0x8($s4)
    /* 3C3C8 8004BBC8 10008DEA */  swc2       $13, 0x10($s4)
    /* 3C3CC 8004BBCC 18008EEA */  swc2       $14, 0x18($s4)
    /* 3C3D0 8004BBD0 D001E88E */  lw         $t0, 0x1D0($s7)
    /* 3C3D4 8004BBD4 000091AE */  sw         $s1, 0x0($s4)
    /* 3C3D8 8004BBD8 040088AE */  sw         $t0, 0x4($s4)
    /* 3C3DC 8004BBDC 0007093C */  lui        $t1, (0x7000000 >> 16)
    /* 3C3E0 8004BBE0 24889E02 */  and        $s1, $s4, $fp
    /* 3C3E4 8004BBE4 25882902 */  or         $s1, $s1, $t1
    /* 3C3E8 8004BBE8 0800E003 */  jr         $ra
    /* 3C3EC 8004BBEC 20009422 */   addi      $s4, $s4, 0x20 /* handwritten instruction */
  .L8004BBF0:
    /* 3C3F0 8004BBF0 28007322 */  addi       $s3, $s3, 0x28 /* handwritten instruction */
    /* 3C3F4 8004BBF4 FFFF8420 */  addi       $a0, $a0, -0x1 /* handwritten instruction */
    /* 3C3F8 8004BBF8 00007FAE */  sw         $ra, 0x0($s3)
    /* 3C3FC 8004BBFC 040064A6 */  sh         $a0, 0x4($s3)
    /* 3C400 8004BC00 E0FF688E */  lw         $t0, -0x20($s3)
    /* 3C404 8004BC04 E4FF698E */  lw         $t1, -0x1C($s3)
    /* 3C408 8004BC08 080068AE */  sw         $t0, 0x8($s3)
    /* 3C40C 8004BC0C 0C0069AE */  sw         $t1, 0xC($s3)
    /* 3C410 8004BC10 E0FF6422 */  addi       $a0, $s3, -0x20 /* handwritten instruction */
    /* 3C414 8004BC14 E8FF6522 */  addi       $a1, $s3, -0x18 /* handwritten instruction */
    /* 3C418 8004BC18 E230010C */  jal        func_8004C388
    /* 3C41C 8004BC1C 10006622 */   addi      $a2, $s3, 0x10 /* handwritten instruction */
    /* 3C420 8004BC20 E0FF6422 */  addi       $a0, $s3, -0x20 /* handwritten instruction */
    /* 3C424 8004BC24 F0FF6522 */  addi       $a1, $s3, -0x10 /* handwritten instruction */
    /* 3C428 8004BC28 E230010C */  jal        func_8004C388
    /* 3C42C 8004BC2C 18006622 */   addi      $a2, $s3, 0x18 /* handwritten instruction */
    /* 3C430 8004BC30 04006486 */  lh         $a0, 0x4($s3)
    /* 3C434 8004BC34 DA2E010C */  jal        func_8004BB68
    /* 3C438 8004BC38 00000000 */   nop
    /* 3C43C 8004BC3C E8FF6422 */  addi       $a0, $s3, -0x18 /* handwritten instruction */
    /* 3C440 8004BC40 F0FF6522 */  addi       $a1, $s3, -0x10 /* handwritten instruction */
    /* 3C444 8004BC44 E230010C */  jal        func_8004C388
    /* 3C448 8004BC48 08006622 */   addi      $a2, $s3, 0x8 /* handwritten instruction */
    /* 3C44C 8004BC4C 04006486 */  lh         $a0, 0x4($s3)
    /* 3C450 8004BC50 DA2E010C */  jal        func_8004BB68
    /* 3C454 8004BC54 00000000 */   nop
    /* 3C458 8004BC58 1000688E */  lw         $t0, 0x10($s3)
    /* 3C45C 8004BC5C 1400698E */  lw         $t1, 0x14($s3)
    /* 3C460 8004BC60 200068AE */  sw         $t0, 0x20($s3)
    /* 3C464 8004BC64 240069AE */  sw         $t1, 0x24($s3)
    /* 3C468 8004BC68 F0FF688E */  lw         $t0, -0x10($s3)
    /* 3C46C 8004BC6C F4FF698E */  lw         $t1, -0xC($s3)
    /* 3C470 8004BC70 100068AE */  sw         $t0, 0x10($s3)
    /* 3C474 8004BC74 140069AE */  sw         $t1, 0x14($s3)
    /* 3C478 8004BC78 04006486 */  lh         $a0, 0x4($s3)
    /* 3C47C 8004BC7C DA2E010C */  jal        func_8004BB68
    /* 3C480 8004BC80 00000000 */   nop
    /* 3C484 8004BC84 2000688E */  lw         $t0, 0x20($s3)
    /* 3C488 8004BC88 2400698E */  lw         $t1, 0x24($s3)
    /* 3C48C 8004BC8C 100068AE */  sw         $t0, 0x10($s3)
    /* 3C490 8004BC90 140069AE */  sw         $t1, 0x14($s3)
    /* 3C494 8004BC94 E8FF688E */  lw         $t0, -0x18($s3)
    /* 3C498 8004BC98 ECFF698E */  lw         $t1, -0x14($s3)
    /* 3C49C 8004BC9C 180068AE */  sw         $t0, 0x18($s3)
    /* 3C4A0 8004BCA0 1C0069AE */  sw         $t1, 0x1C($s3)
    /* 3C4A4 8004BCA4 04006486 */  lh         $a0, 0x4($s3)
    /* 3C4A8 8004BCA8 DA2E010C */  jal        func_8004BB68
    /* 3C4AC 8004BCAC 00000000 */   nop
    /* 3C4B0 8004BCB0 00007F8E */  lw         $ra, 0x0($s3)
    /* 3C4B4 8004BCB4 00000000 */  nop
    /* 3C4B8 8004BCB8 0800E003 */  jr         $ra
    /* 3C4BC 8004BCBC D8FF7322 */   addi      $s3, $s3, -0x28 /* handwritten instruction */
endlabel func_8004BB68
