glabel func_80016A18
    /* 7218 80016A18 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 721C 80016A1C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 7220 80016A20 B00A020C */  jal        irq_DisableInterrupts
    /* 7224 80016A24 00000000 */   nop
    /* 7228 80016A28 1080043C */  lui        $a0, %hi(D_800FF580)
    /* 722C 80016A2C 80F58424 */  addiu      $a0, $a0, %lo(D_800FF580)
    /* 7230 80016A30 08000524 */  addiu      $a1, $zero, 0x8
    /* 7234 80016A34 24008624 */  addiu      $a2, $a0, 0x24
    /* 7238 80016A38 27E3010C */  jal        func_80078C9C
    /* 723C 80016A3C 08000724 */   addiu     $a3, $zero, 0x8
    /* 7240 80016A40 4EE3010C */  jal        func_80078D38
    /* 7244 80016A44 00000000 */   nop
    /* 7248 80016A48 96E2010C */  jal        func_80078A58
    /* 724C 80016A4C 21200000 */   addu      $a0, $zero, $zero
    /* 7250 80016A50 465A000C */  jal        disp_Init
    /* 7254 80016A54 00000000 */   nop
    /* 7258 80016A58 FF000224 */  addiu      $v0, $zero, 0xFF
    /* 725C 80016A5C 9C0682A3 */  sb         $v0, %gp_rel(D_800A3768)($gp)
    /* 7260 80016A60 DC0580A3 */  sb         $zero, %gp_rel(D_800A36A8)($gp)
    /* 7264 80016A64 F8D7000C */  jal        func_80035FE0
    /* 7268 80016A68 00000000 */   nop
    /* 726C 80016A6C 7BDD000C */  jal        func_800375EC
    /* 7270 80016A70 00000000 */   nop
    /* 7274 80016A74 3E5A000C */  jal        sys_InitSound
    /* 7278 80016A78 00000000 */   nop
    /* 727C 80016A7C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 7280 80016A80 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7284 80016A84 0800E003 */  jr         $ra
    /* 7288 80016A88 00000000 */   nop
endlabel func_80016A18
