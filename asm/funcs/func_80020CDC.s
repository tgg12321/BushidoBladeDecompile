glabel func_80020CDC
    /* 114DC 80020CDC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 114E0 80020CE0 0A80033C */  lui        $v1, %hi(D_800A38C6)
    /* 114E4 80020CE4 C6386394 */  lhu        $v1, %lo(D_800A38C6)($v1)
    /* 114E8 80020CE8 FFFF0234 */  ori        $v0, $zero, 0xFFFF
    /* 114EC 80020CEC 03006214 */  bne        $v1, $v0, .L80020CFC
    /* 114F0 80020CF0 1000BFAF */   sw        $ra, 0x10($sp)
    /* 114F4 80020CF4 6214010C */  jal        func_80045188
    /* 114F8 80020CF8 00000000 */   nop
  .L80020CFC:
    /* 114FC 80020CFC FF000224 */  addiu      $v0, $zero, 0xFF
    /* 11500 80020D00 0A80013C */  lui        $at, %hi(D_800A3880)
    /* 11504 80020D04 803820A0 */  sb         $zero, %lo(D_800A3880)($at)
    /* 11508 80020D08 0A80013C */  lui        $at, %hi(D_800A38C6)
    /* 1150C 80020D0C C63820A4 */  sh         $zero, %lo(D_800A38C6)($at)
    /* 11510 80020D10 0A80013C */  lui        $at, %hi(D_800A38C4)
    /* 11514 80020D14 C43820A4 */  sh         $zero, %lo(D_800A38C4)($at)
    /* 11518 80020D18 0A80013C */  lui        $at, %hi(D_800A38C1)
    /* 1151C 80020D1C C13822A0 */  sb         $v0, %lo(D_800A38C1)($at)
    /* 11520 80020D20 0A80013C */  lui        $at, %hi(D_800A38C0)
    /* 11524 80020D24 C03822A0 */  sb         $v0, %lo(D_800A38C0)($at)
    /* 11528 80020D28 1000BF8F */  lw         $ra, 0x10($sp)
    /* 1152C 80020D2C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 11530 80020D30 0800E003 */  jr         $ra
    /* 11534 80020D34 00000000 */   nop
endlabel func_80020CDC
