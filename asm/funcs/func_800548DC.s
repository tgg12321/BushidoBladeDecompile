glabel func_800548DC
    /* 450DC 800548DC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 450E0 800548E0 1000BFAF */  sw         $ra, 0x10($sp)
    /* 450E4 800548E4 CFEC010C */  jal        func_8007B33C
    /* 450E8 800548E8 21200000 */   addu      $a0, $zero, $zero
    /* 450EC 800548EC 6719010C */  jal        func_8004659C
    /* 450F0 800548F0 FFFF0424 */   addiu     $a0, $zero, -0x1
    /* 450F4 800548F4 981A010C */  jal        snd_StopSelection
    /* 450F8 800548F8 00000000 */   nop
    /* 450FC 800548FC 1000BF8F */  lw         $ra, 0x10($sp)
    /* 45100 80054900 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 45104 80054904 0800E003 */  jr         $ra
    /* 45108 80054908 00000000 */   nop
endlabel func_800548DC
