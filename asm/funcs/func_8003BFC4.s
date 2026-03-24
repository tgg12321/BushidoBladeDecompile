glabel func_8003BFC4
    /* 2C7C4 8003BFC4 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 2C7C8 8003BFC8 1800BFAF */  sw         $ra, 0x18($sp)
    /* 2C7CC 8003BFCC 1A5A000C */  jal        func_80016868
    /* 2C7D0 8003BFD0 00000000 */   nop
    /* 2C7D4 8003BFD4 3783000C */  jal        func_80020CDC
    /* 2C7D8 8003BFD8 00000000 */   nop
    /* 2C7DC 8003BFDC 7105010C */  jal        func_800415C4
    /* 2C7E0 8003BFE0 21200000 */   addu      $a0, $zero, $zero
    /* 2C7E4 8003BFE4 7105010C */  jal        func_800415C4
    /* 2C7E8 8003BFE8 01000424 */   addiu     $a0, $zero, 0x1
    /* 2C7EC 8003BFEC 1D5B000C */  jal        func_80016C74
    /* 2C7F0 8003BFF0 00000000 */   nop
    /* 2C7F4 8003BFF4 0516010C */  jal        func_80045814
    /* 2C7F8 8003BFF8 00000000 */   nop
    /* 2C7FC 8003BFFC 21204000 */  addu       $a0, $v0, $zero
    /* 2C800 8003C000 1180053C */  lui        $a1, (0x80118000 >> 16)
    /* 2C804 8003C004 0080A534 */  ori        $a1, $a1, (0x80118000 & 0xFFFF)
    /* 2C808 8003C008 010B0224 */  addiu      $v0, $zero, 0xB01
    /* 2C80C 8003C00C 01000624 */  addiu      $a2, $zero, 0x1
    /* 2C810 8003C010 F80C0724 */  addiu      $a3, $zero, 0xCF8
    /* 2C814 8003C014 50DD000C */  jal        func_80037540
    /* 2C818 8003C018 1000A2AF */   sw        $v0, 0x10($sp)
    /* 2C81C 8003C01C D11A010C */  jal        func_80046B44
    /* 2C820 8003C020 00000000 */   nop
    /* 2C824 8003C024 08000224 */  addiu      $v0, $zero, 0x8
    /* 2C828 8003C028 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2C82C 8003C02C 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 2C830 8003C030 1800BF8F */  lw         $ra, 0x18($sp)
    /* 2C834 8003C034 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 2C838 8003C038 0800E003 */  jr         $ra
    /* 2C83C 8003C03C 00000000 */   nop
endlabel func_8003BFC4
