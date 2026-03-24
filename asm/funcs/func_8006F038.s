glabel func_8006F038
    /* 5F838 8006F038 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 5F83C 8006F03C 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 5F840 8006F040 21888000 */  addu       $s1, $a0, $zero
    /* 5F844 8006F044 2000BFAF */  sw         $ra, 0x20($sp)
    /* 5F848 8006F048 1800B0AF */  sw         $s0, 0x18($sp)
    /* 5F84C 8006F04C 1400308E */  lw         $s0, 0x14($s1)
    /* 5F850 8006F050 B4EA010C */  jal        func_8007AAD0
    /* 5F854 8006F054 21200002 */   addu      $a0, $s0, $zero
    /* 5F858 8006F058 21200002 */  addu       $a0, $s0, $zero
    /* 5F85C 8006F05C 84048297 */  lhu        $v0, %gp_rel(D_800A3550)($gp)
    /* 5F860 8006F060 01000524 */  addiu      $a1, $zero, 0x1
    /* 5F864 8006F064 080000A6 */  sh         $zero, 0x8($s0)
    /* 5F868 8006F068 0A0000A6 */  sh         $zero, 0xA($s0)
    /* 5F86C 8006F06C 040002A2 */  sb         $v0, 0x4($s0)
    /* 5F870 8006F070 84048397 */  lhu        $v1, %gp_rel(D_800A3550)($gp)
    /* 5F874 8006F074 80020224 */  addiu      $v0, $zero, 0x280
    /* 5F878 8006F078 0C0002A6 */  sh         $v0, 0xC($s0)
    /* 5F87C 8006F07C 050003A2 */  sb         $v1, 0x5($s0)
    /* 5F880 8006F080 84048397 */  lhu        $v1, %gp_rel(D_800A3550)($gp)
    /* 5F884 8006F084 F0000224 */  addiu      $v0, $zero, 0xF0
    /* 5F888 8006F088 0E0002A6 */  sh         $v0, 0xE($s0)
    /* 5F88C 8006F08C 5AEA010C */  jal        func_8007A968
    /* 5F890 8006F090 060003A2 */   sb        $v1, 0x6($s0)
    /* 5F894 8006F094 21280002 */  addu       $a1, $s0, $zero
    /* 5F898 8006F098 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5F89C 8006F09C 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5F8A0 8006F0A0 2DEA010C */  jal        func_8007A8B4
    /* 5F8A4 8006F0A4 10001026 */   addiu     $s0, $s0, 0x10
    /* 5F8A8 8006F0A8 01000524 */  addiu      $a1, $zero, 0x1
    /* 5F8AC 8006F0AC 21300000 */  addu       $a2, $zero, $zero
    /* 5F8B0 8006F0B0 140030AE */  sw         $s0, 0x14($s1)
    /* 5F8B4 8006F0B4 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5F8B8 8006F0B8 1800248E */  lw         $a0, 0x18($s1)
    /* 5F8BC 8006F0BC 92F0010C */  jal        func_8007C248
    /* 5F8C0 8006F0C0 40000724 */   addiu     $a3, $zero, 0x40
    /* 5F8C4 8006F0C4 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5F8C8 8006F0C8 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5F8CC 8006F0CC 1800258E */  lw         $a1, 0x18($s1)
    /* 5F8D0 8006F0D0 2DEA010C */  jal        func_8007A8B4
    /* 5F8D4 8006F0D4 00000000 */   nop
    /* 5F8D8 8006F0D8 1800228E */  lw         $v0, 0x18($s1)
    /* 5F8DC 8006F0DC 00000000 */  nop
    /* 5F8E0 8006F0E0 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5F8E4 8006F0E4 180022AE */  sw         $v0, 0x18($s1)
    /* 5F8E8 8006F0E8 2000BF8F */  lw         $ra, 0x20($sp)
    /* 5F8EC 8006F0EC 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 5F8F0 8006F0F0 1800B08F */  lw         $s0, 0x18($sp)
    /* 5F8F4 8006F0F4 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 5F8F8 8006F0F8 0800E003 */  jr         $ra
    /* 5F8FC 8006F0FC 00000000 */   nop
endlabel func_8006F038
