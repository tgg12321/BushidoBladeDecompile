glabel func_8005BDF0
    /* 4C5F0 8005BDF0 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 4C5F4 8005BDF4 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 4C5F8 8005BDF8 0F80133C */  lui        $s3, %hi(D_800EFC38)
    /* 4C5FC 8005BDFC 38FC7326 */  addiu      $s3, $s3, %lo(D_800EFC38)
    /* 4C600 8005BE00 1800B2AF */  sw         $s2, 0x18($sp)
    /* 4C604 8005BE04 0F80123C */  lui        $s2, %hi(D_800EFB38)
    /* 4C608 8005BE08 38FB5226 */  addiu      $s2, $s2, %lo(D_800EFB38)
    /* 4C60C 8005BE0C 1000B0AF */  sw         $s0, 0x10($sp)
    /* 4C610 8005BE10 0A80103C */  lui        $s0, %hi(D_8009AD18)
    /* 4C614 8005BE14 18AD1026 */  addiu      $s0, $s0, %lo(D_8009AD18)
    /* 4C618 8005BE18 1400B1AF */  sw         $s1, 0x14($sp)
    /* 4C61C 8005BE1C 03001126 */  addiu      $s1, $s0, 0x3
    /* 4C620 8005BE20 2000BFAF */  sw         $ra, 0x20($sp)
  .L8005BE24:
    /* 4C624 8005BE24 00000492 */  lbu        $a0, 0x0($s0)
    /* 4C628 8005BE28 D91F020C */  jal        func_80087F64
    /* 4C62C 8005BE2C 00000000 */   nop
    /* 4C630 8005BE30 00000292 */  lbu        $v0, 0x0($s0)
    /* 4C634 8005BE34 00000000 */  nop
    /* 4C638 8005BE38 80100200 */  sll        $v0, $v0, 2
    /* 4C63C 8005BE3C 21105300 */  addu       $v0, $v0, $s3
    /* 4C640 8005BE40 000040AC */  sw         $zero, 0x0($v0)
    /* 4C644 8005BE44 00000292 */  lbu        $v0, 0x0($s0)
    /* 4C648 8005BE48 01001026 */  addiu      $s0, $s0, 0x1
    /* 4C64C 8005BE4C 80100200 */  sll        $v0, $v0, 2
    /* 4C650 8005BE50 21105200 */  addu       $v0, $v0, $s2
    /* 4C654 8005BE54 000040AC */  sw         $zero, 0x0($v0)
    /* 4C658 8005BE58 2A101102 */  slt        $v0, $s0, $s1
    /* 4C65C 8005BE5C F1FF4014 */  bnez       $v0, .L8005BE24
    /* 4C660 8005BE60 00000000 */   nop
    /* 4C664 8005BE64 2000BF8F */  lw         $ra, 0x20($sp)
    /* 4C668 8005BE68 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 4C66C 8005BE6C 1800B28F */  lw         $s2, 0x18($sp)
    /* 4C670 8005BE70 1400B18F */  lw         $s1, 0x14($sp)
    /* 4C674 8005BE74 1000B08F */  lw         $s0, 0x10($sp)
    /* 4C678 8005BE78 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 4C67C 8005BE7C 0800E003 */  jr         $ra
    /* 4C680 8005BE80 00000000 */   nop
endlabel func_8005BDF0
