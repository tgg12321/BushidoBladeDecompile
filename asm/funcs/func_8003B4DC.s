glabel func_8003B4DC
    /* 2BCDC 8003B4DC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2BCE0 8003B4E0 01000224 */  addiu      $v0, $zero, 0x1
    /* 2BCE4 8003B4E4 0A80013C */  lui        $at, %hi(D_800A3712)
    /* 2BCE8 8003B4E8 123722A0 */  sb         $v0, %lo(D_800A3712)($at)
    /* 2BCEC 8003B4EC 1F000224 */  addiu      $v0, $zero, 0x1F
    /* 2BCF0 8003B4F0 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2BCF4 8003B4F4 1080013C */  lui        $at, %hi(D_8010277E)
    /* 2BCF8 8003B4F8 7E2720A0 */  sb         $zero, %lo(D_8010277E)($at)
    /* 2BCFC 8003B4FC 1080013C */  lui        $at, %hi(D_8010277D)
    /* 2BD00 8003B500 7D2722A0 */  sb         $v0, %lo(D_8010277D)($at)
    /* 2BD04 8003B504 1080013C */  lui        $at, %hi(D_8010277F)
    /* 2BD08 8003B508 7F2720A0 */  sb         $zero, %lo(D_8010277F)($at)
    /* 2BD0C 8003B50C D0EB000C */  jal        func_8003AF40
    /* 2BD10 8003B510 21200000 */   addu      $a0, $zero, $zero
    /* 2BD14 8003B514 D0EB000C */  jal        func_8003AF40
    /* 2BD18 8003B518 01000424 */   addiu     $a0, $zero, 0x1
    /* 2BD1C 8003B51C FFEB000C */  jal        func_8003AFFC
    /* 2BD20 8003B520 00000000 */   nop
    /* 2BD24 8003B524 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2BD28 8003B528 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2BD2C 8003B52C 0800E003 */  jr         $ra
    /* 2BD30 8003B530 00000000 */   nop
endlabel func_8003B4DC
