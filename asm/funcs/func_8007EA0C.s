glabel func_8007EA0C
    /* 6F20C 8007EA0C 0000888C */  lw         $t0, 0x0($a0)
    /* 6F210 8007EA10 0400898C */  lw         $t1, 0x4($a0)
    /* 6F214 8007EA14 08008A8C */  lw         $t2, 0x8($a0)
    /* 6F218 8007EA18 08000105 */  bgez       $t0, .L8007EA3C
    /* 6F21C 8007EA1C C35B0800 */   sra       $t3, $t0, 15
    /* 6F220 8007EA20 23400800 */  negu       $t0, $t0
    /* 6F224 8007EA24 C35B0800 */  sra        $t3, $t0, 15
    /* 6F228 8007EA28 FF7F0831 */  andi       $t0, $t0, 0x7FFF
    /* 6F22C 8007EA2C 23580B00 */  negu       $t3, $t3
    /* 6F230 8007EA30 03000010 */  b          .L8007EA40
    /* 6F234 8007EA34 23400800 */   negu      $t0, $t0
    /* 6F238 8007EA38 C35B0800 */  sra        $t3, $t0, 15
  .L8007EA3C:
    /* 6F23C 8007EA3C FF7F0831 */  andi       $t0, $t0, 0x7FFF
  .L8007EA40:
    /* 6F240 8007EA40 08002105 */  bgez       $t1, .L8007EA64
    /* 6F244 8007EA44 C3630900 */   sra       $t4, $t1, 15
    /* 6F248 8007EA48 23480900 */  negu       $t1, $t1
    /* 6F24C 8007EA4C C3630900 */  sra        $t4, $t1, 15
    /* 6F250 8007EA50 FF7F2931 */  andi       $t1, $t1, 0x7FFF
    /* 6F254 8007EA54 23600C00 */  negu       $t4, $t4
    /* 6F258 8007EA58 03000010 */  b          .L8007EA68
    /* 6F25C 8007EA5C 23480900 */   negu      $t1, $t1
    /* 6F260 8007EA60 C3630900 */  sra        $t4, $t1, 15
  .L8007EA64:
    /* 6F264 8007EA64 FF7F2931 */  andi       $t1, $t1, 0x7FFF
  .L8007EA68:
    /* 6F268 8007EA68 08004105 */  bgez       $t2, .L8007EA8C
    /* 6F26C 8007EA6C C36B0A00 */   sra       $t5, $t2, 15
    /* 6F270 8007EA70 23500A00 */  negu       $t2, $t2
    /* 6F274 8007EA74 C36B0A00 */  sra        $t5, $t2, 15
    /* 6F278 8007EA78 FF7F4A31 */  andi       $t2, $t2, 0x7FFF
    /* 6F27C 8007EA7C 23680D00 */  negu       $t5, $t5
    /* 6F280 8007EA80 03000010 */  b          .L8007EA90
    /* 6F284 8007EA84 23500A00 */   negu      $t2, $t2
    /* 6F288 8007EA88 C36B0A00 */  sra        $t5, $t2, 15
  .L8007EA8C:
    /* 6F28C 8007EA8C FF7F4A31 */  andi       $t2, $t2, 0x7FFF
  .L8007EA90:
    /* 6F290 8007EA90 00488B48 */  mtc2       $t3, $9 /* handwritten instruction */
    /* 6F294 8007EA94 00508C48 */  mtc2       $t4, $10 /* handwritten instruction */
    /* 6F298 8007EA98 00588D48 */  mtc2       $t5, $11 /* handwritten instruction */
    /* 6F29C 8007EA9C 00000000 */  nop
    /* 6F2A0 8007EAA0 12E0414A */  mvmva      0, 0, 3, 3, 0
    /* 6F2A4 8007EAA4 00C80B48 */  mfc2       $t3, $25 /* handwritten instruction */
    /* 6F2A8 8007EAA8 00D00C48 */  mfc2       $t4, $26 /* handwritten instruction */
    /* 6F2AC 8007EAAC 00D80D48 */  mfc2       $t5, $27 /* handwritten instruction */
    /* 6F2B0 8007EAB0 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6F2B4 8007EAB4 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6F2B8 8007EAB8 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6F2BC 8007EABC 00000000 */  nop
    /* 6F2C0 8007EAC0 12E0494A */  mvmva      1, 0, 3, 3, 0
    /* 6F2C4 8007EAC4 05006105 */  bgez       $t3, .L8007EADC
    /* 6F2C8 8007EAC8 00000000 */   nop
    /* 6F2CC 8007EACC 23580B00 */  negu       $t3, $t3
    /* 6F2D0 8007EAD0 C0580B00 */  sll        $t3, $t3, 3
    /* 6F2D4 8007EAD4 02000010 */  b          .L8007EAE0
    /* 6F2D8 8007EAD8 23580B00 */   negu      $t3, $t3
  .L8007EADC:
    /* 6F2DC 8007EADC C0580B00 */  sll        $t3, $t3, 3
  .L8007EAE0:
    /* 6F2E0 8007EAE0 05008105 */  bgez       $t4, .L8007EAF8
    /* 6F2E4 8007EAE4 00000000 */   nop
    /* 6F2E8 8007EAE8 23600C00 */  negu       $t4, $t4
    /* 6F2EC 8007EAEC C0600C00 */  sll        $t4, $t4, 3
    /* 6F2F0 8007EAF0 02000010 */  b          .L8007EAFC
    /* 6F2F4 8007EAF4 23600C00 */   negu      $t4, $t4
  .L8007EAF8:
    /* 6F2F8 8007EAF8 C0600C00 */  sll        $t4, $t4, 3
  .L8007EAFC:
    /* 6F2FC 8007EAFC 0500A105 */  bgez       $t5, .L8007EB14
    /* 6F300 8007EB00 00000000 */   nop
    /* 6F304 8007EB04 23680D00 */  negu       $t5, $t5
    /* 6F308 8007EB08 C0680D00 */  sll        $t5, $t5, 3
    /* 6F30C 8007EB0C 02000010 */  b          .L8007EB18
    /* 6F310 8007EB10 23680D00 */   negu      $t5, $t5
  .L8007EB14:
    /* 6F314 8007EB14 C0680D00 */  sll        $t5, $t5, 3
  .L8007EB18:
    /* 6F318 8007EB18 00C80848 */  mfc2       $t0, $25 /* handwritten instruction */
    /* 6F31C 8007EB1C 00D00948 */  mfc2       $t1, $26 /* handwritten instruction */
    /* 6F320 8007EB20 00D80A48 */  mfc2       $t2, $27 /* handwritten instruction */
    /* 6F324 8007EB24 21400B01 */  addu       $t0, $t0, $t3
    /* 6F328 8007EB28 21482C01 */  addu       $t1, $t1, $t4
    /* 6F32C 8007EB2C 21504D01 */  addu       $t2, $t2, $t5
    /* 6F330 8007EB30 0000A8AC */  sw         $t0, 0x0($a1)
    /* 6F334 8007EB34 0400A9AC */  sw         $t1, 0x4($a1)
    /* 6F338 8007EB38 0800AAAC */  sw         $t2, 0x8($a1)
    /* 6F33C 8007EB3C 0800E003 */  jr         $ra
    /* 6F340 8007EB40 2110A000 */   addu      $v0, $a1, $zero
endlabel func_8007EA0C
