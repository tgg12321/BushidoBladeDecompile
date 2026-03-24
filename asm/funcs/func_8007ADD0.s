glabel func_8007ADD0
    /* 6B5D0 8007ADD0 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6B5D4 8007ADD4 1800B0AF */  sw         $s0, 0x18($sp)
    /* 6B5D8 8007ADD8 21808000 */  addu       $s0, $a0, $zero
    /* 6B5DC 8007ADDC 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* 6B5E0 8007ADE0 00000586 */  lh         $a1, 0x0($s0)
    /* 6B5E4 8007ADE4 02000686 */  lh         $a2, 0x2($s0)
    /* 6B5E8 8007ADE8 04000786 */  lh         $a3, 0x4($s0)
    /* 6B5EC 8007ADEC 06000286 */  lh         $v0, 0x6($s0)
    /* 6B5F0 8007ADF0 0A80033C */  lui        $v1, %hi(D_8009BE70)
    /* 6B5F4 8007ADF4 70BE638C */  lw         $v1, %lo(D_8009BE70)($v1)
    /* 6B5F8 8007ADF8 0180043C */  lui        $a0, %hi(D_80015DD8)
    /* 6B5FC 8007ADFC D85D8424 */  addiu      $a0, $a0, %lo(D_80015DD8)
    /* 6B600 8007AE00 09F86000 */  jalr       $v1
    /* 6B604 8007AE04 1000A2AF */   sw        $v0, 0x10($sp)
    /* 6B608 8007AE08 08000586 */  lh         $a1, 0x8($s0)
    /* 6B60C 8007AE0C 0A000686 */  lh         $a2, 0xA($s0)
    /* 6B610 8007AE10 0C000786 */  lh         $a3, 0xC($s0)
    /* 6B614 8007AE14 0E000286 */  lh         $v0, 0xE($s0)
    /* 6B618 8007AE18 0A80033C */  lui        $v1, %hi(D_8009BE70)
    /* 6B61C 8007AE1C 70BE638C */  lw         $v1, %lo(D_8009BE70)($v1)
    /* 6B620 8007AE20 0180043C */  lui        $a0, %hi(D_80015DF4)
    /* 6B624 8007AE24 F45D8424 */  addiu      $a0, $a0, %lo(D_80015DF4)
    /* 6B628 8007AE28 09F86000 */  jalr       $v1
    /* 6B62C 8007AE2C 1000A2AF */   sw        $v0, 0x10($sp)
    /* 6B630 8007AE30 10000592 */  lbu        $a1, 0x10($s0)
    /* 6B634 8007AE34 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6B638 8007AE38 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6B63C 8007AE3C 0180043C */  lui        $a0, %hi(D_80015E10)
    /* 6B640 8007AE40 105E8424 */  addiu      $a0, $a0, %lo(D_80015E10)
    /* 6B644 8007AE44 09F84000 */  jalr       $v0
    /* 6B648 8007AE48 00000000 */   nop
    /* 6B64C 8007AE4C 11000592 */  lbu        $a1, 0x11($s0)
    /* 6B650 8007AE50 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6B654 8007AE54 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6B658 8007AE58 0180043C */  lui        $a0, %hi(D_80015E1C)
    /* 6B65C 8007AE5C 1C5E8424 */  addiu      $a0, $a0, %lo(D_80015E1C)
    /* 6B660 8007AE60 09F84000 */  jalr       $v0
    /* 6B664 8007AE64 00000000 */   nop
    /* 6B668 8007AE68 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* 6B66C 8007AE6C 1800B08F */  lw         $s0, 0x18($sp)
    /* 6B670 8007AE70 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6B674 8007AE74 0800E003 */  jr         $ra
    /* 6B678 8007AE78 00000000 */   nop
endlabel func_8007ADD0
