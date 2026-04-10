glabel game_SetControllerPorts
    /* 2F9E4 8003F1E4 05008010 */  beqz       $a0, .L8003F1FC
    /* 2F9E8 8003F1E8 03000224 */   addiu     $v0, $zero, 0x3
    /* 2F9EC 8003F1EC 0F80013C */  lui        $at, %hi(D_800F6656)
    /* 2F9F0 8003F1F0 566622A4 */  sh         $v0, %lo(D_800F6656)($at)
    /* 2F9F4 8003F1F4 82FC0008 */  j          .L8003F208
    /* 2F9F8 8003F1F8 02000224 */   addiu     $v0, $zero, 0x2
  .L8003F1FC:
    /* 2F9FC 8003F1FC 01000224 */  addiu      $v0, $zero, 0x1
    /* 2FA00 8003F200 0F80013C */  lui        $at, %hi(D_800F6656)
    /* 2FA04 8003F204 566620A4 */  sh         $zero, %lo(D_800F6656)($at)
  .L8003F208:
    /* 2FA08 8003F208 0F80013C */  lui        $at, %hi(D_800F6658)
    /* 2FA0C 8003F20C 586622A4 */  sh         $v0, %lo(D_800F6658)($at)
    /* 2FA10 8003F210 0800E003 */  jr         $ra
    /* 2FA14 8003F214 00000000 */   nop
endlabel game_SetControllerPorts
