glabel func_80037B90
    /* 28390 80037B90 B8FFBD27 */  addiu      $sp, $sp, -0x48
    /* 28394 80037B94 21108000 */  addu       $v0, $a0, $zero
    /* 28398 80037B98 2118A000 */  addu       $v1, $a1, $zero
    /* 2839C 80037B9C 1800A427 */  addiu      $a0, $sp, 0x18
    /* 283A0 80037BA0 0180053C */  lui        $a1, %hi(D_800109BC)
    /* 283A4 80037BA4 BC09A524 */  addiu      $a1, $a1, %lo(D_800109BC)
    /* 283A8 80037BA8 1000A6AF */  sw         $a2, 0x10($sp)
    /* 283AC 80037BAC 21304000 */  addu       $a2, $v0, $zero
    /* 283B0 80037BB0 3C00B1AF */  sw         $s1, 0x3C($sp)
    /* 283B4 80037BB4 2188E000 */  addu       $s1, $a3, $zero
    /* 283B8 80037BB8 21386000 */  addu       $a3, $v1, $zero
    /* 283BC 80037BBC 4000BFAF */  sw         $ra, 0x40($sp)
    /* 283C0 80037BC0 8CE6010C */  jal        func_80079A30
    /* 283C4 80037BC4 3800B0AF */   sw        $s0, 0x38($sp)
    /* 283C8 80037BC8 1800A427 */  addiu      $a0, $sp, 0x18
    /* 283CC 80037BCC 7AE2010C */  jal        func_800789E8
    /* 283D0 80037BD0 01800534 */   ori       $a1, $zero, 0x8001
    /* 283D4 80037BD4 21804000 */  addu       $s0, $v0, $zero
    /* 283D8 80037BD8 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 283DC 80037BDC 0F000212 */  beq        $s0, $v0, .L80037C1C
    /* 283E0 80037BE0 00000000 */   nop
    /* 283E4 80037BE4 C80690AF */  sw         $s0, %gp_rel(D_800A3794)($gp)
    /* 283E8 80037BE8 47DE000C */  jal        func_8003791C
    /* 283EC 80037BEC 00000000 */   nop
    /* 283F0 80037BF0 76DE000C */  jal        func_800379D8
    /* 283F4 80037BF4 00000000 */   nop
    /* 283F8 80037BF8 21200002 */  addu       $a0, $s0, $zero
    /* 283FC 80037BFC 5800A68F */  lw         $a2, 0x58($sp)
    /* 28400 80037C00 7EE2010C */  jal        func_800789F8
    /* 28404 80037C04 21282002 */   addu      $a1, $s1, $zero
    /* 28408 80037C08 59DE000C */  jal        func_80037964
    /* 2840C 80037C0C 00000000 */   nop
    /* 28410 80037C10 01004238 */  xori       $v0, $v0, 0x1
    /* 28414 80037C14 2B100200 */  sltu       $v0, $zero, $v0
    /* 28418 80037C18 23100200 */  negu       $v0, $v0
  .L80037C1C:
    /* 2841C 80037C1C 4000BF8F */  lw         $ra, 0x40($sp)
    /* 28420 80037C20 3C00B18F */  lw         $s1, 0x3C($sp)
    /* 28424 80037C24 3800B08F */  lw         $s0, 0x38($sp)
    /* 28428 80037C28 4800BD27 */  addiu      $sp, $sp, 0x48
    /* 2842C 80037C2C 0800E003 */  jr         $ra
    /* 28430 80037C30 00000000 */   nop
endlabel func_80037B90
