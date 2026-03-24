glabel func_8008AEB0
    /* 7B6B0 8008AEB0 0A80023C */  lui        $v0, %hi(D_800A2878)
    /* 7B6B4 8008AEB4 7828428C */  lw         $v0, %lo(D_800A2878)($v0)
    /* 7B6B8 8008AEB8 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 7B6BC 8008AEBC 1400B1AF */  sw         $s1, 0x14($sp)
    /* 7B6C0 8008AEC0 21888000 */  addu       $s1, $a0, $zero
    /* 7B6C4 8008AEC4 1000B0AF */  sw         $s0, 0x10($sp)
    /* 7B6C8 8008AEC8 01001024 */  addiu      $s0, $zero, 0x1
    /* 7B6CC 8008AECC 06005010 */  beq        $v0, $s0, .L8008AEE8
    /* 7B6D0 8008AED0 1800BFAF */   sw        $ra, 0x18($sp)
    /* 7B6D4 8008AED4 0A80023C */  lui        $v0, %hi(D_800A2D10)
    /* 7B6D8 8008AED8 102D428C */  lw         $v0, %lo(D_800A2D10)($v0)
    /* 7B6DC 8008AEDC 00000000 */  nop
    /* 7B6E0 8008AEE0 03005014 */  bne        $v0, $s0, .L8008AEF0
    /* 7B6E4 8008AEE4 00000000 */   nop
  .L8008AEE8:
    /* 7B6E8 8008AEE8 D02B0208 */  j          .L8008AF40
    /* 7B6EC 8008AEEC 01000224 */   addiu     $v0, $zero, 0x1
  .L8008AEF0:
    /* 7B6F0 8008AEF0 0A80043C */  lui        $a0, %hi(D_800A2870)
    /* 7B6F4 8008AEF4 7028848C */  lw         $a0, %lo(D_800A2870)($a0)
    /* 7B6F8 8008AEF8 66E2010C */  jal        func_80078998
    /* 7B6FC 8008AEFC 00000000 */   nop
    /* 7B700 8008AF00 0B003016 */  bne        $s1, $s0, .L8008AF30
    /* 7B704 8008AF04 00000000 */   nop
    /* 7B708 8008AF08 0B004014 */  bnez       $v0, .L8008AF38
    /* 7B70C 8008AF0C 01000224 */   addiu     $v0, $zero, 0x1
  .L8008AF10:
    /* 7B710 8008AF10 0A80043C */  lui        $a0, %hi(D_800A2870)
    /* 7B714 8008AF14 7028848C */  lw         $a0, %lo(D_800A2870)($a0)
    /* 7B718 8008AF18 66E2010C */  jal        func_80078998
    /* 7B71C 8008AF1C 00000000 */   nop
    /* 7B720 8008AF20 FBFF4010 */  beqz       $v0, .L8008AF10
    /* 7B724 8008AF24 01000224 */   addiu     $v0, $zero, 0x1
    /* 7B728 8008AF28 CE2B0208 */  j          .L8008AF38
    /* 7B72C 8008AF2C 00000000 */   nop
  .L8008AF30:
    /* 7B730 8008AF30 03005014 */  bne        $v0, $s0, .L8008AF40
    /* 7B734 8008AF34 00000000 */   nop
  .L8008AF38:
    /* 7B738 8008AF38 0A80013C */  lui        $at, %hi(D_800A2D10)
    /* 7B73C 8008AF3C 102D22AC */  sw         $v0, %lo(D_800A2D10)($at)
  .L8008AF40:
    /* 7B740 8008AF40 1800BF8F */  lw         $ra, 0x18($sp)
    /* 7B744 8008AF44 1400B18F */  lw         $s1, 0x14($sp)
    /* 7B748 8008AF48 1000B08F */  lw         $s0, 0x10($sp)
    /* 7B74C 8008AF4C 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 7B750 8008AF50 0800E003 */  jr         $ra
    /* 7B754 8008AF54 00000000 */   nop
endlabel func_8008AEB0
