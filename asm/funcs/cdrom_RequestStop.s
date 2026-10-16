glabel cdrom_RequestStop
    /* 27A34 80037234 01000224 */  addiu      $v0, $zero, 0x1
    /* 27A38 80037238 1080013C */  lui        $at, %hi(D_80101E58 + 0xC)
    /* 27A3C 8003723C 641E20A4 */  sh         $zero, %lo(D_80101E58 + 0xC)($at)
    /* 27A40 80037240 1080013C */  lui        $at, %hi(D_80101E58 + 0x10)
    /* 27A44 80037244 681E22A4 */  sh         $v0, %lo(D_80101E58 + 0x10)($at)
    /* 27A48 80037248 0800E003 */  jr         $ra
    /* 27A4C 8003724C 00000000 */   nop
endlabel cdrom_RequestStop
