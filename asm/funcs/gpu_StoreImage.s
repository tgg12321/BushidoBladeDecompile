glabel gpu_StoreImage
    /* 6BE64 8007B664 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6BE68 8007B668 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6BE6C 8007B66C 21808000 */  addu       $s0, $a0, $zero
    /* 6BE70 8007B670 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6BE74 8007B674 2188A000 */  addu       $s1, $a1, $zero
    /* 6BE78 8007B678 0180043C */  lui        $a0, %hi(D_80015F68)
    /* 6BE7C 8007B67C 685F8424 */  addiu      $a0, $a0, %lo(D_80015F68)
    /* 6BE80 8007B680 1800BFAF */  sw         $ra, 0x18($sp)
    /* 6BE84 8007B684 EAEC010C */  jal        func_8007B3A8
    /* 6BE88 8007B688 21280002 */   addu      $a1, $s0, $zero
    /* 6BE8C 8007B68C 21280002 */  addu       $a1, $s0, $zero
    /* 6BE90 8007B690 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6BE94 8007B694 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6BE98 8007B698 08000624 */  addiu      $a2, $zero, 0x8
    /* 6BE9C 8007B69C 1C00448C */  lw         $a0, 0x1C($v0)
    /* 6BEA0 8007B6A0 0800428C */  lw         $v0, 0x8($v0)
    /* 6BEA4 8007B6A4 00000000 */  nop
    /* 6BEA8 8007B6A8 09F84000 */  jalr       $v0
    /* 6BEAC 8007B6AC 21382002 */   addu      $a3, $s1, $zero
    /* 6BEB0 8007B6B0 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6BEB4 8007B6B4 1400B18F */  lw         $s1, 0x14($sp)
    /* 6BEB8 8007B6B8 1000B08F */  lw         $s0, 0x10($sp)
    /* 6BEBC 8007B6BC 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6BEC0 8007B6C0 0800E003 */  jr         $ra
    /* 6BEC4 8007B6C4 00000000 */   nop
endlabel gpu_StoreImage
