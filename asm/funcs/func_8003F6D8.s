glabel func_8003F6D8
    /* 2FED8 8003F6D8 A8FFBD27 */  addiu      $sp, $sp, -0x58
    /* 2FEDC 8003F6DC 5400BFAF */  sw         $ra, 0x54($sp)
    /* 2FEE0 8003F6E0 5000BEAF */  sw         $fp, 0x50($sp)
    /* 2FEE4 8003F6E4 4C00B7AF */  sw         $s7, 0x4C($sp)
    /* 2FEE8 8003F6E8 4800B6AF */  sw         $s6, 0x48($sp)
    /* 2FEEC 8003F6EC 4400B5AF */  sw         $s5, 0x44($sp)
    /* 2FEF0 8003F6F0 4000B4AF */  sw         $s4, 0x40($sp)
    /* 2FEF4 8003F6F4 3C00B3AF */  sw         $s3, 0x3C($sp)
    /* 2FEF8 8003F6F8 3800B2AF */  sw         $s2, 0x38($sp)
    /* 2FEFC 8003F6FC 3400B1AF */  sw         $s1, 0x34($sp)
    /* 2FF00 8003F700 3000B0AF */  sw         $s0, 0x30($sp)
    /* 2FF04 8003F704 1000A4AF */  sw         $a0, 0x10($sp)
    /* 2FF08 8003F708 00008284 */  lh         $v0, 0x0($a0)
    /* 2FF0C 8003F70C 00000000 */  nop
    /* 2FF10 8003F710 2B004018 */  blez       $v0, .L8003F7C0
    /* 2FF14 8003F714 21F00000 */   addu      $fp, $zero, $zero
    /* 2FF18 8003F718 08000724 */  addiu      $a3, $zero, 0x8
    /* 2FF1C 8003F71C 2800A7AF */  sw         $a3, 0x28($sp)
  .L8003F720:
    /* 2FF20 8003F720 1000A88F */  lw         $t0, 0x10($sp)
    /* 2FF24 8003F724 2800A78F */  lw         $a3, 0x28($sp)
    /* 2FF28 8003F728 21B80000 */  addu       $s7, $zero, $zero
    /* 2FF2C 8003F72C 21100701 */  addu       $v0, $t0, $a3
    /* 2FF30 8003F730 1C00438C */  lw         $v1, 0x1C($v0)
    /* 2FF34 8003F734 00000000 */  nop
    /* 2FF38 8003F738 18006018 */  blez       $v1, .L8003F79C
    /* 2FF3C 8003F73C 1C005424 */   addiu     $s4, $v0, 0x1C
    /* 2FF40 8003F740 18001624 */  addiu      $s6, $zero, 0x18
    /* 2FF44 8003F744 84001524 */  addiu      $s5, $zero, 0x84
    /* 2FF48 8003F748 21988002 */  addu       $s3, $s4, $zero
    /* 2FF4C 8003F74C 21909502 */  addu       $s2, $s4, $s5
  .L8003F750:
    /* 2FF50 8003F750 21284002 */  addu       $a1, $s2, $zero
    /* 2FF54 8003F754 21889602 */  addu       $s1, $s4, $s6
    /* 2FF58 8003F758 21302002 */  addu       $a2, $s1, $zero
    /* 2FF5C 8003F75C 2000D626 */  addiu      $s6, $s6, 0x20
    /* 2FF60 8003F760 1000B526 */  addiu      $s5, $s5, 0x10
    /* 2FF64 8003F764 0400708E */  lw         $s0, 0x4($s3)
    /* 2FF68 8003F768 04007326 */  addiu      $s3, $s3, 0x4
    /* 2FF6C 8003F76C 18001026 */  addiu      $s0, $s0, 0x18
    /* 2FF70 8003F770 884A010C */  jal        func_80052A20
    /* 2FF74 8003F774 21200002 */   addu      $a0, $s0, $zero
    /* 2FF78 8003F778 21200002 */  addu       $a0, $s0, $zero
    /* 2FF7C 8003F77C 08004526 */  addiu      $a1, $s2, 0x8
    /* 2FF80 8003F780 884A010C */  jal        func_80052A20
    /* 2FF84 8003F784 10002626 */   addiu     $a2, $s1, 0x10
    /* 2FF88 8003F788 0000828E */  lw         $v0, 0x0($s4)
    /* 2FF8C 8003F78C 0100F726 */  addiu      $s7, $s7, 0x1
    /* 2FF90 8003F790 2A10E202 */  slt        $v0, $s7, $v0
    /* 2FF94 8003F794 EEFF4014 */  bnez       $v0, .L8003F750
    /* 2FF98 8003F798 21909502 */   addu      $s2, $s4, $s5
  .L8003F79C:
    /* 2FF9C 8003F79C 2800A88F */  lw         $t0, 0x28($sp)
    /* 2FFA0 8003F7A0 1000A78F */  lw         $a3, 0x10($sp)
    /* 2FFA4 8003F7A4 D0000825 */  addiu      $t0, $t0, 0xD0
    /* 2FFA8 8003F7A8 2800A8AF */  sw         $t0, 0x28($sp)
    /* 2FFAC 8003F7AC 0000E284 */  lh         $v0, 0x0($a3)
    /* 2FFB0 8003F7B0 0100DE27 */  addiu      $fp, $fp, 0x1
    /* 2FFB4 8003F7B4 2A10C203 */  slt        $v0, $fp, $v0
    /* 2FFB8 8003F7B8 D9FF4014 */  bnez       $v0, .L8003F720
    /* 2FFBC 8003F7BC 00000000 */   nop
  .L8003F7C0:
    /* 2FFC0 8003F7C0 5400BF8F */  lw         $ra, 0x54($sp)
    /* 2FFC4 8003F7C4 5000BE8F */  lw         $fp, 0x50($sp)
    /* 2FFC8 8003F7C8 4C00B78F */  lw         $s7, 0x4C($sp)
    /* 2FFCC 8003F7CC 4800B68F */  lw         $s6, 0x48($sp)
    /* 2FFD0 8003F7D0 4400B58F */  lw         $s5, 0x44($sp)
    /* 2FFD4 8003F7D4 4000B48F */  lw         $s4, 0x40($sp)
    /* 2FFD8 8003F7D8 3C00B38F */  lw         $s3, 0x3C($sp)
    /* 2FFDC 8003F7DC 3800B28F */  lw         $s2, 0x38($sp)
    /* 2FFE0 8003F7E0 3400B18F */  lw         $s1, 0x34($sp)
    /* 2FFE4 8003F7E4 3000B08F */  lw         $s0, 0x30($sp)
    /* 2FFE8 8003F7E8 5800BD27 */  addiu      $sp, $sp, 0x58
    /* 2FFEC 8003F7EC 0800E003 */  jr         $ra
    /* 2FFF0 8003F7F0 00000000 */   nop
endlabel func_8003F6D8
