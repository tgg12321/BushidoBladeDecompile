glabel func_80063BA4
    /* 543A4 80063BA4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 543A8 80063BA8 B403838F */  lw         $v1, %gp_rel(D_800A3480)($gp)
    /* 543AC 80063BAC 92038287 */  lh         $v0, %gp_rel(D_800A345E)($gp)
    /* 543B0 80063BB0 01000424 */  addiu      $a0, $zero, 0x1
    /* 543B4 80063BB4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 543B8 80063BB8 848F010C */  jal        func_80063E10
    /* 543BC 80063BBC 000062AC */   sw        $v0, 0x0($v1)
    /* 543C0 80063BC0 1000BF8F */  lw         $ra, 0x10($sp)
    /* 543C4 80063BC4 FF004230 */  andi       $v0, $v0, 0xFF
    /* 543C8 80063BC8 0800E003 */  jr         $ra
    /* 543CC 80063BCC 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80063BA4
