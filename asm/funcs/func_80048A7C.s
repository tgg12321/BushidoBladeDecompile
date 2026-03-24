glabel func_80048A7C
    /* 3927C 80048A7C D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* 39280 80048A80 21408000 */  addu       $t0, $a0, $zero
    /* 39284 80048A84 2148A000 */  addu       $t1, $a1, $zero
    /* 39288 80048A88 2150C000 */  addu       $t2, $a2, $zero
    /* 3928C 80048A8C 21200000 */  addu       $a0, $zero, $zero
    /* 39290 80048A90 21280001 */  addu       $a1, $t0, $zero
    /* 39294 80048A94 4000A28F */  lw         $v0, 0x40($sp)
    /* 39298 80048A98 4400A38F */  lw         $v1, 0x44($sp)
    /* 3929C 80048A9C 21302001 */  addu       $a2, $t1, $zero
    /* 392A0 80048AA0 1000A7AF */  sw         $a3, 0x10($sp)
    /* 392A4 80048AA4 21384001 */  addu       $a3, $t2, $zero
    /* 392A8 80048AA8 2800BFAF */  sw         $ra, 0x28($sp)
    /* 392AC 80048AAC 1C00A8AF */  sw         $t0, 0x1C($sp)
    /* 392B0 80048AB0 2000A9AF */  sw         $t1, 0x20($sp)
    /* 392B4 80048AB4 1400A2AF */  sw         $v0, 0x14($sp)
    /* 392B8 80048AB8 1922010C */  jal        func_80048864
    /* 392BC 80048ABC 1800A3AF */   sw        $v1, 0x18($sp)
    /* 392C0 80048AC0 2800BF8F */  lw         $ra, 0x28($sp)
    /* 392C4 80048AC4 3000BD27 */  addiu      $sp, $sp, 0x30
    /* 392C8 80048AC8 0800E003 */  jr         $ra
    /* 392CC 80048ACC 00000000 */   nop
endlabel func_80048A7C
