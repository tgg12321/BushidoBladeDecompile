glabel func_8003E120
    /* 2E920 8003E120 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 2E924 8003E124 40010224 */  addiu      $v0, $zero, 0x140
    /* 2E928 8003E128 1000A2A7 */  sh         $v0, 0x10($sp)
    /* 2E92C 8003E12C E8010224 */  addiu      $v0, $zero, 0x1E8
    /* 2E930 8003E130 1200A2A7 */  sh         $v0, 0x12($sp)
    /* 2E934 8003E134 40000224 */  addiu      $v0, $zero, 0x40
    /* 2E938 8003E138 1400A2A7 */  sh         $v0, 0x14($sp)
    /* 2E93C 8003E13C 08000224 */  addiu      $v0, $zero, 0x8
    /* 2E940 8003E140 1000A427 */  addiu      $a0, $sp, 0x10
    /* 2E944 8003E144 13000524 */  addiu      $a1, $zero, 0x13
    /* 2E948 8003E148 1800BFAF */  sw         $ra, 0x18($sp)
    /* 2E94C 8003E14C 85F7000C */  jal        func_8003DE14
    /* 2E950 8003E150 1600A2A7 */   sh        $v0, 0x16($sp)
    /* 2E954 8003E154 1800BF8F */  lw         $ra, 0x18($sp)
    /* 2E958 8003E158 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 2E95C 8003E15C 0800E003 */  jr         $ra
    /* 2E960 8003E160 00000000 */   nop
endlabel func_8003E120
