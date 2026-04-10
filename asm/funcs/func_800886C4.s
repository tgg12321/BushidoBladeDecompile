glabel func_800886C4
    /* 78EC4 800886C4 0A80023C */  lui        $v0, %hi(D_800A2CD8)
    /* 78EC8 800886C8 D82C428C */  lw         $v0, %lo(D_800A2CD8)($v0)
    /* 78ECC 800886CC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 78ED0 800886D0 17004014 */  bnez       $v0, .L80088730
    /* 78ED4 800886D4 1000BFAF */   sw        $ra, 0x10($sp)
    /* 78ED8 800886D8 01000224 */  addiu      $v0, $zero, 0x1
    /* 78EDC 800886DC 0A80013C */  lui        $at, %hi(D_800A2CD8)
    /* 78EE0 800886E0 D82C22AC */  sw         $v0, %lo(D_800A2CD8)($at)
    /* 78EE4 800886E4 6EE2010C */  jal        EnterCriticalSection
    /* 78EE8 800886E8 00000000 */   nop
    /* 78EEC 800886EC 0980043C */  lui        $a0, %hi(D_80088BA0)
    /* 78EF0 800886F0 A08B8424 */  addiu      $a0, $a0, %lo(D_80088BA0)
    /* 78EF4 800886F4 B524020C */  jal        func_800892D4
    /* 78EF8 800886F8 00000000 */   nop
    /* 78EFC 800886FC 00F0043C */  lui        $a0, (0xF0000009 >> 16)
    /* 78F00 80088700 09008434 */  ori        $a0, $a0, (0xF0000009 & 0xFFFF)
    /* 78F04 80088704 20000524 */  addiu      $a1, $zero, 0x20
    /* 78F08 80088708 00200624 */  addiu      $a2, $zero, 0x2000
    /* 78F0C 8008870C 5EE2010C */  jal        func_80078978
    /* 78F10 80088710 21380000 */   addu      $a3, $zero, $zero
    /* 78F14 80088714 21204000 */  addu       $a0, $v0, $zero
    /* 78F18 80088718 0A80013C */  lui        $at, %hi(D_800A2870)
    /* 78F1C 8008871C 702824AC */  sw         $a0, %lo(D_800A2870)($at)
    /* 78F20 80088720 6AE2010C */  jal        func_800789A8
    /* 78F24 80088724 00000000 */   nop
    /* 78F28 80088728 72E2010C */  jal        ExitCriticalSection
    /* 78F2C 8008872C 00000000 */   nop
  .L80088730:
    /* 78F30 80088730 1000BF8F */  lw         $ra, 0x10($sp)
    /* 78F34 80088734 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 78F38 80088738 0800E003 */  jr         $ra
    /* 78F3C 8008873C 00000000 */   nop
endlabel func_800886C4
