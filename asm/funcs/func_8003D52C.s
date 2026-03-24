glabel func_8003D52C
    /* 2DD2C 8003D52C 0400A5AF */  sw         $a1, 0x4($sp)
    /* 2DD30 8003D530 0800A6AF */  sw         $a2, 0x8($sp)
    /* 2DD34 8003D534 0C00A7AF */  sw         $a3, 0xC($sp)
    /* 2DD38 8003D538 C8FABD27 */  addiu      $sp, $sp, -0x538
    /* 2DD3C 8003D53C 1805B2AF */  sw         $s2, 0x518($sp)
    /* 2DD40 8003D540 21908000 */  addu       $s2, $a0, $zero
    /* 2DD44 8003D544 2405B5AF */  sw         $s5, 0x524($sp)
    /* 2DD48 8003D548 21A8A000 */  addu       $s5, $a1, $zero
    /* 2DD4C 8003D54C 2805B6AF */  sw         $s6, 0x528($sp)
    /* 2DD50 8003D550 21B00000 */  addu       $s6, $zero, $zero
    /* 2DD54 8003D554 1405B1AF */  sw         $s1, 0x514($sp)
    /* 2DD58 8003D558 1004B127 */  addiu      $s1, $sp, 0x410
    /* 2DD5C 8003D55C 1C05B3AF */  sw         $s3, 0x51C($sp)
    /* 2DD60 8003D560 4005B327 */  addiu      $s3, $sp, 0x540
    /* 2DD64 8003D564 3405BFAF */  sw         $ra, 0x534($sp)
    /* 2DD68 8003D568 3005BEAF */  sw         $fp, 0x530($sp)
    /* 2DD6C 8003D56C 2C05B7AF */  sw         $s7, 0x52C($sp)
    /* 2DD70 8003D570 2005B4AF */  sw         $s4, 0x520($sp)
    /* 2DD74 8003D574 1005B0AF */  sw         $s0, 0x510($sp)
    /* 2DD78 8003D578 3C05B5AF */  sw         $s5, 0x53C($sp)
    /* 2DD7C 8003D57C 1000A0A3 */  sb         $zero, 0x10($sp)
    /* 2DD80 8003D580 00005092 */  lbu        $s0, 0x0($s2)
    /* 2DD84 8003D584 00000000 */  nop
    /* 2DD88 8003D588 19000012 */  beqz       $s0, .L8003D5F0
    /* 2DD8C 8003D58C 01005226 */   addiu     $s2, $s2, 0x1
    /* 2DD90 8003D590 25001E24 */  addiu      $fp, $zero, 0x25
    /* 2DD94 8003D594 1000B727 */  addiu      $s7, $sp, 0x10
    /* 2DD98 8003D598 21A02002 */  addu       $s4, $s1, $zero
  .L8003D59C:
    /* 2DD9C 8003D59C 0E001E16 */  bne        $s0, $fp, .L8003D5D8
    /* 2DDA0 8003D5A0 00000000 */   nop
    /* 2DDA4 8003D5A4 0300C016 */  bnez       $s6, .L8003D5B4
    /* 2DDA8 8003D5A8 1000A427 */   addiu     $a0, $sp, 0x10
    /* 2DDAC 8003D5AC 76F50008 */  j          .L8003D5D8
    /* 2DDB0 8003D5B0 01001624 */   addiu     $s6, $zero, 0x1
  .L8003D5B4:
    /* 2DDB4 8003D5B4 76E4010C */  jal        func_800791D8
    /* 2DDB8 8003D5B8 000020A2 */   sb        $zero, 0x0($s1)
    /* 2DDBC 8003D5BC 2120E202 */  addu       $a0, $s7, $v0
    /* 2DDC0 8003D5C0 21288002 */  addu       $a1, $s4, $zero
    /* 2DDC4 8003D5C4 8CE6010C */  jal        func_80079A30
    /* 2DDC8 8003D5C8 2130A002 */   addu      $a2, $s5, $zero
    /* 2DDCC 8003D5CC 21888002 */  addu       $s1, $s4, $zero
    /* 2DDD0 8003D5D0 04007326 */  addiu      $s3, $s3, 0x4
    /* 2DDD4 8003D5D4 FCFF758E */  lw         $s5, -0x4($s3)
  .L8003D5D8:
    /* 2DDD8 8003D5D8 000030A2 */  sb         $s0, 0x0($s1)
    /* 2DDDC 8003D5DC 01003126 */  addiu      $s1, $s1, 0x1
    /* 2DDE0 8003D5E0 00005092 */  lbu        $s0, 0x0($s2)
    /* 2DDE4 8003D5E4 00000000 */  nop
    /* 2DDE8 8003D5E8 ECFF0016 */  bnez       $s0, .L8003D59C
    /* 2DDEC 8003D5EC 01005226 */   addiu     $s2, $s2, 0x1
  .L8003D5F0:
    /* 2DDF0 8003D5F0 1000A427 */  addiu      $a0, $sp, 0x10
    /* 2DDF4 8003D5F4 76E4010C */  jal        func_800791D8
    /* 2DDF8 8003D5F8 000020A2 */   sb        $zero, 0x0($s1)
    /* 2DDFC 8003D5FC 1000A427 */  addiu      $a0, $sp, 0x10
    /* 2DE00 8003D600 21208200 */  addu       $a0, $a0, $v0
    /* 2DE04 8003D604 1004A527 */  addiu      $a1, $sp, 0x410
    /* 2DE08 8003D608 8CE6010C */  jal        func_80079A30
    /* 2DE0C 8003D60C 2130A002 */   addu      $a2, $s5, $zero
    /* 2DE10 8003D610 1000B093 */  lbu        $s0, 0x10($sp)
    /* 2DE14 8003D614 00000000 */  nop
    /* 2DE18 8003D618 49000012 */  beqz       $s0, .L8003D740
    /* 2DE1C 8003D61C 1100B127 */   addiu     $s1, $sp, 0x11
  .L8003D620:
    /* 2DE20 8003D620 9402858F */  lw         $a1, %gp_rel(D_800A3360)($gp)
    /* 2DE24 8003D624 00000000 */  nop
    /* 2DE28 8003D628 1A00A228 */  slti       $v0, $a1, 0x1A
    /* 2DE2C 8003D62C 44004010 */  beqz       $v0, .L8003D740
    /* 2DE30 8003D630 20000224 */   addiu     $v0, $zero, 0x20
    /* 2DE34 8003D634 31000212 */  beq        $s0, $v0, .L8003D6FC
    /* 2DE38 8003D638 0A000224 */   addiu     $v0, $zero, 0xA
    /* 2DE3C 8003D63C 06000216 */  bne        $s0, $v0, .L8003D658
    /* 2DE40 8003D640 7E000224 */   addiu     $v0, $zero, 0x7E
    /* 2DE44 8003D644 0100A224 */  addiu      $v0, $a1, 0x1
    /* 2DE48 8003D648 900280AF */  sw         $zero, %gp_rel(D_800A335C)($gp)
    /* 2DE4C 8003D64C 940282AF */  sw         $v0, %gp_rel(D_800A3360)($gp)
    /* 2DE50 8003D650 C3F50008 */  j          .L8003D70C
    /* 2DE54 8003D654 00000000 */   nop
  .L8003D658:
    /* 2DE58 8003D658 21000216 */  bne        $s0, $v0, .L8003D6E0
    /* 2DE5C 8003D65C 21300002 */   addu      $a2, $s0, $zero
    /* 2DE60 8003D660 00003092 */  lbu        $s0, 0x0($s1)
    /* 2DE64 8003D664 00000000 */  nop
    /* 2DE68 8003D668 35000012 */  beqz       $s0, .L8003D740
    /* 2DE6C 8003D66C 01003126 */   addiu     $s1, $s1, 0x1
    /* 2DE70 8003D670 63000224 */  addiu      $v0, $zero, 0x63
    /* 2DE74 8003D674 03000212 */  beq        $s0, $v0, .L8003D684
    /* 2DE78 8003D678 43000224 */   addiu     $v0, $zero, 0x43
    /* 2DE7C 8003D67C 23000216 */  bne        $s0, $v0, .L8003D70C
    /* 2DE80 8003D680 00000000 */   nop
  .L8003D684:
    /* 2DE84 8003D684 00002292 */  lbu        $v0, 0x0($s1)
    /* 2DE88 8003D688 00000000 */  nop
    /* 2DE8C 8003D68C 2C004010 */  beqz       $v0, .L8003D740
    /* 2DE90 8003D690 01003126 */   addiu     $s1, $s1, 0x1
    /* 2DE94 8003D694 00002392 */  lbu        $v1, 0x0($s1)
    /* 2DE98 8003D698 00000000 */  nop
    /* 2DE9C 8003D69C 28006010 */  beqz       $v1, .L8003D740
    /* 2DEA0 8003D6A0 01003126 */   addiu     $s1, $s1, 0x1
    /* 2DEA4 8003D6A4 00002492 */  lbu        $a0, 0x0($s1)
    /* 2DEA8 8003D6A8 00000000 */  nop
    /* 2DEAC 8003D6AC 24008010 */  beqz       $a0, .L8003D740
    /* 2DEB0 8003D6B0 01003126 */   addiu     $s1, $s1, 0x1
    /* 2DEB4 8003D6B4 D0FF4224 */  addiu      $v0, $v0, -0x30
    /* 2DEB8 8003D6B8 40110200 */  sll        $v0, $v0, 5
    /* 2DEBC 8003D6BC D0FF6324 */  addiu      $v1, $v1, -0x30
    /* 2DEC0 8003D6C0 401B0300 */  sll        $v1, $v1, 13
    /* 2DEC4 8003D6C4 25104300 */  or         $v0, $v0, $v1
    /* 2DEC8 8003D6C8 D0FF8324 */  addiu      $v1, $a0, -0x30
    /* 2DECC 8003D6CC 401D0300 */  sll        $v1, $v1, 21
    /* 2DED0 8003D6D0 25104300 */  or         $v0, $v0, $v1
    /* 2DED4 8003D6D4 980282AF */  sw         $v0, %gp_rel(D_800A3364)($gp)
    /* 2DED8 8003D6D8 C3F50008 */  j          .L8003D70C
    /* 2DEDC 8003D6DC 00000000 */   nop
  .L8003D6E0:
    /* 2DEE0 8003D6E0 C0280500 */  sll        $a1, $a1, 3
    /* 2DEE4 8003D6E4 1000A524 */  addiu      $a1, $a1, 0x10
    /* 2DEE8 8003D6E8 9002848F */  lw         $a0, %gp_rel(D_800A335C)($gp)
    /* 2DEEC 8003D6EC 9802878F */  lw         $a3, %gp_rel(D_800A3364)($gp)
    /* 2DEF0 8003D6F0 C0200400 */  sll        $a0, $a0, 3
    /* 2DEF4 8003D6F4 E7F4000C */  jal        func_8003D39C
    /* 2DEF8 8003D6F8 10008424 */   addiu     $a0, $a0, 0x10
  .L8003D6FC:
    /* 2DEFC 8003D6FC 9002828F */  lw         $v0, %gp_rel(D_800A335C)($gp)
    /* 2DF00 8003D700 00000000 */  nop
    /* 2DF04 8003D704 01004224 */  addiu      $v0, $v0, 0x1
    /* 2DF08 8003D708 900282AF */  sw         $v0, %gp_rel(D_800A335C)($gp)
  .L8003D70C:
    /* 2DF0C 8003D70C 9002828F */  lw         $v0, %gp_rel(D_800A335C)($gp)
    /* 2DF10 8003D710 00000000 */  nop
    /* 2DF14 8003D714 4C004228 */  slti       $v0, $v0, 0x4C
    /* 2DF18 8003D718 05004014 */  bnez       $v0, .L8003D730
    /* 2DF1C 8003D71C 00000000 */   nop
    /* 2DF20 8003D720 9402828F */  lw         $v0, %gp_rel(D_800A3360)($gp)
    /* 2DF24 8003D724 900280AF */  sw         $zero, %gp_rel(D_800A335C)($gp)
    /* 2DF28 8003D728 01004224 */  addiu      $v0, $v0, 0x1
    /* 2DF2C 8003D72C 940282AF */  sw         $v0, %gp_rel(D_800A3360)($gp)
  .L8003D730:
    /* 2DF30 8003D730 00003092 */  lbu        $s0, 0x0($s1)
    /* 2DF34 8003D734 00000000 */  nop
    /* 2DF38 8003D738 B9FF0016 */  bnez       $s0, .L8003D620
    /* 2DF3C 8003D73C 01003126 */   addiu     $s1, $s1, 0x1
  .L8003D740:
    /* 2DF40 8003D740 3405BF8F */  lw         $ra, 0x534($sp)
    /* 2DF44 8003D744 3005BE8F */  lw         $fp, 0x530($sp)
    /* 2DF48 8003D748 2C05B78F */  lw         $s7, 0x52C($sp)
    /* 2DF4C 8003D74C 2805B68F */  lw         $s6, 0x528($sp)
    /* 2DF50 8003D750 2405B58F */  lw         $s5, 0x524($sp)
    /* 2DF54 8003D754 2005B48F */  lw         $s4, 0x520($sp)
    /* 2DF58 8003D758 1C05B38F */  lw         $s3, 0x51C($sp)
    /* 2DF5C 8003D75C 1805B28F */  lw         $s2, 0x518($sp)
    /* 2DF60 8003D760 1405B18F */  lw         $s1, 0x514($sp)
    /* 2DF64 8003D764 1005B08F */  lw         $s0, 0x510($sp)
    /* 2DF68 8003D768 3805BD27 */  addiu      $sp, $sp, 0x538
    /* 2DF6C 8003D76C 0800E003 */  jr         $ra
    /* 2DF70 8003D770 00000000 */   nop
endlabel func_8003D52C
