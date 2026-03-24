glabel func_8008AF9C
    /* 7B79C 8008AF9C F0FFBD27 */  addiu      $sp, $sp, -0x10
    /* 7B7A0 8008AFA0 21300000 */  addu       $a2, $zero, $zero
    /* 7B7A4 8008AFA4 0000898C */  lw         $t1, 0x0($a0)
    /* 7B7A8 8008AFA8 00000000 */  nop
    /* 7B7AC 8008AFAC 01002A2D */  sltiu      $t2, $t1, 0x1
    /* 7B7B0 8008AFB0 06004015 */  bnez       $t2, .L8008AFCC
    /* 7B7B4 8008AFB4 21400000 */   addu      $t0, $zero, $zero
    /* 7B7B8 8008AFB8 01002231 */  andi       $v0, $t1, 0x1
    /* 7B7BC 8008AFBC 2F004010 */  beqz       $v0, .L8008B07C
    /* 7B7C0 8008AFC0 04002231 */   andi      $v0, $t1, 0x4
    /* 7B7C4 8008AFC4 1A004010 */  beqz       $v0, .L8008B030
    /* 7B7C8 8008AFC8 00000000 */   nop
  .L8008AFCC:
    /* 7B7CC 8008AFCC 08008384 */  lh         $v1, 0x8($a0)
    /* 7B7D0 8008AFD0 00000000 */  nop
    /* 7B7D4 8008AFD4 0800622C */  sltiu      $v0, $v1, 0x8
    /* 7B7D8 8008AFD8 15004010 */  beqz       $v0, .L8008B030
    /* 7B7DC 8008AFDC 80100300 */   sll       $v0, $v1, 2
    /* 7B7E0 8008AFE0 0180013C */  lui        $at, %hi(jtbl_80016420)
    /* 7B7E4 8008AFE4 21082200 */  addu       $at, $at, $v0
    /* 7B7E8 8008AFE8 2064228C */  lw         $v0, %lo(jtbl_80016420)($at)
    /* 7B7EC 8008AFEC 00000000 */  nop
    /* 7B7F0 8008AFF0 08004000 */  jr         $v0
    /* 7B7F4 8008AFF4 00000000 */   nop
  jlabel .L8008AFF8
    /* 7B7F8 8008AFF8 0E2C0208 */  j          .L8008B038
    /* 7B7FC 8008AFFC 00800534 */   ori       $a1, $zero, 0x8000
  jlabel .L8008B000
    /* 7B800 8008B000 0E2C0208 */  j          .L8008B038
    /* 7B804 8008B004 00900534 */   ori       $a1, $zero, 0x9000
  jlabel .L8008B008
    /* 7B808 8008B008 0E2C0208 */  j          .L8008B038
    /* 7B80C 8008B00C 00A00534 */   ori       $a1, $zero, 0xA000
  jlabel .L8008B010
    /* 7B810 8008B010 0E2C0208 */  j          .L8008B038
    /* 7B814 8008B014 00B00534 */   ori       $a1, $zero, 0xB000
  jlabel .L8008B018
    /* 7B818 8008B018 0E2C0208 */  j          .L8008B038
    /* 7B81C 8008B01C 00C00534 */   ori       $a1, $zero, 0xC000
  jlabel .L8008B020
    /* 7B820 8008B020 0E2C0208 */  j          .L8008B038
    /* 7B824 8008B024 00D00534 */   ori       $a1, $zero, 0xD000
  jlabel .L8008B028
    /* 7B828 8008B028 0E2C0208 */  j          .L8008B038
    /* 7B82C 8008B02C 00E00534 */   ori       $a1, $zero, 0xE000
  jlabel .L8008B030
    /* 7B830 8008B030 04008694 */  lhu        $a2, 0x4($a0)
    /* 7B834 8008B034 21280000 */  addu       $a1, $zero, $zero
  .L8008B038:
    /* 7B838 8008B038 0C00A010 */  beqz       $a1, .L8008B06C
    /* 7B83C 8008B03C FF7FC230 */   andi      $v0, $a2, 0x7FFF
    /* 7B840 8008B040 04008784 */  lh         $a3, 0x4($a0)
    /* 7B844 8008B044 00000000 */  nop
    /* 7B848 8008B048 8000E228 */  slti       $v0, $a3, 0x80
    /* 7B84C 8008B04C 03004014 */  bnez       $v0, .L8008B05C
    /* 7B850 8008B050 2118E000 */   addu      $v1, $a3, $zero
    /* 7B854 8008B054 1A2C0208 */  j          .L8008B068
    /* 7B858 8008B058 7F000624 */   addiu     $a2, $zero, 0x7F
  .L8008B05C:
    /* 7B85C 8008B05C 0200E104 */  bgez       $a3, .L8008B068
    /* 7B860 8008B060 21306000 */   addu      $a2, $v1, $zero
    /* 7B864 8008B064 21300000 */  addu       $a2, $zero, $zero
  .L8008B068:
    /* 7B868 8008B068 FF7FC230 */  andi       $v0, $a2, 0x7FFF
  .L8008B06C:
    /* 7B86C 8008B06C 0A80033C */  lui        $v1, %hi(D_800A2CDC)
    /* 7B870 8008B070 DC2C638C */  lw         $v1, %lo(D_800A2CDC)($v1)
    /* 7B874 8008B074 25104500 */  or         $v0, $v0, $a1
    /* 7B878 8008B078 800162A4 */  sh         $v0, 0x180($v1)
  .L8008B07C:
    /* 7B87C 8008B07C 05004015 */  bnez       $t2, .L8008B094
    /* 7B880 8008B080 02002231 */   andi      $v0, $t1, 0x2
    /* 7B884 8008B084 2F004010 */  beqz       $v0, .L8008B144
    /* 7B888 8008B088 08002231 */   andi      $v0, $t1, 0x8
    /* 7B88C 8008B08C 1A004010 */  beqz       $v0, .L8008B0F8
    /* 7B890 8008B090 00000000 */   nop
  .L8008B094:
    /* 7B894 8008B094 0A008384 */  lh         $v1, 0xA($a0)
    /* 7B898 8008B098 00000000 */  nop
    /* 7B89C 8008B09C 0800622C */  sltiu      $v0, $v1, 0x8
    /* 7B8A0 8008B0A0 15004010 */  beqz       $v0, .L8008B0F8
    /* 7B8A4 8008B0A4 80100300 */   sll       $v0, $v1, 2
    /* 7B8A8 8008B0A8 0180013C */  lui        $at, %hi(jtbl_80016440)
    /* 7B8AC 8008B0AC 21082200 */  addu       $at, $at, $v0
    /* 7B8B0 8008B0B0 4064228C */  lw         $v0, %lo(jtbl_80016440)($at)
    /* 7B8B4 8008B0B4 00000000 */  nop
    /* 7B8B8 8008B0B8 08004000 */  jr         $v0
    /* 7B8BC 8008B0BC 00000000 */   nop
  jlabel .L8008B0C0
    /* 7B8C0 8008B0C0 402C0208 */  j          .L8008B100
    /* 7B8C4 8008B0C4 00800534 */   ori       $a1, $zero, 0x8000
  jlabel .L8008B0C8
    /* 7B8C8 8008B0C8 402C0208 */  j          .L8008B100
    /* 7B8CC 8008B0CC 00900534 */   ori       $a1, $zero, 0x9000
  jlabel .L8008B0D0
    /* 7B8D0 8008B0D0 402C0208 */  j          .L8008B100
    /* 7B8D4 8008B0D4 00A00534 */   ori       $a1, $zero, 0xA000
  jlabel .L8008B0D8
    /* 7B8D8 8008B0D8 402C0208 */  j          .L8008B100
    /* 7B8DC 8008B0DC 00B00534 */   ori       $a1, $zero, 0xB000
  jlabel .L8008B0E0
    /* 7B8E0 8008B0E0 402C0208 */  j          .L8008B100
    /* 7B8E4 8008B0E4 00C00534 */   ori       $a1, $zero, 0xC000
  jlabel .L8008B0E8
    /* 7B8E8 8008B0E8 402C0208 */  j          .L8008B100
    /* 7B8EC 8008B0EC 00D00534 */   ori       $a1, $zero, 0xD000
  jlabel .L8008B0F0
    /* 7B8F0 8008B0F0 402C0208 */  j          .L8008B100
    /* 7B8F4 8008B0F4 00E00534 */   ori       $a1, $zero, 0xE000
  jlabel .L8008B0F8
    /* 7B8F8 8008B0F8 06008894 */  lhu        $t0, 0x6($a0)
    /* 7B8FC 8008B0FC 21280000 */  addu       $a1, $zero, $zero
  .L8008B100:
    /* 7B900 8008B100 0C00A010 */  beqz       $a1, .L8008B134
    /* 7B904 8008B104 FF7F0231 */   andi      $v0, $t0, 0x7FFF
    /* 7B908 8008B108 06008684 */  lh         $a2, 0x6($a0)
    /* 7B90C 8008B10C 00000000 */  nop
    /* 7B910 8008B110 8000C228 */  slti       $v0, $a2, 0x80
    /* 7B914 8008B114 03004014 */  bnez       $v0, .L8008B124
    /* 7B918 8008B118 2118C000 */   addu      $v1, $a2, $zero
    /* 7B91C 8008B11C 4C2C0208 */  j          .L8008B130
    /* 7B920 8008B120 7F000824 */   addiu     $t0, $zero, 0x7F
  .L8008B124:
    /* 7B924 8008B124 0200C104 */  bgez       $a2, .L8008B130
    /* 7B928 8008B128 21406000 */   addu      $t0, $v1, $zero
    /* 7B92C 8008B12C 21400000 */  addu       $t0, $zero, $zero
  .L8008B130:
    /* 7B930 8008B130 FF7F0231 */  andi       $v0, $t0, 0x7FFF
  .L8008B134:
    /* 7B934 8008B134 0A80033C */  lui        $v1, %hi(D_800A2CDC)
    /* 7B938 8008B138 DC2C638C */  lw         $v1, %lo(D_800A2CDC)($v1)
    /* 7B93C 8008B13C 25104500 */  or         $v0, $v0, $a1
    /* 7B940 8008B140 820162A4 */  sh         $v0, 0x182($v1)
  .L8008B144:
    /* 7B944 8008B144 03004015 */  bnez       $t2, .L8008B154
    /* 7B948 8008B148 40002231 */   andi      $v0, $t1, 0x40
    /* 7B94C 8008B14C 06004010 */  beqz       $v0, .L8008B168
    /* 7B950 8008B150 00000000 */   nop
  .L8008B154:
    /* 7B954 8008B154 0A80033C */  lui        $v1, %hi(D_800A2CDC)
    /* 7B958 8008B158 DC2C638C */  lw         $v1, %lo(D_800A2CDC)($v1)
    /* 7B95C 8008B15C 10008294 */  lhu        $v0, 0x10($a0)
    /* 7B960 8008B160 00000000 */  nop
    /* 7B964 8008B164 B00162A4 */  sh         $v0, 0x1B0($v1)
  .L8008B168:
    /* 7B968 8008B168 03004015 */  bnez       $t2, .L8008B178
    /* 7B96C 8008B16C 80002231 */   andi      $v0, $t1, 0x80
    /* 7B970 8008B170 06004010 */  beqz       $v0, .L8008B18C
    /* 7B974 8008B174 00000000 */   nop
  .L8008B178:
    /* 7B978 8008B178 0A80033C */  lui        $v1, %hi(D_800A2CDC)
    /* 7B97C 8008B17C DC2C638C */  lw         $v1, %lo(D_800A2CDC)($v1)
    /* 7B980 8008B180 12008294 */  lhu        $v0, 0x12($a0)
    /* 7B984 8008B184 00000000 */  nop
    /* 7B988 8008B188 B20162A4 */  sh         $v0, 0x1B2($v1)
  .L8008B18C:
    /* 7B98C 8008B18C 03004015 */  bnez       $t2, .L8008B19C
    /* 7B990 8008B190 00042231 */   andi      $v0, $t1, 0x400
    /* 7B994 8008B194 06004010 */  beqz       $v0, .L8008B1B0
    /* 7B998 8008B198 00000000 */   nop
  .L8008B19C:
    /* 7B99C 8008B19C 0A80033C */  lui        $v1, %hi(D_800A2CDC)
    /* 7B9A0 8008B1A0 DC2C638C */  lw         $v1, %lo(D_800A2CDC)($v1)
    /* 7B9A4 8008B1A4 1C008294 */  lhu        $v0, 0x1C($a0)
    /* 7B9A8 8008B1A8 00000000 */  nop
    /* 7B9AC 8008B1AC B40162A4 */  sh         $v0, 0x1B4($v1)
  .L8008B1B0:
    /* 7B9B0 8008B1B0 03004015 */  bnez       $t2, .L8008B1C0
    /* 7B9B4 8008B1B4 00082231 */   andi      $v0, $t1, 0x800
    /* 7B9B8 8008B1B8 06004010 */  beqz       $v0, .L8008B1D4
    /* 7B9BC 8008B1BC 00000000 */   nop
  .L8008B1C0:
    /* 7B9C0 8008B1C0 0A80033C */  lui        $v1, %hi(D_800A2CDC)
    /* 7B9C4 8008B1C4 DC2C638C */  lw         $v1, %lo(D_800A2CDC)($v1)
    /* 7B9C8 8008B1C8 1E008294 */  lhu        $v0, 0x1E($a0)
    /* 7B9CC 8008B1CC 00000000 */  nop
    /* 7B9D0 8008B1D0 B60162A4 */  sh         $v0, 0x1B6($v1)
  .L8008B1D4:
    /* 7B9D4 8008B1D4 03004015 */  bnez       $t2, .L8008B1E4
    /* 7B9D8 8008B1D8 00012231 */   andi      $v0, $t1, 0x100
    /* 7B9DC 8008B1DC 12004010 */  beqz       $v0, .L8008B228
    /* 7B9E0 8008B1E0 00000000 */   nop
  .L8008B1E4:
    /* 7B9E4 8008B1E4 1400828C */  lw         $v0, 0x14($a0)
    /* 7B9E8 8008B1E8 00000000 */  nop
    /* 7B9EC 8008B1EC 07004014 */  bnez       $v0, .L8008B20C
    /* 7B9F0 8008B1F0 00000000 */   nop
    /* 7B9F4 8008B1F4 0A80023C */  lui        $v0, %hi(D_800A2CDC)
    /* 7B9F8 8008B1F8 DC2C428C */  lw         $v0, %lo(D_800A2CDC)($v0)
    /* 7B9FC 8008B1FC 00000000 */  nop
    /* 7BA00 8008B200 AA014394 */  lhu        $v1, 0x1AA($v0)
    /* 7BA04 8008B204 892C0208 */  j          .L8008B224
    /* 7BA08 8008B208 FBFF6330 */   andi      $v1, $v1, 0xFFFB
  .L8008B20C:
    /* 7BA0C 8008B20C 0A80023C */  lui        $v0, %hi(D_800A2CDC)
    /* 7BA10 8008B210 DC2C428C */  lw         $v0, %lo(D_800A2CDC)($v0)
    /* 7BA14 8008B214 00000000 */  nop
    /* 7BA18 8008B218 AA014394 */  lhu        $v1, 0x1AA($v0)
    /* 7BA1C 8008B21C 00000000 */  nop
    /* 7BA20 8008B220 04006334 */  ori        $v1, $v1, 0x4
  .L8008B224:
    /* 7BA24 8008B224 AA0143A4 */  sh         $v1, 0x1AA($v0)
  .L8008B228:
    /* 7BA28 8008B228 03004015 */  bnez       $t2, .L8008B238
    /* 7BA2C 8008B22C 00022231 */   andi      $v0, $t1, 0x200
    /* 7BA30 8008B230 12004010 */  beqz       $v0, .L8008B27C
    /* 7BA34 8008B234 00000000 */   nop
  .L8008B238:
    /* 7BA38 8008B238 1800828C */  lw         $v0, 0x18($a0)
    /* 7BA3C 8008B23C 00000000 */  nop
    /* 7BA40 8008B240 07004014 */  bnez       $v0, .L8008B260
    /* 7BA44 8008B244 00000000 */   nop
    /* 7BA48 8008B248 0A80023C */  lui        $v0, %hi(D_800A2CDC)
    /* 7BA4C 8008B24C DC2C428C */  lw         $v0, %lo(D_800A2CDC)($v0)
    /* 7BA50 8008B250 00000000 */  nop
    /* 7BA54 8008B254 AA014394 */  lhu        $v1, 0x1AA($v0)
    /* 7BA58 8008B258 9E2C0208 */  j          .L8008B278
    /* 7BA5C 8008B25C FEFF6330 */   andi      $v1, $v1, 0xFFFE
  .L8008B260:
    /* 7BA60 8008B260 0A80023C */  lui        $v0, %hi(D_800A2CDC)
    /* 7BA64 8008B264 DC2C428C */  lw         $v0, %lo(D_800A2CDC)($v0)
    /* 7BA68 8008B268 00000000 */  nop
    /* 7BA6C 8008B26C AA014394 */  lhu        $v1, 0x1AA($v0)
    /* 7BA70 8008B270 00000000 */  nop
    /* 7BA74 8008B274 01006334 */  ori        $v1, $v1, 0x1
  .L8008B278:
    /* 7BA78 8008B278 AA0143A4 */  sh         $v1, 0x1AA($v0)
  .L8008B27C:
    /* 7BA7C 8008B27C 03004015 */  bnez       $t2, .L8008B28C
    /* 7BA80 8008B280 00102231 */   andi      $v0, $t1, 0x1000
    /* 7BA84 8008B284 12004010 */  beqz       $v0, .L8008B2D0
    /* 7BA88 8008B288 00000000 */   nop
  .L8008B28C:
    /* 7BA8C 8008B28C 2000828C */  lw         $v0, 0x20($a0)
    /* 7BA90 8008B290 00000000 */  nop
    /* 7BA94 8008B294 07004014 */  bnez       $v0, .L8008B2B4
    /* 7BA98 8008B298 00000000 */   nop
    /* 7BA9C 8008B29C 0A80023C */  lui        $v0, %hi(D_800A2CDC)
    /* 7BAA0 8008B2A0 DC2C428C */  lw         $v0, %lo(D_800A2CDC)($v0)
    /* 7BAA4 8008B2A4 00000000 */  nop
    /* 7BAA8 8008B2A8 AA014394 */  lhu        $v1, 0x1AA($v0)
    /* 7BAAC 8008B2AC B32C0208 */  j          .L8008B2CC
    /* 7BAB0 8008B2B0 F7FF6330 */   andi      $v1, $v1, 0xFFF7
  .L8008B2B4:
    /* 7BAB4 8008B2B4 0A80023C */  lui        $v0, %hi(D_800A2CDC)
    /* 7BAB8 8008B2B8 DC2C428C */  lw         $v0, %lo(D_800A2CDC)($v0)
    /* 7BABC 8008B2BC 00000000 */  nop
    /* 7BAC0 8008B2C0 AA014394 */  lhu        $v1, 0x1AA($v0)
    /* 7BAC4 8008B2C4 00000000 */  nop
    /* 7BAC8 8008B2C8 08006334 */  ori        $v1, $v1, 0x8
  .L8008B2CC:
    /* 7BACC 8008B2CC AA0143A4 */  sh         $v1, 0x1AA($v0)
  .L8008B2D0:
    /* 7BAD0 8008B2D0 03004015 */  bnez       $t2, .L8008B2E0
    /* 7BAD4 8008B2D4 00202231 */   andi      $v0, $t1, 0x2000
    /* 7BAD8 8008B2D8 12004010 */  beqz       $v0, .L8008B324
    /* 7BADC 8008B2DC 00000000 */   nop
  .L8008B2E0:
    /* 7BAE0 8008B2E0 2400828C */  lw         $v0, 0x24($a0)
    /* 7BAE4 8008B2E4 00000000 */  nop
    /* 7BAE8 8008B2E8 07004014 */  bnez       $v0, .L8008B308
    /* 7BAEC 8008B2EC 00000000 */   nop
    /* 7BAF0 8008B2F0 0A80023C */  lui        $v0, %hi(D_800A2CDC)
    /* 7BAF4 8008B2F4 DC2C428C */  lw         $v0, %lo(D_800A2CDC)($v0)
    /* 7BAF8 8008B2F8 00000000 */  nop
    /* 7BAFC 8008B2FC AA014394 */  lhu        $v1, 0x1AA($v0)
    /* 7BB00 8008B300 C82C0208 */  j          .L8008B320
    /* 7BB04 8008B304 FDFF6330 */   andi      $v1, $v1, 0xFFFD
  .L8008B308:
    /* 7BB08 8008B308 0A80023C */  lui        $v0, %hi(D_800A2CDC)
    /* 7BB0C 8008B30C DC2C428C */  lw         $v0, %lo(D_800A2CDC)($v0)
    /* 7BB10 8008B310 00000000 */  nop
    /* 7BB14 8008B314 AA014394 */  lhu        $v1, 0x1AA($v0)
    /* 7BB18 8008B318 00000000 */  nop
    /* 7BB1C 8008B31C 02006334 */  ori        $v1, $v1, 0x2
  .L8008B320:
    /* 7BB20 8008B320 AA0143A4 */  sh         $v1, 0x1AA($v0)
  .L8008B324:
    /* 7BB24 8008B324 1000BD27 */  addiu      $sp, $sp, 0x10
    /* 7BB28 8008B328 0800E003 */  jr         $ra
    /* 7BB2C 8008B32C 00000000 */   nop
    /* 7BB30 8008B330 03008104 */  bgez       $a0, .L8008B340
    /* 7BB34 8008B334 18008228 */   slti      $v0, $a0, 0x18
    /* 7BB38 8008B338 21200000 */  addu       $a0, $zero, $zero
    /* 7BB3C 8008B33C 18008228 */  slti       $v0, $a0, 0x18
  .L8008B340:
    /* 7BB40 8008B340 08004010 */  beqz       $v0, .L8008B364
    /* 7BB44 8008B344 1800A228 */   slti      $v0, $a1, 0x18
    /* 7BB48 8008B348 02004014 */  bnez       $v0, .L8008B354
    /* 7BB4C 8008B34C 00000000 */   nop
    /* 7BB50 8008B350 17000524 */  addiu      $a1, $zero, 0x17
  .L8008B354:
    /* 7BB54 8008B354 0300A004 */  bltz       $a1, .L8008B364
    /* 7BB58 8008B358 2A10A400 */   slt       $v0, $a1, $a0
    /* 7BB5C 8008B35C 03004010 */  beqz       $v0, .L8008B36C
    /* 7BB60 8008B360 21388000 */   addu      $a3, $a0, $zero
  .L8008B364:
    /* 7BB64 8008B364 FE2C0208 */  j          .L8008B3F8
    /* 7BB68 8008B368 FDFF0224 */   addiu     $v0, $zero, -0x3
  .L8008B36C:
    /* 7BB6C 8008B36C 0100A524 */  addiu      $a1, $a1, 0x1
    /* 7BB70 8008B370 2A10E500 */  slt        $v0, $a3, $a1
    /* 7BB74 8008B374 20004010 */  beqz       $v0, .L8008B3F8
    /* 7BB78 8008B378 21100000 */   addu      $v0, $zero, $zero
    /* 7BB7C 8008B37C 01000824 */  addiu      $t0, $zero, 0x1
    /* 7BB80 8008B380 03000A24 */  addiu      $t2, $zero, 0x3
    /* 7BB84 8008B384 02000924 */  addiu      $t1, $zero, 0x2
    /* 7BB88 8008B388 2130E600 */  addu       $a2, $a3, $a2
  .L8008B38C:
    /* 7BB8C 8008B38C 00210700 */  sll        $a0, $a3, 4
    /* 7BB90 8008B390 0A80023C */  lui        $v0, %hi(D_800A2CDC)
    /* 7BB94 8008B394 DC2C428C */  lw         $v0, %lo(D_800A2CDC)($v0)
    /* 7BB98 8008B398 0A80033C */  lui        $v1, %hi(D_800A2874)
    /* 7BB9C 8008B39C 7428638C */  lw         $v1, %lo(D_800A2874)($v1)
    /* 7BBA0 8008B3A0 21208200 */  addu       $a0, $a0, $v0
    /* 7BBA4 8008B3A4 0410E800 */  sllv       $v0, $t0, $a3
    /* 7BBA8 8008B3A8 24186200 */  and        $v1, $v1, $v0
    /* 7BBAC 8008B3AC 0C008294 */  lhu        $v0, 0xC($a0)
    /* 7BBB0 8008B3B0 07006010 */  beqz       $v1, .L8008B3D0
    /* 7BBB4 8008B3B4 00000000 */   nop
    /* 7BBB8 8008B3B8 03004010 */  beqz       $v0, .L8008B3C8
    /* 7BBBC 8008B3BC 00000000 */   nop
    /* 7BBC0 8008B3C0 F92C0208 */  j          .L8008B3E4
    /* 7BBC4 8008B3C4 0000C8A0 */   sb        $t0, 0x0($a2)
  .L8008B3C8:
    /* 7BBC8 8008B3C8 F92C0208 */  j          .L8008B3E4
    /* 7BBCC 8008B3CC 0000CAA0 */   sb        $t2, 0x0($a2)
  .L8008B3D0:
    /* 7BBD0 8008B3D0 03004010 */  beqz       $v0, .L8008B3E0
    /* 7BBD4 8008B3D4 00000000 */   nop
    /* 7BBD8 8008B3D8 F92C0208 */  j          .L8008B3E4
    /* 7BBDC 8008B3DC 0000C9A0 */   sb        $t1, 0x0($a2)
  .L8008B3E0:
    /* 7BBE0 8008B3E0 0000C0A0 */  sb         $zero, 0x0($a2)
  .L8008B3E4:
    /* 7BBE4 8008B3E4 0100E724 */  addiu      $a3, $a3, 0x1
    /* 7BBE8 8008B3E8 2A10E500 */  slt        $v0, $a3, $a1
    /* 7BBEC 8008B3EC E7FF4014 */  bnez       $v0, .L8008B38C
    /* 7BBF0 8008B3F0 0100C624 */   addiu     $a2, $a2, 0x1
    /* 7BBF4 8008B3F4 21100000 */  addu       $v0, $zero, $zero
  .L8008B3F8:
    /* 7BBF8 8008B3F8 0800E003 */  jr         $ra
    /* 7BBFC 8008B3FC 00000000 */   nop
endlabel func_8008AF9C
