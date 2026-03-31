glabel func_8003E6D8
    /* 2EED8 8003E6D8 98FFBD27 */  addiu      $sp, $sp, -0x68
    /* 2EEDC 8003E6DC 0A80023C */  lui        $v0, %hi(D_800A3708)
    /* 2EEE0 8003E6E0 0837428C */  lw         $v0, %lo(D_800A3708)($v0)
    /* 2EEE4 8003E6E4 6210043C */  lui        $a0, (0x10624DD3 >> 16)
    /* 2EEE8 8003E6E8 6400BFAF */  sw         $ra, 0x64($sp)
    /* 2EEEC 8003E6EC 6000B2AF */  sw         $s2, 0x60($sp)
    /* 2EEF0 8003E6F0 5C00B1AF */  sw         $s1, 0x5C($sp)
    /* 2EEF4 8003E6F4 5800B0AF */  sw         $s0, 0x58($sp)
    /* 2EEF8 8003E6F8 4C00438C */  lw         $v1, 0x4C($v0)
    /* 2EEFC 8003E6FC D34D8434 */  ori        $a0, $a0, (0x10624DD3 & 0xFFFF)
    /* 2EF00 8003E700 007D6324 */  addiu      $v1, $v1, 0x7D00
    /* 2EF04 8003E704 18006400 */  mult       $v1, $a0
    /* 2EF08 8003E708 5400428C */  lw         $v0, 0x54($v0)
    /* 2EF0C 8003E70C 10380000 */  mfhi       $a3
    /* 2EF10 8003E710 007D4224 */  addiu      $v0, $v0, 0x7D00
    /* 2EF14 8003E714 00000000 */  nop
    /* 2EF18 8003E718 18004400 */  mult       $v0, $a0
    /* 2EF1C 8003E71C C31F0300 */  sra        $v1, $v1, 31
    /* 2EF20 8003E720 C3890700 */  sra        $s1, $a3, 7
    /* 2EF24 8003E724 23882302 */  subu       $s1, $s1, $v1
    /* 2EF28 8003E728 C3170200 */  sra        $v0, $v0, 31
    /* 2EF2C 8003E72C 10200000 */  mfhi       $a0
    /* 2EF30 8003E730 C3810400 */  sra        $s0, $a0, 7
    /* 2EF34 8003E734 E11C010C */  jal        camera_CalcAngles
    /* 2EF38 8003E738 23800202 */   subu      $s0, $s0, $v0
    /* 2EF3C 8003E73C 0A80043C */  lui        $a0, %hi(D_800A3708)
    /* 2EF40 8003E740 0837848C */  lw         $a0, %lo(D_800A3708)($a0)
    /* 2EF44 8003E744 1000A527 */  addiu      $a1, $sp, 0x10
    /* 2EF48 8003E748 A20A010C */  jal        func_80042A88
    /* 2EF4C 8003E74C 10008424 */   addiu     $a0, $a0, 0x10
    /* 2EF50 8003E750 00100224 */  addiu      $v0, $zero, 0x1000
    /* 2EF54 8003E754 3400A2A7 */  sh         $v0, 0x34($sp)
    /* 2EF58 8003E758 1000A227 */  addiu      $v0, $sp, 0x10
    /* 2EF5C 8003E75C 3000A0A7 */  sh         $zero, 0x30($sp)
    /* 2EF60 8003E760 3200A0A7 */  sh         $zero, 0x32($sp)
    /* 2EF64 8003E764 21604000 */  addu       $t4, $v0, $zero
    /* 2EF68 8003E768 00008D8D */  lw         $t5, 0x0($t4)
    /* 2EF6C 8003E76C 04008E8D */  lw         $t6, 0x4($t4)
    /* 2EF70 8003E770 0000CD48 */  ctc2       $t5, $0 /* handwritten instruction */
    /* 2EF74 8003E774 0008CE48 */  ctc2       $t6, $1 /* handwritten instruction */
    /* 2EF78 8003E778 08008D8D */  lw         $t5, 0x8($t4)
    /* 2EF7C 8003E77C 0C008E8D */  lw         $t6, 0xC($t4)
    /* 2EF80 8003E780 10008F8D */  lw         $t7, 0x10($t4)
    /* 2EF84 8003E784 0010CD48 */  ctc2       $t5, $2 /* handwritten instruction */
    /* 2EF88 8003E788 0018CE48 */  ctc2       $t6, $3 /* handwritten instruction */
    /* 2EF8C 8003E78C 0020CF48 */  ctc2       $t7, $4 /* handwritten instruction */
    /* 2EF90 8003E790 3000A227 */  addiu      $v0, $sp, 0x30
    /* 2EF94 8003E794 21604000 */  addu       $t4, $v0, $zero
    /* 2EF98 8003E798 000080C9 */  lwc2       $0, 0x0($t4)
    /* 2EF9C 8003E79C 040081C9 */  lwc2       $1, 0x4($t4)
    /* 2EFA0 8003E7A0 00000000 */  nop
    /* 2EFA4 8003E7A4 00000000 */  nop
    /* 2EFA8 8003E7A8 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 2EFAC 8003E7AC 3800A227 */  addiu      $v0, $sp, 0x38
    /* 2EFB0 8003E7B0 21604000 */  addu       $t4, $v0, $zero
    /* 2EFB4 8003E7B4 000099E9 */  swc2       $25, 0x0($t4)
    /* 2EFB8 8003E7B8 04009AE9 */  swc2       $26, 0x4($t4) /* handwritten instruction */
    /* 2EFBC 8003E7BC 08009BE9 */  swc2       $27, 0x8($t4) /* handwritten instruction */
    /* 2EFC0 8003E7C0 3800A48F */  lw         $a0, 0x38($sp)
    /* 2EFC4 8003E7C4 4000A58F */  lw         $a1, 0x40($sp)
    /* 2EFC8 8003E7C8 57FF010C */  jal        func_8007FD5C
    /* 2EFCC 8003E7CC 00000000 */   nop
    /* 2EFD0 8003E7D0 F1FF2426 */  addiu      $a0, $s1, -0xF
    /* 2EFD4 8003E7D4 F1FF0526 */  addiu      $a1, $s0, -0xF
    /* 2EFD8 8003E7D8 A00282AF */  sw         $v0, %gp_rel(D_800A336C)($gp)
    /* 2EFDC 8003E7DC 83110200 */  sra        $v0, $v0, 6
    /* 2EFE0 8003E7E0 6001838F */  lw         $v1, %gp_rel(D_800A322C)($gp)
    /* 2EFE4 8003E7E4 00000000 */  nop
    /* 2EFE8 8003E7E8 06006010 */  beqz       $v1, .L8003E804
    /* 2EFEC 8003E7EC 3F004630 */   andi      $a2, $v0, 0x3F
    /* 2EFF0 8003E7F0 C0190600 */  sll        $v1, $a2, 7
    /* 2EFF4 8003E7F4 0980023C */  lui        $v0, %hi(D_800927C0)
    /* 2EFF8 8003E7F8 C0274224 */  addiu      $v0, $v0, %lo(D_800927C0)
    /* 2EFFC 8003E7FC 1FFA0008 */  j          .L8003E87C
    /* 2F000 8003E800 21606200 */   addu      $t4, $v1, $v0
  .L8003E804:
    /* 2F004 8003E804 0F80023C */  lui        $v0, %hi(D_800F6656)
    /* 2F008 8003E808 56664294 */  lhu        $v0, %lo(D_800F6656)($v0)
    /* 2F00C 8003E80C 00000000 */  nop
    /* 2F010 8003E810 01004230 */  andi       $v0, $v0, 0x1
    /* 2F014 8003E814 05004010 */  beqz       $v0, .L8003E82C
    /* 2F018 8003E818 C0190600 */   sll       $v1, $a2, 7
    /* 2F01C 8003E81C 0980023C */  lui        $v0, %hi(D_80090740)
    /* 2F020 8003E820 40074224 */  addiu      $v0, $v0, %lo(D_80090740)
    /* 2F024 8003E824 1FFA0008 */  j          .L8003E87C
    /* 2F028 8003E828 21606200 */   addu      $t4, $v1, $v0
  .L8003E82C:
    /* 2F02C 8003E82C 0980073C */  lui        $a3, %hi(D_80094840)
    /* 2F030 8003E830 4048E724 */  addiu      $a3, $a3, %lo(D_80094840)
    /* 2F034 8003E834 0980023C */  lui        $v0, %hi(D_80090740)
    /* 2F038 8003E838 40074224 */  addiu      $v0, $v0, %lo(D_80090740)
    /* 2F03C 8003E83C 21606200 */  addu       $t4, $v1, $v0
    /* 2F040 8003E840 801F063C */  lui        $a2, (0x1F800004 >> 16)
    /* 2F044 8003E844 0400C634 */  ori        $a2, $a2, (0x1F800004 & 0xFFFF)
    /* 2F048 8003E848 21680000 */  addu       $t5, $zero, $zero
  .L8003E84C:
    /* 2F04C 8003E84C 0000838D */  lw         $v1, 0x0($t4)
    /* 2F050 8003E850 04008C25 */  addiu      $t4, $t4, 0x4
    /* 2F054 8003E854 0000E28C */  lw         $v0, 0x0($a3)
    /* 2F058 8003E858 0400E724 */  addiu      $a3, $a3, 0x4
    /* 2F05C 8003E85C 0100AD25 */  addiu      $t5, $t5, 0x1
    /* 2F060 8003E860 24104300 */  and        $v0, $v0, $v1
    /* 2F064 8003E864 0000C2AC */  sw         $v0, 0x0($a2)
    /* 2F068 8003E868 1F00A229 */  slti       $v0, $t5, 0x1F
    /* 2F06C 8003E86C F7FF4014 */  bnez       $v0, .L8003E84C
    /* 2F070 8003E870 0400C624 */   addiu     $a2, $a2, 0x4
    /* 2F074 8003E874 801F0C3C */  lui        $t4, (0x1F800004 >> 16)
    /* 2F078 8003E878 04008C35 */  ori        $t4, $t4, (0x1F800004 & 0xFFFF)
  .L8003E87C:
    /* 2F07C 8003E87C 0A80063C */  lui        $a2, %hi(D_800A7EF0)
    /* 2F080 8003E880 F07EC624 */  addiu      $a2, $a2, %lo(D_800A7EF0)
    /* 2F084 8003E884 21680000 */  addu       $t5, $zero, $zero
    /* 2F088 8003E888 0A80193C */  lui        $t9, %hi(D_800A7FE0)
    /* 2F08C 8003E88C E07F3927 */  addiu      $t9, $t9, %lo(D_800A7FE0)
    /* 2F090 8003E890 0B80113C */  lui        $s1, %hi(D_800A8FB0)
    /* 2F094 8003E894 B08F3126 */  addiu      $s1, $s1, %lo(D_800A8FB0)
    /* 2F098 8003E898 0B80103C */  lui        $s0, %hi(D_800A87E0)
    /* 2F09C 8003E89C E0871026 */  addiu      $s0, $s0, %lo(D_800A87E0)
    /* 2F0A0 8003E8A0 2110AD00 */  addu       $v0, $a1, $t5
  .L8003E8A4:
    /* 2F0A4 8003E8A4 00140200 */  sll        $v0, $v0, 16
    /* 2F0A8 8003E8A8 031C0200 */  sra        $v1, $v0, 16
    /* 2F0AC 8003E8AC 03006104 */  bgez       $v1, .L8003E8BC
    /* 2F0B0 8003E8B0 20006228 */   slti      $v0, $v1, 0x20
    /* 2F0B4 8003E8B4 8DFA0008 */  j          .L8003EA34
    /* 2F0B8 8003E8B8 04008C25 */   addiu     $t4, $t4, 0x4
  .L8003E8BC:
    /* 2F0BC 8003E8BC 61004010 */  beqz       $v0, .L8003EA44
    /* 2F0C0 8003E8C0 00000000 */   nop
    /* 2F0C4 8003E8C4 0000898D */  lw         $t1, 0x0($t4)
    /* 2F0C8 8003E8C8 00000000 */  nop
    /* 2F0CC 8003E8CC 59002011 */  beqz       $t1, .L8003EA34
    /* 2F0D0 8003E8D0 04008C25 */   addiu     $t4, $t4, 0x4
    /* 2F0D4 8003E8D4 40480900 */  sll        $t1, $t1, 1
    /* 2F0D8 8003E8D8 21700000 */  addu       $t6, $zero, $zero
    /* 2F0DC 8003E8DC 80110300 */  sll        $v0, $v1, 6
    /* 2F0E0 8003E8E0 21C05900 */  addu       $t8, $v0, $t9
    /* 2F0E4 8003E8E4 40790300 */  sll        $t7, $v1, 5
  .L8003E8E8:
    /* 2F0E8 8003E8E8 21108E00 */  addu       $v0, $a0, $t6
    /* 2F0EC 8003E8EC 00140200 */  sll        $v0, $v0, 16
    /* 2F0F0 8003E8F0 031C0200 */  sra        $v1, $v0, 16
    /* 2F0F4 8003E8F4 4B006004 */  bltz       $v1, .L8003EA24
    /* 2F0F8 8003E8F8 20006228 */   slti      $v0, $v1, 0x20
    /* 2F0FC 8003E8FC 4D004010 */  beqz       $v0, .L8003EA34
    /* 2F100 8003E900 00000000 */   nop
    /* 2F104 8003E904 47002105 */  bgez       $t1, .L8003EA24
    /* 2F108 8003E908 40100300 */   sll       $v0, $v1, 1
    /* 2F10C 8003E90C 21105800 */  addu       $v0, $v0, $t8
    /* 2F110 8003E910 00004284 */  lh         $v0, 0x0($v0)
    /* 2F114 8003E914 00000000 */  nop
    /* 2F118 8003E918 42004004 */  bltz       $v0, .L8003EA24
    /* 2F11C 8003E91C 21404000 */   addu      $t0, $v0, $zero
    /* 2F120 8003E920 2110E301 */  addu       $v0, $t7, $v1
    /* 2F124 8003E924 21105100 */  addu       $v0, $v0, $s1
    /* 2F128 8003E928 00004A90 */  lbu        $t2, 0x0($v0)
    /* 2F12C 8003E92C 21100001 */  addu       $v0, $t0, $zero
  .L8003E930:
    /* 2F130 8003E930 00140200 */  sll        $v0, $v0, 16
    /* 2F134 8003E934 C3130200 */  sra        $v0, $v0, 15
    /* 2F138 8003E938 21105000 */  addu       $v0, $v0, $s0
    /* 2F13C 8003E93C 00004B94 */  lhu        $t3, 0x0($v0)
    /* 2F140 8003E940 9C02878F */  lw         $a3, %gp_rel(D_800A3368)($gp)
    /* 2F144 8003E944 FF7F6331 */  andi       $v1, $t3, 0x7FFF
    /* 2F148 8003E948 2A106700 */  slt        $v0, $v1, $a3
    /* 2F14C 8003E94C 21004010 */  beqz       $v0, .L8003E9D4
    /* 2F150 8003E950 01000825 */   addiu     $t0, $t0, 0x1
    /* 2F154 8003E954 00190300 */  sll        $v1, $v1, 4
    /* 2F158 8003E958 0A80023C */  lui        $v0, %hi(D_800A4750)
    /* 2F15C 8003E95C 50474224 */  addiu      $v0, $v0, %lo(D_800A4750)
    /* 2F160 8003E960 21386200 */  addu       $a3, $v1, $v0
    /* 2F164 8003E964 03004231 */  andi       $v0, $t2, 0x3
    /* 2F168 8003E968 0600E2A0 */  sb         $v0, 0x6($a3)
    /* 2F16C 8003E96C 08004231 */  andi       $v0, $t2, 0x8
    /* 2F170 8003E970 09004014 */  bnez       $v0, .L8003E998
    /* 2F174 8003E974 00000000 */   nop
    /* 2F178 8003E978 04004231 */  andi       $v0, $t2, 0x4
    /* 2F17C 8003E97C 09004010 */  beqz       $v0, .L8003E9A4
    /* 2F180 8003E980 00000000 */   nop
    /* 2F184 8003E984 0700E290 */  lbu        $v0, 0x7($a3)
    /* 2F188 8003E988 00000000 */  nop
    /* 2F18C 8003E98C 08004230 */  andi       $v0, $v0, 0x8
    /* 2F190 8003E990 04004010 */  beqz       $v0, .L8003E9A4
    /* 2F194 8003E994 00000000 */   nop
  .L8003E998:
    /* 2F198 8003E998 0700E290 */  lbu        $v0, 0x7($a3)
    /* 2F19C 8003E99C 6CFA0008 */  j          .L8003E9B0
    /* 2F1A0 8003E9A0 01004234 */   ori       $v0, $v0, 0x1
  .L8003E9A4:
    /* 2F1A4 8003E9A4 0700E290 */  lbu        $v0, 0x7($a3)
    /* 2F1A8 8003E9A8 00000000 */  nop
    /* 2F1AC 8003E9AC FE004230 */  andi       $v0, $v0, 0xFE
  .L8003E9B0:
    /* 2F1B0 8003E9B0 0700E2A0 */  sb         $v0, 0x7($a3)
    /* 2F1B4 8003E9B4 0A80033C */  lui        $v1, %hi(D_800A3820)
    /* 2F1B8 8003E9B8 2038638C */  lw         $v1, %lo(D_800A3820)($v1)
    /* 2F1BC 8003E9BC 00000000 */  nop
    /* 2F1C0 8003E9C0 04006224 */  addiu      $v0, $v1, 0x4
    /* 2F1C4 8003E9C4 0A80013C */  lui        $at, %hi(D_800A3820)
    /* 2F1C8 8003E9C8 203822AC */  sw         $v0, %lo(D_800A3820)($at)
    /* 2F1CC 8003E9CC 86FA0008 */  j          .L8003EA18
    /* 2F1D0 8003E9D0 000067AC */   sw        $a3, 0x0($v1)
  .L8003E9D4:
    /* 2F1D4 8003E9D4 23186700 */  subu       $v1, $v1, $a3
    /* 2F1D8 8003E9D8 40100300 */  sll        $v0, $v1, 1
    /* 2F1DC 8003E9DC 21104300 */  addu       $v0, $v0, $v1
    /* 2F1E0 8003E9E0 80100200 */  sll        $v0, $v0, 2
    /* 2F1E4 8003E9E4 21104300 */  addu       $v0, $v0, $v1
    /* 2F1E8 8003E9E8 C0100200 */  sll        $v0, $v0, 3
    /* 2F1EC 8003E9EC 0A80033C */  lui        $v1, %hi(D_800A6690)
    /* 2F1F0 8003E9F0 90666324 */  addiu      $v1, $v1, %lo(D_800A6690)
    /* 2F1F4 8003E9F4 21184300 */  addu       $v1, $v0, $v1
    /* 2F1F8 8003E9F8 58006290 */  lbu        $v0, 0x58($v1)
    /* 2F1FC 8003E9FC 00000000 */  nop
    /* 2F200 8003EA00 06004014 */  bnez       $v0, .L8003EA1C
    /* 2F204 8003EA04 00806231 */   andi      $v0, $t3, 0x8000
    /* 2F208 8003EA08 0000C3AC */  sw         $v1, 0x0($a2)
    /* 2F20C 8003EA0C 0400C624 */  addiu      $a2, $a2, 0x4
    /* 2F210 8003EA10 01000224 */  addiu      $v0, $zero, 0x1
    /* 2F214 8003EA14 580062A0 */  sb         $v0, 0x58($v1)
  .L8003EA18:
    /* 2F218 8003EA18 00806231 */  andi       $v0, $t3, 0x8000
  .L8003EA1C:
    /* 2F21C 8003EA1C C4FF4010 */  beqz       $v0, .L8003E930
    /* 2F220 8003EA20 21100001 */   addu      $v0, $t0, $zero
  .L8003EA24:
    /* 2F224 8003EA24 0100CE25 */  addiu      $t6, $t6, 0x1
    /* 2F228 8003EA28 1F00C229 */  slti       $v0, $t6, 0x1F
    /* 2F22C 8003EA2C AEFF4014 */  bnez       $v0, .L8003E8E8
    /* 2F230 8003EA30 40480900 */   sll       $t1, $t1, 1
  .L8003EA34:
    /* 2F234 8003EA34 0100AD25 */  addiu      $t5, $t5, 0x1
    /* 2F238 8003EA38 1F00A229 */  slti       $v0, $t5, 0x1F
    /* 2F23C 8003EA3C 99FF4014 */  bnez       $v0, .L8003E8A4
    /* 2F240 8003EA40 2110AD00 */   addu      $v0, $a1, $t5
  .L8003EA44:
    /* 2F244 8003EA44 0F80023C */  lui        $v0, %hi(D_800F6656)
    /* 2F248 8003EA48 56664294 */  lhu        $v0, %lo(D_800F6656)($v0)
    /* 2F24C 8003EA4C 00000000 */  nop
    /* 2F250 8003EA50 01004230 */  andi       $v0, $v0, 0x1
    /* 2F254 8003EA54 0A004010 */  beqz       $v0, .L8003EA80
    /* 2F258 8003EA58 00000000 */   nop
    /* 2F25C 8003EA5C 6001828F */  lw         $v0, %gp_rel(D_800A322C)($gp)
    /* 2F260 8003EA60 00000000 */  nop
    /* 2F264 8003EA64 06004014 */  bnez       $v0, .L8003EA80
    /* 2F268 8003EA68 00240400 */   sll       $a0, $a0, 16
    /* 2F26C 8003EA6C 03240400 */  sra        $a0, $a0, 16
    /* 2F270 8003EA70 002C0500 */  sll        $a1, $a1, 16
    /* 2F274 8003EA74 E1FA000C */  jal        func_8003EB84
    /* 2F278 8003EA78 032C0500 */   sra       $a1, $a1, 16
    /* 2F27C 8003EA7C 21304000 */  addu       $a2, $v0, $zero
  .L8003EA80:
    /* 2F280 8003EA80 0A80023C */  lui        $v0, %hi(D_800A7EF0)
    /* 2F284 8003EA84 F07E4224 */  addiu      $v0, $v0, %lo(D_800A7EF0)
    /* 2F288 8003EA88 1500C210 */  beq        $a2, $v0, .L8003EAE0
    /* 2F28C 8003EA8C 21204000 */   addu      $a0, $v0, $zero
    /* 2F290 8003EA90 FCFFC624 */  addiu      $a2, $a2, -0x4
  .L8003EA94:
    /* 2F294 8003EA94 0A80033C */  lui        $v1, %hi(D_800A3820)
    /* 2F298 8003EA98 2038638C */  lw         $v1, %lo(D_800A3820)($v1)
    /* 2F29C 8003EA9C 0000C28C */  lw         $v0, 0x0($a2)
    /* 2F2A0 8003EAA0 00000000 */  nop
    /* 2F2A4 8003EAA4 000062AC */  sw         $v0, 0x0($v1)
    /* 2F2A8 8003EAA8 0A80023C */  lui        $v0, %hi(D_800A3820)
    /* 2F2AC 8003EAAC 2038428C */  lw         $v0, %lo(D_800A3820)($v0)
    /* 2F2B0 8003EAB0 00000000 */  nop
    /* 2F2B4 8003EAB4 0000428C */  lw         $v0, 0x0($v0)
    /* 2F2B8 8003EAB8 00000000 */  nop
    /* 2F2BC 8003EABC 580040A0 */  sb         $zero, 0x58($v0)
    /* 2F2C0 8003EAC0 0A80023C */  lui        $v0, %hi(D_800A3820)
    /* 2F2C4 8003EAC4 2038428C */  lw         $v0, %lo(D_800A3820)($v0)
    /* 2F2C8 8003EAC8 00000000 */  nop
    /* 2F2CC 8003EACC 04004224 */  addiu      $v0, $v0, 0x4
    /* 2F2D0 8003EAD0 0A80013C */  lui        $at, %hi(D_800A3820)
    /* 2F2D4 8003EAD4 203822AC */  sw         $v0, %lo(D_800A3820)($at)
    /* 2F2D8 8003EAD8 EEFFC414 */  bne        $a2, $a0, .L8003EA94
    /* 2F2DC 8003EADC FCFFC624 */   addiu     $a2, $a2, -0x4
  .L8003EAE0:
    /* 2F2E0 8003EAE0 E619010C */  jal        stage_GetId
    /* 2F2E4 8003EAE4 00000000 */   nop
    /* 2F2E8 8003EAE8 C0100200 */  sll        $v0, $v0, 3
    /* 2F2EC 8003EAEC 0980013C */  lui        $at, %hi(D_800948C0)
    /* 2F2F0 8003EAF0 21082200 */  addu       $at, $at, $v0
    /* 2F2F4 8003EAF4 C048228C */  lw         $v0, %lo(D_800948C0)($at)
    /* 2F2F8 8003EAF8 00000000 */  nop
    /* 2F2FC 8003EAFC 0B004010 */  beqz       $v0, .L8003EB2C
    /* 2F300 8003EB00 4800A427 */   addiu     $a0, $sp, 0x48
    /* 2F304 8003EB04 E619010C */  jal        stage_GetId
    /* 2F308 8003EB08 00000000 */   nop
    /* 2F30C 8003EB0C C0100200 */  sll        $v0, $v0, 3
    /* 2F310 8003EB10 0980013C */  lui        $at, %hi(D_800948C0)
    /* 2F314 8003EB14 21082200 */  addu       $at, $at, $v0
    /* 2F318 8003EB18 C048228C */  lw         $v0, %lo(D_800948C0)($at)
    /* 2F31C 8003EB1C 00000000 */  nop
    /* 2F320 8003EB20 09F84000 */  jalr       $v0
    /* 2F324 8003EB24 00000000 */   nop
    /* 2F328 8003EB28 4800A427 */  addiu      $a0, $sp, 0x48
  .L8003EB2C:
    /* 2F32C 8003EB2C 1080053C */  lui        $a1, %hi(D_80101E00)
    /* 2F330 8003EB30 001EA524 */  addiu      $a1, $a1, %lo(D_80101E00)
    /* 2F334 8003EB34 0000A294 */  lhu        $v0, 0x0($a1)
    /* 2F338 8003EB38 1C00A524 */  addiu      $a1, $a1, 0x1C
    /* 2F33C 8003EB3C 1080033C */  lui        $v1, %hi(D_80101E04)
    /* 2F340 8003EB40 041E6394 */  lhu        $v1, %lo(D_80101E04)($v1)
    /* 2F344 8003EB44 23100200 */  negu       $v0, $v0
    /* 2F348 8003EB48 4800A2A7 */  sh         $v0, 0x48($sp)
    /* 2F34C 8003EB4C 1080023C */  lui        $v0, %hi(D_80101E02)
    /* 2F350 8003EB50 021E4294 */  lhu        $v0, %lo(D_80101E02)($v0)
    /* 2F354 8003EB54 23180300 */  negu       $v1, $v1
    /* 2F358 8003EB58 4C00A3A7 */  sh         $v1, 0x4C($sp)
    /* 2F35C 8003EB5C 23100200 */  negu       $v0, $v0
    /* 2F360 8003EB60 2E88010C */  jal        func_800620B8
    /* 2F364 8003EB64 4A00A2A7 */   sh        $v0, 0x4A($sp)
    /* 2F368 8003EB68 6400BF8F */  lw         $ra, 0x64($sp)
    /* 2F36C 8003EB6C 6000B28F */  lw         $s2, 0x60($sp)
    /* 2F370 8003EB70 5C00B18F */  lw         $s1, 0x5C($sp)
    /* 2F374 8003EB74 5800B08F */  lw         $s0, 0x58($sp)
    /* 2F378 8003EB78 6800BD27 */  addiu      $sp, $sp, 0x68
    /* 2F37C 8003EB7C 0800E003 */  jr         $ra
    /* 2F380 8003EB80 00000000 */   nop
endlabel func_8003E6D8
