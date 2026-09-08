glabel _SsVmVSetUp
    /* 7863C 80087E3C 21388000 */  addu       $a3, $a0, $zero
    /* 78640 80087E40 FFFF8230 */  andi       $v0, $a0, 0xFFFF
    /* 78644 80087E44 1000422C */  sltiu      $v0, $v0, 0x10
    /* 78648 80087E48 10004010 */  beqz       $v0, .L80087E8C
    /* 7864C 80087E4C 2140A000 */   addu      $t0, $a1, $zero
    /* 78650 80087E50 00140400 */  sll        $v0, $a0, 16
    /* 78654 80087E54 03240200 */  sra        $a0, $v0, 16
    /* 78658 80087E58 1080013C */  lui        $at, %hi(_svm_vab_used)
    /* 7865C 80087E5C 21082400 */  addu       $at, $at, $a0
    /* 78660 80087E60 682A2390 */  lbu        $v1, %lo(_svm_vab_used)($at)
    /* 78664 80087E64 01000224 */  addiu      $v0, $zero, 0x1
    /* 78668 80087E68 23006214 */  bne        $v1, $v0, .L80087EF8
    /* 7866C 80087E6C FFFF0224 */   addiu     $v0, $zero, -0x1
    /* 78670 80087E70 001C0500 */  sll        $v1, $a1, 16
    /* 78674 80087E74 1080023C */  lui        $v0, %hi(kMaxPrograms)
    /* 78678 80087E78 34F64284 */  lh         $v0, %lo(kMaxPrograms)($v0)
    /* 7867C 80087E7C 031C0300 */  sra        $v1, $v1, 16
    /* 78680 80087E80 2A106200 */  slt        $v0, $v1, $v0
    /* 78684 80087E84 03004014 */  bnez       $v0, .L80087E94
    /* 78688 80087E88 80100400 */   sll       $v0, $a0, 2
  .L80087E8C:
    /* 7868C 80087E8C BE1F0208 */  j          .L80087EF8
    /* 78690 80087E90 FFFF0224 */   addiu     $v0, $zero, -0x1
  .L80087E94:
    /* 78694 80087E94 0F80013C */  lui        $at, %hi(_svm_vab_vh)
    /* 78698 80087E98 21082200 */  addu       $at, $at, $v0
    /* 7869C 80087E9C B866258C */  lw         $a1, %lo(_svm_vab_vh)($at)
    /* 786A0 80087EA0 0F80013C */  lui        $at, %hi(_svm_vab_pg)
    /* 786A4 80087EA4 21082200 */  addu       $at, $at, $v0
    /* 786A8 80087EA8 6066248C */  lw         $a0, %lo(_svm_vab_pg)($at)
    /* 786AC 80087EAC 0F80013C */  lui        $at, %hi(_svm_vab_tn)
    /* 786B0 80087EB0 21082200 */  addu       $at, $at, $v0
    /* 786B4 80087EB4 0067268C */  lw         $a2, %lo(_svm_vab_tn)($at)
    /* 786B8 80087EB8 00110300 */  sll        $v0, $v1, 4
    /* 786BC 80087EBC 1080013C */  lui        $at, %hi(_svm_cur_plus_0x1)
    /* 786C0 80087EC0 F12727A0 */  sb         $a3, %lo(_svm_cur_plus_0x1)($at)
    /* 786C4 80087EC4 1080013C */  lui        $at, %hi(_svm_cur_plus_0x6)
    /* 786C8 80087EC8 F62728A0 */  sb         $t0, %lo(_svm_cur_plus_0x6)($at)
    /* 786CC 80087ECC 21104400 */  addu       $v0, $v0, $a0
    /* 786D0 80087ED0 0800438C */  lw         $v1, 0x8($v0)
    /* 786D4 80087ED4 21100000 */  addu       $v0, $zero, $zero
    /* 786D8 80087ED8 1080013C */  lui        $at, %hi(_svm_vh)
    /* 786DC 80087EDC C41B25AC */  sw         $a1, %lo(_svm_vh)($at)
    /* 786E0 80087EE0 1080013C */  lui        $at, %hi(_svm_pg)
    /* 786E4 80087EE4 A0F624AC */  sw         $a0, %lo(_svm_pg)($at)
    /* 786E8 80087EE8 1080013C */  lui        $at, %hi(_svm_tn)
    /* 786EC 80087EEC C81B26AC */  sw         $a2, %lo(_svm_tn)($at)
    /* 786F0 80087EF0 1080013C */  lui        $at, %hi(_svm_cur_plus_0x7)
    /* 786F4 80087EF4 F72723A0 */  sb         $v1, %lo(_svm_cur_plus_0x7)($at)
  .L80087EF8:
    /* 786F8 80087EF8 0800E003 */  jr         $ra
    /* 786FC 80087EFC 00000000 */   nop
endlabel _SsVmVSetUp
