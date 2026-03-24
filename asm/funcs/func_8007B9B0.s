glabel func_8007B9B0
    /* 6C1B0 8007B9B0 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6C1B4 8007B9B4 1800B2AF */  sw         $s2, 0x18($sp)
    /* 6C1B8 8007B9B8 0A80123C */  lui        $s2, %hi(D_8009BE76)
    /* 6C1BC 8007B9BC 76BE5226 */  addiu      $s2, $s2, %lo(D_8009BE76)
    /* 6C1C0 8007B9C0 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* 6C1C4 8007B9C4 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6C1C8 8007B9C8 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C1CC 8007B9CC 00004292 */  lbu        $v0, 0x0($s2)
    /* 6C1D0 8007B9D0 00000000 */  nop
    /* 6C1D4 8007B9D4 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6C1D8 8007B9D8 08004014 */  bnez       $v0, .L8007B9FC
    /* 6C1DC 8007B9DC 21888000 */   addu      $s1, $a0, $zero
    /* 6C1E0 8007B9E0 0180043C */  lui        $a0, %hi(D_80015FC4)
    /* 6C1E4 8007B9E4 C45F8424 */  addiu      $a0, $a0, %lo(D_80015FC4)
    /* 6C1E8 8007B9E8 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6C1EC 8007B9EC 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6C1F0 8007B9F0 00000000 */  nop
    /* 6C1F4 8007B9F4 09F84000 */  jalr       $v0
    /* 6C1F8 8007B9F8 21282002 */   addu      $a1, $s1, $zero
  .L8007B9FC:
    /* 6C1FC 8007B9FC 1C003026 */  addiu      $s0, $s1, 0x1C
    /* 6C200 8007BA00 21200002 */  addu       $a0, $s0, $zero
    /* 6C204 8007BA04 2EF1010C */  jal        func_8007C4B8
    /* 6C208 8007BA08 21282002 */   addu      $a1, $s1, $zero
    /* 6C20C 8007BA0C FF00043C */  lui        $a0, (0xFFFFFF >> 16)
    /* 6C210 8007BA10 FFFF8434 */  ori        $a0, $a0, (0xFFFFFF & 0xFFFF)
    /* 6C214 8007BA14 21280002 */  addu       $a1, $s0, $zero
    /* 6C218 8007BA18 40000624 */  addiu      $a2, $zero, 0x40
    /* 6C21C 8007BA1C 1C00228E */  lw         $v0, 0x1C($s1)
    /* 6C220 8007BA20 0A80033C */  lui        $v1, %hi(D_8009BE6C)
    /* 6C224 8007BA24 6CBE638C */  lw         $v1, %lo(D_8009BE6C)($v1)
    /* 6C228 8007BA28 25104400 */  or         $v0, $v0, $a0
    /* 6C22C 8007BA2C 1C0022AE */  sw         $v0, 0x1C($s1)
    /* 6C230 8007BA30 1800648C */  lw         $a0, 0x18($v1)
    /* 6C234 8007BA34 0800628C */  lw         $v0, 0x8($v1)
    /* 6C238 8007BA38 00000000 */  nop
    /* 6C23C 8007BA3C 09F84000 */  jalr       $v0
    /* 6C240 8007BA40 21380000 */   addu      $a3, $zero, $zero
    /* 6C244 8007BA44 0E004726 */  addiu      $a3, $s2, 0xE
    /* 6C248 8007BA48 21302002 */  addu       $a2, $s1, $zero
    /* 6C24C 8007BA4C 50002826 */  addiu      $t0, $s1, 0x50
  .L8007BA50:
    /* 6C250 8007BA50 0000C28C */  lw         $v0, 0x0($a2)
    /* 6C254 8007BA54 0400C38C */  lw         $v1, 0x4($a2)
    /* 6C258 8007BA58 0800C48C */  lw         $a0, 0x8($a2)
    /* 6C25C 8007BA5C 0C00C58C */  lw         $a1, 0xC($a2)
    /* 6C260 8007BA60 0000E2AC */  sw         $v0, 0x0($a3)
    /* 6C264 8007BA64 0400E3AC */  sw         $v1, 0x4($a3)
    /* 6C268 8007BA68 0800E4AC */  sw         $a0, 0x8($a3)
    /* 6C26C 8007BA6C 0C00E5AC */  sw         $a1, 0xC($a3)
    /* 6C270 8007BA70 1000C624 */  addiu      $a2, $a2, 0x10
    /* 6C274 8007BA74 F6FFC814 */  bne        $a2, $t0, .L8007BA50
    /* 6C278 8007BA78 1000E724 */   addiu     $a3, $a3, 0x10
    /* 6C27C 8007BA7C 0000C28C */  lw         $v0, 0x0($a2)
    /* 6C280 8007BA80 0400C38C */  lw         $v1, 0x4($a2)
    /* 6C284 8007BA84 0800C48C */  lw         $a0, 0x8($a2)
    /* 6C288 8007BA88 0000E2AC */  sw         $v0, 0x0($a3)
    /* 6C28C 8007BA8C 0400E3AC */  sw         $v1, 0x4($a3)
    /* 6C290 8007BA90 0800E4AC */  sw         $a0, 0x8($a3)
    /* 6C294 8007BA94 21102002 */  addu       $v0, $s1, $zero
    /* 6C298 8007BA98 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* 6C29C 8007BA9C 1800B28F */  lw         $s2, 0x18($sp)
    /* 6C2A0 8007BAA0 1400B18F */  lw         $s1, 0x14($sp)
    /* 6C2A4 8007BAA4 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C2A8 8007BAA8 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6C2AC 8007BAAC 0800E003 */  jr         $ra
    /* 6C2B0 8007BAB0 00000000 */   nop
endlabel func_8007B9B0
