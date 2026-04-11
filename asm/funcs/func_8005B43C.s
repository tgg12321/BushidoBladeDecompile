glabel func_8005B43C
    /* 4BC3C 8005B43C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 4BC40 8005B440 1000BFAF */  sw         $ra, 0x10($sp)
    /* 4BC44 8005B444 21280000 */  addu       $a1, $zero, $zero
    /* 4BC48 8005B448 0F80043C */  lui        $a0, %hi(D_800EFB38)
    /* 4BC4C 8005B44C 38FB8424 */  addiu      $a0, $a0, %lo(D_800EFB38)
    /* 4BC50 8005B450 0F80033C */  lui        $v1, %hi(D_800EFC38)
    /* 4BC54 8005B454 38FC6324 */  addiu      $v1, $v1, %lo(D_800EFC38)
  .L8005B458:
    /* 4BC58 8005B458 000060AC */  sw         $zero, 0x0($v1)
    /* 4BC5C 8005B45C 000080AC */  sw         $zero, 0x0($a0)
    /* 4BC60 8005B460 04008424 */  addiu      $a0, $a0, 0x4
    /* 4BC64 8005B464 0100A524 */  addiu      $a1, $a1, 0x1
    /* 4BC68 8005B468 1000A228 */  slti       $v0, $a1, 0x10
    /* 4BC6C 8005B46C FAFF4014 */  bnez       $v0, .L8005B458
    /* 4BC70 8005B470 04006324 */   addiu     $v1, $v1, 0x4
    /* 4BC74 8005B474 860E020C */  jal        sys_Shutdown
    /* 4BC78 8005B478 00000000 */   nop
    /* 4BC7C 8005B47C 3416020C */  jal        title_mv_exec2
    /* 4BC80 8005B480 21200000 */   addu      $a0, $zero, $zero
    /* 4BC84 8005B484 E617020C */  jal        func_80085F98
    /* 4BC88 8005B488 00000000 */   nop
    /* 4BC8C 8005B48C B917020C */  jal        func_80085EE4
    /* 4BC90 8005B490 21200000 */   addu      $a0, $zero, $zero
    /* 4BC94 8005B494 21200000 */  addu       $a0, $zero, $zero
    /* 4BC98 8005B498 9317020C */  jal        func_80085E4C
    /* 4BC9C 8005B49C 21280000 */   addu      $a1, $zero, $zero
    /* 4BCA0 8005B4A0 CD1F020C */  jal        memcard_SetSlot
    /* 4BCA4 8005B4A4 21200000 */   addu      $a0, $zero, $zero
    /* 4BCA8 8005B4A8 5115020C */  jal        SetBloodSpot
    /* 4BCAC 8005B4AC 01000424 */   addiu     $a0, $zero, 0x1
    /* 4BCB0 8005B4B0 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 4BCB4 8005B4B4 0F80043C */  lui        $a0, %hi(D_800EFB78)
    /* 4BCB8 8005B4B8 78FB8424 */  addiu      $a0, $a0, %lo(D_800EFB78)
    /* 4BCBC 8005B4BC 21180000 */  addu       $v1, $zero, $zero
  .L8005B4C0:
    /* 4BCC0 8005B4C0 0F80013C */  lui        $at, %hi(D_800EFB78)
    /* 4BCC4 8005B4C4 21082300 */  addu       $at, $at, $v1
    /* 4BCC8 8005B4C8 78FB20AC */  sw         $zero, %lo(D_800EFB78)($at)
    /* 4BCCC 8005B4CC 050085A0 */  sb         $a1, 0x5($a0)
    /* 4BCD0 8005B4D0 0F80013C */  lui        $at, %hi(D_800EFB7C)
    /* 4BCD4 8005B4D4 21082300 */  addu       $at, $at, $v1
    /* 4BCD8 8005B4D8 7CFB25A0 */  sb         $a1, %lo(D_800EFB7C)($at)
    /* 4BCDC 8005B4DC 08006324 */  addiu      $v1, $v1, 0x8
    /* 4BCE0 8005B4E0 C0006228 */  slti       $v0, $v1, 0xC0
    /* 4BCE4 8005B4E4 F6FF4014 */  bnez       $v0, .L8005B4C0
    /* 4BCE8 8005B4E8 08008424 */   addiu     $a0, $a0, 0x8
    /* 4BCEC 8005B4EC A70F020C */  jal        DispStuff
    /* 4BCF0 8005B4F0 00000000 */   nop
    /* 4BCF4 8005B4F4 3C0380AF */  sw         $zero, %gp_rel(D_800A3408)($gp)
    /* 4BCF8 8005B4F8 340380A7 */  sh         $zero, %gp_rel(D_800A3400)($gp)
    /* 4BCFC 8005B4FC 1000BF8F */  lw         $ra, 0x10($sp)
    /* 4BD00 8005B500 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 4BD04 8005B504 0800E003 */  jr         $ra
    /* 4BD08 8005B508 00000000 */   nop
endlabel func_8005B43C
