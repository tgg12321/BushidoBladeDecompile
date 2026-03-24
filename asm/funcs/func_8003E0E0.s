glabel func_8003E0E0
    /* 2E8E0 8003E0E0 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 2E8E4 8003E0E4 E1010224 */  addiu      $v0, $zero, 0x1E1
    /* 2E8E8 8003E0E8 1200A2A7 */  sh         $v0, 0x12($sp)
    /* 2E8EC 8003E0EC 40010224 */  addiu      $v0, $zero, 0x140
    /* 2E8F0 8003E0F0 1400A2A7 */  sh         $v0, 0x14($sp)
    /* 2E8F4 8003E0F4 01000224 */  addiu      $v0, $zero, 0x1
    /* 2E8F8 8003E0F8 1000A427 */  addiu      $a0, $sp, 0x10
    /* 2E8FC 8003E0FC 1F000524 */  addiu      $a1, $zero, 0x1F
    /* 2E900 8003E100 1800BFAF */  sw         $ra, 0x18($sp)
    /* 2E904 8003E104 1000A0A7 */  sh         $zero, 0x10($sp)
    /* 2E908 8003E108 85F7000C */  jal        func_8003DE14
    /* 2E90C 8003E10C 1600A2A7 */   sh        $v0, 0x16($sp)
    /* 2E910 8003E110 1800BF8F */  lw         $ra, 0x18($sp)
    /* 2E914 8003E114 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 2E918 8003E118 0800E003 */  jr         $ra
    /* 2E91C 8003E11C 00000000 */   nop
endlabel func_8003E0E0
