glabel func_8008BE04
    /* 7C604 8008BE04 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 7C608 8008BE08 1400BFAF */  sw         $ra, 0x14($sp)
    /* 7C60C 8008BE0C 6EE2010C */  jal        EnterCriticalSection
    /* 7C610 8008BE10 1000B0AF */   sw        $s0, 0x10($sp)
    /* 7C614 8008BE14 0A80043C */  lui        $a0, %hi(D_800A307C)
    /* 7C618 8008BE18 7C308424 */  addiu      $a0, $a0, %lo(D_800A307C)
    /* 7C61C 8008BE1C 1434020C */  jal        func_8008D050
    /* 7C620 8008BE20 21804000 */   addu      $s0, $v0, $zero
    /* 7C624 8008BE24 01000224 */  addiu      $v0, $zero, 0x1
    /* 7C628 8008BE28 03000216 */  bne        $s0, $v0, .L8008BE38
    /* 7C62C 8008BE2C 00000000 */   nop
    /* 7C630 8008BE30 72E2010C */  jal        ExitCriticalSection
    /* 7C634 8008BE34 00000000 */   nop
  .L8008BE38:
    /* 7C638 8008BE38 1400BF8F */  lw         $ra, 0x14($sp)
    /* 7C63C 8008BE3C 1000B08F */  lw         $s0, 0x10($sp)
    /* 7C640 8008BE40 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7C644 8008BE44 0800E003 */  jr         $ra
    /* 7C648 8008BE48 00000000 */   nop
endlabel func_8008BE04
