glabel func_8007C118
    /* 6C918 8007C118 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6C91C 8007C11C 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C920 8007C120 21808000 */  addu       $s0, $a0, $zero
    /* 6C924 8007C124 02000224 */  addiu      $v0, $zero, 0x2
    /* 6C928 8007C128 2120A000 */  addu       $a0, $a1, $zero
    /* 6C92C 8007C12C 1400BFAF */  sw         $ra, 0x14($sp)
    /* 6C930 8007C130 5FF2010C */  jal        func_8007C97C
    /* 6C934 8007C134 030002A2 */   sb        $v0, 0x3($s0)
    /* 6C938 8007C138 040002AE */  sw         $v0, 0x4($s0)
    /* 6C93C 8007C13C 080000AE */  sw         $zero, 0x8($s0)
    /* 6C940 8007C140 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6C944 8007C144 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C948 8007C148 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6C94C 8007C14C 0800E003 */  jr         $ra
    /* 6C950 8007C150 00000000 */   nop
endlabel func_8007C118
