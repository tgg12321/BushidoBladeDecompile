glabel func_8007C4B8
    /* 6CCB8 8007C4B8 C0FFBD27 */  addiu      $sp, $sp, -0x40
    /* 6CCBC 8007C4BC 3000B0AF */  sw         $s0, 0x30($sp)
    /* 6CCC0 8007C4C0 2180A000 */  addu       $s0, $a1, $zero
    /* 6CCC4 8007C4C4 3400B1AF */  sw         $s1, 0x34($sp)
    /* 6CCC8 8007C4C8 21888000 */  addu       $s1, $a0, $zero
    /* 6CCCC 8007C4CC 3800BFAF */  sw         $ra, 0x38($sp)
    /* 6CCD0 8007C4D0 00000486 */  lh         $a0, 0x0($s0)
    /* 6CCD4 8007C4D4 02000586 */  lh         $a1, 0x2($s0)
    /* 6CCD8 8007C4D8 E8F1010C */  jal        func_8007C7A0
    /* 6CCDC 8007C4DC 00000000 */   nop
    /* 6CCE0 8007C4E0 040022AE */  sw         $v0, 0x4($s1)
    /* 6CCE4 8007C4E4 04000496 */  lhu        $a0, 0x4($s0)
    /* 6CCE8 8007C4E8 00000296 */  lhu        $v0, 0x0($s0)
    /* 6CCEC 8007C4EC 02000596 */  lhu        $a1, 0x2($s0)
    /* 6CCF0 8007C4F0 21208200 */  addu       $a0, $a0, $v0
    /* 6CCF4 8007C4F4 FFFF8424 */  addiu      $a0, $a0, -0x1
    /* 6CCF8 8007C4F8 00240400 */  sll        $a0, $a0, 16
    /* 6CCFC 8007C4FC 06000296 */  lhu        $v0, 0x6($s0)
    /* 6CD00 8007C500 03240400 */  sra        $a0, $a0, 16
    /* 6CD04 8007C504 2128A200 */  addu       $a1, $a1, $v0
    /* 6CD08 8007C508 FFFFA524 */  addiu      $a1, $a1, -0x1
    /* 6CD0C 8007C50C 002C0500 */  sll        $a1, $a1, 16
    /* 6CD10 8007C510 1BF2010C */  jal        func_8007C86C
    /* 6CD14 8007C514 032C0500 */   sra       $a1, $a1, 16
    /* 6CD18 8007C518 080022AE */  sw         $v0, 0x8($s1)
    /* 6CD1C 8007C51C 08000486 */  lh         $a0, 0x8($s0)
    /* 6CD20 8007C520 0A000586 */  lh         $a1, 0xA($s0)
    /* 6CD24 8007C524 4EF2010C */  jal        func_8007C938
    /* 6CD28 8007C528 00000000 */   nop
    /* 6CD2C 8007C52C 0C0022AE */  sw         $v0, 0xC($s1)
    /* 6CD30 8007C530 17000492 */  lbu        $a0, 0x17($s0)
    /* 6CD34 8007C534 16000592 */  lbu        $a1, 0x16($s0)
    /* 6CD38 8007C538 14000696 */  lhu        $a2, 0x14($s0)
    /* 6CD3C 8007C53C D2F1010C */  jal        func_8007C748
    /* 6CD40 8007C540 00000000 */   nop
    /* 6CD44 8007C544 0C000426 */  addiu      $a0, $s0, 0xC
    /* 6CD48 8007C548 5FF2010C */  jal        func_8007C97C
    /* 6CD4C 8007C54C 100022AE */   sw        $v0, 0x10($s1)
    /* 6CD50 8007C550 140022AE */  sw         $v0, 0x14($s1)
    /* 6CD54 8007C554 00E6023C */  lui        $v0, (0xE6000000 >> 16)
    /* 6CD58 8007C558 180022AE */  sw         $v0, 0x18($s1)
    /* 6CD5C 8007C55C 18000292 */  lbu        $v0, 0x18($s0)
    /* 6CD60 8007C560 00000000 */  nop
    /* 6CD64 8007C564 70004010 */  beqz       $v0, .L8007C728
    /* 6CD68 8007C568 07000824 */   addiu     $t0, $zero, 0x7
    /* 6CD6C 8007C56C 00000296 */  lhu        $v0, 0x0($s0)
    /* 6CD70 8007C570 00000000 */  nop
    /* 6CD74 8007C574 1000A2A7 */  sh         $v0, 0x10($sp)
    /* 6CD78 8007C578 02000296 */  lhu        $v0, 0x2($s0)
    /* 6CD7C 8007C57C 00000000 */  nop
    /* 6CD80 8007C580 1200A2A7 */  sh         $v0, 0x12($sp)
    /* 6CD84 8007C584 04000496 */  lhu        $a0, 0x4($s0)
    /* 6CD88 8007C588 00000000 */  nop
    /* 6CD8C 8007C58C 1400A4A7 */  sh         $a0, 0x14($sp)
    /* 6CD90 8007C590 06000296 */  lhu        $v0, 0x6($s0)
    /* 6CD94 8007C594 00000000 */  nop
    /* 6CD98 8007C598 1600A2A7 */  sh         $v0, 0x16($sp)
    /* 6CD9C 8007C59C 00140400 */  sll        $v0, $a0, 16
    /* 6CDA0 8007C5A0 032C0200 */  sra        $a1, $v0, 16
    /* 6CDA4 8007C5A4 0A00A004 */  bltz       $a1, .L8007C5D0
    /* 6CDA8 8007C5A8 21100000 */   addu      $v0, $zero, $zero
    /* 6CDAC 8007C5AC 0A80023C */  lui        $v0, %hi(D_8009BE78)
    /* 6CDB0 8007C5B0 78BE4284 */  lh         $v0, %lo(D_8009BE78)($v0)
    /* 6CDB4 8007C5B4 00000000 */  nop
    /* 6CDB8 8007C5B8 21184000 */  addu       $v1, $v0, $zero
    /* 6CDBC 8007C5BC FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6CDC0 8007C5C0 2A104500 */  slt        $v0, $v0, $a1
    /* 6CDC4 8007C5C4 02004014 */  bnez       $v0, .L8007C5D0
    /* 6CDC8 8007C5C8 FFFF6224 */   addiu     $v0, $v1, -0x1
    /* 6CDCC 8007C5CC 21108000 */  addu       $v0, $a0, $zero
  .L8007C5D0:
    /* 6CDD0 8007C5D0 1600A587 */  lh         $a1, 0x16($sp)
    /* 6CDD4 8007C5D4 1400A2A7 */  sh         $v0, 0x14($sp)
    /* 6CDD8 8007C5D8 0B00A004 */  bltz       $a1, .L8007C608
    /* 6CDDC 8007C5DC 2120A000 */   addu      $a0, $a1, $zero
    /* 6CDE0 8007C5E0 0A80023C */  lui        $v0, %hi(D_8009BE7A)
    /* 6CDE4 8007C5E4 7ABE4284 */  lh         $v0, %lo(D_8009BE7A)($v0)
    /* 6CDE8 8007C5E8 00000000 */  nop
    /* 6CDEC 8007C5EC 21184000 */  addu       $v1, $v0, $zero
    /* 6CDF0 8007C5F0 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6CDF4 8007C5F4 2A104500 */  slt        $v0, $v0, $a1
    /* 6CDF8 8007C5F8 04004014 */  bnez       $v0, .L8007C60C
    /* 6CDFC 8007C5FC FFFF6224 */   addiu     $v0, $v1, -0x1
    /* 6CE00 8007C600 83F10108 */  j          .L8007C60C
    /* 6CE04 8007C604 21108000 */   addu      $v0, $a0, $zero
  .L8007C608:
    /* 6CE08 8007C608 21100000 */  addu       $v0, $zero, $zero
  .L8007C60C:
    /* 6CE0C 8007C60C 1000A397 */  lhu        $v1, 0x10($sp)
    /* 6CE10 8007C610 1600A2A7 */  sh         $v0, 0x16($sp)
    /* 6CE14 8007C614 3F006230 */  andi       $v0, $v1, 0x3F
    /* 6CE18 8007C618 06004014 */  bnez       $v0, .L8007C634
    /* 6CE1C 8007C61C 80300800 */   sll       $a2, $t0, 2
    /* 6CE20 8007C620 1400A297 */  lhu        $v0, 0x14($sp)
    /* 6CE24 8007C624 00000000 */  nop
    /* 6CE28 8007C628 3F004230 */  andi       $v0, $v0, 0x3F
    /* 6CE2C 8007C62C 28004010 */  beqz       $v0, .L8007C6D0
    /* 6CE30 8007C630 80280800 */   sll       $a1, $t0, 2
  .L8007C634:
    /* 6CE34 8007C634 01000825 */  addiu      $t0, $t0, 0x1
    /* 6CE38 8007C638 80280800 */  sll        $a1, $t0, 2
    /* 6CE3C 8007C63C 01000825 */  addiu      $t0, $t0, 0x1
    /* 6CE40 8007C640 08000296 */  lhu        $v0, 0x8($s0)
    /* 6CE44 8007C644 2130D100 */  addu       $a2, $a2, $s1
    /* 6CE48 8007C648 23106200 */  subu       $v0, $v1, $v0
    /* 6CE4C 8007C64C 1000A2A7 */  sh         $v0, 0x10($sp)
    /* 6CE50 8007C650 1200A297 */  lhu        $v0, 0x12($sp)
    /* 6CE54 8007C654 0A000396 */  lhu        $v1, 0xA($s0)
    /* 6CE58 8007C658 0060043C */  lui        $a0, (0x60000000 >> 16)
    /* 6CE5C 8007C65C 23104300 */  subu       $v0, $v0, $v1
    /* 6CE60 8007C660 1200A2A7 */  sh         $v0, 0x12($sp)
    /* 6CE64 8007C664 1B000292 */  lbu        $v0, 0x1B($s0)
    /* 6CE68 8007C668 1A000392 */  lbu        $v1, 0x1A($s0)
    /* 6CE6C 8007C66C 00140200 */  sll        $v0, $v0, 16
    /* 6CE70 8007C670 001A0300 */  sll        $v1, $v1, 8
    /* 6CE74 8007C674 25186400 */  or         $v1, $v1, $a0
    /* 6CE78 8007C678 19000492 */  lbu        $a0, 0x19($s0)
    /* 6CE7C 8007C67C 25104300 */  or         $v0, $v0, $v1
    /* 6CE80 8007C680 25104400 */  or         $v0, $v0, $a0
    /* 6CE84 8007C684 0000C2AC */  sw         $v0, 0x0($a2)
    /* 6CE88 8007C688 1000A28F */  lw         $v0, 0x10($sp)
    /* 6CE8C 8007C68C 2128B100 */  addu       $a1, $a1, $s1
    /* 6CE90 8007C690 0000A2AC */  sw         $v0, 0x0($a1)
    /* 6CE94 8007C694 80100800 */  sll        $v0, $t0, 2
    /* 6CE98 8007C698 1400A38F */  lw         $v1, 0x14($sp)
    /* 6CE9C 8007C69C 21105100 */  addu       $v0, $v0, $s1
    /* 6CEA0 8007C6A0 000043AC */  sw         $v1, 0x0($v0)
    /* 6CEA4 8007C6A4 1000A297 */  lhu        $v0, 0x10($sp)
    /* 6CEA8 8007C6A8 08000396 */  lhu        $v1, 0x8($s0)
    /* 6CEAC 8007C6AC 00000000 */  nop
    /* 6CEB0 8007C6B0 21104300 */  addu       $v0, $v0, $v1
    /* 6CEB4 8007C6B4 1000A2A7 */  sh         $v0, 0x10($sp)
    /* 6CEB8 8007C6B8 1200A297 */  lhu        $v0, 0x12($sp)
    /* 6CEBC 8007C6BC 0A000396 */  lhu        $v1, 0xA($s0)
    /* 6CEC0 8007C6C0 01000825 */  addiu      $t0, $t0, 0x1
    /* 6CEC4 8007C6C4 21104300 */  addu       $v0, $v0, $v1
    /* 6CEC8 8007C6C8 CAF10108 */  j          .L8007C728
    /* 6CECC 8007C6CC 1200A2A7 */   sh        $v0, 0x12($sp)
  .L8007C6D0:
    /* 6CED0 8007C6D0 01000825 */  addiu      $t0, $t0, 0x1
    /* 6CED4 8007C6D4 80300800 */  sll        $a2, $t0, 2
    /* 6CED8 8007C6D8 01000825 */  addiu      $t0, $t0, 0x1
    /* 6CEDC 8007C6DC 80380800 */  sll        $a3, $t0, 2
    /* 6CEE0 8007C6E0 01000825 */  addiu      $t0, $t0, 0x1
    /* 6CEE4 8007C6E4 2128B100 */  addu       $a1, $a1, $s1
    /* 6CEE8 8007C6E8 0002043C */  lui        $a0, (0x2000000 >> 16)
    /* 6CEEC 8007C6EC 1B000292 */  lbu        $v0, 0x1B($s0)
    /* 6CEF0 8007C6F0 1A000392 */  lbu        $v1, 0x1A($s0)
    /* 6CEF4 8007C6F4 00140200 */  sll        $v0, $v0, 16
    /* 6CEF8 8007C6F8 001A0300 */  sll        $v1, $v1, 8
    /* 6CEFC 8007C6FC 25186400 */  or         $v1, $v1, $a0
    /* 6CF00 8007C700 19000492 */  lbu        $a0, 0x19($s0)
    /* 6CF04 8007C704 25104300 */  or         $v0, $v0, $v1
    /* 6CF08 8007C708 25104400 */  or         $v0, $v0, $a0
    /* 6CF0C 8007C70C 0000A2AC */  sw         $v0, 0x0($a1)
    /* 6CF10 8007C710 1000A28F */  lw         $v0, 0x10($sp)
    /* 6CF14 8007C714 2130D100 */  addu       $a2, $a2, $s1
    /* 6CF18 8007C718 0000C2AC */  sw         $v0, 0x0($a2)
    /* 6CF1C 8007C71C 1400A28F */  lw         $v0, 0x14($sp)
    /* 6CF20 8007C720 2138F100 */  addu       $a3, $a3, $s1
    /* 6CF24 8007C724 0000E2AC */  sw         $v0, 0x0($a3)
  .L8007C728:
    /* 6CF28 8007C728 FFFF0225 */  addiu      $v0, $t0, -0x1
    /* 6CF2C 8007C72C 030022A2 */  sb         $v0, 0x3($s1)
    /* 6CF30 8007C730 3800BF8F */  lw         $ra, 0x38($sp)
    /* 6CF34 8007C734 3400B18F */  lw         $s1, 0x34($sp)
    /* 6CF38 8007C738 3000B08F */  lw         $s0, 0x30($sp)
    /* 6CF3C 8007C73C 4000BD27 */  addiu      $sp, $sp, 0x40
    /* 6CF40 8007C740 0800E003 */  jr         $ra
    /* 6CF44 8007C744 00000000 */   nop
endlabel func_8007C4B8
