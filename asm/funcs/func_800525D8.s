glabel func_800525D8
    /* 42DD8 800525D8 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 42DDC 800525DC 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 42DE0 800525E0 00580A48 */  mfc2       $t2, $11 /* handwritten instruction */
    /* 42DE4 800525E4 00000000 */  nop
    /* 42DE8 800525E8 00000000 */  nop
    /* 42DEC 800525EC 2804A04A */  sqr        0
    /* 42DF0 800525F0 00000000 */  nop
    /* 42DF4 800525F4 00C80B48 */  mfc2       $t3, $25 /* handwritten instruction */
    /* 42DF8 800525F8 00D00C48 */  mfc2       $t4, $26 /* handwritten instruction */
    /* 42DFC 800525FC 00D80D48 */  mfc2       $t5, $27 /* handwritten instruction */
    /* 42E00 80052600 20586C01 */  add        $t3, $t3, $t4 /* handwritten instruction */
    /* 42E04 80052604 20106D01 */  add        $v0, $t3, $t5 /* handwritten instruction */
    /* 42E08 80052608 00F08248 */  mtc2       $v0, $30 /* handwritten instruction */
    /* 42E0C 8005260C 00000000 */  nop
    /* 42E10 80052610 00000000 */  nop
    /* 42E14 80052614 00F80348 */  mfc2       $v1, $31 /* handwritten instruction */
    /* 42E18 80052618 00000000 */  nop
    /* 42E1C 8005261C 1E006330 */  andi       $v1, $v1, 0x1E
    /* 42E20 80052620 1F000E24 */  addiu      $t6, $zero, 0x1F
    /* 42E24 80052624 2270C301 */  sub        $t6, $t6, $v1 /* handwritten instruction */
    /* 42E28 80052628 E8FF6B20 */  addi       $t3, $v1, -0x18 /* handwritten instruction */
    /* 42E2C 8005262C 03006005 */  bltz       $t3, .L8005263C
    /* 42E30 80052630 43700E00 */   sra       $t6, $t6, 1
    /* 42E34 80052634 92490108 */  j          .L80052648
    /* 42E38 80052638 04606201 */   sllv      $t4, $v0, $t3
  .L8005263C:
    /* 42E3C 8005263C 18000B24 */  addiu      $t3, $zero, 0x18
    /* 42E40 80052640 22586301 */  sub        $t3, $t3, $v1 /* handwritten instruction */
    /* 42E44 80052644 07606201 */  srav       $t4, $v0, $t3
  .L80052648:
    /* 42E48 80052648 C0FF8C21 */  addi       $t4, $t4, -0x40 /* handwritten instruction */
    /* 42E4C 8005264C 40600C00 */  sll        $t4, $t4, 1
    /* 42E50 80052650 01800D3C */  lui        $t5, %hi(D_800154A0)
    /* 42E54 80052654 A054AD25 */  addiu      $t5, $t5, %lo(D_800154A0)
    /* 42E58 80052658 2168AC01 */  addu       $t5, $t5, $t4
    /* 42E5C 8005265C 0000AD85 */  lh         $t5, 0x0($t5)
    /* 42E60 80052660 00000000 */  nop
    /* 42E64 80052664 00408D48 */  mtc2       $t5, $8 /* handwritten instruction */
    /* 42E68 80052668 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 42E6C 8005266C 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 42E70 80052670 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 42E74 80052674 00000000 */  nop
    /* 42E78 80052678 00000000 */  nop
    /* 42E7C 8005267C 3D00904B */  gpf        0
    /* 42E80 80052680 00C80848 */  mfc2       $t0, $25 /* handwritten instruction */
    /* 42E84 80052684 00D00948 */  mfc2       $t1, $26 /* handwritten instruction */
    /* 42E88 80052688 00D80A48 */  mfc2       $t2, $27 /* handwritten instruction */
    /* 42E8C 8005268C 0740C801 */  srav       $t0, $t0, $t6
    /* 42E90 80052690 0748C901 */  srav       $t1, $t1, $t6
    /* 42E94 80052694 0750CA01 */  srav       $t2, $t2, $t6
    /* 42E98 80052698 0800E003 */  jr         $ra
    /* 42E9C 8005269C 00000000 */   nop
endlabel func_800525D8
