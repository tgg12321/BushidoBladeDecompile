glabel func_8008AE24
    /* 7B624 8008AE24 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 7B628 8008AE28 21288000 */  addu       $a1, $a0, $zero
    /* 7B62C 8008AE2C 0700023C */  lui        $v0, (0x7EFE8 >> 16)
    /* 7B630 8008AE30 E8EF4234 */  ori        $v0, $v0, (0x7EFE8 & 0xFFFF)
    /* 7B634 8008AE34 F0EFA324 */  addiu      $v1, $a1, -0x1010
    /* 7B638 8008AE38 2B104300 */  sltu       $v0, $v0, $v1
    /* 7B63C 8008AE3C 0A004014 */  bnez       $v0, .L8008AE68
    /* 7B640 8008AE40 1000BFAF */   sw        $ra, 0x10($sp)
    /* 7B644 8008AE44 3524020C */  jal        func_800890D4
    /* 7B648 8008AE48 FFFF0424 */   addiu     $a0, $zero, -0x1
    /* 7B64C 8008AE4C 0A80033C */  lui        $v1, %hi(D_800A2D04)
    /* 7B650 8008AE50 042D638C */  lw         $v1, %lo(D_800A2D04)($v1)
    /* 7B654 8008AE54 0A80013C */  lui        $at, %hi(D_800A2CF4)
    /* 7B658 8008AE58 F42C22A4 */  sh         $v0, %lo(D_800A2CF4)($at)
    /* 7B65C 8008AE5C FFFF4230 */  andi       $v0, $v0, 0xFFFF
    /* 7B660 8008AE60 9B2B0208 */  j          .L8008AE6C
    /* 7B664 8008AE64 04106200 */   sllv      $v0, $v0, $v1
  .L8008AE68:
    /* 7B668 8008AE68 21100000 */  addu       $v0, $zero, $zero
  .L8008AE6C:
    /* 7B66C 8008AE6C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 7B670 8008AE70 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7B674 8008AE74 0800E003 */  jr         $ra
    /* 7B678 8008AE78 00000000 */   nop
endlabel func_8008AE24
