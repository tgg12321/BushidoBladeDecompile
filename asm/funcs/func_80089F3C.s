glabel func_80089F3C
    /* 7A73C 80089F3C 70FFBD27 */  addiu      $sp, $sp, -0x90
    /* 7A740 80089F40 7000B2AF */  sw         $s2, 0x70($sp)
    /* 7A744 80089F44 21908000 */  addu       $s2, $a0, $zero
    /* 7A748 80089F48 8400B7AF */  sw         $s7, 0x84($sp)
    /* 7A74C 80089F4C 21B80000 */  addu       $s7, $zero, $zero
    /* 7A750 80089F50 7800B4AF */  sw         $s4, 0x78($sp)
    /* 7A754 80089F54 21A00000 */  addu       $s4, $zero, $zero
    /* 7A758 80089F58 8000B6AF */  sw         $s6, 0x80($sp)
    /* 7A75C 80089F5C 21B00000 */  addu       $s6, $zero, $zero
    /* 7A760 80089F60 8C00BFAF */  sw         $ra, 0x8C($sp)
    /* 7A764 80089F64 8800BEAF */  sw         $fp, 0x88($sp)
    /* 7A768 80089F68 7C00B5AF */  sw         $s5, 0x7C($sp)
    /* 7A76C 80089F6C 7400B3AF */  sw         $s3, 0x74($sp)
    /* 7A770 80089F70 6C00B1AF */  sw         $s1, 0x6C($sp)
    /* 7A774 80089F74 6800B0AF */  sw         $s0, 0x68($sp)
    /* 7A778 80089F78 5800A0AF */  sw         $zero, 0x58($sp)
    /* 7A77C 80089F7C 0000538E */  lw         $s3, 0x0($s2)
    /* 7A780 80089F80 21F00000 */  addu       $fp, $zero, $zero
    /* 7A784 80089F84 0100752E */  sltiu      $s5, $s3, 0x1
    /* 7A788 80089F88 0400A016 */  bnez       $s5, .L80089F9C
    /* 7A78C 80089F8C 1000A0AF */   sw        $zero, 0x10($sp)
    /* 7A790 80089F90 01006232 */  andi       $v0, $s3, 0x1
    /* 7A794 80089F94 49004010 */  beqz       $v0, .L8008A0BC
    /* 7A798 80089F98 00000000 */   nop
  .L80089F9C:
    /* 7A79C 80089F9C 0400508E */  lw         $s0, 0x4($s2)
    /* 7A7A0 80089FA0 00000000 */  nop
    /* 7A7A4 80089FA4 00010232 */  andi       $v0, $s0, 0x100
    /* 7A7A8 80089FA8 04004010 */  beqz       $v0, .L80089FBC
    /* 7A7AC 80089FAC FFFE0224 */   addiu     $v0, $zero, -0x101
    /* 7A7B0 80089FB0 24800202 */  and        $s0, $s0, $v0
    /* 7A7B4 80089FB4 01000824 */  addiu      $t0, $zero, 0x1
    /* 7A7B8 80089FB8 5800A8AF */  sw         $t0, 0x58($sp)
  .L80089FBC:
    /* 7A7BC 80089FBC 0A00022E */  sltiu      $v0, $s0, 0xA
    /* 7A7C0 80089FC0 0A004010 */  beqz       $v0, .L80089FEC
    /* 7A7C4 80089FC4 80101000 */   sll       $v0, $s0, 2
    /* 7A7C8 80089FC8 0A80013C */  lui        $at, %hi(D_800A2D44)
    /* 7A7CC 80089FCC 21082200 */  addu       $at, $at, $v0
    /* 7A7D0 80089FD0 442D248C */  lw         $a0, %lo(D_800A2D44)($at)
    /* 7A7D4 80089FD4 0A80113C */  lui        $s1, %hi(D_800A2D44)
    /* 7A7D8 80089FD8 442D3126 */  addiu      $s1, $s1, %lo(D_800A2D44)
    /* 7A7DC 80089FDC AC27020C */  jal        action_check_defense2
    /* 7A7E0 80089FE0 00000000 */   nop
    /* 7A7E4 80089FE4 03004010 */  beqz       $v0, .L80089FF4
    /* 7A7E8 80089FE8 01001424 */   addiu     $s4, $zero, 0x1
  .L80089FEC:
    /* 7A7EC 80089FEC 00290208 */  j          .L8008A400
    /* 7A7F0 80089FF0 FFFF0224 */   addiu     $v0, $zero, -0x1
  .L80089FF4:
    /* 7A7F4 80089FF4 1000A627 */  addiu      $a2, $sp, 0x10
    /* 7A7F8 80089FF8 43000524 */  addiu      $a1, $zero, 0x43
    /* 7A7FC 80089FFC 0A80013C */  lui        $at, %hi(D_800A288C)
    /* 7A800 8008A000 8C2830AC */  sw         $s0, %lo(D_800A288C)($at)
    /* 7A804 8008A004 0A80033C */  lui        $v1, %hi(D_800A288C)
    /* 7A808 8008A008 8C28638C */  lw         $v1, %lo(D_800A288C)($v1)
    /* 7A80C 8008A00C FFFF0724 */  addiu      $a3, $zero, -0x1
    /* 7A810 8008A010 80200300 */  sll        $a0, $v1, 2
    /* 7A814 8008A014 21209100 */  addu       $a0, $a0, $s1
    /* 7A818 8008A018 00110300 */  sll        $v0, $v1, 4
    /* 7A81C 8008A01C 21104300 */  addu       $v0, $v0, $v1
    /* 7A820 8008A020 80100200 */  sll        $v0, $v0, 2
    /* 7A824 8008A024 0A80033C */  lui        $v1, %hi(D_800A2D94)
    /* 7A828 8008A028 942D6324 */  addiu      $v1, $v1, %lo(D_800A2D94)
    /* 7A82C 8008A02C 0000848C */  lw         $a0, 0x0($a0)
    /* 7A830 8008A030 21184300 */  addu       $v1, $v0, $v1
    /* 7A834 8008A034 0A80013C */  lui        $at, %hi(D_800A2884)
    /* 7A838 8008A038 842824AC */  sw         $a0, %lo(D_800A2884)($at)
  .L8008A03C:
    /* 7A83C 8008A03C 00006290 */  lbu        $v0, 0x0($v1)
    /* 7A840 8008A040 01006324 */  addiu      $v1, $v1, 0x1
    /* 7A844 8008A044 FFFFA524 */  addiu      $a1, $a1, -0x1
    /* 7A848 8008A048 0000C2A0 */  sb         $v0, 0x0($a2)
    /* 7A84C 8008A04C FBFFA714 */  bne        $a1, $a3, .L8008A03C
    /* 7A850 8008A050 0100C624 */   addiu     $a2, $a2, 0x1
    /* 7A854 8008A054 0A80033C */  lui        $v1, %hi(D_800A288C)
    /* 7A858 8008A058 8C28638C */  lw         $v1, %lo(D_800A288C)($v1)
    /* 7A85C 8008A05C 07000224 */  addiu      $v0, $zero, 0x7
    /* 7A860 8008A060 05006210 */  beq        $v1, $v0, .L8008A078
    /* 7A864 8008A064 08000224 */   addiu     $v0, $zero, 0x8
    /* 7A868 8008A068 0A006210 */  beq        $v1, $v0, .L8008A094
    /* 7A86C 8008A06C 7F000224 */   addiu     $v0, $zero, 0x7F
    /* 7A870 8008A070 2B280208 */  j          .L8008A0AC
    /* 7A874 8008A074 00000000 */   nop
  .L8008A078:
    /* 7A878 8008A078 7F000224 */  addiu      $v0, $zero, 0x7F
    /* 7A87C 8008A07C 0A80013C */  lui        $at, %hi(D_800A2898)
    /* 7A880 8008A080 982822AC */  sw         $v0, %lo(D_800A2898)($at)
    /* 7A884 8008A084 0A80013C */  lui        $at, %hi(D_800A2894)
    /* 7A888 8008A088 942822AC */  sw         $v0, %lo(D_800A2894)($at)
    /* 7A88C 8008A08C 2F280208 */  j          .L8008A0BC
    /* 7A890 8008A090 00000000 */   nop
  .L8008A094:
    /* 7A894 8008A094 0A80013C */  lui        $at, %hi(D_800A2898)
    /* 7A898 8008A098 982820AC */  sw         $zero, %lo(D_800A2898)($at)
    /* 7A89C 8008A09C 0A80013C */  lui        $at, %hi(D_800A2894)
    /* 7A8A0 8008A0A0 942822AC */  sw         $v0, %lo(D_800A2894)($at)
    /* 7A8A4 8008A0A4 2F280208 */  j          .L8008A0BC
    /* 7A8A8 8008A0A8 00000000 */   nop
  .L8008A0AC:
    /* 7A8AC 8008A0AC 0A80013C */  lui        $at, %hi(D_800A2898)
    /* 7A8B0 8008A0B0 982820AC */  sw         $zero, %lo(D_800A2898)($at)
    /* 7A8B4 8008A0B4 0A80013C */  lui        $at, %hi(D_800A2894)
    /* 7A8B8 8008A0B8 942820AC */  sw         $zero, %lo(D_800A2894)($at)
  .L8008A0BC:
    /* 7A8BC 8008A0BC 0300A016 */  bnez       $s5, .L8008A0CC
    /* 7A8C0 8008A0C0 08006232 */   andi      $v0, $s3, 0x8
    /* 7A8C4 8008A0C4 45004010 */  beqz       $v0, .L8008A1DC
    /* 7A8C8 8008A0C8 00000000 */   nop
  .L8008A0CC:
    /* 7A8CC 8008A0CC 0A80033C */  lui        $v1, %hi(D_800A288C)
    /* 7A8D0 8008A0D0 8C28638C */  lw         $v1, %lo(D_800A288C)($v1)
    /* 7A8D4 8008A0D4 00000000 */  nop
    /* 7A8D8 8008A0D8 09006228 */  slti       $v0, $v1, 0x9
    /* 7A8DC 8008A0DC 3F004010 */  beqz       $v0, .L8008A1DC
    /* 7A8E0 8008A0E0 07006228 */   slti      $v0, $v1, 0x7
    /* 7A8E4 8008A0E4 3D004014 */  bnez       $v0, .L8008A1DC
    /* 7A8E8 8008A0E8 00000000 */   nop
    /* 7A8EC 8008A0EC 15008016 */  bnez       $s4, .L8008A144
    /* 7A8F0 8008A0F0 01001624 */   addiu     $s6, $zero, 0x1
    /* 7A8F4 8008A0F4 1000A527 */  addiu      $a1, $sp, 0x10
    /* 7A8F8 8008A0F8 43000424 */  addiu      $a0, $zero, 0x43
    /* 7A8FC 8008A0FC 0A80023C */  lui        $v0, %hi(D_800A288C)
    /* 7A900 8008A100 8C28428C */  lw         $v0, %lo(D_800A288C)($v0)
    /* 7A904 8008A104 FFFF0624 */  addiu      $a2, $zero, -0x1
    /* 7A908 8008A108 00190200 */  sll        $v1, $v0, 4
    /* 7A90C 8008A10C 21186200 */  addu       $v1, $v1, $v0
    /* 7A910 8008A110 80180300 */  sll        $v1, $v1, 2
    /* 7A914 8008A114 0A80023C */  lui        $v0, %hi(D_800A2D94)
    /* 7A918 8008A118 942D4224 */  addiu      $v0, $v0, %lo(D_800A2D94)
    /* 7A91C 8008A11C 21186200 */  addu       $v1, $v1, $v0
  .L8008A120:
    /* 7A920 8008A120 00006290 */  lbu        $v0, 0x0($v1)
    /* 7A924 8008A124 01006324 */  addiu      $v1, $v1, 0x1
    /* 7A928 8008A128 FFFF8424 */  addiu      $a0, $a0, -0x1
    /* 7A92C 8008A12C 0000A2A0 */  sb         $v0, 0x0($a1)
    /* 7A930 8008A130 FBFF8614 */  bne        $a0, $a2, .L8008A120
    /* 7A934 8008A134 0100A524 */   addiu     $a1, $a1, 0x1
    /* 7A938 8008A138 010C023C */  lui        $v0, (0xC011C00 >> 16)
    /* 7A93C 8008A13C 001C4234 */  ori        $v0, $v0, (0xC011C00 & 0xFFFF)
    /* 7A940 8008A140 1000A2AF */  sw         $v0, 0x10($sp)
  .L8008A144:
    /* 7A944 8008A144 0281043C */  lui        $a0, (0x81020409 >> 16)
    /* 7A948 8008A148 0C00428E */  lw         $v0, 0xC($s2)
    /* 7A94C 8008A14C 09048434 */  ori        $a0, $a0, (0x81020409 & 0xFFFF)
    /* 7A950 8008A150 401B0200 */  sll        $v1, $v0, 13
    /* 7A954 8008A154 18006400 */  mult       $v1, $a0
    /* 7A958 8008A158 10380000 */  mfhi       $a3
    /* 7A95C 8008A15C 002B0200 */  sll        $a1, $v0, 12
    /* 7A960 8008A160 00000000 */  nop
    /* 7A964 8008A164 1800A400 */  mult       $a1, $a0
    /* 7A968 8008A168 0A80013C */  lui        $at, %hi(D_800A2894)
    /* 7A96C 8008A16C 942822AC */  sw         $v0, %lo(D_800A2894)($at)
    /* 7A970 8008A170 2110E300 */  addu       $v0, $a3, $v1
    /* 7A974 8008A174 83110200 */  sra        $v0, $v0, 6
    /* 7A978 8008A178 C31F0300 */  sra        $v1, $v1, 31
    /* 7A97C 8008A17C 23104300 */  subu       $v0, $v0, $v1
    /* 7A980 8008A180 1400A497 */  lhu        $a0, 0x14($sp)
    /* 7A984 8008A184 3600A397 */  lhu        $v1, 0x36($sp)
    /* 7A988 8008A188 23104400 */  subu       $v0, $v0, $a0
    /* 7A98C 8008A18C 2800A2A7 */  sh         $v0, 0x28($sp)
    /* 7A990 8008A190 1600A297 */  lhu        $v0, 0x16($sp)
    /* 7A994 8008A194 10300000 */  mfhi       $a2
    /* 7A998 8008A198 2120C500 */  addu       $a0, $a2, $a1
    /* 7A99C 8008A19C 83210400 */  sra        $a0, $a0, 6
    /* 7A9A0 8008A1A0 C32F0500 */  sra        $a1, $a1, 31
    /* 7A9A4 8008A1A4 23208500 */  subu       $a0, $a0, $a1
    /* 7A9A8 8008A1A8 23108200 */  subu       $v0, $a0, $v0
    /* 7A9AC 8008A1AC 2A00A2A7 */  sh         $v0, 0x2A($sp)
    /* 7A9B0 8008A1B0 2E00A297 */  lhu        $v0, 0x2E($sp)
    /* 7A9B4 8008A1B4 21186400 */  addu       $v1, $v1, $a0
    /* 7A9B8 8008A1B8 3400A3A7 */  sh         $v1, 0x34($sp)
    /* 7A9BC 8008A1BC 4E00A397 */  lhu        $v1, 0x4E($sp)
    /* 7A9C0 8008A1C0 21104400 */  addu       $v0, $v0, $a0
    /* 7A9C4 8008A1C4 2C00A2A7 */  sh         $v0, 0x2C($sp)
    /* 7A9C8 8008A1C8 4C00A297 */  lhu        $v0, 0x4C($sp)
    /* 7A9CC 8008A1CC 21186400 */  addu       $v1, $v1, $a0
    /* 7A9D0 8008A1D0 4A00A3A7 */  sh         $v1, 0x4A($sp)
    /* 7A9D4 8008A1D4 21104400 */  addu       $v0, $v0, $a0
    /* 7A9D8 8008A1D8 4800A2A7 */  sh         $v0, 0x48($sp)
  .L8008A1DC:
    /* 7A9DC 8008A1DC 0300A016 */  bnez       $s5, .L8008A1EC
    /* 7A9E0 8008A1E0 10006232 */   andi      $v0, $s3, 0x10
    /* 7A9E4 8008A1E4 32004010 */  beqz       $v0, .L8008A2B0
    /* 7A9E8 8008A1E8 00000000 */   nop
  .L8008A1EC:
    /* 7A9EC 8008A1EC 0A80033C */  lui        $v1, %hi(D_800A288C)
    /* 7A9F0 8008A1F0 8C28638C */  lw         $v1, %lo(D_800A288C)($v1)
    /* 7A9F4 8008A1F4 00000000 */  nop
    /* 7A9F8 8008A1F8 09006228 */  slti       $v0, $v1, 0x9
    /* 7A9FC 8008A1FC 2C004010 */  beqz       $v0, .L8008A2B0
    /* 7AA00 8008A200 07006228 */   slti      $v0, $v1, 0x7
    /* 7AA04 8008A204 2A004014 */  bnez       $v0, .L8008A2B0
    /* 7AA08 8008A208 00000000 */   nop
    /* 7AA0C 8008A20C 19008016 */  bnez       $s4, .L8008A274
    /* 7AA10 8008A210 01001E24 */   addiu     $fp, $zero, 0x1
    /* 7AA14 8008A214 1300C016 */  bnez       $s6, .L8008A264
    /* 7AA18 8008A218 1000A527 */   addiu     $a1, $sp, 0x10
    /* 7AA1C 8008A21C 43000424 */  addiu      $a0, $zero, 0x43
    /* 7AA20 8008A220 0A80023C */  lui        $v0, %hi(D_800A288C)
    /* 7AA24 8008A224 8C28428C */  lw         $v0, %lo(D_800A288C)($v0)
    /* 7AA28 8008A228 FFFF0624 */  addiu      $a2, $zero, -0x1
    /* 7AA2C 8008A22C 00190200 */  sll        $v1, $v0, 4
    /* 7AA30 8008A230 21186200 */  addu       $v1, $v1, $v0
    /* 7AA34 8008A234 80180300 */  sll        $v1, $v1, 2
    /* 7AA38 8008A238 0A80023C */  lui        $v0, %hi(D_800A2D94)
    /* 7AA3C 8008A23C 942D4224 */  addiu      $v0, $v0, %lo(D_800A2D94)
    /* 7AA40 8008A240 21186200 */  addu       $v1, $v1, $v0
  .L8008A244:
    /* 7AA44 8008A244 00006290 */  lbu        $v0, 0x0($v1)
    /* 7AA48 8008A248 01006324 */  addiu      $v1, $v1, 0x1
    /* 7AA4C 8008A24C FFFF8424 */  addiu      $a0, $a0, -0x1
    /* 7AA50 8008A250 0000A2A0 */  sb         $v0, 0x0($a1)
    /* 7AA54 8008A254 FBFF8614 */  bne        $a0, $a2, .L8008A244
    /* 7AA58 8008A258 0100A524 */   addiu     $a1, $a1, 0x1
    /* 7AA5C 8008A25C 9C280208 */  j          .L8008A270
    /* 7AA60 8008A260 80000224 */   addiu     $v0, $zero, 0x80
  .L8008A264:
    /* 7AA64 8008A264 1000A28F */  lw         $v0, 0x10($sp)
    /* 7AA68 8008A268 00000000 */  nop
    /* 7AA6C 8008A26C 80004234 */  ori        $v0, $v0, 0x80
  .L8008A270:
    /* 7AA70 8008A270 1000A2AF */  sw         $v0, 0x10($sp)
  .L8008A274:
    /* 7AA74 8008A274 0281043C */  lui        $a0, (0x81020409 >> 16)
    /* 7AA78 8008A278 1000438E */  lw         $v1, 0x10($s2)
    /* 7AA7C 8008A27C 09048434 */  ori        $a0, $a0, (0x81020409 & 0xFFFF)
    /* 7AA80 8008A280 C0110300 */  sll        $v0, $v1, 7
    /* 7AA84 8008A284 21104300 */  addu       $v0, $v0, $v1
    /* 7AA88 8008A288 00120200 */  sll        $v0, $v0, 8
    /* 7AA8C 8008A28C 18004400 */  mult       $v0, $a0
    /* 7AA90 8008A290 0A80013C */  lui        $at, %hi(D_800A2898)
    /* 7AA94 8008A294 982823AC */  sw         $v1, %lo(D_800A2898)($at)
    /* 7AA98 8008A298 10400000 */  mfhi       $t0
    /* 7AA9C 8008A29C 21180201 */  addu       $v1, $t0, $v0
    /* 7AAA0 8008A2A0 83190300 */  sra        $v1, $v1, 6
    /* 7AAA4 8008A2A4 C3170200 */  sra        $v0, $v0, 31
    /* 7AAA8 8008A2A8 23186200 */  subu       $v1, $v1, $v0
    /* 7AAAC 8008A2AC 2200A3A7 */  sh         $v1, 0x22($sp)
  .L8008A2B0:
    /* 7AAB0 8008A2B0 10008012 */  beqz       $s4, .L8008A2F4
    /* 7AAB4 8008A2B4 00000000 */   nop
    /* 7AAB8 8008A2B8 0A80043C */  lui        $a0, %hi(D_800A2CDC)
    /* 7AABC 8008A2BC DC2C848C */  lw         $a0, %lo(D_800A2CDC)($a0)
    /* 7AAC0 8008A2C0 00000000 */  nop
    /* 7AAC4 8008A2C4 AA018294 */  lhu        $v0, 0x1AA($a0)
    /* 7AAC8 8008A2C8 00000000 */  nop
    /* 7AACC 8008A2CC C2110200 */  srl        $v0, $v0, 7
    /* 7AAD0 8008A2D0 01005730 */  andi       $s7, $v0, 0x1
    /* 7AAD4 8008A2D4 2100E012 */  beqz       $s7, .L8008A35C
    /* 7AAD8 8008A2D8 00000000 */   nop
    /* 7AADC 8008A2DC AA018394 */  lhu        $v1, 0x1AA($a0)
    /* 7AAE0 8008A2E0 00000000 */  nop
    /* 7AAE4 8008A2E4 7FFF6330 */  andi       $v1, $v1, 0xFF7F
    /* 7AAE8 8008A2E8 AA0183A4 */  sh         $v1, 0x1AA($a0)
    /* 7AAEC 8008A2EC D7280208 */  j          .L8008A35C
    /* 7AAF0 8008A2F0 00000000 */   nop
  .L8008A2F4:
    /* 7AAF4 8008A2F4 0300A016 */  bnez       $s5, .L8008A304
    /* 7AAF8 8008A2F8 02006232 */   andi      $v0, $s3, 0x2
    /* 7AAFC 8008A2FC 09004010 */  beqz       $v0, .L8008A324
    /* 7AB00 8008A300 00000000 */   nop
  .L8008A304:
    /* 7AB04 8008A304 0A80023C */  lui        $v0, %hi(D_800A2CDC)
    /* 7AB08 8008A308 DC2C428C */  lw         $v0, %lo(D_800A2CDC)($v0)
    /* 7AB0C 8008A30C 08004396 */  lhu        $v1, 0x8($s2)
    /* 7AB10 8008A310 00000000 */  nop
    /* 7AB14 8008A314 840143A4 */  sh         $v1, 0x184($v0)
    /* 7AB18 8008A318 08004296 */  lhu        $v0, 0x8($s2)
    /* 7AB1C 8008A31C 0A80013C */  lui        $at, %hi(D_800A2890)
    /* 7AB20 8008A320 902822A4 */  sh         $v0, %lo(D_800A2890)($at)
  .L8008A324:
    /* 7AB24 8008A324 0300A016 */  bnez       $s5, .L8008A334
    /* 7AB28 8008A328 04006232 */   andi      $v0, $s3, 0x4
    /* 7AB2C 8008A32C 14004010 */  beqz       $v0, .L8008A380
    /* 7AB30 8008A330 00000000 */   nop
  .L8008A334:
    /* 7AB34 8008A334 0A80023C */  lui        $v0, %hi(D_800A2CDC)
    /* 7AB38 8008A338 DC2C428C */  lw         $v0, %lo(D_800A2CDC)($v0)
    /* 7AB3C 8008A33C 0A004396 */  lhu        $v1, 0xA($s2)
    /* 7AB40 8008A340 00000000 */  nop
    /* 7AB44 8008A344 860143A4 */  sh         $v1, 0x186($v0)
    /* 7AB48 8008A348 0A004296 */  lhu        $v0, 0xA($s2)
    /* 7AB4C 8008A34C 0A80013C */  lui        $at, %hi(D_800A2892)
    /* 7AB50 8008A350 922822A4 */  sh         $v0, %lo(D_800A2892)($at)
    /* 7AB54 8008A354 E0280208 */  j          .L8008A380
    /* 7AB58 8008A358 00000000 */   nop
  .L8008A35C:
    /* 7AB5C 8008A35C 0A80023C */  lui        $v0, %hi(D_800A2CDC)
    /* 7AB60 8008A360 DC2C428C */  lw         $v0, %lo(D_800A2CDC)($v0)
    /* 7AB64 8008A364 00000000 */  nop
    /* 7AB68 8008A368 840140A4 */  sh         $zero, 0x184($v0)
    /* 7AB6C 8008A36C 860140A4 */  sh         $zero, 0x186($v0)
    /* 7AB70 8008A370 0A80013C */  lui        $at, %hi(D_800A2890)
    /* 7AB74 8008A374 902820A4 */  sh         $zero, %lo(D_800A2890)($at)
    /* 7AB78 8008A378 0A80013C */  lui        $at, %hi(D_800A2892)
    /* 7AB7C 8008A37C 922820A4 */  sh         $zero, %lo(D_800A2892)($at)
  .L8008A380:
    /* 7AB80 8008A380 05008016 */  bnez       $s4, .L8008A398
    /* 7AB84 8008A384 00000000 */   nop
    /* 7AB88 8008A388 0300C016 */  bnez       $s6, .L8008A398
    /* 7AB8C 8008A38C 00000000 */   nop
    /* 7AB90 8008A390 0300C013 */  beqz       $fp, .L8008A3A0
    /* 7AB94 8008A394 00000000 */   nop
  .L8008A398:
    /* 7AB98 8008A398 0D29020C */  jal        func_8008A434
    /* 7AB9C 8008A39C 1000A427 */   addiu     $a0, $sp, 0x10
  .L8008A3A0:
    /* 7ABA0 8008A3A0 5800A88F */  lw         $t0, 0x58($sp)
    /* 7ABA4 8008A3A4 00000000 */  nop
    /* 7ABA8 8008A3A8 05000011 */  beqz       $t0, .L8008A3C0
    /* 7ABAC 8008A3AC 00000000 */   nop
    /* 7ABB0 8008A3B0 0A80043C */  lui        $a0, %hi(D_800A288C)
    /* 7ABB4 8008A3B4 8C28848C */  lw         $a0, %lo(D_800A288C)($a0)
    /* 7ABB8 8008A3B8 4A2A020C */  jal        md_game_check_change_main_mode_katinuki
    /* 7ABBC 8008A3BC 00000000 */   nop
  .L8008A3C0:
    /* 7ABC0 8008A3C0 0E008012 */  beqz       $s4, .L8008A3FC
    /* 7ABC4 8008A3C4 D1000424 */   addiu     $a0, $zero, 0xD1
    /* 7ABC8 8008A3C8 0A80053C */  lui        $a1, %hi(D_800A2884)
    /* 7ABCC 8008A3CC 8428A58C */  lw         $a1, %lo(D_800A2884)($a1)
    /* 7ABD0 8008A3D0 2324020C */  jal        spu_WriteReg
    /* 7ABD4 8008A3D4 21300000 */   addu      $a2, $zero, $zero
    /* 7ABD8 8008A3D8 0900E012 */  beqz       $s7, .L8008A400
    /* 7ABDC 8008A3DC 21100000 */   addu      $v0, $zero, $zero
    /* 7ABE0 8008A3E0 0A80023C */  lui        $v0, %hi(D_800A2CDC)
    /* 7ABE4 8008A3E4 DC2C428C */  lw         $v0, %lo(D_800A2CDC)($v0)
    /* 7ABE8 8008A3E8 00000000 */  nop
    /* 7ABEC 8008A3EC AA014394 */  lhu        $v1, 0x1AA($v0)
    /* 7ABF0 8008A3F0 00000000 */  nop
    /* 7ABF4 8008A3F4 80006334 */  ori        $v1, $v1, 0x80
    /* 7ABF8 8008A3F8 AA0143A4 */  sh         $v1, 0x1AA($v0)
  .L8008A3FC:
    /* 7ABFC 8008A3FC 21100000 */  addu       $v0, $zero, $zero
  .L8008A400:
    /* 7AC00 8008A400 8C00BF8F */  lw         $ra, 0x8C($sp)
    /* 7AC04 8008A404 8800BE8F */  lw         $fp, 0x88($sp)
    /* 7AC08 8008A408 8400B78F */  lw         $s7, 0x84($sp)
    /* 7AC0C 8008A40C 8000B68F */  lw         $s6, 0x80($sp)
    /* 7AC10 8008A410 7C00B58F */  lw         $s5, 0x7C($sp)
    /* 7AC14 8008A414 7800B48F */  lw         $s4, 0x78($sp)
    /* 7AC18 8008A418 7400B38F */  lw         $s3, 0x74($sp)
    /* 7AC1C 8008A41C 7000B28F */  lw         $s2, 0x70($sp)
    /* 7AC20 8008A420 6C00B18F */  lw         $s1, 0x6C($sp)
    /* 7AC24 8008A424 6800B08F */  lw         $s0, 0x68($sp)
    /* 7AC28 8008A428 9000BD27 */  addiu      $sp, $sp, 0x90
    /* 7AC2C 8008A42C 0800E003 */  jr         $ra
    /* 7AC30 8008A430 00000000 */   nop
endlabel func_80089F3C
