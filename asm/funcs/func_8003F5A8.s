glabel func_8003F5A8
    /* 2FDA8 8003F5A8 80300600 */  sll        $a2, $a2, 2
    /* 2FDAC 8003F5AC 0B80013C */  lui        $at, %hi(D_800A93B0)
    /* 2FDB0 8003F5B0 21082600 */  addu       $at, $at, $a2
    /* 2FDB4 8003F5B4 B09324AC */  sw         $a0, %lo(D_800A93B0)($at)
    /* 2FDB8 8003F5B8 0B80013C */  lui        $at, %hi(D_800A93BC)
    /* 2FDBC 8003F5BC 21082600 */  addu       $at, $at, $a2
    /* 2FDC0 8003F5C0 BC9325AC */  sw         $a1, %lo(D_800A93BC)($at)
    /* 2FDC4 8003F5C4 0800E003 */  jr         $ra
    /* 2FDC8 8003F5C8 00000000 */   nop
endlabel func_8003F5A8
