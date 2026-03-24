glabel func_8003D774
    /* 2DF74 8003D774 40100500 */  sll        $v0, $a1, 1
    /* 2DF78 8003D778 21104500 */  addu       $v0, $v0, $a1
    /* 2DF7C 8003D77C C0100200 */  sll        $v0, $v0, 3
    /* 2DF80 8003D780 0A80033C */  lui        $v1, %hi(D_800A3D40)
    /* 2DF84 8003D784 403D6324 */  addiu      $v1, $v1, %lo(D_800A3D40)
    /* 2DF88 8003D788 21104300 */  addu       $v0, $v0, $v1
    /* 2DF8C 8003D78C 000044AC */  sw         $a0, 0x0($v0)
    /* 2DF90 8003D790 040040AC */  sw         $zero, 0x4($v0)
    /* 2DF94 8003D794 080040AC */  sw         $zero, 0x8($v0)
    /* 2DF98 8003D798 160040A4 */  sh         $zero, 0x16($v0)
    /* 2DF9C 8003D79C 140040A4 */  sh         $zero, 0x14($v0)
    /* 2DFA0 8003D7A0 120040A4 */  sh         $zero, 0x12($v0)
    /* 2DFA4 8003D7A4 100040A4 */  sh         $zero, 0x10($v0)
    /* 2DFA8 8003D7A8 0E0040A4 */  sh         $zero, 0xE($v0)
    /* 2DFAC 8003D7AC 0800E003 */  jr         $ra
    /* 2DFB0 8003D7B0 0C0040A4 */   sh        $zero, 0xC($v0)
endlabel func_8003D774
