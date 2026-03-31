glabel func_8005B50C
    /* 4BD0C 8005B50C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 4BD10 8005B510 1000BFAF */  sw         $ra, 0x10($sp)
    /* 4BD14 8005B514 3416020C */  jal        func_800858D0
    /* 4BD18 8005B518 21200000 */   addu      $a0, $zero, $zero
    /* 4BD1C 8005B51C E617020C */  jal        func_80085F98
    /* 4BD20 8005B520 00000000 */   nop
    /* 4BD24 8005B524 B917020C */  jal        func_80085EE4
    /* 4BD28 8005B528 21200000 */   addu      $a0, $zero, $zero
    /* 4BD2C 8005B52C 21200000 */  addu       $a0, $zero, $zero
    /* 4BD30 8005B530 9317020C */  jal        func_80085E4C
    /* 4BD34 8005B534 21280000 */   addu      $a1, $zero, $zero
    /* 4BD38 8005B538 550E020C */  jal        irq_ProcessPending
    /* 4BD3C 8005B53C 00000000 */   nop
    /* 4BD40 8005B540 CC0E020C */  jal        spu_Reset
    /* 4BD44 8005B544 00000000 */   nop
    /* 4BD48 8005B548 21280000 */  addu       $a1, $zero, $zero
    /* 4BD4C 8005B54C 0F80043C */  lui        $a0, %hi(D_800EFB38)
    /* 4BD50 8005B550 38FB8424 */  addiu      $a0, $a0, %lo(D_800EFB38)
    /* 4BD54 8005B554 0F80033C */  lui        $v1, %hi(D_800EFC38)
    /* 4BD58 8005B558 38FC6324 */  addiu      $v1, $v1, %lo(D_800EFC38)
  .L8005B55C:
    /* 4BD5C 8005B55C 000060AC */  sw         $zero, 0x0($v1)
    /* 4BD60 8005B560 000080AC */  sw         $zero, 0x0($a0)
    /* 4BD64 8005B564 04008424 */  addiu      $a0, $a0, 0x4
    /* 4BD68 8005B568 0100A524 */  addiu      $a1, $a1, 0x1
    /* 4BD6C 8005B56C 1000A228 */  slti       $v0, $a1, 0x10
    /* 4BD70 8005B570 FAFF4014 */  bnez       $v0, .L8005B55C
    /* 4BD74 8005B574 04006324 */   addiu     $v1, $v1, 0x4
    /* 4BD78 8005B578 3C0380AF */  sw         $zero, %gp_rel(D_800A3408)($gp)
    /* 4BD7C 8005B57C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 4BD80 8005B580 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 4BD84 8005B584 0800E003 */  jr         $ra
    /* 4BD88 8005B588 00000000 */   nop
endlabel func_8005B50C
