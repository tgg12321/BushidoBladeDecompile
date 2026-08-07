glabel func_8003E164
    /* 2E964 8003E164 5C01828F */  lw         $v0, %gp_rel(D_800A3228)($gp)
    /* 2E968 8003E168 D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* 2E96C 8003E16C 2400B1AF */  sw         $s1, 0x24($sp)
    /* 2E970 8003E170 21888000 */  addu       $s1, $a0, $zero
    /* 2E974 8003E174 2800BFAF */  sw         $ra, 0x28($sp)
    /* 2E978 8003E178 25005110 */  beq        $v0, $s1, .L8003E210
    /* 2E97C 8003E17C 2000B0AF */   sw        $s0, 0x20($sp)
    /* 2E980 8003E180 8BF8000C */  jal        func_8003E22C
    /* 2E984 8003E184 00000000 */   nop
    /* 2E988 8003E188 4F05010C */  jal        func_8004153C
    /* 2E98C 8003E18C 21202002 */   addu      $a0, $s1, $zero
    /* 2E990 8003E190 21804000 */  addu       $s0, $v0, $zero
    /* 2E994 8003E194 1E000012 */  beqz       $s0, .L8003E210
    /* 2E998 8003E198 00000000 */   nop
    /* 2E99C 8003E19C 02002016 */  bnez       $s1, .L8003E1A8
    /* 2E9A0 8003E1A0 00030224 */   addiu     $v0, $zero, 0x300
    /* 2E9A4 8003E1A4 80020224 */  addiu      $v0, $zero, 0x280
  .L8003E1A8:
    /* 2E9A8 8003E1A8 1800A2A7 */  sh         $v0, 0x18($sp)
    /* 2E9AC 8003E1AC 1800A427 */  addiu      $a0, $sp, 0x18
    /* 2E9B0 8003E1B0 40010524 */  addiu      $a1, $zero, 0x140
    /* 2E9B4 8003E1B4 E8010624 */  addiu      $a2, $zero, 0x1E8
    /* 2E9B8 8003E1B8 F8000224 */  addiu      $v0, $zero, 0xF8
    /* 2E9BC 8003E1BC 1A00A2A7 */  sh         $v0, 0x1A($sp)
    /* 2E9C0 8003E1C0 40000224 */  addiu      $v0, $zero, 0x40
    /* 2E9C4 8003E1C4 1C00A2A7 */  sh         $v0, 0x1C($sp)
    /* 2E9C8 8003E1C8 06000224 */  addiu      $v0, $zero, 0x6
    /* 2E9CC 8003E1CC B2ED010C */  jal        func_8007B6C8
    /* 2E9D0 8003E1D0 1E00A2A7 */   sh        $v0, 0x1E($sp)
    /* 2E9D4 8003E1D4 04002016 */  bnez       $s1, .L8003E1E8
    /* 2E9D8 8003E1D8 21280000 */   addu      $a1, $zero, $zero
    /* 2E9DC 8003E1DC 21300000 */  addu       $a2, $zero, $zero
    /* 2E9E0 8003E1E0 7CF80008 */  j          .L8003E1F0
    /* 2E9E4 8003E1E4 C0FE0724 */   addiu     $a3, $zero, -0x140
  .L8003E1E8:
    /* 2E9E8 8003E1E8 21300000 */  addu       $a2, $zero, $zero
    /* 2E9EC 8003E1EC 40FE0724 */  addiu      $a3, $zero, -0x1C0
  .L8003E1F0:
    /* 2E9F0 8003E1F0 14000486 */  lh         $a0, 0x14($s0)
    /* 2E9F4 8003E1F4 E8000224 */  addiu      $v0, $zero, 0xE8
    /* 2E9F8 8003E1F8 A80C010C */  jal        func_800432A0
    /* 2E9FC 8003E1FC 1000A2AF */   sw        $v0, 0x10($sp)
    /* 2EA00 8003E200 CFEC010C */  jal        gpu_DrawSync
    /* 2EA04 8003E204 21200000 */   addu      $a0, $zero, $zero
    /* 2EA08 8003E208 48F8000C */  jal        func_8003E120
    /* 2EA0C 8003E20C 00000000 */   nop
  .L8003E210:
    /* 2EA10 8003E210 5C0191AF */  sw         $s1, %gp_rel(D_800A3228)($gp)
    /* 2EA14 8003E214 2800BF8F */  lw         $ra, 0x28($sp)
    /* 2EA18 8003E218 2400B18F */  lw         $s1, 0x24($sp)
    /* 2EA1C 8003E21C 2000B08F */  lw         $s0, 0x20($sp)
    /* 2EA20 8003E220 3000BD27 */  addiu      $sp, $sp, 0x30
    /* 2EA24 8003E224 0800E003 */  jr         $ra
    /* 2EA28 8003E228 00000000 */   nop
endlabel func_8003E164
