glabel func_80067094
    /* 57894 80067094 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 57898 80067098 04000424 */  addiu      $a0, $zero, 0x4
    /* 5789C 8006709C 02000524 */  addiu      $a1, $zero, 0x2
    /* 578A0 800670A0 1000BFAF */  sw         $ra, 0x10($sp)
    /* 578A4 800670A4 809C010C */  jal        func_80067200
    /* 578A8 800670A8 01000624 */   addiu     $a2, $zero, 0x1
    /* 578AC 800670AC 02000324 */  addiu      $v1, $zero, 0x2
    /* 578B0 800670B0 0F80013C */  lui        $at, %hi(D_800F1128)
    /* 578B4 800670B4 281123AC */  sw         $v1, %lo(D_800F1128)($at)
    /* 578B8 800670B8 1000BF8F */  lw         $ra, 0x10($sp)
    /* 578BC 800670BC FF004230 */  andi       $v0, $v0, 0xFF
    /* 578C0 800670C0 0800E003 */  jr         $ra
    /* 578C4 800670C4 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80067094
