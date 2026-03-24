glabel func_8008C464
    /* 7CC64 8008C464 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 7CC68 8008C468 1000B0AF */  sw         $s0, 0x10($sp)
    /* 7CC6C 8008C46C 21800000 */  addu       $s0, $zero, $zero
    /* 7CC70 8008C470 0600822C */  sltiu      $v0, $a0, 0x6
    /* 7CC74 8008C474 1800BFAF */  sw         $ra, 0x18($sp)
    /* 7CC78 8008C478 38014010 */  beqz       $v0, .L8008C95C
    /* 7CC7C 8008C47C 1400B1AF */   sw        $s1, 0x14($sp)
    /* 7CC80 8008C480 80100400 */  sll        $v0, $a0, 2
    /* 7CC84 8008C484 0180013C */  lui        $at, %hi(func_800164AC)
    /* 7CC88 8008C488 21082200 */  addu       $at, $at, $v0
    /* 7CC8C 8008C48C AC64228C */  lw         $v0, %lo(func_800164AC)($at)
    /* 7CC90 8008C490 00000000 */  nop
    /* 7CC94 8008C494 08004000 */  jr         $v0
    /* 7CC98 8008C498 00000000 */   nop
    /* 7CC9C 8008C49C 0700A22C */  sltiu      $v0, $a1, 0x7
    /* 7CCA0 8008C4A0 2E014010 */  beqz       $v0, .L8008C95C
    /* 7CCA4 8008C4A4 80100500 */   sll       $v0, $a1, 2
    /* 7CCA8 8008C4A8 0180013C */  lui        $at, %hi(func_800164AC + 0x18)
    /* 7CCAC 8008C4AC 21082200 */  addu       $at, $at, $v0
    /* 7CCB0 8008C4B0 C464228C */  lw         $v0, %lo(func_800164AC + 0x18)($at)
    /* 7CCB4 8008C4B4 00000000 */  nop
    /* 7CCB8 8008C4B8 08004000 */  jr         $v0
    /* 7CCBC 8008C4BC 00000000 */   nop
    /* 7CCC0 8008C4C0 0A80023C */  lui        $v0, %hi(D_800A3044)
    /* 7CCC4 8008C4C4 4430428C */  lw         $v0, %lo(D_800A3044)($v0)
    /* 7CCC8 8008C4C8 00000000 */  nop
    /* 7CCCC 8008C4CC 04004294 */  lhu        $v0, 0x4($v0)
    /* 7CCD0 8008C4D0 57320208 */  j          .L8008C95C
    /* 7CCD4 8008C4D4 FFFF5030 */   andi      $s0, $v0, 0xFFFF
    /* 7CCD8 8008C4D8 0A80023C */  lui        $v0, %hi(D_800A3044)
    /* 7CCDC 8008C4DC 4430428C */  lw         $v0, %lo(D_800A3044)($v0)
    /* 7CCE0 8008C4E0 00000000 */  nop
    /* 7CCE4 8008C4E4 0A004394 */  lhu        $v1, 0xA($v0)
    /* 7CCE8 8008C4E8 0A004294 */  lhu        $v0, 0xA($v0)
    /* 7CCEC 8008C4EC 42180300 */  srl        $v1, $v1, 1
    /* 7CCF0 8008C4F0 20004230 */  andi       $v0, $v0, 0x20
    /* 7CCF4 8008C4F4 19014010 */  beqz       $v0, .L8008C95C
    /* 7CCF8 8008C4F8 01007030 */   andi      $s0, $v1, 0x1
    /* 7CCFC 8008C4FC 57320208 */  j          .L8008C95C
    /* 7CD00 8008C500 02001036 */   ori       $s0, $s0, 0x2
    /* 7CD04 8008C504 0F80023C */  lui        $v0, %hi(D_800F1AE0)
    /* 7CD08 8008C508 E01A4224 */  addiu      $v0, $v0, %lo(D_800F1AE0)
    /* 7CD0C 8008C50C 00004294 */  lhu        $v0, 0x0($v0)
    /* 7CD10 8008C510 57320208 */  j          .L8008C95C
    /* 7CD14 8008C514 FFFF5030 */   andi      $s0, $v0, 0xFFFF
    /* 7CD18 8008C518 1F00033C */  lui        $v1, (0x1FA400 >> 16)
    /* 7CD1C 8008C51C 0F80023C */  lui        $v0, %hi(D_800F1AE6)
    /* 7CD20 8008C520 E61A4224 */  addiu      $v0, $v0, %lo(D_800F1AE6)
    /* 7CD24 8008C524 00004294 */  lhu        $v0, 0x0($v0)
    /* 7CD28 8008C528 00A46334 */  ori        $v1, $v1, (0x1FA400 & 0xFFFF)
    /* 7CD2C 8008C52C 1A006200 */  div        $zero, $v1, $v0
    /* 7CD30 8008C530 02004014 */  bnez       $v0, .L8008C53C
    /* 7CD34 8008C534 00000000 */   nop
    /* 7CD38 8008C538 0D000700 */  break      7
  .L8008C53C:
    /* 7CD3C 8008C53C FFFF0124 */  addiu      $at, $zero, -0x1
    /* 7CD40 8008C540 04004114 */  bne        $v0, $at, .L8008C554
    /* 7CD44 8008C544 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 7CD48 8008C548 02006114 */  bne        $v1, $at, .L8008C554
    /* 7CD4C 8008C54C 00000000 */   nop
    /* 7CD50 8008C550 0D000600 */  break      6
  .L8008C554:
    /* 7CD54 8008C554 12800000 */  mflo       $s0
    /* 7CD58 8008C558 58320208 */  j          .L8008C960
    /* 7CD5C 8008C55C 21100002 */   addu      $v0, $s0, $zero
    /* 7CD60 8008C560 0600C014 */  bnez       $a2, .L8008C57C
    /* 7CD64 8008C564 00000000 */   nop
    /* 7CD68 8008C568 0F80023C */  lui        $v0, %hi(D_800F1AF4)
    /* 7CD6C 8008C56C F41A4224 */  addiu      $v0, $v0, %lo(D_800F1AF4)
    /* 7CD70 8008C570 0000508C */  lw         $s0, 0x0($v0)
    /* 7CD74 8008C574 58320208 */  j          .L8008C960
    /* 7CD78 8008C578 21100002 */   addu      $v0, $s0, $zero
  .L8008C57C:
    /* 7CD7C 8008C57C 0F80023C */  lui        $v0, %hi(D_800F1B04)
    /* 7CD80 8008C580 041B4224 */  addiu      $v0, $v0, %lo(D_800F1B04)
    /* 7CD84 8008C584 0000508C */  lw         $s0, 0x0($v0)
    /* 7CD88 8008C588 58320208 */  j          .L8008C960
    /* 7CD8C 8008C58C 21100002 */   addu      $v0, $s0, $zero
    /* 7CD90 8008C590 0F80023C */  lui        $v0, %hi(D_800F1AFC)
    /* 7CD94 8008C594 FC1A4224 */  addiu      $v0, $v0, %lo(D_800F1AFC)
    /* 7CD98 8008C598 0300C014 */  bnez       $a2, .L8008C5A8
    /* 7CD9C 8008C59C 00000000 */   nop
    /* 7CDA0 8008C5A0 0F80023C */  lui        $v0, %hi(D_800F1AEC)
    /* 7CDA4 8008C5A4 EC1A4224 */  addiu      $v0, $v0, %lo(D_800F1AEC)
  .L8008C5A8:
    /* 7CDA8 8008C5A8 0000428C */  lw         $v0, 0x0($v0)
    /* 7CDAC 8008C5AC 57320208 */  j          .L8008C95C
    /* 7CDB0 8008C5B0 2B800200 */   sltu      $s0, $zero, $v0
    /* 7CDB4 8008C5B4 0500A22C */  sltiu      $v0, $a1, 0x5
    /* 7CDB8 8008C5B8 E8004010 */  beqz       $v0, .L8008C95C
    /* 7CDBC 8008C5BC 80100500 */   sll       $v0, $a1, 2
    /* 7CDC0 8008C5C0 0180013C */  lui        $at, %hi(func_800164AC + 0x38)
    /* 7CDC4 8008C5C4 21082200 */  addu       $at, $at, $v0
    /* 7CDC8 8008C5C8 E464228C */  lw         $v0, %lo(func_800164AC + 0x38)($at)
    /* 7CDCC 8008C5CC 00000000 */  nop
    /* 7CDD0 8008C5D0 08004000 */  jr         $v0
    /* 7CDD4 8008C5D4 00000000 */   nop
    /* 7CDD8 8008C5D8 0A80023C */  lui        $v0, %hi(D_800A3044)
    /* 7CDDC 8008C5DC 4430428C */  lw         $v0, %lo(D_800A3044)($v0)
    /* 7CDE0 8008C5E0 00000000 */  nop
    /* 7CDE4 8008C5E4 0A004294 */  lhu        $v0, 0xA($v0)
    /* 7CDE8 8008C5E8 0F80073C */  lui        $a3, %hi(D_800F1AD8)
    /* 7CDEC 8008C5EC D81AE724 */  addiu      $a3, $a3, %lo(D_800F1AD8)
    /* 7CDF0 8008C5F0 DDFF4230 */  andi       $v0, $v0, 0xFFDD
    /* 7CDF4 8008C5F4 0F80013C */  lui        $at, %hi(D_800F1AE2)
    /* 7CDF8 8008C5F8 E21A22A4 */  sh         $v0, %lo(D_800F1AE2)($at)
    /* 7CDFC 8008C5FC 0100C230 */  andi       $v0, $a2, 0x1
    /* 7CE00 8008C600 40280200 */  sll        $a1, $v0, 1
    /* 7CE04 8008C604 0F80023C */  lui        $v0, %hi(D_800F1AE2)
    /* 7CE08 8008C608 E21A4294 */  lhu        $v0, %lo(D_800F1AE2)($v0)
    /* 7CE0C 8008C60C 00000000 */  nop
    /* 7CE10 8008C610 FFFF4330 */  andi       $v1, $v0, 0xFFFF
    /* 7CE14 8008C614 0200C230 */  andi       $v0, $a2, 0x2
    /* 7CE18 8008C618 04004010 */  beqz       $v0, .L8008C62C
    /* 7CE1C 8008C61C 2120A000 */   addu      $a0, $a1, $zero
    /* 7CE20 8008C620 20006234 */  ori        $v0, $v1, 0x20
    /* 7CE24 8008C624 8C310208 */  j          .L8008C630
    /* 7CE28 8008C628 25104500 */   or        $v0, $v0, $a1
  .L8008C62C:
    /* 7CE2C 8008C62C 25106400 */  or         $v0, $v1, $a0
  .L8008C630:
    /* 7CE30 8008C630 0A00E2A4 */  sh         $v0, 0xA($a3)
    /* 7CE34 8008C634 0F80023C */  lui        $v0, %hi(D_800F1AE2)
    /* 7CE38 8008C638 E21A4224 */  addiu      $v0, $v0, %lo(D_800F1AE2)
    /* 7CE3C 8008C63C 0A80033C */  lui        $v1, %hi(D_800A3044)
    /* 7CE40 8008C640 4430638C */  lw         $v1, %lo(D_800A3044)($v1)
    /* 7CE44 8008C644 00004294 */  lhu        $v0, 0x0($v0)
    /* 7CE48 8008C648 00000000 */  nop
    /* 7CE4C 8008C64C 0A0062A4 */  sh         $v0, 0xA($v1)
    /* 7CE50 8008C650 58320208 */  j          .L8008C960
    /* 7CE54 8008C654 21100002 */   addu      $v0, $s0, $zero
    /* 7CE58 8008C658 0A80033C */  lui        $v1, %hi(D_800A3044)
    /* 7CE5C 8008C65C 4430638C */  lw         $v1, %lo(D_800A3044)($v1)
    /* 7CE60 8008C660 0F80013C */  lui        $at, %hi(D_800F1AE0)
    /* 7CE64 8008C664 E01A26A4 */  sh         $a2, %lo(D_800F1AE0)($at)
    /* 7CE68 8008C668 0F80023C */  lui        $v0, %hi(D_800F1AE0)
    /* 7CE6C 8008C66C E01A4294 */  lhu        $v0, %lo(D_800F1AE0)($v0)
    /* 7CE70 8008C670 00000000 */  nop
    /* 7CE74 8008C674 080062A4 */  sh         $v0, 0x8($v1)
    /* 7CE78 8008C678 58320208 */  j          .L8008C960
    /* 7CE7C 8008C67C 21100002 */   addu      $v0, $s0, $zero
    /* 7CE80 8008C680 1F00023C */  lui        $v0, (0x1FA400 >> 16)
    /* 7CE84 8008C684 00A44234 */  ori        $v0, $v0, (0x1FA400 & 0xFFFF)
    /* 7CE88 8008C688 1B004600 */  divu       $zero, $v0, $a2
    /* 7CE8C 8008C68C 0200C014 */  bnez       $a2, .L8008C698
    /* 7CE90 8008C690 00000000 */   nop
    /* 7CE94 8008C694 0D000700 */  break      7
  .L8008C698:
    /* 7CE98 8008C698 12180000 */  mflo       $v1
    /* 7CE9C 8008C69C 10100000 */  mfhi       $v0
    /* 7CEA0 8008C6A0 03004010 */  beqz       $v0, .L8008C6B0
    /* 7CEA4 8008C6A4 00000000 */   nop
    /* 7CEA8 8008C6A8 57320208 */  j          .L8008C95C
    /* 7CEAC 8008C6AC FFFF1024 */   addiu     $s0, $zero, -0x1
  .L8008C6B0:
    /* 7CEB0 8008C6B0 0F80013C */  lui        $at, %hi(D_800F1AE6)
    /* 7CEB4 8008C6B4 E61A23A4 */  sh         $v1, %lo(D_800F1AE6)($at)
    /* 7CEB8 8008C6B8 0A80033C */  lui        $v1, %hi(D_800A3044)
    /* 7CEBC 8008C6BC 4430638C */  lw         $v1, %lo(D_800A3044)($v1)
    /* 7CEC0 8008C6C0 0F80023C */  lui        $v0, %hi(D_800F1AE6)
    /* 7CEC4 8008C6C4 E61A4294 */  lhu        $v0, %lo(D_800F1AE6)($v0)
    /* 7CEC8 8008C6C8 00000000 */  nop
    /* 7CECC 8008C6CC 0E0062A4 */  sh         $v0, 0xE($v1)
    /* 7CED0 8008C6D0 0A006294 */  lhu        $v0, 0xA($v1)
    /* 7CED4 8008C6D4 01320208 */  j          .L8008C804
    /* 7CED8 8008C6D8 10004234 */   ori       $v0, $v0, 0x10
    /* 7CEDC 8008C6DC 9F00C010 */  beqz       $a2, .L8008C95C
    /* 7CEE0 8008C6E0 0900C22C */   sltiu     $v0, $a2, 0x9
    /* 7CEE4 8008C6E4 9D004010 */  beqz       $v0, .L8008C95C
    /* 7CEE8 8008C6E8 40100600 */   sll       $v0, $a2, 1
    /* 7CEEC 8008C6EC 0A80013C */  lui        $at, %hi(D_800A3060)
    /* 7CEF0 8008C6F0 21082200 */  addu       $at, $at, $v0
    /* 7CEF4 8008C6F4 60303084 */  lh         $s0, %lo(D_800A3060)($at)
    /* 7CEF8 8008C6F8 00000000 */  nop
    /* 7CEFC 8008C6FC 98000006 */  bltz       $s0, .L8008C960
    /* 7CF00 8008C700 21100002 */   addu      $v0, $s0, $zero
    /* 7CF04 8008C704 0F80023C */  lui        $v0, %hi(D_800F1AE2)
    /* 7CF08 8008C708 E21A4294 */  lhu        $v0, %lo(D_800F1AE2)($v0)
    /* 7CF0C 8008C70C 00000000 */  nop
    /* 7CF10 8008C710 FFFC4230 */  andi       $v0, $v0, 0xFCFF
    /* 7CF14 8008C714 0F80013C */  lui        $at, %hi(D_800F1AE2)
    /* 7CF18 8008C718 E21A22A4 */  sh         $v0, %lo(D_800F1AE2)($at)
    /* 7CF1C 8008C71C 0F80023C */  lui        $v0, %hi(D_800F1AE2)
    /* 7CF20 8008C720 E21A4294 */  lhu        $v0, %lo(D_800F1AE2)($v0)
    /* 7CF24 8008C724 0A80033C */  lui        $v1, %hi(D_800A3044)
    /* 7CF28 8008C728 4430638C */  lw         $v1, %lo(D_800A3044)($v1)
    /* 7CF2C 8008C72C 25105000 */  or         $v0, $v0, $s0
    /* 7CF30 8008C730 0F80013C */  lui        $at, %hi(D_800F1AE2)
    /* 7CF34 8008C734 E21A22A4 */  sh         $v0, %lo(D_800F1AE2)($at)
    /* 7CF38 8008C738 0F80023C */  lui        $v0, %hi(D_800F1AE2)
    /* 7CF3C 8008C73C E21A4294 */  lhu        $v0, %lo(D_800F1AE2)($v0)
    /* 7CF40 8008C740 00000000 */  nop
    /* 7CF44 8008C744 0A0062A4 */  sh         $v0, 0xA($v1)
    /* 7CF48 8008C748 58320208 */  j          .L8008C960
    /* 7CF4C 8008C74C 21100002 */   addu      $v0, $s0, $zero
    /* 7CF50 8008C750 01001124 */  addiu      $s1, $zero, 0x1
    /* 7CF54 8008C754 2500B110 */  beq        $a1, $s1, .L8008C7EC
    /* 7CF58 8008C758 00000000 */   nop
    /* 7CF5C 8008C75C 0700A010 */  beqz       $a1, .L8008C77C
    /* 7CF60 8008C760 02000224 */   addiu     $v0, $zero, 0x2
    /* 7CF64 8008C764 2A00A210 */  beq        $a1, $v0, .L8008C810
    /* 7CF68 8008C768 03000224 */   addiu     $v0, $zero, 0x3
    /* 7CF6C 8008C76C 3800A210 */  beq        $a1, $v0, .L8008C850
    /* 7CF70 8008C770 21100002 */   addu      $v0, $s0, $zero
    /* 7CF74 8008C774 58320208 */  j          .L8008C960
    /* 7CF78 8008C778 00000000 */   nop
  .L8008C77C:
    /* 7CF7C 8008C77C 6EE2010C */  jal        func_800789B8
    /* 7CF80 8008C780 00000000 */   nop
    /* 7CF84 8008C784 0A80043C */  lui        $a0, %hi(D_800A3044)
    /* 7CF88 8008C788 4430848C */  lw         $a0, %lo(D_800A3044)($a0)
    /* 7CF8C 8008C78C 00000000 */  nop
    /* 7CF90 8008C790 0A008394 */  lhu        $v1, 0xA($a0)
    /* 7CF94 8008C794 00000000 */  nop
    /* 7CF98 8008C798 50006334 */  ori        $v1, $v1, 0x50
    /* 7CF9C 8008C79C 0A0083A4 */  sh         $v1, 0xA($a0)
    /* 7CFA0 8008C7A0 0F80033C */  lui        $v1, %hi(D_800F1AE0)
    /* 7CFA4 8008C7A4 E01A6324 */  addiu      $v1, $v1, %lo(D_800F1AE0)
    /* 7CFA8 8008C7A8 00006394 */  lhu        $v1, 0x0($v1)
    /* 7CFAC 8008C7AC 00000000 */  nop
    /* 7CFB0 8008C7B0 080083A4 */  sh         $v1, 0x8($a0)
    /* 7CFB4 8008C7B4 0F80033C */  lui        $v1, %hi(D_800F1AE2)
    /* 7CFB8 8008C7B8 E21A6394 */  lhu        $v1, %lo(D_800F1AE2)($v1)
    /* 7CFBC 8008C7BC 00000000 */  nop
    /* 7CFC0 8008C7C0 0A0083A4 */  sh         $v1, 0xA($a0)
    /* 7CFC4 8008C7C4 0F80033C */  lui        $v1, %hi(D_800F1AE6)
    /* 7CFC8 8008C7C8 E61A6394 */  lhu        $v1, %lo(D_800F1AE6)($v1)
    /* 7CFCC 8008C7CC 00000000 */  nop
    /* 7CFD0 8008C7D0 0E0083A4 */  sh         $v1, 0xE($a0)
    /* 7CFD4 8008C7D4 0A008394 */  lhu        $v1, 0xA($a0)
    /* 7CFD8 8008C7D8 21800000 */  addu       $s0, $zero, $zero
    /* 7CFDC 8008C7DC 10006334 */  ori        $v1, $v1, 0x10
    /* 7CFE0 8008C7E0 0A0083A4 */  sh         $v1, 0xA($a0)
    /* 7CFE4 8008C7E4 22320208 */  j          .L8008C888
    /* 7CFE8 8008C7E8 00000000 */   nop
  .L8008C7EC:
    /* 7CFEC 8008C7EC 0A80033C */  lui        $v1, %hi(D_800A3044)
    /* 7CFF0 8008C7F0 4430638C */  lw         $v1, %lo(D_800A3044)($v1)
    /* 7CFF4 8008C7F4 00000000 */  nop
    /* 7CFF8 8008C7F8 0A006294 */  lhu        $v0, 0xA($v1)
    /* 7CFFC 8008C7FC 21800000 */  addu       $s0, $zero, $zero
    /* 7D000 8008C800 10004234 */  ori        $v0, $v0, 0x10
  .L8008C804:
    /* 7D004 8008C804 0A0062A4 */  sh         $v0, 0xA($v1)
    /* 7D008 8008C808 58320208 */  j          .L8008C960
    /* 7D00C 8008C80C 21100002 */   addu      $v0, $s0, $zero
  .L8008C810:
    /* 7D010 8008C810 6EE2010C */  jal        func_800789B8
    /* 7D014 8008C814 00000000 */   nop
    /* 7D018 8008C818 0A80033C */  lui        $v1, %hi(D_800A3044)
    /* 7D01C 8008C81C 4430638C */  lw         $v1, %lo(D_800A3044)($v1)
    /* 7D020 8008C820 00000000 */  nop
    /* 7D024 8008C824 0A006494 */  lhu        $a0, 0xA($v1)
    /* 7D028 8008C828 00000000 */  nop
    /* 7D02C 8008C82C FFFB8430 */  andi       $a0, $a0, 0xFBFF
    /* 7D030 8008C830 0A0064A4 */  sh         $a0, 0xA($v1)
    /* 7D034 8008C834 0F80033C */  lui        $v1, %hi(D_800F1AEC)
    /* 7D038 8008C838 EC1A6324 */  addiu      $v1, $v1, %lo(D_800F1AEC)
    /* 7D03C 8008C83C 000060AC */  sw         $zero, 0x0($v1)
    /* 7D040 8008C840 0F80013C */  lui        $at, %hi(D_800F1AF4)
    /* 7D044 8008C844 F41A20AC */  sw         $zero, %lo(D_800F1AF4)($at)
    /* 7D048 8008C848 22320208 */  j          .L8008C888
    /* 7D04C 8008C84C 00000000 */   nop
  .L8008C850:
    /* 7D050 8008C850 6EE2010C */  jal        func_800789B8
    /* 7D054 8008C854 00000000 */   nop
    /* 7D058 8008C858 0A80033C */  lui        $v1, %hi(D_800A3044)
    /* 7D05C 8008C85C 4430638C */  lw         $v1, %lo(D_800A3044)($v1)
    /* 7D060 8008C860 00000000 */  nop
    /* 7D064 8008C864 0A006494 */  lhu        $a0, 0xA($v1)
    /* 7D068 8008C868 00000000 */  nop
    /* 7D06C 8008C86C DFF78430 */  andi       $a0, $a0, 0xF7DF
    /* 7D070 8008C870 0A0064A4 */  sh         $a0, 0xA($v1)
    /* 7D074 8008C874 0F80033C */  lui        $v1, %hi(D_800F1AFC)
    /* 7D078 8008C878 FC1A6324 */  addiu      $v1, $v1, %lo(D_800F1AFC)
    /* 7D07C 8008C87C 000060AC */  sw         $zero, 0x0($v1)
    /* 7D080 8008C880 0F80013C */  lui        $at, %hi(D_800F1B04)
    /* 7D084 8008C884 041B20AC */  sw         $zero, %lo(D_800F1B04)($at)
  .L8008C888:
    /* 7D088 8008C888 35005114 */  bne        $v0, $s1, .L8008C960
    /* 7D08C 8008C88C 21100002 */   addu      $v0, $s0, $zero
    /* 7D090 8008C890 72E2010C */  jal        func_800789C8
    /* 7D094 8008C894 00000000 */   nop
    /* 7D098 8008C898 58320208 */  j          .L8008C960
    /* 7D09C 8008C89C 21100002 */   addu      $v0, $s0, $zero
    /* 7D0A0 8008C8A0 0500A010 */  beqz       $a1, .L8008C8B8
    /* 7D0A4 8008C8A4 01000224 */   addiu     $v0, $zero, 0x1
    /* 7D0A8 8008C8A8 1700A210 */  beq        $a1, $v0, .L8008C908
    /* 7D0AC 8008C8AC 21100002 */   addu      $v0, $s0, $zero
    /* 7D0B0 8008C8B0 58320208 */  j          .L8008C960
    /* 7D0B4 8008C8B4 00000000 */   nop
  .L8008C8B8:
    /* 7D0B8 8008C8B8 0A00C010 */  beqz       $a2, .L8008C8E4
    /* 7D0BC 8008C8BC 00000000 */   nop
    /* 7D0C0 8008C8C0 0A80033C */  lui        $v1, %hi(D_800A3044)
    /* 7D0C4 8008C8C4 4430638C */  lw         $v1, %lo(D_800A3044)($v1)
    /* 7D0C8 8008C8C8 00000000 */  nop
    /* 7D0CC 8008C8CC 0A006294 */  lhu        $v0, 0xA($v1)
    /* 7D0D0 8008C8D0 00000000 */  nop
    /* 7D0D4 8008C8D4 20004234 */  ori        $v0, $v0, 0x20
    /* 7D0D8 8008C8D8 0A0062A4 */  sh         $v0, 0xA($v1)
    /* 7D0DC 8008C8DC 58320208 */  j          .L8008C960
    /* 7D0E0 8008C8E0 21100002 */   addu      $v0, $s0, $zero
  .L8008C8E4:
    /* 7D0E4 8008C8E4 0A80033C */  lui        $v1, %hi(D_800A3044)
    /* 7D0E8 8008C8E8 4430638C */  lw         $v1, %lo(D_800A3044)($v1)
    /* 7D0EC 8008C8EC 00000000 */  nop
    /* 7D0F0 8008C8F0 0A006294 */  lhu        $v0, 0xA($v1)
    /* 7D0F4 8008C8F4 00000000 */  nop
    /* 7D0F8 8008C8F8 DFFF4230 */  andi       $v0, $v0, 0xFFDF
    /* 7D0FC 8008C8FC 0A0062A4 */  sh         $v0, 0xA($v1)
    /* 7D100 8008C900 58320208 */  j          .L8008C960
    /* 7D104 8008C904 21100002 */   addu      $v0, $s0, $zero
  .L8008C908:
    /* 7D108 8008C908 0A80023C */  lui        $v0, %hi(D_800A3044)
    /* 7D10C 8008C90C 4430428C */  lw         $v0, %lo(D_800A3044)($v0)
    /* 7D110 8008C910 00000000 */  nop
    /* 7D114 8008C914 04004294 */  lhu        $v0, 0x4($v0)
    /* 7D118 8008C918 00000000 */  nop
    /* 7D11C 8008C91C 00014230 */  andi       $v0, $v0, 0x100
    /* 7D120 8008C920 0F004010 */  beqz       $v0, .L8008C960
    /* 7D124 8008C924 21100002 */   addu      $v0, $s0, $zero
    /* 7D128 8008C928 57320208 */  j          .L8008C95C
    /* 7D12C 8008C92C 01001024 */   addiu     $s0, $zero, 0x1
    /* 7D130 8008C930 0B00A014 */  bnez       $a1, .L8008C960
    /* 7D134 8008C934 21100002 */   addu      $v0, $s0, $zero
    /* 7D138 8008C938 0F80023C */  lui        $v0, %hi(D_800F1AE8)
    /* 7D13C 8008C93C E81A4224 */  addiu      $v0, $v0, %lo(D_800F1AE8)
    /* 7D140 8008C940 0000508C */  lw         $s0, 0x0($v0)
    /* 7D144 8008C944 57320208 */  j          .L8008C95C
    /* 7D148 8008C948 000046AC */   sw        $a2, 0x0($v0)
    /* 7D14C 8008C94C 0400A014 */  bnez       $a1, .L8008C960
    /* 7D150 8008C950 21100002 */   addu      $v0, $s0, $zero
    /* 7D154 8008C954 9FEB010C */  jal        func_8007AE7C
    /* 7D158 8008C958 05000424 */   addiu     $a0, $zero, 0x5
  .L8008C95C:
    /* 7D15C 8008C95C 21100002 */  addu       $v0, $s0, $zero
  .L8008C960:
    /* 7D160 8008C960 1800BF8F */  lw         $ra, 0x18($sp)
    /* 7D164 8008C964 1400B18F */  lw         $s1, 0x14($sp)
    /* 7D168 8008C968 1000B08F */  lw         $s0, 0x10($sp)
    /* 7D16C 8008C96C 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 7D170 8008C970 0800E003 */  jr         $ra
    /* 7D174 8008C974 00000000 */   nop
    /* 7D178 8008C978 0A80023C */  lui        $v0, %hi(D_800A305C)
    /* 7D17C 8008C97C 5C30428C */  lw         $v0, %lo(D_800A305C)($v0)
    /* 7D180 8008C980 00000000 */  nop
    /* 7D184 8008C984 0000438C */  lw         $v1, 0x0($v0)
    /* 7D188 8008C988 0400428C */  lw         $v0, 0x4($v0)
    /* 7D18C 8008C98C 00000000 */  nop
    /* 7D190 8008C990 24186200 */  and        $v1, $v1, $v0
    /* 7D194 8008C994 00016330 */  andi       $v1, $v1, 0x100
    /* 7D198 8008C998 14006010 */  beqz       $v1, .L8008C9EC
    /* 7D19C 8008C99C 21100000 */   addu      $v0, $zero, $zero
    /* 7D1A0 8008C9A0 0F80023C */  lui        $v0, %hi(D_800F1AEC)
    /* 7D1A4 8008C9A4 EC1A4224 */  addiu      $v0, $v0, %lo(D_800F1AEC)
    /* 7D1A8 8008C9A8 0000428C */  lw         $v0, 0x0($v0)
    /* 7D1AC 8008C9AC 00000000 */  nop
    /* 7D1B0 8008C9B0 0E004010 */  beqz       $v0, .L8008C9EC
    /* 7D1B4 8008C9B4 01000224 */   addiu     $v0, $zero, 0x1
    /* 7D1B8 8008C9B8 0A80033C */  lui        $v1, %hi(D_800A3044)
    /* 7D1BC 8008C9BC 4430638C */  lw         $v1, %lo(D_800A3044)($v1)
    /* 7D1C0 8008C9C0 00000000 */  nop
    /* 7D1C4 8008C9C4 04006294 */  lhu        $v0, 0x4($v1)
    /* 7D1C8 8008C9C8 00000000 */  nop
    /* 7D1CC 8008C9CC 01004230 */  andi       $v0, $v0, 0x1
    /* 7D1D0 8008C9D0 06004010 */  beqz       $v0, .L8008C9EC
    /* 7D1D4 8008C9D4 01000224 */   addiu     $v0, $zero, 0x1
    /* 7D1D8 8008C9D8 0A006294 */  lhu        $v0, 0xA($v1)
    /* 7D1DC 8008C9DC 00000000 */  nop
    /* 7D1E0 8008C9E0 FFFB4230 */  andi       $v0, $v0, 0xFBFF
    /* 7D1E4 8008C9E4 0A0062A4 */  sh         $v0, 0xA($v1)
    /* 7D1E8 8008C9E8 01000224 */  addiu      $v0, $zero, 0x1
  .L8008C9EC:
    /* 7D1EC 8008C9EC 0800E003 */  jr         $ra
    /* 7D1F0 8008C9F0 00000000 */   nop
    /* 7D1F4 8008C9F4 0A80033C */  lui        $v1, %hi(D_800A305C)
    /* 7D1F8 8008C9F8 5C30638C */  lw         $v1, %lo(D_800A305C)($v1)
    /* 7D1FC 8008C9FC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 7D200 8008CA00 1000BFAF */  sw         $ra, 0x10($sp)
    /* 7D204 8008CA04 0000628C */  lw         $v0, 0x0($v1)
    /* 7D208 8008CA08 00000000 */  nop
    /* 7D20C 8008CA0C 00014230 */  andi       $v0, $v0, 0x100
    /* 7D210 8008CA10 02004010 */  beqz       $v0, .L8008CA1C
    /* 7D214 8008CA14 FFFE0224 */   addiu     $v0, $zero, -0x101
    /* 7D218 8008CA18 000062AC */  sw         $v0, 0x0($v1)
  .L8008CA1C:
    /* 7D21C 8008CA1C 0A80063C */  lui        $a2, %hi(D_800A3044)
    /* 7D220 8008CA20 4430C68C */  lw         $a2, %lo(D_800A3044)($a2)
    /* 7D224 8008CA24 00000000 */  nop
    /* 7D228 8008CA28 0400C294 */  lhu        $v0, 0x4($a2)
    /* 7D22C 8008CA2C 00000000 */  nop
    /* 7D230 8008CA30 38004230 */  andi       $v0, $v0, 0x38
    /* 7D234 8008CA34 1E004010 */  beqz       $v0, .L8008CAB0
    /* 7D238 8008CA38 00000000 */   nop
    /* 7D23C 8008CA3C 0A00C294 */  lhu        $v0, 0xA($a2)
    /* 7D240 8008CA40 00000000 */  nop
    /* 7D244 8008CA44 DFF74230 */  andi       $v0, $v0, 0xF7DF
    /* 7D248 8008CA48 0A00C2A4 */  sh         $v0, 0xA($a2)
    /* 7D24C 8008CA4C 0F80023C */  lui        $v0, %hi(D_800F1AFC)
    /* 7D250 8008CA50 FC1A4224 */  addiu      $v0, $v0, %lo(D_800F1AFC)
    /* 7D254 8008CA54 0F80013C */  lui        $at, %hi(D_800F1B04)
    /* 7D258 8008CA58 041B20AC */  sw         $zero, %lo(D_800F1B04)($at)
    /* 7D25C 8008CA5C 000040AC */  sw         $zero, 0x0($v0)
    /* 7D260 8008CA60 0A00C294 */  lhu        $v0, 0xA($a2)
    /* 7D264 8008CA64 50000224 */  addiu      $v0, $zero, 0x50
    /* 7D268 8008CA68 0A00C2A4 */  sh         $v0, 0xA($a2)
    /* 7D26C 8008CA6C 0F80023C */  lui        $v0, %hi(D_800F1AE0)
    /* 7D270 8008CA70 E01A4224 */  addiu      $v0, $v0, %lo(D_800F1AE0)
    /* 7D274 8008CA74 00004294 */  lhu        $v0, 0x0($v0)
    /* 7D278 8008CA78 00F0043C */  lui        $a0, (0xF000000B >> 16)
    /* 7D27C 8008CA7C 0800C2A4 */  sh         $v0, 0x8($a2)
    /* 7D280 8008CA80 0F80023C */  lui        $v0, %hi(D_800F1AE6)
    /* 7D284 8008CA84 E61A4294 */  lhu        $v0, %lo(D_800F1AE6)($v0)
    /* 7D288 8008CA88 0B008434 */  ori        $a0, $a0, (0xF000000B & 0xFFFF)
    /* 7D28C 8008CA8C 0E00C2A4 */  sh         $v0, 0xE($a2)
    /* 7D290 8008CA90 0A00C294 */  lhu        $v0, 0xA($a2)
    /* 7D294 8008CA94 00800534 */  ori        $a1, $zero, 0x8000
    /* 7D298 8008CA98 10004234 */  ori        $v0, $v0, 0x10
    /* 7D29C 8008CA9C 0A00C2A4 */  sh         $v0, 0xA($a2)
    /* 7D2A0 8008CAA0 2300020C */  jal        func_8008008C
    /* 7D2A4 8008CAA4 00000000 */   nop
    /* 7D2A8 8008CAA8 5F330208 */  j          .L8008CD7C
    /* 7D2AC 8008CAAC 21100000 */   addu      $v0, $zero, $zero
  .L8008CAB0:
    /* 7D2B0 8008CAB0 0F80023C */  lui        $v0, %hi(D_800F1AFC)
    /* 7D2B4 8008CAB4 FC1A4224 */  addiu      $v0, $v0, %lo(D_800F1AFC)
    /* 7D2B8 8008CAB8 0000428C */  lw         $v0, 0x0($v0)
    /* 7D2BC 8008CABC 00000000 */  nop
    /* 7D2C0 8008CAC0 6D004010 */  beqz       $v0, .L8008CC78
    /* 7D2C4 8008CAC4 00000000 */   nop
    /* 7D2C8 8008CAC8 0F80023C */  lui        $v0, %hi(D_800F1B04)
    /* 7D2CC 8008CACC 041B428C */  lw         $v0, %lo(D_800F1B04)($v0)
    /* 7D2D0 8008CAD0 00000000 */  nop
    /* 7D2D4 8008CAD4 68004010 */  beqz       $v0, .L8008CC78
    /* 7D2D8 8008CAD8 00000000 */   nop
    /* 7D2DC 8008CADC 0400C294 */  lhu        $v0, 0x4($a2)
    /* 7D2E0 8008CAE0 00000000 */  nop
    /* 7D2E4 8008CAE4 02004230 */  andi       $v0, $v0, 0x2
    /* 7D2E8 8008CAE8 63004010 */  beqz       $v0, .L8008CC78
    /* 7D2EC 8008CAEC 00000000 */   nop
    /* 7D2F0 8008CAF0 0F80023C */  lui        $v0, %hi(D_800F1AE2)
    /* 7D2F4 8008CAF4 E21A4224 */  addiu      $v0, $v0, %lo(D_800F1AE2)
    /* 7D2F8 8008CAF8 00004294 */  lhu        $v0, 0x0($v0)
    /* 7D2FC 8008CAFC 00000000 */  nop
    /* 7D300 8008CB00 00034230 */  andi       $v0, $v0, 0x300
    /* 7D304 8008CB04 C2110200 */  srl        $v0, $v0, 7
    /* 7D308 8008CB08 0A80013C */  lui        $at, %hi(D_800A3074)
    /* 7D30C 8008CB0C 21082200 */  addu       $at, $at, $v0
    /* 7D310 8008CB10 74302484 */  lh         $a0, %lo(D_800A3074)($at)
    /* 7D314 8008CB14 02000224 */  addiu      $v0, $zero, 0x2
    /* 7D318 8008CB18 13008210 */  beq        $a0, $v0, .L8008CB68
    /* 7D31C 8008CB1C 03008228 */   slti      $v0, $a0, 0x3
    /* 7D320 8008CB20 05004010 */  beqz       $v0, .L8008CB38
    /* 7D324 8008CB24 01000224 */   addiu     $v0, $zero, 0x1
    /* 7D328 8008CB28 0A008210 */  beq        $a0, $v0, .L8008CB54
    /* 7D32C 8008CB2C 00000000 */   nop
    /* 7D330 8008CB30 FB320208 */  j          .L8008CBEC
    /* 7D334 8008CB34 00000000 */   nop
  .L8008CB38:
    /* 7D338 8008CB38 04000224 */  addiu      $v0, $zero, 0x4
    /* 7D33C 8008CB3C 16008210 */  beq        $a0, $v0, .L8008CB98
    /* 7D340 8008CB40 08000224 */   addiu     $v0, $zero, 0x8
    /* 7D344 8008CB44 1B008210 */  beq        $a0, $v0, .L8008CBB4
    /* 7D348 8008CB48 00000000 */   nop
    /* 7D34C 8008CB4C FB320208 */  j          .L8008CBEC
    /* 7D350 8008CB50 00000000 */   nop
  .L8008CB54:
    /* 7D354 8008CB54 0F80033C */  lui        $v1, %hi(D_800F1B00)
    /* 7D358 8008CB58 001B638C */  lw         $v1, %lo(D_800F1B00)($v1)
    /* 7D35C 8008CB5C 0000C290 */  lbu        $v0, 0x0($a2)
    /* 7D360 8008CB60 FB320208 */  j          .L8008CBEC
    /* 7D364 8008CB64 000062A0 */   sb        $v0, 0x0($v1)
  .L8008CB68:
    /* 7D368 8008CB68 0F80033C */  lui        $v1, %hi(D_800F1B00)
    /* 7D36C 8008CB6C 001B638C */  lw         $v1, %lo(D_800F1B00)($v1)
    /* 7D370 8008CB70 0000C290 */  lbu        $v0, 0x0($a2)
    /* 7D374 8008CB74 00000000 */  nop
    /* 7D378 8008CB78 000062A0 */  sb         $v0, 0x0($v1)
    /* 7D37C 8008CB7C 0A80023C */  lui        $v0, %hi(D_800A3044)
    /* 7D380 8008CB80 4430428C */  lw         $v0, %lo(D_800A3044)($v0)
    /* 7D384 8008CB84 0F80033C */  lui        $v1, %hi(D_800F1B00)
    /* 7D388 8008CB88 001B638C */  lw         $v1, %lo(D_800F1B00)($v1)
    /* 7D38C 8008CB8C 00004290 */  lbu        $v0, 0x0($v0)
    /* 7D390 8008CB90 FB320208 */  j          .L8008CBEC
    /* 7D394 8008CB94 010062A0 */   sb        $v0, 0x1($v1)
  .L8008CB98:
    /* 7D398 8008CB98 0A80023C */  lui        $v0, %hi(D_800A3048)
    /* 7D39C 8008CB9C 4830428C */  lw         $v0, %lo(D_800A3048)($v0)
    /* 7D3A0 8008CBA0 0F80033C */  lui        $v1, %hi(D_800F1B00)
    /* 7D3A4 8008CBA4 001B638C */  lw         $v1, %lo(D_800F1B00)($v1)
    /* 7D3A8 8008CBA8 0000428C */  lw         $v0, 0x0($v0)
    /* 7D3AC 8008CBAC FB320208 */  j          .L8008CBEC
    /* 7D3B0 8008CBB0 000062AC */   sw        $v0, 0x0($v1)
  .L8008CBB4:
    /* 7D3B4 8008CBB4 0A80023C */  lui        $v0, %hi(D_800A3048)
    /* 7D3B8 8008CBB8 4830428C */  lw         $v0, %lo(D_800A3048)($v0)
    /* 7D3BC 8008CBBC 0F80033C */  lui        $v1, %hi(D_800F1B00)
    /* 7D3C0 8008CBC0 001B638C */  lw         $v1, %lo(D_800F1B00)($v1)
    /* 7D3C4 8008CBC4 0000428C */  lw         $v0, 0x0($v0)
    /* 7D3C8 8008CBC8 00000000 */  nop
    /* 7D3CC 8008CBCC 000062AC */  sw         $v0, 0x0($v1)
    /* 7D3D0 8008CBD0 0A80023C */  lui        $v0, %hi(D_800A3048)
    /* 7D3D4 8008CBD4 4830428C */  lw         $v0, %lo(D_800A3048)($v0)
    /* 7D3D8 8008CBD8 0F80033C */  lui        $v1, %hi(D_800F1B00)
    /* 7D3DC 8008CBDC 001B638C */  lw         $v1, %lo(D_800F1B00)($v1)
    /* 7D3E0 8008CBE0 0000428C */  lw         $v0, 0x0($v0)
    /* 7D3E4 8008CBE4 00000000 */  nop
    /* 7D3E8 8008CBE8 040062AC */  sw         $v0, 0x4($v1)
  .L8008CBEC:
    /* 7D3EC 8008CBEC 0F80023C */  lui        $v0, %hi(D_800F1B00)
    /* 7D3F0 8008CBF0 001B428C */  lw         $v0, %lo(D_800F1B00)($v0)
    /* 7D3F4 8008CBF4 00000000 */  nop
    /* 7D3F8 8008CBF8 21104400 */  addu       $v0, $v0, $a0
    /* 7D3FC 8008CBFC 0F80013C */  lui        $at, %hi(D_800F1B00)
    /* 7D400 8008CC00 001B22AC */  sw         $v0, %lo(D_800F1B00)($at)
    /* 7D404 8008CC04 0F80023C */  lui        $v0, %hi(D_800F1B04)
    /* 7D408 8008CC08 041B428C */  lw         $v0, %lo(D_800F1B04)($v0)
    /* 7D40C 8008CC0C 00000000 */  nop
    /* 7D410 8008CC10 23104400 */  subu       $v0, $v0, $a0
    /* 7D414 8008CC14 0F80013C */  lui        $at, %hi(D_800F1B04)
    /* 7D418 8008CC18 041B22AC */  sw         $v0, %lo(D_800F1B04)($at)
    /* 7D41C 8008CC1C 0F80023C */  lui        $v0, %hi(D_800F1B04)
    /* 7D420 8008CC20 041B428C */  lw         $v0, %lo(D_800F1B04)($v0)
    /* 7D424 8008CC24 00000000 */  nop
    /* 7D428 8008CC28 0C004014 */  bnez       $v0, .L8008CC5C
    /* 7D42C 8008CC2C 00F0043C */   lui       $a0, (0xF000000B >> 16)
    /* 7D430 8008CC30 0A80033C */  lui        $v1, %hi(D_800A3044)
    /* 7D434 8008CC34 4430638C */  lw         $v1, %lo(D_800A3044)($v1)
    /* 7D438 8008CC38 0B008434 */  ori        $a0, $a0, (0xF000000B & 0xFFFF)
    /* 7D43C 8008CC3C 0F80013C */  lui        $at, %hi(D_800F1AFC)
    /* 7D440 8008CC40 FC1A20AC */  sw         $zero, %lo(D_800F1AFC)($at)
    /* 7D444 8008CC44 0A006294 */  lhu        $v0, 0xA($v1)
    /* 7D448 8008CC48 00040524 */  addiu      $a1, $zero, 0x400
    /* 7D44C 8008CC4C DFF74230 */  andi       $v0, $v0, 0xF7DF
    /* 7D450 8008CC50 0A0062A4 */  sh         $v0, 0xA($v1)
    /* 7D454 8008CC54 2300020C */  jal        func_8008008C
    /* 7D458 8008CC58 00000000 */   nop
  .L8008CC5C:
    /* 7D45C 8008CC5C 0A80033C */  lui        $v1, %hi(D_800A3044)
    /* 7D460 8008CC60 4430638C */  lw         $v1, %lo(D_800A3044)($v1)
    /* 7D464 8008CC64 00000000 */  nop
    /* 7D468 8008CC68 0A006294 */  lhu        $v0, 0xA($v1)
    /* 7D46C 8008CC6C 00000000 */  nop
    /* 7D470 8008CC70 02004238 */  xori       $v0, $v0, 0x2
    /* 7D474 8008CC74 0A0062A4 */  sh         $v0, 0xA($v1)
  .L8008CC78:
    /* 7D478 8008CC78 0F80043C */  lui        $a0, %hi(D_800F1AEC)
    /* 7D47C 8008CC7C EC1A8424 */  addiu      $a0, $a0, %lo(D_800F1AEC)
    /* 7D480 8008CC80 0000828C */  lw         $v0, 0x0($a0)
    /* 7D484 8008CC84 00000000 */  nop
    /* 7D488 8008CC88 35004010 */  beqz       $v0, .L8008CD60
    /* 7D48C 8008CC8C 00000000 */   nop
    /* 7D490 8008CC90 0A80053C */  lui        $a1, %hi(D_800A3044)
    /* 7D494 8008CC94 4430A58C */  lw         $a1, %lo(D_800A3044)($a1)
    /* 7D498 8008CC98 00000000 */  nop
    /* 7D49C 8008CC9C 0400A294 */  lhu        $v0, 0x4($a1)
    /* 7D4A0 8008CCA0 00000000 */  nop
    /* 7D4A4 8008CCA4 01004230 */  andi       $v0, $v0, 0x1
    /* 7D4A8 8008CCA8 2D004010 */  beqz       $v0, .L8008CD60
    /* 7D4AC 8008CCAC 00000000 */   nop
    /* 7D4B0 8008CCB0 0400A294 */  lhu        $v0, 0x4($a1)
    /* 7D4B4 8008CCB4 0C00838C */  lw         $v1, 0xC($a0)
    /* 7D4B8 8008CCB8 80004230 */  andi       $v0, $v0, 0x80
    /* 7D4BC 8008CCBC 28004314 */  bne        $v0, $v1, .L8008CD60
    /* 7D4C0 8008CCC0 00000000 */   nop
    /* 7D4C4 8008CCC4 0800828C */  lw         $v0, 0x8($a0)
    /* 7D4C8 8008CCC8 00000000 */  nop
    /* 7D4CC 8008CCCC 08004014 */  bnez       $v0, .L8008CCF0
    /* 7D4D0 8008CCD0 00000000 */   nop
    /* 7D4D4 8008CCD4 000080AC */  sw         $zero, 0x0($a0)
    /* 7D4D8 8008CCD8 00F0043C */  lui        $a0, (0xF000000B >> 16)
    /* 7D4DC 8008CCDC 0B008434 */  ori        $a0, $a0, (0xF000000B & 0xFFFF)
    /* 7D4E0 8008CCE0 2300020C */  jal        func_8008008C
    /* 7D4E4 8008CCE4 00080524 */   addiu     $a1, $zero, 0x800
    /* 7D4E8 8008CCE8 58330208 */  j          .L8008CD60
    /* 7D4EC 8008CCEC 00000000 */   nop
  .L8008CCF0:
    /* 7D4F0 8008CCF0 0C00828C */  lw         $v0, 0xC($a0)
    /* 7D4F4 8008CCF4 00000000 */  nop
    /* 7D4F8 8008CCF8 80004238 */  xori       $v0, $v0, 0x80
    /* 7D4FC 8008CCFC 0C0082AC */  sw         $v0, 0xC($a0)
    /* 7D500 8008CD00 0F80023C */  lui        $v0, %hi(D_800F1AF0)
    /* 7D504 8008CD04 F01A428C */  lw         $v0, %lo(D_800F1AF0)($v0)
    /* 7D508 8008CD08 00000000 */  nop
    /* 7D50C 8008CD0C 00004290 */  lbu        $v0, 0x0($v0)
    /* 7D510 8008CD10 00000000 */  nop
    /* 7D514 8008CD14 0000A2A0 */  sb         $v0, 0x0($a1)
    /* 7D518 8008CD18 0F80023C */  lui        $v0, %hi(D_800F1AF0)
    /* 7D51C 8008CD1C F01A428C */  lw         $v0, %lo(D_800F1AF0)($v0)
    /* 7D520 8008CD20 00000000 */  nop
    /* 7D524 8008CD24 01004224 */  addiu      $v0, $v0, 0x1
    /* 7D528 8008CD28 0F80013C */  lui        $at, %hi(D_800F1AF0)
    /* 7D52C 8008CD2C F01A22AC */  sw         $v0, %lo(D_800F1AF0)($at)
    /* 7D530 8008CD30 0F80023C */  lui        $v0, %hi(D_800F1AF0)
    /* 7D534 8008CD34 F01A428C */  lw         $v0, %lo(D_800F1AF0)($v0)
    /* 7D538 8008CD38 0800828C */  lw         $v0, 0x8($a0)
    /* 7D53C 8008CD3C 0A80033C */  lui        $v1, %hi(D_800A3044)
    /* 7D540 8008CD40 4430638C */  lw         $v1, %lo(D_800A3044)($v1)
    /* 7D544 8008CD44 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 7D548 8008CD48 080082AC */  sw         $v0, 0x8($a0)
    /* 7D54C 8008CD4C 0800828C */  lw         $v0, 0x8($a0)
    /* 7D550 8008CD50 0A006294 */  lhu        $v0, 0xA($v1)
    /* 7D554 8008CD54 00000000 */  nop
    /* 7D558 8008CD58 00044234 */  ori        $v0, $v0, 0x400
    /* 7D55C 8008CD5C 0A0062A4 */  sh         $v0, 0xA($v1)
  .L8008CD60:
    /* 7D560 8008CD60 0A80043C */  lui        $a0, %hi(D_800A3044)
    /* 7D564 8008CD64 4430848C */  lw         $a0, %lo(D_800A3044)($a0)
    /* 7D568 8008CD68 00000000 */  nop
    /* 7D56C 8008CD6C 0A008394 */  lhu        $v1, 0xA($a0)
    /* 7D570 8008CD70 21100000 */  addu       $v0, $zero, $zero
    /* 7D574 8008CD74 10006334 */  ori        $v1, $v1, 0x10
    /* 7D578 8008CD78 0A0083A4 */  sh         $v1, 0xA($a0)
  .L8008CD7C:
    /* 7D57C 8008CD7C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 7D580 8008CD80 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7D584 8008CD84 0800E003 */  jr         $ra
    /* 7D588 8008CD88 00000000 */   nop
    /* 7D58C 8008CD8C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 7D590 8008CD90 0A80033C */  lui        $v1, %hi(D_800A3044)
    /* 7D594 8008CD94 4430638C */  lw         $v1, %lo(D_800A3044)($v1)
    /* 7D598 8008CD98 05000224 */  addiu      $v0, $zero, 0x5
    /* 7D59C 8008CD9C 0F80013C */  lui        $at, %hi(D_800F1AE2)
    /* 7D5A0 8008CDA0 E21A22A4 */  sh         $v0, %lo(D_800F1AE2)($at)
    /* 7D5A4 8008CDA4 CE000224 */  addiu      $v0, $zero, 0xCE
    /* 7D5A8 8008CDA8 0F80013C */  lui        $at, %hi(D_800F1AE0)
    /* 7D5AC 8008CDAC E01A22A4 */  sh         $v0, %lo(D_800F1AE0)($at)
    /* 7D5B0 8008CDB0 D8000224 */  addiu      $v0, $zero, 0xD8
    /* 7D5B4 8008CDB4 0F80013C */  lui        $at, %hi(D_800F1AE6)
    /* 7D5B8 8008CDB8 E61A22A4 */  sh         $v0, %lo(D_800F1AE6)($at)
    /* 7D5BC 8008CDBC 50000224 */  addiu      $v0, $zero, 0x50
    /* 7D5C0 8008CDC0 1400BFAF */  sw         $ra, 0x14($sp)
    /* 7D5C4 8008CDC4 1000B0AF */  sw         $s0, 0x10($sp)
    /* 7D5C8 8008CDC8 0A0062A4 */  sh         $v0, 0xA($v1)
    /* 7D5CC 8008CDCC 0F80023C */  lui        $v0, %hi(D_800F1AE0)
    /* 7D5D0 8008CDD0 E01A4294 */  lhu        $v0, %lo(D_800F1AE0)($v0)
    /* 7D5D4 8008CDD4 00000000 */  nop
    /* 7D5D8 8008CDD8 080062A4 */  sh         $v0, 0x8($v1)
    /* 7D5DC 8008CDDC 0F80023C */  lui        $v0, %hi(D_800F1AE6)
    /* 7D5E0 8008CDE0 E61A4294 */  lhu        $v0, %lo(D_800F1AE6)($v0)
    /* 7D5E4 8008CDE4 03000424 */  addiu      $a0, $zero, 0x3
    /* 7D5E8 8008CDE8 0E0062A4 */  sh         $v0, 0xE($v1)
    /* 7D5EC 8008CDEC 0A006294 */  lhu        $v0, 0xA($v1)
    /* 7D5F0 8008CDF0 0A80103C */  lui        $s0, %hi(D_800A304C)
    /* 7D5F4 8008CDF4 4C301026 */  addiu      $s0, $s0, %lo(D_800A304C)
    /* 7D5F8 8008CDF8 10004234 */  ori        $v0, $v0, 0x10
    /* 7D5FC 8008CDFC 0A0062A4 */  sh         $v0, 0xA($v1)
    /* 7D600 8008CE00 0F80023C */  lui        $v0, %hi(D_800F1AE2)
    /* 7D604 8008CE04 E21A4294 */  lhu        $v0, %lo(D_800F1AE2)($v0)
    /* 7D608 8008CE08 21280002 */  addu       $a1, $s0, $zero
    /* 7D60C 8008CE0C 0A0062A4 */  sh         $v0, 0xA($v1)
    /* 7D610 8008CE10 D4E3010C */  jal        func_80078F50
    /* 7D614 8008CE14 00000000 */   nop
    /* 7D618 8008CE18 03000424 */  addiu      $a0, $zero, 0x3
    /* 7D61C 8008CE1C D0E3010C */  jal        func_80078F40
    /* 7D620 8008CE20 21280002 */   addu      $a1, $s0, $zero
    /* 7D624 8008CE24 0A80033C */  lui        $v1, %hi(D_800A305C)
    /* 7D628 8008CE28 5C30638C */  lw         $v1, %lo(D_800A305C)($v1)
    /* 7D62C 8008CE2C 00000000 */  nop
    /* 7D630 8008CE30 0400628C */  lw         $v0, 0x4($v1)
    /* 7D634 8008CE34 00000000 */  nop
    /* 7D638 8008CE38 00014234 */  ori        $v0, $v0, 0x100
    /* 7D63C 8008CE3C 040062AC */  sw         $v0, 0x4($v1)
    /* 7D640 8008CE40 0F80023C */  lui        $v0, %hi(D_800F1AFC)
    /* 7D644 8008CE44 FC1A4224 */  addiu      $v0, $v0, %lo(D_800F1AFC)
    /* 7D648 8008CE48 000040AC */  sw         $zero, 0x0($v0)
    /* 7D64C 8008CE4C 0000438C */  lw         $v1, 0x0($v0)
    /* 7D650 8008CE50 0F80023C */  lui        $v0, %hi(D_800F1AEC)
    /* 7D654 8008CE54 EC1A4224 */  addiu      $v0, $v0, %lo(D_800F1AEC)
    /* 7D658 8008CE58 000043AC */  sw         $v1, 0x0($v0)
    /* 7D65C 8008CE5C 0F80013C */  lui        $at, %hi(D_800F1B00)
    /* 7D660 8008CE60 001B20AC */  sw         $zero, %lo(D_800F1B00)($at)
    /* 7D664 8008CE64 0F80023C */  lui        $v0, %hi(D_800F1B00)
    /* 7D668 8008CE68 001B428C */  lw         $v0, %lo(D_800F1B00)($v0)
    /* 7D66C 8008CE6C 0F80013C */  lui        $at, %hi(D_800F1AE8)
    /* 7D670 8008CE70 E81A20AC */  sw         $zero, %lo(D_800F1AE8)($at)
    /* 7D674 8008CE74 0F80013C */  lui        $at, %hi(D_800F1AF0)
    /* 7D678 8008CE78 F01A22AC */  sw         $v0, %lo(D_800F1AF0)($at)
    /* 7D67C 8008CE7C 0F80013C */  lui        $at, %hi(D_800F1B04)
    /* 7D680 8008CE80 041B20AC */  sw         $zero, %lo(D_800F1B04)($at)
    /* 7D684 8008CE84 0F80033C */  lui        $v1, %hi(D_800F1B04)
    /* 7D688 8008CE88 041B638C */  lw         $v1, %lo(D_800F1B04)($v1)
    /* 7D68C 8008CE8C 21100000 */  addu       $v0, $zero, $zero
    /* 7D690 8008CE90 0F80013C */  lui        $at, %hi(D_800F1AF4)
    /* 7D694 8008CE94 F41A23AC */  sw         $v1, %lo(D_800F1AF4)($at)
    /* 7D698 8008CE98 1400BF8F */  lw         $ra, 0x14($sp)
    /* 7D69C 8008CE9C 1000B08F */  lw         $s0, 0x10($sp)
    /* 7D6A0 8008CEA0 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7D6A4 8008CEA4 0800E003 */  jr         $ra
    /* 7D6A8 8008CEA8 00000000 */   nop
    /* 7D6AC 8008CEAC 0800E003 */  jr         $ra
    /* 7D6B0 8008CEB0 21100000 */   addu      $v0, $zero, $zero
    /* 7D6B4 8008CEB4 0800E003 */  jr         $ra
    /* 7D6B8 8008CEB8 21100000 */   addu      $v0, $zero, $zero
    /* 7D6BC 8008CEBC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 7D6C0 8008CEC0 0A80053C */  lui        $a1, %hi(D_800A305C)
    /* 7D6C4 8008CEC4 5C30A58C */  lw         $a1, %lo(D_800A305C)($a1)
    /* 7D6C8 8008CEC8 FFFE0424 */  addiu      $a0, $zero, -0x101
    /* 7D6CC 8008CECC 1000BFAF */  sw         $ra, 0x10($sp)
    /* 7D6D0 8008CED0 0400A28C */  lw         $v0, 0x4($a1)
    /* 7D6D4 8008CED4 0A80033C */  lui        $v1, %hi(D_800A3044)
    /* 7D6D8 8008CED8 4430638C */  lw         $v1, %lo(D_800A3044)($v1)
    /* 7D6DC 8008CEDC 24104400 */  and        $v0, $v0, $a0
    /* 7D6E0 8008CEE0 0400A2AC */  sw         $v0, 0x4($a1)
    /* 7D6E4 8008CEE4 0A006294 */  lhu        $v0, 0xA($v1)
    /* 7D6E8 8008CEE8 03000424 */  addiu      $a0, $zero, 0x3
    /* 7D6EC 8008CEEC 50004234 */  ori        $v0, $v0, 0x50
    /* 7D6F0 8008CEF0 0A0062A4 */  sh         $v0, 0xA($v1)
    /* 7D6F4 8008CEF4 0F80023C */  lui        $v0, %hi(D_800F1AE2)
    /* 7D6F8 8008CEF8 E21A4224 */  addiu      $v0, $v0, %lo(D_800F1AE2)
    /* 7D6FC 8008CEFC 00004294 */  lhu        $v0, 0x0($v0)
    /* 7D700 8008CF00 0A80053C */  lui        $a1, %hi(D_800A304C)
    /* 7D704 8008CF04 4C30A524 */  addiu      $a1, $a1, %lo(D_800A304C)
    /* 7D708 8008CF08 0A0062A4 */  sh         $v0, 0xA($v1)
    /* 7D70C 8008CF0C D4E3010C */  jal        func_80078F50
    /* 7D710 8008CF10 00000000 */   nop
    /* 7D714 8008CF14 0F80023C */  lui        $v0, %hi(D_800F1AFC)
    /* 7D718 8008CF18 FC1A4224 */  addiu      $v0, $v0, %lo(D_800F1AFC)
    /* 7D71C 8008CF1C 000040AC */  sw         $zero, 0x0($v0)
    /* 7D720 8008CF20 0000438C */  lw         $v1, 0x0($v0)
    /* 7D724 8008CF24 0F80023C */  lui        $v0, %hi(D_800F1AEC)
    /* 7D728 8008CF28 EC1A4224 */  addiu      $v0, $v0, %lo(D_800F1AEC)
    /* 7D72C 8008CF2C 000043AC */  sw         $v1, 0x0($v0)
    /* 7D730 8008CF30 0F80013C */  lui        $at, %hi(D_800F1B04)
    /* 7D734 8008CF34 041B20AC */  sw         $zero, %lo(D_800F1B04)($at)
    /* 7D738 8008CF38 0F80033C */  lui        $v1, %hi(D_800F1B04)
    /* 7D73C 8008CF3C 041B638C */  lw         $v1, %lo(D_800F1B04)($v1)
    /* 7D740 8008CF40 21100000 */  addu       $v0, $zero, $zero
    /* 7D744 8008CF44 0F80013C */  lui        $at, %hi(D_800F1AF4)
    /* 7D748 8008CF48 F41A23AC */  sw         $v1, %lo(D_800F1AF4)($at)
    /* 7D74C 8008CF4C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 7D750 8008CF50 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7D754 8008CF54 0800E003 */  jr         $ra
    /* 7D758 8008CF58 00000000 */   nop
    /* 7D75C 8008CF5C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 7D760 8008CF60 1000B0AF */  sw         $s0, 0x10($sp)
    /* 7D764 8008CF64 21808000 */  addu       $s0, $a0, $zero
    /* 7D768 8008CF68 01000224 */  addiu      $v0, $zero, 0x1
    /* 7D76C 8008CF6C 1400BFAF */  sw         $ra, 0x14($sp)
    /* 7D770 8008CF70 1400A214 */  bne        $a1, $v0, .L8008CFC4
    /* 7D774 8008CF74 180000AE */   sw        $zero, 0x18($s0)
    /* 7D778 8008CF78 0000028E */  lw         $v0, 0x0($s0)
    /* 7D77C 8008CF7C 00000000 */  nop
    /* 7D780 8008CF80 00804230 */  andi       $v0, $v0, 0x8000
    /* 7D784 8008CF84 09004010 */  beqz       $v0, .L8008CFAC
    /* 7D788 8008CF88 00000000 */   nop
    /* 7D78C 8008CF8C 0800048E */  lw         $a0, 0x8($s0)
    /* 7D790 8008CF90 0C00058E */  lw         $a1, 0xC($s0)
    /* 7D794 8008CF94 A92F020C */  jal        func_8008BEA4
    /* 7D798 8008CF98 00000000 */   nop
    /* 7D79C 8008CF9C 17004010 */  beqz       $v0, .L8008CFFC
    /* 7D7A0 8008CFA0 10000224 */   addiu     $v0, $zero, 0x10
    /* 7D7A4 8008CFA4 0C340208 */  j          .L8008D030
    /* 7D7A8 8008CFA8 180002AE */   sw        $v0, 0x18($s0)
  .L8008CFAC:
    /* 7D7AC 8008CFAC 0800048E */  lw         $a0, 0x8($s0)
    /* 7D7B0 8008CFB0 0C00058E */  lw         $a1, 0xC($s0)
    /* 7D7B4 8008CFB4 C12F020C */  jal        func_8008BF04
    /* 7D7B8 8008CFB8 00000000 */   nop
    /* 7D7BC 8008CFBC 06340208 */  j          .L8008D018
    /* 7D7C0 8008CFC0 21184000 */   addu      $v1, $v0, $zero
  .L8008CFC4:
    /* 7D7C4 8008CFC4 02000224 */  addiu      $v0, $zero, 0x2
    /* 7D7C8 8008CFC8 1800A214 */  bne        $a1, $v0, .L8008D02C
    /* 7D7CC 8008CFCC 16000224 */   addiu     $v0, $zero, 0x16
    /* 7D7D0 8008CFD0 0000028E */  lw         $v0, 0x0($s0)
    /* 7D7D4 8008CFD4 00000000 */  nop
    /* 7D7D8 8008CFD8 00804230 */  andi       $v0, $v0, 0x8000
    /* 7D7DC 8008CFDC 09004010 */  beqz       $v0, .L8008D004
    /* 7D7E0 8008CFE0 00000000 */   nop
    /* 7D7E4 8008CFE4 0800048E */  lw         $a0, 0x8($s0)
    /* 7D7E8 8008CFE8 0C00058E */  lw         $a1, 0xC($s0)
    /* 7D7EC 8008CFEC 6130020C */  jal        func_8008C184
    /* 7D7F0 8008CFF0 00000000 */   nop
    /* 7D7F4 8008CFF4 0D004014 */  bnez       $v0, .L8008D02C
    /* 7D7F8 8008CFF8 10000224 */   addiu     $v0, $zero, 0x10
  .L8008CFFC:
    /* 7D7FC 8008CFFC 0D340208 */  j          .L8008D034
    /* 7D800 8008D000 21100000 */   addu      $v0, $zero, $zero
  .L8008D004:
    /* 7D804 8008D004 0800048E */  lw         $a0, 0x8($s0)
    /* 7D808 8008D008 0C00058E */  lw         $a1, 0xC($s0)
    /* 7D80C 8008D00C 7A30020C */  jal        func_8008C1E8
    /* 7D810 8008D010 00000000 */   nop
    /* 7D814 8008D014 21184000 */  addu       $v1, $v0, $zero
  .L8008D018:
    /* 7D818 8008D018 02006104 */  bgez       $v1, .L8008D024
    /* 7D81C 8008D01C 05000224 */   addiu     $v0, $zero, 0x5
    /* 7D820 8008D020 180002AE */  sw         $v0, 0x18($s0)
  .L8008D024:
    /* 7D824 8008D024 0D340208 */  j          .L8008D034
    /* 7D828 8008D028 21106000 */   addu      $v0, $v1, $zero
  .L8008D02C:
    /* 7D82C 8008D02C 180002AE */  sw         $v0, 0x18($s0)
  .L8008D030:
    /* 7D830 8008D030 FFFF0224 */  addiu      $v0, $zero, -0x1
  .L8008D034:
    /* 7D834 8008D034 1400BF8F */  lw         $ra, 0x14($sp)
    /* 7D838 8008D038 1000B08F */  lw         $s0, 0x10($sp)
    /* 7D83C 8008D03C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7D840 8008D040 0800E003 */  jr         $ra
    /* 7D844 8008D044 00000000 */   nop
    /* 7D848 8008D048 0800E003 */  jr         $ra
    /* 7D84C 8008D04C 21100000 */   addu      $v0, $zero, $zero
endlabel func_8008C464
