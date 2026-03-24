glabel func_8003AAB0
    /* 2B2B0 8003AAB0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2B2B4 8003AAB4 02000224 */  addiu      $v0, $zero, 0x2
    /* 2B2B8 8003AAB8 1400BFAF */  sw         $ra, 0x14($sp)
    /* 2B2BC 8003AABC 1000B0AF */  sw         $s0, 0x10($sp)
    /* 2B2C0 8003AAC0 A40782AF */  sw         $v0, %gp_rel(D_800A3870)($gp)
    /* 2B2C4 8003AAC4 330A020C */  jal        func_800828CC
    /* 2B2C8 8003AAC8 02000424 */   addiu     $a0, $zero, 0x2
    /* 2B2CC 8003AACC 02001024 */  addiu      $s0, $zero, 0x2
  .L8003AAD0:
    /* 2B2D0 8003AAD0 92EA000C */  jal        func_8003AA48
    /* 2B2D4 8003AAD4 00000000 */   nop
    /* 2B2D8 8003AAD8 40018293 */  lbu        $v0, %gp_rel(D_800A320C)($gp)
    /* 2B2DC 8003AADC 00000000 */  nop
    /* 2B2E0 8003AAE0 0D004010 */  beqz       $v0, .L8003AB18
    /* 2B2E4 8003AAE4 00F2043C */   lui       $a0, (0xF2000001 >> 16)
    /* 2B2E8 8003AAE8 EAE2010C */  jal        func_80078BA8
    /* 2B2EC 8003AAEC 01008434 */   ori       $a0, $a0, (0xF2000001 & 0xFFFF)
  .L8003AAF0:
    /* 2B2F0 8003AAF0 00F2043C */  lui        $a0, (0xF2000001 >> 16)
    /* 2B2F4 8003AAF4 C1E2010C */  jal        func_80078B04
    /* 2B2F8 8003AAF8 01008434 */   ori       $a0, $a0, (0xF2000001 & 0xFFFF)
    /* 2B2FC 8003AAFC 00014228 */  slti       $v0, $v0, 0x100
    /* 2B300 8003AB00 FBFF4014 */  bnez       $v0, .L8003AAF0
    /* 2B304 8003AB04 00000000 */   nop
    /* 2B308 8003AB08 A407828F */  lw         $v0, %gp_rel(D_800A3870)($gp)
    /* 2B30C 8003AB0C 00000000 */  nop
    /* 2B310 8003AB10 EFFF5010 */  beq        $v0, $s0, .L8003AAD0
    /* 2B314 8003AB14 00000000 */   nop
  .L8003AB18:
    /* 2B318 8003AB18 330A020C */  jal        func_800828CC
    /* 2B31C 8003AB1C 02000424 */   addiu     $a0, $zero, 0x2
    /* 2B320 8003AB20 92EA000C */  jal        func_8003AA48
    /* 2B324 8003AB24 00000000 */   nop
    /* 2B328 8003AB28 330A020C */  jal        func_800828CC
    /* 2B32C 8003AB2C 02000424 */   addiu     $a0, $zero, 0x2
    /* 2B330 8003AB30 1400BF8F */  lw         $ra, 0x14($sp)
    /* 2B334 8003AB34 1000B08F */  lw         $s0, 0x10($sp)
    /* 2B338 8003AB38 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2B33C 8003AB3C 0800E003 */  jr         $ra
    /* 2B340 8003AB40 00000000 */   nop
endlabel func_8003AAB0
