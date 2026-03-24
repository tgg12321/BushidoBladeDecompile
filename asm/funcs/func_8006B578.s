glabel func_8006B578
    /* 5BD78 8006B578 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 5BD7C 8006B57C 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 5BD80 8006B580 2188A000 */  addu       $s1, $a1, $zero
    /* 5BD84 8006B584 2000B2AF */  sw         $s2, 0x20($sp)
    /* 5BD88 8006B588 21900000 */  addu       $s2, $zero, $zero
    /* 5BD8C 8006B58C 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5BD90 8006B590 21280000 */  addu       $a1, $zero, $zero
    /* 5BD94 8006B594 3004868F */  lw         $a2, %gp_rel(D_800A34FC)($gp)
    /* 5BD98 8006B598 0A80073C */  lui        $a3, %hi(D_800A350C)
    /* 5BD9C 8006B59C 0C35E724 */  addiu      $a3, $a3, %lo(D_800A350C)
    /* 5BDA0 8006B5A0 2400BFAF */  sw         $ra, 0x24($sp)
    /* 5BDA4 8006B5A4 1800B0AF */  sw         $s0, 0x18($sp)
    /* 5BDA8 8006B5A8 0000238E */  lw         $v1, 0x0($s1)
    /* 5BDAC 8006B5AC 0C00C624 */  addiu      $a2, $a2, 0xC
    /* 5BDB0 8006B5B0 FFFF6230 */  andi       $v0, $v1, 0xFFFF
    /* 5BDB4 8006B5B4 021C0300 */  srl        $v1, $v1, 16
    /* 5BDB8 8006B5B8 25104300 */  or         $v0, $v0, $v1
    /* 5BDBC 8006B5BC B0A4010C */  jal        func_800692C0
    /* 5BDC0 8006B5C0 1000A2AF */   sw        $v0, 0x10($sp)
    /* 5BDC4 8006B5C4 21804000 */  addu       $s0, $v0, $zero
    /* 5BDC8 8006B5C8 031C1000 */  sra        $v1, $s0, 16
    /* 5BDCC 8006B5CC 01000224 */  addiu      $v0, $zero, 0x1
    /* 5BDD0 8006B5D0 05006210 */  beq        $v1, $v0, .L8006B5E8
    /* 5BDD4 8006B5D4 02000224 */   addiu     $v0, $zero, 0x2
    /* 5BDD8 8006B5D8 12006210 */  beq        $v1, $v0, .L8006B624
    /* 5BDDC 8006B5DC 00000000 */   nop
    /* 5BDE0 8006B5E0 A1AD0108 */  j          .L8006B684
    /* 5BDE4 8006B5E4 00000000 */   nop
  .L8006B5E8:
    /* 5BDE8 8006B5E8 2C04848F */  lw         $a0, %gp_rel(D_800A34F8)($gp)
    /* 5BDEC 8006B5EC 00140224 */  addiu      $v0, $zero, 0x1400
    /* 5BDF0 8006B5F0 001C8330 */  andi       $v1, $a0, 0x1C00
    /* 5BDF4 8006B5F4 06006214 */  bne        $v1, $v0, .L8006B610
    /* 5BDF8 8006B5F8 FFE30324 */   addiu     $v1, $zero, -0x1C01
    /* 5BDFC 8006B5FC FFE30224 */  addiu      $v0, $zero, -0x1C01
    /* 5BE00 8006B600 24108200 */  and        $v0, $a0, $v0
    /* 5BE04 8006B604 2C0482AF */  sw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5BE08 8006B608 9DAD0108 */  j          .L8006B674
    /* 5BE0C 8006B60C 21200000 */   addu      $a0, $zero, $zero
  .L8006B610:
    /* 5BE10 8006B610 24188300 */  and        $v1, $a0, $v1
    /* 5BE14 8006B614 82120400 */  srl        $v0, $a0, 10
    /* 5BE18 8006B618 07004230 */  andi       $v0, $v0, 0x7
    /* 5BE1C 8006B61C 98AD0108 */  j          .L8006B660
    /* 5BE20 8006B620 01004224 */   addiu     $v0, $v0, 0x1
  .L8006B624:
    /* 5BE24 8006B624 2C04848F */  lw         $a0, %gp_rel(D_800A34F8)($gp)
    /* 5BE28 8006B628 00000000 */  nop
    /* 5BE2C 8006B62C 001C8230 */  andi       $v0, $a0, 0x1C00
    /* 5BE30 8006B630 07004014 */  bnez       $v0, .L8006B650
    /* 5BE34 8006B634 FFE30324 */   addiu     $v1, $zero, -0x1C01
    /* 5BE38 8006B638 FFE30224 */  addiu      $v0, $zero, -0x1C01
    /* 5BE3C 8006B63C 24108200 */  and        $v0, $a0, $v0
    /* 5BE40 8006B640 00144234 */  ori        $v0, $v0, 0x1400
    /* 5BE44 8006B644 2C0482AF */  sw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5BE48 8006B648 9DAD0108 */  j          .L8006B674
    /* 5BE4C 8006B64C 21200000 */   addu      $a0, $zero, $zero
  .L8006B650:
    /* 5BE50 8006B650 24188300 */  and        $v1, $a0, $v1
    /* 5BE54 8006B654 82120400 */  srl        $v0, $a0, 10
    /* 5BE58 8006B658 07004230 */  andi       $v0, $v0, 0x7
    /* 5BE5C 8006B65C FFFF4224 */  addiu      $v0, $v0, -0x1
  .L8006B660:
    /* 5BE60 8006B660 07004230 */  andi       $v0, $v0, 0x7
    /* 5BE64 8006B664 80120200 */  sll        $v0, $v0, 10
    /* 5BE68 8006B668 25186200 */  or         $v1, $v1, $v0
    /* 5BE6C 8006B66C 2C0483AF */  sw         $v1, %gp_rel(D_800A34F8)($gp)
    /* 5BE70 8006B670 21200000 */  addu       $a0, $zero, $zero
  .L8006B674:
    /* 5BE74 8006B674 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5BE78 8006B678 9471010C */  jal        func_8005C650
    /* 5BE7C 8006B67C 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5BE80 8006B680 480480AF */  sw         $zero, %gp_rel(D_800A3514)($gp)
  .L8006B684:
    /* 5BE84 8006B684 2C04828F */  lw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5BE88 8006B688 00000000 */  nop
    /* 5BE8C 8006B68C 82120200 */  srl        $v0, $v0, 10
    /* 5BE90 8006B690 07004330 */  andi       $v1, $v0, 0x7
    /* 5BE94 8006B694 0600622C */  sltiu      $v0, $v1, 0x6
    /* 5BE98 8006B698 6D004010 */  beqz       $v0, .L8006B850
    /* 5BE9C 8006B69C 80100300 */   sll       $v0, $v1, 2
    /* 5BEA0 8006B6A0 0180013C */  lui        $at, %hi(jtbl_80015988)
    /* 5BEA4 8006B6A4 21082200 */  addu       $at, $at, $v0
    /* 5BEA8 8006B6A8 8859228C */  lw         $v0, %lo(jtbl_80015988)($at)
    /* 5BEAC 8006B6AC 00000000 */  nop
    /* 5BEB0 8006B6B0 08004000 */  jr         $v0
    /* 5BEB4 8006B6B4 00000000 */   nop
  jlabel .L8006B6B8
    /* 5BEB8 8006B6B8 FF000232 */  andi       $v0, $s0, 0xFF
    /* 5BEBC 8006B6BC 28004010 */  beqz       $v0, .L8006B760
    /* 5BEC0 8006B6C0 21200000 */   addu      $a0, $zero, $zero
    /* 5BEC4 8006B6C4 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5BEC8 8006B6C8 5804828F */  lw         $v0, %gp_rel(D_800A3524)($gp)
    /* 5BECC 8006B6CC 7F000624 */  addiu      $a2, $zero, 0x7F
    /* 5BED0 8006B6D0 2000438C */  lw         $v1, 0x20($v0)
    /* 5BED4 8006B6D4 FEFF0724 */  addiu      $a3, $zero, -0x2
    /* 5BED8 8006B6D8 24386700 */  and        $a3, $v1, $a3
    /* 5BEDC 8006B6DC 01006330 */  andi       $v1, $v1, 0x1
    /* 5BEE0 8006B6E0 D5AD0108 */  j          .L8006B754
    /* 5BEE4 8006B6E4 01006338 */   xori      $v1, $v1, 0x1
  jlabel .L8006B6E8
    /* 5BEE8 8006B6E8 FF000232 */  andi       $v0, $s0, 0xFF
    /* 5BEEC 8006B6EC 1C004010 */  beqz       $v0, .L8006B760
    /* 5BEF0 8006B6F0 21200000 */   addu      $a0, $zero, $zero
    /* 5BEF4 8006B6F4 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5BEF8 8006B6F8 5804828F */  lw         $v0, %gp_rel(D_800A3524)($gp)
    /* 5BEFC 8006B6FC 7F000624 */  addiu      $a2, $zero, 0x7F
    /* 5BF00 8006B700 2000438C */  lw         $v1, 0x20($v0)
    /* 5BF04 8006B704 FDFF0724 */  addiu      $a3, $zero, -0x3
    /* 5BF08 8006B708 24386700 */  and        $a3, $v1, $a3
    /* 5BF0C 8006B70C 42180300 */  srl        $v1, $v1, 1
    /* 5BF10 8006B710 01006330 */  andi       $v1, $v1, 0x1
    /* 5BF14 8006B714 01006338 */  xori       $v1, $v1, 0x1
    /* 5BF18 8006B718 D5AD0108 */  j          .L8006B754
    /* 5BF1C 8006B71C 40180300 */   sll       $v1, $v1, 1
  jlabel .L8006B720
    /* 5BF20 8006B720 FF000232 */  andi       $v0, $s0, 0xFF
    /* 5BF24 8006B724 0E004010 */  beqz       $v0, .L8006B760
    /* 5BF28 8006B728 21200000 */   addu      $a0, $zero, $zero
    /* 5BF2C 8006B72C 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5BF30 8006B730 5804828F */  lw         $v0, %gp_rel(D_800A3524)($gp)
    /* 5BF34 8006B734 7F000624 */  addiu      $a2, $zero, 0x7F
    /* 5BF38 8006B738 2000438C */  lw         $v1, 0x20($v0)
    /* 5BF3C 8006B73C FBFF0724 */  addiu      $a3, $zero, -0x5
    /* 5BF40 8006B740 24386700 */  and        $a3, $v1, $a3
    /* 5BF44 8006B744 82180300 */  srl        $v1, $v1, 2
    /* 5BF48 8006B748 01006330 */  andi       $v1, $v1, 0x1
    /* 5BF4C 8006B74C 01006338 */  xori       $v1, $v1, 0x1
    /* 5BF50 8006B750 80180300 */  sll        $v1, $v1, 2
  .L8006B754:
    /* 5BF54 8006B754 2538E300 */  or         $a3, $a3, $v1
    /* 5BF58 8006B758 9471010C */  jal        func_8005C650
    /* 5BF5C 8006B75C 200047AC */   sw        $a3, 0x20($v0)
  .L8006B760:
    /* 5BF60 8006B760 4000033C */  lui        $v1, (0x400040 >> 16)
    /* 5BF64 8006B764 0000228E */  lw         $v0, 0x0($s1)
    /* 5BF68 8006B768 40006334 */  ori        $v1, $v1, (0x400040 & 0xFFFF)
    /* 5BF6C 8006B76C 24104300 */  and        $v0, $v0, $v1
    /* 5BF70 8006B770 37004010 */  beqz       $v0, .L8006B850
    /* 5BF74 8006B774 01000424 */   addiu     $a0, $zero, 0x1
    /* 5BF78 8006B778 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5BF7C 8006B77C 9471010C */  jal        func_8005C650
    /* 5BF80 8006B780 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5BF84 8006B784 2C04828F */  lw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5BF88 8006B788 FFE30324 */  addiu      $v1, $zero, -0x1C01
    /* 5BF8C 8006B78C 24184300 */  and        $v1, $v0, $v1
    /* 5BF90 8006B790 82120200 */  srl        $v0, $v0, 10
    /* 5BF94 8006B794 07004230 */  andi       $v0, $v0, 0x7
    /* 5BF98 8006B798 01004224 */  addiu      $v0, $v0, 0x1
    /* 5BF9C 8006B79C 07004230 */  andi       $v0, $v0, 0x7
    /* 5BFA0 8006B7A0 80120200 */  sll        $v0, $v0, 10
    /* 5BFA4 8006B7A4 25186200 */  or         $v1, $v1, $v0
    /* 5BFA8 8006B7A8 2C0483AF */  sw         $v1, %gp_rel(D_800A34F8)($gp)
    /* 5BFAC 8006B7AC 15AE0108 */  j          .L8006B854
    /* 5BFB0 8006B7B0 1000033C */   lui       $v1, (0x100010 >> 16)
  jlabel .L8006B7B4
    /* 5BFB4 8006B7B4 4000033C */  lui        $v1, (0x400040 >> 16)
    /* 5BFB8 8006B7B8 0000228E */  lw         $v0, 0x0($s1)
    /* 5BFBC 8006B7BC 40006334 */  ori        $v1, $v1, (0x400040 & 0xFFFF)
    /* 5BFC0 8006B7C0 24104300 */  and        $v0, $v0, $v1
    /* 5BFC4 8006B7C4 22004010 */  beqz       $v0, .L8006B850
    /* 5BFC8 8006B7C8 01000424 */   addiu     $a0, $zero, 0x1
    /* 5BFCC 8006B7CC 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5BFD0 8006B7D0 9471010C */  jal        func_8005C650
    /* 5BFD4 8006B7D4 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5BFD8 8006B7D8 FFFF033C */  lui        $v1, (0xFFFF1FFF >> 16)
    /* 5BFDC 8006B7DC FF1F6334 */  ori        $v1, $v1, (0xFFFF1FFF & 0xFFFF)
    /* 5BFE0 8006B7E0 2C04828F */  lw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5BFE4 8006B7E4 00000000 */  nop
    /* 5BFE8 8006B7E8 24104300 */  and        $v0, $v0, $v1
    /* 5BFEC 8006B7EC 00404234 */  ori        $v0, $v0, 0x4000
    /* 5BFF0 8006B7F0 2C0482AF */  sw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5BFF4 8006B7F4 14AE0108 */  j          .L8006B850
    /* 5BFF8 8006B7F8 02001224 */   addiu     $s2, $zero, 0x2
  jlabel .L8006B7FC
    /* 5BFFC 8006B7FC 4000033C */  lui        $v1, (0x400040 >> 16)
    /* 5C000 8006B800 0000228E */  lw         $v0, 0x0($s1)
    /* 5C004 8006B804 40006334 */  ori        $v1, $v1, (0x400040 & 0xFFFF)
    /* 5C008 8006B808 24104300 */  and        $v0, $v0, $v1
    /* 5C00C 8006B80C 10004010 */  beqz       $v0, .L8006B850
    /* 5C010 8006B810 01000424 */   addiu     $a0, $zero, 0x1
    /* 5C014 8006B814 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5C018 8006B818 9471010C */  jal        func_8005C650
    /* 5C01C 8006B81C 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5C020 8006B820 14AE0108 */  j          .L8006B850
    /* 5C024 8006B824 03001224 */   addiu     $s2, $zero, 0x3
  jlabel .L8006B828
    /* 5C028 8006B828 4000033C */  lui        $v1, (0x400040 >> 16)
    /* 5C02C 8006B82C 0000228E */  lw         $v0, 0x0($s1)
    /* 5C030 8006B830 40006334 */  ori        $v1, $v1, (0x400040 & 0xFFFF)
    /* 5C034 8006B834 24104300 */  and        $v0, $v0, $v1
    /* 5C038 8006B838 05004010 */  beqz       $v0, .L8006B850
    /* 5C03C 8006B83C 01000424 */   addiu     $a0, $zero, 0x1
    /* 5C040 8006B840 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5C044 8006B844 9471010C */  jal        func_8005C650
    /* 5C048 8006B848 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5C04C 8006B84C 01001224 */  addiu      $s2, $zero, 0x1
  .L8006B850:
    /* 5C050 8006B850 1000033C */  lui        $v1, (0x100010 >> 16)
  .L8006B854:
    /* 5C054 8006B854 0000228E */  lw         $v0, 0x0($s1)
    /* 5C058 8006B858 10006334 */  ori        $v1, $v1, (0x100010 & 0xFFFF)
    /* 5C05C 8006B85C 24104300 */  and        $v0, $v0, $v1
    /* 5C060 8006B860 05004010 */  beqz       $v0, .L8006B878
    /* 5C064 8006B864 02000424 */   addiu     $a0, $zero, 0x2
    /* 5C068 8006B868 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5C06C 8006B86C 9471010C */  jal        func_8005C650
    /* 5C070 8006B870 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5C074 8006B874 01001224 */  addiu      $s2, $zero, 0x1
  .L8006B878:
    /* 5C078 8006B878 21104002 */  addu       $v0, $s2, $zero
    /* 5C07C 8006B87C 2400BF8F */  lw         $ra, 0x24($sp)
    /* 5C080 8006B880 2000B28F */  lw         $s2, 0x20($sp)
    /* 5C084 8006B884 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 5C088 8006B888 1800B08F */  lw         $s0, 0x18($sp)
    /* 5C08C 8006B88C 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 5C090 8006B890 0800E003 */  jr         $ra
    /* 5C094 8006B894 00000000 */   nop
endlabel func_8006B578
