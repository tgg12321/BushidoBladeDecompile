glabel func_800885CC
    /* 78DCC 800885CC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 78DD0 800885D0 1000B0AF */  sw         $s0, 0x10($sp)
    /* 78DD4 800885D4 1400BFAF */  sw         $ra, 0x14($sp)
    /* 78DD8 800885D8 B00A020C */  jal        irq_DisableInterrupts
    /* 78DDC 800885DC 21808000 */   addu      $s0, $a0, $zero
    /* 78DE0 800885E0 D021020C */  jal        func_80088740
    /* 78DE4 800885E4 21200002 */   addu      $a0, $s0, $zero
    /* 78DE8 800885E8 08000016 */  bnez       $s0, .L8008860C
    /* 78DEC 800885EC 00C00434 */   ori       $a0, $zero, 0xC000
    /* 78DF0 800885F0 17000324 */  addiu      $v1, $zero, 0x17
    /* 78DF4 800885F4 0A80023C */  lui        $v0, %hi(D_800A28D2)
    /* 78DF8 800885F8 D2284224 */  addiu      $v0, $v0, %lo(D_800A28D2)
  .L800885FC:
    /* 78DFC 800885FC 000044A4 */  sh         $a0, 0x0($v0)
    /* 78E00 80088600 FFFF6324 */  addiu      $v1, $v1, -0x1
    /* 78E04 80088604 FDFF6104 */  bgez       $v1, .L800885FC
    /* 78E08 80088608 FEFF4224 */   addiu     $v0, $v0, -0x2
  .L8008860C:
    /* 78E0C 8008860C B121020C */  jal        func_800886C4
    /* 78E10 80088610 00000000 */   nop
    /* 78E14 80088614 D1000424 */  addiu      $a0, $zero, 0xD1
    /* 78E18 80088618 0A80053C */  lui        $a1, %hi(D_800A2D44)
    /* 78E1C 8008861C 442DA58C */  lw         $a1, %lo(D_800A2D44)($a1)
    /* 78E20 80088620 0A80013C */  lui        $at, %hi(D_800A287C)
    /* 78E24 80088624 7C2820AC */  sw         $zero, %lo(D_800A287C)($at)
    /* 78E28 80088628 0A80013C */  lui        $at, %hi(D_800A2880)
    /* 78E2C 8008862C 802820AC */  sw         $zero, %lo(D_800A2880)($at)
    /* 78E30 80088630 0A80013C */  lui        $at, %hi(D_800A288C)
    /* 78E34 80088634 8C2820AC */  sw         $zero, %lo(D_800A288C)($at)
    /* 78E38 80088638 0A80013C */  lui        $at, %hi(D_800A2890)
    /* 78E3C 8008863C 902820A4 */  sh         $zero, %lo(D_800A2890)($at)
    /* 78E40 80088640 0A80013C */  lui        $at, %hi(D_800A2892)
    /* 78E44 80088644 922820A4 */  sh         $zero, %lo(D_800A2892)($at)
    /* 78E48 80088648 0A80013C */  lui        $at, %hi(D_800A2894)
    /* 78E4C 8008864C 942820AC */  sw         $zero, %lo(D_800A2894)($at)
    /* 78E50 80088650 0A80013C */  lui        $at, %hi(D_800A2898)
    /* 78E54 80088654 982820AC */  sw         $zero, %lo(D_800A2898)($at)
    /* 78E58 80088658 0A80013C */  lui        $at, %hi(D_800A2884)
    /* 78E5C 8008865C 842825AC */  sw         $a1, %lo(D_800A2884)($at)
    /* 78E60 80088660 2324020C */  jal        func_8008908C
    /* 78E64 80088664 21300000 */   addu      $a2, $zero, $zero
    /* 78E68 80088668 0A80013C */  lui        $at, %hi(D_800A2D38)
    /* 78E6C 8008866C 382D20AC */  sw         $zero, %lo(D_800A2D38)($at)
    /* 78E70 80088670 0A80013C */  lui        $at, %hi(D_800A2D3C)
    /* 78E74 80088674 3C2D20AC */  sw         $zero, %lo(D_800A2D3C)($at)
    /* 78E78 80088678 0A80013C */  lui        $at, %hi(D_800A2D40)
    /* 78E7C 8008867C 402D20AC */  sw         $zero, %lo(D_800A2D40)($at)
    /* 78E80 80088680 0A80013C */  lui        $at, %hi(D_800A2878)
    /* 78E84 80088684 782820AC */  sw         $zero, %lo(D_800A2878)($at)
    /* 78E88 80088688 0A80013C */  lui        $at, %hi(D_800A2CF8)
    /* 78E8C 8008868C F82C20AC */  sw         $zero, %lo(D_800A2CF8)($at)
    /* 78E90 80088690 0A80013C */  lui        $at, %hi(D_800A2874)
    /* 78E94 80088694 742820AC */  sw         $zero, %lo(D_800A2874)($at)
    /* 78E98 80088698 0A80013C */  lui        $at, %hi(D_800A28A0)
    /* 78E9C 8008869C A02820AC */  sw         $zero, %lo(D_800A28A0)($at)
    /* 78EA0 800886A0 0A80013C */  lui        $at, %hi(D_800A289C)
    /* 78EA4 800886A4 9C2820AC */  sw         $zero, %lo(D_800A289C)($at)
    /* 78EA8 800886A8 0A80013C */  lui        $at, %hi(D_800A2CD4)
    /* 78EAC 800886AC D42C20AC */  sw         $zero, %lo(D_800A2CD4)($at)
    /* 78EB0 800886B0 1400BF8F */  lw         $ra, 0x14($sp)
    /* 78EB4 800886B4 1000B08F */  lw         $s0, 0x10($sp)
    /* 78EB8 800886B8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 78EBC 800886BC 0800E003 */  jr         $ra
    /* 78EC0 800886C0 00000000 */   nop
endlabel func_800885CC
