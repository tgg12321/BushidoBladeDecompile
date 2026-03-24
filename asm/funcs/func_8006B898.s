glabel func_8006B898
    /* 5C098 8006B898 C0FFBD27 */  addiu      $sp, $sp, -0x40
    /* 5C09C 8006B89C 4000A4AF */  sw         $a0, 0x40($sp)
    /* 5C0A0 8006B8A0 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5C0A4 8006B8A4 4400A5AF */  sw         $a1, 0x44($sp)
    /* 5C0A8 8006B8A8 4804828F */  lw         $v0, %gp_rel(D_800A3514)($gp)
    /* 5C0AC 8006B8AC 0A80033C */  lui        $v1, %hi(D_800A36AC)
    /* 5C0B0 8006B8B0 AC36638C */  lw         $v1, %lo(D_800A36AC)($v1)
    /* 5C0B4 8006B8B4 0A80053C */  lui        $a1, %hi(D_800A3518)
    /* 5C0B8 8006B8B8 1835A524 */  addiu      $a1, $a1, %lo(D_800A3518)
    /* 5C0BC 8006B8BC 3C00BFAF */  sw         $ra, 0x3C($sp)
    /* 5C0C0 8006B8C0 3800B0AF */  sw         $s0, 0x38($sp)
    /* 5C0C4 8006B8C4 01004224 */  addiu      $v0, $v0, 0x1
    /* 5C0C8 8006B8C8 01006330 */  andi       $v1, $v1, 0x1
    /* 5C0CC 8006B8CC C0810300 */  sll        $s0, $v1, 7
    /* 5C0D0 8006B8D0 21800302 */  addu       $s0, $s0, $v1
    /* 5C0D4 8006B8D4 C0801000 */  sll        $s0, $s0, 3
    /* 5C0D8 8006B8D8 21800302 */  addu       $s0, $s0, $v1
    /* 5C0DC 8006B8DC 00811000 */  sll        $s0, $s0, 4
    /* 5C0E0 8006B8E0 480482AF */  sw         $v0, %gp_rel(D_800A3514)($gp)
    /* 5C0E4 8006B8E4 0F80023C */  lui        $v0, %hi(D_800F7438)
    /* 5C0E8 8006B8E8 38744224 */  addiu      $v0, $v0, %lo(D_800F7438)
    /* 5C0EC 8006B8EC E4B8010C */  jal        func_8006E390
    /* 5C0F0 8006B8F0 21800202 */   addu      $s0, $s0, $v0
    /* 5C0F4 8006B8F4 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5C0F8 8006B8F8 01000524 */  addiu      $a1, $zero, 0x1
    /* 5C0FC 8006B8FC B9A6010C */  jal        func_80069AE4
    /* 5C100 8006B900 21300002 */   addu      $a2, $s0, $zero
    /* 5C104 8006B904 48AC010C */  jal        func_8006B120
    /* 5C108 8006B908 1000A427 */   addiu     $a0, $sp, 0x10
    /* 5C10C 8006B90C 4000A427 */  addiu      $a0, $sp, 0x40
    /* 5C110 8006B910 5EAD010C */  jal        func_8006B578
    /* 5C114 8006B914 4400A527 */   addiu     $a1, $sp, 0x44
    /* 5C118 8006B918 3C00BF8F */  lw         $ra, 0x3C($sp)
    /* 5C11C 8006B91C 3800B08F */  lw         $s0, 0x38($sp)
    /* 5C120 8006B920 4000BD27 */  addiu      $sp, $sp, 0x40
    /* 5C124 8006B924 0800E003 */  jr         $ra
    /* 5C128 8006B928 00000000 */   nop
endlabel func_8006B898
