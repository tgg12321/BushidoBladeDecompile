glabel func_80046EA0
    /* 376A0 80046EA0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 376A4 80046EA4 1000B0AF */  sw         $s0, 0x10($sp)
    /* 376A8 80046EA8 1400BFAF */  sw         $ra, 0x14($sp)
    /* 376AC 80046EAC B6F9000C */  jal        DispHira
    /* 376B0 80046EB0 21808000 */   addu      $s0, $a0, $zero
    /* 376B4 80046EB4 E619010C */  jal        stage_GetId
    /* 376B8 80046EB8 00000000 */   nop
    /* 376BC 80046EBC 21204000 */  addu       $a0, $v0, $zero
    /* 376C0 80046EC0 A3F6000C */  jal        tslLineG5Init
    /* 376C4 80046EC4 21280002 */   addu      $a1, $s0, $zero
    /* 376C8 80046EC8 1400BF8F */  lw         $ra, 0x14($sp)
    /* 376CC 80046ECC 1000B08F */  lw         $s0, 0x10($sp)
    /* 376D0 80046ED0 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 376D4 80046ED4 0800E003 */  jr         $ra
    /* 376D8 80046ED8 00000000 */   nop
endlabel func_80046EA0
