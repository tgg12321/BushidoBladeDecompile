glabel func_8007A62C
    /* 6AE2C 8007A62C D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 6AE30 8007A630 21108000 */  addu       $v0, $a0, $zero
    /* 6AE34 8007A634 1800B0AF */  sw         $s0, 0x18($sp)
    /* 6AE38 8007A638 2180A000 */  addu       $s0, $a1, $zero
    /* 6AE3C 8007A63C 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 6AE40 8007A640 2188C000 */  addu       $s1, $a2, $zero
    /* 6AE44 8007A644 1000A427 */  addiu      $a0, $sp, 0x10
    /* 6AE48 8007A648 21284000 */  addu       $a1, $v0, $zero
    /* 6AE4C 8007A64C 10000224 */  addiu      $v0, $zero, 0x10
    /* 6AE50 8007A650 1400A2A7 */  sh         $v0, 0x14($sp)
    /* 6AE54 8007A654 01000224 */  addiu      $v0, $zero, 0x1
    /* 6AE58 8007A658 2000BFAF */  sw         $ra, 0x20($sp)
    /* 6AE5C 8007A65C 1000B0A7 */  sh         $s0, 0x10($sp)
    /* 6AE60 8007A660 1200B1A7 */  sh         $s1, 0x12($sp)
    /* 6AE64 8007A664 80ED010C */  jal        gpu_LoadImage
    /* 6AE68 8007A668 1600A2A7 */   sh        $v0, 0x16($sp)
    /* 6AE6C 8007A66C 21200002 */  addu       $a0, $s0, $zero
    /* 6AE70 8007A670 F1E9010C */  jal        func_8007A7C4
    /* 6AE74 8007A674 21282002 */   addu      $a1, $s1, $zero
    /* 6AE78 8007A678 FFFF4230 */  andi       $v0, $v0, 0xFFFF
    /* 6AE7C 8007A67C 2000BF8F */  lw         $ra, 0x20($sp)
    /* 6AE80 8007A680 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 6AE84 8007A684 1800B08F */  lw         $s0, 0x18($sp)
    /* 6AE88 8007A688 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 6AE8C 8007A68C 0800E003 */  jr         $ra
    /* 6AE90 8007A690 00000000 */   nop
endlabel func_8007A62C
