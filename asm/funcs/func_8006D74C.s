glabel func_8006D74C
    /* 5DF4C 8006D74C 88FFBD27 */  addiu      $sp, $sp, -0x78
    /* 5DF50 8006D750 6C00B1AF */  sw         $s1, 0x6C($sp)
    /* 5DF54 8006D754 21888000 */  addu       $s1, $a0, $zero
    /* 5DF58 8006D758 7000B2AF */  sw         $s2, 0x70($sp)
    /* 5DF5C 8006D75C 2190A000 */  addu       $s2, $a1, $zero
    /* 5DF60 8006D760 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5DF64 8006D764 4804828F */  lw         $v0, %gp_rel(D_800A3514)($gp)
    /* 5DF68 8006D768 0A80033C */  lui        $v1, %hi(D_800A36AC)
    /* 5DF6C 8006D76C AC36638C */  lw         $v1, %lo(D_800A36AC)($v1)
    /* 5DF70 8006D770 0A80053C */  lui        $a1, %hi(D_800A3518)
    /* 5DF74 8006D774 1835A524 */  addiu      $a1, $a1, %lo(D_800A3518)
    /* 5DF78 8006D778 7400BFAF */  sw         $ra, 0x74($sp)
    /* 5DF7C 8006D77C 6800B0AF */  sw         $s0, 0x68($sp)
    /* 5DF80 8006D780 01004224 */  addiu      $v0, $v0, 0x1
    /* 5DF84 8006D784 01006330 */  andi       $v1, $v1, 0x1
    /* 5DF88 8006D788 C0810300 */  sll        $s0, $v1, 7
    /* 5DF8C 8006D78C 21800302 */  addu       $s0, $s0, $v1
    /* 5DF90 8006D790 C0801000 */  sll        $s0, $s0, 3
    /* 5DF94 8006D794 21800302 */  addu       $s0, $s0, $v1
    /* 5DF98 8006D798 00811000 */  sll        $s0, $s0, 4
    /* 5DF9C 8006D79C 480482AF */  sw         $v0, %gp_rel(D_800A3514)($gp)
    /* 5DFA0 8006D7A0 0F80023C */  lui        $v0, %hi(D_800F7438)
    /* 5DFA4 8006D7A4 38744224 */  addiu      $v0, $v0, %lo(D_800F7438)
    /* 5DFA8 8006D7A8 E4B8010C */  jal        func_8006E390
    /* 5DFAC 8006D7AC 21800202 */   addu      $s0, $s0, $v0
    /* 5DFB0 8006D7B0 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5DFB4 8006D7B4 01000524 */  addiu      $a1, $zero, 0x1
    /* 5DFB8 8006D7B8 B9A6010C */  jal        func_80069AE4
    /* 5DFBC 8006D7BC 21300002 */   addu      $a2, $s0, $zero
    /* 5DFC0 8006D7C0 F7B4010C */  jal        func_8006D3DC
    /* 5DFC4 8006D7C4 1000A427 */   addiu     $a0, $sp, 0x10
    /* 5DFC8 8006D7C8 21202002 */  addu       $a0, $s1, $zero
    /* 5DFCC 8006D7CC 75B5010C */  jal        func_8006D5D4
    /* 5DFD0 8006D7D0 21284002 */   addu      $a1, $s2, $zero
    /* 5DFD4 8006D7D4 B471010C */  jal        func_8005C6D0
    /* 5DFD8 8006D7D8 21804000 */   addu      $s0, $v0, $zero
    /* 5DFDC 8006D7DC 21100002 */  addu       $v0, $s0, $zero
    /* 5DFE0 8006D7E0 7400BF8F */  lw         $ra, 0x74($sp)
    /* 5DFE4 8006D7E4 7000B28F */  lw         $s2, 0x70($sp)
    /* 5DFE8 8006D7E8 6C00B18F */  lw         $s1, 0x6C($sp)
    /* 5DFEC 8006D7EC 6800B08F */  lw         $s0, 0x68($sp)
    /* 5DFF0 8006D7F0 7800BD27 */  addiu      $sp, $sp, 0x78
    /* 5DFF4 8006D7F4 0800E003 */  jr         $ra
    /* 5DFF8 8006D7F8 00000000 */   nop
endlabel func_8006D74C
