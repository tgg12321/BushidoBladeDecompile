glabel func_80081974
    /* 72174 80081974 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 72178 80081978 1000BFAF */  sw         $ra, 0x10($sp)
    /* 7217C 8008197C 0A80013C */  lui        $at, %hi(D_800A11B8)
    /* 72180 80081980 B81120AC */  sw         $zero, %lo(D_800A11B8)($at)
    /* 72184 80081984 0A80013C */  lui        $at, %hi(D_800A11B4)
    /* 72188 80081988 B41120AC */  sw         $zero, %lo(D_800A11B4)($at)
    /* 7218C 8008198C 0A80013C */  lui        $at, %hi(D_800A11C8)
    /* 72190 80081990 C81120AC */  sw         $zero, %lo(D_800A11C8)($at)
    /* 72194 80081994 0A80013C */  lui        $at, %hi(D_800A11C4)
    /* 72198 80081998 C41120AC */  sw         $zero, %lo(D_800A11C4)($at)
    /* 7219C 8008199C B00A020C */  jal        irq_DisableInterrupts
    /* 721A0 800819A0 00000000 */   nop
    /* 721A4 800819A4 0880053C */  lui        $a1, %hi(D_80081F1C)
    /* 721A8 800819A8 1C1FA524 */  addiu      $a1, $a1, %lo(D_80081F1C)
    /* 721AC 800819AC BC0A020C */  jal        irq_EnableInterrupts
    /* 721B0 800819B0 02000424 */   addiu     $a0, $zero, 0x2
    /* 721B4 800819B4 1000BF8F */  lw         $ra, 0x10($sp)
    /* 721B8 800819B8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 721BC 800819BC 0800E003 */  jr         $ra
    /* 721C0 800819C0 00000000 */   nop
endlabel func_80081974
