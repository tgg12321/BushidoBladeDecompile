glabel func_8005C650
    /* 4CE50 8005C650 21380000 */  addu       $a3, $zero, $zero
    /* 4CE54 8005C654 80200400 */  sll        $a0, $a0, 2
    /* 4CE58 8005C658 0A80023C */  lui        $v0, %hi(D_8009AA70)
    /* 4CE5C 8005C65C 70AA4224 */  addiu      $v0, $v0, %lo(D_8009AA70)
    /* 4CE60 8005C660 21208200 */  addu       $a0, $a0, $v0
    /* 4CE64 8005C664 00140700 */  sll        $v0, $a3, 16
  .L8005C668:
    /* 4CE68 8005C668 431B0200 */  sra        $v1, $v0, 13
    /* 4CE6C 8005C66C 0F80013C */  lui        $at, %hi(D_800EFB78)
    /* 4CE70 8005C670 21082300 */  addu       $at, $at, $v1
    /* 4CE74 8005C674 78FB228C */  lw         $v0, %lo(D_800EFB78)($at)
    /* 4CE78 8005C678 00000000 */  nop
    /* 4CE7C 8005C67C 0C004014 */  bnez       $v0, .L8005C6B0
    /* 4CE80 8005C680 0100E224 */   addiu     $v0, $a3, 0x1
    /* 4CE84 8005C684 0F80013C */  lui        $at, %hi(D_800EFB78)
    /* 4CE88 8005C688 21082300 */  addu       $at, $at, $v1
    /* 4CE8C 8005C68C 78FB24AC */  sw         $a0, %lo(D_800EFB78)($at)
    /* 4CE90 8005C690 0F80013C */  lui        $at, %hi(D_800EFB7C)
    /* 4CE94 8005C694 21082300 */  addu       $at, $at, $v1
    /* 4CE98 8005C698 7CFB25A0 */  sb         $a1, %lo(D_800EFB7C)($at)
    /* 4CE9C 8005C69C 0F80013C */  lui        $at, %hi(D_800EFB7D)
    /* 4CEA0 8005C6A0 21082300 */  addu       $at, $at, $v1
    /* 4CEA4 8005C6A4 7DFB26A0 */  sb         $a2, %lo(D_800EFB7D)($at)
    /* 4CEA8 8005C6A8 B2710108 */  j          .L8005C6C8
    /* 4CEAC 8005C6AC 00000000 */   nop
  .L8005C6B0:
    /* 4CEB0 8005C6B0 21384000 */  addu       $a3, $v0, $zero
    /* 4CEB4 8005C6B4 00140200 */  sll        $v0, $v0, 16
    /* 4CEB8 8005C6B8 03140200 */  sra        $v0, $v0, 16
    /* 4CEBC 8005C6BC 18004228 */  slti       $v0, $v0, 0x18
    /* 4CEC0 8005C6C0 E9FF4014 */  bnez       $v0, .L8005C668
    /* 4CEC4 8005C6C4 00140700 */   sll       $v0, $a3, 16
  .L8005C6C8:
    /* 4CEC8 8005C6C8 0800E003 */  jr         $ra
    /* 4CECC 8005C6CC 00000000 */   nop
endlabel func_8005C650
