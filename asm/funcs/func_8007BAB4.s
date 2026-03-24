glabel func_8007BAB4
    /* 6C2B4 8007BAB4 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 6C2B8 8007BAB8 1800B2AF */  sw         $s2, 0x18($sp)
    /* 6C2BC 8007BABC 21908000 */  addu       $s2, $a0, $zero
    /* 6C2C0 8007BAC0 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 6C2C4 8007BAC4 0A80133C */  lui        $s3, %hi(D_8009BE76)
    /* 6C2C8 8007BAC8 76BE7326 */  addiu      $s3, $s3, %lo(D_8009BE76)
    /* 6C2CC 8007BACC 2000BFAF */  sw         $ra, 0x20($sp)
    /* 6C2D0 8007BAD0 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6C2D4 8007BAD4 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C2D8 8007BAD8 00006292 */  lbu        $v0, 0x0($s3)
    /* 6C2DC 8007BADC 00000000 */  nop
    /* 6C2E0 8007BAE0 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6C2E4 8007BAE4 09004014 */  bnez       $v0, .L8007BB0C
    /* 6C2E8 8007BAE8 2188A000 */   addu      $s1, $a1, $zero
    /* 6C2EC 8007BAEC 0180043C */  lui        $a0, %hi(D_80015FDC)
    /* 6C2F0 8007BAF0 DC5F8424 */  addiu      $a0, $a0, %lo(D_80015FDC)
    /* 6C2F4 8007BAF4 21284002 */  addu       $a1, $s2, $zero
    /* 6C2F8 8007BAF8 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6C2FC 8007BAFC 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6C300 8007BB00 00000000 */  nop
    /* 6C304 8007BB04 09F84000 */  jalr       $v0
    /* 6C308 8007BB08 21302002 */   addu      $a2, $s1, $zero
  .L8007BB0C:
    /* 6C30C 8007BB0C 1C003026 */  addiu      $s0, $s1, 0x1C
    /* 6C310 8007BB10 21200002 */  addu       $a0, $s0, $zero
    /* 6C314 8007BB14 2EF1010C */  jal        func_8007C4B8
    /* 6C318 8007BB18 21282002 */   addu      $a1, $s1, $zero
    /* 6C31C 8007BB1C FF00043C */  lui        $a0, (0xFFFFFF >> 16)
    /* 6C320 8007BB20 FFFF8434 */  ori        $a0, $a0, (0xFFFFFF & 0xFFFF)
    /* 6C324 8007BB24 21280002 */  addu       $a1, $s0, $zero
    /* 6C328 8007BB28 40000624 */  addiu      $a2, $zero, 0x40
    /* 6C32C 8007BB2C 00FF033C */  lui        $v1, (0xFF000000 >> 16)
    /* 6C330 8007BB30 1C00228E */  lw         $v0, 0x1C($s1)
    /* 6C334 8007BB34 24204402 */  and        $a0, $s2, $a0
    /* 6C338 8007BB38 24104300 */  and        $v0, $v0, $v1
    /* 6C33C 8007BB3C 0A80033C */  lui        $v1, %hi(D_8009BE6C)
    /* 6C340 8007BB40 6CBE638C */  lw         $v1, %lo(D_8009BE6C)($v1)
    /* 6C344 8007BB44 25104400 */  or         $v0, $v0, $a0
    /* 6C348 8007BB48 1C0022AE */  sw         $v0, 0x1C($s1)
    /* 6C34C 8007BB4C 1800648C */  lw         $a0, 0x18($v1)
    /* 6C350 8007BB50 0800628C */  lw         $v0, 0x8($v1)
    /* 6C354 8007BB54 00000000 */  nop
    /* 6C358 8007BB58 09F84000 */  jalr       $v0
    /* 6C35C 8007BB5C 21380000 */   addu      $a3, $zero, $zero
    /* 6C360 8007BB60 0E006726 */  addiu      $a3, $s3, 0xE
    /* 6C364 8007BB64 21302002 */  addu       $a2, $s1, $zero
    /* 6C368 8007BB68 5000C824 */  addiu      $t0, $a2, 0x50
  .L8007BB6C:
    /* 6C36C 8007BB6C 0000C28C */  lw         $v0, 0x0($a2)
    /* 6C370 8007BB70 0400C38C */  lw         $v1, 0x4($a2)
    /* 6C374 8007BB74 0800C48C */  lw         $a0, 0x8($a2)
    /* 6C378 8007BB78 0C00C58C */  lw         $a1, 0xC($a2)
    /* 6C37C 8007BB7C 0000E2AC */  sw         $v0, 0x0($a3)
    /* 6C380 8007BB80 0400E3AC */  sw         $v1, 0x4($a3)
    /* 6C384 8007BB84 0800E4AC */  sw         $a0, 0x8($a3)
    /* 6C388 8007BB88 0C00E5AC */  sw         $a1, 0xC($a3)
    /* 6C38C 8007BB8C 1000C624 */  addiu      $a2, $a2, 0x10
    /* 6C390 8007BB90 F6FFC814 */  bne        $a2, $t0, .L8007BB6C
    /* 6C394 8007BB94 1000E724 */   addiu     $a3, $a3, 0x10
    /* 6C398 8007BB98 0000C28C */  lw         $v0, 0x0($a2)
    /* 6C39C 8007BB9C 0400C38C */  lw         $v1, 0x4($a2)
    /* 6C3A0 8007BBA0 0800C48C */  lw         $a0, 0x8($a2)
    /* 6C3A4 8007BBA4 0000E2AC */  sw         $v0, 0x0($a3)
    /* 6C3A8 8007BBA8 0400E3AC */  sw         $v1, 0x4($a3)
    /* 6C3AC 8007BBAC 0800E4AC */  sw         $a0, 0x8($a3)
    /* 6C3B0 8007BBB0 2000BF8F */  lw         $ra, 0x20($sp)
    /* 6C3B4 8007BBB4 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 6C3B8 8007BBB8 1800B28F */  lw         $s2, 0x18($sp)
    /* 6C3BC 8007BBBC 1400B18F */  lw         $s1, 0x14($sp)
    /* 6C3C0 8007BBC0 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C3C4 8007BBC4 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 6C3C8 8007BBC8 0800E003 */  jr         $ra
    /* 6C3CC 8007BBCC 00000000 */   nop
endlabel func_8007BAB4
