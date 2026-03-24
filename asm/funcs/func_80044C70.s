glabel func_80044C70
    /* 35470 80044C70 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 35474 80044C74 1000B0AF */  sw         $s0, 0x10($sp)
    /* 35478 80044C78 21808000 */  addu       $s0, $a0, $zero
    /* 3547C 80044C7C 0B80043C */  lui        $a0, %hi(D_800A9CF8)
    /* 35480 80044C80 F89C8484 */  lh         $a0, %lo(D_800A9CF8)($a0)
    /* 35484 80044C84 1400BFAF */  sw         $ra, 0x14($sp)
    /* 35488 80044C88 4010010C */  jal        func_80044100
    /* 3548C 80044C8C 21280002 */   addu      $a1, $s0, $zero
    /* 35490 80044C90 0B80023C */  lui        $v0, %hi(D_800A9D04)
    /* 35494 80044C94 049D428C */  lw         $v0, %lo(D_800A9D04)($v0)
    /* 35498 80044C98 0B80033C */  lui        $v1, %hi(D_800A9D00)
    /* 3549C 80044C9C 009D638C */  lw         $v1, %lo(D_800A9D00)($v1)
    /* 354A0 80044CA0 21105000 */  addu       $v0, $v0, $s0
    /* 354A4 80044CA4 21187000 */  addu       $v1, $v1, $s0
    /* 354A8 80044CA8 0B80013C */  lui        $at, %hi(D_800A9D04)
    /* 354AC 80044CAC 049D22AC */  sw         $v0, %lo(D_800A9D04)($at)
    /* 354B0 80044CB0 0B80013C */  lui        $at, %hi(D_800A9D00)
    /* 354B4 80044CB4 009D23AC */  sw         $v1, %lo(D_800A9D00)($at)
    /* 354B8 80044CB8 1400BF8F */  lw         $ra, 0x14($sp)
    /* 354BC 80044CBC 1000B08F */  lw         $s0, 0x10($sp)
    /* 354C0 80044CC0 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 354C4 80044CC4 0800E003 */  jr         $ra
    /* 354C8 80044CC8 00000000 */   nop
endlabel func_80044C70
