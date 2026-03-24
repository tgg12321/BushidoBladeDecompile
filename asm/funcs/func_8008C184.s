glabel func_8008C184
    /* 7C984 8008C184 0F80033C */  lui        $v1, %hi(D_800F1AEC)
    /* 7C988 8008C188 EC1A6324 */  addiu      $v1, $v1, %lo(D_800F1AEC)
    /* 7C98C 8008C18C 0000628C */  lw         $v0, 0x0($v1)
    /* 7C990 8008C190 00000000 */  nop
    /* 7C994 8008C194 12004014 */  bnez       $v0, .L8008C1E0
    /* 7C998 8008C198 FFFF0224 */   addiu     $v0, $zero, -0x1
    /* 7C99C 8008C19C 0F80013C */  lui        $at, %hi(D_800F1AF4)
    /* 7C9A0 8008C1A0 F41A25AC */  sw         $a1, %lo(D_800F1AF4)($at)
    /* 7C9A4 8008C1A4 0F80013C */  lui        $at, %hi(D_800F1AF0)
    /* 7C9A8 8008C1A8 F01A24AC */  sw         $a0, %lo(D_800F1AF0)($at)
    /* 7C9AC 8008C1AC 0A80043C */  lui        $a0, %hi(D_800A3044)
    /* 7C9B0 8008C1B0 4430848C */  lw         $a0, %lo(D_800A3044)($a0)
    /* 7C9B4 8008C1B4 01000224 */  addiu      $v0, $zero, 0x1
    /* 7C9B8 8008C1B8 000062AC */  sw         $v0, 0x0($v1)
    /* 7C9BC 8008C1BC 04008294 */  lhu        $v0, 0x4($a0)
    /* 7C9C0 8008C1C0 00000000 */  nop
    /* 7C9C4 8008C1C4 80004230 */  andi       $v0, $v0, 0x80
    /* 7C9C8 8008C1C8 0F80013C */  lui        $at, %hi(D_800F1AF8)
    /* 7C9CC 8008C1CC F81A22AC */  sw         $v0, %lo(D_800F1AF8)($at)
    /* 7C9D0 8008C1D0 0A008394 */  lhu        $v1, 0xA($a0)
    /* 7C9D4 8008C1D4 21100000 */  addu       $v0, $zero, $zero
    /* 7C9D8 8008C1D8 00046334 */  ori        $v1, $v1, 0x400
    /* 7C9DC 8008C1DC 0A0083A4 */  sh         $v1, 0xA($a0)
  .L8008C1E0:
    /* 7C9E0 8008C1E0 0800E003 */  jr         $ra
    /* 7C9E4 8008C1E4 00000000 */   nop
endlabel func_8008C184
