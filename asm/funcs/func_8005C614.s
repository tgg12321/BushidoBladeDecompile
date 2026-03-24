glabel func_8005C614
    /* 4CE14 8005C614 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 4CE18 8005C618 1000BFAF */  sw         $ra, 0x10($sp)
    /* 4CE1C 8005C61C 7F000424 */  addiu      $a0, $zero, 0x7F
    /* 4CE20 8005C620 F90E020C */  jal        func_80083BE4
    /* 4CE24 8005C624 7F000524 */   addiu     $a1, $zero, 0x7F
    /* 4CE28 8005C628 3416020C */  jal        func_800858D0
    /* 4CE2C 8005C62C 21200000 */   addu      $a0, $zero, $zero
    /* 4CE30 8005C630 C91F020C */  jal        func_80087F24
    /* 4CE34 8005C634 00000000 */   nop
    /* 4CE38 8005C638 C01F020C */  jal        func_80087F00
    /* 4CE3C 8005C63C 21200000 */   addu      $a0, $zero, $zero
    /* 4CE40 8005C640 1000BF8F */  lw         $ra, 0x10($sp)
    /* 4CE44 8005C644 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 4CE48 8005C648 0800E003 */  jr         $ra
    /* 4CE4C 8005C64C 00000000 */   nop
endlabel func_8005C614
