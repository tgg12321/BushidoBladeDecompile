glabel func_8007B178
    /* 6B978 8007B178 0A80023C */  lui        $v0, %hi(D_8009BE76)
    /* 6B97C 8007B17C 76BE4290 */  lbu        $v0, %lo(D_8009BE76)($v0)
    /* 6B980 8007B180 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6B984 8007B184 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6B988 8007B188 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6B98C 8007B18C 0A80113C */  lui        $s1, %hi(D_8009BE75)
    /* 6B990 8007B190 75BE3126 */  addiu      $s1, $s1, %lo(D_8009BE75)
    /* 6B994 8007B194 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* 6B998 8007B198 1800B2AF */  sw         $s2, 0x18($sp)
    /* 6B99C 8007B19C 00003292 */  lbu        $s2, 0x0($s1)
    /* 6B9A0 8007B1A0 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6B9A4 8007B1A4 08004014 */  bnez       $v0, .L8007B1C8
    /* 6B9A8 8007B1A8 21808000 */   addu      $s0, $a0, $zero
    /* 6B9AC 8007B1AC 0180043C */  lui        $a0, %hi(D_80015ED4)
    /* 6B9B0 8007B1B0 D45E8424 */  addiu      $a0, $a0, %lo(D_80015ED4)
    /* 6B9B4 8007B1B4 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6B9B8 8007B1B8 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6B9BC 8007B1BC 00000000 */  nop
    /* 6B9C0 8007B1C0 09F84000 */  jalr       $v0
    /* 6B9C4 8007B1C4 21280002 */   addu      $a1, $s0, $zero
  .L8007B1C8:
    /* 6B9C8 8007B1C8 00002292 */  lbu        $v0, 0x0($s1)
    /* 6B9CC 8007B1CC 00000000 */  nop
    /* 6B9D0 8007B1D0 0D000212 */  beq        $s0, $v0, .L8007B208
    /* 6B9D4 8007B1D4 21104002 */   addu      $v0, $s2, $zero
    /* 6B9D8 8007B1D8 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6B9DC 8007B1DC 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6B9E0 8007B1E0 00000000 */  nop
    /* 6B9E4 8007B1E4 3400428C */  lw         $v0, 0x34($v0)
    /* 6B9E8 8007B1E8 00000000 */  nop
    /* 6B9EC 8007B1EC 09F84000 */  jalr       $v0
    /* 6B9F0 8007B1F0 01000424 */   addiu     $a0, $zero, 0x1
    /* 6B9F4 8007B1F4 02000424 */  addiu      $a0, $zero, 0x2
    /* 6B9F8 8007B1F8 21280000 */  addu       $a1, $zero, $zero
    /* 6B9FC 8007B1FC C80A020C */  jal        irq_AcknowledgeVblank
    /* 6BA00 8007B200 000030A2 */   sb        $s0, 0x0($s1)
    /* 6BA04 8007B204 21104002 */  addu       $v0, $s2, $zero
  .L8007B208:
    /* 6BA08 8007B208 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* 6BA0C 8007B20C 1800B28F */  lw         $s2, 0x18($sp)
    /* 6BA10 8007B210 1400B18F */  lw         $s1, 0x14($sp)
    /* 6BA14 8007B214 1000B08F */  lw         $s0, 0x10($sp)
    /* 6BA18 8007B218 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6BA1C 8007B21C 0800E003 */  jr         $ra
    /* 6BA20 8007B220 00000000 */   nop
endlabel func_8007B178
