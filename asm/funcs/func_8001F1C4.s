glabel func_8001F1C4
    /* F9C4 8001F1C4 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* F9C8 8001F1C8 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* F9CC 8001F1CC 21988000 */  addu       $s3, $a0, $zero
    /* F9D0 8001F1D0 1000B0AF */  sw         $s0, 0x10($sp)
    /* F9D4 8001F1D4 2180A000 */  addu       $s0, $a1, $zero
    /* F9D8 8001F1D8 1400B1AF */  sw         $s1, 0x14($sp)
    /* F9DC 8001F1DC 2188C000 */  addu       $s1, $a2, $zero
    /* F9E0 8001F1E0 2000BFAF */  sw         $ra, 0x20($sp)
    /* F9E4 8001F1E4 1800B2AF */  sw         $s2, 0x18($sp)
    /* F9E8 8001F1E8 18000292 */  lbu        $v0, 0x18($s0)
    /* F9EC 8001F1EC 00000000 */  nop
    /* F9F0 8001F1F0 80004230 */  andi       $v0, $v0, 0x80
    /* F9F4 8001F1F4 05004014 */  bnez       $v0, .L8001F20C
    /* F9F8 8001F1F8 2190E000 */   addu      $s2, $a3, $zero
    /* F9FC 8001F1FC CD9C000C */  jal        func_80027334
    /* FA00 8001F200 21202002 */   addu      $a0, $s1, $zero
    /* FA04 8001F204 CD9C000C */  jal        func_80027334
    /* FA08 8001F208 21204002 */   addu      $a0, $s2, $zero
  .L8001F20C:
    /* FA0C 8001F20C 36002426 */  addiu      $a0, $s1, 0x36
    /* FA10 8001F210 21380000 */  addu       $a3, $zero, $zero
    /* FA14 8001F214 14000582 */  lb         $a1, 0x14($s0)
    /* FA18 8001F218 15000682 */  lb         $a2, 0x15($s0)
    /* FA1C 8001F21C 80280500 */  sll        $a1, $a1, 2
    /* FA20 8001F220 DCBD000C */  jal        func_8002F770
    /* FA24 8001F224 80300600 */   sll       $a2, $a2, 2
    /* FA28 8001F228 36004426 */  addiu      $a0, $s2, 0x36
    /* FA2C 8001F22C 21380000 */  addu       $a3, $zero, $zero
    /* FA30 8001F230 14000582 */  lb         $a1, 0x14($s0)
    /* FA34 8001F234 15000682 */  lb         $a2, 0x15($s0)
    /* FA38 8001F238 80280500 */  sll        $a1, $a1, 2
    /* FA3C 8001F23C DCBD000C */  jal        func_8002F770
    /* FA40 8001F240 80300600 */   sll       $a2, $a2, 2
    /* FA44 8001F244 0C006386 */  lh         $v1, 0xC($s3)
    /* FA48 8001F248 1D000224 */  addiu      $v0, $zero, 0x1D
    /* FA4C 8001F24C 03006210 */  beq        $v1, $v0, .L8001F25C
    /* FA50 8001F250 0E000224 */   addiu     $v0, $zero, 0xE
    /* FA54 8001F254 0B006214 */  bne        $v1, $v0, .L8001F284
    /* FA58 8001F258 00000000 */   nop
  .L8001F25C:
    /* FA5C 8001F25C 16000282 */  lb         $v0, 0x16($s0)
    /* FA60 8001F260 7E002396 */  lhu        $v1, 0x7E($s1)
    /* FA64 8001F264 80100200 */  sll        $v0, $v0, 2
    /* FA68 8001F268 21186200 */  addu       $v1, $v1, $v0
    /* FA6C 8001F26C 7E0023A6 */  sh         $v1, 0x7E($s1)
    /* FA70 8001F270 16000282 */  lb         $v0, 0x16($s0)
    /* FA74 8001F274 7E004396 */  lhu        $v1, 0x7E($s2)
    /* FA78 8001F278 80100200 */  sll        $v0, $v0, 2
    /* FA7C 8001F27C 21186200 */  addu       $v1, $v1, $v0
    /* FA80 8001F280 7E0043A6 */  sh         $v1, 0x7E($s2)
  .L8001F284:
    /* FA84 8001F284 0E006296 */  lhu        $v0, 0xE($s3)
    /* FA88 8001F288 00000000 */  nop
    /* FA8C 8001F28C FAFF4224 */  addiu      $v0, $v0, -0x6
    /* FA90 8001F290 0200422C */  sltiu      $v0, $v0, 0x2
    /* FA94 8001F294 0B004010 */  beqz       $v0, .L8001F2C4
    /* FA98 8001F298 00000000 */   nop
    /* FA9C 8001F29C 16000282 */  lb         $v0, 0x16($s0)
    /* FAA0 8001F2A0 72002396 */  lhu        $v1, 0x72($s1)
    /* FAA4 8001F2A4 80100200 */  sll        $v0, $v0, 2
    /* FAA8 8001F2A8 21186200 */  addu       $v1, $v1, $v0
    /* FAAC 8001F2AC 720023A6 */  sh         $v1, 0x72($s1)
    /* FAB0 8001F2B0 16000282 */  lb         $v0, 0x16($s0)
    /* FAB4 8001F2B4 72004396 */  lhu        $v1, 0x72($s2)
    /* FAB8 8001F2B8 80100200 */  sll        $v0, $v0, 2
    /* FABC 8001F2BC 21186200 */  addu       $v1, $v1, $v0
    /* FAC0 8001F2C0 720043A6 */  sh         $v1, 0x72($s2)
  .L8001F2C4:
    /* FAC4 8001F2C4 2000BF8F */  lw         $ra, 0x20($sp)
    /* FAC8 8001F2C8 1C00B38F */  lw         $s3, 0x1C($sp)
    /* FACC 8001F2CC 1800B28F */  lw         $s2, 0x18($sp)
    /* FAD0 8001F2D0 1400B18F */  lw         $s1, 0x14($sp)
    /* FAD4 8001F2D4 1000B08F */  lw         $s0, 0x10($sp)
    /* FAD8 8001F2D8 2800BD27 */  addiu      $sp, $sp, 0x28
    /* FADC 8001F2DC 0800E003 */  jr         $ra
    /* FAE0 8001F2E0 00000000 */   nop
endlabel func_8001F1C4
