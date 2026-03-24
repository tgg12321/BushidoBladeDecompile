glabel func_80063AF0
    /* 542F0 80063AF0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 542F4 80063AF4 9C03838F */  lw         $v1, %gp_rel(D_800A3468)($gp)
    /* 542F8 80063AF8 01000224 */  addiu      $v0, $zero, 0x1
    /* 542FC 80063AFC 1000BFAF */  sw         $ra, 0x10($sp)
    /* 54300 80063B00 0F80013C */  lui        $at, %hi(D_800F10D0)
    /* 54304 80063B04 D01022AC */  sw         $v0, %lo(D_800F10D0)($at)
    /* 54308 80063B08 0000628C */  lw         $v0, 0x0($v1)
    /* 5430C 80063B0C 00000000 */  nop
    /* 54310 80063B10 43140200 */  sra        $v0, $v0, 17
    /* 54314 80063B14 03004230 */  andi       $v0, $v0, 0x3
    /* 54318 80063B18 900382A7 */  sh         $v0, %gp_rel(D_800A345C)($gp)
    /* 5431C 80063B1C F48E010C */  jal        func_80063BD0
    /* 54320 80063B20 21200000 */   addu      $a0, $zero, $zero
    /* 54324 80063B24 1000BF8F */  lw         $ra, 0x10($sp)
    /* 54328 80063B28 FF004230 */  andi       $v0, $v0, 0xFF
    /* 5432C 80063B2C 0800E003 */  jr         $ra
    /* 54330 80063B30 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80063AF0
