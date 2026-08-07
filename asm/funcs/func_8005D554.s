glabel func_8005D554
    /* 4DD54 8005D554 A001838F */  lw         $v1, %gp_rel(D_800A326C)($gp)
    /* 4DD58 8005D558 88FFBD27 */  addiu      $sp, $sp, -0x78
    /* 4DD5C 8005D55C 5000B0AF */  sw         $s0, 0x50($sp)
    /* 4DD60 8005D560 2180A000 */  addu       $s0, $a1, $zero
    /* 4DD64 8005D564 7400BFAF */  sw         $ra, 0x74($sp)
    /* 4DD68 8005D568 7000BEAF */  sw         $fp, 0x70($sp)
    /* 4DD6C 8005D56C 6C00B7AF */  sw         $s7, 0x6C($sp)
    /* 4DD70 8005D570 6800B6AF */  sw         $s6, 0x68($sp)
    /* 4DD74 8005D574 6400B5AF */  sw         $s5, 0x64($sp)
    /* 4DD78 8005D578 6000B4AF */  sw         $s4, 0x60($sp)
    /* 4DD7C 8005D57C 5C00B3AF */  sw         $s3, 0x5C($sp)
    /* 4DD80 8005D580 5800B2AF */  sw         $s2, 0x58($sp)
    /* 4DD84 8005D584 5400B1AF */  sw         $s1, 0x54($sp)
    /* 4DD88 8005D588 02006104 */  bgez       $v1, .L8005D594
    /* 4DD8C 8005D58C 21106000 */   addu      $v0, $v1, $zero
    /* 4DD90 8005D590 03006224 */  addiu      $v0, $v1, 0x3
  .L8005D594:
    /* 4DD94 8005D594 83100200 */  sra        $v0, $v0, 2
    /* 4DD98 8005D598 80100200 */  sll        $v0, $v0, 2
    /* 4DD9C 8005D59C 23106200 */  subu       $v0, $v1, $v0
    /* 4DDA0 8005D5A0 A00182AF */  sw         $v0, %gp_rel(D_800A326C)($gp)
    /* 4DDA4 8005D5A4 0200001A */  blez       $s0, .L8005D5B0
    /* 4DDA8 8005D5A8 21888000 */   addu      $s1, $a0, $zero
    /* 4DDAC 8005D5AC FFFF1026 */  addiu      $s0, $s0, -0x1
  .L8005D5B0:
    /* 4DDB0 8005D5B0 55E4010C */  jal        func_80079154
    /* 4DDB4 8005D5B4 21900000 */   addu      $s2, $zero, $zero
    /* 4DDB8 8005D5B8 4C03838F */  lw         $v1, %gp_rel(D_800A3418)($gp)
    /* 4DDBC 8005D5BC 00000000 */  nop
    /* 4DDC0 8005D5C0 26186200 */  xor        $v1, $v1, $v0
    /* 4DDC4 8005D5C4 80100300 */  sll        $v0, $v1, 2
    /* 4DDC8 8005D5C8 21104300 */  addu       $v0, $v0, $v1
    /* 4DDCC 8005D5CC 80100200 */  sll        $v0, $v0, 2
    /* 4DDD0 8005D5D0 23104300 */  subu       $v0, $v0, $v1
    /* 4DDD4 8005D5D4 40110200 */  sll        $v0, $v0, 5
    /* 4DDD8 8005D5D8 4C0383AF */  sw         $v1, %gp_rel(D_800A3418)($gp)
    /* 4DDDC 8005D5DC 55E4010C */  jal        func_80079154
    /* 4DDE0 8005D5E0 C2AB0200 */   srl       $s5, $v0, 15
    /* 4DDE4 8005D5E4 4C03838F */  lw         $v1, %gp_rel(D_800A3418)($gp)
    /* 4DDE8 8005D5E8 00000000 */  nop
    /* 4DDEC 8005D5EC 26186200 */  xor        $v1, $v1, $v0
    /* 4DDF0 8005D5F0 C0100300 */  sll        $v0, $v1, 3
    /* 4DDF4 8005D5F4 23104300 */  subu       $v0, $v0, $v1
    /* 4DDF8 8005D5F8 C0100200 */  sll        $v0, $v0, 3
    /* 4DDFC 8005D5FC 23104300 */  subu       $v0, $v0, $v1
    /* 4DE00 8005D600 80100200 */  sll        $v0, $v0, 2
    /* 4DE04 8005D604 4C0383AF */  sw         $v1, %gp_rel(D_800A3418)($gp)
    /* 4DE08 8005D608 A001838F */  lw         $v1, %gp_rel(D_800A326C)($gp)
    /* 4DE0C 8005D60C 00000000 */  nop
    /* 4DE10 8005D610 01006324 */  addiu      $v1, $v1, 0x1
    /* 4DE14 8005D614 40180300 */  sll        $v1, $v1, 1
    /* 4DE18 8005D618 6D006018 */  blez       $v1, .L8005D7D0
    /* 4DE1C 8005D61C C2A30200 */   srl       $s4, $v0, 15
    /* 4DE20 8005D620 00011324 */  addiu      $s3, $zero, 0x100
    /* 4DE24 8005D624 01001624 */  addiu      $s6, $zero, 0x1
    /* 4DE28 8005D628 00111000 */  sll        $v0, $s0, 4
    /* 4DE2C 8005D62C 23105000 */  subu       $v0, $v0, $s0
    /* 4DE30 8005D630 80800200 */  sll        $s0, $v0, 2
    /* 4DE34 8005D634 0A801E3C */  lui        $fp, %hi(D_8009B2E0)
    /* 4DE38 8005D638 E0B2DE27 */  addiu      $fp, $fp, %lo(D_8009B2E0)
    /* 4DE3C 8005D63C 0C00C227 */  addiu      $v0, $fp, 0xC
    /* 4DE40 8005D640 21100202 */  addu       $v0, $s0, $v0
    /* 4DE44 8005D644 4000A2AF */  sw         $v0, 0x40($sp)
    /* 4DE48 8005D648 0A80173C */  lui        $s7, %hi(D_8009B388)
    /* 4DE4C 8005D64C 88B3F726 */  addiu      $s7, $s7, %lo(D_8009B388)
  .L8005D650:
    /* 4DE50 8005D650 21101E02 */  addu       $v0, $s0, $fp
    /* 4DE54 8005D654 3800A0A3 */  sb         $zero, 0x38($sp)
    /* 4DE58 8005D658 1000A2AF */  sw         $v0, 0x10($sp)
    /* 4DE5C 8005D65C 55E4010C */  jal        func_80079154
    /* 4DE60 8005D660 1400B7AF */   sw        $s7, 0x14($sp)
    /* 4DE64 8005D664 4C03838F */  lw         $v1, %gp_rel(D_800A3418)($gp)
    /* 4DE68 8005D668 3400B3AF */  sw         $s3, 0x34($sp)
    /* 4DE6C 8005D66C 3000B3AF */  sw         $s3, 0x30($sp)
    /* 4DE70 8005D670 26186200 */  xor        $v1, $v1, $v0
    /* 4DE74 8005D674 4C0383AF */  sw         $v1, %gp_rel(D_800A3418)($gp)
    /* 4DE78 8005D678 55E4010C */  jal        func_80079154
    /* 4DE7C 8005D67C 01005226 */   addiu     $s2, $s2, 0x1
    /* 4DE80 8005D680 4C03838F */  lw         $v1, %gp_rel(D_800A3418)($gp)
    /* 4DE84 8005D684 E7FFA426 */  addiu      $a0, $s5, -0x19
    /* 4DE88 8005D688 26186200 */  xor        $v1, $v1, $v0
    /* 4DE8C 8005D68C 40100300 */  sll        $v0, $v1, 1
    /* 4DE90 8005D690 21104300 */  addu       $v0, $v0, $v1
    /* 4DE94 8005D694 C0100200 */  sll        $v0, $v0, 3
    /* 4DE98 8005D698 21104300 */  addu       $v0, $v0, $v1
    /* 4DE9C 8005D69C 40100200 */  sll        $v0, $v0, 1
    /* 4DEA0 8005D6A0 C2130200 */  srl        $v0, $v0, 15
    /* 4DEA4 8005D6A4 21208200 */  addu       $a0, $a0, $v0
    /* 4DEA8 8005D6A8 4C0383AF */  sw         $v1, %gp_rel(D_800A3418)($gp)
    /* 4DEAC 8005D6AC 55E4010C */  jal        func_80079154
    /* 4DEB0 8005D6B0 2800A4AF */   sw        $a0, 0x28($sp)
    /* 4DEB4 8005D6B4 1000A427 */  addiu      $a0, $sp, 0x10
    /* 4DEB8 8005D6B8 21280000 */  addu       $a1, $zero, $zero
    /* 4DEBC 8005D6BC 4C03838F */  lw         $v1, %gp_rel(D_800A3418)($gp)
    /* 4DEC0 8005D6C0 F4FF8626 */  addiu      $a2, $s4, -0xC
    /* 4DEC4 8005D6C4 2000A0AF */  sw         $zero, 0x20($sp)
    /* 4DEC8 8005D6C8 2400B6AF */  sw         $s6, 0x24($sp)
    /* 4DECC 8005D6CC 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 4DED0 8005D6D0 26186200 */  xor        $v1, $v1, $v0
    /* 4DED4 8005D6D4 40100300 */  sll        $v0, $v1, 1
    /* 4DED8 8005D6D8 21104300 */  addu       $v0, $v0, $v1
    /* 4DEDC 8005D6DC C0100200 */  sll        $v0, $v0, 3
    /* 4DEE0 8005D6E0 21104300 */  addu       $v0, $v0, $v1
    /* 4DEE4 8005D6E4 C2130200 */  srl        $v0, $v0, 15
    /* 4DEE8 8005D6E8 2130C200 */  addu       $a2, $a2, $v0
    /* 4DEEC 8005D6EC 4C0383AF */  sw         $v1, %gp_rel(D_800A3418)($gp)
    /* 4DEF0 8005D6F0 CACD010C */  jal        func_80073728
    /* 4DEF4 8005D6F4 2C00A6AF */   sw        $a2, 0x2C($sp)
    /* 4DEF8 8005D6F8 4C03838F */  lw         $v1, %gp_rel(D_800A3418)($gp)
    /* 4DEFC 8005D6FC 0A80073C */  lui        $a3, %hi(D_8009B390)
    /* 4DF00 8005D700 90B3E724 */  addiu      $a3, $a3, %lo(D_8009B390)
    /* 4DF04 8005D704 1400A7AF */  sw         $a3, 0x14($sp)
    /* 4DF08 8005D708 4000A78F */  lw         $a3, 0x40($sp)
    /* 4DF0C 8005D70C 21884000 */  addu       $s1, $v0, $zero
    /* 4DF10 8005D710 3800A0A3 */  sb         $zero, 0x38($sp)
    /* 4DF14 8005D714 3400B3AF */  sw         $s3, 0x34($sp)
    /* 4DF18 8005D718 3000B3AF */  sw         $s3, 0x30($sp)
    /* 4DF1C 8005D71C 01006330 */  andi       $v1, $v1, 0x1
    /* 4DF20 8005D720 40100300 */  sll        $v0, $v1, 1
    /* 4DF24 8005D724 21104300 */  addu       $v0, $v0, $v1
    /* 4DF28 8005D728 80100200 */  sll        $v0, $v0, 2
    /* 4DF2C 8005D72C 2110E200 */  addu       $v0, $a3, $v0
    /* 4DF30 8005D730 55E4010C */  jal        func_80079154
    /* 4DF34 8005D734 1000A2AF */   sw        $v0, 0x10($sp)
    /* 4DF38 8005D738 4C03838F */  lw         $v1, %gp_rel(D_800A3418)($gp)
    /* 4DF3C 8005D73C CEFFA426 */  addiu      $a0, $s5, -0x32
    /* 4DF40 8005D740 26186200 */  xor        $v1, $v1, $v0
    /* 4DF44 8005D744 40100300 */  sll        $v0, $v1, 1
    /* 4DF48 8005D748 21104300 */  addu       $v0, $v0, $v1
    /* 4DF4C 8005D74C C0100200 */  sll        $v0, $v0, 3
    /* 4DF50 8005D750 21104300 */  addu       $v0, $v0, $v1
    /* 4DF54 8005D754 80100200 */  sll        $v0, $v0, 2
    /* 4DF58 8005D758 C2130200 */  srl        $v0, $v0, 15
    /* 4DF5C 8005D75C 21208200 */  addu       $a0, $a0, $v0
    /* 4DF60 8005D760 4C0383AF */  sw         $v1, %gp_rel(D_800A3418)($gp)
    /* 4DF64 8005D764 55E4010C */  jal        func_80079154
    /* 4DF68 8005D768 2800A4AF */   sw        $a0, 0x28($sp)
    /* 4DF6C 8005D76C 1000A427 */  addiu      $a0, $sp, 0x10
    /* 4DF70 8005D770 21280000 */  addu       $a1, $zero, $zero
    /* 4DF74 8005D774 4C03838F */  lw         $v1, %gp_rel(D_800A3418)($gp)
    /* 4DF78 8005D778 E7FF8626 */  addiu      $a2, $s4, -0x19
    /* 4DF7C 8005D77C 2000A0AF */  sw         $zero, 0x20($sp)
    /* 4DF80 8005D780 2400B6AF */  sw         $s6, 0x24($sp)
    /* 4DF84 8005D784 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 4DF88 8005D788 26186200 */  xor        $v1, $v1, $v0
    /* 4DF8C 8005D78C 40100300 */  sll        $v0, $v1, 1
    /* 4DF90 8005D790 21104300 */  addu       $v0, $v0, $v1
    /* 4DF94 8005D794 C0100200 */  sll        $v0, $v0, 3
    /* 4DF98 8005D798 21104300 */  addu       $v0, $v0, $v1
    /* 4DF9C 8005D79C 40100200 */  sll        $v0, $v0, 1
    /* 4DFA0 8005D7A0 C2130200 */  srl        $v0, $v0, 15
    /* 4DFA4 8005D7A4 2130C200 */  addu       $a2, $a2, $v0
    /* 4DFA8 8005D7A8 4C0383AF */  sw         $v1, %gp_rel(D_800A3418)($gp)
    /* 4DFAC 8005D7AC CACD010C */  jal        func_80073728
    /* 4DFB0 8005D7B0 2C00A6AF */   sw        $a2, 0x2C($sp)
    /* 4DFB4 8005D7B4 A001838F */  lw         $v1, %gp_rel(D_800A326C)($gp)
    /* 4DFB8 8005D7B8 00000000 */  nop
    /* 4DFBC 8005D7BC 01006324 */  addiu      $v1, $v1, 0x1
    /* 4DFC0 8005D7C0 40180300 */  sll        $v1, $v1, 1
    /* 4DFC4 8005D7C4 2A184302 */  slt        $v1, $s2, $v1
    /* 4DFC8 8005D7C8 A1FF6014 */  bnez       $v1, .L8005D650
    /* 4DFCC 8005D7CC 21884000 */   addu      $s1, $v0, $zero
  .L8005D7D0:
    /* 4DFD0 8005D7D0 A001838F */  lw         $v1, %gp_rel(D_800A326C)($gp)
    /* 4DFD4 8005D7D4 21102002 */  addu       $v0, $s1, $zero
    /* 4DFD8 8005D7D8 01006324 */  addiu      $v1, $v1, 0x1
    /* 4DFDC 8005D7DC A00183AF */  sw         $v1, %gp_rel(D_800A326C)($gp)
    /* 4DFE0 8005D7E0 7400BF8F */  lw         $ra, 0x74($sp)
    /* 4DFE4 8005D7E4 7000BE8F */  lw         $fp, 0x70($sp)
    /* 4DFE8 8005D7E8 6C00B78F */  lw         $s7, 0x6C($sp)
    /* 4DFEC 8005D7EC 6800B68F */  lw         $s6, 0x68($sp)
    /* 4DFF0 8005D7F0 6400B58F */  lw         $s5, 0x64($sp)
    /* 4DFF4 8005D7F4 6000B48F */  lw         $s4, 0x60($sp)
    /* 4DFF8 8005D7F8 5C00B38F */  lw         $s3, 0x5C($sp)
    /* 4DFFC 8005D7FC 5800B28F */  lw         $s2, 0x58($sp)
    /* 4E000 8005D800 5400B18F */  lw         $s1, 0x54($sp)
    /* 4E004 8005D804 5000B08F */  lw         $s0, 0x50($sp)
    /* 4E008 8005D808 7800BD27 */  addiu      $sp, $sp, 0x78
    /* 4E00C 8005D80C 0800E003 */  jr         $ra
    /* 4E010 8005D810 00000000 */   nop
endlabel func_8005D554
