glabel func_8003F168
    /* 2F968 8003F168 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2F96C 8003F16C 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2F970 8003F170 E619010C */  jal        func_80046798
    /* 2F974 8003F174 00000000 */   nop
    /* 2F978 8003F178 C0100200 */  sll        $v0, $v0, 3
    /* 2F97C 8003F17C 0980013C */  lui        $at, %hi(D_800948BC)
    /* 2F980 8003F180 21082200 */  addu       $at, $at, $v0
    /* 2F984 8003F184 BC48228C */  lw         $v0, %lo(D_800948BC)($at)
    /* 2F988 8003F188 00000000 */  nop
    /* 2F98C 8003F18C 0A004010 */  beqz       $v0, .L8003F1B8
    /* 2F990 8003F190 00000000 */   nop
    /* 2F994 8003F194 E619010C */  jal        func_80046798
    /* 2F998 8003F198 00000000 */   nop
    /* 2F99C 8003F19C C0100200 */  sll        $v0, $v0, 3
    /* 2F9A0 8003F1A0 0980013C */  lui        $at, %hi(D_800948BC)
    /* 2F9A4 8003F1A4 21082200 */  addu       $at, $at, $v0
    /* 2F9A8 8003F1A8 BC48228C */  lw         $v0, %lo(D_800948BC)($at)
    /* 2F9AC 8003F1AC 00000000 */  nop
    /* 2F9B0 8003F1B0 09F84000 */  jalr       $v0
    /* 2F9B4 8003F1B4 00000000 */   nop
  .L8003F1B8:
    /* 2F9B8 8003F1B8 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2F9BC 8003F1BC 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2F9C0 8003F1C0 0800E003 */  jr         $ra
    /* 2F9C4 8003F1C4 00000000 */   nop
endlabel func_8003F168
