glabel func_8003AE5C
    /* 2B65C 8003AE5C E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 2B660 8003AE60 1400B1AF */  sw         $s1, 0x14($sp)
    /* 2B664 8003AE64 1980113C */  lui        $s1, (0x80190800 >> 16)
    /* 2B668 8003AE68 00083136 */  ori        $s1, $s1, (0x80190800 & 0xFFFF)
    /* 2B66C 8003AE6C 1000B0AF */  sw         $s0, 0x10($sp)
    /* 2B670 8003AE70 FFFF1024 */  addiu      $s0, $zero, -0x1
    /* 2B674 8003AE74 21280000 */  addu       $a1, $zero, $zero
    /* 2B678 8003AE78 0180063C */  lui        $a2, %hi(jtbl_80010CC4)
    /* 2B67C 8003AE7C C40CC624 */  addiu      $a2, $a2, %lo(jtbl_80010CC4)
    /* 2B680 8003AE80 1800BFAF */  sw         $ra, 0x18($sp)
  .L8003AE84:
    /* 2B684 8003AE84 00008390 */  lbu        $v1, 0x0($a0)
    /* 2B688 8003AE88 00000000 */  nop
    /* 2B68C 8003AE8C 1500622C */  sltiu      $v0, $v1, 0x15
    /* 2B690 8003AE90 13004010 */  beqz       $v0, .L8003AEE0
    /* 2B694 8003AE94 01008424 */   addiu     $a0, $a0, 0x1
    /* 2B698 8003AE98 80100300 */  sll        $v0, $v1, 2
    /* 2B69C 8003AE9C 21104600 */  addu       $v0, $v0, $a2
    /* 2B6A0 8003AEA0 0000428C */  lw         $v0, 0x0($v0)
    /* 2B6A4 8003AEA4 00000000 */  nop
    /* 2B6A8 8003AEA8 08004000 */  jr         $v0
    /* 2B6AC 8003AEAC 00000000 */   nop
  jlabel .L8003AEB0
    /* 2B6B0 8003AEB0 00009090 */  lbu        $s0, 0x0($a0)
  jlabel .L8003AEB4
    /* 2B6B4 8003AEB4 B8EB0008 */  j          .L8003AEE0
    /* 2B6B8 8003AEB8 01000524 */   addiu     $a1, $zero, 0x1
  jlabel .L8003AEBC
    /* 2B6BC 8003AEBC B8EB0008 */  j          .L8003AEE0
    /* 2B6C0 8003AEC0 05008424 */   addiu     $a0, $a0, 0x5
  jlabel .L8003AEC4
    /* 2B6C4 8003AEC4 B8EB0008 */  j          .L8003AEE0
    /* 2B6C8 8003AEC8 03008424 */   addiu     $a0, $a0, 0x3
  jlabel .L8003AECC
    /* 2B6CC 8003AECC B8EB0008 */  j          .L8003AEE0
    /* 2B6D0 8003AED0 02008424 */   addiu     $a0, $a0, 0x2
  jlabel .L8003AED4
    /* 2B6D4 8003AED4 B8EB0008 */  j          .L8003AEE0
    /* 2B6D8 8003AED8 04008424 */   addiu     $a0, $a0, 0x4
  jlabel .L8003AEDC
    /* 2B6DC 8003AEDC 1D008424 */  addiu      $a0, $a0, 0x1D
  jlabel .L8003AEE0
    /* 2B6E0 8003AEE0 E8FFA010 */  beqz       $a1, .L8003AE84
    /* 2B6E4 8003AEE4 00000000 */   nop
    /* 2B6E8 8003AEE8 0F000006 */  bltz       $s0, .L8003AF28
    /* 2B6EC 8003AEEC 00000000 */   nop
    /* 2B6F0 8003AEF0 0A80033C */  lui        $v1, %hi(D_800A37A0)
    /* 2B6F4 8003AEF4 A0376390 */  lbu        $v1, %lo(D_800A37A0)($v1)
    /* 2B6F8 8003AEF8 0A80023C */  lui        $v0, %hi(D_800A36A4)
    /* 2B6FC 8003AEFC A4364294 */  lhu        $v0, %lo(D_800A36A4)($v0)
    /* 2B700 8003AF00 0A80013C */  lui        $at, %hi(D_800A37A8)
    /* 2B704 8003AF04 21082300 */  addu       $at, $at, $v1
    /* 2B708 8003AF08 A83722A0 */  sb         $v0, %lo(D_800A37A8)($at)
    /* 2B70C 8003AF0C 1A5A000C */  jal        func_80016868
    /* 2B710 8003AF10 00000000 */   nop
    /* 2B714 8003AF14 4E83000C */  jal        func_80020D38
    /* 2B718 8003AF18 00000000 */   nop
    /* 2B71C 8003AF1C 21200002 */  addu       $a0, $s0, $zero
    /* 2B720 8003AF20 AB80010C */  jal        func_800602AC
    /* 2B724 8003AF24 21282002 */   addu      $a1, $s1, $zero
  .L8003AF28:
    /* 2B728 8003AF28 1800BF8F */  lw         $ra, 0x18($sp)
    /* 2B72C 8003AF2C 1400B18F */  lw         $s1, 0x14($sp)
    /* 2B730 8003AF30 1000B08F */  lw         $s0, 0x10($sp)
    /* 2B734 8003AF34 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 2B738 8003AF38 0800E003 */  jr         $ra
    /* 2B73C 8003AF3C 00000000 */   nop
endlabel func_8003AE5C
