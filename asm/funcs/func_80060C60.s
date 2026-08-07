glabel func_80060C60
    /* 51460 80060C60 21180000 */  addu       $v1, $zero, $zero
    /* 51464 80060C64 0F80043C */  lui        $a0, %hi(D_800F10D0)
    /* 51468 80060C68 D0108424 */  addiu      $a0, $a0, %lo(D_800F10D0)
  .L80060C6C:
    /* 5146C 80060C6C 000080AC */  sw         $zero, 0x0($a0)
    /* 51470 80060C70 0F80013C */  lui        $at, %hi(D_800F1150)
    /* 51474 80060C74 21082300 */  addu       $at, $at, $v1
    /* 51478 80060C78 501120A0 */  sb         $zero, %lo(D_800F1150)($at)
    /* 5147C 80060C7C 01006324 */  addiu      $v1, $v1, 0x1
    /* 51480 80060C80 1C006228 */  slti       $v0, $v1, 0x1C
    /* 51484 80060C84 F9FF4014 */  bnez       $v0, .L80060C6C
    /* 51488 80060C88 04008424 */   addiu     $a0, $a0, 0x4
    /* 5148C 80060C8C 920380A7 */  sh         $zero, %gp_rel(D_800A345E)($gp)
    /* 51490 80060C90 900380A7 */  sh         $zero, %gp_rel(D_800A345C)($gp)
    /* 51494 80060C94 8C0380AF */  sw         $zero, %gp_rel(D_800A3458)($gp)
    /* 51498 80060C98 880380AF */  sw         $zero, %gp_rel(D_800A3454)($gp)
    /* 5149C 80060C9C 840380AF */  sw         $zero, %gp_rel(D_800A3450)($gp)
    /* 514A0 80060CA0 800380AF */  sw         $zero, %gp_rel(D_800A344C)($gp)
    /* 514A4 80060CA4 940380AF */  sw         $zero, %gp_rel(D_800A3460)($gp)
    /* 514A8 80060CA8 780380AF */  sw         $zero, %gp_rel(D_800A3444)($gp)
    /* 514AC 80060CAC 7C0380AF */  sw         $zero, %gp_rel(D_800A3448)($gp)
    /* 514B0 80060CB0 0800E003 */  jr         $ra
    /* 514B4 80060CB4 00000000 */   nop
endlabel func_80060C60
