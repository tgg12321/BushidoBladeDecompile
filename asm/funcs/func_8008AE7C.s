glabel func_8008AE7C
    /* 7B67C 8008AE7C 05008010 */  beqz       $a0, .L8008AE94
    /* 7B680 8008AE80 01000224 */   addiu     $v0, $zero, 0x1
    /* 7B684 8008AE84 04008214 */  bne        $a0, $v0, .L8008AE98
    /* 7B688 8008AE88 21100000 */   addu      $v0, $zero, $zero
    /* 7B68C 8008AE8C A62B0208 */  j          .L8008AE98
    /* 7B690 8008AE90 01000224 */   addiu     $v0, $zero, 0x1
  .L8008AE94:
    /* 7B694 8008AE94 21100000 */  addu       $v0, $zero, $zero
  .L8008AE98:
    /* 7B698 8008AE98 0A80013C */  lui        $at, %hi(D_800A2878)
    /* 7B69C 8008AE9C 782824AC */  sw         $a0, %lo(D_800A2878)($at)
    /* 7B6A0 8008AEA0 0A80013C */  lui        $at, %hi(D_800A2CF8)
    /* 7B6A4 8008AEA4 F82C22AC */  sw         $v0, %lo(D_800A2CF8)($at)
    /* 7B6A8 8008AEA8 0800E003 */  jr         $ra
    /* 7B6AC 8008AEAC 00000000 */   nop
endlabel func_8008AE7C
