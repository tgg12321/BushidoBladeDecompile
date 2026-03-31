glabel func_8003D91C
    /* 2E11C 8003D91C D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 2E120 8003D920 1000A427 */  addiu      $a0, $sp, 0x10
    /* 2E124 8003D924 1F000524 */  addiu      $a1, $zero, 0x1F
    /* 2E128 8003D928 0A80063C */  lui        $a2, %hi(light_effect_col)
    /* 2E12C 8003D92C 703DC624 */  addiu      $a2, $a2, %lo(light_effect_col)
    /* 2E130 8003D930 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 2E134 8003D934 E0011124 */  addiu      $s1, $zero, 0x1E0
    /* 2E138 8003D938 1800B0AF */  sw         $s0, 0x18($sp)
    /* 2E13C 8003D93C 40011024 */  addiu      $s0, $zero, 0x140
    /* 2E140 8003D940 01000224 */  addiu      $v0, $zero, 0x1
    /* 2E144 8003D944 2000BFAF */  sw         $ra, 0x20($sp)
    /* 2E148 8003D948 1000A0A7 */  sh         $zero, 0x10($sp)
    /* 2E14C 8003D94C 1200B1A7 */  sh         $s1, 0x12($sp)
    /* 2E150 8003D950 1400B0A7 */  sh         $s0, 0x14($sp)
    /* 2E154 8003D954 68F6000C */  jal        func_8003D9A0
    /* 2E158 8003D958 1600A2A7 */   sh        $v0, 0x16($sp)
    /* 2E15C 8003D95C 1000A427 */  addiu      $a0, $sp, 0x10
    /* 2E160 8003D960 13000524 */  addiu      $a1, $zero, 0x13
    /* 2E164 8003D964 0A80063C */  lui        $a2, %hi(D_800A4340)
    /* 2E168 8003D968 4043C624 */  addiu      $a2, $a2, %lo(D_800A4340)
    /* 2E16C 8003D96C 40000224 */  addiu      $v0, $zero, 0x40
    /* 2E170 8003D970 1400A2A7 */  sh         $v0, 0x14($sp)
    /* 2E174 8003D974 08000224 */  addiu      $v0, $zero, 0x8
    /* 2E178 8003D978 1000B0A7 */  sh         $s0, 0x10($sp)
    /* 2E17C 8003D97C 1200B1A7 */  sh         $s1, 0x12($sp)
    /* 2E180 8003D980 68F6000C */  jal        func_8003D9A0
    /* 2E184 8003D984 1600A2A7 */   sh        $v0, 0x16($sp)
    /* 2E188 8003D988 2000BF8F */  lw         $ra, 0x20($sp)
    /* 2E18C 8003D98C 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 2E190 8003D990 1800B08F */  lw         $s0, 0x18($sp)
    /* 2E194 8003D994 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 2E198 8003D998 0800E003 */  jr         $ra
    /* 2E19C 8003D99C 00000000 */   nop
endlabel func_8003D91C
