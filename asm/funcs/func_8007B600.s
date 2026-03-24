glabel func_8007B600
    /* 6BE00 8007B600 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6BE04 8007B604 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6BE08 8007B608 21808000 */  addu       $s0, $a0, $zero
    /* 6BE0C 8007B60C 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6BE10 8007B610 2188A000 */  addu       $s1, $a1, $zero
    /* 6BE14 8007B614 0180043C */  lui        $a0, %hi(D_80015F5C)
    /* 6BE18 8007B618 5C5F8424 */  addiu      $a0, $a0, %lo(D_80015F5C)
    /* 6BE1C 8007B61C 1800BFAF */  sw         $ra, 0x18($sp)
    /* 6BE20 8007B620 EAEC010C */  jal        func_8007B3A8
    /* 6BE24 8007B624 21280002 */   addu      $a1, $s0, $zero
    /* 6BE28 8007B628 21280002 */  addu       $a1, $s0, $zero
    /* 6BE2C 8007B62C 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6BE30 8007B630 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6BE34 8007B634 08000624 */  addiu      $a2, $zero, 0x8
    /* 6BE38 8007B638 2000448C */  lw         $a0, 0x20($v0)
    /* 6BE3C 8007B63C 0800428C */  lw         $v0, 0x8($v0)
    /* 6BE40 8007B640 00000000 */  nop
    /* 6BE44 8007B644 09F84000 */  jalr       $v0
    /* 6BE48 8007B648 21382002 */   addu      $a3, $s1, $zero
    /* 6BE4C 8007B64C 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6BE50 8007B650 1400B18F */  lw         $s1, 0x14($sp)
    /* 6BE54 8007B654 1000B08F */  lw         $s0, 0x10($sp)
    /* 6BE58 8007B658 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6BE5C 8007B65C 0800E003 */  jr         $ra
    /* 6BE60 8007B660 00000000 */   nop
endlabel func_8007B600
