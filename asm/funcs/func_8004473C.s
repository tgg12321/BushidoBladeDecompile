glabel func_8004473C
    /* 34F3C 8004473C E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 34F40 80044740 1800BFAF */  sw         $ra, 0x18($sp)
    /* 34F44 80044744 75FC000C */  jal        game_GetCharData
    /* 34F48 80044748 00000000 */   nop
    /* 34F4C 8004474C 21300000 */  addu       $a2, $zero, $zero
    /* 34F50 80044750 0B80053C */  lui        $a1, %hi(D_800A9D04)
    /* 34F54 80044754 049DA58C */  lw         $a1, %lo(D_800A9D04)($a1)
    /* 34F58 80044758 0B80033C */  lui        $v1, %hi(D_800A9CFE)
    /* 34F5C 8004475C FE9C6384 */  lh         $v1, %lo(D_800A9CFE)($v1)
    /* 34F60 80044760 0B80043C */  lui        $a0, %hi(D_800A9D08)
    /* 34F64 80044764 089D8424 */  addiu      $a0, $a0, %lo(D_800A9D08)
    /* 34F68 80044768 21006018 */  blez       $v1, .L800447F0
    /* 34F6C 8004476C 000082AC */   sw        $v0, 0x0($a0)
    /* 34F70 80044770 F0FF8724 */  addiu      $a3, $a0, -0x10
    /* 34F74 80044774 04000924 */  addiu      $t1, $zero, 0x4
    /* 34F78 80044778 FFFF0824 */  addiu      $t0, $zero, -0x1
    /* 34F7C 8004477C 34004424 */  addiu      $a0, $v0, 0x34
    /* 34F80 80044780 5800A324 */  addiu      $v1, $a1, 0x58
  .L80044784:
    /* 34F84 80044784 0000A0A0 */  sb         $zero, 0x0($a1)
    /* 34F88 80044788 A9FF60A0 */  sb         $zero, -0x57($v1)
    /* 34F8C 8004478C AAFF60A4 */  sh         $zero, -0x56($v1)
    /* 34F90 80044790 0000E294 */  lhu        $v0, 0x0($a3)
    /* 34F94 80044794 B0FF60A4 */  sh         $zero, -0x50($v1)
    /* 34F98 80044798 B4FF60AC */  sw         $zero, -0x4C($v1)
    /* 34F9C 8004479C B2FF69A4 */  sh         $t1, -0x4E($v1)
    /* 34FA0 800447A0 B8FF60A4 */  sh         $zero, -0x48($v1)
    /* 34FA4 800447A4 BAFF60A4 */  sh         $zero, -0x46($v1)
    /* 34FA8 800447A8 BCFF60A4 */  sh         $zero, -0x44($v1)
    /* 34FAC 800447AC ACFF62A4 */  sh         $v0, -0x54($v1)
    /* 34FB0 800447B0 F8FF828C */  lw         $v0, -0x8($a0)
    /* 34FB4 800447B4 6800A524 */  addiu      $a1, $a1, 0x68
    /* 34FB8 800447B8 F4FF62AC */  sw         $v0, -0xC($v1)
    /* 34FBC 800447BC FCFF828C */  lw         $v0, -0x4($a0)
    /* 34FC0 800447C0 0100C624 */  addiu      $a2, $a2, 0x1
    /* 34FC4 800447C4 F8FF62AC */  sw         $v0, -0x8($v1)
    /* 34FC8 800447C8 0000828C */  lw         $v0, 0x0($a0)
    /* 34FCC 800447CC 68008424 */  addiu      $a0, $a0, 0x68
    /* 34FD0 800447D0 AEFF60A4 */  sh         $zero, -0x52($v1)
    /* 34FD4 800447D4 000068AC */  sw         $t0, 0x0($v1)
    /* 34FD8 800447D8 FCFF62AC */  sw         $v0, -0x4($v1)
    /* 34FDC 800447DC 0600E284 */  lh         $v0, 0x6($a3)
    /* 34FE0 800447E0 00000000 */  nop
    /* 34FE4 800447E4 2A10C200 */  slt        $v0, $a2, $v0
    /* 34FE8 800447E8 E6FF4014 */  bnez       $v0, .L80044784
    /* 34FEC 800447EC 68006324 */   addiu     $v1, $v1, 0x68
  .L800447F0:
    /* 34FF0 800447F0 1800BF8F */  lw         $ra, 0x18($sp)
    /* 34FF4 800447F4 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 34FF8 800447F8 0800E003 */  jr         $ra
    /* 34FFC 800447FC 00000000 */   nop
endlabel func_8004473C
