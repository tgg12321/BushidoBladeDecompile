glabel func_8003BEA8
    /* 2C6A8 8003BEA8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2C6AC 8003BEAC 1000B0AF */  sw         $s0, 0x10($sp)
    /* 2C6B0 8003BEB0 1400BFAF */  sw         $ra, 0x14($sp)
    /* 2C6B4 8003BEB4 2CE2010C */  jal        func_800788B0
    /* 2C6B8 8003BEB8 21800000 */   addu      $s0, $zero, $zero
    /* 2C6BC 8003BEBC 1080033C */  lui        $v1, %hi(D_80102794)
    /* 2C6C0 8003BEC0 9427638C */  lw         $v1, %lo(D_80102794)($v1)
    /* 2C6C4 8003BEC4 00000000 */  nop
    /* 2C6C8 8003BEC8 40006330 */  andi       $v1, $v1, 0x40
    /* 2C6CC 8003BECC 02006010 */  beqz       $v1, .L8003BED8
    /* 2C6D0 8003BED0 2B100200 */   sltu      $v0, $zero, $v0
    /* 2C6D4 8003BED4 01000224 */  addiu      $v0, $zero, 0x1
  .L8003BED8:
    /* 2C6D8 8003BED8 35004010 */  beqz       $v0, .L8003BFB0
    /* 2C6DC 8003BEDC 00000000 */   nop
    /* 2C6E0 8003BEE0 B0DC000C */  jal        func_800372C0
    /* 2C6E4 8003BEE4 00000000 */   nop
    /* 2C6E8 8003BEE8 1080043C */  lui        $a0, %hi(D_80106A50)
    /* 2C6EC 8003BEEC 506A8424 */  addiu      $a0, $a0, %lo(D_80106A50)
    /* 2C6F0 8003BEF0 0000838C */  lw         $v1, 0x0($a0)
    /* 2C6F4 8003BEF4 0A80023C */  lui        $v0, %hi(D_800A37A4)
    /* 2C6F8 8003BEF8 A437428C */  lw         $v0, %lo(D_800A37A4)($v0)
    /* 2C6FC 8003BEFC 00000000 */  nop
    /* 2C700 8003BF00 25106200 */  or         $v0, $v1, $v0
    /* 2C704 8003BF04 03004310 */  beq        $v0, $v1, .L8003BF14
    /* 2C708 8003BF08 00000000 */   nop
    /* 2C70C 8003BF0C 000082AC */  sw         $v0, 0x0($a0)
    /* 2C710 8003BF10 01001024 */  addiu      $s0, $zero, 0x1
  .L8003BF14:
    /* 2C714 8003BF14 1080033C */  lui        $v1, %hi(D_80101ED2)
    /* 2C718 8003BF18 D21E6394 */  lhu        $v1, %lo(D_80101ED2)($v1)
    /* 2C71C 8003BF1C 00000000 */  nop
    /* 2C720 8003BF20 0200622C */  sltiu      $v0, $v1, 0x2
    /* 2C724 8003BF24 0A004014 */  bnez       $v0, .L8003BF50
    /* 2C728 8003BF28 00140300 */   sll       $v0, $v1, 16
    /* 2C72C 8003BF2C 03240200 */  sra        $a0, $v0, 16
    /* 2C730 8003BF30 02000224 */  addiu      $v0, $zero, 0x2
    /* 2C734 8003BF34 06008210 */  beq        $a0, $v0, .L8003BF50
    /* 2C738 8003BF38 F4FF6224 */   addiu     $v0, $v1, -0xC
    /* 2C73C 8003BF3C 0200422C */  sltiu      $v0, $v0, 0x2
    /* 2C740 8003BF40 03004014 */  bnez       $v0, .L8003BF50
    /* 2C744 8003BF44 0E000224 */   addiu     $v0, $zero, 0xE
    /* 2C748 8003BF48 14008214 */  bne        $a0, $v0, .L8003BF9C
    /* 2C74C 8003BF4C 00000000 */   nop
  .L8003BF50:
    /* 2C750 8003BF50 1080033C */  lui        $v1, %hi(D_80101ED2)
    /* 2C754 8003BF54 D21E6384 */  lh         $v1, %lo(D_80101ED2)($v1)
    /* 2C758 8003BF58 00000000 */  nop
    /* 2C75C 8003BF5C 0C006228 */  slti       $v0, $v1, 0xC
    /* 2C760 8003BF60 02004014 */  bnez       $v0, .L8003BF6C
    /* 2C764 8003BF64 00000000 */   nop
    /* 2C768 8003BF68 F7FF6324 */  addiu      $v1, $v1, -0x9
  .L8003BF6C:
    /* 2C76C 8003BF6C 1080053C */  lui        $a1, %hi(D_80106A54)
    /* 2C770 8003BF70 546AA524 */  addiu      $a1, $a1, %lo(D_80106A54)
    /* 2C774 8003BF74 0000A490 */  lbu        $a0, 0x0($a1)
    /* 2C778 8003BF78 00000000 */  nop
    /* 2C77C 8003BF7C 07106400 */  srav       $v0, $a0, $v1
    /* 2C780 8003BF80 01004230 */  andi       $v0, $v0, 0x1
    /* 2C784 8003BF84 05004014 */  bnez       $v0, .L8003BF9C
    /* 2C788 8003BF88 01000224 */   addiu     $v0, $zero, 0x1
    /* 2C78C 8003BF8C 04106200 */  sllv       $v0, $v0, $v1
    /* 2C790 8003BF90 25108200 */  or         $v0, $a0, $v0
    /* 2C794 8003BF94 0000A2A0 */  sb         $v0, 0x0($a1)
    /* 2C798 8003BF98 01001024 */  addiu      $s0, $zero, 0x1
  .L8003BF9C:
    /* 2C79C 8003BF9C 02000016 */  bnez       $s0, .L8003BFA8
    /* 2C7A0 8003BFA0 1A000224 */   addiu     $v0, $zero, 0x1A
    /* 2C7A4 8003BFA4 08000224 */  addiu      $v0, $zero, 0x8
  .L8003BFA8:
    /* 2C7A8 8003BFA8 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2C7AC 8003BFAC 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
  .L8003BFB0:
    /* 2C7B0 8003BFB0 1400BF8F */  lw         $ra, 0x14($sp)
    /* 2C7B4 8003BFB4 1000B08F */  lw         $s0, 0x10($sp)
    /* 2C7B8 8003BFB8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2C7BC 8003BFBC 0800E003 */  jr         $ra
    /* 2C7C0 8003BFC0 00000000 */   nop
endlabel func_8003BEA8
