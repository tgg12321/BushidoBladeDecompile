glabel func_80053584
    /* 43D84 80053584 0F80023C */  lui        $v0, %hi(D_800EF9F8)
    /* 43D88 80053588 F8F94224 */  addiu      $v0, $v0, %lo(D_800EF9F8)
    /* 43D8C 8005358C 280382AF */  sw         $v0, %gp_rel(D_800A33F4)($gp)
    /* 43D90 80053590 2803828F */  lw         $v0, %gp_rel(D_800A33F4)($gp)
    /* 43D94 80053594 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 43D98 80053598 1000BFAF */  sw         $ra, 0x10($sp)
    /* 43D9C 8005359C 0F800A3C */  lui        $t2, %hi(D_800EFA00)
    /* 43DA0 800535A0 00FA4A25 */  addiu      $t2, $t2, %lo(D_800EFA00)
    /* 43DA4 800535A4 0000838C */  lw         $v1, 0x0($a0)
    /* 43DA8 800535A8 0400888C */  lw         $t0, 0x4($a0)
    /* 43DAC 800535AC 0800898C */  lw         $t1, 0x8($a0)
    /* 43DB0 800535B0 000043AD */  sw         $v1, 0x0($t2)
    /* 43DB4 800535B4 040048AD */  sw         $t0, 0x4($t2)
    /* 43DB8 800535B8 080049AD */  sw         $t1, 0x8($t2)
    /* 43DBC 800535BC 0C00838C */  lw         $v1, 0xC($a0)
    /* 43DC0 800535C0 00000000 */  nop
    /* 43DC4 800535C4 0C0043AD */  sw         $v1, 0xC($t2)
    /* 43DC8 800535C8 0000A38C */  lw         $v1, 0x0($a1)
    /* 43DCC 800535CC 0400A48C */  lw         $a0, 0x4($a1)
    /* 43DD0 800535D0 0800A88C */  lw         $t0, 0x8($a1)
    /* 43DD4 800535D4 0C00A98C */  lw         $t1, 0xC($a1)
    /* 43DD8 800535D8 180043AC */  sw         $v1, 0x18($v0)
    /* 43DDC 800535DC 1C0044AC */  sw         $a0, 0x1C($v0)
    /* 43DE0 800535E0 200048AC */  sw         $t0, 0x20($v0)
    /* 43DE4 800535E4 240049AC */  sw         $t1, 0x24($v0)
    /* 43DE8 800535E8 0580023C */  lui        $v0, %hi(func_80053E9C)
    /* 43DEC 800535EC 9C3E4224 */  addiu      $v0, $v0, %lo(func_80053E9C)
    /* 43DF0 800535F0 2120C000 */  addu       $a0, $a2, $zero
    /* 43DF4 800535F4 2803838F */  lw         $v1, %gp_rel(D_800A33F4)($gp)
    /* 43DF8 800535F8 2128E000 */  addu       $a1, $a3, $zero
    /* 43DFC 800535FC 404B010C */  jal        func_80052D00
    /* 43E00 80053600 5C0062AC */   sw        $v0, 0x5C($v1)
    /* 43E04 80053604 1000BF8F */  lw         $ra, 0x10($sp)
    /* 43E08 80053608 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 43E0C 8005360C 0800E003 */  jr         $ra
    /* 43E10 80053610 00000000 */   nop
endlabel func_80053584
