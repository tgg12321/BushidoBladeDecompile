glabel func_8007C248
    /* 6CA48 8007C248 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6CA4C 8007C24C 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6CA50 8007C250 21808000 */  addu       $s0, $a0, $zero
    /* 6CA54 8007C254 2120A000 */  addu       $a0, $a1, $zero
    /* 6CA58 8007C258 02000224 */  addiu      $v0, $zero, 0x2
    /* 6CA5C 8007C25C 2128C000 */  addu       $a1, $a2, $zero
    /* 6CA60 8007C260 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6CA64 8007C264 3000B18F */  lw         $s1, 0x30($sp)
    /* 6CA68 8007C268 FFFFE630 */  andi       $a2, $a3, 0xFFFF
    /* 6CA6C 8007C26C 1800BFAF */  sw         $ra, 0x18($sp)
    /* 6CA70 8007C270 D2F1010C */  jal        func_8007C748
    /* 6CA74 8007C274 030002A2 */   sb        $v0, 0x3($s0)
    /* 6CA78 8007C278 040002AE */  sw         $v0, 0x4($s0)
    /* 6CA7C 8007C27C 5FF2010C */  jal        func_8007C97C
    /* 6CA80 8007C280 21202002 */   addu      $a0, $s1, $zero
    /* 6CA84 8007C284 080002AE */  sw         $v0, 0x8($s0)
    /* 6CA88 8007C288 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6CA8C 8007C28C 1400B18F */  lw         $s1, 0x14($sp)
    /* 6CA90 8007C290 1000B08F */  lw         $s0, 0x10($sp)
    /* 6CA94 8007C294 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6CA98 8007C298 0800E003 */  jr         $ra
    /* 6CA9C 8007C29C 00000000 */   nop
endlabel func_8007C248
