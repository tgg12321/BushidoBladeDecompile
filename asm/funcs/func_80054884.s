glabel func_80054884
    /* 45084 80054884 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 45088 80054888 3C00A28F */  lw         $v0, 0x3C($sp)
    /* 4508C 8005488C 4000A38F */  lw         $v1, 0x40($sp)
    /* 45090 80054890 40200400 */  sll        $a0, $a0, 1
    /* 45094 80054894 2000BFAF */  sw         $ra, 0x20($sp)
    /* 45098 80054898 0A80013C */  lui        $at, %hi(InfoPosYTbl1)
    /* 4509C 8005489C 21082400 */  addu       $at, $at, $a0
    /* 450A0 800548A0 549D2484 */  lh         $a0, %lo(InfoPosYTbl1)($at)
    /* 450A4 800548A4 4400A88F */  lw         $t0, 0x44($sp)
    /* 450A8 800548A8 21208500 */  addu       $a0, $a0, $a1
    /* 450AC 800548AC 2128C000 */  addu       $a1, $a2, $zero
    /* 450B0 800548B0 2130E000 */  addu       $a2, $a3, $zero
    /* 450B4 800548B4 1000A2AF */  sw         $v0, 0x10($sp)
    /* 450B8 800548B8 1400A3AF */  sw         $v1, 0x14($sp)
    /* 450BC 800548BC 1800A8AF */  sw         $t0, 0x18($sp)
    /* 450C0 800548C0 3800A78F */  lw         $a3, 0x38($sp)
    /* 450C4 800548C4 8151010C */  jal        func_80054604
    /* 450C8 800548C8 CFFE8424 */   addiu     $a0, $a0, -0x131
    /* 450CC 800548CC 2000BF8F */  lw         $ra, 0x20($sp)
    /* 450D0 800548D0 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 450D4 800548D4 0800E003 */  jr         $ra
    /* 450D8 800548D8 00000000 */   nop
endlabel func_80054884
