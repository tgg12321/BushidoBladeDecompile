glabel func_80033498
    /* 23C98 80033498 0A80023C */  lui        $v0, %hi(D_800A36A4)
    /* 23C9C 8003349C A4364294 */  lhu        $v0, %lo(D_800A36A4)($v0)
    /* 23CA0 800334A0 00000000 */  nop
    /* 23CA4 800334A4 FEFF4224 */  addiu      $v0, $v0, -0x2
    /* 23CA8 800334A8 00140200 */  sll        $v0, $v0, 16
    /* 23CAC 800334AC 031C0200 */  sra        $v1, $v0, 16
    /* 23CB0 800334B0 1700622C */  sltiu      $v0, $v1, 0x17
    /* 23CB4 800334B4 13004010 */  beqz       $v0, .L80033504
    /* 23CB8 800334B8 80100300 */   sll       $v0, $v1, 2
    /* 23CBC 800334BC 0180013C */  lui        $at, %hi(jtbl_800107C0)
    /* 23CC0 800334C0 21082200 */  addu       $at, $at, $v0
    /* 23CC4 800334C4 C007228C */  lw         $v0, %lo(jtbl_800107C0)($at)
    /* 23CC8 800334C8 00000000 */  nop
    /* 23CCC 800334CC 08004000 */  jr         $v0
    /* 23CD0 800334D0 00000000 */   nop
  jlabel .L800334D4
    /* 23CD4 800334D4 42CD0008 */  j          .L80033508
    /* 23CD8 800334D8 21100000 */   addu      $v0, $zero, $zero
  jlabel .L800334DC
    /* 23CDC 800334DC 42CD0008 */  j          .L80033508
    /* 23CE0 800334E0 01000224 */   addiu     $v0, $zero, 0x1
  jlabel .L800334E4
    /* 23CE4 800334E4 42CD0008 */  j          .L80033508
    /* 23CE8 800334E8 02000224 */   addiu     $v0, $zero, 0x2
  jlabel .L800334EC
    /* 23CEC 800334EC 42CD0008 */  j          .L80033508
    /* 23CF0 800334F0 03000224 */   addiu     $v0, $zero, 0x3
  jlabel .L800334F4
    /* 23CF4 800334F4 42CD0008 */  j          .L80033508
    /* 23CF8 800334F8 04000224 */   addiu     $v0, $zero, 0x4
  jlabel .L800334FC
    /* 23CFC 800334FC 42CD0008 */  j          .L80033508
    /* 23D00 80033500 05000224 */   addiu     $v0, $zero, 0x5
  jlabel .L80033504
    /* 23D04 80033504 FF000224 */  addiu      $v0, $zero, 0xFF
  .L80033508:
    /* 23D08 80033508 0800E003 */  jr         $ra
    /* 23D0C 8003350C 00000000 */   nop
endlabel func_80033498
