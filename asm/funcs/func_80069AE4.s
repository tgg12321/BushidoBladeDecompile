glabel func_80069AE4
    /* 5A2E4 80069AE4 A0FFBD27 */  addiu      $sp, $sp, -0x60
    /* 5A2E8 80069AE8 5800B4AF */  sw         $s4, 0x58($sp)
    /* 5A2EC 80069AEC 21A08000 */  addu       $s4, $a0, $zero
    /* 5A2F0 80069AF0 02000224 */  addiu      $v0, $zero, 0x2
    /* 5A2F4 80069AF4 5C00BFAF */  sw         $ra, 0x5C($sp)
    /* 5A2F8 80069AF8 5400B3AF */  sw         $s3, 0x54($sp)
    /* 5A2FC 80069AFC 5000B2AF */  sw         $s2, 0x50($sp)
    /* 5A300 80069B00 4C00B1AF */  sw         $s1, 0x4C($sp)
    /* 5A304 80069B04 4800B0AF */  sw         $s0, 0x48($sp)
    /* 5A308 80069B08 1800938E */  lw         $s3, 0x18($s4)
    /* 5A30C 80069B0C 2200A214 */  bne        $a1, $v0, .L80069B98
    /* 5A310 80069B10 01000224 */   addiu     $v0, $zero, 0x1
    /* 5A314 80069B14 B4EA010C */  jal        initTile
    /* 5A318 80069B18 21206002 */   addu      $a0, $s3, $zero
    /* 5A31C 80069B1C 8CA6010C */  jal        func_80069A30
    /* 5A320 80069B20 21206002 */   addu      $a0, $s3, $zero
    /* 5A324 80069B24 21206002 */  addu       $a0, $s3, $zero
    /* 5A328 80069B28 01000524 */  addiu      $a1, $zero, 0x1
    /* 5A32C 80069B2C 4E000224 */  addiu      $v0, $zero, 0x4E
    /* 5A330 80069B30 30001224 */  addiu      $s2, $zero, 0x30
    /* 5A334 80069B34 CC001124 */  addiu      $s1, $zero, 0xCC
    /* 5A338 80069B38 B0001024 */  addiu      $s0, $zero, 0xB0
    /* 5A33C 80069B3C 080062A6 */  sh         $v0, 0x8($s3)
    /* 5A340 80069B40 0A0072A6 */  sh         $s2, 0xA($s3)
    /* 5A344 80069B44 0C0071A6 */  sh         $s1, 0xC($s3)
    /* 5A348 80069B48 5AEA010C */  jal        gpu_SetSemiTransp
    /* 5A34C 80069B4C 0E0070A6 */   sh        $s0, 0xE($s3)
    /* 5A350 80069B50 21286002 */  addu       $a1, $s3, $zero
    /* 5A354 80069B54 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5A358 80069B58 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5A35C 80069B5C 10007326 */  addiu      $s3, $s3, 0x10
    /* 5A360 80069B60 2DEA010C */  jal        ot_Link
    /* 5A364 80069B64 44008424 */   addiu     $a0, $a0, 0x44
    /* 5A368 80069B68 B4EA010C */  jal        initTile
    /* 5A36C 80069B6C 21206002 */   addu      $a0, $s3, $zero
    /* 5A370 80069B70 8CA6010C */  jal        func_80069A30
    /* 5A374 80069B74 21206002 */   addu      $a0, $s3, $zero
    /* 5A378 80069B78 21206002 */  addu       $a0, $s3, $zero
    /* 5A37C 80069B7C 01000524 */  addiu      $a1, $zero, 0x1
    /* 5A380 80069B80 66010224 */  addiu      $v0, $zero, 0x166
    /* 5A384 80069B84 080062A6 */  sh         $v0, 0x8($s3)
    /* 5A388 80069B88 0A0072A6 */  sh         $s2, 0xA($s3)
    /* 5A38C 80069B8C 0C0071A6 */  sh         $s1, 0xC($s3)
    /* 5A390 80069B90 04A70108 */  j          .L80069C10
    /* 5A394 80069B94 0E0070A6 */   sh        $s0, 0xE($s3)
  .L80069B98:
    /* 5A398 80069B98 0F00A214 */  bne        $a1, $v0, .L80069BD8
    /* 5A39C 80069B9C 00000000 */   nop
    /* 5A3A0 80069BA0 B4EA010C */  jal        initTile
    /* 5A3A4 80069BA4 21206002 */   addu      $a0, $s3, $zero
    /* 5A3A8 80069BA8 8CA6010C */  jal        func_80069A30
    /* 5A3AC 80069BAC 21206002 */   addu      $a0, $s3, $zero
    /* 5A3B0 80069BB0 21206002 */  addu       $a0, $s3, $zero
    /* 5A3B4 80069BB4 01000524 */  addiu      $a1, $zero, 0x1
    /* 5A3B8 80069BB8 3F000224 */  addiu      $v0, $zero, 0x3F
    /* 5A3BC 80069BBC 080062A6 */  sh         $v0, 0x8($s3)
    /* 5A3C0 80069BC0 30000224 */  addiu      $v0, $zero, 0x30
    /* 5A3C4 80069BC4 0A0062A6 */  sh         $v0, 0xA($s3)
    /* 5A3C8 80069BC8 02020224 */  addiu      $v0, $zero, 0x202
    /* 5A3CC 80069BCC 0C0062A6 */  sh         $v0, 0xC($s3)
    /* 5A3D0 80069BD0 03A70108 */  j          .L80069C0C
    /* 5A3D4 80069BD4 B0000224 */   addiu     $v0, $zero, 0xB0
  .L80069BD8:
    /* 5A3D8 80069BD8 B4EA010C */  jal        initTile
    /* 5A3DC 80069BDC 21206002 */   addu      $a0, $s3, $zero
    /* 5A3E0 80069BE0 8CA6010C */  jal        func_80069A30
    /* 5A3E4 80069BE4 21206002 */   addu      $a0, $s3, $zero
    /* 5A3E8 80069BE8 21206002 */  addu       $a0, $s3, $zero
    /* 5A3EC 80069BEC 01000524 */  addiu      $a1, $zero, 0x1
    /* 5A3F0 80069BF0 30010224 */  addiu      $v0, $zero, 0x130
    /* 5A3F4 80069BF4 080062A6 */  sh         $v0, 0x8($s3)
    /* 5A3F8 80069BF8 3A000224 */  addiu      $v0, $zero, 0x3A
    /* 5A3FC 80069BFC 0A0062A6 */  sh         $v0, 0xA($s3)
    /* 5A400 80069C00 26010224 */  addiu      $v0, $zero, 0x126
    /* 5A404 80069C04 0C0062A6 */  sh         $v0, 0xC($s3)
    /* 5A408 80069C08 AB000224 */  addiu      $v0, $zero, 0xAB
  .L80069C0C:
    /* 5A40C 80069C0C 0E0062A6 */  sh         $v0, 0xE($s3)
  .L80069C10:
    /* 5A410 80069C10 5AEA010C */  jal        gpu_SetSemiTransp
    /* 5A414 80069C14 21880000 */   addu      $s1, $zero, $zero
    /* 5A418 80069C18 21286002 */  addu       $a1, $s3, $zero
    /* 5A41C 80069C1C 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5A420 80069C20 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5A424 80069C24 10007326 */  addiu      $s3, $s3, 0x10
    /* 5A428 80069C28 2DEA010C */  jal        ot_Link
    /* 5A42C 80069C2C 44008424 */   addiu     $a0, $a0, 0x44
    /* 5A430 80069C30 12000224 */  addiu      $v0, $zero, 0x12
    /* 5A434 80069C34 180093AE */  sw         $s3, 0x18($s4)
    /* 5A438 80069C38 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 5A43C 80069C3C 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5A440 80069C40 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5A444 80069C44 0400828E */  lw         $v0, 0x4($s4)
    /* 5A448 80069C48 00000000 */  nop
    /* 5A44C 80069C4C 3400528C */  lw         $s2, 0x34($v0)
    /* 5A450 80069C50 3000A0AF */  sw         $zero, 0x30($sp)
    /* 5A454 80069C54 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5A458 80069C58 21804002 */  addu       $s0, $s2, $zero
  .L80069C5C:
    /* 5A45C 80069C5C 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5A460 80069C60 0000028E */  lw         $v0, 0x0($s0)
    /* 5A464 80069C64 04001026 */  addiu      $s0, $s0, 0x4
    /* 5A468 80069C68 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5A46C 80069C6C 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5A470 80069C70 1C00A2AF */  sw         $v0, 0x1C($sp)
    /* 5A474 80069C74 1400828E */  lw         $v0, 0x14($s4)
    /* 5A478 80069C78 01003126 */  addiu      $s1, $s1, 0x1
    /* 5A47C 80069C7C 4BCD010C */  jal        func_8007352C
    /* 5A480 80069C80 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5A484 80069C84 140082AE */  sw         $v0, 0x14($s4)
    /* 5A488 80069C88 0300222A */  slti       $v0, $s1, 0x3
    /* 5A48C 80069C8C F3FF4014 */  bnez       $v0, .L80069C5C
    /* 5A490 80069C90 21280000 */   addu      $a1, $zero, $zero
    /* 5A494 80069C94 0000448E */  lw         $a0, 0x0($s2)
    /* 5A498 80069C98 20B9010C */  jal        func_8006E480
    /* 5A49C 80069C9C 1800A4AF */   sw        $a0, 0x18($sp)
    /* 5A4A0 80069CA0 01000524 */  addiu      $a1, $zero, 0x1
    /* 5A4A4 80069CA4 21300000 */  addu       $a2, $zero, $zero
    /* 5A4A8 80069CA8 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5A4AC 80069CAC 1C00848E */  lw         $a0, 0x1C($s4)
    /* 5A4B0 80069CB0 92F0010C */  jal        initTexPage
    /* 5A4B4 80069CB4 21384000 */   addu      $a3, $v0, $zero
    /* 5A4B8 80069CB8 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5A4BC 80069CBC 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5A4C0 80069CC0 1C00858E */  lw         $a1, 0x1C($s4)
    /* 5A4C4 80069CC4 2DEA010C */  jal        ot_Link
    /* 5A4C8 80069CC8 48008424 */   addiu     $a0, $a0, 0x48
    /* 5A4CC 80069CCC 0C00908E */  lw         $s0, 0xC($s4)
    /* 5A4D0 80069CD0 1C00828E */  lw         $v0, 0x1C($s4)
    /* 5A4D4 80069CD4 21200002 */  addu       $a0, $s0, $zero
    /* 5A4D8 80069CD8 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5A4DC 80069CDC 82EA010C */  jal        initPolyF4
    /* 5A4E0 80069CE0 1C0082AE */   sw        $v0, 0x1C($s4)
    /* 5A4E4 80069CE4 A3A6010C */  jal        func_80069A8C
    /* 5A4E8 80069CE8 21200002 */   addu      $a0, $s0, $zero
    /* 5A4EC 80069CEC 21200002 */  addu       $a0, $s0, $zero
    /* 5A4F0 80069CF0 21280000 */  addu       $a1, $zero, $zero
    /* 5A4F4 80069CF4 B9000224 */  addiu      $v0, $zero, 0xB9
    /* 5A4F8 80069CF8 22011324 */  addiu      $s3, $zero, 0x122
    /* 5A4FC 80069CFC EF001224 */  addiu      $s2, $zero, 0xEF
    /* 5A500 80069D00 080000A6 */  sh         $zero, 0x8($s0)
    /* 5A504 80069D04 0A0002A6 */  sh         $v0, 0xA($s0)
    /* 5A508 80069D08 0C0013A6 */  sh         $s3, 0xC($s0)
    /* 5A50C 80069D0C 0E0000A6 */  sh         $zero, 0xE($s0)
    /* 5A510 80069D10 100000A6 */  sh         $zero, 0x10($s0)
    /* 5A514 80069D14 120012A6 */  sh         $s2, 0x12($s0)
    /* 5A518 80069D18 140013A6 */  sh         $s3, 0x14($s0)
    /* 5A51C 80069D1C 5AEA010C */  jal        gpu_SetSemiTransp
    /* 5A520 80069D20 160012A6 */   sh        $s2, 0x16($s0)
    /* 5A524 80069D24 21280002 */  addu       $a1, $s0, $zero
    /* 5A528 80069D28 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5A52C 80069D2C 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5A530 80069D30 18001026 */  addiu      $s0, $s0, 0x18
    /* 5A534 80069D34 2DEA010C */  jal        ot_Link
    /* 5A538 80069D38 4C008424 */   addiu     $a0, $a0, 0x4C
    /* 5A53C 80069D3C 82EA010C */  jal        initPolyF4
    /* 5A540 80069D40 21200002 */   addu      $a0, $s0, $zero
    /* 5A544 80069D44 A3A6010C */  jal        func_80069A8C
    /* 5A548 80069D48 21200002 */   addu      $a0, $s0, $zero
    /* 5A54C 80069D4C 21200002 */  addu       $a0, $s0, $zero
    /* 5A550 80069D50 21280000 */  addu       $a1, $zero, $zero
    /* 5A554 80069D54 5E011124 */  addiu      $s1, $zero, 0x15E
    /* 5A558 80069D58 7F020224 */  addiu      $v0, $zero, 0x27F
    /* 5A55C 80069D5C 0C0002A6 */  sh         $v0, 0xC($s0)
    /* 5A560 80069D60 140002A6 */  sh         $v0, 0x14($s0)
    /* 5A564 80069D64 36000224 */  addiu      $v0, $zero, 0x36
    /* 5A568 80069D68 080011A6 */  sh         $s1, 0x8($s0)
    /* 5A56C 80069D6C 0A0000A6 */  sh         $zero, 0xA($s0)
    /* 5A570 80069D70 0E0000A6 */  sh         $zero, 0xE($s0)
    /* 5A574 80069D74 100011A6 */  sh         $s1, 0x10($s0)
    /* 5A578 80069D78 120012A6 */  sh         $s2, 0x12($s0)
    /* 5A57C 80069D7C 5AEA010C */  jal        gpu_SetSemiTransp
    /* 5A580 80069D80 160002A6 */   sh        $v0, 0x16($s0)
    /* 5A584 80069D84 21280002 */  addu       $a1, $s0, $zero
    /* 5A588 80069D88 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5A58C 80069D8C 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5A590 80069D90 18001026 */  addiu      $s0, $s0, 0x18
    /* 5A594 80069D94 2DEA010C */  jal        ot_Link
    /* 5A598 80069D98 4C008424 */   addiu     $a0, $a0, 0x4C
    /* 5A59C 80069D9C 82EA010C */  jal        initPolyF4
    /* 5A5A0 80069DA0 21200002 */   addu      $a0, $s0, $zero
    /* 5A5A4 80069DA4 A3A6010C */  jal        func_80069A8C
    /* 5A5A8 80069DA8 21200002 */   addu      $a0, $s0, $zero
    /* 5A5AC 80069DAC 21200002 */  addu       $a0, $s0, $zero
    /* 5A5B0 80069DB0 21280000 */  addu       $a1, $zero, $zero
    /* 5A5B4 80069DB4 080013A6 */  sh         $s3, 0x8($s0)
    /* 5A5B8 80069DB8 0A0000A6 */  sh         $zero, 0xA($s0)
    /* 5A5BC 80069DBC 0C0011A6 */  sh         $s1, 0xC($s0)
    /* 5A5C0 80069DC0 0E0000A6 */  sh         $zero, 0xE($s0)
    /* 5A5C4 80069DC4 100013A6 */  sh         $s3, 0x10($s0)
    /* 5A5C8 80069DC8 120012A6 */  sh         $s2, 0x12($s0)
    /* 5A5CC 80069DCC 140011A6 */  sh         $s1, 0x14($s0)
    /* 5A5D0 80069DD0 5AEA010C */  jal        gpu_SetSemiTransp
    /* 5A5D4 80069DD4 160012A6 */   sh        $s2, 0x16($s0)
    /* 5A5D8 80069DD8 21280002 */  addu       $a1, $s0, $zero
    /* 5A5DC 80069DDC 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5A5E0 80069DE0 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5A5E4 80069DE4 18001026 */  addiu      $s0, $s0, 0x18
    /* 5A5E8 80069DE8 2DEA010C */  jal        ot_Link
    /* 5A5EC 80069DEC 4C008424 */   addiu     $a0, $a0, 0x4C
    /* 5A5F0 80069DF0 0C0090AE */  sw         $s0, 0xC($s4)
    /* 5A5F4 80069DF4 5C00BF8F */  lw         $ra, 0x5C($sp)
    /* 5A5F8 80069DF8 5800B48F */  lw         $s4, 0x58($sp)
    /* 5A5FC 80069DFC 5400B38F */  lw         $s3, 0x54($sp)
    /* 5A600 80069E00 5000B28F */  lw         $s2, 0x50($sp)
    /* 5A604 80069E04 4C00B18F */  lw         $s1, 0x4C($sp)
    /* 5A608 80069E08 4800B08F */  lw         $s0, 0x48($sp)
    /* 5A60C 80069E0C 6000BD27 */  addiu      $sp, $sp, 0x60
    /* 5A610 80069E10 0800E003 */  jr         $ra
    /* 5A614 80069E14 00000000 */   nop
endlabel func_80069AE4
