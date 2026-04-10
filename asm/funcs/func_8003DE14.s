glabel func_8003DE14
    /* 2E614 8003DE14 A8F7BD27 */  addiu      $sp, $sp, -0x858
    /* 2E618 8003DE18 3008B0AF */  sw         $s0, 0x830($sp)
    /* 2E61C 8003DE1C 21808000 */  addu       $s0, $a0, $zero
    /* 2E620 8003DE20 3808B2AF */  sw         $s2, 0x838($sp)
    /* 2E624 8003DE24 2190A000 */  addu       $s2, $a1, $zero
    /* 2E628 8003DE28 21200000 */  addu       $a0, $zero, $zero
    /* 2E62C 8003DE2C 5408BFAF */  sw         $ra, 0x854($sp)
    /* 2E630 8003DE30 5008BEAF */  sw         $fp, 0x850($sp)
    /* 2E634 8003DE34 4C08B7AF */  sw         $s7, 0x84C($sp)
    /* 2E638 8003DE38 4808B6AF */  sw         $s6, 0x848($sp)
    /* 2E63C 8003DE3C 4408B5AF */  sw         $s5, 0x844($sp)
    /* 2E640 8003DE40 4008B4AF */  sw         $s4, 0x840($sp)
    /* 2E644 8003DE44 3C08B3AF */  sw         $s3, 0x83C($sp)
    /* 2E648 8003DE48 CFEC010C */  jal        gpu_DrawSync
    /* 2E64C 8003DE4C 3408B1AF */   sw        $s1, 0x834($sp)
    /* 2E650 8003DE50 FFFF5226 */  addiu      $s2, $s2, -0x1
    /* 2E654 8003DE54 21200002 */  addu       $a0, $s0, $zero
    /* 2E658 8003DE58 99ED010C */  jal        gpu_StoreImage
    /* 2E65C 8003DE5C 1000A527 */   addiu     $a1, $sp, 0x10
    /* 2E660 8003DE60 CFEC010C */  jal        gpu_DrawSync
    /* 2E664 8003DE64 21200000 */   addu      $a0, $zero, $zero
    /* 2E668 8003DE68 21200002 */  addu       $a0, $s0, $zero
    /* 2E66C 8003DE6C 1000A527 */  addiu      $a1, $sp, 0x10
    /* 2E670 8003DE70 02000296 */  lhu        $v0, 0x2($s0)
    /* 2E674 8003DE74 06000396 */  lhu        $v1, 0x6($s0)
    /* 2E678 8003DE78 21880000 */  addu       $s1, $zero, $zero
    /* 2E67C 8003DE7C 23104300 */  subu       $v0, $v0, $v1
    /* 2E680 8003DE80 80ED010C */  jal        gpu_LoadImage
    /* 2E684 8003DE84 020002A6 */   sh        $v0, 0x2($s0)
    /* 2E688 8003DE88 02001786 */  lh         $s7, 0x2($s0)
    /* 2E68C 8003DE8C 06000296 */  lhu        $v0, 0x6($s0)
    /* 2E690 8003DE90 1008A427 */  addiu      $a0, $sp, 0x810
    /* 2E694 8003DE94 21105700 */  addu       $v0, $v0, $s7
    /* 2E698 8003DE98 F94A010C */  jal        func_80052BE4
    /* 2E69C 8003DE9C 020002A6 */   sh        $v0, 0x2($s0)
    /* 2E6A0 8003DEA0 00800324 */  addiu      $v1, $zero, -0x8000
    /* 2E6A4 8003DEA4 1008B593 */  lbu        $s5, 0x810($sp)
    /* 2E6A8 8003DEA8 1108B493 */  lbu        $s4, 0x811($sp)
    /* 2E6AC 8003DEAC 1208B393 */  lbu        $s3, 0x812($sp)
    /* 2E6B0 8003DEB0 C2201500 */  srl        $a0, $s5, 3
    /* 2E6B4 8003DEB4 F8008232 */  andi       $v0, $s4, 0xF8
    /* 2E6B8 8003DEB8 80100200 */  sll        $v0, $v0, 2
    /* 2E6BC 8003DEBC 25104300 */  or         $v0, $v0, $v1
    /* 2E6C0 8003DEC0 25208200 */  or         $a0, $a0, $v0
    /* 2E6C4 8003DEC4 F8006232 */  andi       $v0, $s3, 0xF8
    /* 2E6C8 8003DEC8 C0110200 */  sll        $v0, $v0, 7
    /* 2E6CC 8003DECC 7700401A */  blez       $s2, .L8003E0AC
    /* 2E6D0 8003DED0 25B08200 */   or        $s6, $a0, $v0
    /* 2E6D4 8003DED4 00101E24 */  addiu      $fp, $zero, 0x1000
  .L8003DED8:
    /* 2E6D8 8003DED8 04000386 */  lh         $v1, 0x4($s0)
    /* 2E6DC 8003DEDC 06000286 */  lh         $v0, 0x6($s0)
    /* 2E6E0 8003DEE0 00000000 */  nop
    /* 2E6E4 8003DEE4 18006200 */  mult       $v1, $v0
    /* 2E6E8 8003DEE8 1000A727 */  addiu      $a3, $sp, 0x10
    /* 2E6EC 8003DEEC 1004A627 */  addiu      $a2, $sp, 0x410
    /* 2E6F0 8003DEF0 12180000 */  mflo       $v1
    /* 2E6F4 8003DEF4 01002226 */  addiu      $v0, $s1, 0x1
    /* 2E6F8 8003DEF8 00130200 */  sll        $v0, $v0, 12
    /* 2E6FC 8003DEFC 1A005200 */  div        $zero, $v0, $s2
    /* 2E700 8003DF00 02004016 */  bnez       $s2, .L8003DF0C
    /* 2E704 8003DF04 00000000 */   nop
    /* 2E708 8003DF08 0D000700 */  break      7
  .L8003DF0C:
    /* 2E70C 8003DF0C FFFF0124 */  addiu      $at, $zero, -0x1
    /* 2E710 8003DF10 04004116 */  bne        $s2, $at, .L8003DF24
    /* 2E714 8003DF14 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 2E718 8003DF18 02004114 */  bne        $v0, $at, .L8003DF24
    /* 2E71C 8003DF1C 00000000 */   nop
    /* 2E720 8003DF20 0D000600 */  break      6
  .L8003DF24:
    /* 2E724 8003DF24 12580000 */  mflo       $t3
    /* 2E728 8003DF28 48006018 */  blez       $v1, .L8003E04C
    /* 2E72C 8003DF2C 21600000 */   addu      $t4, $zero, $zero
    /* 2E730 8003DF30 2368CB03 */  subu       $t5, $fp, $t3
    /* 2E734 8003DF34 FFFF4226 */  addiu      $v0, $s2, -0x1
  .L8003DF38:
    /* 2E738 8003DF38 0C002216 */  bne        $s1, $v0, .L8003DF6C
    /* 2E73C 8003DF3C 00000000 */   nop
    /* 2E740 8003DF40 0000E294 */  lhu        $v0, 0x0($a3)
    /* 2E744 8003DF44 00000000 */  nop
    /* 2E748 8003DF48 04004014 */  bnez       $v0, .L8003DF5C
    /* 2E74C 8003DF4C 00000000 */   nop
    /* 2E750 8003DF50 0000C2A4 */  sh         $v0, 0x0($a2)
    /* 2E754 8003DF54 09F80008 */  j          .L8003E024
    /* 2E758 8003DF58 0200E724 */   addiu     $a3, $a3, 0x2
  .L8003DF5C:
    /* 2E75C 8003DF5C 0000D6A4 */  sh         $s6, 0x0($a2)
    /* 2E760 8003DF60 0200C624 */  addiu      $a2, $a2, 0x2
    /* 2E764 8003DF64 0AF80008 */  j          .L8003E028
    /* 2E768 8003DF68 0200E724 */   addiu     $a3, $a3, 0x2
  .L8003DF6C:
    /* 2E76C 8003DF6C 0000E894 */  lhu        $t0, 0x0($a3)
    /* 2E770 8003DF70 00000000 */  nop
    /* 2E774 8003DF74 FFFF0431 */  andi       $a0, $t0, 0xFFFF
    /* 2E778 8003DF78 04008014 */  bnez       $a0, .L8003DF8C
    /* 2E77C 8003DF7C 1F000231 */   andi      $v0, $t0, 0x1F
    /* 2E780 8003DF80 0000C8A4 */  sh         $t0, 0x0($a2)
    /* 2E784 8003DF84 09F80008 */  j          .L8003E024
    /* 2E788 8003DF88 0200E724 */   addiu     $a3, $a3, 0x2
  .L8003DF8C:
    /* 2E78C 8003DF8C C0280200 */  sll        $a1, $v0, 3
    /* 2E790 8003DF90 1800AD00 */  mult       $a1, $t5
    /* 2E794 8003DF94 12500000 */  mflo       $t2
    /* 2E798 8003DF98 00000000 */  nop
    /* 2E79C 8003DF9C 00000000 */  nop
    /* 2E7A0 8003DFA0 1800AB02 */  mult       $s5, $t3
    /* 2E7A4 8003DFA4 12280000 */  mflo       $a1
    /* 2E7A8 8003DFA8 82100400 */  srl        $v0, $a0, 2
    /* 2E7AC 8003DFAC F8004330 */  andi       $v1, $v0, 0xF8
    /* 2E7B0 8003DFB0 18006D00 */  mult       $v1, $t5
    /* 2E7B4 8003DFB4 12480000 */  mflo       $t1
    /* 2E7B8 8003DFB8 00000000 */  nop
    /* 2E7BC 8003DFBC 00000000 */  nop
    /* 2E7C0 8003DFC0 18008B02 */  mult       $s4, $t3
    /* 2E7C4 8003DFC4 12180000 */  mflo       $v1
    /* 2E7C8 8003DFC8 C2110400 */  srl        $v0, $a0, 7
    /* 2E7CC 8003DFCC F8004430 */  andi       $a0, $v0, 0xF8
    /* 2E7D0 8003DFD0 18008D00 */  mult       $a0, $t5
    /* 2E7D4 8003DFD4 12200000 */  mflo       $a0
    /* 2E7D8 8003DFD8 00000000 */  nop
    /* 2E7DC 8003DFDC 00000000 */  nop
    /* 2E7E0 8003DFE0 18006B02 */  mult       $s3, $t3
    /* 2E7E4 8003DFE4 0200E724 */  addiu      $a3, $a3, 0x2
    /* 2E7E8 8003DFE8 21104501 */  addu       $v0, $t2, $a1
    /* 2E7EC 8003DFEC C32B0200 */  sra        $a1, $v0, 15
    /* 2E7F0 8003DFF0 1F00A530 */  andi       $a1, $a1, 0x1F
    /* 2E7F4 8003DFF4 21102301 */  addu       $v0, $t1, $v1
    /* 2E7F8 8003DFF8 831A0200 */  sra        $v1, $v0, 10
    /* 2E7FC 8003DFFC E0036330 */  andi       $v1, $v1, 0x3E0
    /* 2E800 8003E000 12780000 */  mflo       $t7
    /* 2E804 8003E004 21108F00 */  addu       $v0, $a0, $t7
    /* 2E808 8003E008 43210200 */  sra        $a0, $v0, 5
    /* 2E80C 8003E00C 00800231 */  andi       $v0, $t0, 0x8000
    /* 2E810 8003E010 25104500 */  or         $v0, $v0, $a1
    /* 2E814 8003E014 25104300 */  or         $v0, $v0, $v1
    /* 2E818 8003E018 007C8330 */  andi       $v1, $a0, 0x7C00
    /* 2E81C 8003E01C 25104300 */  or         $v0, $v0, $v1
    /* 2E820 8003E020 0000C2A4 */  sh         $v0, 0x0($a2)
  .L8003E024:
    /* 2E824 8003E024 0200C624 */  addiu      $a2, $a2, 0x2
  .L8003E028:
    /* 2E828 8003E028 04000286 */  lh         $v0, 0x4($s0)
    /* 2E82C 8003E02C 06000386 */  lh         $v1, 0x6($s0)
    /* 2E830 8003E030 00000000 */  nop
    /* 2E834 8003E034 18004300 */  mult       $v0, $v1
    /* 2E838 8003E038 01008C25 */  addiu      $t4, $t4, 0x1
    /* 2E83C 8003E03C 12700000 */  mflo       $t6
    /* 2E840 8003E040 2A108E01 */  slt        $v0, $t4, $t6
    /* 2E844 8003E044 BCFF4014 */  bnez       $v0, .L8003DF38
    /* 2E848 8003E048 FFFF4226 */   addiu     $v0, $s2, -0x1
  .L8003E04C:
    /* 2E84C 8003E04C 02000296 */  lhu        $v0, 0x2($s0)
    /* 2E850 8003E050 06000396 */  lhu        $v1, 0x6($s0)
    /* 2E854 8003E054 00000000 */  nop
    /* 2E858 8003E058 21104300 */  addu       $v0, $v0, $v1
    /* 2E85C 8003E05C 020002A6 */  sh         $v0, 0x2($s0)
    /* 2E860 8003E060 00140200 */  sll        $v0, $v0, 16
    /* 2E864 8003E064 03140200 */  sra        $v0, $v0, 16
    /* 2E868 8003E068 00024228 */  slti       $v0, $v0, 0x200
    /* 2E86C 8003E06C 06004014 */  bnez       $v0, .L8003E088
    /* 2E870 8003E070 00000000 */   nop
    /* 2E874 8003E074 00000296 */  lhu        $v0, 0x0($s0)
    /* 2E878 8003E078 04000396 */  lhu        $v1, 0x4($s0)
    /* 2E87C 8003E07C 020017A6 */  sh         $s7, 0x2($s0)
    /* 2E880 8003E080 21104300 */  addu       $v0, $v0, $v1
    /* 2E884 8003E084 000002A6 */  sh         $v0, 0x0($s0)
  .L8003E088:
    /* 2E888 8003E088 21200002 */  addu       $a0, $s0, $zero
    /* 2E88C 8003E08C 80ED010C */  jal        gpu_LoadImage
    /* 2E890 8003E090 1004A527 */   addiu     $a1, $sp, 0x410
    /* 2E894 8003E094 CFEC010C */  jal        gpu_DrawSync
    /* 2E898 8003E098 21200000 */   addu      $a0, $zero, $zero
    /* 2E89C 8003E09C 01003126 */  addiu      $s1, $s1, 0x1
    /* 2E8A0 8003E0A0 2A103202 */  slt        $v0, $s1, $s2
    /* 2E8A4 8003E0A4 8CFF4014 */  bnez       $v0, .L8003DED8
    /* 2E8A8 8003E0A8 00000000 */   nop
  .L8003E0AC:
    /* 2E8AC 8003E0AC 5408BF8F */  lw         $ra, 0x854($sp)
    /* 2E8B0 8003E0B0 5008BE8F */  lw         $fp, 0x850($sp)
    /* 2E8B4 8003E0B4 4C08B78F */  lw         $s7, 0x84C($sp)
    /* 2E8B8 8003E0B8 4808B68F */  lw         $s6, 0x848($sp)
    /* 2E8BC 8003E0BC 4408B58F */  lw         $s5, 0x844($sp)
    /* 2E8C0 8003E0C0 4008B48F */  lw         $s4, 0x840($sp)
    /* 2E8C4 8003E0C4 3C08B38F */  lw         $s3, 0x83C($sp)
    /* 2E8C8 8003E0C8 3808B28F */  lw         $s2, 0x838($sp)
    /* 2E8CC 8003E0CC 3408B18F */  lw         $s1, 0x834($sp)
    /* 2E8D0 8003E0D0 3008B08F */  lw         $s0, 0x830($sp)
    /* 2E8D4 8003E0D4 5808BD27 */  addiu      $sp, $sp, 0x858
    /* 2E8D8 8003E0D8 0800E003 */  jr         $ra
    /* 2E8DC 8003E0DC 00000000 */   nop
endlabel func_8003DE14
