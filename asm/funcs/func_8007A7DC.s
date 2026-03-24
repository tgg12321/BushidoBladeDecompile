glabel func_8007A7DC
    /* 6AFDC 8007A7DC E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6AFE0 8007A7E0 FFFF8730 */  andi       $a3, $a0, 0xFFFF
    /* 6AFE4 8007A7E4 00190700 */  sll        $v1, $a3, 4
    /* 6AFE8 8007A7E8 00016330 */  andi       $v1, $v1, 0x100
    /* 6AFEC 8007A7EC 82100700 */  srl        $v0, $a3, 2
    /* 6AFF0 8007A7F0 00024230 */  andi       $v0, $v0, 0x200
    /* 6AFF4 8007A7F4 21186200 */  addu       $v1, $v1, $v0
    /* 6AFF8 8007A7F8 C2290700 */  srl        $a1, $a3, 7
    /* 6AFFC 8007A7FC 42310700 */  srl        $a2, $a3, 5
    /* 6B000 8007A800 80390700 */  sll        $a3, $a3, 6
    /* 6B004 8007A804 0180043C */  lui        $a0, %hi(D_80015D58)
    /* 6B008 8007A808 585D8424 */  addiu      $a0, $a0, %lo(D_80015D58)
    /* 6B00C 8007A80C 0300A530 */  andi       $a1, $a1, 0x3
    /* 6B010 8007A810 0300C630 */  andi       $a2, $a2, 0x3
    /* 6B014 8007A814 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6B018 8007A818 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6B01C 8007A81C C007E730 */  andi       $a3, $a3, 0x7C0
    /* 6B020 8007A820 1800BFAF */  sw         $ra, 0x18($sp)
    /* 6B024 8007A824 09F84000 */  jalr       $v0
    /* 6B028 8007A828 1000A3AF */   sw        $v1, 0x10($sp)
    /* 6B02C 8007A82C 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6B030 8007A830 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6B034 8007A834 0800E003 */  jr         $ra
    /* 6B038 8007A838 00000000 */   nop
endlabel func_8007A7DC
