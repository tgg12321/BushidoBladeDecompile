glabel func_8007B244
    /* 6BA44 8007B244 0A80023C */  lui        $v0, %hi(D_8009BE76)
    /* 6BA48 8007B248 76BE4290 */  lbu        $v0, %lo(D_8009BE76)($v0)
    /* 6BA4C 8007B24C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6BA50 8007B250 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6BA54 8007B254 21808000 */  addu       $s0, $a0, $zero
    /* 6BA58 8007B258 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6BA5C 8007B25C 07004014 */  bnez       $v0, .L8007B27C
    /* 6BA60 8007B260 1400BFAF */   sw        $ra, 0x14($sp)
    /* 6BA64 8007B264 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6BA68 8007B268 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6BA6C 8007B26C 0180043C */  lui        $a0, %hi(D_80015EE8)
    /* 6BA70 8007B270 E85E8424 */  addiu      $a0, $a0, %lo(D_80015EE8)
    /* 6BA74 8007B274 09F84000 */  jalr       $v0
    /* 6BA78 8007B278 21280002 */   addu      $a1, $s0, $zero
  .L8007B27C:
    /* 6BA7C 8007B27C 0A80023C */  lui        $v0, %hi(D_8009BE80)
    /* 6BA80 8007B280 80BE428C */  lw         $v0, %lo(D_8009BE80)($v0)
    /* 6BA84 8007B284 0A80013C */  lui        $at, %hi(D_8009BE80)
    /* 6BA88 8007B288 80BE30AC */  sw         $s0, %lo(D_8009BE80)($at)
    /* 6BA8C 8007B28C 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6BA90 8007B290 1000B08F */  lw         $s0, 0x10($sp)
    /* 6BA94 8007B294 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6BA98 8007B298 0800E003 */  jr         $ra
    /* 6BA9C 8007B29C 00000000 */   nop
endlabel func_8007B244
