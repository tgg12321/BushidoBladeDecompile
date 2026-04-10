glabel obj_ExecTask
    /* 4C26C 8005BA6C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 4C270 8005BA70 1000BFAF */  sw         $ra, 0x10($sp)
    /* 4C274 8005BA74 3071010C */  jal        func_8005C4C0
    /* 4C278 8005BA78 09000524 */   addiu     $a1, $zero, 0x9
    /* 4C27C 8005BA7C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 4C280 8005BA80 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 4C284 8005BA84 0800E003 */  jr         $ra
    /* 4C288 8005BA88 00000000 */   nop
endlabel obj_ExecTask
