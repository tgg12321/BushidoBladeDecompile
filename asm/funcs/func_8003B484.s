glabel func_8003B484
    /* 2BC84 8003B484 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2BC88 8003B488 01000224 */  addiu      $v0, $zero, 0x1
    /* 2BC8C 8003B48C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2BC90 8003B490 0A80013C */  lui        $at, %hi(D_800A3712)
    /* 2BC94 8003B494 123722A0 */  sb         $v0, %lo(D_800A3712)($at)
    /* 2BC98 8003B498 00008290 */  lbu        $v0, 0x0($a0)
    /* 2BC9C 8003B49C 1080013C */  lui        $at, %hi(D_8010277D)
    /* 2BCA0 8003B4A0 7D2722A0 */  sb         $v0, %lo(D_8010277D)($at)
    /* 2BCA4 8003B4A4 01008290 */  lbu        $v0, 0x1($a0)
    /* 2BCA8 8003B4A8 1080013C */  lui        $at, %hi(D_8010277F)
    /* 2BCAC 8003B4AC 7F2722A0 */  sb         $v0, %lo(D_8010277F)($at)
    /* 2BCB0 8003B4B0 02008290 */  lbu        $v0, 0x2($a0)
    /* 2BCB4 8003B4B4 1080013C */  lui        $at, %hi(D_80102783)
    /* 2BCB8 8003B4B8 832722A0 */  sb         $v0, %lo(D_80102783)($at)
    /* 2BCBC 8003B4BC D0EB000C */  jal        func_8003AF40
    /* 2BCC0 8003B4C0 01000424 */   addiu     $a0, $zero, 0x1
    /* 2BCC4 8003B4C4 FFEB000C */  jal        md_menu_logo_exec
    /* 2BCC8 8003B4C8 00000000 */   nop
    /* 2BCCC 8003B4CC 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2BCD0 8003B4D0 03000224 */  addiu      $v0, $zero, 0x3
    /* 2BCD4 8003B4D4 0800E003 */  jr         $ra
    /* 2BCD8 8003B4D8 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_8003B484
