glabel _spu_Fr_
    /* 79460 80088C60 0A80023C */  lui        $v0, %hi(D_800A2CDC)
    /* 79464 80088C64 DC2C428C */  lw         $v0, %lo(D_800A2CDC)($v0)
    /* 79468 80088C68 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 7946C 80088C6C 1400B1AF */  sw         $s1, 0x14($sp)
    /* 79470 80088C70 21888000 */  addu       $s1, $a0, $zero
    /* 79474 80088C74 1000B0AF */  sw         $s0, 0x10($sp)
    /* 79478 80088C78 1800BFAF */  sw         $ra, 0x18($sp)
    /* 7947C 80088C7C A60145A4 */  sh         $a1, 0x1A6($v0)
    /* 79480 80088C80 9B24020C */  jal        _spu_Fw1ts
    /* 79484 80088C84 2180C000 */   addu      $s0, $a2, $zero
    /* 79488 80088C88 0A80033C */  lui        $v1, %hi(D_800A2CDC)
    /* 7948C 80088C8C DC2C638C */  lw         $v1, %lo(D_800A2CDC)($v1)
    /* 79490 80088C90 00000000 */  nop
    /* 79494 80088C94 AA016294 */  lhu        $v0, 0x1AA($v1)
    /* 79498 80088C98 00000000 */  nop
    /* 7949C 80088C9C 30004234 */  ori        $v0, $v0, 0x30
    /* 794A0 80088CA0 AA0162A4 */  sh         $v0, 0x1AA($v1)
    /* 794A4 80088CA4 9B24020C */  jal        _spu_Fw1ts
    /* 794A8 80088CA8 00841000 */   sll       $s0, $s0, 16
    /* 794AC 80088CAC 9024020C */  jal        _spu_FsetDelayR
    /* 794B0 80088CB0 00000000 */   nop
    /* 794B4 80088CB4 0001043C */  lui        $a0, (0x1000200 >> 16)
    /* 794B8 80088CB8 00028434 */  ori        $a0, $a0, (0x1000200 & 0xFFFF)
    /* 794BC 80088CBC 0A80023C */  lui        $v0, %hi(D_800A2CE0)
    /* 794C0 80088CC0 E02C428C */  lw         $v0, %lo(D_800A2CE0)($v0)
    /* 794C4 80088CC4 00000000 */  nop
    /* 794C8 80088CC8 000051AC */  sw         $s1, 0x0($v0)
    /* 794CC 80088CCC 0A80023C */  lui        $v0, %hi(D_800A2CE4)
    /* 794D0 80088CD0 E42C428C */  lw         $v0, %lo(D_800A2CE4)($v0)
    /* 794D4 80088CD4 10001036 */  ori        $s0, $s0, 0x10
    /* 794D8 80088CD8 000050AC */  sw         $s0, 0x0($v0)
    /* 794DC 80088CDC 0A80033C */  lui        $v1, %hi(D_800A2CE8)
    /* 794E0 80088CE0 E82C638C */  lw         $v1, %lo(D_800A2CE8)($v1)
    /* 794E4 80088CE4 01000224 */  addiu      $v0, $zero, 0x1
    /* 794E8 80088CE8 0A80013C */  lui        $at, %hi(D_800A2D2C)
    /* 794EC 80088CEC 2C2D22AC */  sw         $v0, %lo(D_800A2D2C)($at)
    /* 794F0 80088CF0 000064AC */  sw         $a0, 0x0($v1)
    /* 794F4 80088CF4 1800BF8F */  lw         $ra, 0x18($sp)
    /* 794F8 80088CF8 1400B18F */  lw         $s1, 0x14($sp)
    /* 794FC 80088CFC 1000B08F */  lw         $s0, 0x10($sp)
    /* 79500 80088D00 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 79504 80088D04 0800E003 */  jr         $ra
    /* 79508 80088D08 00000000 */   nop
endlabel _spu_Fr_
