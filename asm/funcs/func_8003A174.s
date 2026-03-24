glabel func_8003A174
    /* 2A974 8003A174 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2A978 8003A178 1400BFAF */  sw         $ra, 0x14($sp)
    /* 2A97C 8003A17C 6EE2010C */  jal        func_800789B8
    /* 2A980 8003A180 1000B0AF */   sw        $s0, 0x10($sp)
    /* 2A984 8003A184 FFFF1024 */  addiu      $s0, $zero, -0x1
    /* 2A988 8003A188 00F0043C */  lui        $a0, (0xF000000B >> 16)
  .L8003A18C:
    /* 2A98C 8003A18C 0B008434 */  ori        $a0, $a0, (0xF000000B & 0xFFFF)
    /* 2A990 8003A190 00040524 */  addiu      $a1, $zero, 0x400
    /* 2A994 8003A194 00200624 */  addiu      $a2, $zero, 0x2000
    /* 2A998 8003A198 5EE2010C */  jal        func_80078978
    /* 2A99C 8003A19C 21380000 */   addu      $a3, $zero, $zero
    /* 2A9A0 8003A1A0 6C0682AF */  sw         $v0, %gp_rel(D_800A3738)($gp)
    /* 2A9A4 8003A1A4 F9FF5010 */  beq        $v0, $s0, .L8003A18C
    /* 2A9A8 8003A1A8 00F0043C */   lui       $a0, (0xF000000B >> 16)
    /* 2A9AC 8003A1AC FFFF1024 */  addiu      $s0, $zero, -0x1
  .L8003A1B0:
    /* 2A9B0 8003A1B0 0B008434 */  ori        $a0, $a0, (0xF000000B & 0xFFFF)
    /* 2A9B4 8003A1B4 00800534 */  ori        $a1, $zero, 0x8000
    /* 2A9B8 8003A1B8 00200624 */  addiu      $a2, $zero, 0x2000
    /* 2A9BC 8003A1BC 5EE2010C */  jal        func_80078978
    /* 2A9C0 8003A1C0 21380000 */   addu      $a3, $zero, $zero
    /* 2A9C4 8003A1C4 440782AF */  sw         $v0, %gp_rel(D_800A3810)($gp)
    /* 2A9C8 8003A1C8 F9FF5010 */  beq        $v0, $s0, .L8003A1B0
    /* 2A9CC 8003A1CC 00F0043C */   lui       $a0, (0xF000000B >> 16)
    /* 2A9D0 8003A1D0 72E2010C */  jal        func_800789C8
    /* 2A9D4 8003A1D4 FFFF1024 */   addiu     $s0, $zero, -0x1
    /* 2A9D8 8003A1D8 330A020C */  jal        func_800828CC
    /* 2A9DC 8003A1DC 02000424 */   addiu     $a0, $zero, 0x2
    /* 2A9E0 8003A1E0 812F020C */  jal        func_8008BE04
    /* 2A9E4 8003A1E4 00000000 */   nop
  .L8003A1E8:
    /* 2A9E8 8003A1E8 0A80043C */  lui        $a0, %hi(D_800A3210)
    /* 2A9EC 8003A1EC 10328424 */  addiu      $a0, $a0, %lo(D_800A3210)
    /* 2A9F0 8003A1F0 7AE2010C */  jal        func_800789E8
    /* 2A9F4 8003A1F4 02000524 */   addiu     $a1, $zero, 0x2
    /* 2A9F8 8003A1F8 700682AF */  sw         $v0, %gp_rel(D_800A373C)($gp)
    /* 2A9FC 8003A1FC FAFF5010 */  beq        $v0, $s0, .L8003A1E8
    /* 2AA00 8003A200 00000000 */   nop
    /* 2AA04 8003A204 FFFF1024 */  addiu      $s0, $zero, -0x1
  .L8003A208:
    /* 2AA08 8003A208 0A80043C */  lui        $a0, %hi(D_800A3210)
    /* 2AA0C 8003A20C 10328424 */  addiu      $a0, $a0, %lo(D_800A3210)
    /* 2AA10 8003A210 7AE2010C */  jal        func_800789E8
    /* 2AA14 8003A214 01800534 */   ori       $a1, $zero, 0x8001
    /* 2AA18 8003A218 680682AF */  sw         $v0, %gp_rel(D_800A3734)($gp)
    /* 2AA1C 8003A21C FAFF5010 */  beq        $v0, $s0, .L8003A208
    /* 2AA20 8003A220 02000424 */   addiu     $a0, $zero, 0x2
    /* 2AA24 8003A224 21280000 */  addu       $a1, $zero, $zero
    /* 2AA28 8003A228 1931020C */  jal        func_8008C464
    /* 2AA2C 8003A22C 21300000 */   addu      $a2, $zero, $zero
    /* 2AA30 8003A230 01000424 */  addiu      $a0, $zero, 0x1
    /* 2AA34 8003A234 03000524 */  addiu      $a1, $zero, 0x3
    /* 2AA38 8003A238 1931020C */  jal        func_8008C464
    /* 2AA3C 8003A23C 00E10634 */   ori       $a2, $zero, 0xE100
    /* 2AA40 8003A240 01000424 */  addiu      $a0, $zero, 0x1
    /* 2AA44 8003A244 04000524 */  addiu      $a1, $zero, 0x4
    /* 2AA48 8003A248 1931020C */  jal        func_8008C464
    /* 2AA4C 8003A24C 01000624 */   addiu     $a2, $zero, 0x1
    /* 2AA50 8003A250 1400BF8F */  lw         $ra, 0x14($sp)
    /* 2AA54 8003A254 1000B08F */  lw         $s0, 0x10($sp)
    /* 2AA58 8003A258 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2AA5C 8003A25C 0800E003 */  jr         $ra
    /* 2AA60 8003A260 00000000 */   nop
endlabel func_8003A174
