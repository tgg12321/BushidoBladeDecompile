glabel func_8001B3C0
    /* BBC0 8001B3C0 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* BBC4 8001B3C4 1000B0AF */  sw         $s0, 0x10($sp)
    /* BBC8 8001B3C8 21808000 */  addu       $s0, $a0, $zero
    /* BBCC 8001B3CC 1400B1AF */  sw         $s1, 0x14($sp)
    /* BBD0 8001B3D0 2188A000 */  addu       $s1, $a1, $zero
    /* BBD4 8001B3D4 64000224 */  addiu      $v0, $zero, 0x64
    /* BBD8 8001B3D8 1800BFAF */  sw         $ra, 0x18($sp)
    /* BBDC 8001B3DC 0A80013C */  lui        $at, %hi(D_800A36FA)
    /* BBE0 8001B3E0 FA3620A0 */  sb         $zero, %lo(D_800A36FA)($at)
    /* BBE4 8001B3E4 0F80013C */  lui        $at, %hi(D_800F5358)
    /* BBE8 8001B3E8 585322A4 */  sh         $v0, %lo(D_800F5358)($at)
    /* BBEC 8001B3EC 0F80013C */  lui        $at, %hi(D_800F535A)
    /* BBF0 8001B3F0 5A5320A4 */  sh         $zero, %lo(D_800F535A)($at)
    /* BBF4 8001B3F4 0F80013C */  lui        $at, %hi(D_800F535C)
    /* BBF8 8001B3F8 5C5322A4 */  sh         $v0, %lo(D_800F535C)($at)
    /* BBFC 8001B3FC 0F80013C */  lui        $at, %hi(D_800F5360)
    /* BC00 8001B400 605322A4 */  sh         $v0, %lo(D_800F5360)($at)
    /* BC04 8001B404 0F80013C */  lui        $at, %hi(D_800F5362)
    /* BC08 8001B408 625320A4 */  sh         $zero, %lo(D_800F5362)($at)
    /* BC0C 8001B40C 0F80013C */  lui        $at, %hi(D_800F5364)
    /* BC10 8001B410 645322A4 */  sh         $v0, %lo(D_800F5364)($at)
    /* BC14 8001B414 79FC000C */  jal        func_8003F1E4
    /* BC18 8001B418 21200000 */   addu      $a0, $zero, $zero
    /* BC1C 8001B41C 0A80023C */  lui        $v0, %hi(D_800A36F6)
    /* BC20 8001B420 F6364284 */  lh         $v0, %lo(D_800A36F6)($v0)
    /* BC24 8001B424 00000000 */  nop
    /* BC28 8001B428 02004010 */  beqz       $v0, .L8001B434
    /* BC2C 8001B42C 00000000 */   nop
    /* BC30 8001B430 21802002 */  addu       $s0, $s1, $zero
  .L8001B434:
    /* BC34 8001B434 8001028E */  lw         $v0, 0x180($s0)
    /* BC38 8001B438 0F80013C */  lui        $at, %hi(D_800F5328)
    /* BC3C 8001B43C 285322AC */  sw         $v0, %lo(D_800F5328)($at)
    /* BC40 8001B440 8801028E */  lw         $v0, 0x188($s0)
    /* BC44 8001B444 0F80013C */  lui        $at, %hi(D_800F5330)
    /* BC48 8001B448 305322AC */  sw         $v0, %lo(D_800F5330)($at)
    /* BC4C 8001B44C 8401028E */  lw         $v0, 0x184($s0)
    /* BC50 8001B450 0F80013C */  lui        $at, %hi(D_800F5368)
    /* BC54 8001B454 685320A0 */  sb         $zero, %lo(D_800F5368)($at)
    /* BC58 8001B458 0F80013C */  lui        $at, %hi(D_800F532C)
    /* BC5C 8001B45C 2C5322AC */  sw         $v0, %lo(D_800F532C)($at)
    /* BC60 8001B460 1800BF8F */  lw         $ra, 0x18($sp)
    /* BC64 8001B464 1400B18F */  lw         $s1, 0x14($sp)
    /* BC68 8001B468 1000B08F */  lw         $s0, 0x10($sp)
    /* BC6C 8001B46C 2000BD27 */  addiu      $sp, $sp, 0x20
    /* BC70 8001B470 0800E003 */  jr         $ra
    /* BC74 8001B474 00000000 */   nop
endlabel func_8001B3C0
