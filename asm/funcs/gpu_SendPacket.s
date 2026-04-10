glabel gpu_SendPacket
    /* 6C0DC 8007B8DC E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6C0E0 8007B8E0 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6C0E4 8007B8E4 21808000 */  addu       $s0, $a0, $zero
    /* 6C0E8 8007B8E8 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6C0EC 8007B8EC 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6C0F0 8007B8F0 1800BFAF */  sw         $ra, 0x18($sp)
    /* 6C0F4 8007B8F4 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6C0F8 8007B8F8 3C00428C */  lw         $v0, 0x3C($v0)
    /* 6C0FC 8007B8FC 03001192 */  lbu        $s1, 0x3($s0)
    /* 6C100 8007B900 09F84000 */  jalr       $v0
    /* 6C104 8007B904 21200000 */   addu      $a0, $zero, $zero
    /* 6C108 8007B908 0A80023C */  lui        $v0, %hi(D_8009BE6C)
    /* 6C10C 8007B90C 6CBE428C */  lw         $v0, %lo(D_8009BE6C)($v0)
    /* 6C110 8007B910 04000426 */  addiu      $a0, $s0, 0x4
    /* 6C114 8007B914 1400428C */  lw         $v0, 0x14($v0)
    /* 6C118 8007B918 00000000 */  nop
    /* 6C11C 8007B91C 09F84000 */  jalr       $v0
    /* 6C120 8007B920 21282002 */   addu      $a1, $s1, $zero
    /* 6C124 8007B924 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6C128 8007B928 1400B18F */  lw         $s1, 0x14($sp)
    /* 6C12C 8007B92C 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C130 8007B930 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6C134 8007B934 0800E003 */  jr         $ra
    /* 6C138 8007B938 00000000 */   nop
endlabel gpu_SendPacket
