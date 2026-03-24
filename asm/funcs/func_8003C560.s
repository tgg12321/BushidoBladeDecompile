glabel func_8003C560
    /* 2CD60 8003C560 0A80023C */  lui        $v0, %hi(D_800A37B8)
    /* 2CD64 8003C564 B837428C */  lw         $v0, %lo(D_800A37B8)($v0)
    /* 2CD68 8003C568 0A80033C */  lui        $v1, %hi(D_800A382D)
    /* 2CD6C 8003C56C 2D386390 */  lbu        $v1, %lo(D_800A382D)($v1)
    /* 2CD70 8003C570 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2CD74 8003C574 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2CD78 8003C578 01004424 */  addiu      $a0, $v0, 0x1
    /* 2CD7C 8003C57C 02000224 */  addiu      $v0, $zero, 0x2
    /* 2CD80 8003C580 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* 2CD84 8003C584 B83724AC */  sw         $a0, %lo(D_800A37B8)($at)
    /* 2CD88 8003C588 05006214 */  bne        $v1, $v0, .L8003C5A0
    /* 2CD8C 8003C58C 1E000224 */   addiu     $v0, $zero, 0x1E
    /* 2CD90 8003C590 20008214 */  bne        $a0, $v0, .L8003C614
    /* 2CD94 8003C594 A4000424 */   addiu     $a0, $zero, 0xA4
    /* 2CD98 8003C598 83F10008 */  j          .L8003C60C
    /* 2CD9C 8003C59C 7F000524 */   addiu     $a1, $zero, 0x7F
  .L8003C5A0:
    /* 2CDA0 8003C5A0 14008214 */  bne        $a0, $v0, .L8003C5F4
    /* 2CDA4 8003C5A4 00110300 */   sll       $v0, $v1, 4
    /* 2CDA8 8003C5A8 21104300 */  addu       $v0, $v0, $v1
    /* 2CDAC 8003C5AC 80100200 */  sll        $v0, $v0, 2
    /* 2CDB0 8003C5B0 21104300 */  addu       $v0, $v0, $v1
    /* 2CDB4 8003C5B4 80100200 */  sll        $v0, $v0, 2
    /* 2CDB8 8003C5B8 23104300 */  subu       $v0, $v0, $v1
    /* 2CDBC 8003C5BC 80100200 */  sll        $v0, $v0, 2
    /* 2CDC0 8003C5C0 1080013C */  lui        $at, %hi(D_80101ED2)
    /* 2CDC4 8003C5C4 21082200 */  addu       $at, $at, $v0
    /* 2CDC8 8003C5C8 D21E2284 */  lh         $v0, %lo(D_80101ED2)($at)
    /* 2CDCC 8003C5CC 0980013C */  lui        $at, %hi(D_8008D9EC)
    /* 2CDD0 8003C5D0 21082200 */  addu       $at, $at, $v0
    /* 2CDD4 8003C5D4 ECD92290 */  lbu        $v0, %lo(D_8008D9EC)($at)
    /* 2CDD8 8003C5D8 00000000 */  nop
    /* 2CDDC 8003C5DC 02004010 */  beqz       $v0, .L8003C5E8
    /* 2CDE0 8003C5E0 A7000424 */   addiu     $a0, $zero, 0xA7
    /* 2CDE4 8003C5E4 A8000424 */  addiu      $a0, $zero, 0xA8
  .L8003C5E8:
    /* 2CDE8 8003C5E8 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 2CDEC 8003C5EC 9471010C */  jal        func_8005C650
    /* 2CDF0 8003C5F0 7F000624 */   addiu     $a2, $zero, 0x7F
  .L8003C5F4:
    /* 2CDF4 8003C5F4 0A80033C */  lui        $v1, %hi(D_800A37B8)
    /* 2CDF8 8003C5F8 B837638C */  lw         $v1, %lo(D_800A37B8)($v1)
    /* 2CDFC 8003C5FC 43000224 */  addiu      $v0, $zero, 0x43
    /* 2CE00 8003C600 04006214 */  bne        $v1, $v0, .L8003C614
    /* 2CE04 8003C604 A9000424 */   addiu     $a0, $zero, 0xA9
    /* 2CE08 8003C608 7F000524 */  addiu      $a1, $zero, 0x7F
  .L8003C60C:
    /* 2CE0C 8003C60C 9471010C */  jal        func_8005C650
    /* 2CE10 8003C610 7F000624 */   addiu     $a2, $zero, 0x7F
  .L8003C614:
    /* 2CE14 8003C614 0A80043C */  lui        $a0, %hi(D_800A3784)
    /* 2CE18 8003C618 8437848C */  lw         $a0, %lo(D_800A3784)($a0)
    /* 2CE1C 8003C61C 0A80053C */  lui        $a1, %hi(D_800A38B4)
    /* 2CE20 8003C620 B438A58C */  lw         $a1, %lo(D_800A38B4)($a1)
    /* 2CE24 8003C624 5379010C */  jal        func_8005E54C
    /* 2CE28 8003C628 01000624 */   addiu     $a2, $zero, 0x1
    /* 2CE2C 8003C62C 02004104 */  bgez       $v0, .L8003C638
    /* 2CE30 8003C630 4000053C */   lui       $a1, (0x400040 >> 16)
    /* 2CE34 8003C634 03004224 */  addiu      $v0, $v0, 0x3
  .L8003C638:
    /* 2CE38 8003C638 4000A534 */  ori        $a1, $a1, (0x400040 & 0xFFFF)
    /* 2CE3C 8003C63C 83180200 */  sra        $v1, $v0, 2
    /* 2CE40 8003C640 80180300 */  sll        $v1, $v1, 2
    /* 2CE44 8003C644 0A80023C */  lui        $v0, %hi(D_800A38B4)
    /* 2CE48 8003C648 B438428C */  lw         $v0, %lo(D_800A38B4)($v0)
    /* 2CE4C 8003C64C 1080043C */  lui        $a0, %hi(D_80102794)
    /* 2CE50 8003C650 9427848C */  lw         $a0, %lo(D_80102794)($a0)
    /* 2CE54 8003C654 21104300 */  addu       $v0, $v0, $v1
    /* 2CE58 8003C658 24208500 */  and        $a0, $a0, $a1
    /* 2CE5C 8003C65C 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 2CE60 8003C660 B43822AC */  sw         $v0, %lo(D_800A38B4)($at)
    /* 2CE64 8003C664 07008014 */  bnez       $a0, .L8003C684
    /* 2CE68 8003C668 00000000 */   nop
    /* 2CE6C 8003C66C 0A80023C */  lui        $v0, %hi(D_800A37B8)
    /* 2CE70 8003C670 B837428C */  lw         $v0, %lo(D_800A37B8)($v0)
    /* 2CE74 8003C674 00000000 */  nop
    /* 2CE78 8003C678 F1004228 */  slti       $v0, $v0, 0xF1
    /* 2CE7C 8003C67C 21004014 */  bnez       $v0, .L8003C704
    /* 2CE80 8003C680 00000000 */   nop
  .L8003C684:
    /* 2CE84 8003C684 0A80033C */  lui        $v1, %hi(D_800A382D)
    /* 2CE88 8003C688 2D386390 */  lbu        $v1, %lo(D_800A382D)($v1)
    /* 2CE8C 8003C68C 02000224 */  addiu      $v0, $zero, 0x2
    /* 2CE90 8003C690 1A006210 */  beq        $v1, $v0, .L8003C6FC
    /* 2CE94 8003C694 18000224 */   addiu     $v0, $zero, 0x18
    /* 2CE98 8003C698 B0DC000C */  jal        func_800372C0
    /* 2CE9C 8003C69C 00000000 */   nop
    /* 2CEA0 8003C6A0 0A80033C */  lui        $v1, %hi(D_800A382D)
    /* 2CEA4 8003C6A4 2D386390 */  lbu        $v1, %lo(D_800A382D)($v1)
    /* 2CEA8 8003C6A8 00000000 */  nop
    /* 2CEAC 8003C6AC 00110300 */  sll        $v0, $v1, 4
    /* 2CEB0 8003C6B0 21104300 */  addu       $v0, $v0, $v1
    /* 2CEB4 8003C6B4 80100200 */  sll        $v0, $v0, 2
    /* 2CEB8 8003C6B8 21104300 */  addu       $v0, $v0, $v1
    /* 2CEBC 8003C6BC 80100200 */  sll        $v0, $v0, 2
    /* 2CEC0 8003C6C0 23104300 */  subu       $v0, $v0, $v1
    /* 2CEC4 8003C6C4 80100200 */  sll        $v0, $v0, 2
    /* 2CEC8 8003C6C8 1080013C */  lui        $at, %hi(D_80101ED2)
    /* 2CECC 8003C6CC 21082200 */  addu       $at, $at, $v0
    /* 2CED0 8003C6D0 D21E2284 */  lh         $v0, %lo(D_80101ED2)($at)
    /* 2CED4 8003C6D4 0980013C */  lui        $at, %hi(D_8008D9EC)
    /* 2CED8 8003C6D8 21082200 */  addu       $at, $at, $v0
    /* 2CEDC 8003C6DC ECD92290 */  lbu        $v0, %lo(D_8008D9EC)($at)
    /* 2CEE0 8003C6E0 00000000 */  nop
    /* 2CEE4 8003C6E4 02004010 */  beqz       $v0, .L8003C6F0
    /* 2CEE8 8003C6E8 04000324 */   addiu     $v1, $zero, 0x4
    /* 2CEEC 8003C6EC 05000324 */  addiu      $v1, $zero, 0x5
  .L8003C6F0:
    /* 2CEF0 8003C6F0 12000224 */  addiu      $v0, $zero, 0x12
    /* 2CEF4 8003C6F4 0A80013C */  lui        $at, %hi(D_800A38A4)
    /* 2CEF8 8003C6F8 A43823A0 */  sb         $v1, %lo(D_800A38A4)($at)
  .L8003C6FC:
    /* 2CEFC 8003C6FC 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2CF00 8003C700 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
  .L8003C704:
    /* 2CF04 8003C704 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2CF08 8003C708 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2CF0C 8003C70C 0800E003 */  jr         $ra
    /* 2CF10 8003C710 00000000 */   nop
endlabel func_8003C560
