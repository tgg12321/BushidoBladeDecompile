glabel func_8007E74C
    /* 6EF4C 8007E74C 0000888C */  lw         $t0, 0x0($a0)
    /* 6EF50 8007E750 0400898C */  lw         $t1, 0x4($a0)
    /* 6EF54 8007E754 08008A8C */  lw         $t2, 0x8($a0)
    /* 6EF58 8007E758 0C008B8C */  lw         $t3, 0xC($a0)
    /* 6EF5C 8007E75C 10008C8C */  lw         $t4, 0x10($a0)
    /* 6EF60 8007E760 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 6EF64 8007E764 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 6EF68 8007E768 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 6EF6C 8007E76C 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 6EF70 8007E770 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 6EF74 8007E774 0000A88C */  lw         $t0, 0x0($a1)
    /* 6EF78 8007E778 0400A98C */  lw         $t1, 0x4($a1)
    /* 6EF7C 8007E77C 0800AA8C */  lw         $t2, 0x8($a1)
    /* 6EF80 8007E780 08000105 */  bgez       $t0, .L8007E7A4
    /* 6EF84 8007E784 C35B0800 */   sra       $t3, $t0, 15
    /* 6EF88 8007E788 23400800 */  negu       $t0, $t0
    /* 6EF8C 8007E78C C35B0800 */  sra        $t3, $t0, 15
    /* 6EF90 8007E790 FF7F0831 */  andi       $t0, $t0, 0x7FFF
    /* 6EF94 8007E794 23580B00 */  negu       $t3, $t3
    /* 6EF98 8007E798 03000010 */  b          .L8007E7A8
    /* 6EF9C 8007E79C 23400800 */   negu      $t0, $t0
    /* 6EFA0 8007E7A0 C35B0800 */  sra        $t3, $t0, 15
  .L8007E7A4:
    /* 6EFA4 8007E7A4 FF7F0831 */  andi       $t0, $t0, 0x7FFF
  .L8007E7A8:
    /* 6EFA8 8007E7A8 08002105 */  bgez       $t1, .L8007E7CC
    /* 6EFAC 8007E7AC C3630900 */   sra       $t4, $t1, 15
    /* 6EFB0 8007E7B0 23480900 */  negu       $t1, $t1
    /* 6EFB4 8007E7B4 C3630900 */  sra        $t4, $t1, 15
    /* 6EFB8 8007E7B8 FF7F2931 */  andi       $t1, $t1, 0x7FFF
    /* 6EFBC 8007E7BC 23600C00 */  negu       $t4, $t4
    /* 6EFC0 8007E7C0 03000010 */  b          .L8007E7D0
    /* 6EFC4 8007E7C4 23480900 */   negu      $t1, $t1
    /* 6EFC8 8007E7C8 C3630900 */  sra        $t4, $t1, 15
  .L8007E7CC:
    /* 6EFCC 8007E7CC FF7F2931 */  andi       $t1, $t1, 0x7FFF
  .L8007E7D0:
    /* 6EFD0 8007E7D0 08004105 */  bgez       $t2, .L8007E7F4
    /* 6EFD4 8007E7D4 C36B0A00 */   sra       $t5, $t2, 15
    /* 6EFD8 8007E7D8 23500A00 */  negu       $t2, $t2
    /* 6EFDC 8007E7DC C36B0A00 */  sra        $t5, $t2, 15
    /* 6EFE0 8007E7E0 FF7F4A31 */  andi       $t2, $t2, 0x7FFF
    /* 6EFE4 8007E7E4 23680D00 */  negu       $t5, $t5
    /* 6EFE8 8007E7E8 03000010 */  b          .L8007E7F8
    /* 6EFEC 8007E7EC 23500A00 */   negu      $t2, $t2
    /* 6EFF0 8007E7F0 C36B0A00 */  sra        $t5, $t2, 15
  .L8007E7F4:
    /* 6EFF4 8007E7F4 FF7F4A31 */  andi       $t2, $t2, 0x7FFF
  .L8007E7F8:
    /* 6EFF8 8007E7F8 00488B48 */  mtc2       $t3, $9 /* handwritten instruction */
    /* 6EFFC 8007E7FC 00508C48 */  mtc2       $t4, $10 /* handwritten instruction */
    /* 6F000 8007E800 00588D48 */  mtc2       $t5, $11 /* handwritten instruction */
    /* 6F004 8007E804 00000000 */  nop
    /* 6F008 8007E808 12E0414A */  mvmva      0, 0, 3, 3, 0
    /* 6F00C 8007E80C 00C80B48 */  mfc2       $t3, $25 /* handwritten instruction */
    /* 6F010 8007E810 00D00C48 */  mfc2       $t4, $26 /* handwritten instruction */
    /* 6F014 8007E814 00D80D48 */  mfc2       $t5, $27 /* handwritten instruction */
    /* 6F018 8007E818 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6F01C 8007E81C 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6F020 8007E820 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6F024 8007E824 00000000 */  nop
    /* 6F028 8007E828 12E0494A */  mvmva      1, 0, 3, 3, 0
    /* 6F02C 8007E82C 05006105 */  bgez       $t3, .L8007E844
    /* 6F030 8007E830 00000000 */   nop
    /* 6F034 8007E834 23580B00 */  negu       $t3, $t3
    /* 6F038 8007E838 C0580B00 */  sll        $t3, $t3, 3
    /* 6F03C 8007E83C 02000010 */  b          .L8007E848
    /* 6F040 8007E840 23580B00 */   negu      $t3, $t3
  .L8007E844:
    /* 6F044 8007E844 C0580B00 */  sll        $t3, $t3, 3
  .L8007E848:
    /* 6F048 8007E848 05008105 */  bgez       $t4, .L8007E860
    /* 6F04C 8007E84C 00000000 */   nop
    /* 6F050 8007E850 23600C00 */  negu       $t4, $t4
    /* 6F054 8007E854 C0600C00 */  sll        $t4, $t4, 3
    /* 6F058 8007E858 02000010 */  b          .L8007E864
    /* 6F05C 8007E85C 23600C00 */   negu      $t4, $t4
  .L8007E860:
    /* 6F060 8007E860 C0600C00 */  sll        $t4, $t4, 3
  .L8007E864:
    /* 6F064 8007E864 0500A105 */  bgez       $t5, .L8007E87C
    /* 6F068 8007E868 00000000 */   nop
    /* 6F06C 8007E86C 23680D00 */  negu       $t5, $t5
    /* 6F070 8007E870 C0680D00 */  sll        $t5, $t5, 3
    /* 6F074 8007E874 02000010 */  b          .L8007E880
    /* 6F078 8007E878 23680D00 */   negu      $t5, $t5
  .L8007E87C:
    /* 6F07C 8007E87C C0680D00 */  sll        $t5, $t5, 3
  .L8007E880:
    /* 6F080 8007E880 00C80848 */  mfc2       $t0, $25 /* handwritten instruction */
    /* 6F084 8007E884 00D00948 */  mfc2       $t1, $26 /* handwritten instruction */
    /* 6F088 8007E888 00D80A48 */  mfc2       $t2, $27 /* handwritten instruction */
    /* 6F08C 8007E88C 21400B01 */  addu       $t0, $t0, $t3
    /* 6F090 8007E890 21482C01 */  addu       $t1, $t1, $t4
    /* 6F094 8007E894 21504D01 */  addu       $t2, $t2, $t5
    /* 6F098 8007E898 0000C8AC */  sw         $t0, 0x0($a2)
    /* 6F09C 8007E89C 0400C9AC */  sw         $t1, 0x4($a2)
    /* 6F0A0 8007E8A0 0800CAAC */  sw         $t2, 0x8($a2)
    /* 6F0A4 8007E8A4 0800E003 */  jr         $ra
    /* 6F0A8 8007E8A8 2110C000 */   addu      $v0, $a2, $zero
endlabel func_8007E74C
