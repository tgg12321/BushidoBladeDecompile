glabel func_8008BDE8
    /* 7C5E8 8008BDE8 0A80023C */  lui        $v0, %hi(D_800A2CDC)
    /* 7C5EC 8008BDEC DC2C428C */  lw         $v0, %lo(D_800A2CDC)($v0)
    /* 7C5F0 8008BDF0 00210400 */  sll        $a0, $a0, 4
    /* 7C5F4 8008BDF4 21208200 */  addu       $a0, $a0, $v0
    /* 7C5F8 8008BDF8 0C008294 */  lhu        $v0, 0xC($a0)
    /* 7C5FC 8008BDFC 0800E003 */  jr         $ra
    /* 7C600 8008BE00 0000A2A4 */   sh        $v0, 0x0($a1)
endlabel func_8008BDE8
