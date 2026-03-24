glabel func_8007A7C4
    /* 6AFC4 8007A7C4 80110500 */  sll        $v0, $a1, 6
    /* 6AFC8 8007A7C8 03210400 */  sra        $a0, $a0, 4
    /* 6AFCC 8007A7CC 3F008430 */  andi       $a0, $a0, 0x3F
    /* 6AFD0 8007A7D0 25104400 */  or         $v0, $v0, $a0
    /* 6AFD4 8007A7D4 0800E003 */  jr         $ra
    /* 6AFD8 8007A7D8 FFFF4230 */   andi      $v0, $v0, 0xFFFF
endlabel func_8007A7C4
