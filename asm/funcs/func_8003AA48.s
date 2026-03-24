glabel func_8003AA48
    /* 2B248 8003AA48 D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* 2B24C 8003AA4C 04000224 */  addiu      $v0, $zero, 0x4
    /* 2B250 8003AA50 1000A427 */  addiu      $a0, $sp, 0x10
    /* 2B254 8003AA54 2800BFAF */  sw         $ra, 0x28($sp)
    /* 2B258 8003AA58 1800A0AF */  sw         $zero, 0x18($sp)
    /* 2B25C 8003AA5C 1200A2A7 */  sh         $v0, 0x12($sp)
    /* 2B260 8003AA60 CAE9000C */  jal        func_8003A728
    /* 2B264 8003AA64 1000A2A7 */   sh        $v0, 0x10($sp)
    /* 2B268 8003AA68 2800BF8F */  lw         $ra, 0x28($sp)
    /* 2B26C 8003AA6C 3000BD27 */  addiu      $sp, $sp, 0x30
    /* 2B270 8003AA70 0800E003 */  jr         $ra
    /* 2B274 8003AA74 00000000 */   nop
endlabel func_8003AA48
