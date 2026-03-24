glabel func_8008AF58
    /* 7B758 8008AF58 01000224 */  addiu      $v0, $zero, 0x1
    /* 7B75C 8008AF5C 05008214 */  bne        $a0, $v0, .L8008AF74
    /* 7B760 8008AF60 00000000 */   nop
    /* 7B764 8008AF64 0A80013C */  lui        $at, %hi(D_800A2D10)
    /* 7B768 8008AF68 102D20AC */  sw         $zero, %lo(D_800A2D10)($at)
    /* 7B76C 8008AF6C DF2B0208 */  j          .L8008AF7C
    /* 7B770 8008AF70 00000000 */   nop
  .L8008AF74:
    /* 7B774 8008AF74 0A80013C */  lui        $at, %hi(D_800A2D10)
    /* 7B778 8008AF78 102D22AC */  sw         $v0, %lo(D_800A2D10)($at)
  .L8008AF7C:
    /* 7B77C 8008AF7C 0800E003 */  jr         $ra
    /* 7B780 8008AF80 00000000 */   nop
endlabel func_8008AF58
