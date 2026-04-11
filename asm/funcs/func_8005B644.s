glabel func_8005B644
    /* 4BE44 8005B644 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 4BE48 8005B648 1000B0AF */  sw         $s0, 0x10($sp)
    /* 4BE4C 8005B64C 21808000 */  addu       $s0, $a0, $zero
    /* 4BE50 8005B650 21200000 */  addu       $a0, $zero, $zero
    /* 4BE54 8005B654 1800BFAF */  sw         $ra, 0x18($sp)
    /* 4BE58 8005B658 3416020C */  jal        title_mv_exec2
    /* 4BE5C 8005B65C 1400B1AF */   sw        $s1, 0x14($sp)
    /* 4BE60 8005B660 40881000 */  sll        $s1, $s0, 1
    /* 4BE64 8005B664 21883002 */  addu       $s1, $s1, $s0
    /* 4BE68 8005B668 01003126 */  addiu      $s1, $s1, 0x1
    /* 4BE6C 8005B66C 00241100 */  sll        $a0, $s1, 16
    /* 4BE70 8005B670 D91F020C */  jal        func_80087F64
    /* 4BE74 8005B674 03240400 */   sra       $a0, $a0, 16
    /* 4BE78 8005B678 80881100 */  sll        $s1, $s1, 2
    /* 4BE7C 8005B67C 0F80013C */  lui        $at, %hi(D_800EFC38)
    /* 4BE80 8005B680 21083100 */  addu       $at, $at, $s1
    /* 4BE84 8005B684 38FC20AC */  sw         $zero, %lo(D_800EFC38)($at)
    /* 4BE88 8005B688 0F80013C */  lui        $at, %hi(D_800EFB38)
    /* 4BE8C 8005B68C 21083100 */  addu       $at, $at, $s1
    /* 4BE90 8005B690 38FB20AC */  sw         $zero, %lo(D_800EFB38)($at)
    /* 4BE94 8005B694 1800BF8F */  lw         $ra, 0x18($sp)
    /* 4BE98 8005B698 1400B18F */  lw         $s1, 0x14($sp)
    /* 4BE9C 8005B69C 1000B08F */  lw         $s0, 0x10($sp)
    /* 4BEA0 8005B6A0 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 4BEA4 8005B6A4 0800E003 */  jr         $ra
    /* 4BEA8 8005B6A8 00000000 */   nop
endlabel func_8005B644
