glabel func_80082BB4
    /* 733B4 80082BB4 0A80023C */  lui        $v0, %hi(D_800A2600)
    /* 733B8 80082BB8 0026428C */  lw         $v0, %lo(D_800A2600)($v0)
    /* 733BC 80082BBC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 733C0 80082BC0 1000BFAF */  sw         $ra, 0x10($sp)
    /* 733C4 80082BC4 1000428C */  lw         $v0, 0x10($v0)
    /* 733C8 80082BC8 00000000 */  nop
    /* 733CC 80082BCC 09F84000 */  jalr       $v0
    /* 733D0 80082BD0 00000000 */   nop
    /* 733D4 80082BD4 1000BF8F */  lw         $ra, 0x10($sp)
    /* 733D8 80082BD8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 733DC 80082BDC 0800E003 */  jr         $ra
    /* 733E0 80082BE0 00000000 */   nop
    /* 733E4 80082BE4 0A80023C */  lui        $v0, %hi(D_800A2600)
    /* 733E8 80082BE8 0026428C */  lw         $v0, %lo(D_800A2600)($v0)
    /* 733EC 80082BEC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 733F0 80082BF0 1000BFAF */  sw         $ra, 0x10($sp)
    /* 733F4 80082BF4 1800428C */  lw         $v0, 0x18($v0)
    /* 733F8 80082BF8 00000000 */  nop
    /* 733FC 80082BFC 09F84000 */  jalr       $v0
    /* 73400 80082C00 00000000 */   nop
    /* 73404 80082C04 1000BF8F */  lw         $ra, 0x10($sp)
    /* 73408 80082C08 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7340C 80082C0C 0800E003 */  jr         $ra
    /* 73410 80082C10 00000000 */   nop
endlabel func_80082BB4
