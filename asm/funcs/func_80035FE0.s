glabel func_80035FE0
    /* 267E0 80035FE0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 267E4 80035FE4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 267E8 80035FE8 DFFF010C */  jal        func_8007FF7C
    /* 267EC 80035FEC 00000000 */   nop
    /* 267F0 80035FF0 5A00020C */  jal        func_80080168
    /* 267F4 80035FF4 21200000 */   addu      $a0, $zero, $zero
    /* 267F8 80035FF8 21200000 */  addu       $a0, $zero, $zero
    /* 267FC 80035FFC 21280000 */  addu       $a1, $zero, $zero
    /* 26800 80036000 21300000 */  addu       $a2, $zero, $zero
    /* 26804 80036004 CCD7000C */  jal        func_80035F30
    /* 26808 80036008 21380000 */   addu      $a3, $zero, $zero
    /* 2680C 8003600C 18018293 */  lbu        $v0, %gp_rel(D_800A31E4)($gp)
    /* 26810 80036010 1080013C */  lui        $at, %hi(D_80101E62)
    /* 26814 80036014 621E20A4 */  sh         $zero, %lo(D_80101E62)($at)
    /* 26818 80036018 02004014 */  bnez       $v0, .L80036024
    /* 2681C 8003601C 01000224 */   addiu     $v0, $zero, 0x1
    /* 26820 80036020 180182A3 */  sb         $v0, %gp_rel(D_800A31E4)($gp)
  .L80036024:
    /* 26824 80036024 1000BF8F */  lw         $ra, 0x10($sp)
    /* 26828 80036028 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2682C 8003602C 0800E003 */  jr         $ra
    /* 26830 80036030 00000000 */   nop
endlabel func_80035FE0
