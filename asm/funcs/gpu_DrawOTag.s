glabel gpu_DrawOTag
    /* 6C13C 8007B93C 0A80023C */  lui        $v0, %hi(D_8009BE76)
    /* 6C140 8007B940 76BE4290 */  lbu        $v0, %lo(D_8009BE76)($v0)
    /* 6C144 8007B944 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6C148 8007B948 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C14C 8007B94C 21808000 */  addu       $s0, $a0, $zero
    /* 6C150 8007B950 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6C154 8007B954 08004014 */  bnez       $v0, .L8007B978
    /* 6C158 8007B958 1400BFAF */   sw        $ra, 0x14($sp)
    /* 6C15C 8007B95C 0180043C */  lui        $a0, %hi(D_80015FB0)
    /* 6C160 8007B960 B05F8424 */  addiu      $a0, $a0, %lo(D_80015FB0)
    /* 6C164 8007B964 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6C168 8007B968 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6C16C 8007B96C 00000000 */  nop
    /* 6C170 8007B970 09F84000 */  jalr       $v0
    /* 6C174 8007B974 21280002 */   addu      $a1, $s0, $zero
  .L8007B978:
    /* 6C178 8007B978 21280002 */  addu       $a1, $s0, $zero
    /* 6C17C 8007B97C 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6C180 8007B980 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6C184 8007B984 21300000 */  addu       $a2, $zero, $zero
    /* 6C188 8007B988 1800448C */  lw         $a0, 0x18($v0)
    /* 6C18C 8007B98C 0800428C */  lw         $v0, 0x8($v0)
    /* 6C190 8007B990 00000000 */  nop
    /* 6C194 8007B994 09F84000 */  jalr       $v0
    /* 6C198 8007B998 21380000 */   addu      $a3, $zero, $zero
    /* 6C19C 8007B99C 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6C1A0 8007B9A0 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C1A4 8007B9A4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6C1A8 8007B9A8 0800E003 */  jr         $ra
    /* 6C1AC 8007B9AC 00000000 */   nop
endlabel gpu_DrawOTag
