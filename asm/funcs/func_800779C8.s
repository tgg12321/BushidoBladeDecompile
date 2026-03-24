glabel func_800779C8
    /* 681C8 800779C8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 681CC 800779CC 1400BFAF */  sw         $ra, 0x14($sp)
    /* 681D0 800779D0 B3BA010C */  jal        func_8006EACC
    /* 681D4 800779D4 1000B0AF */   sw        $s0, 0x10($sp)
    /* 681D8 800779D8 21804000 */  addu       $s0, $v0, $zero
    /* 681DC 800779DC 04000012 */  beqz       $s0, .L800779F0
    /* 681E0 800779E0 21100002 */   addu      $v0, $s0, $zero
    /* 681E4 800779E4 BF6D010C */  jal        func_8005B6FC
    /* 681E8 800779E8 00000000 */   nop
    /* 681EC 800779EC 21100002 */  addu       $v0, $s0, $zero
  .L800779F0:
    /* 681F0 800779F0 1400BF8F */  lw         $ra, 0x14($sp)
    /* 681F4 800779F4 1000B08F */  lw         $s0, 0x10($sp)
    /* 681F8 800779F8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 681FC 800779FC 0800E003 */  jr         $ra
    /* 68200 80077A00 00000000 */   nop
endlabel func_800779C8
