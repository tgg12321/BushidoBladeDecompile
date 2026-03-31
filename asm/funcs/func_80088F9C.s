glabel func_80088F9C
    /* 7979C 80088F9C 0A80023C */  lui        $v0, %hi(D_800A2CF8)
    /* 797A0 80088FA0 F82C428C */  lw         $v0, %lo(D_800A2CF8)($v0)
    /* 797A4 80088FA4 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 797A8 80088FA8 1400B1AF */  sw         $s1, 0x14($sp)
    /* 797AC 80088FAC 21888000 */  addu       $s1, $a0, $zero
    /* 797B0 80088FB0 1000B0AF */  sw         $s0, 0x10($sp)
    /* 797B4 80088FB4 2180A000 */  addu       $s0, $a1, $zero
    /* 797B8 80088FB8 10004014 */  bnez       $v0, .L80088FFC
    /* 797BC 80088FBC 1800BFAF */   sw        $ra, 0x18($sp)
    /* 797C0 80088FC0 0A80023C */  lui        $v0, %hi(D_800A2CF4)
    /* 797C4 80088FC4 F42C4294 */  lhu        $v0, %lo(D_800A2CF4)($v0)
    /* 797C8 80088FC8 0A80053C */  lui        $a1, %hi(D_800A2D04)
    /* 797CC 80088FCC 042DA58C */  lw         $a1, %lo(D_800A2D04)($a1)
    /* 797D0 80088FD0 02000424 */  addiu      $a0, $zero, 0x2
    /* 797D4 80088FD4 4323020C */  jal        saTan0GaugeDraw
    /* 797D8 80088FD8 0428A200 */   sllv      $a1, $v0, $a1
    /* 797DC 80088FDC 4323020C */  jal        saTan0GaugeDraw
    /* 797E0 80088FE0 01000424 */   addiu     $a0, $zero, 0x1
    /* 797E4 80088FE4 03000424 */  addiu      $a0, $zero, 0x3
    /* 797E8 80088FE8 21282002 */  addu       $a1, $s1, $zero
    /* 797EC 80088FEC 4323020C */  jal        saTan0GaugeDraw
    /* 797F0 80088FF0 21300002 */   addu      $a2, $s0, $zero
    /* 797F4 80088FF4 03240208 */  j          .L8008900C
    /* 797F8 80088FF8 21100002 */   addu      $v0, $s0, $zero
  .L80088FFC:
    /* 797FC 80088FFC 21202002 */  addu       $a0, $s1, $zero
    /* 79800 80089000 7522020C */  jal        DispUpdateStatusMessage
    /* 79804 80089004 21280002 */   addu      $a1, $s0, $zero
    /* 79808 80089008 21100002 */  addu       $v0, $s0, $zero
  .L8008900C:
    /* 7980C 8008900C 1800BF8F */  lw         $ra, 0x18($sp)
    /* 79810 80089010 1400B18F */  lw         $s1, 0x14($sp)
    /* 79814 80089014 1000B08F */  lw         $s0, 0x10($sp)
    /* 79818 80089018 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 7981C 8008901C 0800E003 */  jr         $ra
    /* 79820 80089020 00000000 */   nop
endlabel func_80088F9C
