glabel func_80077894
    /* 68094 80077894 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 68098 80077898 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6809C 8007789C 1400BFAF */  sw         $ra, 0x14($sp)
    /* 680A0 800778A0 F3A4010C */  jal        func_800693CC
    /* 680A4 800778A4 21800000 */   addu      $s0, $zero, $zero
    /* 680A8 800778A8 21284000 */  addu       $a1, $v0, $zero
    /* 680AC 800778AC 0B00A004 */  bltz       $a1, .L800778DC
    /* 680B0 800778B0 F0FF0224 */   addiu     $v0, $zero, -0x10
    /* 680B4 800778B4 0A80043C */  lui        $a0, %hi(D_8009BD38)
    /* 680B8 800778B8 38BD8424 */  addiu      $a0, $a0, %lo(D_8009BD38)
    /* 680BC 800778BC 01001024 */  addiu      $s0, $zero, 0x1
    /* 680C0 800778C0 0000838C */  lw         $v1, 0x0($a0)
    /* 680C4 800778C4 180580AF */  sw         $zero, %gp_rel(D_800A35E4)($gp)
    /* 680C8 800778C8 24186200 */  and        $v1, $v1, $v0
    /* 680CC 800778CC 0F00A230 */  andi       $v0, $a1, 0xF
    /* 680D0 800778D0 25186200 */  or         $v1, $v1, $v0
    /* 680D4 800778D4 3BDE0108 */  j          .L800778EC
    /* 680D8 800778D8 000083AC */   sw        $v1, 0x0($a0)
  .L800778DC:
    /* 680DC 800778DC FEFF0224 */  addiu      $v0, $zero, -0x2
    /* 680E0 800778E0 0300A214 */  bne        $a1, $v0, .L800778F0
    /* 680E4 800778E4 21100002 */   addu      $v0, $s0, $zero
    /* 680E8 800778E8 FFFF1024 */  addiu      $s0, $zero, -0x1
  .L800778EC:
    /* 680EC 800778EC 21100002 */  addu       $v0, $s0, $zero
  .L800778F0:
    /* 680F0 800778F0 1400BF8F */  lw         $ra, 0x14($sp)
    /* 680F4 800778F4 1000B08F */  lw         $s0, 0x10($sp)
    /* 680F8 800778F8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 680FC 800778FC 0800E003 */  jr         $ra
    /* 68100 80077900 00000000 */   nop
endlabel func_80077894
