glabel func_8008BE4C
    /* 7C64C 8008BE4C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 7C650 8008BE50 1400BFAF */  sw         $ra, 0x14($sp)
    /* 7C654 8008BE54 6EE2010C */  jal        func_800789B8
    /* 7C658 8008BE58 1000B0AF */   sw        $s0, 0x10($sp)
    /* 7C65C 8008BE5C 0180043C */  lui        $a0, %hi(D_800164A8)
    /* 7C660 8008BE60 A8648424 */  addiu      $a0, $a0, %lo(D_800164A8)
    /* 7C664 8008BE64 1834020C */  jal        func_8008D060
    /* 7C668 8008BE68 21804000 */   addu      $s0, $v0, $zero
    /* 7C66C 8008BE6C FCE3010C */  jal        func_80078FF0
    /* 7C670 8008BE70 00000000 */   nop
    /* 7C674 8008BE74 01000224 */  addiu      $v0, $zero, 0x1
    /* 7C678 8008BE78 03000216 */  bne        $s0, $v0, .L8008BE88
    /* 7C67C 8008BE7C 00000000 */   nop
    /* 7C680 8008BE80 72E2010C */  jal        func_800789C8
    /* 7C684 8008BE84 00000000 */   nop
  .L8008BE88:
    /* 7C688 8008BE88 1400BF8F */  lw         $ra, 0x14($sp)
    /* 7C68C 8008BE8C 1000B08F */  lw         $s0, 0x10($sp)
    /* 7C690 8008BE90 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7C694 8008BE94 0800E003 */  jr         $ra
    /* 7C698 8008BE98 00000000 */   nop
    /* 7C69C 8008BE9C 0800E003 */  jr         $ra
    /* 7C6A0 8008BEA0 00000000 */   nop
endlabel func_8008BE4C
