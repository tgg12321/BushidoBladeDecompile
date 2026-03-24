glabel func_8003B20C
    /* 2BA0C 8003B20C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2BA10 8003B210 01000324 */  addiu      $v1, $zero, 0x1
    /* 2BA14 8003B214 80200400 */  sll        $a0, $a0, 2
    /* 2BA18 8003B218 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2BA1C 8003B21C 1080013C */  lui        $at, %hi(D_80102780)
    /* 2BA20 8003B220 802720A0 */  sb         $zero, %lo(D_80102780)($at)
    /* 2BA24 8003B224 1080013C */  lui        $at, %hi(D_80102781)
    /* 2BA28 8003B228 812723A0 */  sb         $v1, %lo(D_80102781)($at)
    /* 2BA2C 8003B22C 0980013C */  lui        $at, %hi(D_800900EC)
    /* 2BA30 8003B230 21082400 */  addu       $at, $at, $a0
    /* 2BA34 8003B234 EC00248C */  lw         $a0, %lo(D_800900EC)($at)
    /* 2BA38 8003B238 FF000224 */  addiu      $v0, $zero, 0xFF
    /* 2BA3C 8003B23C 0A80013C */  lui        $at, %hi(D_800A3894)
    /* 2BA40 8003B240 943820AC */  sw         $zero, %lo(D_800A3894)($at)
    /* 2BA44 8003B244 0A80013C */  lui        $at, %hi(D_800A385C)
    /* 2BA48 8003B248 5C3820AC */  sw         $zero, %lo(D_800A385C)($at)
    /* 2BA4C 8003B24C 0A80013C */  lui        $at, %hi(D_800A3836)
    /* 2BA50 8003B250 363822A0 */  sb         $v0, %lo(D_800A3836)($at)
    /* 2BA54 8003B254 0A80013C */  lui        $at, %hi(D_800A3915)
    /* 2BA58 8003B258 153922A0 */  sb         $v0, %lo(D_800A3915)($at)
    /* 2BA5C 8003B25C 0A80013C */  lui        $at, %hi(D_800A37C6)
    /* 2BA60 8003B260 C63723A0 */  sb         $v1, %lo(D_800A37C6)($at)
    /* 2BA64 8003B264 0A80013C */  lui        $at, %hi(D_800A37A0)
    /* 2BA68 8003B268 A03720A0 */  sb         $zero, %lo(D_800A37A0)($at)
    /* 2BA6C 8003B26C 0A80013C */  lui        $at, %hi(D_800A37A4)
    /* 2BA70 8003B270 A43720AC */  sw         $zero, %lo(D_800A37A4)($at)
    /* 2BA74 8003B274 0A80013C */  lui        $at, %hi(D_800A3844)
    /* 2BA78 8003B278 443824AC */  sw         $a0, %lo(D_800A3844)($at)
    /* 2BA7C 8003B27C 205B000C */  jal        func_80016C80
    /* 2BA80 8003B280 00000000 */   nop
    /* 2BA84 8003B284 0A80043C */  lui        $a0, %hi(D_800A3844)
    /* 2BA88 8003B288 4438848C */  lw         $a0, %lo(D_800A3844)($a0)
    /* 2BA8C 8003B28C 97EB000C */  jal        func_8003AE5C
    /* 2BA90 8003B290 00000000 */   nop
    /* 2BA94 8003B294 D0EB000C */  jal        func_8003AF40
    /* 2BA98 8003B298 21200000 */   addu      $a0, $zero, $zero
    /* 2BA9C 8003B29C 1080023C */  lui        $v0, %hi(D_8010277C)
    /* 2BAA0 8003B2A0 7C274280 */  lb         $v0, %lo(D_8010277C)($v0)
    /* 2BAA4 8003B2A4 0980013C */  lui        $at, %hi(D_8008D538)
    /* 2BAA8 8003B2A8 21082200 */  addu       $at, $at, $v0
    /* 2BAAC 8003B2AC 38D52290 */  lbu        $v0, %lo(D_8008D538)($at)
    /* 2BAB0 8003B2B0 0A80013C */  lui        $at, %hi(D_800A376C)
    /* 2BAB4 8003B2B4 6C3722A0 */  sb         $v0, %lo(D_800A376C)($at)
    /* 2BAB8 8003B2B8 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2BABC 8003B2BC 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2BAC0 8003B2C0 0800E003 */  jr         $ra
    /* 2BAC4 8003B2C4 00000000 */   nop
endlabel func_8003B20C
