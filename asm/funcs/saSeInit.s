glabel saSeInit
    /* 1F038 8002E838 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 1F03C 8002E83C 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 1F040 8002E840 21888000 */  addu       $s1, $a0, $zero
    /* 1F044 8002E844 2000BFAF */  sw         $ra, 0x20($sp)
    /* 1F048 8002E848 1800B0AF */  sw         $s0, 0x18($sp)
    /* 1F04C 8002E84C 6400228E */  lw         $v0, 0x64($s1)
    /* 1F050 8002E850 6000238E */  lw         $v1, 0x60($s1)
    /* 1F054 8002E854 0000428C */  lw         $v0, 0x0($v0)
    /* 1F058 8002E858 0000638C */  lw         $v1, 0x0($v1)
    /* 1F05C 8002E85C 00000000 */  nop
    /* 1F060 8002E860 23104300 */  subu       $v0, $v0, $v1
    /* 1F064 8002E864 A80022AE */  sw         $v0, 0xA8($s1)
    /* 1F068 8002E868 6400228E */  lw         $v0, 0x64($s1)
    /* 1F06C 8002E86C 6000238E */  lw         $v1, 0x60($s1)
    /* 1F070 8002E870 0400428C */  lw         $v0, 0x4($v0)
    /* 1F074 8002E874 0400638C */  lw         $v1, 0x4($v1)
    /* 1F078 8002E878 00000000 */  nop
    /* 1F07C 8002E87C 23104300 */  subu       $v0, $v0, $v1
    /* 1F080 8002E880 AC0022AE */  sw         $v0, 0xAC($s1)
    /* 1F084 8002E884 6400228E */  lw         $v0, 0x64($s1)
    /* 1F088 8002E888 6000238E */  lw         $v1, 0x60($s1)
    /* 1F08C 8002E88C 0800458C */  lw         $a1, 0x8($v0)
    /* 1F090 8002E890 0800628C */  lw         $v0, 0x8($v1)
    /* 1F094 8002E894 A800248E */  lw         $a0, 0xA8($s1)
    /* 1F098 8002E898 2328A200 */  subu       $a1, $a1, $v0
    /* 1F09C 8002E89C 57FF010C */  jal        func_8007FD5C
    /* 1F0A0 8002E8A0 B00025AE */   sw        $a1, 0xB0($s1)
    /* 1F0A4 8002E8A4 A800238E */  lw         $v1, 0xA8($s1)
    /* 1F0A8 8002E8A8 00000000 */  nop
    /* 1F0AC 8002E8AC 18006300 */  mult       $v1, $v1
    /* 1F0B0 8002E8B0 12200000 */  mflo       $a0
    /* 1F0B4 8002E8B4 B000238E */  lw         $v1, 0xB0($s1)
    /* 1F0B8 8002E8B8 00000000 */  nop
    /* 1F0BC 8002E8BC 18006300 */  mult       $v1, $v1
    /* 1F0C0 8002E8C0 00080324 */  addiu      $v1, $zero, 0x800
    /* 1F0C4 8002E8C4 23186200 */  subu       $v1, $v1, $v0
    /* 1F0C8 8002E8C8 12380000 */  mflo       $a3
    /* 1F0CC 8002E8CC 21208700 */  addu       $a0, $a0, $a3
    /* 1F0D0 8002E8D0 0004822C */  sltiu      $v0, $a0, 0x400
    /* 1F0D4 8002E8D4 06004010 */  beqz       $v0, .L8002E8F0
    /* 1F0D8 8002E8D8 FA0023A6 */   sh        $v1, 0xFA($s1)
    /* 1F0DC 8002E8DC 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1F0E0 8002E8E0 21082400 */  addu       $at, $at, $a0
    /* 1F0E4 8002E8E4 18D12290 */  lbu        $v0, %lo(D_8008D118)($at)
    /* 1F0E8 8002E8E8 53BA0008 */  j          .L8002E94C
    /* 1F0EC 8002E8EC C2280200 */   srl       $a1, $v0, 3
  .L8002E8F0:
    /* 1F0F0 8002E8F0 09008004 */  bltz       $a0, .L8002E918
    /* 1F0F4 8002E8F4 21180000 */   addu      $v1, $zero, $zero
    /* 1F0F8 8002E8F8 21608000 */  addu       $t4, $a0, $zero
    /* 1F0FC 8002E8FC 00F08C48 */  mtc2       $t4, $30 /* handwritten instruction */
    /* 1F100 8002E900 00000000 */  nop
    /* 1F104 8002E904 00000000 */  nop
    /* 1F108 8002E908 1000A227 */  addiu      $v0, $sp, 0x10
    /* 1F10C 8002E90C 21604000 */  addu       $t4, $v0, $zero
    /* 1F110 8002E910 00009FE9 */  swc2       $31, 0x0($t4)
    /* 1F114 8002E914 1000A38F */  lw         $v1, 0x10($sp)
  .L8002E918:
    /* 1F118 8002E918 FEFF0224 */  addiu      $v0, $zero, -0x2
    /* 1F11C 8002E91C 24106200 */  and        $v0, $v1, $v0
    /* 1F120 8002E920 16000324 */  addiu      $v1, $zero, 0x16
    /* 1F124 8002E924 23186200 */  subu       $v1, $v1, $v0
    /* 1F128 8002E928 06106400 */  srlv       $v0, $a0, $v1
    /* 1F12C 8002E92C 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1F130 8002E930 21082200 */  addu       $at, $at, $v0
    /* 1F134 8002E934 18D12490 */  lbu        $a0, %lo(D_8008D118)($at)
    /* 1F138 8002E938 42180300 */  srl        $v1, $v1, 1
    /* 1F13C 8002E93C 13000224 */  addiu      $v0, $zero, 0x13
    /* 1F140 8002E940 23104300 */  subu       $v0, $v0, $v1
    /* 1F144 8002E944 00240400 */  sll        $a0, $a0, 16
    /* 1F148 8002E948 06284400 */  srlv       $a1, $a0, $v0
  .L8002E94C:
    /* 1F14C 8002E94C AC00248E */  lw         $a0, 0xAC($s1)
    /* 1F150 8002E950 57FF010C */  jal        func_8007FD5C
    /* 1F154 8002E954 D8003026 */   addiu     $s0, $s1, 0xD8
    /* 1F158 8002E958 21280002 */  addu       $a1, $s0, $zero
    /* 1F15C 8002E95C 00080324 */  addiu      $v1, $zero, 0x800
    /* 1F160 8002E960 23186200 */  subu       $v1, $v1, $v0
    /* 1F164 8002E964 FA002486 */  lh         $a0, 0xFA($s1)
    /* 1F168 8002E968 00100224 */  addiu      $v0, $zero, 0x1000
    /* 1F16C 8002E96C F80023A6 */  sh         $v1, 0xF8($s1)
    /* 1F170 8002E970 D80022A6 */  sh         $v0, 0xD8($s1)
    /* 1F174 8002E974 DA0020A6 */  sh         $zero, 0xDA($s1)
    /* 1F178 8002E978 DC0020A6 */  sh         $zero, 0xDC($s1)
    /* 1F17C 8002E97C DE0020A6 */  sh         $zero, 0xDE($s1)
    /* 1F180 8002E980 E00022A6 */  sh         $v0, 0xE0($s1)
    /* 1F184 8002E984 E20020A6 */  sh         $zero, 0xE2($s1)
    /* 1F188 8002E988 E40020A6 */  sh         $zero, 0xE4($s1)
    /* 1F18C 8002E98C E60020A6 */  sh         $zero, 0xE6($s1)
    /* 1F190 8002E990 87FE010C */  jal        func_8007FA1C
    /* 1F194 8002E994 E80022A6 */   sh        $v0, 0xE8($s1)
    /* 1F198 8002E998 F8002486 */  lh         $a0, 0xF8($s1)
    /* 1F19C 8002E99C 1FFE010C */  jal        func_8007F87C
    /* 1F1A0 8002E9A0 21280002 */   addu      $a1, $s0, $zero
    /* 1F1A4 8002E9A4 21600002 */  addu       $t4, $s0, $zero
    /* 1F1A8 8002E9A8 00008D8D */  lw         $t5, 0x0($t4)
    /* 1F1AC 8002E9AC 04008E8D */  lw         $t6, 0x4($t4)
    /* 1F1B0 8002E9B0 0000CD48 */  ctc2       $t5, $0 /* handwritten instruction */
    /* 1F1B4 8002E9B4 0008CE48 */  ctc2       $t6, $1 /* handwritten instruction */
    /* 1F1B8 8002E9B8 08008D8D */  lw         $t5, 0x8($t4)
    /* 1F1BC 8002E9BC 0C008E8D */  lw         $t6, 0xC($t4)
    /* 1F1C0 8002E9C0 10008F8D */  lw         $t7, 0x10($t4)
    /* 1F1C4 8002E9C4 0010CD48 */  ctc2       $t5, $2 /* handwritten instruction */
    /* 1F1C8 8002E9C8 0018CE48 */  ctc2       $t6, $3 /* handwritten instruction */
    /* 1F1CC 8002E9CC 0020CF48 */  ctc2       $t7, $4 /* handwritten instruction */
    /* 1F1D0 8002E9D0 A8002226 */  addiu      $v0, $s1, 0xA8
    /* 1F1D4 8002E9D4 21604000 */  addu       $t4, $v0, $zero
    /* 1F1D8 8002E9D8 04008E95 */  lhu        $t6, 0x4($t4)
    /* 1F1DC 8002E9DC 00008D95 */  lhu        $t5, 0x0($t4)
    /* 1F1E0 8002E9E0 00740E00 */  sll        $t6, $t6, 16
    /* 1F1E4 8002E9E4 2568AE01 */  or         $t5, $t5, $t6
    /* 1F1E8 8002E9E8 00008D48 */  mtc2       $t5, $0 /* handwritten instruction */
    /* 1F1EC 8002E9EC 080081C9 */  lwc2       $1, 0x8($t4)
    /* 1F1F0 8002E9F0 00000000 */  nop
    /* 1F1F4 8002E9F4 00000000 */  nop
    /* 1F1F8 8002E9F8 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 1F1FC 8002E9FC 21604000 */  addu       $t4, $v0, $zero
    /* 1F200 8002EA00 000099E9 */  swc2       $25, 0x0($t4)
    /* 1F204 8002EA04 04009AE9 */  swc2       $26, 0x4($t4) /* handwritten instruction */
    /* 1F208 8002EA08 08009BE9 */  swc2       $27, 0x8($t4) /* handwritten instruction */
    /* 1F20C 8002EA0C 2000BF8F */  lw         $ra, 0x20($sp)
    /* 1F210 8002EA10 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 1F214 8002EA14 1800B08F */  lw         $s0, 0x18($sp)
    /* 1F218 8002EA18 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 1F21C 8002EA1C 0800E003 */  jr         $ra
    /* 1F220 8002EA20 00000000 */   nop
endlabel saSeInit
