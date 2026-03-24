glabel func_8002EBDC
    /* 1F3DC 8002EBDC C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 1F3E0 8002EBE0 2400B3AF */  sw         $s3, 0x24($sp)
    /* 1F3E4 8002EBE4 21988000 */  addu       $s3, $a0, $zero
    /* 1F3E8 8002EBE8 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 1F3EC 8002EBEC 2188A000 */  addu       $s1, $a1, $zero
    /* 1F3F0 8002EBF0 3000B6AF */  sw         $s6, 0x30($sp)
    /* 1F3F4 8002EBF4 21B0C000 */  addu       $s6, $a2, $zero
    /* 1F3F8 8002EBF8 2800B4AF */  sw         $s4, 0x28($sp)
    /* 1F3FC 8002EBFC 3400BFAF */  sw         $ra, 0x34($sp)
    /* 1F400 8002EC00 2C00B5AF */  sw         $s5, 0x2C($sp)
    /* 1F404 8002EC04 2000B2AF */  sw         $s2, 0x20($sp)
    /* 1F408 8002EC08 1800B0AF */  sw         $s0, 0x18($sp)
    /* 1F40C 8002EC0C 00002486 */  lh         $a0, 0x0($s1)
    /* 1F410 8002EC10 04002586 */  lh         $a1, 0x4($s1)
    /* 1F414 8002EC14 4800B58F */  lw         $s5, 0x48($sp)
    /* 1F418 8002EC18 57FF010C */  jal        func_8007FD5C
    /* 1F41C 8002EC1C 21A0E000 */   addu      $s4, $a3, $zero
    /* 1F420 8002EC20 00080324 */  addiu      $v1, $zero, 0x800
    /* 1F424 8002EC24 23186200 */  subu       $v1, $v1, $v0
    /* 1F428 8002EC28 801F013C */  lui        $at, (0x1F8003B2 >> 16)
    /* 1F42C 8002EC2C B20323A4 */  sh         $v1, (0x1F8003B2 & 0xFFFF)($at)
    /* 1F430 8002EC30 00002286 */  lh         $v0, 0x0($s1)
    /* 1F434 8002EC34 00000000 */  nop
    /* 1F438 8002EC38 18004200 */  mult       $v0, $v0
    /* 1F43C 8002EC3C 12180000 */  mflo       $v1
    /* 1F440 8002EC40 04002286 */  lh         $v0, 0x4($s1)
    /* 1F444 8002EC44 00000000 */  nop
    /* 1F448 8002EC48 18004200 */  mult       $v0, $v0
    /* 1F44C 8002EC4C 801F103C */  lui        $s0, (0x1F8002B8 >> 16)
    /* 1F450 8002EC50 12480000 */  mflo       $t1
    /* 1F454 8002EC54 21206900 */  addu       $a0, $v1, $t1
    /* 1F458 8002EC58 0004822C */  sltiu      $v0, $a0, 0x400
    /* 1F45C 8002EC5C 06004010 */  beqz       $v0, .L8002EC78
    /* 1F460 8002EC60 B8021036 */   ori       $s0, $s0, (0x1F8002B8 & 0xFFFF)
    /* 1F464 8002EC64 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1F468 8002EC68 21082400 */  addu       $at, $at, $a0
    /* 1F46C 8002EC6C 18D12290 */  lbu        $v0, %lo(D_8008D118)($at)
    /* 1F470 8002EC70 35BB0008 */  j          .L8002ECD4
    /* 1F474 8002EC74 C2280200 */   srl       $a1, $v0, 3
  .L8002EC78:
    /* 1F478 8002EC78 09008004 */  bltz       $a0, .L8002ECA0
    /* 1F47C 8002EC7C 21180000 */   addu      $v1, $zero, $zero
    /* 1F480 8002EC80 21608000 */  addu       $t4, $a0, $zero
    /* 1F484 8002EC84 00F08C48 */  mtc2       $t4, $30 /* handwritten instruction */
    /* 1F488 8002EC88 00000000 */  nop
    /* 1F48C 8002EC8C 00000000 */  nop
    /* 1F490 8002EC90 1000A227 */  addiu      $v0, $sp, 0x10
    /* 1F494 8002EC94 21604000 */  addu       $t4, $v0, $zero
    /* 1F498 8002EC98 00009FE9 */  swc2       $31, 0x0($t4)
    /* 1F49C 8002EC9C 1000A38F */  lw         $v1, 0x10($sp)
  .L8002ECA0:
    /* 1F4A0 8002ECA0 FEFF0224 */  addiu      $v0, $zero, -0x2
    /* 1F4A4 8002ECA4 24106200 */  and        $v0, $v1, $v0
    /* 1F4A8 8002ECA8 16000324 */  addiu      $v1, $zero, 0x16
    /* 1F4AC 8002ECAC 23186200 */  subu       $v1, $v1, $v0
    /* 1F4B0 8002ECB0 06106400 */  srlv       $v0, $a0, $v1
    /* 1F4B4 8002ECB4 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1F4B8 8002ECB8 21082200 */  addu       $at, $at, $v0
    /* 1F4BC 8002ECBC 18D12490 */  lbu        $a0, %lo(D_8008D118)($at)
    /* 1F4C0 8002ECC0 42180300 */  srl        $v1, $v1, 1
    /* 1F4C4 8002ECC4 13000224 */  addiu      $v0, $zero, 0x13
    /* 1F4C8 8002ECC8 23104300 */  subu       $v0, $v0, $v1
    /* 1F4CC 8002ECCC 00240400 */  sll        $a0, $a0, 16
    /* 1F4D0 8002ECD0 06284400 */  srlv       $a1, $a0, $v0
  .L8002ECD4:
    /* 1F4D4 8002ECD4 02002486 */  lh         $a0, 0x2($s1)
    /* 1F4D8 8002ECD8 57FF010C */  jal        func_8007FD5C
    /* 1F4DC 8002ECDC D8001126 */   addiu     $s1, $s0, 0xD8
    /* 1F4E0 8002ECE0 21282002 */  addu       $a1, $s1, $zero
    /* 1F4E4 8002ECE4 00080324 */  addiu      $v1, $zero, 0x800
    /* 1F4E8 8002ECE8 23186200 */  subu       $v1, $v1, $v0
    /* 1F4EC 8002ECEC FA000486 */  lh         $a0, 0xFA($s0)
    /* 1F4F0 8002ECF0 00101224 */  addiu      $s2, $zero, 0x1000
    /* 1F4F4 8002ECF4 F80003A6 */  sh         $v1, 0xF8($s0)
    /* 1F4F8 8002ECF8 D80012A6 */  sh         $s2, 0xD8($s0)
    /* 1F4FC 8002ECFC DA0000A6 */  sh         $zero, 0xDA($s0)
    /* 1F500 8002ED00 DC0000A6 */  sh         $zero, 0xDC($s0)
    /* 1F504 8002ED04 DE0000A6 */  sh         $zero, 0xDE($s0)
    /* 1F508 8002ED08 E00012A6 */  sh         $s2, 0xE0($s0)
    /* 1F50C 8002ED0C E20000A6 */  sh         $zero, 0xE2($s0)
    /* 1F510 8002ED10 E40000A6 */  sh         $zero, 0xE4($s0)
    /* 1F514 8002ED14 E60000A6 */  sh         $zero, 0xE6($s0)
    /* 1F518 8002ED18 87FE010C */  jal        func_8007FA1C
    /* 1F51C 8002ED1C E80012A6 */   sh        $s2, 0xE8($s0)
    /* 1F520 8002ED20 F8000486 */  lh         $a0, 0xF8($s0)
    /* 1F524 8002ED24 1FFE010C */  jal        func_8007F87C
    /* 1F528 8002ED28 21282002 */   addu      $a1, $s1, $zero
    /* 1F52C 8002ED2C 21602002 */  addu       $t4, $s1, $zero
    /* 1F530 8002ED30 00008D8D */  lw         $t5, 0x0($t4)
    /* 1F534 8002ED34 04008E8D */  lw         $t6, 0x4($t4)
    /* 1F538 8002ED38 0000CD48 */  ctc2       $t5, $0 /* handwritten instruction */
    /* 1F53C 8002ED3C 0008CE48 */  ctc2       $t6, $1 /* handwritten instruction */
    /* 1F540 8002ED40 08008D8D */  lw         $t5, 0x8($t4)
    /* 1F544 8002ED44 0C008E8D */  lw         $t6, 0xC($t4)
    /* 1F548 8002ED48 10008F8D */  lw         $t7, 0x10($t4)
    /* 1F54C 8002ED4C 0010CD48 */  ctc2       $t5, $2 /* handwritten instruction */
    /* 1F550 8002ED50 0018CE48 */  ctc2       $t6, $3 /* handwritten instruction */
    /* 1F554 8002ED54 0020CF48 */  ctc2       $t7, $4 /* handwritten instruction */
    /* 1F558 8002ED58 21606002 */  addu       $t4, $s3, $zero
    /* 1F55C 8002ED5C 04008E95 */  lhu        $t6, 0x4($t4)
    /* 1F560 8002ED60 00008D95 */  lhu        $t5, 0x0($t4)
    /* 1F564 8002ED64 00740E00 */  sll        $t6, $t6, 16
    /* 1F568 8002ED68 2568AE01 */  or         $t5, $t5, $t6
    /* 1F56C 8002ED6C 00008D48 */  mtc2       $t5, $0 /* handwritten instruction */
    /* 1F570 8002ED70 080081C9 */  lwc2       $1, 0x8($t4)
    /* 1F574 8002ED74 00000000 */  nop
    /* 1F578 8002ED78 00000000 */  nop
    /* 1F57C 8002ED7C 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 1F580 8002ED80 A8001326 */  addiu      $s3, $s0, 0xA8
    /* 1F584 8002ED84 21606002 */  addu       $t4, $s3, $zero
    /* 1F588 8002ED88 000099E9 */  swc2       $25, 0x0($t4)
    /* 1F58C 8002ED8C 04009AE9 */  swc2       $26, 0x4($t4) /* handwritten instruction */
    /* 1F590 8002ED90 08009BE9 */  swc2       $27, 0x8($t4) /* handwritten instruction */
    /* 1F594 8002ED94 B000028E */  lw         $v0, 0xB0($s0)
    /* 1F598 8002ED98 00000000 */  nop
    /* 1F59C 8002ED9C 18005400 */  mult       $v0, $s4
    /* 1F5A0 8002EDA0 12380000 */  mflo       $a3
    /* 1F5A4 8002EDA4 0200E104 */  bgez       $a3, .L8002EDB0
    /* 1F5A8 8002EDA8 00000000 */   nop
    /* 1F5AC 8002EDAC FF00E724 */  addiu      $a3, $a3, 0xFF
  .L8002EDB0:
    /* 1F5B0 8002EDB0 A800028E */  lw         $v0, 0xA8($s0)
    /* 1F5B4 8002EDB4 00000000 */  nop
    /* 1F5B8 8002EDB8 18005500 */  mult       $v0, $s5
    /* 1F5BC 8002EDBC 03120700 */  sra        $v0, $a3, 8
    /* 1F5C0 8002EDC0 12180000 */  mflo       $v1
    /* 1F5C4 8002EDC4 02006104 */  bgez       $v1, .L8002EDD0
    /* 1F5C8 8002EDC8 B00002AE */   sw        $v0, 0xB0($s0)
    /* 1F5CC 8002EDCC FF006324 */  addiu      $v1, $v1, 0xFF
  .L8002EDD0:
    /* 1F5D0 8002EDD0 AC00028E */  lw         $v0, 0xAC($s0)
    /* 1F5D4 8002EDD4 00000000 */  nop
    /* 1F5D8 8002EDD8 18005500 */  mult       $v0, $s5
    /* 1F5DC 8002EDDC 03120300 */  sra        $v0, $v1, 8
    /* 1F5E0 8002EDE0 12300000 */  mflo       $a2
    /* 1F5E4 8002EDE4 0200C104 */  bgez       $a2, .L8002EDF0
    /* 1F5E8 8002EDE8 A80002AE */   sw        $v0, 0xA8($s0)
    /* 1F5EC 8002EDEC FF00C624 */  addiu      $a2, $a2, 0xFF
  .L8002EDF0:
    /* 1F5F0 8002EDF0 21282002 */  addu       $a1, $s1, $zero
    /* 1F5F4 8002EDF4 F8000486 */  lh         $a0, 0xF8($s0)
    /* 1F5F8 8002EDF8 03120600 */  sra        $v0, $a2, 8
    /* 1F5FC 8002EDFC AC0002AE */  sw         $v0, 0xAC($s0)
    /* 1F600 8002EE00 D80012A6 */  sh         $s2, 0xD8($s0)
    /* 1F604 8002EE04 DA0000A6 */  sh         $zero, 0xDA($s0)
    /* 1F608 8002EE08 DC0000A6 */  sh         $zero, 0xDC($s0)
    /* 1F60C 8002EE0C DE0000A6 */  sh         $zero, 0xDE($s0)
    /* 1F610 8002EE10 E00012A6 */  sh         $s2, 0xE0($s0)
    /* 1F614 8002EE14 E20000A6 */  sh         $zero, 0xE2($s0)
    /* 1F618 8002EE18 E40000A6 */  sh         $zero, 0xE4($s0)
    /* 1F61C 8002EE1C E60000A6 */  sh         $zero, 0xE6($s0)
    /* 1F620 8002EE20 E80012A6 */  sh         $s2, 0xE8($s0)
    /* 1F624 8002EE24 1FFE010C */  jal        func_8007F87C
    /* 1F628 8002EE28 23200400 */   negu      $a0, $a0
    /* 1F62C 8002EE2C FA000486 */  lh         $a0, 0xFA($s0)
    /* 1F630 8002EE30 21282002 */  addu       $a1, $s1, $zero
    /* 1F634 8002EE34 87FE010C */  jal        func_8007FA1C
    /* 1F638 8002EE38 23200400 */   negu      $a0, $a0
    /* 1F63C 8002EE3C 21602002 */  addu       $t4, $s1, $zero
    /* 1F640 8002EE40 00008D8D */  lw         $t5, 0x0($t4)
    /* 1F644 8002EE44 04008E8D */  lw         $t6, 0x4($t4)
    /* 1F648 8002EE48 0000CD48 */  ctc2       $t5, $0 /* handwritten instruction */
    /* 1F64C 8002EE4C 0008CE48 */  ctc2       $t6, $1 /* handwritten instruction */
    /* 1F650 8002EE50 08008D8D */  lw         $t5, 0x8($t4)
    /* 1F654 8002EE54 0C008E8D */  lw         $t6, 0xC($t4)
    /* 1F658 8002EE58 10008F8D */  lw         $t7, 0x10($t4)
    /* 1F65C 8002EE5C 0010CD48 */  ctc2       $t5, $2 /* handwritten instruction */
    /* 1F660 8002EE60 0018CE48 */  ctc2       $t6, $3 /* handwritten instruction */
    /* 1F664 8002EE64 0020CF48 */  ctc2       $t7, $4 /* handwritten instruction */
    /* 1F668 8002EE68 21606002 */  addu       $t4, $s3, $zero
    /* 1F66C 8002EE6C 04008E95 */  lhu        $t6, 0x4($t4)
    /* 1F670 8002EE70 00008D95 */  lhu        $t5, 0x0($t4)
    /* 1F674 8002EE74 00740E00 */  sll        $t6, $t6, 16
    /* 1F678 8002EE78 2568AE01 */  or         $t5, $t5, $t6
    /* 1F67C 8002EE7C 00008D48 */  mtc2       $t5, $0 /* handwritten instruction */
    /* 1F680 8002EE80 080081C9 */  lwc2       $1, 0x8($t4)
    /* 1F684 8002EE84 00000000 */  nop
    /* 1F688 8002EE88 00000000 */  nop
    /* 1F68C 8002EE8C 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 1F690 8002EE90 2160C002 */  addu       $t4, $s6, $zero
    /* 1F694 8002EE94 000099E9 */  swc2       $25, 0x0($t4)
    /* 1F698 8002EE98 04009AE9 */  swc2       $26, 0x4($t4) /* handwritten instruction */
    /* 1F69C 8002EE9C 08009BE9 */  swc2       $27, 0x8($t4) /* handwritten instruction */
    /* 1F6A0 8002EEA0 3400BF8F */  lw         $ra, 0x34($sp)
    /* 1F6A4 8002EEA4 3000B68F */  lw         $s6, 0x30($sp)
    /* 1F6A8 8002EEA8 2C00B58F */  lw         $s5, 0x2C($sp)
    /* 1F6AC 8002EEAC 2800B48F */  lw         $s4, 0x28($sp)
    /* 1F6B0 8002EEB0 2400B38F */  lw         $s3, 0x24($sp)
    /* 1F6B4 8002EEB4 2000B28F */  lw         $s2, 0x20($sp)
    /* 1F6B8 8002EEB8 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 1F6BC 8002EEBC 1800B08F */  lw         $s0, 0x18($sp)
    /* 1F6C0 8002EEC0 3800BD27 */  addiu      $sp, $sp, 0x38
    /* 1F6C4 8002EEC4 0800E003 */  jr         $ra
    /* 1F6C8 8002EEC8 00000000 */   nop
endlabel func_8002EBDC
