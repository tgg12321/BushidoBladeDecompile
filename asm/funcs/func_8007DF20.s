glabel func_8007DF20
    /* 6E720 8007DF20 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6E724 8007DF24 05008004 */  bltz       $a0, .L8007DF3C
    /* 6E728 8007DF28 1000BFAF */   sw        $ra, 0x10($sp)
    /* 6E72C 8007DF2C D7F7010C */  jal        func_8007DF5C
    /* 6E730 8007DF30 FF0F8430 */   andi      $a0, $a0, 0xFFF
    /* 6E734 8007DF34 D3F70108 */  j          .L8007DF4C
    /* 6E738 8007DF38 00000000 */   nop
  .L8007DF3C:
    /* 6E73C 8007DF3C 23200400 */  negu       $a0, $a0
    /* 6E740 8007DF40 D7F7010C */  jal        func_8007DF5C
    /* 6E744 8007DF44 FF0F8430 */   andi      $a0, $a0, 0xFFF
    /* 6E748 8007DF48 23100200 */  negu       $v0, $v0
  .L8007DF4C:
    /* 6E74C 8007DF4C 1000BF8F */  lw         $ra, 0x10($sp)
    /* 6E750 8007DF50 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6E754 8007DF54 0800E003 */  jr         $ra
    /* 6E758 8007DF58 00000000 */   nop
endlabel func_8007DF20
