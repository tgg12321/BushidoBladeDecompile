glabel func_800602AC
    /* 50AAC 800602AC C0FFBD27 */  addiu      $sp, $sp, -0x40
    /* 50AB0 800602B0 3000B0AF */  sw         $s0, 0x30($sp)
    /* 50AB4 800602B4 2180A000 */  addu       $s0, $a1, $zero
    /* 50AB8 800602B8 3D008524 */  addiu      $a1, $a0, 0x3D
    /* 50ABC 800602BC 02000424 */  addiu      $a0, $zero, 0x2
    /* 50AC0 800602C0 3800BFAF */  sw         $ra, 0x38($sp)
    /* 50AC4 800602C4 AADB000C */  jal        func_80036EA8
    /* 50AC8 800602C8 3400B1AF */   sw        $s1, 0x34($sp)
    /* 50ACC 800602CC 21884000 */  addu       $s1, $v0, $zero
    /* 50AD0 800602D0 21202002 */  addu       $a0, $s1, $zero
    /* 50AD4 800602D4 66DB000C */  jal        replay_camera_Init
    /* 50AD8 800602D8 21280002 */   addu      $a1, $s0, $zero
    /* 50ADC 800602DC D0DB000C */  jal        game_FrameLoop
    /* 50AE0 800602E0 00000000 */   nop
    /* 50AE4 800602E4 CADB000C */  jal        func_80036F28
    /* 50AE8 800602E8 21202002 */   addu      $a0, $s1, $zero
    /* 50AEC 800602EC 0000028E */  lw         $v0, 0x0($s0)
    /* 50AF0 800602F0 0400038E */  lw         $v1, 0x4($s0)
    /* 50AF4 800602F4 21105000 */  addu       $v0, $v0, $s0
    /* 50AF8 800602F8 21187000 */  addu       $v1, $v1, $s0
    /* 50AFC 800602FC 000002AE */  sw         $v0, 0x0($s0)
    /* 50B00 80060300 040003AE */  sw         $v1, 0x4($s0)
    /* 50B04 80060304 0A80053C */  lui        $a1, %hi(D_800A3294)
    /* 50B08 80060308 9432A524 */  addiu      $a1, $a1, %lo(D_800A3294)
    /* 50B0C 8006030C 0300A288 */  lwl        $v0, 0x3($a1)
    /* 50B10 80060310 0000A298 */  lwr        $v0, 0x0($a1)
    /* 50B14 80060314 0700A388 */  lwl        $v1, 0x7($a1)
    /* 50B18 80060318 0400A398 */  lwr        $v1, 0x4($a1)
    /* 50B1C 8006031C 1300A2AB */  swl        $v0, 0x13($sp)
    /* 50B20 80060320 1000A2BB */  swr        $v0, 0x10($sp)
    /* 50B24 80060324 1700A3AB */  swl        $v1, 0x17($sp)
    /* 50B28 80060328 1400A3BB */  swr        $v1, 0x14($sp)
    /* 50B2C 8006032C 0A80053C */  lui        $a1, %hi(D_800A329C)
    /* 50B30 80060330 9C32A524 */  addiu      $a1, $a1, %lo(D_800A329C)
    /* 50B34 80060334 0300A288 */  lwl        $v0, 0x3($a1)
    /* 50B38 80060338 0000A298 */  lwr        $v0, 0x0($a1)
    /* 50B3C 8006033C 0700A388 */  lwl        $v1, 0x7($a1)
    /* 50B40 80060340 0400A398 */  lwr        $v1, 0x4($a1)
    /* 50B44 80060344 1B00A2AB */  swl        $v0, 0x1B($sp)
    /* 50B48 80060348 1800A2BB */  swr        $v0, 0x18($sp)
    /* 50B4C 8006034C 1F00A3AB */  swl        $v1, 0x1F($sp)
    /* 50B50 80060350 1C00A3BB */  swr        $v1, 0x1C($sp)
    /* 50B54 80060354 0000118E */  lw         $s1, 0x0($s0)
    /* 50B58 80060358 1000A427 */  addiu      $a0, $sp, 0x10
    /* 50B5C 8006035C 80ED010C */  jal        gpu_LoadImage
    /* 50B60 80060360 40002526 */   addiu     $a1, $s1, 0x40
    /* 50B64 80060364 CFEC010C */  jal        gpu_DrawSync
    /* 50B68 80060368 21200000 */   addu      $a0, $zero, $zero
    /* 50B6C 8006036C 1800A427 */  addiu      $a0, $sp, 0x18
    /* 50B70 80060370 80ED010C */  jal        gpu_LoadImage
    /* 50B74 80060374 14002526 */   addiu     $a1, $s1, 0x14
    /* 50B78 80060378 CFEC010C */  jal        gpu_DrawSync
    /* 50B7C 8006037C 21200000 */   addu      $a0, $zero, $zero
    /* 50B80 80060380 0A80053C */  lui        $a1, %hi(D_800A32A4)
    /* 50B84 80060384 A432A524 */  addiu      $a1, $a1, %lo(D_800A32A4)
    /* 50B88 80060388 0300A288 */  lwl        $v0, 0x3($a1)
    /* 50B8C 8006038C 0000A298 */  lwr        $v0, 0x0($a1)
    /* 50B90 80060390 0700A388 */  lwl        $v1, 0x7($a1)
    /* 50B94 80060394 0400A398 */  lwr        $v1, 0x4($a1)
    /* 50B98 80060398 2300A2AB */  swl        $v0, 0x23($sp)
    /* 50B9C 8006039C 2000A2BB */  swr        $v0, 0x20($sp)
    /* 50BA0 800603A0 2700A3AB */  swl        $v1, 0x27($sp)
    /* 50BA4 800603A4 2400A3BB */  swr        $v1, 0x24($sp)
    /* 50BA8 800603A8 0A80053C */  lui        $a1, %hi(D_800A32AC)
    /* 50BAC 800603AC AC32A524 */  addiu      $a1, $a1, %lo(D_800A32AC)
    /* 50BB0 800603B0 0300A288 */  lwl        $v0, 0x3($a1)
    /* 50BB4 800603B4 0000A298 */  lwr        $v0, 0x0($a1)
    /* 50BB8 800603B8 0700A388 */  lwl        $v1, 0x7($a1)
    /* 50BBC 800603BC 0400A398 */  lwr        $v1, 0x4($a1)
    /* 50BC0 800603C0 2B00A2AB */  swl        $v0, 0x2B($sp)
    /* 50BC4 800603C4 2800A2BB */  swr        $v0, 0x28($sp)
    /* 50BC8 800603C8 2F00A3AB */  swl        $v1, 0x2F($sp)
    /* 50BCC 800603CC 2C00A3BB */  swr        $v1, 0x2C($sp)
    /* 50BD0 800603D0 0400108E */  lw         $s0, 0x4($s0)
    /* 50BD4 800603D4 2000A427 */  addiu      $a0, $sp, 0x20
    /* 50BD8 800603D8 80ED010C */  jal        gpu_LoadImage
    /* 50BDC 800603DC 60000526 */   addiu     $a1, $s0, 0x60
    /* 50BE0 800603E0 CFEC010C */  jal        gpu_DrawSync
    /* 50BE4 800603E4 21200000 */   addu      $a0, $zero, $zero
    /* 50BE8 800603E8 2800A427 */  addiu      $a0, $sp, 0x28
    /* 50BEC 800603EC 80ED010C */  jal        gpu_LoadImage
    /* 50BF0 800603F0 14000526 */   addiu     $a1, $s0, 0x14
    /* 50BF4 800603F4 CFEC010C */  jal        gpu_DrawSync
    /* 50BF8 800603F8 21200000 */   addu      $a0, $zero, $zero
    /* 50BFC 800603FC 3800BF8F */  lw         $ra, 0x38($sp)
    /* 50C00 80060400 3400B18F */  lw         $s1, 0x34($sp)
    /* 50C04 80060404 3000B08F */  lw         $s0, 0x30($sp)
    /* 50C08 80060408 4000BD27 */  addiu      $sp, $sp, 0x40
    /* 50C0C 8006040C 0800E003 */  jr         $ra
    /* 50C10 80060410 00000000 */   nop
endlabel func_800602AC
