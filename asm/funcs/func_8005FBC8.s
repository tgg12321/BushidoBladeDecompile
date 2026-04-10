glabel func_8005FBC8
    /* 503C8 8005FBC8 D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* 503CC 8005FBCC 2400B1AF */  sw         $s1, 0x24($sp)
    /* 503D0 8005FBD0 2188A000 */  addu       $s1, $a1, $zero
    /* 503D4 8005FBD4 33008524 */  addiu      $a1, $a0, 0x33
    /* 503D8 8005FBD8 02000424 */  addiu      $a0, $zero, 0x2
    /* 503DC 8005FBDC 2800BFAF */  sw         $ra, 0x28($sp)
    /* 503E0 8005FBE0 AADB000C */  jal        func_80036EA8
    /* 503E4 8005FBE4 2000B0AF */   sw        $s0, 0x20($sp)
    /* 503E8 8005FBE8 21804000 */  addu       $s0, $v0, $zero
    /* 503EC 8005FBEC 21200002 */  addu       $a0, $s0, $zero
    /* 503F0 8005FBF0 66DB000C */  jal        replay_camera_Init
    /* 503F4 8005FBF4 21282002 */   addu      $a1, $s1, $zero
    /* 503F8 8005FBF8 D0DB000C */  jal        game_FrameLoop
    /* 503FC 8005FBFC 00000000 */   nop
    /* 50400 8005FC00 CADB000C */  jal        func_80036F28
    /* 50404 8005FC04 21200002 */   addu      $a0, $s0, $zero
    /* 50408 8005FC08 0A80053C */  lui        $a1, %hi(D_800A327C)
    /* 5040C 8005FC0C 7C32A524 */  addiu      $a1, $a1, %lo(D_800A327C)
    /* 50410 8005FC10 0300A288 */  lwl        $v0, 0x3($a1)
    /* 50414 8005FC14 0000A298 */  lwr        $v0, 0x0($a1)
    /* 50418 8005FC18 0700A388 */  lwl        $v1, 0x7($a1)
    /* 5041C 8005FC1C 0400A398 */  lwr        $v1, 0x4($a1)
    /* 50420 8005FC20 1300A2AB */  swl        $v0, 0x13($sp)
    /* 50424 8005FC24 1000A2BB */  swr        $v0, 0x10($sp)
    /* 50428 8005FC28 1700A3AB */  swl        $v1, 0x17($sp)
    /* 5042C 8005FC2C 1400A3BB */  swr        $v1, 0x14($sp)
    /* 50430 8005FC30 0A80053C */  lui        $a1, %hi(D_800A3284)
    /* 50434 8005FC34 8432A524 */  addiu      $a1, $a1, %lo(D_800A3284)
    /* 50438 8005FC38 0300A288 */  lwl        $v0, 0x3($a1)
    /* 5043C 8005FC3C 0000A298 */  lwr        $v0, 0x0($a1)
    /* 50440 8005FC40 0700A388 */  lwl        $v1, 0x7($a1)
    /* 50444 8005FC44 0400A398 */  lwr        $v1, 0x4($a1)
    /* 50448 8005FC48 1B00A2AB */  swl        $v0, 0x1B($sp)
    /* 5044C 8005FC4C 1800A2BB */  swr        $v0, 0x18($sp)
    /* 50450 8005FC50 1F00A3AB */  swl        $v1, 0x1F($sp)
    /* 50454 8005FC54 1C00A3BB */  swr        $v1, 0x1C($sp)
    /* 50458 8005FC58 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5045C 8005FC5C 80ED010C */  jal        gpu_LoadImage
    /* 50460 8005FC60 40002526 */   addiu     $a1, $s1, 0x40
    /* 50464 8005FC64 CFEC010C */  jal        gpu_DrawSync
    /* 50468 8005FC68 21200000 */   addu      $a0, $zero, $zero
    /* 5046C 8005FC6C 1800A427 */  addiu      $a0, $sp, 0x18
    /* 50470 8005FC70 80ED010C */  jal        gpu_LoadImage
    /* 50474 8005FC74 14002526 */   addiu     $a1, $s1, 0x14
    /* 50478 8005FC78 CFEC010C */  jal        gpu_DrawSync
    /* 5047C 8005FC7C 21200000 */   addu      $a0, $zero, $zero
    /* 50480 8005FC80 AC0180AF */  sw         $zero, %gp_rel(D_800A3278)($gp)
    /* 50484 8005FC84 2800BF8F */  lw         $ra, 0x28($sp)
    /* 50488 8005FC88 2400B18F */  lw         $s1, 0x24($sp)
    /* 5048C 8005FC8C 2000B08F */  lw         $s0, 0x20($sp)
    /* 50490 8005FC90 3000BD27 */  addiu      $sp, $sp, 0x30
    /* 50494 8005FC94 0800E003 */  jr         $ra
    /* 50498 8005FC98 00000000 */   nop
endlabel func_8005FBC8
