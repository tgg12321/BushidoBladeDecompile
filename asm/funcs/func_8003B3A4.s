glabel func_8003B3A4
    /* 2BBA4 8003B3A4 1080023C */  lui        $v0, %hi(D_8010277C)
    /* 2BBA8 8003B3A8 7C274280 */  lb         $v0, %lo(D_8010277C)($v0)
    /* 2BBAC 8003B3AC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2BBB0 8003B3B0 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2BBB4 8003B3B4 0A80013C */  lui        $at, %hi(D_800A3712)
    /* 2BBB8 8003B3B8 123720A0 */  sb         $zero, %lo(D_800A3712)($at)
    /* 2BBBC 8003B3BC 0980013C */  lui        $at, %hi(D_8008D538)
    /* 2BBC0 8003B3C0 21082200 */  addu       $at, $at, $v0
    /* 2BBC4 8003B3C4 38D52290 */  lbu        $v0, %lo(D_8008D538)($at)
    /* 2BBC8 8003B3C8 0980013C */  lui        $at, %hi(D_8008D9EC)
    /* 2BBCC 8003B3CC 21082200 */  addu       $at, $at, $v0
    /* 2BBD0 8003B3D0 ECD92590 */  lbu        $a1, %lo(D_8008D9EC)($at)
    /* 2BBD4 8003B3D4 00000000 */  nop
    /* 2BBD8 8003B3D8 0700A010 */  beqz       $a1, .L8003B3F8
    /* 2BBDC 8003B3DC 01000224 */   addiu     $v0, $zero, 0x1
    /* 2BBE0 8003B3E0 0A80033C */  lui        $v1, %hi(D_800A37A0)
    /* 2BBE4 8003B3E4 A0376390 */  lbu        $v1, %lo(D_800A37A0)($v1)
    /* 2BBE8 8003B3E8 00000000 */  nop
    /* 2BBEC 8003B3EC 02006214 */  bne        $v1, $v0, .L8003B3F8
    /* 2BBF0 8003B3F0 00000000 */   nop
    /* 2BBF4 8003B3F4 21280000 */  addu       $a1, $zero, $zero
  .L8003B3F8:
    /* 2BBF8 8003B3F8 1080033C */  lui        $v1, %hi(D_8010277D)
    /* 2BBFC 8003B3FC 7D276324 */  addiu      $v1, $v1, %lo(D_8010277D)
    /* 2BC00 8003B400 0300A010 */  beqz       $a1, .L8003B410
    /* 2BC04 8003B404 00000000 */   nop
    /* 2BC08 8003B408 05ED0008 */  j          .L8003B414
    /* 2BC0C 8003B40C 0E000224 */   addiu     $v0, $zero, 0xE
  .L8003B410:
    /* 2BC10 8003B410 1D000224 */  addiu      $v0, $zero, 0x1D
  .L8003B414:
    /* 2BC14 8003B414 000062A0 */  sb         $v0, 0x0($v1)
    /* 2BC18 8003B418 1080013C */  lui        $at, %hi(D_8010277F)
    /* 2BC1C 8003B41C 7F2720A0 */  sb         $zero, %lo(D_8010277F)($at)
    /* 2BC20 8003B420 00008290 */  lbu        $v0, 0x0($a0)
    /* 2BC24 8003B424 0A80013C */  lui        $at, %hi(D_800A3680)
    /* 2BC28 8003B428 803622A0 */  sb         $v0, %lo(D_800A3680)($at)
    /* 2BC2C 8003B42C 0A80013C */  lui        $at, %hi(D_800A3671)
    /* 2BC30 8003B430 713622A0 */  sb         $v0, %lo(D_800A3671)($at)
    /* 2BC34 8003B434 01008290 */  lbu        $v0, 0x1($a0)
    /* 2BC38 8003B438 1080013C */  lui        $at, %hi(D_80102783)
    /* 2BC3C 8003B43C 832722A0 */  sb         $v0, %lo(D_80102783)($at)
    /* 2BC40 8003B440 02008290 */  lbu        $v0, 0x2($a0)
    /* 2BC44 8003B444 0A80013C */  lui        $at, %hi(D_800A37B4)
    /* 2BC48 8003B448 B43722A0 */  sb         $v0, %lo(D_800A37B4)($at)
    /* 2BC4C 8003B44C 03008290 */  lbu        $v0, 0x3($a0)
    /* 2BC50 8003B450 0A80013C */  lui        $at, %hi(D_800A37B5)
    /* 2BC54 8003B454 B53722A0 */  sb         $v0, %lo(D_800A37B5)($at)
    /* 2BC58 8003B458 04008290 */  lbu        $v0, 0x4($a0)
    /* 2BC5C 8003B45C 0A80013C */  lui        $at, %hi(D_800A37B6)
    /* 2BC60 8003B460 B63722A0 */  sb         $v0, %lo(D_800A37B6)($at)
    /* 2BC64 8003B464 D0EB000C */  jal        func_8003AF40
    /* 2BC68 8003B468 01000424 */   addiu     $a0, $zero, 0x1
    /* 2BC6C 8003B46C FFEB000C */  jal        md_menu_logo_exec
    /* 2BC70 8003B470 00000000 */   nop
    /* 2BC74 8003B474 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2BC78 8003B478 05000224 */  addiu      $v0, $zero, 0x5
    /* 2BC7C 8003B47C 0800E003 */  jr         $ra
    /* 2BC80 8003B480 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_8003B3A4
