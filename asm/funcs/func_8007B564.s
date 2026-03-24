glabel func_8007B564
    /* 6BD64 8007B564 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 6BD68 8007B568 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 6BD6C 8007B56C 21988000 */  addu       $s3, $a0, $zero
    /* 6BD70 8007B570 0180043C */  lui        $a0, %hi(D_80015F50)
    /* 6BD74 8007B574 505F8424 */  addiu      $a0, $a0, %lo(D_80015F50)
    /* 6BD78 8007B578 1800B2AF */  sw         $s2, 0x18($sp)
    /* 6BD7C 8007B57C 2190A000 */  addu       $s2, $a1, $zero
    /* 6BD80 8007B580 21286002 */  addu       $a1, $s3, $zero
    /* 6BD84 8007B584 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6BD88 8007B588 2180C000 */  addu       $s0, $a2, $zero
    /* 6BD8C 8007B58C 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6BD90 8007B590 2000BFAF */  sw         $ra, 0x20($sp)
    /* 6BD94 8007B594 EAEC010C */  jal        func_8007B3A8
    /* 6BD98 8007B598 2188E000 */   addu      $s1, $a3, $zero
    /* 6BD9C 8007B59C 21286002 */  addu       $a1, $s3, $zero
    /* 6BDA0 8007B5A0 FF003132 */  andi       $s1, $s1, 0xFF
    /* 6BDA4 8007B5A4 008C1100 */  sll        $s1, $s1, 16
    /* 6BDA8 8007B5A8 FF001032 */  andi       $s0, $s0, 0xFF
    /* 6BDAC 8007B5AC 00821000 */  sll        $s0, $s0, 8
    /* 6BDB0 8007B5B0 0080023C */  lui        $v0, (0x80000000 >> 16)
    /* 6BDB4 8007B5B4 25800202 */  or         $s0, $s0, $v0
    /* 6BDB8 8007B5B8 25883002 */  or         $s1, $s1, $s0
    /* 6BDBC 8007B5BC FF005232 */  andi       $s2, $s2, 0xFF
    /* 6BDC0 8007B5C0 0A80033C */  lui        $v1, %hi(D_8009BE6C)
    /* 6BDC4 8007B5C4 6CBE638C */  lw         $v1, %lo(D_8009BE6C)($v1)
    /* 6BDC8 8007B5C8 08000624 */  addiu      $a2, $zero, 0x8
    /* 6BDCC 8007B5CC 0C00648C */  lw         $a0, 0xC($v1)
    /* 6BDD0 8007B5D0 0800628C */  lw         $v0, 0x8($v1)
    /* 6BDD4 8007B5D4 00000000 */  nop
    /* 6BDD8 8007B5D8 09F84000 */  jalr       $v0
    /* 6BDDC 8007B5DC 25383202 */   or        $a3, $s1, $s2
    /* 6BDE0 8007B5E0 2000BF8F */  lw         $ra, 0x20($sp)
    /* 6BDE4 8007B5E4 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 6BDE8 8007B5E8 1800B28F */  lw         $s2, 0x18($sp)
    /* 6BDEC 8007B5EC 1400B18F */  lw         $s1, 0x14($sp)
    /* 6BDF0 8007B5F0 1000B08F */  lw         $s0, 0x10($sp)
    /* 6BDF4 8007B5F4 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 6BDF8 8007B5F8 0800E003 */  jr         $ra
    /* 6BDFC 8007B5FC 00000000 */   nop
endlabel func_8007B564
