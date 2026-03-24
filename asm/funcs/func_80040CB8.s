glabel func_80040CB8
    /* 314B8 80040CB8 F8FFBD27 */  addiu      $sp, $sp, -0x8
    /* 314BC 80040CBC B4088624 */  addiu      $a2, $a0, 0x8B4
    /* 314C0 80040CC0 21480000 */  addu       $t1, $zero, $zero
    /* 314C4 80040CC4 FFFF0C24 */  addiu      $t4, $zero, -0x1
    /* 314C8 80040CC8 03000B24 */  addiu      $t3, $zero, 0x3
    /* 314CC 80040CCC 01000A24 */  addiu      $t2, $zero, 0x1
    /* 314D0 80040CD0 94008824 */  addiu      $t0, $a0, 0x94
    /* 314D4 80040CD4 0980073C */  lui        $a3, %hi(D_80094B9E)
    /* 314D8 80040CD8 9E4BE724 */  addiu      $a3, $a3, %lo(D_80094B9E)
    /* 314DC 80040CDC 0C098524 */  addiu      $a1, $a0, 0x90C
  .L80040CE0:
    /* 314E0 80040CE0 0000E284 */  lh         $v0, 0x0($a3)
    /* 314E4 80040CE4 00000000 */  nop
    /* 314E8 80040CE8 0D004C10 */  beq        $v0, $t4, .L80040D20
    /* 314EC 80040CEC 21184000 */   addu      $v1, $v0, $zero
    /* 314F0 80040CF0 AAFFA3A4 */  sh         $v1, -0x56($a1)
    /* 314F4 80040CF4 0000CBA0 */  sb         $t3, 0x0($a2)
    /* 314F8 80040CF8 A9FFA0A0 */  sb         $zero, -0x57($a1)
    /* 314FC 80040CFC B0FFA0A4 */  sh         $zero, -0x50($a1)
    /* 31500 80040D00 B4FFA8AC */  sw         $t0, -0x4C($a1)
    /* 31504 80040D04 AEFFAAA4 */  sh         $t2, -0x52($a1)
    /* 31508 80040D08 B2FFA0A4 */  sh         $zero, -0x4E($a1)
    /* 3150C 80040D0C 16008294 */  lhu        $v0, 0x16($a0)
    /* 31510 80040D10 6800C624 */  addiu      $a2, $a2, 0x68
    /* 31514 80040D14 0000A0AC */  sw         $zero, 0x0($a1)
    /* 31518 80040D18 ACFFA2A4 */  sh         $v0, -0x54($a1)
    /* 3151C 80040D1C 6800A524 */  addiu      $a1, $a1, 0x68
  .L80040D20:
    /* 31520 80040D20 68000825 */  addiu      $t0, $t0, 0x68
    /* 31524 80040D24 01002925 */  addiu      $t1, $t1, 0x1
    /* 31528 80040D28 12002229 */  slti       $v0, $t1, 0x12
    /* 3152C 80040D2C ECFF4014 */  bnez       $v0, .L80040CE0
    /* 31530 80040D30 0A00E724 */   addiu     $a3, $a3, 0xA
    /* 31534 80040D34 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 31538 80040D38 0200C2A4 */  sh         $v0, 0x2($a2)
    /* 3153C 80040D3C 0800BD27 */  addiu      $sp, $sp, 0x8
    /* 31540 80040D40 0800E003 */  jr         $ra
    /* 31544 80040D44 00000000 */   nop
endlabel func_80040CB8
