glabel func_8006D338
    /* 5DB38 8006D338 90FFBD27 */  addiu      $sp, $sp, -0x70
    /* 5DB3C 8006D33C 7000A4AF */  sw         $a0, 0x70($sp)
    /* 5DB40 8006D340 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5DB44 8006D344 7400A5AF */  sw         $a1, 0x74($sp)
    /* 5DB48 8006D348 4804828F */  lw         $v0, %gp_rel(D_800A3514)($gp)
    /* 5DB4C 8006D34C 0A80033C */  lui        $v1, %hi(D_800A36AC)
    /* 5DB50 8006D350 AC36638C */  lw         $v1, %lo(D_800A36AC)($v1)
    /* 5DB54 8006D354 0A80053C */  lui        $a1, %hi(D_800A3518)
    /* 5DB58 8006D358 1835A524 */  addiu      $a1, $a1, %lo(D_800A3518)
    /* 5DB5C 8006D35C 6C00BFAF */  sw         $ra, 0x6C($sp)
    /* 5DB60 8006D360 6800B0AF */  sw         $s0, 0x68($sp)
    /* 5DB64 8006D364 01004224 */  addiu      $v0, $v0, 0x1
    /* 5DB68 8006D368 01006330 */  andi       $v1, $v1, 0x1
    /* 5DB6C 8006D36C C0810300 */  sll        $s0, $v1, 7
    /* 5DB70 8006D370 21800302 */  addu       $s0, $s0, $v1
    /* 5DB74 8006D374 C0801000 */  sll        $s0, $s0, 3
    /* 5DB78 8006D378 21800302 */  addu       $s0, $s0, $v1
    /* 5DB7C 8006D37C 00811000 */  sll        $s0, $s0, 4
    /* 5DB80 8006D380 480482AF */  sw         $v0, %gp_rel(D_800A3514)($gp)
    /* 5DB84 8006D384 0F80023C */  lui        $v0, %hi(D_800F7438)
    /* 5DB88 8006D388 38744224 */  addiu      $v0, $v0, %lo(D_800F7438)
    /* 5DB8C 8006D38C E4B8010C */  jal        func_8006E390
    /* 5DB90 8006D390 21800202 */   addu      $s0, $s0, $v0
    /* 5DB94 8006D394 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5DB98 8006D398 02000524 */  addiu      $a1, $zero, 0x2
    /* 5DB9C 8006D39C B9A6010C */  jal        func_80069AE4
    /* 5DBA0 8006D3A0 21300002 */   addu      $a2, $s0, $zero
    /* 5DBA4 8006D3A4 87B0010C */  jal        func_8006C21C
    /* 5DBA8 8006D3A8 1000A427 */   addiu     $a0, $sp, 0x10
    /* 5DBAC 8006D3AC EFB3010C */  jal        func_8006CFBC
    /* 5DBB0 8006D3B0 1000A427 */   addiu     $a0, $sp, 0x10
    /* 5DBB4 8006D3B4 7000A427 */  addiu      $a0, $sp, 0x70
    /* 5DBB8 8006D3B8 7400A527 */  addiu      $a1, $sp, 0x74
    /* 5DBBC 8006D3BC 00140200 */  sll        $v0, $v0, 16
    /* 5DBC0 8006D3C0 32B3010C */  jal        func_8006CCC8
    /* 5DBC4 8006D3C4 03340200 */   sra       $a2, $v0, 16
    /* 5DBC8 8006D3C8 6C00BF8F */  lw         $ra, 0x6C($sp)
    /* 5DBCC 8006D3CC 6800B08F */  lw         $s0, 0x68($sp)
    /* 5DBD0 8006D3D0 7000BD27 */  addiu      $sp, $sp, 0x70
    /* 5DBD4 8006D3D4 0800E003 */  jr         $ra
    /* 5DBD8 8006D3D8 00000000 */   nop
endlabel func_8006D338
