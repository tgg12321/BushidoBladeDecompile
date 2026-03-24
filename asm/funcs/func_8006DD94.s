glabel func_8006DD94
    /* 5E594 8006DD94 88FFBD27 */  addiu      $sp, $sp, -0x78
    /* 5E598 8006DD98 5800B0AF */  sw         $s0, 0x58($sp)
    /* 5E59C 8006DD9C 21808000 */  addu       $s0, $a0, $zero
    /* 5E5A0 8006DDA0 6C00B5AF */  sw         $s5, 0x6C($sp)
    /* 5E5A4 8006DDA4 21A80000 */  addu       $s5, $zero, $zero
    /* 5E5A8 8006DDA8 5C00B1AF */  sw         $s1, 0x5C($sp)
    /* 5E5AC 8006DDAC 21880000 */  addu       $s1, $zero, $zero
    /* 5E5B0 8006DDB0 0A000224 */  addiu      $v0, $zero, 0xA
    /* 5E5B4 8006DDB4 7000BFAF */  sw         $ra, 0x70($sp)
    /* 5E5B8 8006DDB8 6800B4AF */  sw         $s4, 0x68($sp)
    /* 5E5BC 8006DDBC 6400B3AF */  sw         $s3, 0x64($sp)
    /* 5E5C0 8006DDC0 6000B2AF */  sw         $s2, 0x60($sp)
    /* 5E5C4 8006DDC4 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 5E5C8 8006DDC8 0400028E */  lw         $v0, 0x4($s0)
    /* 5E5CC 8006DDCC 80001424 */  addiu      $s4, $zero, 0x80
    /* 5E5D0 8006DDD0 3C00528C */  lw         $s2, 0x3C($v0)
    /* 5E5D4 8006DDD4 40001324 */  addiu      $s3, $zero, 0x40
    /* 5E5D8 8006DDD8 3000A0AF */  sw         $zero, 0x30($sp)
    /* 5E5DC 8006DDDC 2800A0AF */  sw         $zero, 0x28($sp)
  .L8006DDE0:
    /* 5E5E0 8006DDE0 01000224 */  addiu      $v0, $zero, 0x1
    /* 5E5E4 8006DDE4 001C1100 */  sll        $v1, $s1, 16
    /* 5E5E8 8006DDE8 4000A2A3 */  sb         $v0, 0x40($sp)
    /* 5E5EC 8006DDEC 6004828F */  lw         $v0, %gp_rel(D_800A352C)($gp)
    /* 5E5F0 8006DDF0 031C0300 */  sra        $v1, $v1, 16
    /* 5E5F4 8006DDF4 01004224 */  addiu      $v0, $v0, 0x1
    /* 5E5F8 8006DDF8 10006214 */  bne        $v1, $v0, .L8006DE3C
    /* 5E5FC 8006DDFC 00000000 */   nop
    /* 5E600 8006DE00 4804848F */  lw         $a0, %gp_rel(D_800A3514)($gp)
    /* 5E604 8006DE04 3004828F */  lw         $v0, %gp_rel(D_800A34FC)($gp)
    /* 5E608 8006DE08 1F008430 */  andi       $a0, $a0, 0x1F
    /* 5E60C 8006DE0C C0210400 */  sll        $a0, $a0, 7
    /* 5E610 8006DE10 0E004284 */  lh         $v0, 0xE($v0)
    /* 5E614 8006DE14 FF018424 */  addiu      $a0, $a0, 0x1FF
    /* 5E618 8006DE18 C8F7010C */  jal        func_8007DF20
    /* 5E61C 8006DE1C 3400A2AF */   sw        $v0, 0x34($sp)
    /* 5E620 8006DE20 40110200 */  sll        $v0, $v0, 5
    /* 5E624 8006DE24 03130200 */  sra        $v0, $v0, 12
    /* 5E628 8006DE28 80FF4224 */  addiu      $v0, $v0, -0x80
    /* 5E62C 8006DE2C 4300A2A3 */  sb         $v0, 0x43($sp)
    /* 5E630 8006DE30 4200A2A3 */  sb         $v0, 0x42($sp)
    /* 5E634 8006DE34 99B70108 */  j          .L8006DE64
    /* 5E638 8006DE38 4100A2A3 */   sb        $v0, 0x41($sp)
  .L8006DE3C:
    /* 5E63C 8006DE3C 05006014 */  bnez       $v1, .L8006DE54
    /* 5E640 8006DE40 00000000 */   nop
    /* 5E644 8006DE44 4300B4A3 */  sb         $s4, 0x43($sp)
    /* 5E648 8006DE48 4200B4A3 */  sb         $s4, 0x42($sp)
    /* 5E64C 8006DE4C 98B70108 */  j          .L8006DE60
    /* 5E650 8006DE50 4100B4A3 */   sb        $s4, 0x41($sp)
  .L8006DE54:
    /* 5E654 8006DE54 4300B3A3 */  sb         $s3, 0x43($sp)
    /* 5E658 8006DE58 4200B3A3 */  sb         $s3, 0x42($sp)
    /* 5E65C 8006DE5C 4100B3A3 */  sb         $s3, 0x41($sp)
  .L8006DE60:
    /* 5E660 8006DE60 3400A0AF */  sw         $zero, 0x34($sp)
  .L8006DE64:
    /* 5E664 8006DE64 00141100 */  sll        $v0, $s1, 16
    /* 5E668 8006DE68 83130200 */  sra        $v0, $v0, 14
    /* 5E66C 8006DE6C 21105200 */  addu       $v0, $v0, $s2
    /* 5E670 8006DE70 2000428C */  lw         $v0, 0x20($v0)
    /* 5E674 8006DE74 00000000 */  nop
    /* 5E678 8006DE78 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5E67C 8006DE7C 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5E680 8006DE80 1C00A2AF */  sw         $v0, 0x1C($sp)
    /* 5E684 8006DE84 1400028E */  lw         $v0, 0x14($s0)
    /* 5E688 8006DE88 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5E68C 8006DE8C 4BCD010C */  jal        func_8007352C
    /* 5E690 8006DE90 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5E694 8006DE94 140002AE */  sw         $v0, 0x14($s0)
    /* 5E698 8006DE98 1800A48F */  lw         $a0, 0x18($sp)
    /* 5E69C 8006DE9C 20B9010C */  jal        func_8006E480
    /* 5E6A0 8006DEA0 2128A002 */   addu      $a1, $s5, $zero
    /* 5E6A4 8006DEA4 01000524 */  addiu      $a1, $zero, 0x1
    /* 5E6A8 8006DEA8 21300000 */  addu       $a2, $zero, $zero
    /* 5E6AC 8006DEAC 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5E6B0 8006DEB0 1C00048E */  lw         $a0, 0x1C($s0)
    /* 5E6B4 8006DEB4 92F0010C */  jal        func_8007C248
    /* 5E6B8 8006DEB8 21384000 */   addu      $a3, $v0, $zero
    /* 5E6BC 8006DEBC 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5E6C0 8006DEC0 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5E6C4 8006DEC4 1C00058E */  lw         $a1, 0x1C($s0)
    /* 5E6C8 8006DEC8 2DEA010C */  jal        func_8007A8B4
    /* 5E6CC 8006DECC 28008424 */   addiu     $a0, $a0, 0x28
    /* 5E6D0 8006DED0 01002226 */  addiu      $v0, $s1, 0x1
    /* 5E6D4 8006DED4 21884000 */  addu       $s1, $v0, $zero
    /* 5E6D8 8006DED8 00140200 */  sll        $v0, $v0, 16
    /* 5E6DC 8006DEDC 03140200 */  sra        $v0, $v0, 16
    /* 5E6E0 8006DEE0 1C00038E */  lw         $v1, 0x1C($s0)
    /* 5E6E4 8006DEE4 03004228 */  slti       $v0, $v0, 0x3
    /* 5E6E8 8006DEE8 0C006324 */  addiu      $v1, $v1, 0xC
    /* 5E6EC 8006DEEC BCFF4014 */  bnez       $v0, .L8006DDE0
    /* 5E6F0 8006DEF0 1C0003AE */   sw        $v1, 0x1C($s0)
    /* 5E6F4 8006DEF4 14000426 */  addiu      $a0, $s0, 0x14
    /* 5E6F8 8006DEF8 1C000526 */  addiu      $a1, $s0, 0x1C
    /* 5E6FC 8006DEFC FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 5E700 8006DF00 1000A2AF */  sw         $v0, 0x10($sp)
    /* 5E704 8006DF04 2C00A78F */  lw         $a3, 0x2C($sp)
    /* 5E708 8006DF08 02B6010C */  jal        func_8006D808
    /* 5E70C 8006DF0C 21304002 */   addu      $a2, $s2, $zero
    /* 5E710 8006DF10 21200002 */  addu       $a0, $s0, $zero
    /* 5E714 8006DF14 5000A527 */  addiu      $a1, $sp, 0x50
    /* 5E718 8006DF18 11000624 */  addiu      $a2, $zero, 0x11
    /* 5E71C 8006DF1C 96000224 */  addiu      $v0, $zero, 0x96
    /* 5E720 8006DF20 5400A2A7 */  sh         $v0, 0x54($sp)
    /* 5E724 8006DF24 F5000224 */  addiu      $v0, $zero, 0xF5
    /* 5E728 8006DF28 5000A2A7 */  sh         $v0, 0x50($sp)
    /* 5E72C 8006DF2C 25000224 */  addiu      $v0, $zero, 0x25
    /* 5E730 8006DF30 5200A2A7 */  sh         $v0, 0x52($sp)
    /* 5E734 8006DF34 01000224 */  addiu      $v0, $zero, 0x1
    /* 5E738 8006DF38 26A6010C */  jal        func_80069898
    /* 5E73C 8006DF3C 5600A2A7 */   sh        $v0, 0x56($sp)
    /* 5E740 8006DF40 7000BF8F */  lw         $ra, 0x70($sp)
    /* 5E744 8006DF44 6C00B58F */  lw         $s5, 0x6C($sp)
    /* 5E748 8006DF48 6800B48F */  lw         $s4, 0x68($sp)
    /* 5E74C 8006DF4C 6400B38F */  lw         $s3, 0x64($sp)
    /* 5E750 8006DF50 6000B28F */  lw         $s2, 0x60($sp)
    /* 5E754 8006DF54 5C00B18F */  lw         $s1, 0x5C($sp)
    /* 5E758 8006DF58 5800B08F */  lw         $s0, 0x58($sp)
    /* 5E75C 8006DF5C 7800BD27 */  addiu      $sp, $sp, 0x78
    /* 5E760 8006DF60 0800E003 */  jr         $ra
    /* 5E764 8006DF64 00000000 */   nop
endlabel func_8006DD94
