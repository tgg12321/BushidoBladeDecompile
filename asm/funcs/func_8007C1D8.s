glabel func_8007C1D8
    /* 6C9D8 8007C1D8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6C9DC 8007C1DC 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C9E0 8007C1E0 21808000 */  addu       $s0, $a0, $zero
    /* 6C9E4 8007C1E4 02000224 */  addiu      $v0, $zero, 0x2
    /* 6C9E8 8007C1E8 1400BFAF */  sw         $ra, 0x14($sp)
    /* 6C9EC 8007C1EC 030002A2 */  sb         $v0, 0x3($s0)
    /* 6C9F0 8007C1F0 0000A484 */  lh         $a0, 0x0($a1)
    /* 6C9F4 8007C1F4 0200A584 */  lh         $a1, 0x2($a1)
    /* 6C9F8 8007C1F8 4EF2010C */  jal        func_8007C938
    /* 6C9FC 8007C1FC 00000000 */   nop
    /* 6CA00 8007C200 040002AE */  sw         $v0, 0x4($s0)
    /* 6CA04 8007C204 080000AE */  sw         $zero, 0x8($s0)
    /* 6CA08 8007C208 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6CA0C 8007C20C 1000B08F */  lw         $s0, 0x10($sp)
    /* 6CA10 8007C210 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6CA14 8007C214 0800E003 */  jr         $ra
    /* 6CA18 8007C218 00000000 */   nop
endlabel func_8007C1D8
