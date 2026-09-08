glabel SsUtKeyOffV
    /* 765D4 80085DD4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 765D8 80085DD8 21288000 */  addu       $a1, $a0, $zero
    /* 765DC 80085DDC 1080023C */  lui        $v0, %hi(_snd_ev_flag)
    /* 765E0 80085DE0 30F6428C */  lw         $v0, %lo(_snd_ev_flag)($v0)
    /* 765E4 80085DE4 01000324 */  addiu      $v1, $zero, 0x1
    /* 765E8 80085DE8 03004314 */  bne        $v0, $v1, .L80085DF8
    /* 765EC 80085DEC 1000BFAF */   sw        $ra, 0x10($sp)
    /* 765F0 80085DF0 8F170208 */  j          .L80085E3C
    /* 765F4 80085DF4 FFFF0224 */   addiu     $v0, $zero, -0x1
  .L80085DF8:
    /* 765F8 80085DF8 1080013C */  lui        $at, %hi(_snd_ev_flag)
    /* 765FC 80085DFC 30F623AC */  sw         $v1, %lo(_snd_ev_flag)($at)
    /* 76600 80085E00 FFFF8230 */  andi       $v0, $a0, 0xFFFF
    /* 76604 80085E04 1800422C */  sltiu      $v0, $v0, 0x18
    /* 76608 80085E08 05004014 */  bnez       $v0, .L80085E20
    /* 7660C 80085E0C FFFF0224 */   addiu     $v0, $zero, -0x1
    /* 76610 80085E10 1080013C */  lui        $at, %hi(_snd_ev_flag)
    /* 76614 80085E14 30F620AC */  sw         $zero, %lo(_snd_ev_flag)($at)
    /* 76618 80085E18 8F170208 */  j          .L80085E3C
    /* 7661C 80085E1C 00000000 */   nop
  .L80085E20:
    /* 76620 80085E20 1080013C */  lui        $at, %hi(D_8010280A)
    /* 76624 80085E24 0A2825A4 */  sh         $a1, %lo(D_8010280A)($at)
    /* 76628 80085E28 751C020C */  jal        _SsVmKeyOffNow
    /* 7662C 80085E2C 21200000 */   addu      $a0, $zero, $zero
    /* 76630 80085E30 21100000 */  addu       $v0, $zero, $zero
    /* 76634 80085E34 1080013C */  lui        $at, %hi(_snd_ev_flag)
    /* 76638 80085E38 30F620AC */  sw         $zero, %lo(_snd_ev_flag)($at)
  .L80085E3C:
    /* 7663C 80085E3C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 76640 80085E40 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 76644 80085E44 0800E003 */  jr         $ra
    /* 76648 80085E48 00000000 */   nop
endlabel SsUtKeyOffV
