glabel func_8003AF40
    /* 2B740 8003AF40 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 2B744 8003AF44 1800B0AF */  sw         $s0, 0x18($sp)
    /* 2B748 8003AF48 21808000 */  addu       $s0, $a0, $zero
    /* 2B74C 8003AF4C 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* 2B750 8003AF50 1080013C */  lui        $at, %hi(D_8010277E)
    /* 2B754 8003AF54 21083000 */  addu       $at, $at, $s0
    /* 2B758 8003AF58 7E272390 */  lbu        $v1, %lo(D_8010277E)($at)
    /* 2B75C 8003AF5C FF000224 */  addiu      $v0, $zero, 0xFF
    /* 2B760 8003AF60 07006214 */  bne        $v1, $v0, .L8003AF80
    /* 2B764 8003AF64 0100022E */   sltiu     $v0, $s0, 0x1
    /* 2B768 8003AF68 1080013C */  lui        $at, %hi(D_8010277E)
    /* 2B76C 8003AF6C 21082200 */  addu       $at, $at, $v0
    /* 2B770 8003AF70 7E272290 */  lbu        $v0, %lo(D_8010277E)($at)
    /* 2B774 8003AF74 1080013C */  lui        $at, %hi(D_8010277E)
    /* 2B778 8003AF78 21083000 */  addu       $at, $at, $s0
    /* 2B77C 8003AF7C 7E2722A0 */  sb         $v0, %lo(D_8010277E)($at)
  .L8003AF80:
    /* 2B780 8003AF80 1080013C */  lui        $at, %hi(D_80102780)
    /* 2B784 8003AF84 21083000 */  addu       $at, $at, $s0
    /* 2B788 8003AF88 80272580 */  lb         $a1, %lo(D_80102780)($at)
    /* 2B78C 8003AF8C 1080013C */  lui        $at, %hi(D_8010277C)
    /* 2B790 8003AF90 21083000 */  addu       $at, $at, $s0
    /* 2B794 8003AF94 7C272680 */  lb         $a2, %lo(D_8010277C)($at)
    /* 2B798 8003AF98 1080013C */  lui        $at, %hi(D_8010277E)
    /* 2B79C 8003AF9C 21083000 */  addu       $at, $at, $s0
    /* 2B7A0 8003AFA0 7E272780 */  lb         $a3, %lo(D_8010277E)($at)
    /* 2B7A4 8003AFA4 21200002 */  addu       $a0, $s0, $zero
    /* 2B7A8 8003AFA8 6089000C */  jal        func_80022580
    /* 2B7AC 8003AFAC 1000A0AF */   sw        $zero, 0x10($sp)
    /* 2B7B0 8003AFB0 1A5A000C */  jal        func_80016868
    /* 2B7B4 8003AFB4 00000000 */   nop
    /* 2B7B8 8003AFB8 4E83000C */  jal        func_80020D38
    /* 2B7BC 8003AFBC 00000000 */   nop
    /* 2B7C0 8003AFC0 21200002 */  addu       $a0, $s0, $zero
    /* 2B7C4 8003AFC4 1080013C */  lui        $at, %hi(D_8010277C)
    /* 2B7C8 8003AFC8 21082400 */  addu       $at, $at, $a0
    /* 2B7CC 8003AFCC 7C272280 */  lb         $v0, %lo(D_8010277C)($at)
    /* 2B7D0 8003AFD0 1980063C */  lui        $a2, (0x80190800 >> 16)
    /* 2B7D4 8003AFD4 0980013C */  lui        $at, %hi(D_8008D578)
    /* 2B7D8 8003AFD8 21082200 */  addu       $at, $at, $v0
    /* 2B7DC 8003AFDC 78D52590 */  lbu        $a1, %lo(D_8008D578)($at)
    /* 2B7E0 8003AFE0 4401010C */  jal        func_80040510
    /* 2B7E4 8003AFE4 0008C634 */   ori       $a2, $a2, (0x80190800 & 0xFFFF)
    /* 2B7E8 8003AFE8 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* 2B7EC 8003AFEC 1800B08F */  lw         $s0, 0x18($sp)
    /* 2B7F0 8003AFF0 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 2B7F4 8003AFF4 0800E003 */  jr         $ra
    /* 2B7F8 8003AFF8 00000000 */   nop
endlabel func_8003AF40
