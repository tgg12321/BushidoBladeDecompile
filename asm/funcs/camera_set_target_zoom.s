glabel camera_set_target_zoom
    /* D660 8001CE60 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* D664 8001CE64 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* D668 8001CE68 DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* D66C 8001CE6C 01000424 */  addiu      $a0, $zero, 0x1
    /* D670 8001CE70 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* D674 8001CE74 09006414 */  bne        $v1, $a0, .L8001CE9C
    /* D678 8001CE78 1800B0AF */   sw        $s0, 0x18($sp)
    /* D67C 8001CE7C 0A80043C */  lui        $a0, %hi(D_800A3783)
    /* D680 8001CE80 83378490 */  lbu        $a0, %lo(D_800A3783)($a0)
    /* D684 8001CE84 0A80053C */  lui        $a1, %hi(D_800A38B4)
    /* D688 8001CE88 B438A58C */  lw         $a1, %lo(D_800A38B4)($a1)
    /* D68C 8001CE8C 4779010C */  jal        func_8005E51C
    /* D690 8001CE90 01000624 */   addiu     $a2, $zero, 0x1
    /* D694 8001CE94 D5750008 */  j          .L8001D754
    /* D698 8001CE98 00000000 */   nop
  .L8001CE9C:
    /* D69C 8001CE9C 03000224 */  addiu      $v0, $zero, 0x3
    /* D6A0 8001CEA0 27006214 */  bne        $v1, $v0, .L8001CF40
    /* D6A4 8001CEA4 02001024 */   addiu     $s0, $zero, 0x2
    /* D6A8 8001CEA8 1080023C */  lui        $v0, %hi(D_80101F5E)
    /* D6AC 8001CEAC 5E1F4284 */  lh         $v0, %lo(D_80101F5E)($v0)
    /* D6B0 8001CEB0 00000000 */  nop
    /* D6B4 8001CEB4 17004014 */  bnez       $v0, .L8001CF14
    /* D6B8 8001CEB8 00000000 */   nop
    /* D6BC 8001CEBC 1080023C */  lui        $v0, %hi(D_801023AA)
    /* D6C0 8001CEC0 AA234284 */  lh         $v0, %lo(D_801023AA)($v0)
    /* D6C4 8001CEC4 00000000 */  nop
    /* D6C8 8001CEC8 06004010 */  beqz       $v0, .L8001CEE4
    /* D6CC 8001CECC 64000224 */   addiu     $v0, $zero, 0x64
    /* D6D0 8001CED0 0A80033C */  lui        $v1, %hi(D_800A38E2)
    /* D6D4 8001CED4 E2386390 */  lbu        $v1, %lo(D_800A38E2)($v1)
    /* D6D8 8001CED8 00000000 */  nop
    /* D6DC 8001CEDC 0D006210 */  beq        $v1, $v0, .L8001CF14
    /* D6E0 8001CEE0 00000000 */   nop
  .L8001CEE4:
    /* D6E4 8001CEE4 0200033C */  lui        $v1, (0x2BF20 >> 16)
    /* D6E8 8001CEE8 0A80023C */  lui        $v0, %hi(D_800A3858)
    /* D6EC 8001CEEC 5838428C */  lw         $v0, %lo(D_800A3858)($v0)
    /* D6F0 8001CEF0 20BF6334 */  ori        $v1, $v1, (0x2BF20 & 0xFFFF)
    /* D6F4 8001CEF4 01004224 */  addiu      $v0, $v0, 0x1
    /* D6F8 8001CEF8 0A80013C */  lui        $at, %hi(D_800A3858)
    /* D6FC 8001CEFC 583822AC */  sw         $v0, %lo(D_800A3858)($at)
    /* D700 8001CF00 2A106200 */  slt        $v0, $v1, $v0
    /* D704 8001CF04 03004010 */  beqz       $v0, .L8001CF14
    /* D708 8001CF08 00000000 */   nop
    /* D70C 8001CF0C 0A80013C */  lui        $at, %hi(D_800A3858)
    /* D710 8001CF10 583823AC */  sw         $v1, %lo(D_800A3858)($at)
  .L8001CF14:
    /* D714 8001CF14 5A73000C */  jal        func_8001CD68
    /* D718 8001CF18 1000A427 */   addiu     $a0, $sp, 0x10
    /* D71C 8001CF1C 1000A427 */  addiu      $a0, $sp, 0x10
    /* D720 8001CF20 0A80053C */  lui        $a1, %hi(D_800A38E2)
    /* D724 8001CF24 E238A590 */  lbu        $a1, %lo(D_800A38E2)($a1)
    /* D728 8001CF28 0A80063C */  lui        $a2, %hi(D_800A38B4)
    /* D72C 8001CF2C B438C68C */  lw         $a2, %lo(D_800A38B4)($a2)
    /* D730 8001CF30 0576010C */  jal        func_8005D814
    /* D734 8001CF34 01000724 */   addiu     $a3, $zero, 0x1
    /* D738 8001CF38 D5750008 */  j          .L8001D754
    /* D73C 8001CF3C 00000000 */   nop
  .L8001CF40:
    /* D740 8001CF40 06007014 */  bne        $v1, $s0, .L8001CF5C
    /* D744 8001CF44 04000224 */   addiu     $v0, $zero, 0x4
    /* D748 8001CF48 0A80023C */  lui        $v0, %hi(D_800A389A)
    /* D74C 8001CF4C 9A384290 */  lbu        $v0, %lo(D_800A389A)($v0)
    /* D750 8001CF50 00000000 */  nop
    /* D754 8001CF54 03004410 */  beq        $v0, $a0, .L8001CF64
    /* D758 8001CF58 04000224 */   addiu     $v0, $zero, 0x4
  .L8001CF5C:
    /* D75C 8001CF5C 0B006214 */  bne        $v1, $v0, .L8001CF8C
    /* D760 8001CF60 05000224 */   addiu     $v0, $zero, 0x5
  .L8001CF64:
    /* D764 8001CF64 0A80043C */  lui        $a0, %hi(D_800A37D2)
    /* D768 8001CF68 D2378490 */  lbu        $a0, %lo(D_800A37D2)($a0)
    /* D76C 8001CF6C 0A80053C */  lui        $a1, %hi(D_800A37D3)
    /* D770 8001CF70 D337A590 */  lbu        $a1, %lo(D_800A37D3)($a1)
    /* D774 8001CF74 0A80063C */  lui        $a2, %hi(D_800A38B4)
    /* D778 8001CF78 B438C68C */  lw         $a2, %lo(D_800A38B4)($a2)
    /* D77C 8001CF7C 2678010C */  jal        func_8005E098
    /* D780 8001CF80 01000724 */   addiu     $a3, $zero, 0x1
    /* D784 8001CF84 D5750008 */  j          .L8001D754
    /* D788 8001CF88 00000000 */   nop
  .L8001CF8C:
    /* D78C 8001CF8C FB016214 */  bne        $v1, $v0, .L8001D77C
    /* D790 8001CF90 00000000 */   nop
    /* D794 8001CF94 0A80033C */  lui        $v1, %hi(D_800A381E)
    /* D798 8001CF98 1E386390 */  lbu        $v1, %lo(D_800A381E)($v1)
    /* D79C 8001CF9C 00000000 */  nop
    /* D7A0 8001CFA0 13006010 */  beqz       $v1, .L8001CFF0
    /* D7A4 8001CFA4 2D000224 */   addiu     $v0, $zero, 0x2D
    /* D7A8 8001CFA8 04006214 */  bne        $v1, $v0, .L8001CFBC
    /* D7AC 8001CFAC A3000424 */   addiu     $a0, $zero, 0xA3
    /* D7B0 8001CFB0 7F000524 */  addiu      $a1, $zero, 0x7F
    /* D7B4 8001CFB4 9471010C */  jal        func_8005C650
    /* D7B8 8001CFB8 7F000624 */   addiu     $a2, $zero, 0x7F
  .L8001CFBC:
    /* D7BC 8001CFBC 0A80023C */  lui        $v0, %hi(D_800A381E)
    /* D7C0 8001CFC0 1E384290 */  lbu        $v0, %lo(D_800A381E)($v0)
    /* D7C4 8001CFC4 50000324 */  addiu      $v1, $zero, 0x50
    /* D7C8 8001CFC8 01004224 */  addiu      $v0, $v0, 0x1
    /* D7CC 8001CFCC 0A80013C */  lui        $at, %hi(D_800A381E)
    /* D7D0 8001CFD0 1E3822A0 */  sb         $v0, %lo(D_800A381E)($at)
    /* D7D4 8001CFD4 FF004230 */  andi       $v0, $v0, 0xFF
    /* D7D8 8001CFD8 A8014314 */  bne        $v0, $v1, .L8001D67C
    /* D7DC 8001CFDC 00000000 */   nop
    /* D7E0 8001CFE0 0A80013C */  lui        $at, %hi(D_800A381E)
    /* D7E4 8001CFE4 1E3820A0 */  sb         $zero, %lo(D_800A381E)($at)
    /* D7E8 8001CFE8 A6740008 */  j          .L8001D298
    /* D7EC 8001CFEC 00000000 */   nop
  .L8001CFF0:
    /* D7F0 8001CFF0 0A80023C */  lui        $v0, %hi(D_800A3816)
    /* D7F4 8001CFF4 16384290 */  lbu        $v0, %lo(D_800A3816)($v0)
    /* D7F8 8001CFF8 00000000 */  nop
    /* D7FC 8001CFFC 0F004010 */  beqz       $v0, .L8001D03C
    /* D800 8001D000 01004224 */   addiu     $v0, $v0, 0x1
    /* D804 8001D004 0A80013C */  lui        $at, %hi(D_800A3816)
    /* D808 8001D008 163822A0 */  sb         $v0, %lo(D_800A3816)($at)
    /* D80C 8001D00C FF004330 */  andi       $v1, $v0, 0xFF
    /* D810 8001D010 46000224 */  addiu      $v0, $zero, 0x46
    /* D814 8001D014 03006214 */  bne        $v1, $v0, .L8001D024
    /* D818 8001D018 82000224 */   addiu     $v0, $zero, 0x82
    /* D81C 8001D01C BE740008 */  j          .L8001D2F8
    /* D820 8001D020 9D000424 */   addiu     $a0, $zero, 0x9D
  .L8001D024:
    /* D824 8001D024 95016214 */  bne        $v1, $v0, .L8001D67C
    /* D828 8001D028 01000224 */   addiu     $v0, $zero, 0x1
    /* D82C 8001D02C 0A80013C */  lui        $at, %hi(D_800A3816)
    /* D830 8001D030 163820A0 */  sb         $zero, %lo(D_800A3816)($at)
    /* D834 8001D034 AE740008 */  j          .L8001D2B8
    /* D838 8001D038 00000000 */   nop
  .L8001D03C:
    /* D83C 8001D03C 0A80023C */  lui        $v0, %hi(D_800A37E1)
    /* D840 8001D040 E1374290 */  lbu        $v0, %lo(D_800A37E1)($v0)
    /* D844 8001D044 00000000 */  nop
    /* D848 8001D048 30004010 */  beqz       $v0, .L8001D10C
    /* D84C 8001D04C 02000424 */   addiu     $a0, $zero, 0x2
    /* D850 8001D050 0A80053C */  lui        $a1, %hi(D_800A38B4)
    /* D854 8001D054 B438A58C */  lw         $a1, %lo(D_800A38B4)($a1)
    /* D858 8001D058 A67E010C */  jal        func_8005FA98
    /* D85C 8001D05C 01000624 */   addiu     $a2, $zero, 0x1
    /* D860 8001D060 02004104 */  bgez       $v0, .L8001D06C
    /* D864 8001D064 00000000 */   nop
    /* D868 8001D068 03004224 */  addiu      $v0, $v0, 0x3
  .L8001D06C:
    /* D86C 8001D06C 83100200 */  sra        $v0, $v0, 2
    /* D870 8001D070 80100200 */  sll        $v0, $v0, 2
    /* D874 8001D074 0A80043C */  lui        $a0, %hi(D_800A37E1)
    /* D878 8001D078 E1378490 */  lbu        $a0, %lo(D_800A37E1)($a0)
    /* D87C 8001D07C 0A80033C */  lui        $v1, %hi(D_800A38B4)
    /* D880 8001D080 B438638C */  lw         $v1, %lo(D_800A38B4)($v1)
    /* D884 8001D084 01008424 */  addiu      $a0, $a0, 0x1
    /* D888 8001D088 21186200 */  addu       $v1, $v1, $v0
    /* D88C 8001D08C 0A80013C */  lui        $at, %hi(D_800A37E1)
    /* D890 8001D090 E13724A0 */  sb         $a0, %lo(D_800A37E1)($at)
    /* D894 8001D094 FF008430 */  andi       $a0, $a0, 0xFF
    /* D898 8001D098 3C000224 */  addiu      $v0, $zero, 0x3C
    /* D89C 8001D09C 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* D8A0 8001D0A0 B43823AC */  sw         $v1, %lo(D_800A38B4)($at)
    /* D8A4 8001D0A4 75018214 */  bne        $a0, $v0, .L8001D67C
    /* D8A8 8001D0A8 00000000 */   nop
    /* D8AC 8001D0AC 0A80023C */  lui        $v0, %hi(D_800A38B0)
    /* D8B0 8001D0B0 B0384290 */  lbu        $v0, %lo(D_800A38B0)($v0)
    /* D8B4 8001D0B4 0A80013C */  lui        $at, %hi(D_800A37E1)
    /* D8B8 8001D0B8 E13720A0 */  sb         $zero, %lo(D_800A37E1)($at)
    /* D8BC 8001D0BC 7D005010 */  beq        $v0, $s0, .L8001D2B4
    /* D8C0 8001D0C0 A0000424 */   addiu     $a0, $zero, 0xA0
    /* D8C4 8001D0C4 7F000524 */  addiu      $a1, $zero, 0x7F
    /* D8C8 8001D0C8 9471010C */  jal        func_8005C650
    /* D8CC 8001D0CC 7F000624 */   addiu     $a2, $zero, 0x7F
    /* D8D0 8001D0D0 0A80023C */  lui        $v0, %hi(D_800A38B0)
    /* D8D4 8001D0D4 B0384290 */  lbu        $v0, %lo(D_800A38B0)($v0)
    /* D8D8 8001D0D8 0A80013C */  lui        $at, %hi(D_800A38AA)
    /* D8DC 8001D0DC 21082200 */  addu       $at, $at, $v0
    /* D8E0 8001D0E0 AA382390 */  lbu        $v1, %lo(D_800A38AA)($at)
    /* D8E4 8001D0E4 00000000 */  nop
    /* D8E8 8001D0E8 01006324 */  addiu      $v1, $v1, 0x1
    /* D8EC 8001D0EC 0A80013C */  lui        $at, %hi(D_800A38AA)
    /* D8F0 8001D0F0 21082200 */  addu       $at, $at, $v0
    /* D8F4 8001D0F4 AA3823A0 */  sb         $v1, %lo(D_800A38AA)($at)
    /* D8F8 8001D0F8 01000224 */  addiu      $v0, $zero, 0x1
    /* D8FC 8001D0FC 0A80013C */  lui        $at, %hi(D_800A38B8)
    /* D900 8001D100 B83822A0 */  sb         $v0, %lo(D_800A38B8)($at)
    /* D904 8001D104 9F750008 */  j          .L8001D67C
    /* D908 8001D108 00000000 */   nop
  .L8001D10C:
    /* D90C 8001D10C 0A80023C */  lui        $v0, %hi(D_800A38B8)
    /* D910 8001D110 B8384290 */  lbu        $v0, %lo(D_800A38B8)($v0)
    /* D914 8001D114 00000000 */  nop
    /* D918 8001D118 23004010 */  beqz       $v0, .L8001D1A8
    /* D91C 8001D11C 01004224 */   addiu     $v0, $v0, 0x1
    /* D920 8001D120 0A80013C */  lui        $at, %hi(D_800A38B8)
    /* D924 8001D124 B83822A0 */  sb         $v0, %lo(D_800A38B8)($at)
    /* D928 8001D128 FF004230 */  andi       $v0, $v0, 0xFF
    /* D92C 8001D12C 3C000324 */  addiu      $v1, $zero, 0x3C
    /* D930 8001D130 52014314 */  bne        $v0, $v1, .L8001D67C
    /* D934 8001D134 00000000 */   nop
    /* D938 8001D138 0A80033C */  lui        $v1, %hi(D_800A38B0)
    /* D93C 8001D13C B0386390 */  lbu        $v1, %lo(D_800A38B0)($v1)
    /* D940 8001D140 0A80013C */  lui        $at, %hi(D_800A38B8)
    /* D944 8001D144 B83820A0 */  sb         $zero, %lo(D_800A38B8)($at)
    /* D948 8001D148 0A80013C */  lui        $at, %hi(D_800A38AA)
    /* D94C 8001D14C 21082300 */  addu       $at, $at, $v1
    /* D950 8001D150 AA382290 */  lbu        $v0, %lo(D_800A38AA)($at)
    /* D954 8001D154 00000000 */  nop
    /* D958 8001D158 57005014 */  bne        $v0, $s0, .L8001D2B8
    /* D95C 8001D15C 01000224 */   addiu     $v0, $zero, 0x1
    /* D960 8001D160 0A80023C */  lui        $v0, %hi(D_800A3898)
    /* D964 8001D164 98384224 */  addiu      $v0, $v0, %lo(D_800A3898)
    /* D968 8001D168 0100632C */  sltiu      $v1, $v1, 0x1
    /* D96C 8001D16C 21186200 */  addu       $v1, $v1, $v0
    /* D970 8001D170 00006290 */  lbu        $v0, 0x0($v1)
    /* D974 8001D174 00000000 */  nop
    /* D978 8001D178 01004224 */  addiu      $v0, $v0, 0x1
    /* D97C 8001D17C 000062A0 */  sb         $v0, 0x0($v1)
    /* D980 8001D180 0A80023C */  lui        $v0, %hi(D_800A38B0)
    /* D984 8001D184 B0384290 */  lbu        $v0, %lo(D_800A38B0)($v0)
    /* D988 8001D188 0A80013C */  lui        $at, %hi(D_800A38AA)
    /* D98C 8001D18C 21082200 */  addu       $at, $at, $v0
    /* D990 8001D190 AA3820A0 */  sb         $zero, %lo(D_800A38AA)($at)
    /* D994 8001D194 5A000224 */  addiu      $v0, $zero, 0x5A
    /* D998 8001D198 0A80013C */  lui        $at, %hi(D_800A3920)
    /* D99C 8001D19C 203922A0 */  sb         $v0, %lo(D_800A3920)($at)
    /* D9A0 8001D1A0 9F750008 */  j          .L8001D67C
    /* D9A4 8001D1A4 00000000 */   nop
  .L8001D1A8:
    /* D9A8 8001D1A8 0A80023C */  lui        $v0, %hi(D_800A3920)
    /* D9AC 8001D1AC 20394290 */  lbu        $v0, %lo(D_800A3920)($v0)
    /* D9B0 8001D1B0 00000000 */  nop
    /* D9B4 8001D1B4 46004010 */  beqz       $v0, .L8001D2D0
    /* D9B8 8001D1B8 01000424 */   addiu     $a0, $zero, 0x1
    /* D9BC 8001D1BC 0A80053C */  lui        $a1, %hi(D_800A38B4)
    /* D9C0 8001D1C0 B438A58C */  lw         $a1, %lo(D_800A38B4)($a1)
    /* D9C4 8001D1C4 A67E010C */  jal        func_8005FA98
    /* D9C8 8001D1C8 01000624 */   addiu     $a2, $zero, 0x1
    /* D9CC 8001D1CC 02004104 */  bgez       $v0, .L8001D1D8
    /* D9D0 8001D1D0 00000000 */   nop
    /* D9D4 8001D1D4 03004224 */  addiu      $v0, $v0, 0x3
  .L8001D1D8:
    /* D9D8 8001D1D8 83100200 */  sra        $v0, $v0, 2
    /* D9DC 8001D1DC 80100200 */  sll        $v0, $v0, 2
    /* D9E0 8001D1E0 0A80043C */  lui        $a0, %hi(D_800A3920)
    /* D9E4 8001D1E4 20398490 */  lbu        $a0, %lo(D_800A3920)($a0)
    /* D9E8 8001D1E8 0A80033C */  lui        $v1, %hi(D_800A38B4)
    /* D9EC 8001D1EC B438638C */  lw         $v1, %lo(D_800A38B4)($v1)
    /* D9F0 8001D1F0 01008424 */  addiu      $a0, $a0, 0x1
    /* D9F4 8001D1F4 21186200 */  addu       $v1, $v1, $v0
    /* D9F8 8001D1F8 0A80013C */  lui        $at, %hi(D_800A3920)
    /* D9FC 8001D1FC 203924A0 */  sb         $a0, %lo(D_800A3920)($at)
    /* DA00 8001D200 FF008430 */  andi       $a0, $a0, 0xFF
    /* DA04 8001D204 1E000224 */  addiu      $v0, $zero, 0x1E
    /* DA08 8001D208 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* DA0C 8001D20C B43823AC */  sw         $v1, %lo(D_800A38B4)($at)
    /* DA10 8001D210 04008214 */  bne        $a0, $v0, .L8001D224
    /* DA14 8001D214 A1000424 */   addiu     $a0, $zero, 0xA1
    /* DA18 8001D218 7F000524 */  addiu      $a1, $zero, 0x7F
    /* DA1C 8001D21C 9471010C */  jal        func_8005C650
    /* DA20 8001D220 7F000624 */   addiu     $a2, $zero, 0x7F
  .L8001D224:
    /* DA24 8001D224 0A80033C */  lui        $v1, %hi(D_800A3920)
    /* DA28 8001D228 20396390 */  lbu        $v1, %lo(D_800A3920)($v1)
    /* DA2C 8001D22C 69000224 */  addiu      $v0, $zero, 0x69
    /* DA30 8001D230 08006214 */  bne        $v1, $v0, .L8001D254
    /* DA34 8001D234 5A000224 */   addiu     $v0, $zero, 0x5A
    /* DA38 8001D238 A2000424 */  addiu      $a0, $zero, 0xA2
    /* DA3C 8001D23C 7F000524 */  addiu      $a1, $zero, 0x7F
    /* DA40 8001D240 9471010C */  jal        func_8005C650
    /* DA44 8001D244 7F000624 */   addiu     $a2, $zero, 0x7F
    /* DA48 8001D248 0A80033C */  lui        $v1, %hi(D_800A3920)
    /* DA4C 8001D24C 20396390 */  lbu        $v1, %lo(D_800A3920)($v1)
    /* DA50 8001D250 5A000224 */  addiu      $v0, $zero, 0x5A
  .L8001D254:
    /* DA54 8001D254 03006210 */  beq        $v1, $v0, .L8001D264
    /* DA58 8001D258 B4000224 */   addiu     $v0, $zero, 0xB4
    /* DA5C 8001D25C 07016214 */  bne        $v1, $v0, .L8001D67C
    /* DA60 8001D260 00000000 */   nop
  .L8001D264:
    /* DA64 8001D264 0A80023C */  lui        $v0, %hi(D_800A38B0)
    /* DA68 8001D268 B0384290 */  lbu        $v0, %lo(D_800A38B0)($v0)
    /* DA6C 8001D26C 0A80013C */  lui        $at, %hi(D_800A3920)
    /* DA70 8001D270 203920A0 */  sb         $zero, %lo(D_800A3920)($at)
    /* DA74 8001D274 0100422C */  sltiu      $v0, $v0, 0x1
    /* DA78 8001D278 0A80013C */  lui        $at, %hi(D_800A3898)
    /* DA7C 8001D27C 21082200 */  addu       $at, $at, $v0
    /* DA80 8001D280 98382390 */  lbu        $v1, %lo(D_800A3898)($at)
    /* DA84 8001D284 0A80023C */  lui        $v0, %hi(D_800A37F8)
    /* DA88 8001D288 F8374290 */  lbu        $v0, %lo(D_800A37F8)($v0)
    /* DA8C 8001D28C 00000000 */  nop
    /* DA90 8001D290 09006214 */  bne        $v1, $v0, .L8001D2B8
    /* DA94 8001D294 01000224 */   addiu     $v0, $zero, 0x1
  .L8001D298:
    /* DA98 8001D298 28D0000C */  jal        robtest_disp
    /* DA9C 8001D29C 00000000 */   nop
    /* DAA0 8001D2A0 01000224 */  addiu      $v0, $zero, 0x1
    /* DAA4 8001D2A4 0A80013C */  lui        $at, %hi(D_800A36E8)
    /* DAA8 8001D2A8 E83622A0 */  sb         $v0, %lo(D_800A36E8)($at)
    /* DAAC 8001D2AC 9F750008 */  j          .L8001D67C
    /* DAB0 8001D2B0 00000000 */   nop
  .L8001D2B4:
    /* DAB4 8001D2B4 01000224 */  addiu      $v0, $zero, 0x1
  .L8001D2B8:
    /* DAB8 8001D2B8 0A80013C */  lui        $at, %hi(D_800A3670)
    /* DABC 8001D2BC 703622A0 */  sb         $v0, %lo(D_800A3670)($at)
    /* DAC0 8001D2C0 0A80013C */  lui        $at, %hi(D_800A3834)
    /* DAC4 8001D2C4 343820A4 */  sh         $zero, %lo(D_800A3834)($at)
    /* DAC8 8001D2C8 9F750008 */  j          .L8001D67C
    /* DACC 8001D2CC 00000000 */   nop
  .L8001D2D0:
    /* DAD0 8001D2D0 0A80023C */  lui        $v0, %hi(D_800A391E)
    /* DAD4 8001D2D4 1E394290 */  lbu        $v0, %lo(D_800A391E)($v0)
    /* DAD8 8001D2D8 00000000 */  nop
    /* DADC 8001D2DC 0B004010 */  beqz       $v0, .L8001D30C
    /* DAE0 8001D2E0 FFFF4224 */   addiu     $v0, $v0, -0x1
    /* DAE4 8001D2E4 0A80013C */  lui        $at, %hi(D_800A391E)
    /* DAE8 8001D2E8 1E3922A0 */  sb         $v0, %lo(D_800A391E)($at)
    /* DAEC 8001D2EC FF004230 */  andi       $v0, $v0, 0xFF
    /* DAF0 8001D2F0 E2004014 */  bnez       $v0, .L8001D67C
    /* DAF4 8001D2F4 9C000424 */   addiu     $a0, $zero, 0x9C
  .L8001D2F8:
    /* DAF8 8001D2F8 7F000524 */  addiu      $a1, $zero, 0x7F
    /* DAFC 8001D2FC 9471010C */  jal        func_8005C650
    /* DB00 8001D300 7F000624 */   addiu     $a2, $zero, 0x7F
    /* DB04 8001D304 9F750008 */  j          .L8001D67C
    /* DB08 8001D308 00000000 */   nop
  .L8001D30C:
    /* DB0C 8001D30C 0A80033C */  lui        $v1, %hi(D_800A36E8)
    /* DB10 8001D310 E8366390 */  lbu        $v1, %lo(D_800A36E8)($v1)
    /* DB14 8001D314 00000000 */  nop
    /* DB18 8001D318 45006010 */  beqz       $v1, .L8001D430
    /* DB1C 8001D31C 00000000 */   nop
    /* DB20 8001D320 0A80023C */  lui        $v0, %hi(D_800A3874)
    /* DB24 8001D324 74384290 */  lbu        $v0, %lo(D_800A3874)($v0)
    /* DB28 8001D328 0A80013C */  lui        $at, %hi(D_800A377B)
    /* DB2C 8001D32C 21082200 */  addu       $at, $at, $v0
    /* DB30 8001D330 7B372290 */  lbu        $v0, %lo(D_800A377B)($at)
    /* DB34 8001D334 00000000 */  nop
    /* DB38 8001D338 09005014 */  bne        $v0, $s0, .L8001D360
    /* DB3C 8001D33C 01000224 */   addiu     $v0, $zero, 0x1
    /* DB40 8001D340 2C006214 */  bne        $v1, $v0, .L8001D3F4
    /* DB44 8001D344 50000324 */   addiu     $v1, $zero, 0x50
    /* DB48 8001D348 A4000424 */  addiu      $a0, $zero, 0xA4
    /* DB4C 8001D34C 7F000524 */  addiu      $a1, $zero, 0x7F
    /* DB50 8001D350 9471010C */  jal        func_8005C650
    /* DB54 8001D354 7F000624 */   addiu     $a2, $zero, 0x7F
    /* DB58 8001D358 FD740008 */  j          .L8001D3F4
    /* DB5C 8001D35C 50000324 */   addiu     $v1, $zero, 0x50
  .L8001D360:
    /* DB60 8001D360 04006214 */  bne        $v1, $v0, .L8001D374
    /* DB64 8001D364 A6000424 */   addiu     $a0, $zero, 0xA6
    /* DB68 8001D368 7F000524 */  addiu      $a1, $zero, 0x7F
    /* DB6C 8001D36C 9471010C */  jal        func_8005C650
    /* DB70 8001D370 7F000624 */   addiu     $a2, $zero, 0x7F
  .L8001D374:
    /* DB74 8001D374 0A80033C */  lui        $v1, %hi(D_800A36E8)
    /* DB78 8001D378 E8366390 */  lbu        $v1, %lo(D_800A36E8)($v1)
    /* DB7C 8001D37C 14000224 */  addiu      $v0, $zero, 0x14
    /* DB80 8001D380 1C006214 */  bne        $v1, $v0, .L8001D3F4
    /* DB84 8001D384 64000324 */   addiu     $v1, $zero, 0x64
    /* DB88 8001D388 0A80023C */  lui        $v0, %hi(D_800A3874)
    /* DB8C 8001D38C 74384290 */  lbu        $v0, %lo(D_800A3874)($v0)
    /* DB90 8001D390 0A80013C */  lui        $at, %hi(D_800A377B)
    /* DB94 8001D394 21082200 */  addu       $at, $at, $v0
    /* DB98 8001D398 7B372390 */  lbu        $v1, %lo(D_800A377B)($at)
    /* DB9C 8001D39C 00000000 */  nop
    /* DBA0 8001D3A0 00110300 */  sll        $v0, $v1, 4
    /* DBA4 8001D3A4 21104300 */  addu       $v0, $v0, $v1
    /* DBA8 8001D3A8 80100200 */  sll        $v0, $v0, 2
    /* DBAC 8001D3AC 21104300 */  addu       $v0, $v0, $v1
    /* DBB0 8001D3B0 80100200 */  sll        $v0, $v0, 2
    /* DBB4 8001D3B4 23104300 */  subu       $v0, $v0, $v1
    /* DBB8 8001D3B8 80100200 */  sll        $v0, $v0, 2
    /* DBBC 8001D3BC 1080013C */  lui        $at, %hi(D_80101ED2)
    /* DBC0 8001D3C0 21082200 */  addu       $at, $at, $v0
    /* DBC4 8001D3C4 D21E2284 */  lh         $v0, %lo(D_80101ED2)($at)
    /* DBC8 8001D3C8 0980013C */  lui        $at, %hi(D_8008D9EC)
    /* DBCC 8001D3CC 21082200 */  addu       $at, $at, $v0
    /* DBD0 8001D3D0 ECD92290 */  lbu        $v0, %lo(D_8008D9EC)($at)
    /* DBD4 8001D3D4 00000000 */  nop
    /* DBD8 8001D3D8 02004010 */  beqz       $v0, .L8001D3E4
    /* DBDC 8001D3DC A7000424 */   addiu     $a0, $zero, 0xA7
    /* DBE0 8001D3E0 A8000424 */  addiu      $a0, $zero, 0xA8
  .L8001D3E4:
    /* DBE4 8001D3E4 7F000524 */  addiu      $a1, $zero, 0x7F
    /* DBE8 8001D3E8 9471010C */  jal        func_8005C650
    /* DBEC 8001D3EC 7F000624 */   addiu     $a2, $zero, 0x7F
    /* DBF0 8001D3F0 64000324 */  addiu      $v1, $zero, 0x64
  .L8001D3F4:
    /* DBF4 8001D3F4 0A80023C */  lui        $v0, %hi(D_800A36E8)
    /* DBF8 8001D3F8 E8364290 */  lbu        $v0, %lo(D_800A36E8)($v0)
    /* DBFC 8001D3FC 00000000 */  nop
    /* DC00 8001D400 01004224 */  addiu      $v0, $v0, 0x1
    /* DC04 8001D404 0A80013C */  lui        $at, %hi(D_800A36E8)
    /* DC08 8001D408 E83622A0 */  sb         $v0, %lo(D_800A36E8)($at)
    /* DC0C 8001D40C FF004230 */  andi       $v0, $v0, 0xFF
    /* DC10 8001D410 9A004314 */  bne        $v0, $v1, .L8001D67C
    /* DC14 8001D414 00000000 */   nop
    /* DC18 8001D418 0A80013C */  lui        $at, %hi(D_800A36E8)
    /* DC1C 8001D41C E83620A0 */  sb         $zero, %lo(D_800A36E8)($at)
    /* DC20 8001D420 A8D0000C */  jal        func_800342A0
    /* DC24 8001D424 00000000 */   nop
    /* DC28 8001D428 9F750008 */  j          .L8001D67C
    /* DC2C 8001D42C 00000000 */   nop
  .L8001D430:
    /* DC30 8001D430 1080023C */  lui        $v0, %hi(D_80101F5E)
    /* DC34 8001D434 5E1F4284 */  lh         $v0, %lo(D_80101F5E)($v0)
    /* DC38 8001D438 00000000 */  nop
    /* DC3C 8001D43C 0A004010 */  beqz       $v0, .L8001D468
    /* DC40 8001D440 00000000 */   nop
    /* DC44 8001D444 1080023C */  lui        $v0, %hi(D_801023AA)
    /* DC48 8001D448 AA234284 */  lh         $v0, %lo(D_801023AA)($v0)
    /* DC4C 8001D44C 00000000 */  nop
    /* DC50 8001D450 15004010 */  beqz       $v0, .L8001D4A8
    /* DC54 8001D454 01000224 */   addiu     $v0, $zero, 0x1
    /* DC58 8001D458 0A80013C */  lui        $at, %hi(D_800A3816)
    /* DC5C 8001D45C 163822A0 */  sb         $v0, %lo(D_800A3816)($at)
    /* DC60 8001D460 9F750008 */  j          .L8001D67C
    /* DC64 8001D464 00000000 */   nop
  .L8001D468:
    /* DC68 8001D468 1080023C */  lui        $v0, %hi(D_801023AA)
    /* DC6C 8001D46C AA234284 */  lh         $v0, %lo(D_801023AA)($v0)
    /* DC70 8001D470 00000000 */  nop
    /* DC74 8001D474 0C004010 */  beqz       $v0, .L8001D4A8
    /* DC78 8001D478 01000324 */   addiu     $v1, $zero, 0x1
    /* DC7C 8001D47C 0A80023C */  lui        $v0, %hi(D_800A3898)
    /* DC80 8001D480 98384290 */  lbu        $v0, %lo(D_800A3898)($v0)
    /* DC84 8001D484 0A80013C */  lui        $at, %hi(D_800A38B0)
    /* DC88 8001D488 B03823A0 */  sb         $v1, %lo(D_800A38B0)($at)
    /* DC8C 8001D48C 0A80013C */  lui        $at, %hi(D_800A3920)
    /* DC90 8001D490 203923A0 */  sb         $v1, %lo(D_800A3920)($at)
    /* DC94 8001D494 01004224 */  addiu      $v0, $v0, 0x1
    /* DC98 8001D498 0A80013C */  lui        $at, %hi(D_800A3898)
    /* DC9C 8001D49C 983822A0 */  sb         $v0, %lo(D_800A3898)($at)
    /* DCA0 8001D4A0 9F750008 */  j          .L8001D67C
    /* DCA4 8001D4A4 00000000 */   nop
  .L8001D4A8:
    /* DCA8 8001D4A8 1080023C */  lui        $v0, %hi(D_80101F5E)
    /* DCAC 8001D4AC 5E1F4284 */  lh         $v0, %lo(D_80101F5E)($v0)
    /* DCB0 8001D4B0 00000000 */  nop
    /* DCB4 8001D4B4 0C004010 */  beqz       $v0, .L8001D4E8
    /* DCB8 8001D4B8 01000324 */   addiu     $v1, $zero, 0x1
    /* DCBC 8001D4BC 0A80023C */  lui        $v0, %hi(D_800A3899)
    /* DCC0 8001D4C0 99384290 */  lbu        $v0, %lo(D_800A3899)($v0)
    /* DCC4 8001D4C4 0A80013C */  lui        $at, %hi(D_800A38B0)
    /* DCC8 8001D4C8 B03820A0 */  sb         $zero, %lo(D_800A38B0)($at)
    /* DCCC 8001D4CC 0A80013C */  lui        $at, %hi(D_800A3920)
    /* DCD0 8001D4D0 203923A0 */  sb         $v1, %lo(D_800A3920)($at)
    /* DCD4 8001D4D4 01004224 */  addiu      $v0, $v0, 0x1
    /* DCD8 8001D4D8 0A80013C */  lui        $at, %hi(D_800A3899)
    /* DCDC 8001D4DC 993822A0 */  sb         $v0, %lo(D_800A3899)($at)
    /* DCE0 8001D4E0 9F750008 */  j          .L8001D67C
    /* DCE4 8001D4E4 00000000 */   nop
  .L8001D4E8:
    /* DCE8 8001D4E8 1080023C */  lui        $v0, %hi(D_80101F32)
    /* DCEC 8001D4EC 321F4294 */  lhu        $v0, %lo(D_80101F32)($v0)
    /* DCF0 8001D4F0 06000324 */  addiu      $v1, $zero, 0x6
    /* DCF4 8001D4F4 07004310 */  beq        $v0, $v1, .L8001D514
    /* DCF8 8001D4F8 3C000224 */   addiu     $v0, $zero, 0x3C
    /* DCFC 8001D4FC 1080023C */  lui        $v0, %hi(D_8010237E)
    /* DD00 8001D500 7E234294 */  lhu        $v0, %lo(D_8010237E)($v0)
    /* DD04 8001D504 00000000 */  nop
    /* DD08 8001D508 06004314 */  bne        $v0, $v1, .L8001D524
    /* DD0C 8001D50C 02000324 */   addiu     $v1, $zero, 0x2
    /* DD10 8001D510 3C000224 */  addiu      $v0, $zero, 0x3C
  .L8001D514:
    /* DD14 8001D514 0A80013C */  lui        $at, %hi(D_800A3816)
    /* DD18 8001D518 163822A0 */  sb         $v0, %lo(D_800A3816)($at)
    /* DD1C 8001D51C 9F750008 */  j          .L8001D67C
    /* DD20 8001D520 00000000 */   nop
  .L8001D524:
    /* DD24 8001D524 1080023C */  lui        $v0, %hi(D_80101F79)
    /* DD28 8001D528 791F4290 */  lbu        $v0, %lo(D_80101F79)($v0)
    /* DD2C 8001D52C 00000000 */  nop
    /* DD30 8001D530 1A004314 */  bne        $v0, $v1, .L8001D59C
    /* DD34 8001D534 9F000424 */   addiu     $a0, $zero, 0x9F
    /* DD38 8001D538 7F000524 */  addiu      $a1, $zero, 0x7F
    /* DD3C 8001D53C 9471010C */  jal        func_8005C650
    /* DD40 8001D540 7F000624 */   addiu     $a2, $zero, 0x7F
    /* DD44 8001D544 1080043C */  lui        $a0, %hi(D_80101F32)
    /* DD48 8001D548 321F8494 */  lhu        $a0, %lo(D_80101F32)($a0)
    /* DD4C 8001D54C 01000224 */  addiu      $v0, $zero, 0x1
    /* DD50 8001D550 0A80013C */  lui        $at, %hi(D_800A37E1)
    /* DD54 8001D554 E13722A0 */  sb         $v0, %lo(D_800A37E1)($at)
    /* DD58 8001D558 13000224 */  addiu      $v0, $zero, 0x13
    /* DD5C 8001D55C FFFF8330 */  andi       $v1, $a0, 0xFFFF
    /* DD60 8001D560 0A006210 */  beq        $v1, $v0, .L8001D58C
    /* DD64 8001D564 1B000224 */   addiu     $v0, $zero, 0x1B
    /* DD68 8001D568 08006210 */  beq        $v1, $v0, .L8001D58C
    /* DD6C 8001D56C 30000224 */   addiu     $v0, $zero, 0x30
    /* DD70 8001D570 06006210 */  beq        $v1, $v0, .L8001D58C
    /* DD74 8001D574 E7FF8224 */   addiu     $v0, $a0, -0x19
    /* DD78 8001D578 0200422C */  sltiu      $v0, $v0, 0x2
    /* DD7C 8001D57C 03004014 */  bnez       $v0, .L8001D58C
    /* DD80 8001D580 18000224 */   addiu     $v0, $zero, 0x18
    /* DD84 8001D584 24006214 */  bne        $v1, $v0, .L8001D618
    /* DD88 8001D588 02000224 */   addiu     $v0, $zero, 0x2
  .L8001D58C:
    /* DD8C 8001D58C 0A80013C */  lui        $at, %hi(D_800A38B0)
    /* DD90 8001D590 B03820A0 */  sb         $zero, %lo(D_800A38B0)($at)
    /* DD94 8001D594 9F750008 */  j          .L8001D67C
    /* DD98 8001D598 00000000 */   nop
  .L8001D59C:
    /* DD9C 8001D59C 1080023C */  lui        $v0, %hi(D_801023C5)
    /* DDA0 8001D5A0 C5234290 */  lbu        $v0, %lo(D_801023C5)($v0)
    /* DDA4 8001D5A4 00000000 */  nop
    /* DDA8 8001D5A8 1F004314 */  bne        $v0, $v1, .L8001D628
    /* DDAC 8001D5AC 7F000524 */   addiu     $a1, $zero, 0x7F
    /* DDB0 8001D5B0 9471010C */  jal        func_8005C650
    /* DDB4 8001D5B4 7F000624 */   addiu     $a2, $zero, 0x7F
    /* DDB8 8001D5B8 1080043C */  lui        $a0, %hi(D_8010237E)
    /* DDBC 8001D5BC 7E238494 */  lhu        $a0, %lo(D_8010237E)($a0)
    /* DDC0 8001D5C0 01000224 */  addiu      $v0, $zero, 0x1
    /* DDC4 8001D5C4 0A80013C */  lui        $at, %hi(D_800A37E1)
    /* DDC8 8001D5C8 E13722A0 */  sb         $v0, %lo(D_800A37E1)($at)
    /* DDCC 8001D5CC 13000224 */  addiu      $v0, $zero, 0x13
    /* DDD0 8001D5D0 FFFF8330 */  andi       $v1, $a0, 0xFFFF
    /* DDD4 8001D5D4 0B006210 */  beq        $v1, $v0, .L8001D604
    /* DDD8 8001D5D8 1B000224 */   addiu     $v0, $zero, 0x1B
    /* DDDC 8001D5DC 09006210 */  beq        $v1, $v0, .L8001D604
    /* DDE0 8001D5E0 30000224 */   addiu     $v0, $zero, 0x30
    /* DDE4 8001D5E4 07006210 */  beq        $v1, $v0, .L8001D604
    /* DDE8 8001D5E8 E7FF8224 */   addiu     $v0, $a0, -0x19
    /* DDEC 8001D5EC 0200422C */  sltiu      $v0, $v0, 0x2
    /* DDF0 8001D5F0 05004014 */  bnez       $v0, .L8001D608
    /* DDF4 8001D5F4 01000224 */   addiu     $v0, $zero, 0x1
    /* DDF8 8001D5F8 18000224 */  addiu      $v0, $zero, 0x18
    /* DDFC 8001D5FC 06006214 */  bne        $v1, $v0, .L8001D618
    /* DE00 8001D600 02000224 */   addiu     $v0, $zero, 0x2
  .L8001D604:
    /* DE04 8001D604 01000224 */  addiu      $v0, $zero, 0x1
  .L8001D608:
    /* DE08 8001D608 0A80013C */  lui        $at, %hi(D_800A38B0)
    /* DE0C 8001D60C B03822A0 */  sb         $v0, %lo(D_800A38B0)($at)
    /* DE10 8001D610 9F750008 */  j          .L8001D67C
    /* DE14 8001D614 00000000 */   nop
  .L8001D618:
    /* DE18 8001D618 0A80013C */  lui        $at, %hi(D_800A38B0)
    /* DE1C 8001D61C B03822A0 */  sb         $v0, %lo(D_800A38B0)($at)
    /* DE20 8001D620 9F750008 */  j          .L8001D67C
    /* DE24 8001D624 00000000 */   nop
  .L8001D628:
    /* DE28 8001D628 0A80043C */  lui        $a0, %hi(D_800A36CC)
    /* DE2C 8001D62C CC368490 */  lbu        $a0, %lo(D_800A36CC)($a0)
    /* DE30 8001D630 00000000 */  nop
    /* DE34 8001D634 34008010 */  beqz       $a0, .L8001D708
    /* DE38 8001D638 00110400 */   sll       $v0, $a0, 4
    /* DE3C 8001D63C 23104400 */  subu       $v0, $v0, $a0
    /* DE40 8001D640 0A80033C */  lui        $v1, %hi(D_800A38F4)
    /* DE44 8001D644 F438638C */  lw         $v1, %lo(D_800A38F4)($v1)
    /* DE48 8001D648 40100200 */  sll        $v0, $v0, 1
    /* DE4C 8001D64C 01006324 */  addiu      $v1, $v1, 0x1
    /* DE50 8001D650 0A80013C */  lui        $at, %hi(D_800A38F4)
    /* DE54 8001D654 F43823AC */  sw         $v1, %lo(D_800A38F4)($at)
    /* DE58 8001D658 2A186200 */  slt        $v1, $v1, $v0
    /* DE5C 8001D65C 07006014 */  bnez       $v1, .L8001D67C
    /* DE60 8001D660 9E000424 */   addiu     $a0, $zero, 0x9E
    /* DE64 8001D664 7F000524 */  addiu      $a1, $zero, 0x7F
    /* DE68 8001D668 9471010C */  jal        func_8005C650
    /* DE6C 8001D66C 7F000624 */   addiu     $a2, $zero, 0x7F
    /* DE70 8001D670 01000224 */  addiu      $v0, $zero, 0x1
    /* DE74 8001D674 0A80013C */  lui        $at, %hi(D_800A381E)
    /* DE78 8001D678 1E3822A0 */  sb         $v0, %lo(D_800A381E)($at)
  .L8001D67C:
    /* DE7C 8001D67C 0A80033C */  lui        $v1, %hi(D_800A36CC)
    /* DE80 8001D680 CC366390 */  lbu        $v1, %lo(D_800A36CC)($v1)
    /* DE84 8001D684 00000000 */  nop
    /* DE88 8001D688 1F006010 */  beqz       $v1, .L8001D708
    /* DE8C 8001D68C 00110300 */   sll       $v0, $v1, 4
    /* DE90 8001D690 8888053C */  lui        $a1, (0x88888889 >> 16)
    /* DE94 8001D694 8988A534 */  ori        $a1, $a1, (0x88888889 & 0xFFFF)
    /* DE98 8001D698 23104300 */  subu       $v0, $v0, $v1
    /* DE9C 8001D69C 0A80033C */  lui        $v1, %hi(D_800A38F4)
    /* DEA0 8001D6A0 F438638C */  lw         $v1, %lo(D_800A38F4)($v1)
    /* DEA4 8001D6A4 40100200 */  sll        $v0, $v0, 1
    /* DEA8 8001D6A8 23184300 */  subu       $v1, $v0, $v1
    /* DEAC 8001D6AC 18006500 */  mult       $v1, $a1
    /* DEB0 8001D6B0 C3170300 */  sra        $v0, $v1, 31
    /* DEB4 8001D6B4 10400000 */  mfhi       $t0
    /* DEB8 8001D6B8 21200301 */  addu       $a0, $t0, $v1
    /* DEBC 8001D6BC 03210400 */  sra        $a0, $a0, 4
    /* DEC0 8001D6C0 23208200 */  subu       $a0, $a0, $v0
    /* DEC4 8001D6C4 00110400 */  sll        $v0, $a0, 4
    /* DEC8 8001D6C8 23104400 */  subu       $v0, $v0, $a0
    /* DECC 8001D6CC 40100200 */  sll        $v0, $v0, 1
    /* DED0 8001D6D0 23106200 */  subu       $v0, $v1, $v0
    /* DED4 8001D6D4 40180200 */  sll        $v1, $v0, 1
    /* DED8 8001D6D8 21186200 */  addu       $v1, $v1, $v0
    /* DEDC 8001D6DC C0180300 */  sll        $v1, $v1, 3
    /* DEE0 8001D6E0 21186200 */  addu       $v1, $v1, $v0
    /* DEE4 8001D6E4 80180300 */  sll        $v1, $v1, 2
    /* DEE8 8001D6E8 18006500 */  mult       $v1, $a1
    /* DEEC 8001D6EC 1200A4A3 */  sb         $a0, 0x12($sp)
    /* DEF0 8001D6F0 10400000 */  mfhi       $t0
    /* DEF4 8001D6F4 21100301 */  addu       $v0, $t0, $v1
    /* DEF8 8001D6F8 03110200 */  sra        $v0, $v0, 4
    /* DEFC 8001D6FC C31F0300 */  sra        $v1, $v1, 31
    /* DF00 8001D700 23104300 */  subu       $v0, $v0, $v1
    /* DF04 8001D704 1300A2A3 */  sb         $v0, 0x13($sp)
  .L8001D708:
    /* DF08 8001D708 1000A427 */  addiu      $a0, $sp, 0x10
    /* DF0C 8001D70C 01000724 */  addiu      $a3, $zero, 0x1
    /* DF10 8001D710 0A80053C */  lui        $a1, %hi(D_800A38AA)
    /* DF14 8001D714 AA38A590 */  lbu        $a1, %lo(D_800A38AA)($a1)
    /* DF18 8001D718 0A80063C */  lui        $a2, %hi(D_800A38B4)
    /* DF1C 8001D71C B438C68C */  lw         $a2, %lo(D_800A38B4)($a2)
    /* DF20 8001D720 0A80033C */  lui        $v1, %hi(D_800A3898)
    /* DF24 8001D724 98386390 */  lbu        $v1, %lo(D_800A3898)($v1)
    /* DF28 8001D728 0A80023C */  lui        $v0, %hi(D_800A3899)
    /* DF2C 8001D72C 99384290 */  lbu        $v0, %lo(D_800A3899)($v0)
    /* DF30 8001D730 002A0500 */  sll        $a1, $a1, 8
    /* DF34 8001D734 25186500 */  or         $v1, $v1, $a1
    /* DF38 8001D738 00110200 */  sll        $v0, $v0, 4
    /* DF3C 8001D73C 0A80053C */  lui        $a1, %hi(D_800A38AB)
    /* DF40 8001D740 AB38A590 */  lbu        $a1, %lo(D_800A38AB)($a1)
    /* DF44 8001D744 25186200 */  or         $v1, $v1, $v0
    /* DF48 8001D748 002B0500 */  sll        $a1, $a1, 12
    /* DF4C 8001D74C 727C010C */  jal        func_8005F1C8
    /* DF50 8001D750 25286500 */   or        $a1, $v1, $a1
  .L8001D754:
    /* DF54 8001D754 02004104 */  bgez       $v0, .L8001D760
    /* DF58 8001D758 00000000 */   nop
    /* DF5C 8001D75C 03004224 */  addiu      $v0, $v0, 0x3
  .L8001D760:
    /* DF60 8001D760 83100200 */  sra        $v0, $v0, 2
    /* DF64 8001D764 0A80033C */  lui        $v1, %hi(D_800A38B4)
    /* DF68 8001D768 B438638C */  lw         $v1, %lo(D_800A38B4)($v1)
    /* DF6C 8001D76C 80100200 */  sll        $v0, $v0, 2
    /* DF70 8001D770 21186200 */  addu       $v1, $v1, $v0
    /* DF74 8001D774 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* DF78 8001D778 B43823AC */  sw         $v1, %lo(D_800A38B4)($at)
  .L8001D77C:
    /* DF7C 8001D77C 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* DF80 8001D780 1800B08F */  lw         $s0, 0x18($sp)
    /* DF84 8001D784 2000BD27 */  addiu      $sp, $sp, 0x20
    /* DF88 8001D788 0800E003 */  jr         $ra
    /* DF8C 8001D78C 00000000 */   nop
endlabel camera_set_target_zoom
