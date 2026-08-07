glabel func_8003CE18
    /* 2D618 8003CE18 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 2D61C 8003CE1C 2400BFAF */  sw         $ra, 0x24($sp)
    /* 2D620 8003CE20 8B76000C */  jal        func_8001DA2C
    /* 2D624 8003CE24 2000B0AF */   sw        $s0, 0x20($sp)
    /* 2D628 8003CE28 B0DC000C */  jal        func_800372C0
    /* 2D62C 8003CE2C 00000000 */   nop
    /* 2D630 8003CE30 225A000C */  jal        gpu_InitDisplay
    /* 2D634 8003CE34 00000000 */   nop
    /* 2D638 8003CE38 1A5A000C */  jal        gpu_EnableDisplay
    /* 2D63C 8003CE3C 00000000 */   nop
    /* 2D640 8003CE40 01000424 */  addiu      $a0, $zero, 0x1
    /* 2D644 8003CE44 21280000 */  addu       $a1, $zero, $zero
    /* 2D648 8003CE48 21300000 */  addu       $a2, $zero, $zero
    /* 2D64C 8003CE4C DA59000C */  jal        disp_SetFramebufferMode
    /* 2D650 8003CE50 21380000 */   addu      $a3, $zero, $zero
    /* 2D654 8003CE54 8BF8000C */  jal        func_8003E22C
    /* 2D658 8003CE58 00000000 */   nop
    /* 2D65C 8003CE5C 86FC000C */  jal        game_SetPlayerCount
    /* 2D660 8003CE60 21200000 */   addu      $a0, $zero, $zero
    /* 2D664 8003CE64 B159000C */  jal        disp_CalcFov
    /* 2D668 8003CE68 2D000424 */   addiu     $a0, $zero, 0x2D
    /* 2D66C 8003CE6C FFFB010C */  jal        func_8007EFFC
    /* 2D670 8003CE70 21204000 */   addu      $a0, $v0, $zero
    /* 2D674 8003CE74 0A80033C */  lui        $v1, %hi(D_800A3748)
    /* 2D678 8003CE78 48376380 */  lb         $v1, %lo(D_800A3748)($v1)
    /* 2D67C 8003CE7C 00000000 */  nop
    /* 2D680 8003CE80 00110300 */  sll        $v0, $v1, 4
    /* 2D684 8003CE84 21104300 */  addu       $v0, $v0, $v1
    /* 2D688 8003CE88 80100200 */  sll        $v0, $v0, 2
    /* 2D68C 8003CE8C 21104300 */  addu       $v0, $v0, $v1
    /* 2D690 8003CE90 80100200 */  sll        $v0, $v0, 2
    /* 2D694 8003CE94 23104300 */  subu       $v0, $v0, $v1
    /* 2D698 8003CE98 80100200 */  sll        $v0, $v0, 2
    /* 2D69C 8003CE9C 1080013C */  lui        $at, %hi(D_80101ED6)
    /* 2D6A0 8003CEA0 21082200 */  addu       $at, $at, $v0
    /* 2D6A4 8003CEA4 D61E2294 */  lhu        $v0, %lo(D_80101ED6)($at)
    /* 2D6A8 8003CEA8 00000000 */  nop
    /* 2D6AC 8003CEAC FAFF4224 */  addiu      $v0, $v0, -0x6
    /* 2D6B0 8003CEB0 0200422C */  sltiu      $v0, $v0, 0x2
    /* 2D6B4 8003CEB4 05004010 */  beqz       $v0, .L8003CECC
    /* 2D6B8 8003CEB8 00000000 */   nop
    /* 2D6BC 8003CEBC 06006010 */  beqz       $v1, .L8003CED8
    /* 2D6C0 8003CEC0 08001024 */   addiu     $s0, $zero, 0x8
    /* 2D6C4 8003CEC4 B6F30008 */  j          .L8003CED8
    /* 2D6C8 8003CEC8 09001024 */   addiu     $s0, $zero, 0x9
  .L8003CECC:
    /* 2D6CC 8003CECC 02006010 */  beqz       $v1, .L8003CED8
    /* 2D6D0 8003CED0 06001024 */   addiu     $s0, $zero, 0x6
    /* 2D6D4 8003CED4 07001024 */  addiu      $s0, $zero, 0x7
  .L8003CED8:
    /* 2D6D8 8003CED8 0A80023C */  lui        $v0, %hi(D_800A3748)
    /* 2D6DC 8003CEDC 48374280 */  lb         $v0, %lo(D_800A3748)($v0)
    /* 2D6E0 8003CEE0 1080043C */  lui        $a0, %hi(D_80101FBC)
    /* 2D6E4 8003CEE4 BC1F8424 */  addiu      $a0, $a0, %lo(D_80101FBC)
    /* 2D6E8 8003CEE8 02004014 */  bnez       $v0, .L8003CEF4
    /* 2D6EC 8003CEEC 00000000 */   nop
    /* 2D6F0 8003CEF0 4C048424 */  addiu      $a0, $a0, 0x44C
  .L8003CEF4:
    /* 2D6F4 8003CEF4 0289000C */  jal        func_80022408
    /* 2D6F8 8003CEF8 00000000 */   nop
    /* 2D6FC 8003CEFC 16000424 */  addiu      $a0, $zero, 0x16
    /* 2D700 8003CF00 21304000 */  addu       $a2, $v0, $zero
    /* 2D704 8003CF04 1080073C */  lui        $a3, %hi(D_80101EDA)
    /* 2D708 8003CF08 DA1EE784 */  lh         $a3, %lo(D_80101EDA)($a3)
    /* 2D70C 8003CF0C FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 2D710 8003CF10 1400A2AF */  sw         $v0, 0x14($sp)
    /* 2D714 8003CF14 1800A2AF */  sw         $v0, 0x18($sp)
    /* 2D718 8003CF18 1080023C */  lui        $v0, %hi(D_80102326)
    /* 2D71C 8003CF1C 26234284 */  lh         $v0, %lo(D_80102326)($v0)
    /* 2D720 8003CF20 21280002 */  addu       $a1, $s0, $zero
    /* 2D724 8003CF24 4C0786AF */  sw         $a2, %gp_rel(D_800A3818)($gp)
    /* 2D728 8003CF28 1C00A0AF */  sw         $zero, 0x1C($sp)
    /* 2D72C 8003CF2C 2152010C */  jal        func_80054884
    /* 2D730 8003CF30 1000A2AF */   sw        $v0, 0x10($sp)
    /* 2D734 8003CF34 21200000 */  addu       $a0, $zero, $zero
    /* 2D738 8003CF38 A205010C */  jal        func_80041688
    /* 2D73C 8003CF3C 21280000 */   addu      $a1, $zero, $zero
    /* 2D740 8003CF40 01000424 */  addiu      $a0, $zero, 0x1
    /* 2D744 8003CF44 A205010C */  jal        func_80041688
    /* 2D748 8003CF48 21280000 */   addu      $a1, $zero, $zero
    /* 2D74C 8003CF4C 5E84010C */  jal        game_Cleanup
    /* 2D750 8003CF50 00000000 */   nop
    /* 2D754 8003CF54 1D000224 */  addiu      $v0, $zero, 0x1D
    /* 2D758 8003CF58 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* 2D75C 8003CF5C B83720AC */  sw         $zero, %lo(D_800A37B8)($at)
    /* 2D760 8003CF60 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2D764 8003CF64 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 2D768 8003CF68 345A000C */  jal        gpu_DisableDisplay
    /* 2D76C 8003CF6C 00000000 */   nop
    /* 2D770 8003CF70 2400BF8F */  lw         $ra, 0x24($sp)
    /* 2D774 8003CF74 2000B08F */  lw         $s0, 0x20($sp)
    /* 2D778 8003CF78 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 2D77C 8003CF7C 0800E003 */  jr         $ra
    /* 2D780 8003CF80 00000000 */   nop
endlabel func_8003CE18
