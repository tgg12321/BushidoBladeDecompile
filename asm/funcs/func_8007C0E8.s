glabel func_8007C0E8
    /* 6C8E8 8007C0E8 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6C8EC 8007C0EC 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6C8F0 8007C0F0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6C8F4 8007C0F4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 6C8F8 8007C0F8 3800428C */  lw         $v0, 0x38($v0)
    /* 6C8FC 8007C0FC 00000000 */  nop
    /* 6C900 8007C100 09F84000 */  jalr       $v0
    /* 6C904 8007C104 00000000 */   nop
    /* 6C908 8007C108 1000BF8F */  lw         $ra, 0x10($sp)
    /* 6C90C 8007C10C C2170200 */  srl        $v0, $v0, 31
    /* 6C910 8007C110 0800E003 */  jr         $ra
    /* 6C914 8007C114 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_8007C0E8
