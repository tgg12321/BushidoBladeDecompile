glabel func_8004A1FC
    /* 3A9FC 8004A1FC D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* 3AA00 8004A200 2000B4AF */  sw         $s4, 0x20($sp)
    /* 3AA04 8004A204 21A08000 */  addu       $s4, $a0, $zero
    /* 3AA08 8004A208 2400B5AF */  sw         $s5, 0x24($sp)
    /* 3AA0C 8004A20C 21A80000 */  addu       $s5, $zero, $zero
    /* 3AA10 8004A210 2800BFAF */  sw         $ra, 0x28($sp)
    /* 3AA14 8004A214 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 3AA18 8004A218 1800B2AF */  sw         $s2, 0x18($sp)
    /* 3AA1C 8004A21C 1400B1AF */  sw         $s1, 0x14($sp)
    /* 3AA20 8004A220 1000B0AF */  sw         $s0, 0x10($sp)
    /* 3AA24 8004A224 00141500 */  sll        $v0, $s5, 16
  .L8004A228:
    /* 3AA28 8004A228 03940200 */  sra        $s2, $v0, 16
    /* 3AA2C 8004A22C C0101200 */  sll        $v0, $s2, 3
    /* 3AA30 8004A230 21988202 */  addu       $s3, $s4, $v0
    /* 3AA34 8004A234 04006286 */  lh         $v0, 0x4($s3)
    /* 3AA38 8004A238 00000000 */  nop
    /* 3AA3C 8004A23C 2B004010 */  beqz       $v0, .L8004A2EC
    /* 3AA40 8004A240 40101200 */   sll       $v0, $s2, 1
    /* 3AA44 8004A244 00006486 */  lh         $a0, 0x0($s3)
    /* 3AA48 8004A248 FBF7010C */  jal        func_8007DFEC
    /* 3AA4C 8004A24C 40801200 */   sll       $s0, $s2, 1
    /* 3AA50 8004A250 02006486 */  lh         $a0, 0x2($s3)
    /* 3AA54 8004A254 C8F7010C */  jal        func_8007DF20
    /* 3AA58 8004A258 21884000 */   addu      $s1, $v0, $zero
    /* 3AA5C 8004A25C 008C1100 */  sll        $s1, $s1, 16
    /* 3AA60 8004A260 038C1100 */  sra        $s1, $s1, 16
    /* 3AA64 8004A264 18005100 */  mult       $v0, $s1
    /* 3AA68 8004A268 12280000 */  mflo       $a1
    /* 3AA6C 8004A26C 5C008386 */  lh         $v1, 0x5C($s4)
    /* 3AA70 8004A270 03130500 */  sra        $v0, $a1, 12
    /* 3AA74 8004A274 18004300 */  mult       $v0, $v1
    /* 3AA78 8004A278 21801202 */  addu       $s0, $s0, $s2
    /* 3AA7C 8004A27C 40801000 */  sll        $s0, $s0, 1
    /* 3AA80 8004A280 21809002 */  addu       $s0, $s4, $s0
    /* 3AA84 8004A284 12280000 */  mflo       $a1
    /* 3AA88 8004A288 23100500 */  negu       $v0, $a1
    /* 3AA8C 8004A28C 03130200 */  sra        $v0, $v0, 12
    /* 3AA90 8004A290 180002A6 */  sh         $v0, 0x18($s0)
    /* 3AA94 8004A294 00006486 */  lh         $a0, 0x0($s3)
    /* 3AA98 8004A298 C8F7010C */  jal        func_8007DF20
    /* 3AA9C 8004A29C 00000000 */   nop
    /* 3AAA0 8004A2A0 5C008386 */  lh         $v1, 0x5C($s4)
    /* 3AAA4 8004A2A4 00000000 */  nop
    /* 3AAA8 8004A2A8 18004300 */  mult       $v0, $v1
    /* 3AAAC 8004A2AC 12280000 */  mflo       $a1
    /* 3AAB0 8004A2B0 03130500 */  sra        $v0, $a1, 12
    /* 3AAB4 8004A2B4 1A0002A6 */  sh         $v0, 0x1A($s0)
    /* 3AAB8 8004A2B8 02006486 */  lh         $a0, 0x2($s3)
    /* 3AABC 8004A2BC FBF7010C */  jal        func_8007DFEC
    /* 3AAC0 8004A2C0 00000000 */   nop
    /* 3AAC4 8004A2C4 18005100 */  mult       $v0, $s1
    /* 3AAC8 8004A2C8 12280000 */  mflo       $a1
    /* 3AACC 8004A2CC 5C008386 */  lh         $v1, 0x5C($s4)
    /* 3AAD0 8004A2D0 03130500 */  sra        $v0, $a1, 12
    /* 3AAD4 8004A2D4 18004300 */  mult       $v0, $v1
    /* 3AAD8 8004A2D8 12280000 */  mflo       $a1
    /* 3AADC 8004A2DC 23100500 */  negu       $v0, $a1
    /* 3AAE0 8004A2E0 03130200 */  sra        $v0, $v0, 12
    /* 3AAE4 8004A2E4 C1280108 */  j          .L8004A304
    /* 3AAE8 8004A2E8 1C0002A6 */   sh        $v0, 0x1C($s0)
  .L8004A2EC:
    /* 3AAEC 8004A2EC 21105200 */  addu       $v0, $v0, $s2
    /* 3AAF0 8004A2F0 40100200 */  sll        $v0, $v0, 1
    /* 3AAF4 8004A2F4 21108202 */  addu       $v0, $s4, $v0
    /* 3AAF8 8004A2F8 180040A4 */  sh         $zero, 0x18($v0)
    /* 3AAFC 8004A2FC 1A0040A4 */  sh         $zero, 0x1A($v0)
    /* 3AB00 8004A300 1C0040A4 */  sh         $zero, 0x1C($v0)
  .L8004A304:
    /* 3AB04 8004A304 0100A226 */  addiu      $v0, $s5, 0x1
    /* 3AB08 8004A308 21A84000 */  addu       $s5, $v0, $zero
    /* 3AB0C 8004A30C 00140200 */  sll        $v0, $v0, 16
    /* 3AB10 8004A310 03140200 */  sra        $v0, $v0, 16
    /* 3AB14 8004A314 03004228 */  slti       $v0, $v0, 0x3
    /* 3AB18 8004A318 C3FF4014 */  bnez       $v0, .L8004A228
    /* 3AB1C 8004A31C 00141500 */   sll       $v0, $s5, 16
    /* 3AB20 8004A320 2800BF8F */  lw         $ra, 0x28($sp)
    /* 3AB24 8004A324 2400B58F */  lw         $s5, 0x24($sp)
    /* 3AB28 8004A328 2000B48F */  lw         $s4, 0x20($sp)
    /* 3AB2C 8004A32C 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 3AB30 8004A330 1800B28F */  lw         $s2, 0x18($sp)
    /* 3AB34 8004A334 1400B18F */  lw         $s1, 0x14($sp)
    /* 3AB38 8004A338 1000B08F */  lw         $s0, 0x10($sp)
    /* 3AB3C 8004A33C 3000BD27 */  addiu      $sp, $sp, 0x30
    /* 3AB40 8004A340 0800E003 */  jr         $ra
    /* 3AB44 8004A344 00000000 */   nop
endlabel func_8004A1FC
