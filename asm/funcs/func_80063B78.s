glabel func_80063B78
    /* 54378 80063B78 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 5437C 80063B7C B403838F */  lw         $v1, %gp_rel(D_800A3480)($gp)
    /* 54380 80063B80 90038287 */  lh         $v0, %gp_rel(D_800A345C)($gp)
    /* 54384 80063B84 21200000 */  addu       $a0, $zero, $zero
    /* 54388 80063B88 1000BFAF */  sw         $ra, 0x10($sp)
    /* 5438C 80063B8C 848F010C */  jal        func_80063E10
    /* 54390 80063B90 000062AC */   sw        $v0, 0x0($v1)
    /* 54394 80063B94 1000BF8F */  lw         $ra, 0x10($sp)
    /* 54398 80063B98 FF004230 */  andi       $v0, $v0, 0xFF
    /* 5439C 80063B9C 0800E003 */  jr         $ra
    /* 543A0 80063BA0 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80063B78
