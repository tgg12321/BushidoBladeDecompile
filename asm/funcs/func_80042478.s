glabel func_80042478
    /* 32C78 80042478 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 32C7C 8004247C 03140400 */  sra        $v0, $a0, 16
    /* 32C80 80042480 1400B1AF */  sw         $s1, 0x14($sp)
    /* 32C84 80042484 FF005130 */  andi       $s1, $v0, 0xFF
    /* 32C88 80042488 03120400 */  sra        $v0, $a0, 8
    /* 32C8C 8004248C 1800B2AF */  sw         $s2, 0x18($sp)
    /* 32C90 80042490 FF005230 */  andi       $s2, $v0, 0xFF
    /* 32C94 80042494 1000B0AF */  sw         $s0, 0x10($sp)
    /* 32C98 80042498 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* 32C9C 8004249C BF21010C */  jal        func_800486FC
    /* 32CA0 800424A0 FF009030 */   andi      $s0, $a0, 0xFF
    /* 32CA4 800424A4 07004010 */  beqz       $v0, .L800424C4
    /* 32CA8 800424A8 21202002 */   addu      $a0, $s1, $zero
    /* 32CAC 800424AC 21284002 */  addu       $a1, $s2, $zero
    /* 32CB0 800424B0 0722010C */  jal        func_8004881C
    /* 32CB4 800424B4 21300002 */   addu      $a2, $s0, $zero
    /* 32CB8 800424B8 21804000 */  addu       $s0, $v0, $zero
    /* 32CBC 800424BC 21900002 */  addu       $s2, $s0, $zero
    /* 32CC0 800424C0 21880002 */  addu       $s1, $s0, $zero
  .L800424C4:
    /* 32CC4 800424C4 01000424 */  addiu      $a0, $zero, 0x1
    /* 32CC8 800424C8 21282002 */  addu       $a1, $s1, $zero
    /* 32CCC 800424CC 21304002 */  addu       $a2, $s2, $zero
    /* 32CD0 800424D0 DA59000C */  jal        disp_SetFramebufferMode
    /* 32CD4 800424D4 21380002 */   addu      $a3, $s0, $zero
    /* 32CD8 800424D8 21202002 */  addu       $a0, $s1, $zero
    /* 32CDC 800424DC 21284002 */  addu       $a1, $s2, $zero
    /* 32CE0 800424E0 EFFB010C */  jal        func_8007EFBC
    /* 32CE4 800424E4 21300002 */   addu      $a2, $s0, $zero
    /* 32CE8 800424E8 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* 32CEC 800424EC 1800B28F */  lw         $s2, 0x18($sp)
    /* 32CF0 800424F0 1400B18F */  lw         $s1, 0x14($sp)
    /* 32CF4 800424F4 1000B08F */  lw         $s0, 0x10($sp)
    /* 32CF8 800424F8 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 32CFC 800424FC 0800E003 */  jr         $ra
    /* 32D00 80042500 00000000 */   nop
endlabel func_80042478
