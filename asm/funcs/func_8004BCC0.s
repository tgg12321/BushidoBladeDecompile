glabel func_8004BCC0
    /* 3C4C0 8004BCC0 801F083C */  lui        $t0, (0x1F8001D4 >> 16)
    /* 3C4C4 8004BCC4 EC011FAD */  sw         $ra, (0x1F8001EC & 0xFFFF)($t0)
    /* 3C4C8 8004BCC8 F0011EAD */  sw         $fp, (0x1F8001F0 & 0xFFFF)($t0)
    /* 3C4CC 8004BCCC F40117AD */  sw         $s7, (0x1F8001F4 & 0xFFFF)($t0)
    /* 3C4D0 8004BCD0 F80116AD */  sw         $s6, (0x1F8001F8 & 0xFFFF)($t0)
    /* 3C4D4 8004BCD4 FC0115AD */  sw         $s5, (0x1F8001FC & 0xFFFF)($t0)
    /* 3C4D8 8004BCD8 000214AD */  sw         $s4, (0x1F800200 & 0xFFFF)($t0)
    /* 3C4DC 8004BCDC 040213AD */  sw         $s3, (0x1F800204 & 0xFFFF)($t0)
    /* 3C4E0 8004BCE0 080212AD */  sw         $s2, (0x1F800208 & 0xFFFF)($t0)
    /* 3C4E4 8004BCE4 0C0211AD */  sw         $s1, (0x1F80020C & 0xFFFF)($t0)
    /* 3C4E8 8004BCE8 100210AD */  sw         $s0, (0x1F800210 & 0xFFFF)($t0)
    /* 3C4EC 8004BCEC 21B80001 */  addu       $s7, $t0, $zero
    /* 3C4F0 8004BCF0 E001E7A6 */  sh         $a3, (0x1F8001E0 & 0xFFFF)($s7)
    /* 3C4F4 8004BCF4 D801E5AE */  sw         $a1, (0x1F8001D8 & 0xFFFF)($s7)
    /* 3C4F8 8004BCF8 2180C000 */  addu       $s0, $a2, $zero
    /* 3C4FC 8004BCFC 0C00F28E */  lw         $s2, (0x1F80000C & 0xFFFF)($s7)
    /* 3C500 8004BD00 0A80143C */  lui        $s4, %hi(D_800A38B4)
    /* 3C504 8004BD04 B438948E */  lw         $s4, %lo(D_800A38B4)($s4)
    /* 3C508 8004BD08 2000F522 */  addi       $s5, $s7, (0x1F800020 & 0xFFFF) /* handwritten instruction */
    /* 3C50C 8004BD0C 1001F622 */  addi       $s6, $s7, (0x1F800110 & 0xFFFF) /* handwritten instruction */
    /* 3C510 8004BD10 1402F322 */  addi       $s3, $s7, (0x1F800214 & 0xFFFF) /* handwritten instruction */
    /* 3C514 8004BD14 FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3C518 8004BD18 FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3C51C 8004BD1C FFFF8420 */  addi       $a0, $a0, -0x1 /* handwritten instruction */
  .L8004BD20:
    /* 3C520 8004BD20 E101E982 */  lb         $t1, (0x1F8001E1 & 0xFFFF)($s7)
    /* 3C524 8004BD24 0000088E */  lw         $t0, 0x0($s0)
    /* 3C528 8004BD28 01002931 */  andi       $t1, $t1, 0x1
    /* 3C52C 8004BD2C 404E0900 */  sll        $t1, $t1, 25
    /* 3C530 8004BD30 25400901 */  or         $t0, $t0, $t1
    /* 3C534 8004BD34 00308848 */  mtc2       $t0, $6 /* handwritten instruction */
    /* 3C538 8004BD38 02440800 */  srl        $t0, $t0, 16
    /* 3C53C 8004BD3C C401E8A6 */  sh         $t0, (0x1F8001C4 & 0xFFFF)($s7)
    /* 3C540 8004BD40 10000396 */  lhu        $v1, 0x10($s0)
    /* 3C544 8004BD44 0E000296 */  lhu        $v0, 0xE($s0)
    /* 3C548 8004BD48 001C0300 */  sll        $v1, $v1, 16
    /* 3C54C 8004BD4C 25104300 */  or         $v0, $v0, $v1
    /* 3C550 8004BD50 D401E4AE */  sw         $a0, (0x1F8001D4 & 0xFFFF)($s7)
    /* 3C554 8004BD54 80600200 */  sll        $t4, $v0, 2
    /* 3C558 8004BD58 FC038C31 */  andi       $t4, $t4, 0x3FC
    /* 3C55C 8004BD5C 21409501 */  addu       $t0, $t4, $s5
    /* 3C560 8004BD60 0000088D */  lw         $t0, 0x0($t0)
    /* 3C564 8004BD64 82690200 */  srl        $t5, $v0, 6
    /* 3C568 8004BD68 FC03AD31 */  andi       $t5, $t5, 0x3FC
    /* 3C56C 8004BD6C 2148B501 */  addu       $t1, $t5, $s5
    /* 3C570 8004BD70 0000298D */  lw         $t1, 0x0($t1)
    /* 3C574 8004BD74 82730200 */  srl        $t6, $v0, 14
    /* 3C578 8004BD78 FC03CE31 */  andi       $t6, $t6, 0x3FC
    /* 3C57C 8004BD7C 2150D501 */  addu       $t2, $t6, $s5
    /* 3C580 8004BD80 00004A8D */  lw         $t2, 0x0($t2)
    /* 3C584 8004BD84 827D0200 */  srl        $t7, $v0, 22
    /* 3C588 8004BD88 FC03EF31 */  andi       $t7, $t7, 0x3FC
    /* 3C58C 8004BD8C 2158F501 */  addu       $t3, $t7, $s5
    /* 3C590 8004BD90 00006B8D */  lw         $t3, 0x0($t3)
    /* 3C594 8004BD94 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3C598 8004BD98 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3C59C 8004BD9C 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3C5A0 8004BDA0 24180901 */  and        $v1, $t0, $t1
    /* 3C5A4 8004BDA4 24186A00 */  and        $v1, $v1, $t2
    /* 3C5A8 8004BDA8 24186B00 */  and        $v1, $v1, $t3
    /* 3C5AC 8004BDAC 0600404B */  nclip
    /* 3C5B0 8004BDB0 FC006004 */  bltz       $v1, .L8004C1A4
    /* 3C5B4 8004BDB4 00000000 */   nop
    /* 3C5B8 8004BDB8 00806330 */  andi       $v1, $v1, 0x8000
    /* 3C5BC 8004BDBC 00C00248 */  mfc2       $v0, $24 /* handwritten instruction */
    /* 3C5C0 8004BDC0 F8006014 */  bnez       $v1, .L8004C1A4
    /* 3C5C4 8004BDC4 00000000 */   nop
    /* 3C5C8 8004BDC8 F6004104 */  bgez       $v0, .L8004C1A4
    /* 3C5CC 8004BDCC 22180200 */   neg       $v1, $v0 /* handwritten instruction */
    /* 3C5D0 8004BDD0 03240800 */  sra        $a0, $t0, 16
    /* 3C5D4 8004BDD4 10FF8420 */  addi       $a0, $a0, -0xF0 /* handwritten instruction */
    /* 3C5D8 8004BDD8 032C0900 */  sra        $a1, $t1, 16
    /* 3C5DC 8004BDDC 10FFA520 */  addi       $a1, $a1, -0xF0 /* handwritten instruction */
    /* 3C5E0 8004BDE0 25208500 */  or         $a0, $a0, $a1
    /* 3C5E4 8004BDE4 032C0A00 */  sra        $a1, $t2, 16
    /* 3C5E8 8004BDE8 10FFA520 */  addi       $a1, $a1, -0xF0 /* handwritten instruction */
    /* 3C5EC 8004BDEC 25208500 */  or         $a0, $a0, $a1
    /* 3C5F0 8004BDF0 032C0B00 */  sra        $a1, $t3, 16
    /* 3C5F4 8004BDF4 10FFA520 */  addi       $a1, $a1, -0xF0 /* handwritten instruction */
    /* 3C5F8 8004BDF8 25208500 */  or         $a0, $a0, $a1
    /* 3C5FC 8004BDFC 00808430 */  andi       $a0, $a0, 0x8000
    /* 3C600 8004BE00 E8008010 */  beqz       $a0, .L8004C1A4
    /* 3C604 8004BE04 00000000 */   nop
    /* 3C608 8004BE08 42600C00 */  srl        $t4, $t4, 1
    /* 3C60C 8004BE0C 20209601 */  add        $a0, $t4, $s6 /* handwritten instruction */
    /* 3C610 8004BE10 00008484 */  lh         $a0, 0x0($a0)
    /* 3C614 8004BE14 42680D00 */  srl        $t5, $t5, 1
    /* 3C618 8004BE18 2028B601 */  add        $a1, $t5, $s6 /* handwritten instruction */
    /* 3C61C 8004BE1C 0000A584 */  lh         $a1, 0x0($a1)
    /* 3C620 8004BE20 42700E00 */  srl        $t6, $t6, 1
    /* 3C624 8004BE24 2030D601 */  add        $a2, $t6, $s6 /* handwritten instruction */
    /* 3C628 8004BE28 0000C684 */  lh         $a2, 0x0($a2)
    /* 3C62C 8004BE2C 42780F00 */  srl        $t7, $t7, 1
    /* 3C630 8004BE30 2038F601 */  add        $a3, $t7, $s6 /* handwritten instruction */
    /* 3C634 8004BE34 0000E784 */  lh         $a3, 0x0($a3)
    /* 3C638 8004BE38 2A108500 */  slt        $v0, $a0, $a1
    /* 3C63C 8004BE3C 03004010 */  beqz       $v0, .L8004BE4C
    /* 3C640 8004BE40 2A108600 */   slt       $v0, $a0, $a2
    /* 3C644 8004BE44 2120A000 */  addu       $a0, $a1, $zero
    /* 3C648 8004BE48 2A108600 */  slt        $v0, $a0, $a2
  .L8004BE4C:
    /* 3C64C 8004BE4C 03004010 */  beqz       $v0, .L8004BE5C
    /* 3C650 8004BE50 2A108700 */   slt       $v0, $a0, $a3
    /* 3C654 8004BE54 2120C000 */  addu       $a0, $a2, $zero
    /* 3C658 8004BE58 2A108700 */  slt        $v0, $a0, $a3
  .L8004BE5C:
    /* 3C65C 8004BE5C 02004010 */  beqz       $v0, .L8004BE68
    /* 3C660 8004BE60 00000000 */   nop
    /* 3C664 8004BE64 2120E000 */  addu       $a0, $a3, $zero
  .L8004BE68:
    /* 3C668 8004BE68 21108000 */  addu       $v0, $a0, $zero
    /* 3C66C 8004BE6C 21384000 */  addu       $a3, $v0, $zero
    /* 3C670 8004BE70 02001892 */  lbu        $t8, 0x2($s0)
    /* 3C674 8004BE74 00000000 */  nop
    /* 3C678 8004BE78 01001833 */  andi       $t8, $t8, 0x1
    /* 3C67C 8004BE7C 02000013 */  beqz       $t8, .L8004BE88
    /* 3C680 8004BE80 00000000 */   nop
    /* 3C684 8004BE84 E8034220 */  addi       $v0, $v0, 0x3E8 /* handwritten instruction */
  .L8004BE88:
    /* 3C688 8004BE88 C601F886 */  lh         $t8, (0x1F8001C6 & 0xFFFF)($s7)
    /* 3C68C 8004BE8C 1000E58E */  lw         $a1, (0x1F800010 & 0xFFFF)($s7)
    /* 3C690 8004BE90 1800E68E */  lw         $a2, (0x1F800018 & 0xFFFF)($s7)
    /* 3C694 8004BE94 06100203 */  srlv       $v0, $v0, $t8
    /* 3C698 8004BE98 C2C20200 */  srl        $t8, $v0, 11
    /* 3C69C 8004BE9C FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3C6A0 8004BEA0 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3C6A4 8004BEA4 07100203 */  srav       $v0, $v0, $t8
    /* 3C6A8 8004BEA8 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3C6AC 8004BEAC 2A08A200 */  slt        $at, $a1, $v0
    /* 3C6B0 8004BEB0 BC002014 */  bnez       $at, .L8004C1A4
    /* 3C6B4 8004BEB4 00000000 */   nop
    /* 3C6B8 8004BEB8 2A084600 */  slt        $at, $v0, $a2
    /* 3C6BC 8004BEBC B9002014 */  bnez       $at, .L8004C1A4
    /* 3C6C0 8004BEC0 00000000 */   nop
    /* 3C6C4 8004BEC4 12000596 */  lhu        $a1, 0x12($s0)
    /* 3C6C8 8004BEC8 140000CA */  lwc2       $0, 0x14($s0)
    /* 3C6CC 8004BECC 00088548 */  mtc2       $a1, $1 /* handwritten instruction */
    /* 3C6D0 8004BED0 04000486 */  lh         $a0, 0x4($s0)
    /* 3C6D4 8004BED4 00000586 */  lh         $a1, 0x0($s0)
    /* 3C6D8 8004BED8 CC01E4A6 */  sh         $a0, (0x1F8001CC & 0xFFFF)($s7)
    /* 3C6DC 8004BEDC CE01E5A6 */  sh         $a1, (0x1F8001CE & 0xFFFF)($s7)
    /* 3C6E0 8004BEE0 1E04C84A */  ncs
    /* 3C6E4 8004BEE4 00046428 */  slti       $a0, $v1, 0x400
    /* 3C6E8 8004BEE8 00076328 */  slti       $v1, $v1, 0x700
    /* 3C6EC 8004BEEC 20188300 */  add        $v1, $a0, $v1 /* handwritten instruction */
    /* 3C6F0 8004BEF0 C001E58E */  lw         $a1, (0x1F8001C0 & 0xFFFF)($s7)
    /* 3C6F4 8004BEF4 00000000 */  nop
    /* 3C6F8 8004BEF8 0300A530 */  andi       $a1, $a1, 0x3
    /* 3C6FC 8004BEFC 1100A010 */  beqz       $a1, .L8004BF44
    /* 3C700 8004BF00 21200000 */   addu      $a0, $zero, $zero
    /* 3C704 8004BF04 FEFFA620 */  addi       $a2, $a1, -0x2 /* handwritten instruction */
    /* 3C708 8004BF08 0F00C010 */  beqz       $a2, .L8004BF48
    /* 3C70C 8004BF0C 00000000 */   nop
    /* 3C710 8004BF10 E001E482 */  lb         $a0, (0x1F8001E0 & 0xFFFF)($s7)
    /* 3C714 8004BF14 00000000 */  nop
    /* 3C718 8004BF18 FEFF8420 */  addi       $a0, $a0, -0x2 /* handwritten instruction */
    /* 3C71C 8004BF1C 22200400 */  neg        $a0, $a0 /* handwritten instruction */
    /* 3C720 8004BF20 FFFFA620 */  addi       $a2, $a1, -0x1 /* handwritten instruction */
    /* 3C724 8004BF24 0700C010 */  beqz       $a2, .L8004BF44
    /* 3C728 8004BF28 00000000 */   nop
    /* 3C72C 8004BF2C E001E482 */  lb         $a0, (0x1F8001E0 & 0xFFFF)($s7)
    /* 3C730 8004BF30 00000000 */  nop
    /* 3C734 8004BF34 2A288300 */  slt        $a1, $a0, $v1
    /* 3C738 8004BF38 22280500 */  neg        $a1, $a1 /* handwritten instruction */
    /* 3C73C 8004BF3C 22206400 */  sub        $a0, $v1, $a0 /* handwritten instruction */
    /* 3C740 8004BF40 24208500 */  and        $a0, $a0, $a1
  .L8004BF44:
    /* 3C744 8004BF44 21188000 */  addu       $v1, $a0, $zero
  .L8004BF48:
    /* 3C748 8004BF48 06000496 */  lhu        $a0, 0x6($s0)
    /* 3C74C 8004BF4C 08000596 */  lhu        $a1, 0x8($s0)
    /* 3C750 8004BF50 0A000696 */  lhu        $a2, 0xA($s0)
    /* 3C754 8004BF54 0C000796 */  lhu        $a3, 0xC($s0)
    /* 3C758 8004BF58 0C006010 */  beqz       $v1, .L8004BF8C
    /* 3C75C 8004BF5C FF001824 */   addiu     $t8, $zero, 0xFF
    /* 3C760 8004BF60 06C07800 */  srlv       $t8, $t8, $v1
    /* 3C764 8004BF64 00CA1800 */  sll        $t9, $t8, 8
    /* 3C768 8004BF68 25C01903 */  or         $t8, $t8, $t9
    /* 3C76C 8004BF6C 06206400 */  srlv       $a0, $a0, $v1
    /* 3C770 8004BF70 24209800 */  and        $a0, $a0, $t8
    /* 3C774 8004BF74 06286500 */  srlv       $a1, $a1, $v1
    /* 3C778 8004BF78 2428B800 */  and        $a1, $a1, $t8
    /* 3C77C 8004BF7C 06306600 */  srlv       $a2, $a2, $v1
    /* 3C780 8004BF80 2430D800 */  and        $a2, $a2, $t8
    /* 3C784 8004BF84 06386700 */  srlv       $a3, $a3, $v1
    /* 3C788 8004BF88 2438F800 */  and        $a3, $a3, $t8
  .L8004BF8C:
    /* 3C78C 8004BF8C E201E4A6 */  sh         $a0, (0x1F8001E2 & 0xFFFF)($s7)
    /* 3C790 8004BF90 E401E5A6 */  sh         $a1, (0x1F8001E4 & 0xFFFF)($s7)
    /* 3C794 8004BF94 E601E6A6 */  sh         $a2, (0x1F8001E6 & 0xFFFF)($s7)
    /* 3C798 8004BF98 E801E7A6 */  sh         $a3, (0x1F8001E8 & 0xFFFF)($s7)
    /* 3C79C 8004BF9C 02001892 */  lbu        $t8, 0x2($s0)
    /* 3C7A0 8004BFA0 E001E782 */  lb         $a3, (0x1F8001E0 & 0xFFFF)($s7)
    /* 3C7A4 8004BFA4 42C01800 */  srl        $t8, $t8, 1
    /* 3C7A8 8004BFA8 03001833 */  andi       $t8, $t8, 0x3
    /* 3C7AC 8004BFAC 04000013 */  beqz       $t8, .L8004BFC0
    /* 3C7B0 8004BFB0 FFFF1823 */   addi      $t8, $t8, -0x1 /* handwritten instruction */
    /* 3C7B4 8004BFB4 2238F800 */  sub        $a3, $a3, $t8 /* handwritten instruction */
    /* 3C7B8 8004BFB8 2300E01C */  bgtz       $a3, .L8004C048
    /* 3C7BC 8004BFBC 00000000 */   nop
  .L8004BFC0:
    /* 3C7C0 8004BFC0 CC01EC86 */  lh         $t4, (0x1F8001CC & 0xFFFF)($s7)
    /* 3C7C4 8004BFC4 CE01ED86 */  lh         $t5, (0x1F8001CE & 0xFFFF)($s7)
    /* 3C7C8 8004BFC8 080088AE */  sw         $t0, 0x8($s4)
    /* 3C7CC 8004BFCC 100089AE */  sw         $t1, 0x10($s4)
    /* 3C7D0 8004BFD0 18008AAE */  sw         $t2, 0x18($s4)
    /* 3C7D4 8004BFD4 20008BAE */  sw         $t3, 0x20($s4)
    /* 3C7D8 8004BFD8 00640C00 */  sll        $t4, $t4, 16
    /* 3C7DC 8004BFDC 006C0D00 */  sll        $t5, $t5, 16
    /* 3C7E0 8004BFE0 E201E896 */  lhu        $t0, (0x1F8001E2 & 0xFFFF)($s7)
    /* 3C7E4 8004BFE4 E401E996 */  lhu        $t1, (0x1F8001E4 & 0xFFFF)($s7)
    /* 3C7E8 8004BFE8 E601EA96 */  lhu        $t2, (0x1F8001E6 & 0xFFFF)($s7)
    /* 3C7EC 8004BFEC E801EB96 */  lhu        $t3, (0x1F8001E8 & 0xFFFF)($s7)
    /* 3C7F0 8004BFF0 25400C01 */  or         $t0, $t0, $t4
    /* 3C7F4 8004BFF4 25482D01 */  or         $t1, $t1, $t5
    /* 3C7F8 8004BFF8 0C0088AE */  sw         $t0, 0xC($s4)
    /* 3C7FC 8004BFFC 140089AE */  sw         $t1, 0x14($s4)
    /* 3C800 8004C000 1C008AA6 */  sh         $t2, 0x1C($s4)
    /* 3C804 8004C004 24008BA6 */  sh         $t3, 0x24($s4)
    /* 3C808 8004C008 040096EA */  swc2       $22, 0x4($s4)
    /* 3C80C 8004C00C 80100200 */  sll        $v0, $v0, 2
    /* 3C810 8004C010 21105200 */  addu       $v0, $v0, $s2
    /* 3C814 8004C014 0000498C */  lw         $t1, 0x0($v0)
    /* 3C818 8004C018 24409E02 */  and        $t0, $s4, $fp
    /* 3C81C 8004C01C 000048AC */  sw         $t0, 0x0($v0)
    /* 3C820 8004C020 00090A3C */  lui        $t2, (0x9000000 >> 16)
    /* 3C824 8004C024 25482A01 */  or         $t1, $t1, $t2
    /* 3C828 8004C028 000089AE */  sw         $t1, 0x0($s4)
    /* 3C82C 8004C02C D401E48E */  lw         $a0, (0x1F8001D4 & 0xFFFF)($s7)
    /* 3C830 8004C030 18001022 */  addi       $s0, $s0, 0x18 /* handwritten instruction */
    /* 3C834 8004C034 28009422 */  addi       $s4, $s4, 0x28 /* handwritten instruction */
    /* 3C838 8004C038 39FF8014 */  bnez       $a0, .L8004BD20
    /* 3C83C 8004C03C FFFF8420 */   addi      $a0, $a0, -0x1 /* handwritten instruction */
    /* 3C840 8004C040 6D300108 */  j          .L8004C1B4
    /* 3C844 8004C044 00000000 */   nop
  .L8004C048:
    /* 3C848 8004C048 C001E48E */  lw         $a0, (0x1F8001C0 & 0xFFFF)($s7)
    /* 3C84C 8004C04C 00000000 */  nop
    /* 3C850 8004C050 04008430 */  andi       $a0, $a0, 0x4
    /* 3C854 8004C054 14008010 */  beqz       $a0, .L8004C0A8
    /* 3C858 8004C058 00000000 */   nop
    /* 3C85C 8004C05C 2120E000 */  addu       $a0, $a3, $zero
    /* 3C860 8004C060 0000058E */  lw         $a1, 0x0($s0)
    /* 3C864 8004C064 00000000 */  nop
    /* 3C868 8004C068 FF00013C */  lui        $at, (0xFFFFFF >> 16)
    /* 3C86C 8004C06C FFFF2134 */  ori        $at, $at, (0xFFFFFF & 0xFFFF)
    /* 3C870 8004C070 2528A100 */  or         $a1, $a1, $at
    /* 3C874 8004C074 09008010 */  beqz       $a0, .L8004C09C
    /* 3C878 8004C078 00000000 */   nop
    /* 3C87C 8004C07C 00FF0124 */  addiu      $at, $zero, -0x100
    /* 3C880 8004C080 2428A100 */  and        $a1, $a1, $at
    /* 3C884 8004C084 FFFF8420 */  addi       $a0, $a0, -0x1 /* handwritten instruction */
    /* 3C888 8004C088 04008010 */  beqz       $a0, .L8004C09C
    /* 3C88C 8004C08C 00000000 */   nop
    /* 3C890 8004C090 FFFF013C */  lui        $at, (0xFFFF4000 >> 16)
    /* 3C894 8004C094 00402134 */  ori        $at, $at, (0xFFFF4000 & 0xFFFF)
    /* 3C898 8004C098 2428A100 */  and        $a1, $a1, $at
  .L8004C09C:
    /* 3C89C 8004C09C 00B08548 */  mtc2       $a1, $22 /* handwritten instruction */
    /* 3C8A0 8004C0A0 00000000 */  nop
    /* 3C8A4 8004C0A4 00000000 */  nop
  .L8004C0A8:
    /* 3C8A8 8004C0A8 80100200 */  sll        $v0, $v0, 2
    /* 3C8AC 8004C0AC 21105200 */  addu       $v0, $v0, $s2
    /* 3C8B0 8004C0B0 0000518C */  lw         $s1, 0x0($v0)
    /* 3C8B4 8004C0B4 DC01E2AE */  sw         $v0, (0x1F8001DC & 0xFFFF)($s7)
    /* 3C8B8 8004C0B8 0009083C */  lui        $t0, (0x9000000 >> 16)
    /* 3C8BC 8004C0BC 25882802 */  or         $s1, $s1, $t0
    /* 3C8C0 8004C0C0 D801E58E */  lw         $a1, (0x1F8001D8 & 0xFFFF)($s7)
    /* 3C8C4 8004C0C4 D001F6EA */  swc2       $22, (0x1F8001D0 & 0xFFFF)($s7)
    /* 3C8C8 8004C0C8 20408C01 */  add        $t0, $t4, $t4 /* handwritten instruction */
    /* 3C8CC 8004C0CC 20608801 */  add        $t4, $t4, $t0 /* handwritten instruction */
    /* 3C8D0 8004C0D0 20608501 */  add        $t4, $t4, $a1 /* handwritten instruction */
    /* 3C8D4 8004C0D4 00008885 */  lh         $t0, 0x0($t4)
    /* 3C8D8 8004C0D8 02008985 */  lh         $t1, 0x2($t4)
    /* 3C8DC 8004C0DC 04008A85 */  lh         $t2, 0x4($t4)
    /* 3C8E0 8004C0E0 E201EB86 */  lh         $t3, (0x1F8001E2 & 0xFFFF)($s7)
    /* 3C8E4 8004C0E4 080068A6 */  sh         $t0, 0x8($s3)
    /* 3C8E8 8004C0E8 0A0069A6 */  sh         $t1, 0xA($s3)
    /* 3C8EC 8004C0EC 0C006AA6 */  sh         $t2, 0xC($s3)
    /* 3C8F0 8004C0F0 0E006BA6 */  sh         $t3, 0xE($s3)
    /* 3C8F4 8004C0F4 2040AD01 */  add        $t0, $t5, $t5 /* handwritten instruction */
    /* 3C8F8 8004C0F8 2068A801 */  add        $t5, $t5, $t0 /* handwritten instruction */
    /* 3C8FC 8004C0FC 2068A501 */  add        $t5, $t5, $a1 /* handwritten instruction */
    /* 3C900 8004C100 0000A885 */  lh         $t0, 0x0($t5)
    /* 3C904 8004C104 0200A985 */  lh         $t1, 0x2($t5)
    /* 3C908 8004C108 0400AA85 */  lh         $t2, 0x4($t5)
    /* 3C90C 8004C10C E401EB86 */  lh         $t3, (0x1F8001E4 & 0xFFFF)($s7)
    /* 3C910 8004C110 100068A6 */  sh         $t0, 0x10($s3)
    /* 3C914 8004C114 120069A6 */  sh         $t1, 0x12($s3)
    /* 3C918 8004C118 14006AA6 */  sh         $t2, 0x14($s3)
    /* 3C91C 8004C11C 16006BA6 */  sh         $t3, 0x16($s3)
    /* 3C920 8004C120 2040CE01 */  add        $t0, $t6, $t6 /* handwritten instruction */
    /* 3C924 8004C124 2070C801 */  add        $t6, $t6, $t0 /* handwritten instruction */
    /* 3C928 8004C128 2070C501 */  add        $t6, $t6, $a1 /* handwritten instruction */
    /* 3C92C 8004C12C 0000C885 */  lh         $t0, 0x0($t6)
    /* 3C930 8004C130 0200C985 */  lh         $t1, 0x2($t6)
    /* 3C934 8004C134 0400CA85 */  lh         $t2, 0x4($t6)
    /* 3C938 8004C138 E601EB86 */  lh         $t3, (0x1F8001E6 & 0xFFFF)($s7)
    /* 3C93C 8004C13C 180068A6 */  sh         $t0, 0x18($s3)
    /* 3C940 8004C140 1A0069A6 */  sh         $t1, 0x1A($s3)
    /* 3C944 8004C144 1C006AA6 */  sh         $t2, 0x1C($s3)
    /* 3C948 8004C148 1E006BA6 */  sh         $t3, 0x1E($s3)
    /* 3C94C 8004C14C 2040EF01 */  add        $t0, $t7, $t7 /* handwritten instruction */
    /* 3C950 8004C150 2078E801 */  add        $t7, $t7, $t0 /* handwritten instruction */
    /* 3C954 8004C154 2078E501 */  add        $t7, $t7, $a1 /* handwritten instruction */
    /* 3C958 8004C158 0000E885 */  lh         $t0, 0x0($t7)
    /* 3C95C 8004C15C 0200E985 */  lh         $t1, 0x2($t7)
    /* 3C960 8004C160 0400EA85 */  lh         $t2, 0x4($t7)
    /* 3C964 8004C164 E801EB86 */  lh         $t3, (0x1F8001E8 & 0xFFFF)($s7)
    /* 3C968 8004C168 200068A6 */  sh         $t0, 0x20($s3)
    /* 3C96C 8004C16C 220069A6 */  sh         $t1, 0x22($s3)
    /* 3C970 8004C170 24006AA6 */  sh         $t2, 0x24($s3)
    /* 3C974 8004C174 26006BA6 */  sh         $t3, 0x26($s3)
    /* 3C978 8004C178 7D30010C */  jal        func_8004C1F4
    /* 3C97C 8004C17C 2120E000 */   addu      $a0, $a3, $zero
    /* 3C980 8004C180 DC01E88E */  lw         $t0, (0x1F8001DC & 0xFFFF)($s7)
    /* 3C984 8004C184 24483E02 */  and        $t1, $s1, $fp
    /* 3C988 8004C188 000009AD */  sw         $t1, 0x0($t0)
    /* 3C98C 8004C18C D401E48E */  lw         $a0, (0x1F8001D4 & 0xFFFF)($s7)
    /* 3C990 8004C190 18001022 */  addi       $s0, $s0, 0x18 /* handwritten instruction */
    /* 3C994 8004C194 E2FE8014 */  bnez       $a0, .L8004BD20
    /* 3C998 8004C198 FFFF8420 */   addi      $a0, $a0, -0x1 /* handwritten instruction */
    /* 3C99C 8004C19C 6D300108 */  j          .L8004C1B4
    /* 3C9A0 8004C1A0 00000000 */   nop
  .L8004C1A4:
    /* 3C9A4 8004C1A4 D401E48E */  lw         $a0, (0x1F8001D4 & 0xFFFF)($s7)
    /* 3C9A8 8004C1A8 18001022 */  addi       $s0, $s0, 0x18 /* handwritten instruction */
    /* 3C9AC 8004C1AC DCFE8014 */  bnez       $a0, .L8004BD20
    /* 3C9B0 8004C1B0 FFFF8420 */   addi      $a0, $a0, -0x1 /* handwritten instruction */
  .L8004C1B4:
    /* 3C9B4 8004C1B4 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 3C9B8 8004C1B8 B43834AC */  sw         $s4, %lo(D_800A38B4)($at)
    /* 3C9BC 8004C1BC 21100002 */  addu       $v0, $s0, $zero
    /* 3C9C0 8004C1C0 2140E002 */  addu       $t0, $s7, $zero
    /* 3C9C4 8004C1C4 EC011F8D */  lw         $ra, (0x1F8001EC & 0xFFFF)($t0)
    /* 3C9C8 8004C1C8 F0011E8D */  lw         $fp, (0x1F8001F0 & 0xFFFF)($t0)
    /* 3C9CC 8004C1CC F401178D */  lw         $s7, (0x1F8001F4 & 0xFFFF)($t0)
    /* 3C9D0 8004C1D0 F801168D */  lw         $s6, (0x1F8001F8 & 0xFFFF)($t0)
    /* 3C9D4 8004C1D4 FC01158D */  lw         $s5, (0x1F8001FC & 0xFFFF)($t0)
    /* 3C9D8 8004C1D8 0002148D */  lw         $s4, (0x1F800200 & 0xFFFF)($t0)
    /* 3C9DC 8004C1DC 0402138D */  lw         $s3, (0x1F800204 & 0xFFFF)($t0)
    /* 3C9E0 8004C1E0 0802128D */  lw         $s2, (0x1F800208 & 0xFFFF)($t0)
    /* 3C9E4 8004C1E4 0C02118D */  lw         $s1, (0x1F80020C & 0xFFFF)($t0)
    /* 3C9E8 8004C1E8 1002108D */  lw         $s0, (0x1F800210 & 0xFFFF)($t0)
    /* 3C9EC 8004C1EC 0800E003 */  jr         $ra
    /* 3C9F0 8004C1F0 00000000 */   nop
endlabel func_8004BCC0
