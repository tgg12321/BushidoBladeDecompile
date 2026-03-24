glabel func_8004C404
    /* 3CC04 8004C404 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3CC08 8004C408 3000BFAF */  sw         $ra, 0x30($sp)
    /* 3CC0C 8004C40C 2C00BEAF */  sw         $fp, 0x2C($sp)
    /* 3CC10 8004C410 2800B7AF */  sw         $s7, 0x28($sp)
    /* 3CC14 8004C414 2400B6AF */  sw         $s6, 0x24($sp)
    /* 3CC18 8004C418 2000B5AF */  sw         $s5, 0x20($sp)
    /* 3CC1C 8004C41C 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 3CC20 8004C420 1800B3AF */  sw         $s3, 0x18($sp)
    /* 3CC24 8004C424 1400B2AF */  sw         $s2, 0x14($sp)
    /* 3CC28 8004C428 1000B1AF */  sw         $s1, 0x10($sp)
    /* 3CC2C 8004C42C 21908000 */  addu       $s2, $a0, $zero
    /* 3CC30 8004C430 2188A000 */  addu       $s1, $a1, $zero
    /* 3CC34 8004C434 801F133C */  lui        $s3, (0x1F8001B0 >> 16)
    /* 3CC38 8004C438 0F80143C */  lui        $s4, %hi(D_800F6658)
    /* 3CC3C 8004C43C 58669486 */  lh         $s4, %lo(D_800F6658)($s4)
    /* 3CC40 8004C440 03000824 */  addiu      $t0, $zero, 0x3
    /* 3CC44 8004C444 22401401 */  sub        $t0, $t0, $s4 /* handwritten instruction */
    /* 3CC48 8004C448 C60168A6 */  sh         $t0, (0x1F8001C6 & 0xFFFF)($s3)
    /* 3CC4C 8004C44C 1080093C */  lui        $t1, %hi(D_80103624)
    /* 3CC50 8004C450 2436298D */  lw         $t1, %lo(D_80103624)($t1)
    /* 3CC54 8004C454 00000000 */  nop
    /* 3CC58 8004C458 C80169AE */  sw         $t1, (0x1F8001C8 & 0xFFFF)($s3)
    /* 3CC5C 8004C45C 0A80043C */  lui        $a0, %hi(D_800A3790)
    /* 3CC60 8004C460 9037848C */  lw         $a0, %lo(D_800A3790)($a0)
    /* 3CC64 8004C464 00000000 */  nop
    /* 3CC68 8004C468 C00164AE */  sw         $a0, (0x1F8001C0 & 0xFFFF)($s3)
    /* 3CC6C 8004C46C 0000C88C */  lw         $t0, 0x0($a2)
    /* 3CC70 8004C470 0400C98C */  lw         $t1, 0x4($a2)
    /* 3CC74 8004C474 0800CA8C */  lw         $t2, 0x8($a2)
    /* 3CC78 8004C478 0C00CB8C */  lw         $t3, 0xC($a2)
    /* 3CC7C 8004C47C 1000CC84 */  lh         $t4, 0x10($a2)
    /* 3CC80 8004C480 880168AE */  sw         $t0, (0x1F800188 & 0xFFFF)($s3)
    /* 3CC84 8004C484 8C0169AE */  sw         $t1, (0x1F80018C & 0xFFFF)($s3)
    /* 3CC88 8004C488 90016AAE */  sw         $t2, (0x1F800190 & 0xFFFF)($s3)
    /* 3CC8C 8004C48C 94016BAE */  sw         $t3, (0x1F800194 & 0xFFFF)($s3)
    /* 3CC90 8004C490 98016CA6 */  sh         $t4, (0x1F800198 & 0xFFFF)($s3)
    /* 3CC94 8004C494 10009422 */  addi       $s4, $s4, 0x10 /* handwritten instruction */
    /* 3CC98 8004C498 07408802 */  srav       $t0, $t0, $s4
    /* 3CC9C 8004C49C 07488902 */  srav       $t1, $t1, $s4
    /* 3CCA0 8004C4A0 07508A02 */  srav       $t2, $t2, $s4
    /* 3CCA4 8004C4A4 07588B02 */  srav       $t3, $t3, $s4
    /* 3CCA8 8004C4A8 F0FF9422 */  addi       $s4, $s4, -0x10 /* handwritten instruction */
    /* 3CCAC 8004C4AC 88016D86 */  lh         $t5, (0x1F800188 & 0xFFFF)($s3)
    /* 3CCB0 8004C4B0 8C016E86 */  lh         $t6, (0x1F80018C & 0xFFFF)($s3)
    /* 3CCB4 8004C4B4 90016F86 */  lh         $t7, (0x1F800190 & 0xFFFF)($s3)
    /* 3CCB8 8004C4B8 94017886 */  lh         $t8, (0x1F800194 & 0xFFFF)($s3)
    /* 3CCBC 8004C4BC 07608C02 */  srav       $t4, $t4, $s4
    /* 3CCC0 8004C4C0 07688D02 */  srav       $t5, $t5, $s4
    /* 3CCC4 8004C4C4 07708E02 */  srav       $t6, $t6, $s4
    /* 3CCC8 8004C4C8 07788F02 */  srav       $t7, $t7, $s4
    /* 3CCCC 8004C4CC 07C09802 */  srav       $t8, $t8, $s4
    /* 3CCD0 8004C4D0 43480900 */  sra        $t1, $t1, 1
    /* 3CCD4 8004C4D4 43780F00 */  sra        $t7, $t7, 1
    /* 3CCD8 8004C4D8 43500A00 */  sra        $t2, $t2, 1
    /* 3CCDC 8004C4DC 9C016DA6 */  sh         $t5, (0x1F80019C & 0xFFFF)($s3)
    /* 3CCE0 8004C4E0 9E0168A6 */  sh         $t0, (0x1F80019E & 0xFFFF)($s3)
    /* 3CCE4 8004C4E4 A0016EA6 */  sh         $t6, (0x1F8001A0 & 0xFFFF)($s3)
    /* 3CCE8 8004C4E8 A20169A6 */  sh         $t1, (0x1F8001A2 & 0xFFFF)($s3)
    /* 3CCEC 8004C4EC A4016FA6 */  sh         $t7, (0x1F8001A4 & 0xFFFF)($s3)
    /* 3CCF0 8004C4F0 A6016AA6 */  sh         $t2, (0x1F8001A6 & 0xFFFF)($s3)
    /* 3CCF4 8004C4F4 A80178A6 */  sh         $t8, (0x1F8001A8 & 0xFFFF)($s3)
    /* 3CCF8 8004C4F8 AA016BA6 */  sh         $t3, (0x1F8001AA & 0xFFFF)($s3)
    /* 3CCFC 8004C4FC AC016CA6 */  sh         $t4, (0x1F8001AC & 0xFFFF)($s3)
    /* 3CD00 8004C500 0A80083C */  lui        $t0, %hi(D_800A3708)
    /* 3CD04 8004C504 0837088D */  lw         $t0, %lo(D_800A3708)($t0)
    /* 3CD08 8004C508 00000000 */  nop
    /* 3CD0C 8004C50C 2C00098D */  lw         $t1, 0x2C($t0)
    /* 3CD10 8004C510 30000A8D */  lw         $t2, 0x30($t0)
    /* 3CD14 8004C514 34000B8D */  lw         $t3, 0x34($t0)
    /* 3CD18 8004C518 B40169AE */  sw         $t1, (0x1F8001B4 & 0xFFFF)($s3)
    /* 3CD1C 8004C51C B8016AAE */  sw         $t2, (0x1F8001B8 & 0xFFFF)($s3)
    /* 3CD20 8004C520 BC016BAE */  sw         $t3, (0x1F8001BC & 0xFFFF)($s3)
    /* 3CD24 8004C524 04000824 */  addiu      $t0, $zero, 0x4
    /* 3CD28 8004C528 0F800A3C */  lui        $t2, %hi(D_800F62E0)
    /* 3CD2C 8004C52C E0624A25 */  addiu      $t2, $t2, %lo(D_800F62E0)
    /* 3CD30 8004C530 40480800 */  sll        $t1, $t0, 1
    /* 3CD34 8004C534 20482801 */  add        $t1, $t1, $t0 /* handwritten instruction */
    /* 3CD38 8004C538 40490900 */  sll        $t1, $t1, 5
    /* 3CD3C 8004C53C 21402A01 */  addu       $t0, $t1, $t2
    /* 3CD40 8004C540 1800098D */  lw         $t1, 0x18($t0)
    /* 3CD44 8004C544 1C000A8D */  lw         $t2, 0x1C($t0)
    /* 3CD48 8004C548 20000B8D */  lw         $t3, 0x20($t0)
    /* 3CD4C 8004C54C 24000C8D */  lw         $t4, 0x24($t0)
    /* 3CD50 8004C550 28000D8D */  lw         $t5, 0x28($t0)
    /* 3CD54 8004C554 0040C948 */  ctc2       $t1, $8 /* handwritten instruction */
    /* 3CD58 8004C558 0048CA48 */  ctc2       $t2, $9 /* handwritten instruction */
    /* 3CD5C 8004C55C 0050CB48 */  ctc2       $t3, $10 /* handwritten instruction */
    /* 3CD60 8004C560 0058CC48 */  ctc2       $t4, $11 /* handwritten instruction */
    /* 3CD64 8004C564 0060CD48 */  ctc2       $t5, $12 /* handwritten instruction */
    /* 3CD68 8004C568 58000E8D */  lw         $t6, 0x58($t0)
    /* 3CD6C 8004C56C 3800098D */  lw         $t1, 0x38($t0)
    /* 3CD70 8004C570 3C000A8D */  lw         $t2, 0x3C($t0)
    /* 3CD74 8004C574 40000B8D */  lw         $t3, 0x40($t0)
    /* 3CD78 8004C578 44000C8D */  lw         $t4, 0x44($t0)
    /* 3CD7C 8004C57C 48000D8D */  lw         $t5, 0x48($t0)
    /* 3CD80 8004C580 0080C948 */  ctc2       $t1, $16 /* handwritten instruction */
    /* 3CD84 8004C584 0088CA48 */  ctc2       $t2, $17 /* handwritten instruction */
    /* 3CD88 8004C588 0090CB48 */  ctc2       $t3, $18 /* handwritten instruction */
    /* 3CD8C 8004C58C 0098CC48 */  ctc2       $t4, $19 /* handwritten instruction */
    /* 3CD90 8004C590 00A0CD48 */  ctc2       $t5, $20 /* handwritten instruction */
    /* 3CD94 8004C594 F00F0824 */  addiu      $t0, $zero, 0xFF0
    /* 3CD98 8004C598 00490E00 */  sll        $t1, $t6, 4
    /* 3CD9C 8004C59C 02510E00 */  srl        $t2, $t6, 4
    /* 3CDA0 8004C5A0 025B0E00 */  srl        $t3, $t6, 12
    /* 3CDA4 8004C5A4 24482801 */  and        $t1, $t1, $t0
    /* 3CDA8 8004C5A8 24504801 */  and        $t2, $t2, $t0
    /* 3CDAC 8004C5AC 24586801 */  and        $t3, $t3, $t0
    /* 3CDB0 8004C5B0 0068C948 */  ctc2       $t1, $13 /* handwritten instruction */
    /* 3CDB4 8004C5B4 0070CA48 */  ctc2       $t2, $14 /* handwritten instruction */
    /* 3CDB8 8004C5B8 0078CB48 */  ctc2       $t3, $15 /* handwritten instruction */
  .L8004C5BC:
    /* 3CDBC 8004C5BC B401688E */  lw         $t0, (0x1F8001B4 & 0xFFFF)($s3)
    /* 3CDC0 8004C5C0 B801698E */  lw         $t1, (0x1F8001B8 & 0xFFFF)($s3)
    /* 3CDC4 8004C5C4 BC016A8E */  lw         $t2, (0x1F8001BC & 0xFFFF)($s3)
    /* 3CDC8 8004C5C8 08002B8E */  lw         $t3, 0x8($s1)
    /* 3CDCC 8004C5CC 21600000 */  addu       $t4, $zero, $zero
    /* 3CDD0 8004C5D0 0C002D8E */  lw         $t5, 0xC($s1)
    /* 3CDD4 8004C5D4 22586801 */  sub        $t3, $t3, $t0 /* handwritten instruction */
    /* 3CDD8 8004C5D8 22608901 */  sub        $t4, $t4, $t1 /* handwritten instruction */
    /* 3CDDC 8004C5DC 2268AA01 */  sub        $t5, $t5, $t2 /* handwritten instruction */
    /* 3CDE0 8004C5E0 07588B02 */  srav       $t3, $t3, $s4
    /* 3CDE4 8004C5E4 07608C02 */  srav       $t4, $t4, $s4
    /* 3CDE8 8004C5E8 07688D02 */  srav       $t5, $t5, $s4
    /* 3CDEC 8004C5EC FFFF6B31 */  andi       $t3, $t3, 0xFFFF
    /* 3CDF0 8004C5F0 00640C00 */  sll        $t4, $t4, 16
    /* 3CDF4 8004C5F4 25586C01 */  or         $t3, $t3, $t4
    /* 3CDF8 8004C5F8 00008B48 */  mtc2       $t3, $0 /* handwritten instruction */
    /* 3CDFC 8004C5FC 00088D48 */  mtc2       $t5, $1 /* handwritten instruction */
    /* 3CE00 8004C600 8801688E */  lw         $t0, (0x1F800188 & 0xFFFF)($s3)
    /* 3CE04 8004C604 8C01698E */  lw         $t1, (0x1F80018C & 0xFFFF)($s3)
    /* 3CE08 8004C608 90016A8E */  lw         $t2, (0x1F800190 & 0xFFFF)($s3)
    /* 3CE0C 8004C60C 94016B8E */  lw         $t3, (0x1F800194 & 0xFFFF)($s3)
    /* 3CE10 8004C610 98016C8E */  lw         $t4, (0x1F800198 & 0xFFFF)($s3)
    /* 3CE14 8004C614 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3CE18 8004C618 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 3CE1C 8004C61C 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 3CE20 8004C620 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 3CE24 8004C624 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 3CE28 8004C628 0028C048 */  ctc2       $zero, $5 /* handwritten instruction */
    /* 3CE2C 8004C62C 0030C048 */  ctc2       $zero, $6 /* handwritten instruction */
    /* 3CE30 8004C630 0038C048 */  ctc2       $zero, $7 /* handwritten instruction */
    /* 3CE34 8004C634 00000000 */  nop
    /* 3CE38 8004C638 00000000 */  nop
    /* 3CE3C 8004C63C 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 3CE40 8004C640 00000000 */  nop
    /* 3CE44 8004C644 00000000 */  nop
    /* 3CE48 8004C648 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 3CE4C 8004C64C 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 3CE50 8004C650 00580A48 */  mfc2       $t2, $11 /* handwritten instruction */
    /* 3CE54 8004C654 43480900 */  sra        $t1, $t1, 1
    /* 3CE58 8004C658 0028C848 */  ctc2       $t0, $5 /* handwritten instruction */
    /* 3CE5C 8004C65C 0030C948 */  ctc2       $t1, $6 /* handwritten instruction */
    /* 3CE60 8004C660 0038CA48 */  ctc2       $t2, $7 /* handwritten instruction */
    /* 3CE64 8004C664 9C01688E */  lw         $t0, (0x1F80019C & 0xFFFF)($s3)
    /* 3CE68 8004C668 A001698E */  lw         $t1, (0x1F8001A0 & 0xFFFF)($s3)
    /* 3CE6C 8004C66C A4016A8E */  lw         $t2, (0x1F8001A4 & 0xFFFF)($s3)
    /* 3CE70 8004C670 A8016B8E */  lw         $t3, (0x1F8001A8 & 0xFFFF)($s3)
    /* 3CE74 8004C674 AC016C8E */  lw         $t4, (0x1F8001AC & 0xFFFF)($s3)
    /* 3CE78 8004C678 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3CE7C 8004C67C 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 3CE80 8004C680 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 3CE84 8004C684 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 3CE88 8004C688 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 3CE8C 8004C68C 02002886 */  lh         $t0, 0x2($s1)
    /* 3CE90 8004C690 04002986 */  lh         $t1, 0x4($s1)
    /* 3CE94 8004C694 80400800 */  sll        $t0, $t0, 2
    /* 3CE98 8004C698 10800A3C */  lui        $t2, %hi(D_80103608)
    /* 3CE9C 8004C69C 08364A25 */  addiu      $t2, $t2, %lo(D_80103608)
    /* 3CEA0 8004C6A0 21400A01 */  addu       $t0, $t0, $t2
    /* 3CEA4 8004C6A4 0000088D */  lw         $t0, 0x0($t0)
    /* 3CEA8 8004C6A8 80480900 */  sll        $t1, $t1, 2
    /* 3CEAC 8004C6AC 21400901 */  addu       $t0, $t0, $t1
    /* 3CEB0 8004C6B0 0000108D */  lw         $s0, 0x0($t0)
    /* 3CEB4 8004C6B4 06003786 */  lh         $s7, 0x6($s1)
    /* 3CEB8 8004C6B8 00000000 */  nop
    /* 3CEBC 8004C6BC 0000058E */  lw         $a1, 0x0($s0)
    /* 3CEC0 8004C6C0 00000000 */  nop
    /* 3CEC4 8004C6C4 FFFFA430 */  andi       $a0, $a1, 0xFFFF
    /* 3CEC8 8004C6C8 022C0500 */  srl        $a1, $a1, 16
    /* 3CECC 8004C6CC 98008010 */  beqz       $a0, .L8004C930
    /* 3CED0 8004C6D0 04001022 */   addi      $s0, $s0, 0x4 /* handwritten instruction */
    /* 3CED4 8004C6D4 B00170AE */  sw         $s0, (0x1F8001B0 & 0xFFFF)($s3)
    /* 3CED8 8004C6D8 04000824 */  addiu      $t0, $zero, 0x4
    /* 3CEDC 8004C6DC 32000415 */  bne        $t0, $a0, .L8004C7A8
    /* 3CEE0 8004C6E0 00000000 */   nop
    /* 3CEE4 8004C6E4 00000E96 */  lhu        $t6, 0x0($s0)
    /* 3CEE8 8004C6E8 02000F96 */  lhu        $t7, 0x2($s0)
    /* 3CEEC 8004C6EC 04001896 */  lhu        $t8, 0x4($s0)
    /* 3CEF0 8004C6F0 007C0F00 */  sll        $t7, $t7, 16
    /* 3CEF4 8004C6F4 2570CF01 */  or         $t6, $t6, $t7
    /* 3CEF8 8004C6F8 00008E48 */  mtc2       $t6, $0 /* handwritten instruction */
    /* 3CEFC 8004C6FC 00089848 */  mtc2       $t8, $1 /* handwritten instruction */
    /* 3CF00 8004C700 06000E96 */  lhu        $t6, 0x6($s0)
    /* 3CF04 8004C704 08000F96 */  lhu        $t7, 0x8($s0)
    /* 3CF08 8004C708 0A001896 */  lhu        $t8, 0xA($s0)
    /* 3CF0C 8004C70C 007C0F00 */  sll        $t7, $t7, 16
    /* 3CF10 8004C710 2570CF01 */  or         $t6, $t6, $t7
    /* 3CF14 8004C714 00108E48 */  mtc2       $t6, $2 /* handwritten instruction */
    /* 3CF18 8004C718 00189848 */  mtc2       $t8, $3 /* handwritten instruction */
    /* 3CF1C 8004C71C 0C000E96 */  lhu        $t6, 0xC($s0)
    /* 3CF20 8004C720 0E000F96 */  lhu        $t7, 0xE($s0)
    /* 3CF24 8004C724 10001896 */  lhu        $t8, 0x10($s0)
    /* 3CF28 8004C728 007C0F00 */  sll        $t7, $t7, 16
    /* 3CF2C 8004C72C 2570CF01 */  or         $t6, $t6, $t7
    /* 3CF30 8004C730 00208E48 */  mtc2       $t6, $4 /* handwritten instruction */
    /* 3CF34 8004C734 00289848 */  mtc2       $t8, $5 /* handwritten instruction */
    /* 3CF38 8004C738 12000E96 */  lhu        $t6, 0x12($s0)
    /* 3CF3C 8004C73C 14000F96 */  lhu        $t7, 0x14($s0)
    /* 3CF40 8004C740 3000284A */  rtpt
    /* 3CF44 8004C744 16001896 */  lhu        $t8, 0x16($s0)
    /* 3CF48 8004C748 007C0F00 */  sll        $t7, $t7, 16
    /* 3CF4C 8004C74C 2570CF01 */  or         $t6, $t6, $t7
    /* 3CF50 8004C750 18001022 */  addi       $s0, $s0, 0x18 /* handwritten instruction */
    /* 3CF54 8004C754 20007526 */  addiu      $s5, $s3, %lo(D_1F800020)
    /* 3CF58 8004C758 10017626 */  addiu      $s6, $s3, %lo(D_1F800110)
    /* 3CF5C 8004C75C 0000ACEA */  swc2       $12, 0x0($s5)
    /* 3CF60 8004C760 0400ADEA */  swc2       $13, 0x4($s5)
    /* 3CF64 8004C764 0800AEEA */  swc2       $14, 0x8($s5)
    /* 3CF68 8004C768 00880848 */  mfc2       $t0, $17 /* handwritten instruction */
    /* 3CF6C 8004C76C 00900948 */  mfc2       $t1, $18 /* handwritten instruction */
    /* 3CF70 8004C770 00980A48 */  mfc2       $t2, $19 /* handwritten instruction */
    /* 3CF74 8004C774 00008E48 */  mtc2       $t6, $0 /* handwritten instruction */
    /* 3CF78 8004C778 00089848 */  mtc2       $t8, $1 /* handwritten instruction */
    /* 3CF7C 8004C77C 00000000 */  nop
    /* 3CF80 8004C780 00000000 */  nop
    /* 3CF84 8004C784 0100184A */  rtps
    /* 3CF88 8004C788 0000C8A6 */  sh         $t0, 0x0($s6)
    /* 3CF8C 8004C78C 0200C9A6 */  sh         $t1, 0x2($s6)
    /* 3CF90 8004C790 0400CAA6 */  sh         $t2, 0x4($s6)
    /* 3CF94 8004C794 00980848 */  mfc2       $t0, $19 /* handwritten instruction */
    /* 3CF98 8004C798 0C00AEEA */  swc2       $14, 0xC($s5)
    /* 3CF9C 8004C79C 0600C8A6 */  sh         $t0, 0x6($s6)
    /* 3CFA0 8004C7A0 4B000104 */  bgez       $zero, .L8004C8D0
    /* 3CFA4 8004C7A4 00000000 */   nop
  .L8004C7A8:
    /* 3CFA8 8004C7A8 FDFF8424 */  addiu      $a0, $a0, -0x3
    /* 3CFAC 8004C7AC 00000E96 */  lhu        $t6, 0x0($s0)
    /* 3CFB0 8004C7B0 02000F96 */  lhu        $t7, 0x2($s0)
    /* 3CFB4 8004C7B4 04001896 */  lhu        $t8, 0x4($s0)
    /* 3CFB8 8004C7B8 007C0F00 */  sll        $t7, $t7, 16
    /* 3CFBC 8004C7BC 2570CF01 */  or         $t6, $t6, $t7
    /* 3CFC0 8004C7C0 00008E48 */  mtc2       $t6, $0 /* handwritten instruction */
    /* 3CFC4 8004C7C4 00089848 */  mtc2       $t8, $1 /* handwritten instruction */
    /* 3CFC8 8004C7C8 06000E96 */  lhu        $t6, 0x6($s0)
    /* 3CFCC 8004C7CC 08000F96 */  lhu        $t7, 0x8($s0)
    /* 3CFD0 8004C7D0 0A001896 */  lhu        $t8, 0xA($s0)
    /* 3CFD4 8004C7D4 007C0F00 */  sll        $t7, $t7, 16
    /* 3CFD8 8004C7D8 2570CF01 */  or         $t6, $t6, $t7
    /* 3CFDC 8004C7DC 00108E48 */  mtc2       $t6, $2 /* handwritten instruction */
    /* 3CFE0 8004C7E0 00189848 */  mtc2       $t8, $3 /* handwritten instruction */
    /* 3CFE4 8004C7E4 0C000E96 */  lhu        $t6, 0xC($s0)
    /* 3CFE8 8004C7E8 0E000F96 */  lhu        $t7, 0xE($s0)
    /* 3CFEC 8004C7EC 10001896 */  lhu        $t8, 0x10($s0)
    /* 3CFF0 8004C7F0 007C0F00 */  sll        $t7, $t7, 16
    /* 3CFF4 8004C7F4 2570CF01 */  or         $t6, $t6, $t7
    /* 3CFF8 8004C7F8 00208E48 */  mtc2       $t6, $4 /* handwritten instruction */
    /* 3CFFC 8004C7FC 00289848 */  mtc2       $t8, $5 /* handwritten instruction */
    /* 3D000 8004C800 20007526 */  addiu      $s5, $s3, %lo(D_1F800020)
    /* 3D004 8004C804 10017626 */  addiu      $s6, $s3, %lo(D_1F800110)
    /* 3D008 8004C808 3000284A */  rtpt
    /* 3D00C 8004C80C 0A320108 */  j          .L8004C828
    /* 3D010 8004C810 12001026 */   addiu     $s0, $s0, 0x12
  .L8004C814:
    /* 3D014 8004C814 3000284A */  rtpt
    /* 3D018 8004C818 0600D626 */  addiu      $s6, $s6, 0x6
    /* 3D01C 8004C81C FAFFC8A6 */  sh         $t0, -0x6($s6)
    /* 3D020 8004C820 FCFFC9A6 */  sh         $t1, -0x4($s6)
    /* 3D024 8004C824 FEFFCAA6 */  sh         $t2, -0x2($s6)
  .L8004C828:
    /* 3D028 8004C828 0000ACEA */  swc2       $12, 0x0($s5)
    /* 3D02C 8004C82C 0400ADEA */  swc2       $13, 0x4($s5)
    /* 3D030 8004C830 0800AEEA */  swc2       $14, 0x8($s5)
    /* 3D034 8004C834 00880848 */  mfc2       $t0, $17 /* handwritten instruction */
    /* 3D038 8004C838 00900948 */  mfc2       $t1, $18 /* handwritten instruction */
    /* 3D03C 8004C83C 00980A48 */  mfc2       $t2, $19 /* handwritten instruction */
    /* 3D040 8004C840 0C00B526 */  addiu      $s5, $s5, 0xC
    /* 3D044 8004C844 19008018 */  blez       $a0, .L8004C8AC
    /* 3D048 8004C848 00000000 */   nop
    /* 3D04C 8004C84C FDFF8424 */  addiu      $a0, $a0, -0x3
    /* 3D050 8004C850 00000E96 */  lhu        $t6, 0x0($s0)
    /* 3D054 8004C854 02000F96 */  lhu        $t7, 0x2($s0)
    /* 3D058 8004C858 04001896 */  lhu        $t8, 0x4($s0)
    /* 3D05C 8004C85C 007C0F00 */  sll        $t7, $t7, 16
    /* 3D060 8004C860 2570CF01 */  or         $t6, $t6, $t7
    /* 3D064 8004C864 00008E48 */  mtc2       $t6, $0 /* handwritten instruction */
    /* 3D068 8004C868 00089848 */  mtc2       $t8, $1 /* handwritten instruction */
    /* 3D06C 8004C86C 06000E96 */  lhu        $t6, 0x6($s0)
    /* 3D070 8004C870 08000F96 */  lhu        $t7, 0x8($s0)
    /* 3D074 8004C874 0A001896 */  lhu        $t8, 0xA($s0)
    /* 3D078 8004C878 007C0F00 */  sll        $t7, $t7, 16
    /* 3D07C 8004C87C 2570CF01 */  or         $t6, $t6, $t7
    /* 3D080 8004C880 00108E48 */  mtc2       $t6, $2 /* handwritten instruction */
    /* 3D084 8004C884 00189848 */  mtc2       $t8, $3 /* handwritten instruction */
    /* 3D088 8004C888 0C000E96 */  lhu        $t6, 0xC($s0)
    /* 3D08C 8004C88C 0E000F96 */  lhu        $t7, 0xE($s0)
    /* 3D090 8004C890 10001896 */  lhu        $t8, 0x10($s0)
    /* 3D094 8004C894 007C0F00 */  sll        $t7, $t7, 16
    /* 3D098 8004C898 2570CF01 */  or         $t6, $t6, $t7
    /* 3D09C 8004C89C 00208E48 */  mtc2       $t6, $4 /* handwritten instruction */
    /* 3D0A0 8004C8A0 00289848 */  mtc2       $t8, $5 /* handwritten instruction */
    /* 3D0A4 8004C8A4 05320108 */  j          .L8004C814
    /* 3D0A8 8004C8A8 12001026 */   addiu     $s0, $s0, 0x12
  .L8004C8AC:
    /* 3D0AC 8004C8AC 0000C8A6 */  sh         $t0, 0x0($s6)
    /* 3D0B0 8004C8B0 0200C9A6 */  sh         $t1, 0x2($s6)
    /* 3D0B4 8004C8B4 0400CAA6 */  sh         $t2, 0x4($s6)
    /* 3D0B8 8004C8B8 20208400 */  add        $a0, $a0, $a0 /* handwritten instruction */
    /* 3D0BC 8004C8BC 20408400 */  add        $t0, $a0, $a0 /* handwritten instruction */
    /* 3D0C0 8004C8C0 20200401 */  add        $a0, $t0, $a0 /* handwritten instruction */
    /* 3D0C4 8004C8C4 20800402 */  add        $s0, $s0, $a0 /* handwritten instruction */
    /* 3D0C8 8004C8C8 03000832 */  andi       $t0, $s0, 0x3
    /* 3D0CC 8004C8CC 20800802 */  add        $s0, $s0, $t0 /* handwritten instruction */
  .L8004C8D0:
    /* 3D0D0 8004C8D0 00000496 */  lhu        $a0, 0x0($s0)
    /* 3D0D4 8004C8D4 02001026 */  addiu      $s0, $s0, 0x2
  .L8004C8D8:
    /* 3D0D8 8004C8D8 00000896 */  lhu        $t0, 0x0($s0)
    /* 3D0DC 8004C8DC B001658E */  lw         $a1, (0x1F8001B0 & 0xFFFF)($s3)
    /* 3D0E0 8004C8E0 02001026 */  addiu      $s0, $s0, 0x2
    /* 3D0E4 8004C8E4 C2400800 */  srl        $t0, $t0, 3
    /* 3D0E8 8004C8E8 80400800 */  sll        $t0, $t0, 2
    /* 3D0EC 8004C8EC 0180013C */  lui        $at, %hi(D_80015470)
    /* 3D0F0 8004C8F0 21082800 */  addu       $at, $at, $t0
    /* 3D0F4 8004C8F4 7054288C */  lw         $t0, %lo(D_80015470)($at)
    /* 3D0F8 8004C8F8 21300002 */  addu       $a2, $s0, $zero
    /* 3D0FC 8004C8FC 09F80001 */  jalr       $t0
    /* 3D100 8004C900 2138E002 */   addu      $a3, $s7, $zero
    /* 3D104 8004C904 21804000 */  addu       $s0, $v0, $zero
    /* 3D108 8004C908 00000496 */  lhu        $a0, 0x0($s0)
    /* 3D10C 8004C90C 02001026 */  addiu      $s0, $s0, 0x2
    /* 3D110 8004C910 F1FF8014 */  bnez       $a0, .L8004C8D8
    /* 3D114 8004C914 00000000 */   nop
    /* 3D118 8004C918 00000896 */  lhu        $t0, 0x0($s0)
    /* 3D11C 8004C91C 02001026 */  addiu      $s0, $s0, 0x2
    /* 3D120 8004C920 03000011 */  beqz       $t0, .L8004C930
    /* 3D124 8004C924 00000000 */   nop
    /* 3D128 8004C928 044B0108 */  j          func_80052C10
    /* 3D12C 8004C92C 00000000 */   nop
  .L8004C930:
    /* 3D130 8004C930 0A80023C */  lui        $v0, %hi(D_800A3820)
    /* 3D134 8004C934 2038428C */  lw         $v0, %lo(D_800A3820)($v0)
    /* 3D138 8004C938 00000000 */  nop
    /* 3D13C 8004C93C 09005210 */  beq        $v0, $s2, .L8004C964
    /* 3D140 8004C940 00000000 */   nop
    /* 3D144 8004C944 0000518E */  lw         $s1, 0x0($s2)
    /* 3D148 8004C948 00000000 */  nop
    /* 3D14C 8004C94C 00002286 */  lh         $v0, 0x0($s1)
    /* 3D150 8004C950 00000000 */  nop
    /* 3D154 8004C954 0C000124 */  addiu      $at, $zero, 0xC
    /* 3D158 8004C958 18FF4110 */  beq        $v0, $at, .L8004C5BC
    /* 3D15C 8004C95C 04005222 */   addi      $s2, $s2, 0x4 /* handwritten instruction */
    /* 3D160 8004C960 FCFF5222 */  addi       $s2, $s2, -0x4 /* handwritten instruction */
  .L8004C964:
    /* 3D164 8004C964 21104002 */  addu       $v0, $s2, $zero
    /* 3D168 8004C968 3000BF8F */  lw         $ra, 0x30($sp)
    /* 3D16C 8004C96C 2C00BE8F */  lw         $fp, 0x2C($sp)
    /* 3D170 8004C970 2800B78F */  lw         $s7, 0x28($sp)
    /* 3D174 8004C974 2400B68F */  lw         $s6, 0x24($sp)
    /* 3D178 8004C978 2000B58F */  lw         $s5, 0x20($sp)
    /* 3D17C 8004C97C 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 3D180 8004C980 1800B38F */  lw         $s3, 0x18($sp)
    /* 3D184 8004C984 1400B28F */  lw         $s2, 0x14($sp)
    /* 3D188 8004C988 1000B18F */  lw         $s1, 0x10($sp)
    /* 3D18C 8004C98C 0800E003 */  jr         $ra
    /* 3D190 8004C990 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004C404
