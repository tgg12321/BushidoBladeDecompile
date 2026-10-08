glabel func_8003F274
    /* 2FA74 8003F274 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2FA78 8003F278 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2FA7C 8003F27C 0B80023C */  lui        $v0, %hi(D_800A8FB0)
    /* 2FA80 8003F280 B08F4224 */  addiu      $v0, $v0, %lo(D_800A8FB0)
    /* 2FA84 8003F284 FF000624 */  addiu      $a2, $zero, 0xFF
  .L8003F288:
    /* 2FA88 8003F288 000040AC */  sw         $zero, 0x0($v0)
    /* 2FA8C 8003F28C FFFFC624 */  addiu      $a2, $a2, -0x1
    /* 2FA90 8003F290 FDFFC104 */  bgez       $a2, .L8003F288
    /* 2FA94 8003F294 04004224 */   addiu     $v0, $v0, 0x4
    /* 2FA98 8003F298 9AFC000C */  jal        func_8003F268
    /* 2FA9C 8003F29C 00000000 */   nop
    /* 2FAA0 8003F2A0 0A80023C */  lui        $v0, %hi(D_800A3708)
    /* 2FAA4 8003F2A4 0837428C */  lw         $v0, %lo(D_800A3708)($v0)
    /* 2FAA8 8003F2A8 6210043C */  lui        $a0, (0x10624DD3 >> 16)
    /* 2FAAC 8003F2AC 4C00438C */  lw         $v1, 0x4C($v0)
    /* 2FAB0 8003F2B0 D34D8434 */  ori        $a0, $a0, (0x10624DD3 & 0xFFFF)
    /* 2FAB4 8003F2B4 007D6324 */  addiu      $v1, $v1, 0x7D00
    /* 2FAB8 8003F2B8 18006400 */  mult       $v1, $a0
    /* 2FABC 8003F2BC 5400428C */  lw         $v0, 0x54($v0)
    /* 2FAC0 8003F2C0 10380000 */  mfhi       $a3
    /* 2FAC4 8003F2C4 007D4224 */  addiu      $v0, $v0, 0x7D00
    /* 2FAC8 8003F2C8 00000000 */  nop
    /* 2FACC 8003F2CC 18004400 */  mult       $v0, $a0
    /* 2FAD0 8003F2D0 21300000 */  addu       $a2, $zero, $zero
    /* 2FAD4 8003F2D4 0F000C24 */  addiu      $t4, $zero, 0xF
    /* 2FAD8 8003F2D8 0B800B3C */  lui        $t3, %hi(D_800A8FB0)
    /* 2FADC 8003F2DC B08F6B25 */  addiu      $t3, $t3, %lo(D_800A8FB0)
    /* 2FAE0 8003F2E0 0980093C */  lui        $t1, %hi(D_80094A6C)
    /* 2FAE4 8003F2E4 6C4A2925 */  addiu      $t1, $t1, %lo(D_80094A6C)
    /* 2FAE8 8003F2E8 C31F0300 */  sra        $v1, $v1, 31
    /* 2FAEC 8003F2EC C3210700 */  sra        $a0, $a3, 7
    /* 2FAF0 8003F2F0 23508300 */  subu       $t2, $a0, $v1
    /* 2FAF4 8003F2F4 C3170200 */  sra        $v0, $v0, 31
    /* 2FAF8 8003F2F8 10280000 */  mfhi       $a1
    /* 2FAFC 8003F2FC C3190500 */  sra        $v1, $a1, 7
    /* 2FB00 8003F300 23686200 */  subu       $t5, $v1, $v0
  .L8003F304:
    /* 2FB04 8003F304 F8FFC224 */  addiu      $v0, $a2, -0x8
    /* 2FB08 8003F308 2120A201 */  addu       $a0, $t5, $v0
    /* 2FB0C 8003F30C 2000822C */  sltiu      $v0, $a0, 0x20
    /* 2FB10 8003F310 15004010 */  beqz       $v0, .L8003F368
    /* 2FB14 8003F314 00000000 */   nop
    /* 2FB18 8003F318 0000288D */  lw         $t0, 0x0($t1)
    /* 2FB1C 8003F31C 21280000 */  addu       $a1, $zero, $zero
    /* 2FB20 8003F320 40390400 */  sll        $a3, $a0, 5
    /* 2FB24 8003F324 F8FFA224 */  addiu      $v0, $a1, -0x8
  .L8003F328:
    /* 2FB28 8003F328 21184201 */  addu       $v1, $t2, $v0
    /* 2FB2C 8003F32C 2000622C */  sltiu      $v0, $v1, 0x20
    /* 2FB30 8003F330 09004010 */  beqz       $v0, .L8003F358
    /* 2FB34 8003F334 23108501 */   subu      $v0, $t4, $a1
    /* 2FB38 8003F338 40100200 */  sll        $v0, $v0, 1
    /* 2FB3C 8003F33C 07104800 */  srav       $v0, $t0, $v0
    /* 2FB40 8003F340 2120E300 */  addu       $a0, $a3, $v1
    /* 2FB44 8003F344 21208B00 */  addu       $a0, $a0, $t3
    /* 2FB48 8003F348 00008390 */  lbu        $v1, 0x0($a0)
    /* 2FB4C 8003F34C 03004230 */  andi       $v0, $v0, 0x3
    /* 2FB50 8003F350 25186200 */  or         $v1, $v1, $v0
    /* 2FB54 8003F354 000083A0 */  sb         $v1, 0x0($a0)
  .L8003F358:
    /* 2FB58 8003F358 0100A524 */  addiu      $a1, $a1, 0x1
    /* 2FB5C 8003F35C 1000A228 */  slti       $v0, $a1, 0x10
    /* 2FB60 8003F360 F1FF4014 */  bnez       $v0, .L8003F328
    /* 2FB64 8003F364 F8FFA224 */   addiu     $v0, $a1, -0x8
  .L8003F368:
    /* 2FB68 8003F368 0100C624 */  addiu      $a2, $a2, 0x1
    /* 2FB6C 8003F36C 1000C228 */  slti       $v0, $a2, 0x10
    /* 2FB70 8003F370 E4FF4014 */  bnez       $v0, .L8003F304
    /* 2FB74 8003F374 04002925 */   addiu     $t1, $t1, 0x4
    /* 2FB78 8003F378 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2FB7C 8003F37C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2FB80 8003F380 0800E003 */  jr         $ra
    /* 2FB84 8003F384 00000000 */   nop
endlabel func_8003F274
