glabel func_80083A18
    /* 74218 80083A18 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 7421C 80083A1C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 74220 80083A20 B00A020C */  jal        irq_DisableInterrupts
    /* 74224 80083A24 00000000 */   nop
    /* 74228 80083A28 6B21020C */  jal        spu_Init
    /* 7422C 80083A2C 00000000 */   nop
    /* 74230 80083A30 920E020C */  jal        func_80083A48
    /* 74234 80083A34 00000000 */   nop
    /* 74238 80083A38 1000BF8F */  lw         $ra, 0x10($sp)
    /* 7423C 80083A3C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 74240 80083A40 0800E003 */  jr         $ra
    /* 74244 80083A44 00000000 */   nop
endlabel func_80083A18
