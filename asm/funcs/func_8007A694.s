glabel func_8007A694
    /* 6AE94 8007A694 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 6AE98 8007A698 1800B2AF */  sw         $s2, 0x18($sp)
    /* 6AE9C 8007A69C 3800B28F */  lw         $s2, 0x38($sp)
    /* 6AEA0 8007A6A0 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6AEA4 8007A6A4 21888000 */  addu       $s1, $a0, $zero
    /* 6AEA8 8007A6A8 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 6AEAC 8007A6AC 2198A000 */  addu       $s3, $a1, $zero
    /* 6AEB0 8007A6B0 2000B4AF */  sw         $s4, 0x20($sp)
    /* 6AEB4 8007A6B4 21A0C000 */  addu       $s4, $a2, $zero
    /* 6AEB8 8007A6B8 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6AEBC 8007A6BC 2400BFAF */  sw         $ra, 0x24($sp)
    /* 6AEC0 8007A6C0 A20D020C */  jal        sys_GetVideoMode
    /* 6AEC4 8007A6C4 2180E000 */   addu      $s0, $a3, $zero
    /* 6AEC8 8007A6C8 01000324 */  addiu      $v1, $zero, 0x1
    /* 6AECC 8007A6CC 000033A6 */  sh         $s3, 0x0($s1)
    /* 6AED0 8007A6D0 020034A6 */  sh         $s4, 0x2($s1)
    /* 6AED4 8007A6D4 040030A6 */  sh         $s0, 0x4($s1)
    /* 6AED8 8007A6D8 0C0020A6 */  sh         $zero, 0xC($s1)
    /* 6AEDC 8007A6DC 0E0020A6 */  sh         $zero, 0xE($s1)
    /* 6AEE0 8007A6E0 100020A6 */  sh         $zero, 0x10($s1)
    /* 6AEE4 8007A6E4 120020A6 */  sh         $zero, 0x12($s1)
    /* 6AEE8 8007A6E8 190020A2 */  sb         $zero, 0x19($s1)
    /* 6AEEC 8007A6EC 1A0020A2 */  sb         $zero, 0x1A($s1)
    /* 6AEF0 8007A6F0 1B0020A2 */  sb         $zero, 0x1B($s1)
    /* 6AEF4 8007A6F4 160023A2 */  sb         $v1, 0x16($s1)
    /* 6AEF8 8007A6F8 03004010 */  beqz       $v0, .L8007A708
    /* 6AEFC 8007A6FC 060032A6 */   sh        $s2, 0x6($s1)
    /* 6AF00 8007A700 C3E90108 */  j          .L8007A70C
    /* 6AF04 8007A704 2101422A */   slti      $v0, $s2, 0x121
  .L8007A708:
    /* 6AF08 8007A708 0101422A */  slti       $v0, $s2, 0x101
  .L8007A70C:
    /* 6AF0C 8007A70C 170022A2 */  sb         $v0, 0x17($s1)
    /* 6AF10 8007A710 21102002 */  addu       $v0, $s1, $zero
    /* 6AF14 8007A714 0A000324 */  addiu      $v1, $zero, 0xA
    /* 6AF18 8007A718 080053A4 */  sh         $s3, 0x8($v0)
    /* 6AF1C 8007A71C 0A0054A4 */  sh         $s4, 0xA($v0)
    /* 6AF20 8007A720 140043A4 */  sh         $v1, 0x14($v0)
    /* 6AF24 8007A724 180040A0 */  sb         $zero, 0x18($v0)
    /* 6AF28 8007A728 2400BF8F */  lw         $ra, 0x24($sp)
    /* 6AF2C 8007A72C 2000B48F */  lw         $s4, 0x20($sp)
    /* 6AF30 8007A730 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 6AF34 8007A734 1800B28F */  lw         $s2, 0x18($sp)
    /* 6AF38 8007A738 1400B18F */  lw         $s1, 0x14($sp)
    /* 6AF3C 8007A73C 1000B08F */  lw         $s0, 0x10($sp)
    /* 6AF40 8007A740 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 6AF44 8007A744 0800E003 */  jr         $ra
    /* 6AF48 8007A748 00000000 */   nop
endlabel func_8007A694
