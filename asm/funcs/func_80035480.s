glabel func_80035480
    /* 25C80 80035480 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 25C84 80035484 1000BFAF */  sw         $ra, 0x10($sp)
    /* 25C88 80035488 1A5A000C */  jal        gpu_EnableDisplay
    /* 25C8C 8003548C 00000000 */   nop
    /* 25C90 80035490 225A000C */  jal        gpu_InitDisplay
    /* 25C94 80035494 00000000 */   nop
    /* 25C98 80035498 0E018293 */  lbu        $v0, %gp_rel(D_800A31DA)($gp)
    /* 25C9C 8003549C 00000000 */  nop
    /* 25CA0 800354A0 03004014 */  bnez       $v0, .L800354B0
    /* 25CA4 800354A4 00000000 */   nop
    /* 25CA8 800354A8 07E9000C */  jal        func_8003A41C
    /* 25CAC 800354AC 00000000 */   nop
  .L800354B0:
    /* 25CB0 800354B0 3783000C */  jal        func_80020CDC
    /* 25CB4 800354B4 00000000 */   nop
    /* 25CB8 800354B8 7105010C */  jal        player_Destroy
    /* 25CBC 800354BC 21200000 */   addu      $a0, $zero, $zero
    /* 25CC0 800354C0 7105010C */  jal        player_Destroy
    /* 25CC4 800354C4 01000424 */   addiu     $a0, $zero, 0x1
    /* 25CC8 800354C8 1D5B000C */  jal        file_ResetDmaFlag
    /* 25CCC 800354CC 00000000 */   nop
    /* 25CD0 800354D0 0C018293 */  lbu        $v0, %gp_rel(D_800A31D8)($gp)
    /* 25CD4 800354D4 00000000 */  nop
    /* 25CD8 800354D8 07004010 */  beqz       $v0, .L800354F8
    /* 25CDC 800354DC 01000224 */   addiu     $v0, $zero, 0x1
    /* 25CE0 800354E0 CB6D010C */  jal        obj_InitAll
    /* 25CE4 800354E4 00000000 */   nop
    /* 25CE8 800354E8 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 25CEC 800354EC 0A80013C */  lui        $at, %hi(D_800A390E)
    /* 25CF0 800354F0 0E3922A0 */  sb         $v0, %lo(D_800A390E)($at)
    /* 25CF4 800354F4 01000224 */  addiu      $v0, $zero, 0x1
  .L800354F8:
    /* 25CF8 800354F8 0C0182A3 */  sb         $v0, %gp_rel(D_800A31D8)($gp)
    /* 25CFC 800354FC 0ED5000C */  jal        func_80035438
    /* 25D00 80035500 00000000 */   nop
    /* 25D04 80035504 1180043C */  lui        $a0, (0x80118800 >> 16)
    /* 25D08 80035508 08DE010C */  jal        func_80077820
    /* 25D0C 8003550C 00888434 */   ori       $a0, $a0, (0x80118800 & 0xFFFF)
    /* 25D10 80035510 09000224 */  addiu      $v0, $zero, 0x9
    /* 25D14 80035514 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* 25D18 80035518 B83720AC */  sw         $zero, %lo(D_800A37B8)($at)
    /* 25D1C 8003551C 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 25D20 80035520 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 25D24 80035524 345A000C */  jal        gpu_DisableDisplay
    /* 25D28 80035528 00000000 */   nop
    /* 25D2C 8003552C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 25D30 80035530 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 25D34 80035534 0800E003 */  jr         $ra
    /* 25D38 80035538 00000000 */   nop
endlabel func_80035480
