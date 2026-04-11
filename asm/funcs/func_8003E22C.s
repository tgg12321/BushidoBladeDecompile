glabel func_8003E22C
    /* 2EA2C 8003E22C E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 2EA30 8003E230 5C01848F */  lw         $a0, %gp_rel(D_800A3228)($gp)
    /* 2EA34 8003E234 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 2EA38 8003E238 15008210 */  beq        $a0, $v0, .L8003E290
    /* 2EA3C 8003E23C 1800BFAF */   sw        $ra, 0x18($sp)
    /* 2EA40 8003E240 4F05010C */  jal        func_8004153C
    /* 2EA44 8003E244 00000000 */   nop
    /* 2EA48 8003E248 21184000 */  addu       $v1, $v0, $zero
    /* 2EA4C 8003E24C 0F006010 */  beqz       $v1, .L8003E28C
    /* 2EA50 8003E250 FFFF0224 */   addiu     $v0, $zero, -0x1
    /* 2EA54 8003E254 5C01828F */  lw         $v0, %gp_rel(D_800A3228)($gp)
    /* 2EA58 8003E258 00000000 */  nop
    /* 2EA5C 8003E25C 04004014 */  bnez       $v0, .L8003E270
    /* 2EA60 8003E260 21280000 */   addu      $a1, $zero, $zero
    /* 2EA64 8003E264 21300000 */  addu       $a2, $zero, $zero
    /* 2EA68 8003E268 9EF80008 */  j          .L8003E278
    /* 2EA6C 8003E26C 40010724 */   addiu     $a3, $zero, 0x140
  .L8003E270:
    /* 2EA70 8003E270 21300000 */  addu       $a2, $zero, $zero
    /* 2EA74 8003E274 C0010724 */  addiu      $a3, $zero, 0x1C0
  .L8003E278:
    /* 2EA78 8003E278 14006484 */  lh         $a0, 0x14($v1)
    /* 2EA7C 8003E27C 18FF0224 */  addiu      $v0, $zero, -0xE8
    /* 2EA80 8003E280 A80C010C */  jal        videoDecCreate
    /* 2EA84 8003E284 1000A2AF */   sw        $v0, 0x10($sp)
    /* 2EA88 8003E288 FFFF0224 */  addiu      $v0, $zero, -0x1
  .L8003E28C:
    /* 2EA8C 8003E28C 5C0182AF */  sw         $v0, %gp_rel(D_800A3228)($gp)
  .L8003E290:
    /* 2EA90 8003E290 1800BF8F */  lw         $ra, 0x18($sp)
    /* 2EA94 8003E294 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 2EA98 8003E298 0800E003 */  jr         $ra
    /* 2EA9C 8003E29C 00000000 */   nop
endlabel func_8003E22C
