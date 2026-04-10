glabel func_8007BC08
    /* 6C408 8007BC08 0A80023C */  lui        $v0, %hi(D_8009BE76)
    /* 6C40C 8007BC0C 76BE4290 */  lbu        $v0, %lo(D_8009BE76)($v0)
    /* 6C410 8007BC10 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 6C414 8007BC14 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C418 8007BC18 21808000 */  addu       $s0, $a0, $zero
    /* 6C41C 8007BC1C 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 6C420 8007BC20 0008133C */  lui        $s3, (0x8000008 >> 16)
    /* 6C424 8007BC24 2000BFAF */  sw         $ra, 0x20($sp)
    /* 6C428 8007BC28 1800B2AF */  sw         $s2, 0x18($sp)
    /* 6C42C 8007BC2C 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6C430 8007BC30 08004014 */  bnez       $v0, .L8007BC54
    /* 6C434 8007BC34 1400B1AF */   sw        $s1, 0x14($sp)
    /* 6C438 8007BC38 0180043C */  lui        $a0, %hi(D_80015FF8)
    /* 6C43C 8007BC3C F85F8424 */  addiu      $a0, $a0, %lo(D_80015FF8)
    /* 6C440 8007BC40 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6C444 8007BC44 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6C448 8007BC48 00000000 */  nop
    /* 6C44C 8007BC4C 09F84000 */  jalr       $v0
    /* 6C450 8007BC50 21280002 */   addu      $a1, $s0, $zero
  .L8007BC54:
    /* 6C454 8007BC54 0A80023C */  lui        $v0, %hi(D_8009BE74)
    /* 6C458 8007BC58 74BE4290 */  lbu        $v0, %lo(D_8009BE74)($v0)
    /* 6C45C 8007BC5C 00000000 */  nop
    /* 6C460 8007BC60 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6C464 8007BC64 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6C468 8007BC68 0A004010 */  beqz       $v0, .L8007BC94
    /* 6C46C 8007BC6C 00000000 */   nop
    /* 6C470 8007BC70 80F2010C */  jal        func_8007CA00
    /* 6C474 8007BC74 21200002 */   addu      $a0, $s0, $zero
    /* 6C478 8007BC78 02000396 */  lhu        $v1, 0x2($s0)
    /* 6C47C 8007BC7C FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 6C480 8007BC80 FF0F6330 */  andi       $v1, $v1, 0xFFF
    /* 6C484 8007BC84 001B0300 */  sll        $v1, $v1, 12
    /* 6C488 8007BC88 25186200 */  or         $v1, $v1, $v0
    /* 6C48C 8007BC8C 2CEF0108 */  j          .L8007BCB0
    /* 6C490 8007BC90 0005023C */   lui       $v0, (0x5000000 >> 16)
  .L8007BC94:
    /* 6C494 8007BC94 02000296 */  lhu        $v0, 0x2($s0)
    /* 6C498 8007BC98 00000396 */  lhu        $v1, 0x0($s0)
    /* 6C49C 8007BC9C FF034230 */  andi       $v0, $v0, 0x3FF
    /* 6C4A0 8007BCA0 80120200 */  sll        $v0, $v0, 10
    /* 6C4A4 8007BCA4 FF036330 */  andi       $v1, $v1, 0x3FF
    /* 6C4A8 8007BCA8 25104300 */  or         $v0, $v0, $v1
    /* 6C4AC 8007BCAC 0005033C */  lui        $v1, (0x5000000 >> 16)
  .L8007BCB0:
    /* 6C4B0 8007BCB0 25204300 */  or         $a0, $v0, $v1
    /* 6C4B4 8007BCB4 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6C4B8 8007BCB8 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6C4BC 8007BCBC 00000000 */  nop
    /* 6C4C0 8007BCC0 1000428C */  lw         $v0, 0x10($v0)
    /* 6C4C4 8007BCC4 00000000 */  nop
    /* 6C4C8 8007BCC8 09F84000 */  jalr       $v0
    /* 6C4CC 8007BCCC 00000000 */   nop
    /* 6C4D0 8007BCD0 0A80023C */  lui        $v0, %hi(D_8009BEE8)
    /* 6C4D4 8007BCD4 E8BE4224 */  addiu      $v0, $v0, %lo(D_8009BEE8)
    /* 6C4D8 8007BCD8 00004294 */  lhu        $v0, 0x0($v0)
    /* 6C4DC 8007BCDC 08000386 */  lh         $v1, 0x8($s0)
    /* 6C4E0 8007BCE0 00140200 */  sll        $v0, $v0, 16
    /* 6C4E4 8007BCE4 03140200 */  sra        $v0, $v0, 16
    /* 6C4E8 8007BCE8 16004314 */  bne        $v0, $v1, .L8007BD44
    /* 6C4EC 8007BCEC 00000000 */   nop
    /* 6C4F0 8007BCF0 0A80023C */  lui        $v0, %hi(D_8009BEEA)
    /* 6C4F4 8007BCF4 EABE4294 */  lhu        $v0, %lo(D_8009BEEA)($v0)
    /* 6C4F8 8007BCF8 0A000386 */  lh         $v1, 0xA($s0)
    /* 6C4FC 8007BCFC 00140200 */  sll        $v0, $v0, 16
    /* 6C500 8007BD00 03140200 */  sra        $v0, $v0, 16
    /* 6C504 8007BD04 0F004314 */  bne        $v0, $v1, .L8007BD44
    /* 6C508 8007BD08 00000000 */   nop
    /* 6C50C 8007BD0C 0A80023C */  lui        $v0, %hi(D_8009BEEC)
    /* 6C510 8007BD10 ECBE4294 */  lhu        $v0, %lo(D_8009BEEC)($v0)
    /* 6C514 8007BD14 0C000386 */  lh         $v1, 0xC($s0)
    /* 6C518 8007BD18 00140200 */  sll        $v0, $v0, 16
    /* 6C51C 8007BD1C 03140200 */  sra        $v0, $v0, 16
    /* 6C520 8007BD20 08004314 */  bne        $v0, $v1, .L8007BD44
    /* 6C524 8007BD24 00000000 */   nop
    /* 6C528 8007BD28 0A80023C */  lui        $v0, %hi(D_8009BEEE)
    /* 6C52C 8007BD2C EEBE4294 */  lhu        $v0, %lo(D_8009BEEE)($v0)
    /* 6C530 8007BD30 0E000386 */  lh         $v1, 0xE($s0)
    /* 6C534 8007BD34 00140200 */  sll        $v0, $v0, 16
    /* 6C538 8007BD38 03140200 */  sra        $v0, $v0, 16
    /* 6C53C 8007BD3C 73004310 */  beq        $v0, $v1, .L8007BF0C
    /* 6C540 8007BD40 00000000 */   nop
  .L8007BD44:
    /* 6C544 8007BD44 A20D020C */  jal        sys_GetVideoMode
    /* 6C548 8007BD48 00000000 */   nop
    /* 6C54C 8007BD4C 08000486 */  lh         $a0, 0x8($s0)
    /* 6C550 8007BD50 120002A2 */  sb         $v0, 0x12($s0)
    /* 6C554 8007BD54 FF004230 */  andi       $v0, $v0, 0xFF
    /* 6C558 8007BD58 80180400 */  sll        $v1, $a0, 2
    /* 6C55C 8007BD5C 21186400 */  addu       $v1, $v1, $a0
    /* 6C560 8007BD60 40180300 */  sll        $v1, $v1, 1
    /* 6C564 8007BD64 0A000486 */  lh         $a0, 0xA($s0)
    /* 6C568 8007BD68 03004010 */  beqz       $v0, .L8007BD78
    /* 6C56C 8007BD6C 60026324 */   addiu     $v1, $v1, 0x260
    /* 6C570 8007BD70 5FEF0108 */  j          .L8007BD7C
    /* 6C574 8007BD74 13009124 */   addiu     $s1, $a0, 0x13
  .L8007BD78:
    /* 6C578 8007BD78 10009124 */  addiu      $s1, $a0, 0x10
  .L8007BD7C:
    /* 6C57C 8007BD7C 0C000586 */  lh         $a1, 0xC($s0)
    /* 6C580 8007BD80 00000000 */  nop
    /* 6C584 8007BD84 0500A010 */  beqz       $a1, .L8007BD9C
    /* 6C588 8007BD88 80100500 */   sll       $v0, $a1, 2
    /* 6C58C 8007BD8C 21104500 */  addu       $v0, $v0, $a1
    /* 6C590 8007BD90 40100200 */  sll        $v0, $v0, 1
    /* 6C594 8007BD94 68EF0108 */  j          .L8007BDA0
    /* 6C598 8007BD98 21306200 */   addu      $a2, $v1, $v0
  .L8007BD9C:
    /* 6C59C 8007BD9C 000A6624 */  addiu      $a2, $v1, 0xA00
  .L8007BDA0:
    /* 6C5A0 8007BDA0 0E000286 */  lh         $v0, 0xE($s0)
    /* 6C5A4 8007BDA4 00000000 */  nop
    /* 6C5A8 8007BDA8 02004014 */  bnez       $v0, .L8007BDB4
    /* 6C5AC 8007BDAC 21902202 */   addu      $s2, $s1, $v0
    /* 6C5B0 8007BDB0 F0003226 */  addiu      $s2, $s1, 0xF0
  .L8007BDB4:
    /* 6C5B4 8007BDB4 F4016228 */  slti       $v0, $v1, 0x1F4
    /* 6C5B8 8007BDB8 05004014 */  bnez       $v0, .L8007BDD0
    /* 6C5BC 8007BDBC DB0C6228 */   slti      $v0, $v1, 0xCDB
    /* 6C5C0 8007BDC0 04004010 */  beqz       $v0, .L8007BDD4
    /* 6C5C4 8007BDC4 DA0C0524 */   addiu     $a1, $zero, 0xCDA
    /* 6C5C8 8007BDC8 75EF0108 */  j          .L8007BDD4
    /* 6C5CC 8007BDCC 21286000 */   addu      $a1, $v1, $zero
  .L8007BDD0:
    /* 6C5D0 8007BDD0 F4010524 */  addiu      $a1, $zero, 0x1F4
  .L8007BDD4:
    /* 6C5D4 8007BDD4 2118A000 */  addu       $v1, $a1, $zero
    /* 6C5D8 8007BDD8 50006524 */  addiu      $a1, $v1, 0x50
    /* 6C5DC 8007BDDC 2A10C500 */  slt        $v0, $a2, $a1
    /* 6C5E0 8007BDE0 06004014 */  bnez       $v0, .L8007BDFC
    /* 6C5E4 8007BDE4 1000222A */   slti      $v0, $s1, 0x10
    /* 6C5E8 8007BDE8 DB0CC228 */  slti       $v0, $a2, 0xCDB
    /* 6C5EC 8007BDEC 02004010 */  beqz       $v0, .L8007BDF8
    /* 6C5F0 8007BDF0 DA0C0524 */   addiu     $a1, $zero, 0xCDA
    /* 6C5F4 8007BDF4 2128C000 */  addu       $a1, $a2, $zero
  .L8007BDF8:
    /* 6C5F8 8007BDF8 1000222A */  slti       $v0, $s1, 0x10
  .L8007BDFC:
    /* 6C5FC 8007BDFC 14004014 */  bnez       $v0, .L8007BE50
    /* 6C600 8007BE00 2130A000 */   addu      $a2, $a1, $zero
    /* 6C604 8007BE04 12000292 */  lbu        $v0, 0x12($s0)
    /* 6C608 8007BE08 00000000 */  nop
    /* 6C60C 8007BE0C 05004010 */  beqz       $v0, .L8007BE24
    /* 6C610 8007BE10 3701222A */   slti      $v0, $s1, 0x137
    /* 6C614 8007BE14 06004010 */  beqz       $v0, .L8007BE30
    /* 6C618 8007BE18 00000000 */   nop
    /* 6C61C 8007BE1C 95EF0108 */  j          .L8007BE54
    /* 6C620 8007BE20 21202002 */   addu      $a0, $s1, $zero
  .L8007BE24:
    /* 6C624 8007BE24 0101222A */  slti       $v0, $s1, 0x101
    /* 6C628 8007BE28 07004014 */  bnez       $v0, .L8007BE48
    /* 6C62C 8007BE2C 00000000 */   nop
  .L8007BE30:
    /* 6C630 8007BE30 12000292 */  lbu        $v0, 0x12($s0)
    /* 6C634 8007BE34 00000000 */  nop
    /* 6C638 8007BE38 06004010 */  beqz       $v0, .L8007BE54
    /* 6C63C 8007BE3C 00010424 */   addiu     $a0, $zero, 0x100
    /* 6C640 8007BE40 95EF0108 */  j          .L8007BE54
    /* 6C644 8007BE44 36010424 */   addiu     $a0, $zero, 0x136
  .L8007BE48:
    /* 6C648 8007BE48 95EF0108 */  j          .L8007BE54
    /* 6C64C 8007BE4C 21202002 */   addu      $a0, $s1, $zero
  .L8007BE50:
    /* 6C650 8007BE50 10000424 */  addiu      $a0, $zero, 0x10
  .L8007BE54:
    /* 6C654 8007BE54 21888000 */  addu       $s1, $a0, $zero
    /* 6C658 8007BE58 02002526 */  addiu      $a1, $s1, 0x2
    /* 6C65C 8007BE5C 2A104502 */  slt        $v0, $s2, $a1
    /* 6C660 8007BE60 13004014 */  bnez       $v0, .L8007BEB0
    /* 6C664 8007BE64 00000000 */   nop
    /* 6C668 8007BE68 12000292 */  lbu        $v0, 0x12($s0)
    /* 6C66C 8007BE6C 00000000 */  nop
    /* 6C670 8007BE70 05004010 */  beqz       $v0, .L8007BE88
    /* 6C674 8007BE74 3901422A */   slti      $v0, $s2, 0x139
    /* 6C678 8007BE78 06004010 */  beqz       $v0, .L8007BE94
    /* 6C67C 8007BE7C 00000000 */   nop
    /* 6C680 8007BE80 ACEF0108 */  j          .L8007BEB0
    /* 6C684 8007BE84 21284002 */   addu      $a1, $s2, $zero
  .L8007BE88:
    /* 6C688 8007BE88 0301422A */  slti       $v0, $s2, 0x103
    /* 6C68C 8007BE8C 07004014 */  bnez       $v0, .L8007BEAC
    /* 6C690 8007BE90 00000000 */   nop
  .L8007BE94:
    /* 6C694 8007BE94 12000292 */  lbu        $v0, 0x12($s0)
    /* 6C698 8007BE98 00000000 */  nop
    /* 6C69C 8007BE9C 04004010 */  beqz       $v0, .L8007BEB0
    /* 6C6A0 8007BEA0 02010524 */   addiu     $a1, $zero, 0x102
    /* 6C6A4 8007BEA4 ACEF0108 */  j          .L8007BEB0
    /* 6C6A8 8007BEA8 38010524 */   addiu     $a1, $zero, 0x138
  .L8007BEAC:
    /* 6C6AC 8007BEAC 21284002 */  addu       $a1, $s2, $zero
  .L8007BEB0:
    /* 6C6B0 8007BEB0 2190A000 */  addu       $s2, $a1, $zero
    /* 6C6B4 8007BEB4 FF0FC230 */  andi       $v0, $a2, 0xFFF
    /* 6C6B8 8007BEB8 00130200 */  sll        $v0, $v0, 12
    /* 6C6BC 8007BEBC FF0F6430 */  andi       $a0, $v1, 0xFFF
    /* 6C6C0 8007BEC0 0006033C */  lui        $v1, (0x6000000 >> 16)
    /* 6C6C4 8007BEC4 0A80053C */  lui        $a1, %hi(D_8009BE6C)
    /* 6C6C8 8007BEC8 6CBEA58C */  lw         $a1, %lo(D_8009BE6C)($a1)
    /* 6C6CC 8007BECC 25208300 */  or         $a0, $a0, $v1
    /* 6C6D0 8007BED0 1000A38C */  lw         $v1, 0x10($a1)
    /* 6C6D4 8007BED4 00000000 */  nop
    /* 6C6D8 8007BED8 09F86000 */  jalr       $v1
    /* 6C6DC 8007BEDC 25204400 */   or        $a0, $v0, $a0
    /* 6C6E0 8007BEE0 FF034232 */  andi       $v0, $s2, 0x3FF
    /* 6C6E4 8007BEE4 80120200 */  sll        $v0, $v0, 10
    /* 6C6E8 8007BEE8 FF032432 */  andi       $a0, $s1, 0x3FF
    /* 6C6EC 8007BEEC 0007033C */  lui        $v1, (0x7000000 >> 16)
    /* 6C6F0 8007BEF0 0A80053C */  lui        $a1, %hi(D_8009BE6C)
    /* 6C6F4 8007BEF4 6CBEA58C */  lw         $a1, %lo(D_8009BE6C)($a1)
    /* 6C6F8 8007BEF8 25208300 */  or         $a0, $a0, $v1
    /* 6C6FC 8007BEFC 1000A38C */  lw         $v1, 0x10($a1)
    /* 6C700 8007BF00 00000000 */  nop
    /* 6C704 8007BF04 09F86000 */  jalr       $v1
    /* 6C708 8007BF08 25204400 */   or        $a0, $v0, $a0
  .L8007BF0C:
    /* 6C70C 8007BF0C 0A80033C */  lui        $v1, %hi(D_8009BEF0)
    /* 6C710 8007BF10 F0BE638C */  lw         $v1, %lo(D_8009BEF0)($v1)
    /* 6C714 8007BF14 1000028E */  lw         $v0, 0x10($s0)
    /* 6C718 8007BF18 00000000 */  nop
    /* 6C71C 8007BF1C 1D006214 */  bne        $v1, $v0, .L8007BF94
    /* 6C720 8007BF20 00000000 */   nop
    /* 6C724 8007BF24 0A80023C */  lui        $v0, %hi(D_8009BEE0)
    /* 6C728 8007BF28 E0BE4294 */  lhu        $v0, %lo(D_8009BEE0)($v0)
    /* 6C72C 8007BF2C 00000386 */  lh         $v1, 0x0($s0)
    /* 6C730 8007BF30 00140200 */  sll        $v0, $v0, 16
    /* 6C734 8007BF34 03140200 */  sra        $v0, $v0, 16
    /* 6C738 8007BF38 16004314 */  bne        $v0, $v1, .L8007BF94
    /* 6C73C 8007BF3C 00000000 */   nop
    /* 6C740 8007BF40 0A80023C */  lui        $v0, %hi(D_8009BEE2)
    /* 6C744 8007BF44 E2BE4294 */  lhu        $v0, %lo(D_8009BEE2)($v0)
    /* 6C748 8007BF48 02000386 */  lh         $v1, 0x2($s0)
    /* 6C74C 8007BF4C 00140200 */  sll        $v0, $v0, 16
    /* 6C750 8007BF50 03140200 */  sra        $v0, $v0, 16
    /* 6C754 8007BF54 0F004314 */  bne        $v0, $v1, .L8007BF94
    /* 6C758 8007BF58 00000000 */   nop
    /* 6C75C 8007BF5C 0A80023C */  lui        $v0, %hi(D_8009BEE4)
    /* 6C760 8007BF60 E4BE4294 */  lhu        $v0, %lo(D_8009BEE4)($v0)
    /* 6C764 8007BF64 04000386 */  lh         $v1, 0x4($s0)
    /* 6C768 8007BF68 00140200 */  sll        $v0, $v0, 16
    /* 6C76C 8007BF6C 03140200 */  sra        $v0, $v0, 16
    /* 6C770 8007BF70 08004314 */  bne        $v0, $v1, .L8007BF94
    /* 6C774 8007BF74 00000000 */   nop
    /* 6C778 8007BF78 0A80023C */  lui        $v0, %hi(D_8009BEE6)
    /* 6C77C 8007BF7C E6BE4294 */  lhu        $v0, %lo(D_8009BEE6)($v0)
    /* 6C780 8007BF80 06000386 */  lh         $v1, 0x6($s0)
    /* 6C784 8007BF84 00140200 */  sll        $v0, $v0, 16
    /* 6C788 8007BF88 03140200 */  sra        $v0, $v0, 16
    /* 6C78C 8007BF8C 3A004310 */  beq        $v0, $v1, .L8007C078
    /* 6C790 8007BF90 00000000 */   nop
  .L8007BF94:
    /* 6C794 8007BF94 A20D020C */  jal        sys_GetVideoMode
    /* 6C798 8007BF98 00000000 */   nop
    /* 6C79C 8007BF9C 120002A2 */  sb         $v0, 0x12($s0)
    /* 6C7A0 8007BFA0 FF004230 */  andi       $v0, $v0, 0xFF
    /* 6C7A4 8007BFA4 01000324 */  addiu      $v1, $zero, 0x1
    /* 6C7A8 8007BFA8 02004314 */  bne        $v0, $v1, .L8007BFB4
    /* 6C7AC 8007BFAC 00000000 */   nop
    /* 6C7B0 8007BFB0 08007336 */  ori        $s3, $s3, (0x8000008 & 0xFFFF)
  .L8007BFB4:
    /* 6C7B4 8007BFB4 11000292 */  lbu        $v0, 0x11($s0)
    /* 6C7B8 8007BFB8 00000000 */  nop
    /* 6C7BC 8007BFBC 02004010 */  beqz       $v0, .L8007BFC8
    /* 6C7C0 8007BFC0 00000000 */   nop
    /* 6C7C4 8007BFC4 10007336 */  ori        $s3, $s3, (0x8000010 & 0xFFFF)
  .L8007BFC8:
    /* 6C7C8 8007BFC8 10000292 */  lbu        $v0, 0x10($s0)
    /* 6C7CC 8007BFCC 00000000 */  nop
    /* 6C7D0 8007BFD0 02004010 */  beqz       $v0, .L8007BFDC
    /* 6C7D4 8007BFD4 00000000 */   nop
    /* 6C7D8 8007BFD8 20007336 */  ori        $s3, $s3, (0x8000020 & 0xFFFF)
  .L8007BFDC:
    /* 6C7DC 8007BFDC 0A80023C */  lui        $v0, %hi(D_8009BE77)
    /* 6C7E0 8007BFE0 77BE4290 */  lbu        $v0, %lo(D_8009BE77)($v0)
    /* 6C7E4 8007BFE4 00000000 */  nop
    /* 6C7E8 8007BFE8 02004010 */  beqz       $v0, .L8007BFF4
    /* 6C7EC 8007BFEC 00000000 */   nop
    /* 6C7F0 8007BFF0 80007336 */  ori        $s3, $s3, (0x8000080 & 0xFFFF)
  .L8007BFF4:
    /* 6C7F4 8007BFF4 04000386 */  lh         $v1, 0x4($s0)
    /* 6C7F8 8007BFF8 00000000 */  nop
    /* 6C7FC 8007BFFC 19016228 */  slti       $v0, $v1, 0x119
    /* 6C800 8007C000 0E004014 */  bnez       $v0, .L8007C03C
    /* 6C804 8007C004 61016228 */   slti      $v0, $v1, 0x161
    /* 6C808 8007C008 03004010 */  beqz       $v0, .L8007C018
    /* 6C80C 8007C00C 91016228 */   slti      $v0, $v1, 0x191
    /* 6C810 8007C010 0FF00108 */  j          .L8007C03C
    /* 6C814 8007C014 01007336 */   ori       $s3, $s3, (0x8000001 & 0xFFFF)
  .L8007C018:
    /* 6C818 8007C018 03004010 */  beqz       $v0, .L8007C028
    /* 6C81C 8007C01C 31026228 */   slti      $v0, $v1, 0x231
    /* 6C820 8007C020 0FF00108 */  j          .L8007C03C
    /* 6C824 8007C024 40007336 */   ori       $s3, $s3, (0x8000040 & 0xFFFF)
  .L8007C028:
    /* 6C828 8007C028 03004010 */  beqz       $v0, .L8007C038
    /* 6C82C 8007C02C 00000000 */   nop
    /* 6C830 8007C030 0FF00108 */  j          .L8007C03C
    /* 6C834 8007C034 02007336 */   ori       $s3, $s3, (0x8000002 & 0xFFFF)
  .L8007C038:
    /* 6C838 8007C038 03007336 */  ori        $s3, $s3, (0x8000003 & 0xFFFF)
  .L8007C03C:
    /* 6C83C 8007C03C 12000292 */  lbu        $v0, 0x12($s0)
    /* 6C840 8007C040 06000386 */  lh         $v1, 0x6($s0)
    /* 6C844 8007C044 02004014 */  bnez       $v0, .L8007C050
    /* 6C848 8007C048 21016228 */   slti      $v0, $v1, 0x121
    /* 6C84C 8007C04C 01016228 */  slti       $v0, $v1, 0x101
  .L8007C050:
    /* 6C850 8007C050 02004014 */  bnez       $v0, .L8007C05C
    /* 6C854 8007C054 00000000 */   nop
    /* 6C858 8007C058 24007336 */  ori        $s3, $s3, (0x8000024 & 0xFFFF)
  .L8007C05C:
    /* 6C85C 8007C05C 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6C860 8007C060 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6C864 8007C064 00000000 */  nop
    /* 6C868 8007C068 1000428C */  lw         $v0, 0x10($v0)
    /* 6C86C 8007C06C 00000000 */  nop
    /* 6C870 8007C070 09F84000 */  jalr       $v0
    /* 6C874 8007C074 21206002 */   addu      $a0, $s3, $zero
  .L8007C078:
    /* 6C878 8007C078 0A80043C */  lui        $a0, %hi(D_8009BEE0)
    /* 6C87C 8007C07C E0BE8424 */  addiu      $a0, $a0, %lo(D_8009BEE0)
    /* 6C880 8007C080 21280002 */  addu       $a1, $s0, $zero
    /* 6C884 8007C084 48E4010C */  jal        bb2_memcpy
    /* 6C888 8007C088 14000624 */   addiu     $a2, $zero, 0x14
    /* 6C88C 8007C08C 21100002 */  addu       $v0, $s0, $zero
    /* 6C890 8007C090 2000BF8F */  lw         $ra, 0x20($sp)
    /* 6C894 8007C094 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 6C898 8007C098 1800B28F */  lw         $s2, 0x18($sp)
    /* 6C89C 8007C09C 1400B18F */  lw         $s1, 0x14($sp)
    /* 6C8A0 8007C0A0 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C8A4 8007C0A4 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 6C8A8 8007C0A8 0800E003 */  jr         $ra
    /* 6C8AC 8007C0AC 00000000 */   nop
endlabel func_8007BC08
