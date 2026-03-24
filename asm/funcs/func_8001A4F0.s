glabel func_8001A4F0
    /* ACF0 8001A4F0 FF0F8330 */  andi       $v1, $a0, 0xFFF
    /* ACF4 8001A4F4 00086228 */  slti       $v0, $v1, 0x800
    /* ACF8 8001A4F8 02004014 */  bnez       $v0, .L8001A504
    /* ACFC 8001A4FC 00000000 */   nop
    /* AD00 8001A500 00F06324 */  addiu      $v1, $v1, -0x1000
  .L8001A504:
    /* AD04 8001A504 1A006500 */  div        $zero, $v1, $a1
    /* AD08 8001A508 0200A014 */  bnez       $a1, .L8001A514
    /* AD0C 8001A50C 00000000 */   nop
    /* AD10 8001A510 0D000700 */  break      7
  .L8001A514:
    /* AD14 8001A514 FFFF0124 */  addiu      $at, $zero, -0x1
    /* AD18 8001A518 0400A114 */  bne        $a1, $at, .L8001A52C
    /* AD1C 8001A51C 0080013C */   lui       $at, (0x80000000 >> 16)
    /* AD20 8001A520 02006114 */  bne        $v1, $at, .L8001A52C
    /* AD24 8001A524 00000000 */   nop
    /* AD28 8001A528 0D000600 */  break      6
  .L8001A52C:
    /* AD2C 8001A52C 12100000 */  mflo       $v0
    /* AD30 8001A530 0800E003 */  jr         $ra
    /* AD34 8001A534 00000000 */   nop
endlabel func_8001A4F0
