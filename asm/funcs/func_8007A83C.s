glabel func_8007A83C
    /* 6B03C 8007A83C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6B040 8007A840 21308000 */  addu       $a2, $a0, $zero
    /* 6B044 8007A844 3F00C530 */  andi       $a1, $a2, 0x3F
    /* 6B048 8007A848 FFFFC630 */  andi       $a2, $a2, 0xFFFF
    /* 6B04C 8007A84C 0180043C */  lui        $a0, %hi(D_80015D70)
    /* 6B050 8007A850 705D8424 */  addiu      $a0, $a0, %lo(D_80015D70)
    /* 6B054 8007A854 00290500 */  sll        $a1, $a1, 4
    /* 6B058 8007A858 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6B05C 8007A85C 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6B060 8007A860 1000BFAF */  sw         $ra, 0x10($sp)
    /* 6B064 8007A864 09F84000 */  jalr       $v0
    /* 6B068 8007A868 82310600 */   srl       $a2, $a2, 6
    /* 6B06C 8007A86C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 6B070 8007A870 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6B074 8007A874 0800E003 */  jr         $ra
    /* 6B078 8007A878 00000000 */   nop
endlabel func_8007A83C
