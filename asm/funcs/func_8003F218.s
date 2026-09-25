glabel func_8003F218
    /* 2FA18 8003F218 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2FA1C 8003F21C 0200822C */  sltiu      $v0, $a0, 0x2
    /* 2FA20 8003F220 0D004010 */  beqz       $v0, .L8003F258
    /* 2FA24 8003F224 1000BFAF */   sw        $ra, 0x10($sp)
    /* 2FA28 8003F228 6001828F */  lw         $v0, %gp_rel(D_800A322C)($gp)
    /* 2FA2C 8003F22C 00000000 */  nop
    /* 2FA30 8003F230 09008210 */  beq        $a0, $v0, .L8003F258
    /* 2FA34 8003F234 00000000 */   nop
    /* 2FA38 8003F238 600184AF */  sw         $a0, %gp_rel(D_800A322C)($gp)
    /* 2FA3C 8003F23C 03008014 */  bnez       $a0, .L8003F24C
    /* 2FA40 8003F240 00000000 */   nop
    /* 2FA44 8003F244 79FC000C */  jal        func_8003F1E4
    /* 2FA48 8003F248 21200000 */   addu      $a0, $zero, $zero
  .L8003F24C:
    /* 2FA4C 8003F24C 6001828F */  lw         $v0, %gp_rel(D_800A322C)($gp)
    /* 2FA50 8003F250 0F80013C */  lui        $at, %hi(D_800F665C)
    /* 2FA54 8003F254 5C6622A4 */  sh         $v0, %lo(D_800F665C)($at)
  .L8003F258:
    /* 2FA58 8003F258 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2FA5C 8003F25C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2FA60 8003F260 0800E003 */  jr         $ra
    /* 2FA64 8003F264 00000000 */   nop
endlabel func_8003F218
