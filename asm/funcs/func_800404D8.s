glabel func_800404D8
    /* 30CD8 800404D8 21280000 */  addu       $a1, $zero, $zero
    /* 30CDC 800404DC 0980043C */  lui        $a0, %hi(D_80094B88)
    /* 30CE0 800404E0 884B8424 */  addiu      $a0, $a0, %lo(D_80094B88)
    /* 30CE4 800404E4 0B80033C */  lui        $v1, %hi(D_800A9A10)
    /* 30CE8 800404E8 109A6324 */  addiu      $v1, $v1, %lo(D_800A9A10)
  .L800404EC:
    /* 30CEC 800404EC 000060AC */  sw         $zero, 0x0($v1)
    /* 30CF0 800404F0 000080AC */  sw         $zero, 0x0($a0)
    /* 30CF4 800404F4 04008424 */  addiu      $a0, $a0, 0x4
    /* 30CF8 800404F8 0100A524 */  addiu      $a1, $a1, 0x1
    /* 30CFC 800404FC 0300A228 */  slti       $v0, $a1, 0x3
    /* 30D00 80040500 FAFF4014 */  bnez       $v0, .L800404EC
    /* 30D04 80040504 04006324 */   addiu     $v1, $v1, 0x4
    /* 30D08 80040508 0800E003 */  jr         $ra
    /* 30D0C 8004050C 00000000 */   nop
endlabel func_800404D8
