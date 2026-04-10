glabel func_80037468
    /* 27C68 80037468 A0FFBD27 */  addiu      $sp, $sp, -0x60
    /* 27C6C 8003746C 5000B0AF */  sw         $s0, 0x50($sp)
    /* 27C70 80037470 21808000 */  addu       $s0, $a0, $zero
    /* 27C74 80037474 5800B2AF */  sw         $s2, 0x58($sp)
    /* 27C78 80037478 2190A000 */  addu       $s2, $a1, $zero
    /* 27C7C 8003747C 5400B1AF */  sw         $s1, 0x54($sp)
    /* 27C80 80037480 2188C000 */  addu       $s1, $a2, $zero
    /* 27C84 80037484 5C00BFAF */  sw         $ra, 0x5C($sp)
    /* 27C88 80037488 330A020C */  jal        sys_VSync
    /* 27C8C 8003748C 21200000 */   addu      $a0, $zero, $zero
    /* 27C90 80037490 A8EC010C */  jal        gpu_SetDispMask
    /* 27C94 80037494 21200000 */   addu      $a0, $zero, $zero
    /* 27C98 80037498 1A5A000C */  jal        gpu_EnableDisplay
    /* 27C9C 8003749C 00000000 */   nop
    /* 27CA0 800374A0 436D010C */  jal        func_8005B50C
    /* 27CA4 800374A4 00000000 */   nop
    /* 27CA8 800374A8 DDDD000C */  jal        func_80037774
    /* 27CAC 800374AC 00000000 */   nop
    /* 27CB0 800374B0 B00A020C */  jal        irq_DisableInterrupts
    /* 27CB4 800374B4 00000000 */   nop
    /* 27CB8 800374B8 DFFF010C */  jal        func_8007FF7C
    /* 27CBC 800374BC 00000000 */   nop
    /* 27CC0 800374C0 D2DC000C */  jal        special_camera_get_rot_dir
    /* 27CC4 800374C4 1000A427 */   addiu     $a0, $sp, 0x10
    /* 27CC8 800374C8 CFEC010C */  jal        gpu_DrawSync
    /* 27CCC 800374CC 21200000 */   addu      $a0, $zero, $zero
    /* 27CD0 800374D0 9FEB010C */  jal        func_8007AE7C
    /* 27CD4 800374D4 21200000 */   addu      $a0, $zero, $zero
    /* 27CD8 800374D8 5AE3010C */  jal        pad_Init
    /* 27CDC 800374DC 00000000 */   nop
    /* 27CE0 800374E0 ED0A020C */  jal        irq_Reset
    /* 27CE4 800374E4 00000000 */   nop
    /* 27CE8 800374E8 3000B1AF */  sw         $s1, 0x30($sp)
    /* 27CEC 800374EC 6EE2010C */  jal        EnterCriticalSection
    /* 27CF0 800374F0 3400A0AF */   sw        $zero, 0x34($sp)
    /* 27CF4 800374F4 1000A427 */  addiu      $a0, $sp, 0x10
    /* 27CF8 800374F8 21280002 */  addu       $a1, $s0, $zero
    /* 27CFC 800374FC 52E2010C */  jal        func_80078948
    /* 27D00 80037500 21304002 */   addu      $a2, $s2, $zero
    /* 27D04 80037504 865A000C */  jal        sys_Init
    /* 27D08 80037508 00000000 */   nop
    /* 27D0C 8003750C 3E5B000C */  jal        file_LoadSoundData
    /* 27D10 80037510 00000000 */   nop
    /* 27D14 80037514 330A020C */  jal        sys_VSync
    /* 27D18 80037518 21200000 */   addu      $a0, $zero, $zero
    /* 27D1C 8003751C A8EC010C */  jal        gpu_SetDispMask
    /* 27D20 80037520 01000424 */   addiu     $a0, $zero, 0x1
    /* 27D24 80037524 5C00BF8F */  lw         $ra, 0x5C($sp)
    /* 27D28 80037528 5800B28F */  lw         $s2, 0x58($sp)
    /* 27D2C 8003752C 5400B18F */  lw         $s1, 0x54($sp)
    /* 27D30 80037530 5000B08F */  lw         $s0, 0x50($sp)
    /* 27D34 80037534 6000BD27 */  addiu      $sp, $sp, 0x60
    /* 27D38 80037538 0800E003 */  jr         $ra
    /* 27D3C 8003753C 00000000 */   nop
endlabel func_80037468
