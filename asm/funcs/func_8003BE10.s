glabel func_8003BE10
    /* 2C610 8003BE10 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2C614 8003BE14 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2C618 8003BE18 1A5A000C */  jal        gpu_EnableDisplay
    /* 2C61C 8003BE1C 00000000 */   nop
    /* 2C620 8003BE20 225A000C */  jal        gpu_InitDisplay
    /* 2C624 8003BE24 00000000 */   nop
    /* 2C628 8003BE28 3783000C */  jal        func_80020CDC
    /* 2C62C 8003BE2C 00000000 */   nop
    /* 2C630 8003BE30 7105010C */  jal        func_800415C4
    /* 2C634 8003BE34 21200000 */   addu      $a0, $zero, $zero
    /* 2C638 8003BE38 7105010C */  jal        func_800415C4
    /* 2C63C 8003BE3C 01000424 */   addiu     $a0, $zero, 0x1
    /* 2C640 8003BE40 1D5B000C */  jal        file_ResetDmaFlag
    /* 2C644 8003BE44 00000000 */   nop
    /* 2C648 8003BE48 CB6D010C */  jal        func_8005B72C
    /* 2C64C 8003BE4C 00000000 */   nop
    /* 2C650 8003BE50 1180043C */  lui        $a0, (0x80118800 >> 16)
    /* 2C654 8003BE54 09E2010C */  jal        func_80078824
    /* 2C658 8003BE58 00888434 */   ori       $a0, $a0, (0x80118800 & 0xFFFF)
    /* 2C65C 8003BE5C EAD7000C */  jal        func_80035FA8
    /* 2C660 8003BE60 00000000 */   nop
    /* 2C664 8003BE64 05000424 */  addiu      $a0, $zero, 0x5
    /* 2C668 8003BE68 AADB000C */  jal        func_80036EA8
    /* 2C66C 8003BE6C 20000524 */   addiu     $a1, $zero, 0x20
    /* 2C670 8003BE70 21204000 */  addu       $a0, $v0, $zero
    /* 2C674 8003BE74 F5DB000C */  jal        func_80036FD4
    /* 2C678 8003BE78 04000524 */   addiu     $a1, $zero, 0x4
    /* 2C67C 8003BE7C 98DC000C */  jal        func_80037260
    /* 2C680 8003BE80 00000000 */   nop
    /* 2C684 8003BE84 0B000224 */  addiu      $v0, $zero, 0xB
    /* 2C688 8003BE88 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2C68C 8003BE8C 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 2C690 8003BE90 345A000C */  jal        gpu_DisableDisplay
    /* 2C694 8003BE94 00000000 */   nop
    /* 2C698 8003BE98 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2C69C 8003BE9C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2C6A0 8003BEA0 0800E003 */  jr         $ra
    /* 2C6A4 8003BEA4 00000000 */   nop
endlabel func_8003BE10
