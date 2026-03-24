glabel func_8005FA98
    /* 50298 8005FA98 B0FFBD27 */  addiu      $sp, $sp, -0x50
    /* 5029C 8005FA9C 4000B0AF */  sw         $s0, 0x40($sp)
    /* 502A0 8005FAA0 21808000 */  addu       $s0, $a0, $zero
    /* 502A4 8005FAA4 4400B1AF */  sw         $s1, 0x44($sp)
    /* 502A8 8005FAA8 2188A000 */  addu       $s1, $a1, $zero
    /* 502AC 8005FAAC 00020224 */  addiu      $v0, $zero, 0x200
    /* 502B0 8005FAB0 3000A2AF */  sw         $v0, 0x30($sp)
    /* 502B4 8005FAB4 00010224 */  addiu      $v0, $zero, 0x100
    /* 502B8 8005FAB8 3400A2AF */  sw         $v0, 0x34($sp)
    /* 502BC 8005FABC 40101000 */  sll        $v0, $s0, 1
    /* 502C0 8005FAC0 21105000 */  addu       $v0, $v0, $s0
    /* 502C4 8005FAC4 80100200 */  sll        $v0, $v0, 2
    /* 502C8 8005FAC8 0A80033C */  lui        $v1, %hi(D_8009B63C)
    /* 502CC 8005FACC 3CB66324 */  addiu      $v1, $v1, %lo(D_8009B63C)
    /* 502D0 8005FAD0 21104300 */  addu       $v0, $v0, $v1
    /* 502D4 8005FAD4 21182002 */  addu       $v1, $s1, $zero
    /* 502D8 8005FAD8 4800B2AF */  sw         $s2, 0x48($sp)
    /* 502DC 8005FADC 90013226 */  addiu      $s2, $s1, 0x190
    /* 502E0 8005FAE0 1000A2AF */  sw         $v0, 0x10($sp)
    /* 502E4 8005FAE4 01000224 */  addiu      $v0, $zero, 0x1
    /* 502E8 8005FAE8 4C00BFAF */  sw         $ra, 0x4C($sp)
    /* 502EC 8005FAEC 3800A0A3 */  sb         $zero, 0x38($sp)
    /* 502F0 8005FAF0 2C00A0AF */  sw         $zero, 0x2C($sp)
    /* 502F4 8005FAF4 2800A0AF */  sw         $zero, 0x28($sp)
    /* 502F8 8005FAF8 2000A0AF */  sw         $zero, 0x20($sp)
    /* 502FC 8005FAFC 11000212 */  beq        $s0, $v0, .L8005FB44
    /* 50300 8005FB00 2400A6AF */   sw        $a2, 0x24($sp)
    /* 50304 8005FB04 0200022A */  slti       $v0, $s0, 0x2
    /* 50308 8005FB08 05004010 */  beqz       $v0, .L8005FB20
    /* 5030C 8005FB0C 00000000 */   nop
    /* 50310 8005FB10 08000012 */  beqz       $s0, .L8005FB34
    /* 50314 8005FB14 1000A427 */   addiu     $a0, $sp, 0x10
    /* 50318 8005FB18 D97E0108 */  j          .L8005FB64
    /* 5031C 8005FB1C 00000000 */   nop
  .L8005FB20:
    /* 50320 8005FB20 02000224 */  addiu      $v0, $zero, 0x2
    /* 50324 8005FB24 0B000212 */  beq        $s0, $v0, .L8005FB54
    /* 50328 8005FB28 1000A427 */   addiu     $a0, $sp, 0x10
    /* 5032C 8005FB2C D97E0108 */  j          .L8005FB64
    /* 50330 8005FB30 00000000 */   nop
  .L8005FB34:
    /* 50334 8005FB34 0A80023C */  lui        $v0, %hi(D_8009B660)
    /* 50338 8005FB38 60B64224 */  addiu      $v0, $v0, %lo(D_8009B660)
    /* 5033C 8005FB3C D87E0108 */  j          .L8005FB60
    /* 50340 8005FB40 1400A2AF */   sw        $v0, 0x14($sp)
  .L8005FB44:
    /* 50344 8005FB44 0A80023C */  lui        $v0, %hi(D_8009B670)
    /* 50348 8005FB48 70B64224 */  addiu      $v0, $v0, %lo(D_8009B670)
    /* 5034C 8005FB4C D87E0108 */  j          .L8005FB60
    /* 50350 8005FB50 1400A2AF */   sw        $v0, 0x14($sp)
  .L8005FB54:
    /* 50354 8005FB54 0A80023C */  lui        $v0, %hi(D_8009B678)
    /* 50358 8005FB58 78B64224 */  addiu      $v0, $v0, %lo(D_8009B678)
    /* 5035C 8005FB5C 1400A2AF */  sw         $v0, 0x14($sp)
  .L8005FB60:
    /* 50360 8005FB60 1000A427 */  addiu      $a0, $sp, 0x10
  .L8005FB64:
    /* 50364 8005FB64 21280000 */  addu       $a1, $zero, $zero
    /* 50368 8005FB68 CACD010C */  jal        func_80073728
    /* 5036C 8005FB6C 1C00A3AF */   sw        $v1, 0x1C($sp)
    /* 50370 8005FB70 1000A427 */  addiu      $a0, $sp, 0x10
    /* 50374 8005FB74 21280000 */  addu       $a1, $zero, $zero
    /* 50378 8005FB78 40181000 */  sll        $v1, $s0, 1
    /* 5037C 8005FB7C 21187000 */  addu       $v1, $v1, $s0
    /* 50380 8005FB80 80180300 */  sll        $v1, $v1, 2
    /* 50384 8005FB84 0A80063C */  lui        $a2, %hi(D_8009B610)
    /* 50388 8005FB88 10B6C624 */  addiu      $a2, $a2, %lo(D_8009B610)
    /* 5038C 8005FB8C 21186600 */  addu       $v1, $v1, $a2
    /* 50390 8005FB90 1000A3AF */  sw         $v1, 0x10($sp)
    /* 50394 8005FB94 0A80033C */  lui        $v1, %hi(D_8009B634)
    /* 50398 8005FB98 34B66324 */  addiu      $v1, $v1, %lo(D_8009B634)
    /* 5039C 8005FB9C 1400A3AF */  sw         $v1, 0x14($sp)
    /* 503A0 8005FBA0 CACD010C */  jal        func_80073728
    /* 503A4 8005FBA4 1C00A2AF */   sw        $v0, 0x1C($sp)
    /* 503A8 8005FBA8 23105102 */  subu       $v0, $s2, $s1
    /* 503AC 8005FBAC 4C00BF8F */  lw         $ra, 0x4C($sp)
    /* 503B0 8005FBB0 4800B28F */  lw         $s2, 0x48($sp)
    /* 503B4 8005FBB4 4400B18F */  lw         $s1, 0x44($sp)
    /* 503B8 8005FBB8 4000B08F */  lw         $s0, 0x40($sp)
    /* 503BC 8005FBBC 5000BD27 */  addiu      $sp, $sp, 0x50
    /* 503C0 8005FBC0 0800E003 */  jr         $ra
    /* 503C4 8005FBC4 00000000 */   nop
endlabel func_8005FA98
