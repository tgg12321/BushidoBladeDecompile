glabel obj_InitChars
    /* 4BDAC 8005B5AC D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 4BDB0 8005B5B0 21200000 */  addu       $a0, $zero, $zero
    /* 4BDB4 8005B5B4 2000BFAF */  sw         $ra, 0x20($sp)
    /* 4BDB8 8005B5B8 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 4BDBC 8005B5BC 1800B2AF */  sw         $s2, 0x18($sp)
    /* 4BDC0 8005B5C0 1400B1AF */  sw         $s1, 0x14($sp)
    /* 4BDC4 8005B5C4 3416020C */  jal        func_800858D0
    /* 4BDC8 8005B5C8 1000B0AF */   sw        $s0, 0x10($sp)
    /* 4BDCC 8005B5CC 21880000 */  addu       $s1, $zero, $zero
    /* 4BDD0 8005B5D0 7F001324 */  addiu      $s3, $zero, 0x7F
    /* 4BDD4 8005B5D4 0F80123C */  lui        $s2, %hi(D_800EFB78)
    /* 4BDD8 8005B5D8 78FB5226 */  addiu      $s2, $s2, %lo(D_800EFB78)
    /* 4BDDC 8005B5DC 21800000 */  addu       $s0, $zero, $zero
  .L8005B5E0:
    /* 4BDE0 8005B5E0 00241100 */  sll        $a0, $s1, 16
    /* 4BDE4 8005B5E4 03240400 */  sra        $a0, $a0, 16
    /* 4BDE8 8005B5E8 21280000 */  addu       $a1, $zero, $zero
    /* 4BDEC 8005B5EC 0F80013C */  lui        $at, %hi(D_800EFB78)
    /* 4BDF0 8005B5F0 21083000 */  addu       $at, $at, $s0
    /* 4BDF4 8005B5F4 78FB20AC */  sw         $zero, %lo(D_800EFB78)($at)
    /* 4BDF8 8005B5F8 050053A2 */  sb         $s3, 0x5($s2)
    /* 4BDFC 8005B5FC 0F80013C */  lui        $at, %hi(D_800EFB7C)
    /* 4BE00 8005B600 21083000 */  addu       $at, $at, $s0
    /* 4BE04 8005B604 7CFB33A0 */  sb         $s3, %lo(D_800EFB7C)($at)
    /* 4BE08 8005B608 4C18020C */  jal        func_80086130
    /* 4BE0C 8005B60C 21300000 */   addu      $a2, $zero, $zero
    /* 4BE10 8005B610 08005226 */  addiu      $s2, $s2, 0x8
    /* 4BE14 8005B614 01003126 */  addiu      $s1, $s1, 0x1
    /* 4BE18 8005B618 1800222A */  slti       $v0, $s1, 0x18
    /* 4BE1C 8005B61C F0FF4014 */  bnez       $v0, .L8005B5E0
    /* 4BE20 8005B620 08001026 */   addiu     $s0, $s0, 0x8
    /* 4BE24 8005B624 2000BF8F */  lw         $ra, 0x20($sp)
    /* 4BE28 8005B628 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 4BE2C 8005B62C 1800B28F */  lw         $s2, 0x18($sp)
    /* 4BE30 8005B630 1400B18F */  lw         $s1, 0x14($sp)
    /* 4BE34 8005B634 1000B08F */  lw         $s0, 0x10($sp)
    /* 4BE38 8005B638 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 4BE3C 8005B63C 0800E003 */  jr         $ra
    /* 4BE40 8005B640 00000000 */   nop
endlabel obj_InitChars
