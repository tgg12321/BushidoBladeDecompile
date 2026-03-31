glabel md_game_check_change_main_mode_katinuki
    /* 7B128 8008A928 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 7B12C 8008A92C 1800B0AF */  sw         $s0, 0x18($sp)
    /* 7B130 8008A930 21808000 */  addu       $s0, $a0, $zero
    /* 7B134 8008A934 2C00B5AF */  sw         $s5, 0x2C($sp)
    /* 7B138 8008A938 0A00022E */  sltiu      $v0, $s0, 0xA
    /* 7B13C 8008A93C 3000BFAF */  sw         $ra, 0x30($sp)
    /* 7B140 8008A940 2800B4AF */  sw         $s4, 0x28($sp)
    /* 7B144 8008A944 2400B3AF */  sw         $s3, 0x24($sp)
    /* 7B148 8008A948 2000B2AF */  sw         $s2, 0x20($sp)
    /* 7B14C 8008A94C 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 7B150 8008A950 1000A0AF */  sw         $zero, 0x10($sp)
    /* 7B154 8008A954 0A004010 */  beqz       $v0, .L8008A980
    /* 7B158 8008A958 21A80000 */   addu      $s5, $zero, $zero
    /* 7B15C 8008A95C 0A80033C */  lui        $v1, %hi(D_800A2D44)
    /* 7B160 8008A960 442D6324 */  addiu      $v1, $v1, %lo(D_800A2D44)
    /* 7B164 8008A964 80101000 */  sll        $v0, $s0, 2
    /* 7B168 8008A968 21884300 */  addu       $s1, $v0, $v1
    /* 7B16C 8008A96C 0000248E */  lw         $a0, 0x0($s1)
    /* 7B170 8008A970 AC27020C */  jal        func_80089EB0
    /* 7B174 8008A974 00000000 */   nop
    /* 7B178 8008A978 03004010 */  beqz       $v0, .L8008A988
    /* 7B17C 8008A97C 00000000 */   nop
  .L8008A980:
    /* 7B180 8008A980 A72A0208 */  j          .L8008AA9C
    /* 7B184 8008A984 FFFF0224 */   addiu     $v0, $zero, -0x1
  .L8008A988:
    /* 7B188 8008A988 08000016 */  bnez       $s0, .L8008A9AC
    /* 7B18C 8008A98C 0100023C */   lui       $v0, (0x10000 >> 16)
    /* 7B190 8008A990 0A80023C */  lui        $v0, %hi(D_800A2D04)
    /* 7B194 8008A994 042D428C */  lw         $v0, %lo(D_800A2D04)($v0)
    /* 7B198 8008A998 10000324 */  addiu      $v1, $zero, 0x10
    /* 7B19C 8008A99C 04884300 */  sllv       $s1, $v1, $v0
    /* 7B1A0 8008A9A0 F0FF0334 */  ori        $v1, $zero, 0xFFF0
    /* 7B1A4 8008A9A4 712A0208 */  j          .L8008A9C4
    /* 7B1A8 8008A9A8 04904300 */   sllv      $s2, $v1, $v0
  .L8008A9AC:
    /* 7B1AC 8008A9AC 0000248E */  lw         $a0, 0x0($s1)
    /* 7B1B0 8008A9B0 0A80033C */  lui        $v1, %hi(D_800A2D04)
    /* 7B1B4 8008A9B4 042D638C */  lw         $v1, %lo(D_800A2D04)($v1)
    /* 7B1B8 8008A9B8 23104400 */  subu       $v0, $v0, $a0
    /* 7B1BC 8008A9BC 04886200 */  sllv       $s1, $v0, $v1
    /* 7B1C0 8008A9C0 04906400 */  sllv       $s2, $a0, $v1
  .L8008A9C4:
    /* 7B1C4 8008A9C4 0A80143C */  lui        $s4, %hi(D_800A2CF8)
    /* 7B1C8 8008A9C8 F82C948E */  lw         $s4, %lo(D_800A2CF8)($s4)
    /* 7B1CC 8008A9CC 01000224 */  addiu      $v0, $zero, 0x1
    /* 7B1D0 8008A9D0 04008216 */  bne        $s4, $v0, .L8008A9E4
    /* 7B1D4 8008A9D4 00000000 */   nop
    /* 7B1D8 8008A9D8 0A80013C */  lui        $at, %hi(D_800A2CF8)
    /* 7B1DC 8008A9DC F82C20AC */  sw         $zero, %lo(D_800A2CF8)($at)
    /* 7B1E0 8008A9E0 01001524 */  addiu      $s5, $zero, 0x1
  .L8008A9E4:
    /* 7B1E4 8008A9E4 0A80023C */  lui        $v0, %hi(D_800A2D14)
    /* 7B1E8 8008A9E8 142D428C */  lw         $v0, %lo(D_800A2D14)($v0)
    /* 7B1EC 8008A9EC 00000000 */  nop
    /* 7B1F0 8008A9F0 07004010 */  beqz       $v0, .L8008AA10
    /* 7B1F4 8008A9F4 01001324 */   addiu     $s3, $zero, 0x1
    /* 7B1F8 8008A9F8 0A80023C */  lui        $v0, %hi(D_800A2D14)
    /* 7B1FC 8008A9FC 142D428C */  lw         $v0, %lo(D_800A2D14)($v0)
    /* 7B200 8008AA00 00000000 */  nop
    /* 7B204 8008AA04 1000A2AF */  sw         $v0, 0x10($sp)
    /* 7B208 8008AA08 0A80013C */  lui        $at, %hi(D_800A2D14)
    /* 7B20C 8008AA0C 142D20AC */  sw         $zero, %lo(D_800A2D14)($at)
  .L8008AA10:
    /* 7B210 8008AA10 0104222E */  sltiu      $v0, $s1, 0x401
  .L8008AA14:
    /* 7B214 8008AA14 03004014 */  bnez       $v0, .L8008AA24
    /* 7B218 8008AA18 21802002 */   addu      $s0, $s1, $zero
    /* 7B21C 8008AA1C 8A2A0208 */  j          .L8008AA28
    /* 7B220 8008AA20 00041024 */   addiu     $s0, $zero, 0x400
  .L8008AA24:
    /* 7B224 8008AA24 21980000 */  addu       $s3, $zero, $zero
  .L8008AA28:
    /* 7B228 8008AA28 02000424 */  addiu      $a0, $zero, 0x2
    /* 7B22C 8008AA2C 4323020C */  jal        saTan0GaugeDraw
    /* 7B230 8008AA30 21284002 */   addu      $a1, $s2, $zero
    /* 7B234 8008AA34 4323020C */  jal        saTan0GaugeDraw
    /* 7B238 8008AA38 01000424 */   addiu     $a0, $zero, 0x1
    /* 7B23C 8008AA3C 03000424 */  addiu      $a0, $zero, 0x3
    /* 7B240 8008AA40 0A80053C */  lui        $a1, %hi(D_800A28D4)
    /* 7B244 8008AA44 D428A524 */  addiu      $a1, $a1, %lo(D_800A28D4)
    /* 7B248 8008AA48 4323020C */  jal        saTan0GaugeDraw
    /* 7B24C 8008AA4C 21300002 */   addu      $a2, $s0, $zero
    /* 7B250 8008AA50 0A80043C */  lui        $a0, %hi(D_800A2870)
    /* 7B254 8008AA54 7028848C */  lw         $a0, %lo(D_800A2870)($a0)
    /* 7B258 8008AA58 00FC3126 */  addiu      $s1, $s1, -0x400
    /* 7B25C 8008AA5C B12A020C */  jal        func_8008AAC4
    /* 7B260 8008AA60 00045226 */   addiu     $s2, $s2, 0x400
    /* 7B264 8008AA64 EBFF6016 */  bnez       $s3, .L8008AA14
    /* 7B268 8008AA68 0104222E */   sltiu     $v0, $s1, 0x401
    /* 7B26C 8008AA6C 0300A012 */  beqz       $s5, .L8008AA7C
    /* 7B270 8008AA70 00000000 */   nop
    /* 7B274 8008AA74 0A80013C */  lui        $at, %hi(D_800A2CF8)
    /* 7B278 8008AA78 F82C34AC */  sw         $s4, %lo(D_800A2CF8)($at)
  .L8008AA7C:
    /* 7B27C 8008AA7C 1000A28F */  lw         $v0, 0x10($sp)
    /* 7B280 8008AA80 00000000 */  nop
    /* 7B284 8008AA84 05004010 */  beqz       $v0, .L8008AA9C
    /* 7B288 8008AA88 21100000 */   addu      $v0, $zero, $zero
    /* 7B28C 8008AA8C 1000A28F */  lw         $v0, 0x10($sp)
    /* 7B290 8008AA90 0A80013C */  lui        $at, %hi(D_800A2D14)
    /* 7B294 8008AA94 142D22AC */  sw         $v0, %lo(D_800A2D14)($at)
    /* 7B298 8008AA98 21100000 */  addu       $v0, $zero, $zero
  .L8008AA9C:
    /* 7B29C 8008AA9C 3000BF8F */  lw         $ra, 0x30($sp)
    /* 7B2A0 8008AAA0 2C00B58F */  lw         $s5, 0x2C($sp)
    /* 7B2A4 8008AAA4 2800B48F */  lw         $s4, 0x28($sp)
    /* 7B2A8 8008AAA8 2400B38F */  lw         $s3, 0x24($sp)
    /* 7B2AC 8008AAAC 2000B28F */  lw         $s2, 0x20($sp)
    /* 7B2B0 8008AAB0 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 7B2B4 8008AAB4 1800B08F */  lw         $s0, 0x18($sp)
    /* 7B2B8 8008AAB8 3800BD27 */  addiu      $sp, $sp, 0x38
    /* 7B2BC 8008AABC 0800E003 */  jr         $ra
    /* 7B2C0 8008AAC0 00000000 */   nop
endlabel md_game_check_change_main_mode_katinuki
