glabel func_80016D78
    /* 7578 80016D78 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 757C 80016D7C 1080053C */  lui        $a1, (0x8010DB00 >> 16)
    /* 7580 80016D80 0180043C */  lui        $a0, %hi(D_80010028)
    /* 7584 80016D84 28008424 */  addiu      $a0, $a0, %lo(D_80010028)
    /* 7588 80016D88 1000BFAF */  sw         $ra, 0x10($sp)
    /* 758C 80016D8C 82E4010C */  jal        func_80079208
    /* 7590 80016D90 00DBA534 */   ori       $a1, $a1, (0x8010DB00 & 0xFFFF)
    /* 7594 80016D94 FB59000C */  jal        func_800167EC
    /* 7598 80016D98 00000000 */   nop
    /* 759C 80016D9C 5C83000C */  jal        func_80020D70
    /* 75A0 80016DA0 00000000 */   nop
    /* 75A4 80016DA4 1D80043C */  lui        $a0, (0x801D8800 >> 16)
    /* 75A8 80016DA8 00888434 */  ori        $a0, $a0, (0x801D8800 & 0xFFFF)
    /* 75AC 80016DAC 1E80033C */  lui        $v1, (0x801EBC00 >> 16)
    /* 75B0 80016DB0 00BC6334 */  ori        $v1, $v1, (0x801EBC00 & 0xFFFF)
    /* 75B4 80016DB4 0100023C */  lui        $v0, (0x13400 >> 16)
    /* 75B8 80016DB8 00344234 */  ori        $v0, $v0, (0x13400 & 0xFFFF)
    /* 75BC 80016DBC A40684AF */  sw         $a0, %gp_rel(D_800A3770)($gp)
    /* 75C0 80016DC0 0A80013C */  lui        $at, %hi(D_800A3774)
    /* 75C4 80016DC4 743723AC */  sw         $v1, %lo(D_800A3774)($at)
    /* 75C8 80016DC8 CC0682AF */  sw         $v0, %gp_rel(D_800A3798)($gp)
    /* 75CC 80016DCC 4A0680A3 */  sb         $zero, %gp_rel(D_800A3716)($gp)
    /* 75D0 80016DD0 3A0880A3 */  sb         $zero, %gp_rel(D_800A3906)($gp)
    /* 75D4 80016DD4 3E5B000C */  jal        func_80016CF8
    /* 75D8 80016DD8 00000000 */   nop
    /* 75DC 80016DDC 4D65000C */  jal        func_80019534
    /* 75E0 80016DE0 00000000 */   nop
    /* 75E4 80016DE4 B1F4000C */  jal        func_8003D2C4
    /* 75E8 80016DE8 00000000 */   nop
    /* 75EC 80016DEC 1171000C */  jal        func_8001C444
    /* 75F0 80016DF0 00000000 */   nop
    /* 75F4 80016DF4 2D0680A3 */  sb         $zero, %gp_rel(D_800A36F9)($gp)
    /* 75F8 80016DF8 C40580A3 */  sb         $zero, %gp_rel(D_800A3690)($gp)
    /* 75FC 80016DFC 780680A3 */  sb         $zero, %gp_rel(D_800A3744)($gp)
    /* 7600 80016E00 0A80013C */  lui        $at, %hi(D_800A3745)
    /* 7604 80016E04 453720A0 */  sb         $zero, %lo(D_800A3745)($at)
    /* 7608 80016E08 0A80013C */  lui        $at, %hi(D_800A3746)
    /* 760C 80016E0C 463720A0 */  sb         $zero, %lo(D_800A3746)($at)
    /* 7610 80016E10 D11A010C */  jal        func_80046B44
    /* 7614 80016E14 00000000 */   nop
    /* 7618 80016E18 02000224 */  addiu      $v0, $zero, 0x2
    /* 761C 80016E1C 250682A3 */  sb         $v0, %gp_rel(D_800A36F1)($gp)
    /* 7620 80016E20 0A80013C */  lui        $at, %hi(D_800A38C6)
    /* 7624 80016E24 C63820A4 */  sh         $zero, %lo(D_800A38C6)($at)
    /* 7628 80016E28 E40580A3 */  sb         $zero, %gp_rel(D_800A36B0)($gp)
    /* 762C 80016E2C 5C0880A3 */  sb         $zero, %gp_rel(D_800A3928)($gp)
    /* 7630 80016E30 1000BF8F */  lw         $ra, 0x10($sp)
    /* 7634 80016E34 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7638 80016E38 0800E003 */  jr         $ra
    /* 763C 80016E3C 00000000 */   nop
endlabel func_80016D78
