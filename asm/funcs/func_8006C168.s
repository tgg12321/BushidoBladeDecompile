glabel func_8006C168
    /* 5C968 8006C168 90FFBD27 */  addiu      $sp, $sp, -0x70
    /* 5C96C 8006C16C 7000A4AF */  sw         $a0, 0x70($sp)
    /* 5C970 8006C170 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5C974 8006C174 7400A5AF */  sw         $a1, 0x74($sp)
    /* 5C978 8006C178 4804828F */  lw         $v0, %gp_rel(D_800A3514)($gp)
    /* 5C97C 8006C17C 0A80033C */  lui        $v1, %hi(D_800A36AC)
    /* 5C980 8006C180 AC36638C */  lw         $v1, %lo(D_800A36AC)($v1)
    /* 5C984 8006C184 0A80053C */  lui        $a1, %hi(D_800A3518)
    /* 5C988 8006C188 1835A524 */  addiu      $a1, $a1, %lo(D_800A3518)
    /* 5C98C 8006C18C 6C00BFAF */  sw         $ra, 0x6C($sp)
    /* 5C990 8006C190 6800B0AF */  sw         $s0, 0x68($sp)
    /* 5C994 8006C194 01004224 */  addiu      $v0, $v0, 0x1
    /* 5C998 8006C198 01006330 */  andi       $v1, $v1, 0x1
    /* 5C99C 8006C19C C0810300 */  sll        $s0, $v1, 7
    /* 5C9A0 8006C1A0 21800302 */  addu       $s0, $s0, $v1
    /* 5C9A4 8006C1A4 C0801000 */  sll        $s0, $s0, 3
    /* 5C9A8 8006C1A8 21800302 */  addu       $s0, $s0, $v1
    /* 5C9AC 8006C1AC 00811000 */  sll        $s0, $s0, 4
    /* 5C9B0 8006C1B0 480482AF */  sw         $v0, %gp_rel(D_800A3514)($gp)
    /* 5C9B4 8006C1B4 0F80023C */  lui        $v0, %hi(D_800F7438)
    /* 5C9B8 8006C1B8 38744224 */  addiu      $v0, $v0, %lo(D_800F7438)
    /* 5C9BC 8006C1BC E4B8010C */  jal        func_8006E390
    /* 5C9C0 8006C1C0 21800202 */   addu      $s0, $s0, $v0
    /* 5C9C4 8006C1C4 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5C9C8 8006C1C8 01000524 */  addiu      $a1, $zero, 0x1
    /* 5C9CC 8006C1CC B9A6010C */  jal        func_80069AE4
    /* 5C9D0 8006C1D0 21300002 */   addu      $a2, $s0, $zero
    /* 5C9D4 8006C1D4 DAAE010C */  jal        func_8006BB68
    /* 5C9D8 8006C1D8 1000A427 */   addiu     $a0, $sp, 0x10
    /* 5C9DC 8006C1DC 7000A427 */  addiu      $a0, $sp, 0x70
    /* 5C9E0 8006C1E0 4BAE010C */  jal        func_8006B92C
    /* 5C9E4 8006C1E4 7400A527 */   addiu     $a1, $sp, 0x74
    /* 5C9E8 8006C1E8 6C00BF8F */  lw         $ra, 0x6C($sp)
    /* 5C9EC 8006C1EC 6800B08F */  lw         $s0, 0x68($sp)
    /* 5C9F0 8006C1F0 7000BD27 */  addiu      $sp, $sp, 0x70
    /* 5C9F4 8006C1F4 0800E003 */  jr         $ra
    /* 5C9F8 8006C1F8 00000000 */   nop
endlabel func_8006C168
