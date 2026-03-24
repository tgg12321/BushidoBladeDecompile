glabel func_8007D3D4
    /* 6DBD4 8007D3D4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6DBD8 8007D3D8 1000BFAF */  sw         $ra, 0x10($sp)
    /* 6DBDC 8007D3DC 2138C000 */  addu       $a3, $a2, $zero
    /* 6DBE0 8007D3E0 FEF4010C */  jal        func_8007D3F8
    /* 6DBE4 8007D3E4 21300000 */   addu      $a2, $zero, $zero
    /* 6DBE8 8007D3E8 1000BF8F */  lw         $ra, 0x10($sp)
    /* 6DBEC 8007D3EC 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6DBF0 8007D3F0 0800E003 */  jr         $ra
    /* 6DBF4 8007D3F4 00000000 */   nop
endlabel func_8007D3D4
