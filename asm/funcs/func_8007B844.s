glabel func_8007B844
    /* 6C044 8007B844 0A80023C */  lui        $v0, %hi(D_8009BE76)
    /* 6C048 8007B848 76BE4290 */  lbu        $v0, %lo(D_8009BE76)($v0)
    /* 6C04C 8007B84C E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6C050 8007B850 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C054 8007B854 21808000 */  addu       $s0, $a0, $zero
    /* 6C058 8007B858 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6C05C 8007B85C 2188A000 */  addu       $s1, $a1, $zero
    /* 6C060 8007B860 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6C064 8007B864 09004014 */  bnez       $v0, .L8007B88C
    /* 6C068 8007B868 1800BFAF */   sw        $ra, 0x18($sp)
    /* 6C06C 8007B86C 0180043C */  lui        $a0, %hi(D_80015F98)
    /* 6C070 8007B870 985F8424 */  addiu      $a0, $a0, %lo(D_80015F98)
    /* 6C074 8007B874 21280002 */  addu       $a1, $s0, $zero
    /* 6C078 8007B878 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6C07C 8007B87C 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6C080 8007B880 00000000 */  nop
    /* 6C084 8007B884 09F84000 */  jalr       $v0
    /* 6C088 8007B888 21302002 */   addu      $a2, $s1, $zero
  .L8007B88C:
    /* 6C08C 8007B88C 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6C090 8007B890 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6C094 8007B894 21200002 */  addu       $a0, $s0, $zero
    /* 6C098 8007B898 2C00428C */  lw         $v0, 0x2C($v0)
    /* 6C09C 8007B89C 00000000 */  nop
    /* 6C0A0 8007B8A0 09F84000 */  jalr       $v0
    /* 6C0A4 8007B8A4 21282002 */   addu      $a1, $s1, $zero
    /* 6C0A8 8007B8A8 FF00043C */  lui        $a0, (0xFFFFFF >> 16)
    /* 6C0AC 8007B8AC FFFF8434 */  ori        $a0, $a0, (0xFFFFFF & 0xFFFF)
    /* 6C0B0 8007B8B0 21100002 */  addu       $v0, $s0, $zero
    /* 6C0B4 8007B8B4 0A80033C */  lui        $v1, %hi(D_8009BF30)
    /* 6C0B8 8007B8B8 30BF6324 */  addiu      $v1, $v1, %lo(D_8009BF30)
    /* 6C0BC 8007B8BC 24186400 */  and        $v1, $v1, $a0
    /* 6C0C0 8007B8C0 000043AC */  sw         $v1, 0x0($v0)
    /* 6C0C4 8007B8C4 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6C0C8 8007B8C8 1400B18F */  lw         $s1, 0x14($sp)
    /* 6C0CC 8007B8CC 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C0D0 8007B8D0 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6C0D4 8007B8D4 0800E003 */  jr         $ra
    /* 6C0D8 8007B8D8 00000000 */   nop
endlabel func_8007B844
