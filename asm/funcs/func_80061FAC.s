glabel func_80061FAC
    /* 527AC 80061FAC 21188000 */  addu       $v1, $a0, $zero
    /* 527B0 80061FB0 2004848F */  lw         $a0, %gp_rel(D_800A34EC)($gp)
    /* 527B4 80061FB4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 527B8 80061FB8 1400BFAF */  sw         $ra, 0x14($sp)
    /* 527BC 80061FBC 1000B0AF */  sw         $s0, 0x10($sp)
    /* 527C0 80061FC0 00006294 */  lhu        $v0, 0x0($v1)
    /* 527C4 80061FC4 00000000 */  nop
    /* 527C8 80061FC8 000082A4 */  sh         $v0, 0x0($a0)
    /* 527CC 80061FCC 02006294 */  lhu        $v0, 0x2($v1)
    /* 527D0 80061FD0 2180C000 */  addu       $s0, $a2, $zero
    /* 527D4 80061FD4 020082A4 */  sh         $v0, 0x2($a0)
    /* 527D8 80061FD8 04006294 */  lhu        $v0, 0x4($v1)
    /* 527DC 80061FDC 21280002 */  addu       $a1, $s0, $zero
    /* 527E0 80061FE0 D7FC010C */  jal        func_8007F35C
    /* 527E4 80061FE4 040082A4 */   sh        $v0, 0x4($a0)
    /* 527E8 80061FE8 21200002 */  addu       $a0, $s0, $zero
    /* 527EC 80061FEC 0A80053C */  lui        $a1, %hi(D_8009BB74)
    /* 527F0 80061FF0 74BBA524 */  addiu      $a1, $a1, %lo(D_8009BB74)
    /* 527F4 80061FF4 1C0000AE */  sw         $zero, 0x1C($s0)
    /* 527F8 80061FF8 180000AE */  sw         $zero, 0x18($s0)
    /* 527FC 80061FFC 37FA010C */  jal        func_8007E8DC
    /* 52800 80062000 140000AE */   sw        $zero, 0x14($s0)
    /* 52804 80062004 BBFB010C */  jal        gte_SetRotMatrix
    /* 52808 80062008 21200002 */   addu      $a0, $s0, $zero
    /* 5280C 8006200C 1400BF8F */  lw         $ra, 0x14($sp)
    /* 52810 80062010 1000B08F */  lw         $s0, 0x10($sp)
    /* 52814 80062014 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 52818 80062018 0800E003 */  jr         $ra
    /* 5281C 8006201C 00000000 */   nop
endlabel func_80061FAC
