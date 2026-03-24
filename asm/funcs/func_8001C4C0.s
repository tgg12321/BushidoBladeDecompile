glabel func_8001C4C0
    /* CCC0 8001C4C0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* CCC4 8001C4C4 1080033C */  lui        $v1, %hi(D_80101F32)
    /* CCC8 8001C4C8 321F6394 */  lhu        $v1, %lo(D_80101F32)($v1)
    /* CCCC 8001C4CC 32000224 */  addiu      $v0, $zero, 0x32
    /* CCD0 8001C4D0 04006210 */  beq        $v1, $v0, .L8001C4E4
    /* CCD4 8001C4D4 1000BFAF */   sw        $ra, 0x10($sp)
    /* CCD8 8001C4D8 11000224 */  addiu      $v0, $zero, 0x11
    /* CCDC 8001C4DC 0B006214 */  bne        $v1, $v0, .L8001C50C
    /* CCE0 8001C4E0 00000000 */   nop
  .L8001C4E4:
    /* CCE4 8001C4E4 3286000C */  jal        func_800218C8
    /* CCE8 8001C4E8 21200000 */   addu      $a0, $zero, $zero
    /* CCEC 8001C4EC 5D86000C */  jal        func_80021974
    /* CCF0 8001C4F0 21200000 */   addu      $a0, $zero, $zero
    /* CCF4 8001C4F4 1080013C */  lui        $at, %hi(D_80101F26)
    /* CCF8 8001C4F8 261F20A4 */  sh         $zero, %lo(D_80101F26)($at)
    /* CCFC 8001C4FC 21200000 */  addu       $a0, $zero, $zero
    /* CD00 8001C500 21284000 */  addu       $a1, $v0, $zero
    /* CD04 8001C504 A686000C */  jal        func_80021A98
    /* CD08 8001C508 21300000 */   addu      $a2, $zero, $zero
  .L8001C50C:
    /* CD0C 8001C50C 1000BF8F */  lw         $ra, 0x10($sp)
    /* CD10 8001C510 1800BD27 */  addiu      $sp, $sp, 0x18
    /* CD14 8001C514 0800E003 */  jr         $ra
    /* CD18 8001C518 00000000 */   nop
endlabel func_8001C4C0
