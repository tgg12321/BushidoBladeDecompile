glabel func_8003D9A0
    /* 2E1A0 8003D9A0 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 2E1A4 8003D9A4 1800B0AF */  sw         $s0, 0x18($sp)
    /* 2E1A8 8003D9A8 21808000 */  addu       $s0, $a0, $zero
    /* 2E1AC 8003D9AC 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 2E1B0 8003D9B0 2188C000 */  addu       $s1, $a2, $zero
    /* 2E1B4 8003D9B4 2000B2AF */  sw         $s2, 0x20($sp)
    /* 2E1B8 8003D9B8 3000BFAF */  sw         $ra, 0x30($sp)
    /* 2E1BC 8003D9BC 2C00B5AF */  sw         $s5, 0x2C($sp)
    /* 2E1C0 8003D9C0 2800B4AF */  sw         $s4, 0x28($sp)
    /* 2E1C4 8003D9C4 2400B3AF */  sw         $s3, 0x24($sp)
    /* 2E1C8 8003D9C8 00001486 */  lh         $s4, 0x0($s0)
    /* 2E1CC 8003D9CC 02001386 */  lh         $s3, 0x2($s0)
    /* 2E1D0 8003D9D0 2400A010 */  beqz       $a1, .L8003DA64
    /* 2E1D4 8003D9D4 FFFFB224 */   addiu     $s2, $a1, -0x1
    /* 2E1D8 8003D9D8 FFFF1524 */  addiu      $s5, $zero, -0x1
  .L8003D9DC:
    /* 2E1DC 8003D9DC 02000296 */  lhu        $v0, 0x2($s0)
    /* 2E1E0 8003D9E0 06000396 */  lhu        $v1, 0x6($s0)
    /* 2E1E4 8003D9E4 00000000 */  nop
    /* 2E1E8 8003D9E8 21104300 */  addu       $v0, $v0, $v1
    /* 2E1EC 8003D9EC 020002A6 */  sh         $v0, 0x2($s0)
    /* 2E1F0 8003D9F0 00140200 */  sll        $v0, $v0, 16
    /* 2E1F4 8003D9F4 03140200 */  sra        $v0, $v0, 16
    /* 2E1F8 8003D9F8 00024228 */  slti       $v0, $v0, 0x200
    /* 2E1FC 8003D9FC 06004014 */  bnez       $v0, .L8003DA18
    /* 2E200 8003DA00 21202002 */   addu      $a0, $s1, $zero
    /* 2E204 8003DA04 00000296 */  lhu        $v0, 0x0($s0)
    /* 2E208 8003DA08 04000396 */  lhu        $v1, 0x4($s0)
    /* 2E20C 8003DA0C 020013A6 */  sh         $s3, 0x2($s0)
    /* 2E210 8003DA10 21104300 */  addu       $v0, $v0, $v1
    /* 2E214 8003DA14 000002A6 */  sh         $v0, 0x0($s0)
  .L8003DA18:
    /* 2E218 8003DA18 21280002 */  addu       $a1, $s0, $zero
    /* 2E21C 8003DA1C 21308002 */  addu       $a2, $s4, $zero
    /* 2E220 8003DA20 EEEA010C */  jal        initLoadImage
    /* 2E224 8003DA24 21386002 */   addu      $a3, $s3, $zero
    /* 2E228 8003DA28 0000228E */  lw         $v0, 0x0($s1)
    /* 2E22C 8003DA2C 0400238E */  lw         $v1, 0x4($s1)
    /* 2E230 8003DA30 0800248E */  lw         $a0, 0x8($s1)
    /* 2E234 8003DA34 0C00258E */  lw         $a1, 0xC($s1)
    /* 2E238 8003DA38 180022AE */  sw         $v0, 0x18($s1)
    /* 2E23C 8003DA3C 1C0023AE */  sw         $v1, 0x1C($s1)
    /* 2E240 8003DA40 200024AE */  sw         $a0, 0x20($s1)
    /* 2E244 8003DA44 240025AE */  sw         $a1, 0x24($s1)
    /* 2E248 8003DA48 1000228E */  lw         $v0, 0x10($s1)
    /* 2E24C 8003DA4C 1400238E */  lw         $v1, 0x14($s1)
    /* 2E250 8003DA50 280022AE */  sw         $v0, 0x28($s1)
    /* 2E254 8003DA54 2C0023AE */  sw         $v1, 0x2C($s1)
    /* 2E258 8003DA58 FFFF5226 */  addiu      $s2, $s2, -0x1
    /* 2E25C 8003DA5C DFFF5516 */  bne        $s2, $s5, .L8003D9DC
    /* 2E260 8003DA60 30003126 */   addiu     $s1, $s1, 0x30
  .L8003DA64:
    /* 2E264 8003DA64 3000BF8F */  lw         $ra, 0x30($sp)
    /* 2E268 8003DA68 2C00B58F */  lw         $s5, 0x2C($sp)
    /* 2E26C 8003DA6C 2800B48F */  lw         $s4, 0x28($sp)
    /* 2E270 8003DA70 2400B38F */  lw         $s3, 0x24($sp)
    /* 2E274 8003DA74 2000B28F */  lw         $s2, 0x20($sp)
    /* 2E278 8003DA78 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 2E27C 8003DA7C 1800B08F */  lw         $s0, 0x18($sp)
    /* 2E280 8003DA80 3800BD27 */  addiu      $sp, $sp, 0x38
    /* 2E284 8003DA84 0800E003 */  jr         $ra
    /* 2E288 8003DA88 00000000 */   nop
endlabel func_8003D9A0
