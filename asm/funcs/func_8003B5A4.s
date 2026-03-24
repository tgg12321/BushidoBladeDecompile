glabel func_8003B5A4
    /* 2BDA4 8003B5A4 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 2BDA8 8003B5A8 1800BFAF */  sw         $ra, 0x18($sp)
    /* 2BDAC 8003B5AC 1400B1AF */  sw         $s1, 0x14($sp)
    /* 2BDB0 8003B5B0 6B6D010C */  jal        func_8005B5AC
    /* 2BDB4 8003B5B4 1000B0AF */   sw        $s0, 0x10($sp)
    /* 2BDB8 8003B5B8 21800000 */  addu       $s0, $zero, $zero
    /* 2BDBC 8003B5BC 1080113C */  lui        $s1, %hi(D_8010277D)
    /* 2BDC0 8003B5C0 7D273126 */  addiu      $s1, $s1, %lo(D_8010277D)
  .L8003B5C4:
    /* 2BDC4 8003B5C4 0A80023C */  lui        $v0, %hi(D_800A3844)
    /* 2BDC8 8003B5C8 4438428C */  lw         $v0, %lo(D_800A3844)($v0)
    /* 2BDCC 8003B5CC 00000000 */  nop
    /* 2BDD0 8003B5D0 01004324 */  addiu      $v1, $v0, 0x1
    /* 2BDD4 8003B5D4 0A80013C */  lui        $at, %hi(D_800A3844)
    /* 2BDD8 8003B5D8 443823AC */  sw         $v1, %lo(D_800A3844)($at)
    /* 2BDDC 8003B5DC 00004390 */  lbu        $v1, 0x0($v0)
    /* 2BDE0 8003B5E0 00000000 */  nop
    /* 2BDE4 8003B5E4 1500622C */  sltiu      $v0, $v1, 0x15
    /* 2BDE8 8003B5E8 99004010 */  beqz       $v0, .L8003B850
    /* 2BDEC 8003B5EC 80100300 */   sll       $v0, $v1, 2
    /* 2BDF0 8003B5F0 0180013C */  lui        $at, %hi(jtbl_80010D1C)
    /* 2BDF4 8003B5F4 21082200 */  addu       $at, $at, $v0
    /* 2BDF8 8003B5F8 1C0D228C */  lw         $v0, %lo(jtbl_80010D1C)($at)
    /* 2BDFC 8003B5FC 00000000 */  nop
    /* 2BE00 8003B600 08004000 */  jr         $v0
    /* 2BE04 8003B604 00000000 */   nop
  jlabel .L8003B608
    /* 2BE08 8003B608 0A80043C */  lui        $a0, %hi(D_800A3844)
    /* 2BE0C 8003B60C 4438848C */  lw         $a0, %lo(D_800A3844)($a0)
    /* 2BE10 8003B610 E9EC000C */  jal        func_8003B3A4
    /* 2BE14 8003B614 00000000 */   nop
    /* 2BE18 8003B618 8CED0008 */  j          .L8003B630
    /* 2BE1C 8003B61C 00000000 */   nop
  jlabel .L8003B620
    /* 2BE20 8003B620 0A80043C */  lui        $a0, %hi(D_800A3844)
    /* 2BE24 8003B624 4438848C */  lw         $a0, %lo(D_800A3844)($a0)
    /* 2BE28 8003B628 21ED000C */  jal        func_8003B484
    /* 2BE2C 8003B62C 00000000 */   nop
  .L8003B630:
    /* 2BE30 8003B630 0A80033C */  lui        $v1, %hi(D_800A3844)
    /* 2BE34 8003B634 4438638C */  lw         $v1, %lo(D_800A3844)($v1)
    /* 2BE38 8003B638 00000000 */  nop
    /* 2BE3C 8003B63C 21186200 */  addu       $v1, $v1, $v0
    /* 2BE40 8003B640 0A80013C */  lui        $at, %hi(D_800A3844)
    /* 2BE44 8003B644 443823AC */  sw         $v1, %lo(D_800A3844)($at)
    /* 2BE48 8003B648 14EE0008 */  j          .L8003B850
    /* 2BE4C 8003B64C 00000000 */   nop
  jlabel .L8003B650
    /* 2BE50 8003B650 0A80043C */  lui        $a0, %hi(D_800A3844)
    /* 2BE54 8003B654 4438848C */  lw         $a0, %lo(D_800A3844)($a0)
    /* 2BE58 8003B658 00000000 */  nop
    /* 2BE5C 8003B65C 01008224 */  addiu      $v0, $a0, 0x1
    /* 2BE60 8003B660 0A80013C */  lui        $at, %hi(D_800A3844)
    /* 2BE64 8003B664 443822AC */  sw         $v0, %lo(D_800A3844)($at)
    /* 2BE68 8003B668 00008390 */  lbu        $v1, 0x0($a0)
    /* 2BE6C 8003B66C 02008224 */  addiu      $v0, $a0, 0x2
    /* 2BE70 8003B670 0A80013C */  lui        $at, %hi(D_800A3844)
    /* 2BE74 8003B674 443822AC */  sw         $v0, %lo(D_800A3844)($at)
    /* 2BE78 8003B678 000023A2 */  sb         $v1, 0x0($s1)
    /* 2BE7C 8003B67C 01008290 */  lbu        $v0, 0x1($a0)
    /* 2BE80 8003B680 001E0300 */  sll        $v1, $v1, 24
    /* 2BE84 8003B684 020022A2 */  sb         $v0, 0x2($s1)
    /* 2BE88 8003B688 0A80023C */  lui        $v0, %hi(D_800A3915)
    /* 2BE8C 8003B68C 15394290 */  lbu        $v0, %lo(D_800A3915)($v0)
    /* 2BE90 8003B690 031E0300 */  sra        $v1, $v1, 24
    /* 2BE94 8003B694 05006214 */  bne        $v1, $v0, .L8003B6AC
    /* 2BE98 8003B698 00000000 */   nop
    /* 2BE9C 8003B69C 0A80053C */  lui        $a1, %hi(D_800A36F4)
    /* 2BEA0 8003B6A0 F436A590 */  lbu        $a1, %lo(D_800A36F4)($a1)
    /* 2BEA4 8003B6A4 8105010C */  jal        func_80041604
    /* 2BEA8 8003B6A8 01000424 */   addiu     $a0, $zero, 0x1
  .L8003B6AC:
    /* 2BEAC 8003B6AC D0EB000C */  jal        func_8003AF40
    /* 2BEB0 8003B6B0 01000424 */   addiu     $a0, $zero, 0x1
    /* 2BEB4 8003B6B4 FFEB000C */  jal        func_8003AFFC
    /* 2BEB8 8003B6B8 00000000 */   nop
    /* 2BEBC 8003B6BC 14EE0008 */  j          .L8003B850
    /* 2BEC0 8003B6C0 00000000 */   nop
  jlabel .L8003B6C4
    /* 2BEC4 8003B6C4 1080043C */  lui        $a0, %hi(D_80101EC8)
    /* 2BEC8 8003B6C8 C81E8424 */  addiu      $a0, $a0, %lo(D_80101EC8)
    /* 2BECC 8003B6CC 0A80053C */  lui        $a1, %hi(D_800A3844)
    /* 2BED0 8003B6D0 4438A58C */  lw         $a1, %lo(D_800A3844)($a1)
    /* 2BED4 8003B6D4 0A80033C */  lui        $v1, %hi(D_800A37A0)
    /* 2BED8 8003B6D8 A0376390 */  lbu        $v1, %lo(D_800A37A0)($v1)
    /* 2BEDC 8003B6DC 0100A224 */  addiu      $v0, $a1, 0x1
    /* 2BEE0 8003B6E0 0A80013C */  lui        $at, %hi(D_800A3844)
    /* 2BEE4 8003B6E4 443822AC */  sw         $v0, %lo(D_800A3844)($at)
    /* 2BEE8 8003B6E8 01006224 */  addiu      $v0, $v1, 0x1
    /* 2BEEC 8003B6EC 0000A590 */  lbu        $a1, 0x0($a1)
    /* 2BEF0 8003B6F0 FF006330 */  andi       $v1, $v1, 0xFF
    /* 2BEF4 8003B6F4 0A80013C */  lui        $at, %hi(D_800A37A0)
    /* 2BEF8 8003B6F8 A03722A0 */  sb         $v0, %lo(D_800A37A0)($at)
    /* 2BEFC 8003B6FC 16000224 */  addiu      $v0, $zero, 0x16
    /* 2BF00 8003B700 0A80013C */  lui        $at, %hi(D_800A36A4)
    /* 2BF04 8003B704 A43625A4 */  sh         $a1, %lo(D_800A36A4)($at)
    /* 2BF08 8003B708 0A80013C */  lui        $at, %hi(D_800A37A8)
    /* 2BF0C 8003B70C 21082300 */  addu       $at, $at, $v1
    /* 2BF10 8003B710 A83725A0 */  sb         $a1, %lo(D_800A37A8)($at)
    /* 2BF14 8003B714 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2BF18 8003B718 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 2BF1C 8003B71C 5A89000C */  jal        func_80022568
    /* 2BF20 8003B720 01001024 */   addiu     $s0, $zero, 0x1
    /* 2BF24 8003B724 0A80013C */  lui        $at, %hi(D_800A3907)
    /* 2BF28 8003B728 073920A0 */  sb         $zero, %lo(D_800A3907)($at)
    /* 2BF2C 8003B72C 14EE0008 */  j          .L8003B850
    /* 2BF30 8003B730 00000000 */   nop
  jlabel .L8003B734
    /* 2BF34 8003B734 0A80023C */  lui        $v0, %hi(D_800A380C)
    /* 2BF38 8003B738 0C38428C */  lw         $v0, %lo(D_800A380C)($v0)
    /* 2BF3C 8003B73C 00000000 */  nop
    /* 2BF40 8003B740 0E004014 */  bnez       $v0, .L8003B77C
    /* 2BF44 8003B744 0A000224 */   addiu     $v0, $zero, 0xA
    /* 2BF48 8003B748 1080023C */  lui        $v0, %hi(D_80101ED2)
    /* 2BF4C 8003B74C D21E4284 */  lh         $v0, %lo(D_80101ED2)($v0)
    /* 2BF50 8003B750 0980013C */  lui        $at, %hi(D_8008D9EC)
    /* 2BF54 8003B754 21082200 */  addu       $at, $at, $v0
    /* 2BF58 8003B758 ECD92390 */  lbu        $v1, %lo(D_8008D9EC)($at)
    /* 2BF5C 8003B75C 12000224 */  addiu      $v0, $zero, 0x12
    /* 2BF60 8003B760 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2BF64 8003B764 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 2BF68 8003B768 2B180300 */  sltu       $v1, $zero, $v1
    /* 2BF6C 8003B76C 0A80013C */  lui        $at, %hi(D_800A38A4)
    /* 2BF70 8003B770 A43823A0 */  sb         $v1, %lo(D_800A38A4)($at)
    /* 2BF74 8003B774 14EE0008 */  j          .L8003B850
    /* 2BF78 8003B778 01001024 */   addiu     $s0, $zero, 0x1
  .L8003B77C:
    /* 2BF7C 8003B77C 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2BF80 8003B780 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 2BF84 8003B784 14EE0008 */  j          .L8003B850
    /* 2BF88 8003B788 01001024 */   addiu     $s0, $zero, 0x1
  jlabel .L8003B78C
    /* 2BF8C 8003B78C 0A80033C */  lui        $v1, %hi(D_800A3844)
    /* 2BF90 8003B790 4438638C */  lw         $v1, %lo(D_800A3844)($v1)
    /* 2BF94 8003B794 06000224 */  addiu      $v0, $zero, 0x6
    /* 2BF98 8003B798 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2BF9C 8003B79C 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 2BFA0 8003B7A0 0A80013C */  lui        $at, %hi(D_800A3878)
    /* 2BFA4 8003B7A4 783823AC */  sw         $v1, %lo(D_800A3878)($at)
    /* 2BFA8 8003B7A8 04006324 */  addiu      $v1, $v1, 0x4
    /* 2BFAC 8003B7AC 0A80013C */  lui        $at, %hi(D_800A3844)
    /* 2BFB0 8003B7B0 443823AC */  sw         $v1, %lo(D_800A3844)($at)
    /* 2BFB4 8003B7B4 14EE0008 */  j          .L8003B850
    /* 2BFB8 8003B7B8 01001024 */   addiu     $s0, $zero, 0x1
  jlabel .L8003B7BC
    /* 2BFBC 8003B7BC 0A80023C */  lui        $v0, %hi(D_800A3844)
    /* 2BFC0 8003B7C0 4438428C */  lw         $v0, %lo(D_800A3844)($v0)
    /* 2BFC4 8003B7C4 0A80013C */  lui        $at, %hi(D_800A3894)
    /* 2BFC8 8003B7C8 943822AC */  sw         $v0, %lo(D_800A3894)($at)
    /* 2BFCC 8003B7CC 1D004224 */  addiu      $v0, $v0, 0x1D
    /* 2BFD0 8003B7D0 0A80013C */  lui        $at, %hi(D_800A3844)
    /* 2BFD4 8003B7D4 443822AC */  sw         $v0, %lo(D_800A3844)($at)
    /* 2BFD8 8003B7D8 B2EC000C */  jal        func_8003B2C8
    /* 2BFDC 8003B7DC 00000000 */   nop
    /* 2BFE0 8003B7E0 D0EB000C */  jal        func_8003AF40
    /* 2BFE4 8003B7E4 21200000 */   addu      $a0, $zero, $zero
    /* 2BFE8 8003B7E8 0A80043C */  lui        $a0, %hi(D_800A3894)
    /* 2BFEC 8003B7EC 9438848C */  lw         $a0, %lo(D_800A3894)($a0)
    /* 2BFF0 8003B7F0 E9EC000C */  jal        func_8003B3A4
    /* 2BFF4 8003B7F4 01008424 */   addiu     $a0, $a0, 0x1
  jlabel .L8003B7F8
    /* 2BFF8 8003B7F8 0A80023C */  lui        $v0, %hi(D_800A3907)
    /* 2BFFC 8003B7FC 07394290 */  lbu        $v0, %lo(D_800A3907)($v0)
    /* 2C000 8003B800 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2C004 8003B804 343820A4 */  sh         $zero, %lo(D_800A3834)($at)
    /* 2C008 8003B808 01004224 */  addiu      $v0, $v0, 0x1
    /* 2C00C 8003B80C 0A80013C */  lui        $at, %hi(D_800A3907)
    /* 2C010 8003B810 073922A0 */  sb         $v0, %lo(D_800A3907)($at)
    /* 2C014 8003B814 14EE0008 */  j          .L8003B850
    /* 2C018 8003B818 01001024 */   addiu     $s0, $zero, 0x1
  jlabel .L8003B81C
    /* 2C01C 8003B81C 0A80023C */  lui        $v0, %hi(D_800A3844)
    /* 2C020 8003B820 4438428C */  lw         $v0, %lo(D_800A3844)($v0)
    /* 2C024 8003B824 0A80013C */  lui        $at, %hi(D_800A385C)
    /* 2C028 8003B828 5C3822AC */  sw         $v0, %lo(D_800A385C)($at)
    /* 2C02C 8003B82C 0C004224 */  addiu      $v0, $v0, 0xC
    /* 2C030 8003B830 0A80013C */  lui        $at, %hi(D_800A3844)
    /* 2C034 8003B834 443822AC */  sw         $v0, %lo(D_800A3844)($at)
    /* 2C038 8003B838 37ED000C */  jal        func_8003B4DC
    /* 2C03C 8003B83C 01001024 */   addiu     $s0, $zero, 0x1
    /* 2C040 8003B840 5BED000C */  jal        func_8003B56C
    /* 2C044 8003B844 01000424 */   addiu     $a0, $zero, 0x1
    /* 2C048 8003B848 0A80013C */  lui        $at, %hi(D_800A38BA)
    /* 2C04C 8003B84C BA3820A4 */  sh         $zero, %lo(D_800A38BA)($at)
  jlabel .L8003B850
    /* 2C050 8003B850 5CFF0012 */  beqz       $s0, .L8003B5C4
    /* 2C054 8003B854 00000000 */   nop
    /* 2C058 8003B858 1800BF8F */  lw         $ra, 0x18($sp)
    /* 2C05C 8003B85C 1400B18F */  lw         $s1, 0x14($sp)
    /* 2C060 8003B860 1000B08F */  lw         $s0, 0x10($sp)
    /* 2C064 8003B864 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 2C068 8003B868 0800E003 */  jr         $ra
    /* 2C06C 8003B86C 00000000 */   nop
endlabel func_8003B5A4
