glabel func_8006E068
    /* 5E868 8006E068 88FFBD27 */  addiu      $sp, $sp, -0x78
    /* 5E86C 8006E06C 6C00B1AF */  sw         $s1, 0x6C($sp)
    /* 5E870 8006E070 21888000 */  addu       $s1, $a0, $zero
    /* 5E874 8006E074 7000B2AF */  sw         $s2, 0x70($sp)
    /* 5E878 8006E078 2190A000 */  addu       $s2, $a1, $zero
    /* 5E87C 8006E07C 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5E880 8006E080 4804828F */  lw         $v0, %gp_rel(D_800A3514)($gp)
    /* 5E884 8006E084 0A80033C */  lui        $v1, %hi(D_800A36AC)
    /* 5E888 8006E088 AC36638C */  lw         $v1, %lo(D_800A36AC)($v1)
    /* 5E88C 8006E08C 0A80053C */  lui        $a1, %hi(D_800A3518)
    /* 5E890 8006E090 1835A524 */  addiu      $a1, $a1, %lo(D_800A3518)
    /* 5E894 8006E094 7400BFAF */  sw         $ra, 0x74($sp)
    /* 5E898 8006E098 6800B0AF */  sw         $s0, 0x68($sp)
    /* 5E89C 8006E09C 01004224 */  addiu      $v0, $v0, 0x1
    /* 5E8A0 8006E0A0 01006330 */  andi       $v1, $v1, 0x1
    /* 5E8A4 8006E0A4 C0810300 */  sll        $s0, $v1, 7
    /* 5E8A8 8006E0A8 21800302 */  addu       $s0, $s0, $v1
    /* 5E8AC 8006E0AC C0801000 */  sll        $s0, $s0, 3
    /* 5E8B0 8006E0B0 21800302 */  addu       $s0, $s0, $v1
    /* 5E8B4 8006E0B4 00811000 */  sll        $s0, $s0, 4
    /* 5E8B8 8006E0B8 480482AF */  sw         $v0, %gp_rel(D_800A3514)($gp)
    /* 5E8BC 8006E0BC 0F80023C */  lui        $v0, %hi(D_800F7438)
    /* 5E8C0 8006E0C0 38744224 */  addiu      $v0, $v0, %lo(D_800F7438)
    /* 5E8C4 8006E0C4 E4B8010C */  jal        func_8006E390
    /* 5E8C8 8006E0C8 21800202 */   addu      $s0, $s0, $v0
    /* 5E8CC 8006E0CC 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5E8D0 8006E0D0 01000524 */  addiu      $a1, $zero, 0x1
    /* 5E8D4 8006E0D4 B9A6010C */  jal        func_80069AE4
    /* 5E8D8 8006E0D8 21300002 */   addu      $a2, $s0, $zero
    /* 5E8DC 8006E0DC 65B7010C */  jal        func_8006DD94
    /* 5E8E0 8006E0E0 1000A427 */   addiu     $a0, $sp, 0x10
    /* 5E8E4 8006E0E4 21202002 */  addu       $a0, $s1, $zero
    /* 5E8E8 8006E0E8 DAB7010C */  jal        func_8006DF68
    /* 5E8EC 8006E0EC 21284002 */   addu      $a1, $s2, $zero
    /* 5E8F0 8006E0F0 7400BF8F */  lw         $ra, 0x74($sp)
    /* 5E8F4 8006E0F4 7000B28F */  lw         $s2, 0x70($sp)
    /* 5E8F8 8006E0F8 6C00B18F */  lw         $s1, 0x6C($sp)
    /* 5E8FC 8006E0FC 6800B08F */  lw         $s0, 0x68($sp)
    /* 5E900 8006E100 7800BD27 */  addiu      $sp, $sp, 0x78
    /* 5E904 8006E104 0800E003 */  jr         $ra
    /* 5E908 8006E108 00000000 */   nop
endlabel func_8006E068
