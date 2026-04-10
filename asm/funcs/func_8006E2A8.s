glabel func_8006E2A8
    /* 5EAA8 8006E2A8 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 5EAAC 8006E2AC 21200000 */  addu       $a0, $zero, $zero
    /* 5EAB0 8006E2B0 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* 5EAB4 8006E2B4 A8EC010C */  jal        gpu_SetDispMask
    /* 5EAB8 8006E2B8 1800B0AF */   sw        $s0, 0x18($sp)
    /* 5EABC 8006E2BC 4C04828F */  lw         $v0, %gp_rel(D_800A3518)($gp)
    /* 5EAC0 8006E2C0 0F80103C */  lui        $s0, %hi(D_800F7438)
    /* 5EAC4 8006E2C4 38741026 */  addiu      $s0, $s0, %lo(D_800F7438)
    /* 5EAC8 8006E2C8 01004230 */  andi       $v0, $v0, 0x1
    /* 5EACC 8006E2CC C0210200 */  sll        $a0, $v0, 7
    /* 5EAD0 8006E2D0 21208200 */  addu       $a0, $a0, $v0
    /* 5EAD4 8006E2D4 C0200400 */  sll        $a0, $a0, 3
    /* 5EAD8 8006E2D8 21208200 */  addu       $a0, $a0, $v0
    /* 5EADC 8006E2DC 00210400 */  sll        $a0, $a0, 4
    /* 5EAE0 8006E2E0 6CEE010C */  jal        func_8007B9B0
    /* 5EAE4 8006E2E4 21209000 */   addu      $a0, $a0, $s0
    /* 5EAE8 8006E2E8 4C04828F */  lw         $v0, %gp_rel(D_800A3518)($gp)
    /* 5EAEC 8006E2EC 5C001026 */  addiu      $s0, $s0, 0x5C
    /* 5EAF0 8006E2F0 01004230 */  andi       $v0, $v0, 0x1
    /* 5EAF4 8006E2F4 C0210200 */  sll        $a0, $v0, 7
    /* 5EAF8 8006E2F8 21208200 */  addu       $a0, $a0, $v0
    /* 5EAFC 8006E2FC C0200400 */  sll        $a0, $a0, 3
    /* 5EB00 8006E300 21208200 */  addu       $a0, $a0, $v0
    /* 5EB04 8006E304 00210400 */  sll        $a0, $a0, 4
    /* 5EB08 8006E308 02EF010C */  jal        func_8007BC08
    /* 5EB0C 8006E30C 21209000 */   addu      $a0, $a0, $s0
    /* 5EB10 8006E310 CFEC010C */  jal        gpu_DrawSync
    /* 5EB14 8006E314 21200000 */   addu      $a0, $zero, $zero
    /* 5EB18 8006E318 0A80053C */  lui        $a1, %hi(D_800A32E0)
    /* 5EB1C 8006E31C E032A524 */  addiu      $a1, $a1, %lo(D_800A32E0)
    /* 5EB20 8006E320 0300A288 */  lwl        $v0, 0x3($a1)
    /* 5EB24 8006E324 0000A298 */  lwr        $v0, 0x0($a1)
    /* 5EB28 8006E328 0700A388 */  lwl        $v1, 0x7($a1)
    /* 5EB2C 8006E32C 0400A398 */  lwr        $v1, 0x4($a1)
    /* 5EB30 8006E330 1300A2AB */  swl        $v0, 0x13($sp)
    /* 5EB34 8006E334 1000A2BB */  swr        $v0, 0x10($sp)
    /* 5EB38 8006E338 1700A3AB */  swl        $v1, 0x17($sp)
    /* 5EB3C 8006E33C 1400A3BB */  swr        $v1, 0x14($sp)
    /* 5EB40 8006E340 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5EB44 8006E344 21280000 */  addu       $a1, $zero, $zero
    /* 5EB48 8006E348 21300000 */  addu       $a2, $zero, $zero
    /* 5EB4C 8006E34C 34ED010C */  jal        func_8007B4D0
    /* 5EB50 8006E350 21380000 */   addu      $a3, $zero, $zero
    /* 5EB54 8006E354 CFEC010C */  jal        gpu_DrawSync
    /* 5EB58 8006E358 21200000 */   addu      $a0, $zero, $zero
    /* 5EB5C 8006E35C 01000424 */  addiu      $a0, $zero, 0x1
    /* 5EB60 8006E360 21280000 */  addu       $a1, $zero, $zero
    /* 5EB64 8006E364 21300000 */  addu       $a2, $zero, $zero
    /* 5EB68 8006E368 DA59000C */  jal        disp_SetFramebufferMode
    /* 5EB6C 8006E36C 21380000 */   addu      $a3, $zero, $zero
    /* 5EB70 8006E370 A8EC010C */  jal        gpu_SetDispMask
    /* 5EB74 8006E374 01000424 */   addiu     $a0, $zero, 0x1
    /* 5EB78 8006E378 01000224 */  addiu      $v0, $zero, 0x1
    /* 5EB7C 8006E37C 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* 5EB80 8006E380 1800B08F */  lw         $s0, 0x18($sp)
    /* 5EB84 8006E384 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 5EB88 8006E388 0800E003 */  jr         $ra
    /* 5EB8C 8006E38C 00000000 */   nop
endlabel func_8006E2A8
