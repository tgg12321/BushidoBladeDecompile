glabel func_8008BEA4
    /* 7C6A4 8008BEA4 0F80033C */  lui        $v1, %hi(D_800F1AFC)
    /* 7C6A8 8008BEA8 FC1A6324 */  addiu      $v1, $v1, %lo(D_800F1AFC)
    /* 7C6AC 8008BEAC 0000628C */  lw         $v0, 0x0($v1)
    /* 7C6B0 8008BEB0 00000000 */  nop
    /* 7C6B4 8008BEB4 11004014 */  bnez       $v0, .L8008BEFC
    /* 7C6B8 8008BEB8 FFFF0224 */   addiu     $v0, $zero, -0x1
    /* 7C6BC 8008BEBC 0F80013C */  lui        $at, %hi(D_800F1B04)
    /* 7C6C0 8008BEC0 041B25AC */  sw         $a1, %lo(D_800F1B04)($at)
    /* 7C6C4 8008BEC4 0F80013C */  lui        $at, %hi(D_800F1B00)
    /* 7C6C8 8008BEC8 001B24AC */  sw         $a0, %lo(D_800F1B00)($at)
    /* 7C6CC 8008BECC 0A80043C */  lui        $a0, %hi(D_800A3044)
    /* 7C6D0 8008BED0 4430848C */  lw         $a0, %lo(D_800A3044)($a0)
    /* 7C6D4 8008BED4 01000224 */  addiu      $v0, $zero, 0x1
    /* 7C6D8 8008BED8 000062AC */  sw         $v0, 0x0($v1)
    /* 7C6DC 8008BEDC 0A008294 */  lhu        $v0, 0xA($a0)
    /* 7C6E0 8008BEE0 00000000 */  nop
    /* 7C6E4 8008BEE4 00084234 */  ori        $v0, $v0, 0x800
    /* 7C6E8 8008BEE8 0A0082A4 */  sh         $v0, 0xA($a0)
    /* 7C6EC 8008BEEC 0A008394 */  lhu        $v1, 0xA($a0)
    /* 7C6F0 8008BEF0 21100000 */  addu       $v0, $zero, $zero
    /* 7C6F4 8008BEF4 20006334 */  ori        $v1, $v1, 0x20
    /* 7C6F8 8008BEF8 0A0083A4 */  sh         $v1, 0xA($a0)
  .L8008BEFC:
    /* 7C6FC 8008BEFC 0800E003 */  jr         $ra
    /* 7C700 8008BF00 00000000 */   nop
endlabel func_8008BEA4
