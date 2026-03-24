glabel func_80037540
    /* 27D40 80037540 B8FFBD27 */  addiu      $sp, $sp, -0x48
    /* 27D44 80037544 3800B2AF */  sw         $s2, 0x38($sp)
    /* 27D48 80037548 21908000 */  addu       $s2, $a0, $zero
    /* 27D4C 8003754C 3C00B3AF */  sw         $s3, 0x3C($sp)
    /* 27D50 80037550 2198A000 */  addu       $s3, $a1, $zero
    /* 27D54 80037554 06000424 */  addiu      $a0, $zero, 0x6
    /* 27D58 80037558 4000B4AF */  sw         $s4, 0x40($sp)
    /* 27D5C 8003755C 5800B48F */  lw         $s4, 0x58($sp)
    /* 27D60 80037560 2128C000 */  addu       $a1, $a2, $zero
    /* 27D64 80037564 3400B1AF */  sw         $s1, 0x34($sp)
    /* 27D68 80037568 2188E000 */  addu       $s1, $a3, $zero
    /* 27D6C 8003756C 4400BFAF */  sw         $ra, 0x44($sp)
    /* 27D70 80037570 AADB000C */  jal        func_80036EA8
    /* 27D74 80037574 3000B0AF */   sw        $s0, 0x30($sp)
    /* 27D78 80037578 06000424 */  addiu      $a0, $zero, 0x6
    /* 27D7C 8003757C 02000524 */  addiu      $a1, $zero, 0x2
    /* 27D80 80037580 C0100200 */  sll        $v0, $v0, 3
    /* 27D84 80037584 0980103C */  lui        $s0, %hi(D_8008EC34)
    /* 27D88 80037588 34EC1026 */  addiu      $s0, $s0, %lo(D_8008EC34)
    /* 27D8C 8003758C 21105000 */  addu       $v0, $v0, $s0
    /* 27D90 80037590 1000A2AF */  sw         $v0, 0x10($sp)
    /* 27D94 80037594 1400B1AF */  sw         $s1, 0x14($sp)
    /* 27D98 80037598 1800B2AF */  sw         $s2, 0x18($sp)
    /* 27D9C 8003759C AADB000C */  jal        func_80036EA8
    /* 27DA0 800375A0 1C00B3AF */   sw        $s3, 0x1C($sp)
    /* 27DA4 800375A4 C0100200 */  sll        $v0, $v0, 3
    /* 27DA8 800375A8 21105000 */  addu       $v0, $v0, $s0
    /* 27DAC 800375AC 2000A2AF */  sw         $v0, 0x20($sp)
    /* 27DB0 800375B0 AEE4000C */  jal        func_800392B8
    /* 27DB4 800375B4 2400B4AF */   sw        $s4, 0x24($sp)
    /* 27DB8 800375B8 06000424 */  addiu      $a0, $zero, 0x6
    /* 27DBC 800375BC 1000A527 */  addiu      $a1, $sp, 0x10
    /* 27DC0 800375C0 1ADD000C */  jal        func_80037468
    /* 27DC4 800375C4 FC074624 */   addiu     $a2, $v0, 0x7FC
    /* 27DC8 800375C8 4400BF8F */  lw         $ra, 0x44($sp)
    /* 27DCC 800375CC 4000B48F */  lw         $s4, 0x40($sp)
    /* 27DD0 800375D0 3C00B38F */  lw         $s3, 0x3C($sp)
    /* 27DD4 800375D4 3800B28F */  lw         $s2, 0x38($sp)
    /* 27DD8 800375D8 3400B18F */  lw         $s1, 0x34($sp)
    /* 27DDC 800375DC 3000B08F */  lw         $s0, 0x30($sp)
    /* 27DE0 800375E0 4800BD27 */  addiu      $sp, $sp, 0x48
    /* 27DE4 800375E4 0800E003 */  jr         $ra
    /* 27DE8 800375E8 00000000 */   nop
endlabel func_80037540
