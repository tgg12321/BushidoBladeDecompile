glabel func_8006CCC8
    /* 5D4C8 8006CCC8 B8FFBD27 */  addiu      $sp, $sp, -0x48
    /* 5D4CC 8006CCCC 3000B4AF */  sw         $s4, 0x30($sp)
    /* 5D4D0 8006CCD0 21A0A000 */  addu       $s4, $a1, $zero
    /* 5D4D4 8006CCD4 4000BEAF */  sw         $fp, 0x40($sp)
    /* 5D4D8 8006CCD8 21F00000 */  addu       $fp, $zero, $zero
    /* 5D4DC 8006CCDC 0500023C */  lui        $v0, (0x50005 >> 16)
    /* 5D4E0 8006CCE0 3004838F */  lw         $v1, %gp_rel(D_800A34FC)($gp)
    /* 5D4E4 8006CCE4 05004234 */  ori        $v0, $v0, (0x50005 & 0xFFFF)
    /* 5D4E8 8006CCE8 4400BFAF */  sw         $ra, 0x44($sp)
    /* 5D4EC 8006CCEC 3C00B7AF */  sw         $s7, 0x3C($sp)
    /* 5D4F0 8006CCF0 3800B6AF */  sw         $s6, 0x38($sp)
    /* 5D4F4 8006CCF4 3400B5AF */  sw         $s5, 0x34($sp)
    /* 5D4F8 8006CCF8 2C00B3AF */  sw         $s3, 0x2C($sp)
    /* 5D4FC 8006CCFC 2800B2AF */  sw         $s2, 0x28($sp)
    /* 5D500 8006CD00 2400B1AF */  sw         $s1, 0x24($sp)
    /* 5D504 8006CD04 2000B0AF */  sw         $s0, 0x20($sp)
    /* 5D508 8006CD08 2800638C */  lw         $v1, 0x28($v1)
    /* 5D50C 8006CD0C 00000000 */  nop
    /* 5D510 8006CD10 0B006214 */  bne        $v1, $v0, .L8006CD40
    /* 5D514 8006CD14 21B8C000 */   addu      $s7, $a2, $zero
    /* 5D518 8006CD18 4000033C */  lui        $v1, (0x400040 >> 16)
    /* 5D51C 8006CD1C 0000828E */  lw         $v0, 0x0($s4)
    /* 5D520 8006CD20 40006334 */  ori        $v1, $v1, (0x400040 & 0xFFFF)
    /* 5D524 8006CD24 24104300 */  and        $v0, $v0, $v1
    /* 5D528 8006CD28 05004010 */  beqz       $v0, .L8006CD40
    /* 5D52C 8006CD2C 01000424 */   addiu     $a0, $zero, 0x1
    /* 5D530 8006CD30 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5D534 8006CD34 9471010C */  jal        func_8005C650
    /* 5D538 8006CD38 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5D53C 8006CD3C 01001E24 */  addiu      $fp, $zero, 0x1
  .L8006CD40:
    /* 5D540 8006CD40 21880000 */  addu       $s1, $zero, $zero
    /* 5D544 8006CD44 0F001624 */  addiu      $s6, $zero, 0xF
    /* 5D548 8006CD48 21A80000 */  addu       $s5, $zero, $zero
    /* 5D54C 8006CD4C 21900000 */  addu       $s2, $zero, $zero
    /* 5D550 8006CD50 21980000 */  addu       $s3, $zero, $zero
  .L8006CD54:
    /* 5D554 8006CD54 00141700 */  sll        $v0, $s7, 16
    /* 5D558 8006CD58 03140200 */  sra        $v0, $v0, 16
    /* 5D55C 8006CD5C 07102202 */  srav       $v0, $v0, $s1
    /* 5D560 8006CD60 01004230 */  andi       $v0, $v0, 0x1
    /* 5D564 8006CD64 02004010 */  beqz       $v0, .L8006CD70
    /* 5D568 8006CD68 05001024 */   addiu     $s0, $zero, 0x5
    /* 5D56C 8006CD6C 04001024 */  addiu      $s0, $zero, 0x4
  .L8006CD70:
    /* 5D570 8006CD70 00100224 */  addiu      $v0, $zero, 0x1000
    /* 5D574 8006CD74 0000838E */  lw         $v1, 0x0($s4)
    /* 5D578 8006CD78 04104202 */  sllv       $v0, $v0, $s2
    /* 5D57C 8006CD7C 24106200 */  and        $v0, $v1, $v0
    /* 5D580 8006CD80 10004010 */  beqz       $v0, .L8006CDC4
    /* 5D584 8006CD84 21200000 */   addu      $a0, $zero, $zero
    /* 5D588 8006CD88 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5D58C 8006CD8C 9471010C */  jal        func_8005C650
    /* 5D590 8006CD90 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5D594 8006CD94 3004828F */  lw         $v0, %gp_rel(D_800A34FC)($gp)
    /* 5D598 8006CD98 00000000 */  nop
    /* 5D59C 8006CD9C 21206202 */  addu       $a0, $s3, $v0
    /* 5D5A0 8006CDA0 28008284 */  lh         $v0, 0x28($a0)
    /* 5D5A4 8006CDA4 00000000 */  nop
    /* 5D5A8 8006CDA8 0300401C */  bgtz       $v0, .L8006CDB8
    /* 5D5AC 8006CDAC 21184000 */   addu      $v1, $v0, $zero
    /* 5D5B0 8006CDB0 84B30108 */  j          .L8006CE10
    /* 5D5B4 8006CDB4 280090A4 */   sh        $s0, 0x28($a0)
  .L8006CDB8:
    /* 5D5B8 8006CDB8 FFFF6224 */  addiu      $v0, $v1, -0x1
    /* 5D5BC 8006CDBC 84B30108 */  j          .L8006CE10
    /* 5D5C0 8006CDC0 280082A4 */   sh        $v0, 0x28($a0)
  .L8006CDC4:
    /* 5D5C4 8006CDC4 00400224 */  addiu      $v0, $zero, 0x4000
    /* 5D5C8 8006CDC8 04104202 */  sllv       $v0, $v0, $s2
    /* 5D5CC 8006CDCC 24106200 */  and        $v0, $v1, $v0
    /* 5D5D0 8006CDD0 0F004010 */  beqz       $v0, .L8006CE10
    /* 5D5D4 8006CDD4 7F000524 */   addiu     $a1, $zero, 0x7F
    /* 5D5D8 8006CDD8 9471010C */  jal        func_8005C650
    /* 5D5DC 8006CDDC 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5D5E0 8006CDE0 3004828F */  lw         $v0, %gp_rel(D_800A34FC)($gp)
    /* 5D5E4 8006CDE4 00000000 */  nop
    /* 5D5E8 8006CDE8 21186202 */  addu       $v1, $s3, $v0
    /* 5D5EC 8006CDEC 28006284 */  lh         $v0, 0x28($v1)
    /* 5D5F0 8006CDF0 00000000 */  nop
    /* 5D5F4 8006CDF4 21204000 */  addu       $a0, $v0, $zero
    /* 5D5F8 8006CDF8 2A105000 */  slt        $v0, $v0, $s0
    /* 5D5FC 8006CDFC 03004014 */  bnez       $v0, .L8006CE0C
    /* 5D600 8006CE00 01008224 */   addiu     $v0, $a0, 0x1
    /* 5D604 8006CE04 84B30108 */  j          .L8006CE10
    /* 5D608 8006CE08 280060A4 */   sh        $zero, 0x28($v1)
  .L8006CE0C:
    /* 5D60C 8006CE0C 280062A4 */  sh         $v0, 0x28($v1)
  .L8006CE10:
    /* 5D610 8006CE10 3004828F */  lw         $v0, %gp_rel(D_800A34FC)($gp)
    /* 5D614 8006CE14 00000000 */  nop
    /* 5D618 8006CE18 21106202 */  addu       $v0, $s3, $v0
    /* 5D61C 8006CE1C 28004384 */  lh         $v1, 0x28($v0)
    /* 5D620 8006CE20 03000224 */  addiu      $v0, $zero, 0x3
    /* 5D624 8006CE24 16006210 */  beq        $v1, $v0, .L8006CE80
    /* 5D628 8006CE28 04006228 */   slti      $v0, $v1, 0x4
    /* 5D62C 8006CE2C 05004014 */  bnez       $v0, .L8006CE44
    /* 5D630 8006CE30 04000224 */   addiu     $v0, $zero, 0x4
    /* 5D634 8006CE34 31006210 */  beq        $v1, $v0, .L8006CEFC
    /* 5D638 8006CE38 40000224 */   addiu     $v0, $zero, 0x40
    /* 5D63C 8006CE3C DCB30108 */  j          .L8006CF70
    /* 5D640 8006CE40 0400B526 */   addiu     $s5, $s5, 0x4
  .L8006CE44:
    /* 5D644 8006CE44 49006004 */  bltz       $v1, .L8006CF6C
    /* 5D648 8006CE48 F0000224 */   addiu     $v0, $zero, 0xF0
    /* 5D64C 8006CE4C 0000838E */  lw         $v1, 0x0($s4)
    /* 5D650 8006CE50 04104202 */  sllv       $v0, $v0, $s2
    /* 5D654 8006CE54 24186200 */  and        $v1, $v1, $v0
    /* 5D658 8006CE58 44006010 */  beqz       $v1, .L8006CF6C
    /* 5D65C 8006CE5C 21200000 */   addu      $a0, $zero, $zero
    /* 5D660 8006CE60 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5D664 8006CE64 9471010C */  jal        func_8005C650
    /* 5D668 8006CE68 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5D66C 8006CE6C 0000858E */  lw         $a1, 0x0($s4)
    /* 5D670 8006CE70 F5B2010C */  jal        func_8006CBD4
    /* 5D674 8006CE74 21202002 */   addu      $a0, $s1, $zero
    /* 5D678 8006CE78 DCB30108 */  j          .L8006CF70
    /* 5D67C 8006CE7C 0400B526 */   addiu     $s5, $s5, 0x4
  .L8006CE80:
    /* 5D680 8006CE80 40000224 */  addiu      $v0, $zero, 0x40
    /* 5D684 8006CE84 0000838E */  lw         $v1, 0x0($s4)
    /* 5D688 8006CE88 04104202 */  sllv       $v0, $v0, $s2
    /* 5D68C 8006CE8C 24186200 */  and        $v1, $v1, $v0
    /* 5D690 8006CE90 36006010 */  beqz       $v1, .L8006CF6C
    /* 5D694 8006CE94 01000424 */   addiu     $a0, $zero, 0x1
    /* 5D698 8006CE98 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5D69C 8006CE9C 9471010C */  jal        func_8005C650
    /* 5D6A0 8006CEA0 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5D6A4 8006CEA4 21280000 */  addu       $a1, $zero, $zero
    /* 5D6A8 8006CEA8 0430B602 */  sllv       $a2, $s6, $s5
  .L8006CEAC:
    /* 5D6AC 8006CEAC 5804828F */  lw         $v0, %gp_rel(D_800A3524)($gp)
    /* 5D6B0 8006CEB0 00000000 */  nop
    /* 5D6B4 8006CEB4 21184500 */  addu       $v1, $v0, $a1
    /* 5D6B8 8006CEB8 1A006290 */  lbu        $v0, 0x1A($v1)
    /* 5D6BC 8006CEBC 00000000 */  nop
    /* 5D6C0 8006CEC0 24204600 */  and        $a0, $v0, $a2
    /* 5D6C4 8006CEC4 17006290 */  lbu        $v0, 0x17($v1)
    /* 5D6C8 8006CEC8 03002016 */  bnez       $s1, .L8006CED8
    /* 5D6CC 8006CECC 00000000 */   nop
    /* 5D6D0 8006CED0 B7B30108 */  j          .L8006CEDC
    /* 5D6D4 8006CED4 F0004230 */   andi      $v0, $v0, 0xF0
  .L8006CED8:
    /* 5D6D8 8006CED8 0F004230 */  andi       $v0, $v0, 0xF
  .L8006CEDC:
    /* 5D6DC 8006CEDC 21104400 */  addu       $v0, $v0, $a0
    /* 5D6E0 8006CEE0 170062A0 */  sb         $v0, 0x17($v1)
    /* 5D6E4 8006CEE4 0100A524 */  addiu      $a1, $a1, 0x1
    /* 5D6E8 8006CEE8 0300A228 */  slti       $v0, $a1, 0x3
    /* 5D6EC 8006CEEC EFFF4014 */  bnez       $v0, .L8006CEAC
    /* 5D6F0 8006CEF0 00000000 */   nop
    /* 5D6F4 8006CEF4 DCB30108 */  j          .L8006CF70
    /* 5D6F8 8006CEF8 0400B526 */   addiu     $s5, $s5, 0x4
  .L8006CEFC:
    /* 5D6FC 8006CEFC 0000838E */  lw         $v1, 0x0($s4)
    /* 5D700 8006CF00 04104202 */  sllv       $v0, $v0, $s2
    /* 5D704 8006CF04 24186200 */  and        $v1, $v1, $v0
    /* 5D708 8006CF08 18006010 */  beqz       $v1, .L8006CF6C
    /* 5D70C 8006CF0C 01000424 */   addiu     $a0, $zero, 0x1
    /* 5D710 8006CF10 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5D714 8006CF14 9471010C */  jal        func_8005C650
    /* 5D718 8006CF18 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5D71C 8006CF1C 21280000 */  addu       $a1, $zero, $zero
    /* 5D720 8006CF20 0430B602 */  sllv       $a2, $s6, $s5
  .L8006CF24:
    /* 5D724 8006CF24 5804828F */  lw         $v0, %gp_rel(D_800A3524)($gp)
    /* 5D728 8006CF28 00000000 */  nop
    /* 5D72C 8006CF2C 21184500 */  addu       $v1, $v0, $a1
    /* 5D730 8006CF30 1D006290 */  lbu        $v0, 0x1D($v1)
    /* 5D734 8006CF34 00000000 */  nop
    /* 5D738 8006CF38 24204600 */  and        $a0, $v0, $a2
    /* 5D73C 8006CF3C 17006290 */  lbu        $v0, 0x17($v1)
    /* 5D740 8006CF40 03002016 */  bnez       $s1, .L8006CF50
    /* 5D744 8006CF44 00000000 */   nop
    /* 5D748 8006CF48 D5B30108 */  j          .L8006CF54
    /* 5D74C 8006CF4C F0004230 */   andi      $v0, $v0, 0xF0
  .L8006CF50:
    /* 5D750 8006CF50 0F004230 */  andi       $v0, $v0, 0xF
  .L8006CF54:
    /* 5D754 8006CF54 21104400 */  addu       $v0, $v0, $a0
    /* 5D758 8006CF58 170062A0 */  sb         $v0, 0x17($v1)
    /* 5D75C 8006CF5C 0100A524 */  addiu      $a1, $a1, 0x1
    /* 5D760 8006CF60 0300A228 */  slti       $v0, $a1, 0x3
    /* 5D764 8006CF64 EFFF4014 */  bnez       $v0, .L8006CF24
    /* 5D768 8006CF68 00000000 */   nop
  .L8006CF6C:
    /* 5D76C 8006CF6C 0400B526 */  addiu      $s5, $s5, 0x4
  .L8006CF70:
    /* 5D770 8006CF70 10005226 */  addiu      $s2, $s2, 0x10
    /* 5D774 8006CF74 01003126 */  addiu      $s1, $s1, 0x1
    /* 5D778 8006CF78 0200222A */  slti       $v0, $s1, 0x2
    /* 5D77C 8006CF7C 75FF4014 */  bnez       $v0, .L8006CD54
    /* 5D780 8006CF80 02007326 */   addiu     $s3, $s3, 0x2
    /* 5D784 8006CF84 2110C003 */  addu       $v0, $fp, $zero
    /* 5D788 8006CF88 4400BF8F */  lw         $ra, 0x44($sp)
    /* 5D78C 8006CF8C 4000BE8F */  lw         $fp, 0x40($sp)
    /* 5D790 8006CF90 3C00B78F */  lw         $s7, 0x3C($sp)
    /* 5D794 8006CF94 3800B68F */  lw         $s6, 0x38($sp)
    /* 5D798 8006CF98 3400B58F */  lw         $s5, 0x34($sp)
    /* 5D79C 8006CF9C 3000B48F */  lw         $s4, 0x30($sp)
    /* 5D7A0 8006CFA0 2C00B38F */  lw         $s3, 0x2C($sp)
    /* 5D7A4 8006CFA4 2800B28F */  lw         $s2, 0x28($sp)
    /* 5D7A8 8006CFA8 2400B18F */  lw         $s1, 0x24($sp)
    /* 5D7AC 8006CFAC 2000B08F */  lw         $s0, 0x20($sp)
    /* 5D7B0 8006CFB0 4800BD27 */  addiu      $sp, $sp, 0x48
    /* 5D7B4 8006CFB4 0800E003 */  jr         $ra
    /* 5D7B8 8006CFB8 00000000 */   nop
endlabel func_8006CCC8
