glabel func_8006E390
    /* 5EB90 8006E390 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 5EB94 8006E394 1000B0AF */  sw         $s0, 0x10($sp)
    /* 5EB98 8006E398 21808000 */  addu       $s0, $a0, $zero
    /* 5EB9C 8006E39C 1400BFAF */  sw         $ra, 0x14($sp)
    /* 5EBA0 8006E3A0 0000A48C */  lw         $a0, 0x0($a1)
    /* 5EBA4 8006E3A4 00000000 */  nop
    /* 5EBA8 8006E3A8 01008424 */  addiu      $a0, $a0, 0x1
    /* 5EBAC 8006E3AC 0000A4AC */  sw         $a0, 0x0($a1)
    /* 5EBB0 8006E3B0 48A4010C */  jal        func_80069120
    /* 5EBB4 8006E3B4 01008430 */   andi      $a0, $a0, 0x1
    /* 5EBB8 8006E3B8 3004838F */  lw         $v1, %gp_rel(D_800A34FC)($gp)
    /* 5EBBC 8006E3BC 00000000 */  nop
    /* 5EBC0 8006E3C0 2400638C */  lw         $v1, 0x24($v1)
    /* 5EBC4 8006E3C4 00000000 */  nop
    /* 5EBC8 8006E3C8 040003AE */  sw         $v1, 0x4($s0)
    /* 5EBCC 8006E3CC 0400438C */  lw         $v1, 0x4($v0)
    /* 5EBD0 8006E3D0 00000000 */  nop
    /* 5EBD4 8006E3D4 0C0003AE */  sw         $v1, 0xC($s0)
    /* 5EBD8 8006E3D8 0000438C */  lw         $v1, 0x0($v0)
    /* 5EBDC 8006E3DC 00000000 */  nop
    /* 5EBE0 8006E3E0 080003AE */  sw         $v1, 0x8($s0)
    /* 5EBE4 8006E3E4 0800438C */  lw         $v1, 0x8($v0)
    /* 5EBE8 8006E3E8 00000000 */  nop
    /* 5EBEC 8006E3EC 100003AE */  sw         $v1, 0x10($s0)
    /* 5EBF0 8006E3F0 1000438C */  lw         $v1, 0x10($v0)
    /* 5EBF4 8006E3F4 00000000 */  nop
    /* 5EBF8 8006E3F8 140003AE */  sw         $v1, 0x14($s0)
    /* 5EBFC 8006E3FC 0C00438C */  lw         $v1, 0xC($v0)
    /* 5EC00 8006E400 00000000 */  nop
    /* 5EC04 8006E404 180003AE */  sw         $v1, 0x18($s0)
    /* 5EC08 8006E408 1400438C */  lw         $v1, 0x14($v0)
    /* 5EC0C 8006E40C 00000000 */  nop
    /* 5EC10 8006E410 1C0003AE */  sw         $v1, 0x1C($s0)
    /* 5EC14 8006E414 1800438C */  lw         $v1, 0x18($v0)
    /* 5EC18 8006E418 00000000 */  nop
    /* 5EC1C 8006E41C 200003AE */  sw         $v1, 0x20($s0)
    /* 5EC20 8006E420 1C00438C */  lw         $v1, 0x1C($v0)
    /* 5EC24 8006E424 540482AF */  sw         $v0, %gp_rel(D_800A3520)($gp)
    /* 5EC28 8006E428 240003AE */  sw         $v1, 0x24($s0)
    /* 5EC2C 8006E42C 1400BF8F */  lw         $ra, 0x14($sp)
    /* 5EC30 8006E430 1000B08F */  lw         $s0, 0x10($sp)
    /* 5EC34 8006E434 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 5EC38 8006E438 0800E003 */  jr         $ra
    /* 5EC3C 8006E43C 00000000 */   nop
endlabel func_8006E390
