glabel func_8004C1F4
    /* 3C9F4 8004C1F4 28008014 */  bnez       $a0, .L8004C298
    /* 3C9F8 8004C1F8 00000000 */   nop
    /* 3C9FC 8004C1FC 080060CA */  lwc2       $0, 0x8($s3)
    /* 3CA00 8004C200 0C0061CA */  lwc2       $1, 0xC($s3)
    /* 3CA04 8004C204 100062CA */  lwc2       $2, 0x10($s3)
    /* 3CA08 8004C208 140063CA */  lwc2       $3, 0x14($s3)
    /* 3CA0C 8004C20C 180064CA */  lwc2       $4, 0x18($s3)
    /* 3CA10 8004C210 1C0065CA */  lwc2       $5, 0x1C($s3)
    /* 3CA14 8004C214 00000000 */  nop
    /* 3CA18 8004C218 00000000 */  nop
    /* 3CA1C 8004C21C 3000284A */  rtpt
    /* 3CA20 8004C220 CC01EC96 */  lhu        $t4, 0x1CC($s7)
    /* 3CA24 8004C224 CE01ED96 */  lhu        $t5, 0x1CE($s7)
    /* 3CA28 8004C228 0E006896 */  lhu        $t0, 0xE($s3)
    /* 3CA2C 8004C22C 16006996 */  lhu        $t1, 0x16($s3)
    /* 3CA30 8004C230 1E006A96 */  lhu        $t2, 0x1E($s3)
    /* 3CA34 8004C234 26006B96 */  lhu        $t3, 0x26($s3)
    /* 3CA38 8004C238 00640C00 */  sll        $t4, $t4, 16
    /* 3CA3C 8004C23C 006C0D00 */  sll        $t5, $t5, 16
    /* 3CA40 8004C240 25400C01 */  or         $t0, $t0, $t4
    /* 3CA44 8004C244 25482D01 */  or         $t1, $t1, $t5
    /* 3CA48 8004C248 0C0088AE */  sw         $t0, 0xC($s4)
    /* 3CA4C 8004C24C 140089AE */  sw         $t1, 0x14($s4)
    /* 3CA50 8004C250 1C008AA6 */  sh         $t2, 0x1C($s4)
    /* 3CA54 8004C254 24008BA6 */  sh         $t3, 0x24($s4)
    /* 3CA58 8004C258 08008CEA */  swc2       $12, 0x8($s4)
    /* 3CA5C 8004C25C 10008DEA */  swc2       $13, 0x10($s4)
    /* 3CA60 8004C260 18008EEA */  swc2       $14, 0x18($s4)
    /* 3CA64 8004C264 200060CA */  lwc2       $0, 0x20($s3)
    /* 3CA68 8004C268 240061CA */  lwc2       $1, 0x24($s3)
    /* 3CA6C 8004C26C D001E88E */  lw         $t0, 0x1D0($s7)
    /* 3CA70 8004C270 00000000 */  nop
    /* 3CA74 8004C274 0100184A */  rtps
    /* 3CA78 8004C278 040088AE */  sw         $t0, 0x4($s4)
    /* 3CA7C 8004C27C 000091AE */  sw         $s1, 0x0($s4)
    /* 3CA80 8004C280 20008EEA */  swc2       $14, 0x20($s4)
    /* 3CA84 8004C284 0009093C */  lui        $t1, (0x9000000 >> 16)
    /* 3CA88 8004C288 24889E02 */  and        $s1, $s4, $fp
    /* 3CA8C 8004C28C 25882902 */  or         $s1, $s1, $t1
    /* 3CA90 8004C290 0800E003 */  jr         $ra
    /* 3CA94 8004C294 28009422 */   addi      $s4, $s4, 0x28 /* handwritten instruction */
  .L8004C298:
    /* 3CA98 8004C298 28007322 */  addi       $s3, $s3, 0x28 /* handwritten instruction */
    /* 3CA9C 8004C29C FFFF8420 */  addi       $a0, $a0, -0x1 /* handwritten instruction */
    /* 3CAA0 8004C2A0 00007FAE */  sw         $ra, 0x0($s3)
    /* 3CAA4 8004C2A4 040064A6 */  sh         $a0, 0x4($s3)
    /* 3CAA8 8004C2A8 E0FF688E */  lw         $t0, -0x20($s3)
    /* 3CAAC 8004C2AC E4FF698E */  lw         $t1, -0x1C($s3)
    /* 3CAB0 8004C2B0 080068AE */  sw         $t0, 0x8($s3)
    /* 3CAB4 8004C2B4 0C0069AE */  sw         $t1, 0xC($s3)
    /* 3CAB8 8004C2B8 E0FF6422 */  addi       $a0, $s3, -0x20 /* handwritten instruction */
    /* 3CABC 8004C2BC E8FF6522 */  addi       $a1, $s3, -0x18 /* handwritten instruction */
    /* 3CAC0 8004C2C0 E230010C */  jal        func_8004C388
    /* 3CAC4 8004C2C4 10006622 */   addi      $a2, $s3, 0x10 /* handwritten instruction */
    /* 3CAC8 8004C2C8 E0FF6422 */  addi       $a0, $s3, -0x20 /* handwritten instruction */
    /* 3CACC 8004C2CC F0FF6522 */  addi       $a1, $s3, -0x10 /* handwritten instruction */
    /* 3CAD0 8004C2D0 E230010C */  jal        func_8004C388
    /* 3CAD4 8004C2D4 18006622 */   addi      $a2, $s3, 0x18 /* handwritten instruction */
    /* 3CAD8 8004C2D8 E0FF6422 */  addi       $a0, $s3, -0x20 /* handwritten instruction */
    /* 3CADC 8004C2DC F8FF6522 */  addi       $a1, $s3, -0x8 /* handwritten instruction */
    /* 3CAE0 8004C2E0 E230010C */  jal        func_8004C388
    /* 3CAE4 8004C2E4 20006622 */   addi      $a2, $s3, 0x20 /* handwritten instruction */
    /* 3CAE8 8004C2E8 04006486 */  lh         $a0, 0x4($s3)
    /* 3CAEC 8004C2EC 7D30010C */  jal        func_8004C1F4
    /* 3CAF0 8004C2F0 00000000 */   nop
    /* 3CAF4 8004C2F4 E8FF688E */  lw         $t0, -0x18($s3)
    /* 3CAF8 8004C2F8 ECFF698E */  lw         $t1, -0x14($s3)
    /* 3CAFC 8004C2FC 080068AE */  sw         $t0, 0x8($s3)
    /* 3CB00 8004C300 0C0069AE */  sw         $t1, 0xC($s3)
    /* 3CB04 8004C304 E8FF6422 */  addi       $a0, $s3, -0x18 /* handwritten instruction */
    /* 3CB08 8004C308 F8FF6522 */  addi       $a1, $s3, -0x8 /* handwritten instruction */
    /* 3CB0C 8004C30C E230010C */  jal        func_8004C388
    /* 3CB10 8004C310 18006622 */   addi      $a2, $s3, 0x18 /* handwritten instruction */
    /* 3CB14 8004C314 04006486 */  lh         $a0, 0x4($s3)
    /* 3CB18 8004C318 7D30010C */  jal        func_8004C1F4
    /* 3CB1C 8004C31C 00000000 */   nop
    /* 3CB20 8004C320 F8FF688E */  lw         $t0, -0x8($s3)
    /* 3CB24 8004C324 FCFF698E */  lw         $t1, -0x4($s3)
    /* 3CB28 8004C328 080068AE */  sw         $t0, 0x8($s3)
    /* 3CB2C 8004C32C 0C0069AE */  sw         $t1, 0xC($s3)
    /* 3CB30 8004C330 F0FF6422 */  addi       $a0, $s3, -0x10 /* handwritten instruction */
    /* 3CB34 8004C334 F8FF6522 */  addi       $a1, $s3, -0x8 /* handwritten instruction */
    /* 3CB38 8004C338 E230010C */  jal        func_8004C388
    /* 3CB3C 8004C33C 10006622 */   addi      $a2, $s3, 0x10 /* handwritten instruction */
    /* 3CB40 8004C340 04006486 */  lh         $a0, 0x4($s3)
    /* 3CB44 8004C344 7D30010C */  jal        func_8004C1F4
    /* 3CB48 8004C348 00000000 */   nop
    /* 3CB4C 8004C34C F0FF688E */  lw         $t0, -0x10($s3)
    /* 3CB50 8004C350 F4FF698E */  lw         $t1, -0xC($s3)
    /* 3CB54 8004C354 080068AE */  sw         $t0, 0x8($s3)
    /* 3CB58 8004C358 0C0069AE */  sw         $t1, 0xC($s3)
    /* 3CB5C 8004C35C E0FF6422 */  addi       $a0, $s3, -0x20 /* handwritten instruction */
    /* 3CB60 8004C360 F0FF6522 */  addi       $a1, $s3, -0x10 /* handwritten instruction */
    /* 3CB64 8004C364 E230010C */  jal        func_8004C388
    /* 3CB68 8004C368 18006622 */   addi      $a2, $s3, 0x18 /* handwritten instruction */
    /* 3CB6C 8004C36C 04006486 */  lh         $a0, 0x4($s3)
    /* 3CB70 8004C370 7D30010C */  jal        func_8004C1F4
    /* 3CB74 8004C374 00000000 */   nop
    /* 3CB78 8004C378 00007F8E */  lw         $ra, 0x0($s3)
    /* 3CB7C 8004C37C 00000000 */  nop
    /* 3CB80 8004C380 0800E003 */  jr         $ra
    /* 3CB84 8004C384 D8FF7322 */   addi      $s3, $s3, -0x28 /* handwritten instruction */
endlabel func_8004C1F4
