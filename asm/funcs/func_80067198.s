glabel func_80067198
    /* 57998 80067198 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 5799C 8006719C 07000424 */  addiu      $a0, $zero, 0x7
    /* 579A0 800671A0 02000524 */  addiu      $a1, $zero, 0x2
    /* 579A4 800671A4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 579A8 800671A8 809C010C */  jal        func_80067200
    /* 579AC 800671AC 21300000 */   addu      $a2, $zero, $zero
    /* 579B0 800671B0 01000324 */  addiu      $v1, $zero, 0x1
    /* 579B4 800671B4 0F80013C */  lui        $at, %hi(D_800F1134)
    /* 579B8 800671B8 341123AC */  sw         $v1, %lo(D_800F1134)($at)
    /* 579BC 800671BC 1000BF8F */  lw         $ra, 0x10($sp)
    /* 579C0 800671C0 FF004230 */  andi       $v0, $v0, 0xFF
    /* 579C4 800671C4 0800E003 */  jr         $ra
    /* 579C8 800671C8 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_80067198
