glabel func_8005C5A8
    /* 4CDA8 8005C5A8 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 4CDAC 8005C5AC 1400B1AF */  sw         $s1, 0x14($sp)
    /* 4CDB0 8005C5B0 21888000 */  addu       $s1, $a0, $zero
    /* 4CDB4 8005C5B4 1000B0AF */  sw         $s0, 0x10($sp)
    /* 4CDB8 8005C5B8 00840500 */  sll        $s0, $a1, 16
    /* 4CDBC 8005C5BC 03841000 */  sra        $s0, $s0, 16
    /* 4CDC0 8005C5C0 1800BFAF */  sw         $ra, 0x18($sp)
    /* 4CDC4 8005C5C4 D91F020C */  jal        func_80087F64
    /* 4CDC8 8005C5C8 21200002 */   addu      $a0, $s0, $zero
    /* 4CDCC 8005C5CC 0400248E */  lw         $a0, 0x4($s1)
    /* 4CDD0 8005C5D0 3803868F */  lw         $a2, %gp_rel(D_800A3404)($gp)
    /* 4CDD4 8005C5D4 2220020C */  jal        func_80088088
    /* 4CDD8 8005C5D8 21280002 */   addu      $a1, $s0, $zero
    /* 4CDDC 8005C5DC 0400228E */  lw         $v0, 0x4($s1)
    /* 4CDE0 8005C5E0 00000000 */  nop
    /* 4CDE4 8005C5E4 080050AC */  sw         $s0, 0x8($v0)
    /* 4CDE8 8005C5E8 0800248E */  lw         $a0, 0x8($s1)
    /* 4CDEC 8005C5EC 3121020C */  jal        func_800884C4
    /* 4CDF0 8005C5F0 21280002 */   addu      $a1, $s0, $zero
    /* 4CDF4 8005C5F4 00140200 */  sll        $v0, $v0, 16
    /* 4CDF8 8005C5F8 03140200 */  sra        $v0, $v0, 16
    /* 4CDFC 8005C5FC 1800BF8F */  lw         $ra, 0x18($sp)
    /* 4CE00 8005C600 1400B18F */  lw         $s1, 0x14($sp)
    /* 4CE04 8005C604 1000B08F */  lw         $s0, 0x10($sp)
    /* 4CE08 8005C608 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 4CE0C 8005C60C 0800E003 */  jr         $ra
    /* 4CE10 8005C610 00000000 */   nop
endlabel func_8005C5A8
