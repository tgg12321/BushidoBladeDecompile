glabel func_8006A880
    /* 5B080 8006A880 80FFBD27 */  addiu      $sp, $sp, -0x80
    /* 5B084 8006A884 5800B0AF */  sw         $s0, 0x58($sp)
    /* 5B088 8006A888 21808000 */  addu       $s0, $a0, $zero
    /* 5B08C 8006A88C 7C00BFAF */  sw         $ra, 0x7C($sp)
    /* 5B090 8006A890 7800BEAF */  sw         $fp, 0x78($sp)
    /* 5B094 8006A894 7400B7AF */  sw         $s7, 0x74($sp)
    /* 5B098 8006A898 7000B6AF */  sw         $s6, 0x70($sp)
    /* 5B09C 8006A89C 6C00B5AF */  sw         $s5, 0x6C($sp)
    /* 5B0A0 8006A8A0 6800B4AF */  sw         $s4, 0x68($sp)
    /* 5B0A4 8006A8A4 6400B3AF */  sw         $s3, 0x64($sp)
    /* 5B0A8 8006A8A8 6000B2AF */  sw         $s2, 0x60($sp)
    /* 5B0AC 8006A8AC 5C00B1AF */  sw         $s1, 0x5C($sp)
    /* 5B0B0 8006A8B0 0400028E */  lw         $v0, 0x4($s0)
    /* 5B0B4 8006A8B4 2198A000 */  addu       $s3, $a1, $zero
    /* 5B0B8 8006A8B8 1800528C */  lw         $s2, 0x18($v0)
    /* 5B0BC 8006A8BC 0A80163C */  lui        $s6, %hi(D_8009BC08)
    /* 5B0C0 8006A8C0 08BCD68E */  lw         $s6, %lo(D_8009BC08)($s6)
    /* 5B0C4 8006A8C4 0000448E */  lw         $a0, 0x0($s2)
    /* 5B0C8 8006A8C8 21280000 */  addu       $a1, $zero, $zero
    /* 5B0CC 8006A8CC 20B9010C */  jal        func_8006E480
    /* 5B0D0 8006A8D0 1800A4AF */   sw        $a0, 0x18($sp)
    /* 5B0D4 8006A8D4 01000524 */  addiu      $a1, $zero, 0x1
    /* 5B0D8 8006A8D8 21300000 */  addu       $a2, $zero, $zero
    /* 5B0DC 8006A8DC 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5B0E0 8006A8E0 1C00048E */  lw         $a0, 0x1C($s0)
    /* 5B0E4 8006A8E4 92F0010C */  jal        func_8007C248
    /* 5B0E8 8006A8E8 21384000 */   addu      $a3, $v0, $zero
    /* 5B0EC 8006A8EC 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5B0F0 8006A8F0 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5B0F4 8006A8F4 1C00058E */  lw         $a1, 0x1C($s0)
    /* 5B0F8 8006A8F8 2DEA010C */  jal        func_8007A8B4
    /* 5B0FC 8006A8FC 30008424 */   addiu     $a0, $a0, 0x30
    /* 5B100 8006A900 1C00028E */  lw         $v0, 0x1C($s0)
    /* 5B104 8006A904 00000000 */  nop
    /* 5B108 8006A908 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5B10C 8006A90C 1C0002AE */  sw         $v0, 0x1C($s0)
    /* 5B110 8006A910 00006296 */  lhu        $v0, 0x0($s3)
    /* 5B114 8006A914 21880000 */  addu       $s1, $zero, $zero
    /* 5B118 8006A918 4800A2A7 */  sh         $v0, 0x48($sp)
    /* 5B11C 8006A91C 02006396 */  lhu        $v1, 0x2($s3)
    /* 5B120 8006A920 F6000224 */  addiu      $v0, $zero, 0xF6
    /* 5B124 8006A924 4C00A2A7 */  sh         $v0, 0x4C($sp)
    /* 5B128 8006A928 92000224 */  addiu      $v0, $zero, 0x92
    /* 5B12C 8006A92C 4E00A2A7 */  sh         $v0, 0x4E($sp)
    /* 5B130 8006A930 3A006324 */  addiu      $v1, $v1, 0x3A
    /* 5B134 8006A934 4A00A3A7 */  sh         $v1, 0x4A($sp)
    /* 5B138 8006A938 2000048E */  lw         $a0, 0x20($s0)
    /* 5B13C 8006A93C 55F0010C */  jal        func_8007C154
    /* 5B140 8006A940 4800A527 */   addiu     $a1, $sp, 0x48
    /* 5B144 8006A944 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5B148 8006A948 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5B14C 8006A94C 2000058E */  lw         $a1, 0x20($s0)
    /* 5B150 8006A950 2DEA010C */  jal        func_8007A8B4
    /* 5B154 8006A954 30008424 */   addiu     $a0, $a0, 0x30
    /* 5B158 8006A958 2000028E */  lw         $v0, 0x20($s0)
    /* 5B15C 8006A95C 00000000 */  nop
    /* 5B160 8006A960 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5B164 8006A964 200002AE */  sw         $v0, 0x20($s0)
    /* 5B168 8006A968 0100C232 */  andi       $v0, $s6, 0x1
    /* 5B16C 8006A96C 07004014 */  bnez       $v0, .L8006A98C
    /* 5B170 8006A970 21180000 */   addu      $v1, $zero, $zero
    /* 5B174 8006A974 01000424 */  addiu      $a0, $zero, 0x1
  .L8006A978:
    /* 5B178 8006A978 01003126 */  addiu      $s1, $s1, 0x1
    /* 5B17C 8006A97C 04102402 */  sllv       $v0, $a0, $s1
    /* 5B180 8006A980 2410C202 */  and        $v0, $s6, $v0
    /* 5B184 8006A984 FCFF4010 */  beqz       $v0, .L8006A978
    /* 5B188 8006A988 E8FF6324 */   addiu     $v1, $v1, -0x18
  .L8006A98C:
    /* 5B18C 8006A98C 5000A527 */  addiu      $a1, $sp, 0x50
    /* 5B190 8006A990 21880000 */  addu       $s1, $zero, $zero
    /* 5B194 8006A994 08006296 */  lhu        $v0, 0x8($s3)
    /* 5B198 8006A998 30001424 */  addiu      $s4, $zero, 0x30
    /* 5B19C 8006A99C 5000A2A7 */  sh         $v0, 0x50($sp)
    /* 5B1A0 8006A9A0 0A006296 */  lhu        $v0, 0xA($s3)
    /* 5B1A4 8006A9A4 01001724 */  addiu      $s7, $zero, 0x1
    /* 5B1A8 8006A9A8 21104300 */  addu       $v0, $v0, $v1
    /* 5B1AC 8006A9AC 5200A2A7 */  sh         $v0, 0x52($sp)
    /* 5B1B0 8006A9B0 2400048E */  lw         $a0, 0x24($s0)
    /* 5B1B4 8006A9B4 76F0010C */  jal        func_8007C1D8
    /* 5B1B8 8006A9B8 21A84002 */   addu      $s5, $s2, $zero
    /* 5B1BC 8006A9BC 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5B1C0 8006A9C0 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5B1C4 8006A9C4 2400058E */  lw         $a1, 0x24($s0)
    /* 5B1C8 8006A9C8 2DEA010C */  jal        func_8007A8B4
    /* 5B1CC 8006A9CC 30008424 */   addiu     $a0, $a0, 0x30
    /* 5B1D0 8006A9D0 2400028E */  lw         $v0, 0x24($s0)
    /* 5B1D4 8006A9D4 3F001224 */  addiu      $s2, $zero, 0x3F
    /* 5B1D8 8006A9D8 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5B1DC 8006A9DC 240002AE */  sw         $v0, 0x24($s0)
    /* 5B1E0 8006A9E0 0A80163C */  lui        $s6, %hi(D_8009BC04)
    /* 5B1E4 8006A9E4 04BCD68E */  lw         $s6, %lo(D_8009BC04)($s6)
    /* 5B1E8 8006A9E8 0B000224 */  addiu      $v0, $zero, 0xB
    /* 5B1EC 8006A9EC 2C00A2AF */  sw         $v0, 0x2C($sp)
  .L8006A9F0:
    /* 5B1F0 8006A9F0 2C04838F */  lw         $v1, %gp_rel(D_800A34F8)($gp)
    /* 5B1F4 8006A9F4 76000224 */  addiu      $v0, $zero, 0x76
    /* 5B1F8 8006A9F8 3000A2AF */  sw         $v0, 0x30($sp)
    /* 5B1FC 8006A9FC 0F006330 */  andi       $v1, $v1, 0xF
    /* 5B200 8006AA00 21007114 */  bne        $v1, $s1, .L8006AA88
    /* 5B204 8006AA04 4000B7A3 */   sb        $s7, 0x40($sp)
    /* 5B208 8006AA08 4804848F */  lw         $a0, %gp_rel(D_800A3514)($gp)
    /* 5B20C 8006AA0C 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5B210 8006AA10 1F008430 */  andi       $a0, $a0, 0x1F
    /* 5B214 8006AA14 C0210400 */  sll        $a0, $a0, 7
    /* 5B218 8006AA18 C8F7010C */  jal        func_8007DF20
    /* 5B21C 8006AA1C FF018424 */   addiu     $a0, $a0, 0x1FF
    /* 5B220 8006AA20 80190200 */  sll        $v1, $v0, 6
    /* 5B224 8006AA24 23186200 */  subu       $v1, $v1, $v0
    /* 5B228 8006AA28 031B0300 */  sra        $v1, $v1, 12
    /* 5B22C 8006AA2C 4804848F */  lw         $a0, %gp_rel(D_800A3514)($gp)
    /* 5B230 8006AA30 9FFF6324 */  addiu      $v1, $v1, -0x61
    /* 5B234 8006AA34 4100A3A3 */  sb         $v1, 0x41($sp)
    /* 5B238 8006AA38 1F008430 */  andi       $a0, $a0, 0x1F
    /* 5B23C 8006AA3C C0210400 */  sll        $a0, $a0, 7
    /* 5B240 8006AA40 C8F7010C */  jal        func_8007DF20
    /* 5B244 8006AA44 FF018424 */   addiu     $a0, $a0, 0x1FF
    /* 5B248 8006AA48 40180200 */  sll        $v1, $v0, 1
    /* 5B24C 8006AA4C 21186200 */  addu       $v1, $v1, $v0
    /* 5B250 8006AA50 00110300 */  sll        $v0, $v1, 4
    /* 5B254 8006AA54 23104300 */  subu       $v0, $v0, $v1
    /* 5B258 8006AA58 03130200 */  sra        $v0, $v0, 12
    /* 5B25C 8006AA5C 3004838F */  lw         $v1, %gp_rel(D_800A34FC)($gp)
    /* 5B260 8006AA60 83FF4224 */  addiu      $v0, $v0, -0x7D
    /* 5B264 8006AA64 4200A2A3 */  sb         $v0, 0x42($sp)
    /* 5B268 8006AA68 4804828F */  lw         $v0, %gp_rel(D_800A3514)($gp)
    /* 5B26C 8006AA6C 4300B4A3 */  sb         $s4, 0x43($sp)
    /* 5B270 8006AA70 0E006384 */  lh         $v1, 0xE($v1)
    /* 5B274 8006AA74 01004224 */  addiu      $v0, $v0, 0x1
    /* 5B278 8006AA78 480482AF */  sw         $v0, %gp_rel(D_800A3514)($gp)
    /* 5B27C 8006AA7C 21187200 */  addu       $v1, $v1, $s2
    /* 5B280 8006AA80 B9AA0108 */  j          .L8006AAE4
    /* 5B284 8006AA84 3400A3AF */   sw        $v1, 0x34($sp)
  .L8006AA88:
    /* 5B288 8006AA88 06103602 */  srlv       $v0, $s6, $s1
    /* 5B28C 8006AA8C 01004230 */  andi       $v0, $v0, 0x1
    /* 5B290 8006AA90 0F004010 */  beqz       $v0, .L8006AAD0
    /* 5B294 8006AA94 80000224 */   addiu     $v0, $zero, 0x80
    /* 5B298 8006AA98 5804838F */  lw         $v1, %gp_rel(D_800A3524)($gp)
    /* 5B29C 8006AA9C 4100A2A3 */  sb         $v0, 0x41($sp)
    /* 5B2A0 8006AAA0 6C000224 */  addiu      $v0, $zero, 0x6C
    /* 5B2A4 8006AAA4 4200A2A3 */  sb         $v0, 0x42($sp)
    /* 5B2A8 8006AAA8 4300B4A3 */  sb         $s4, 0x43($sp)
    /* 5B2AC 8006AAAC 2800B7AF */  sw         $s7, 0x28($sp)
    /* 5B2B0 8006AAB0 3400B2AF */  sw         $s2, 0x34($sp)
    /* 5B2B4 8006AAB4 2000628C */  lw         $v0, 0x20($v1)
    /* 5B2B8 8006AAB8 00000000 */  nop
    /* 5B2BC 8006AABC 01004230 */  andi       $v0, $v0, 0x1
    /* 5B2C0 8006AAC0 08004010 */  beqz       $v0, .L8006AAE4
    /* 5B2C4 8006AAC4 00000000 */   nop
    /* 5B2C8 8006AAC8 B9AA0108 */  j          .L8006AAE4
    /* 5B2CC 8006AACC 4000A0A3 */   sb        $zero, 0x40($sp)
  .L8006AAD0:
    /* 5B2D0 8006AAD0 4100B4A3 */  sb         $s4, 0x41($sp)
    /* 5B2D4 8006AAD4 4200B4A3 */  sb         $s4, 0x42($sp)
    /* 5B2D8 8006AAD8 4300B4A3 */  sb         $s4, 0x43($sp)
    /* 5B2DC 8006AADC 2800B7AF */  sw         $s7, 0x28($sp)
    /* 5B2E0 8006AAE0 3400B2AF */  sw         $s2, 0x34($sp)
  .L8006AAE4:
    /* 5B2E4 8006AAE4 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5B2E8 8006AAE8 0000A28E */  lw         $v0, 0x0($s5)
    /* 5B2EC 8006AAEC 0400B526 */  addiu      $s5, $s5, 0x4
    /* 5B2F0 8006AAF0 0C004524 */  addiu      $a1, $v0, 0xC
    /* 5B2F4 8006AAF4 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5B2F8 8006AAF8 1C00A5AF */  sw         $a1, 0x1C($sp)
    /* 5B2FC 8006AAFC 1400028E */  lw         $v0, 0x14($s0)
    /* 5B300 8006AB00 18005226 */  addiu      $s2, $s2, 0x18
    /* 5B304 8006AB04 4BCD010C */  jal        func_8007352C
    /* 5B308 8006AB08 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5B30C 8006AB0C 140002AE */  sw         $v0, 0x14($s0)
    /* 5B310 8006AB10 1800A48F */  lw         $a0, 0x18($sp)
    /* 5B314 8006AB14 20B9010C */  jal        func_8006E480
    /* 5B318 8006AB18 21280000 */   addu      $a1, $zero, $zero
    /* 5B31C 8006AB1C 01000524 */  addiu      $a1, $zero, 0x1
    /* 5B320 8006AB20 21300000 */  addu       $a2, $zero, $zero
    /* 5B324 8006AB24 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5B328 8006AB28 1C00048E */  lw         $a0, 0x1C($s0)
    /* 5B32C 8006AB2C 92F0010C */  jal        func_8007C248
    /* 5B330 8006AB30 21384000 */   addu      $a3, $v0, $zero
    /* 5B334 8006AB34 1C00058E */  lw         $a1, 0x1C($s0)
    /* 5B338 8006AB38 2C00A48F */  lw         $a0, 0x2C($sp)
    /* 5B33C 8006AB3C 0A80023C */  lui        $v0, %hi(D_800A374C)
    /* 5B340 8006AB40 4C37428C */  lw         $v0, %lo(D_800A374C)($v0)
    /* 5B344 8006AB44 80200400 */  sll        $a0, $a0, 2
    /* 5B348 8006AB48 2DEA010C */  jal        func_8007A8B4
    /* 5B34C 8006AB4C 21204400 */   addu      $a0, $v0, $a0
    /* 5B350 8006AB50 21200002 */  addu       $a0, $s0, $zero
    /* 5B354 8006AB54 1800A527 */  addiu      $a1, $sp, 0x18
    /* 5B358 8006AB58 21302002 */  addu       $a2, $s1, $zero
    /* 5B35C 8006AB5C 01003126 */  addiu      $s1, $s1, 0x1
    /* 5B360 8006AB60 1C00028E */  lw         $v0, 0x1C($s0)
    /* 5B364 8006AB64 01001E24 */  addiu      $fp, $zero, 0x1
    /* 5B368 8006AB68 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5B36C 8006AB6C 1C0002AE */  sw         $v0, 0x1C($s0)
    /* 5B370 8006AB70 59A9010C */  jal        func_8006A564
    /* 5B374 8006AB74 4000BEA3 */   sb        $fp, 0x40($sp)
    /* 5B378 8006AB78 0700222A */  slti       $v0, $s1, 0x7
    /* 5B37C 8006AB7C 9CFF4014 */  bnez       $v0, .L8006A9F0
    /* 5B380 8006AB80 41000324 */   addiu     $v1, $zero, 0x41
    /* 5B384 8006AB84 0400028E */  lw         $v0, 0x4($s0)
    /* 5B388 8006AB88 00000000 */  nop
    /* 5B38C 8006AB8C 4000528C */  lw         $s2, 0x40($v0)
    /* 5B390 8006AB90 0A000224 */  addiu      $v0, $zero, 0xA
    /* 5B394 8006AB94 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 5B398 8006AB98 2C04828F */  lw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5B39C 8006AB9C 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5B3A0 8006ABA0 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5B3A4 8006ABA4 3000A0AF */  sw         $zero, 0x30($sp)
    /* 5B3A8 8006ABA8 3400A3AF */  sw         $v1, 0x34($sp)
    /* 5B3AC 8006ABAC 0F004230 */  andi       $v0, $v0, 0xF
    /* 5B3B0 8006ABB0 80100200 */  sll        $v0, $v0, 2
    /* 5B3B4 8006ABB4 21105200 */  addu       $v0, $v0, $s2
    /* 5B3B8 8006ABB8 0000428C */  lw         $v0, 0x0($v0)
    /* 5B3BC 8006ABBC 00000000 */  nop
    /* 5B3C0 8006ABC0 0C004524 */  addiu      $a1, $v0, 0xC
    /* 5B3C4 8006ABC4 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5B3C8 8006ABC8 1C00A5AF */  sw         $a1, 0x1C($sp)
    /* 5B3CC 8006ABCC 1400028E */  lw         $v0, 0x14($s0)
    /* 5B3D0 8006ABD0 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5B3D4 8006ABD4 4BCD010C */  jal        func_8007352C
    /* 5B3D8 8006ABD8 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5B3DC 8006ABDC 140002AE */  sw         $v0, 0x14($s0)
    /* 5B3E0 8006ABE0 1800A48F */  lw         $a0, 0x18($sp)
    /* 5B3E4 8006ABE4 20B9010C */  jal        func_8006E480
    /* 5B3E8 8006ABE8 21280000 */   addu      $a1, $zero, $zero
    /* 5B3EC 8006ABEC 01000524 */  addiu      $a1, $zero, 0x1
    /* 5B3F0 8006ABF0 21300000 */  addu       $a2, $zero, $zero
    /* 5B3F4 8006ABF4 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5B3F8 8006ABF8 1C00048E */  lw         $a0, 0x1C($s0)
    /* 5B3FC 8006ABFC 92F0010C */  jal        func_8007C248
    /* 5B400 8006AC00 21384000 */   addu      $a3, $v0, $zero
    /* 5B404 8006AC04 1C00058E */  lw         $a1, 0x1C($s0)
    /* 5B408 8006AC08 2C00A48F */  lw         $a0, 0x2C($sp)
    /* 5B40C 8006AC0C 0A80023C */  lui        $v0, %hi(D_800A374C)
    /* 5B410 8006AC10 4C37428C */  lw         $v0, %lo(D_800A374C)($v0)
    /* 5B414 8006AC14 80200400 */  sll        $a0, $a0, 2
    /* 5B418 8006AC18 2DEA010C */  jal        func_8007A8B4
    /* 5B41C 8006AC1C 21204400 */   addu      $a0, $v0, $a0
    /* 5B420 8006AC20 1C00028E */  lw         $v0, 0x1C($s0)
    /* 5B424 8006AC24 0400038E */  lw         $v1, 0x4($s0)
    /* 5B428 8006AC28 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5B42C 8006AC2C 1C0002AE */  sw         $v0, 0x1C($s0)
    /* 5B430 8006AC30 00006296 */  lhu        $v0, 0x0($s3)
    /* 5B434 8006AC34 1800728C */  lw         $s2, 0x18($v1)
    /* 5B438 8006AC38 4800A2A7 */  sh         $v0, 0x48($sp)
    /* 5B43C 8006AC3C 02006296 */  lhu        $v0, 0x2($s3)
    /* 5B440 8006AC40 00000000 */  nop
    /* 5B444 8006AC44 4A00A2A7 */  sh         $v0, 0x4A($sp)
    /* 5B448 8006AC48 04006296 */  lhu        $v0, 0x4($s3)
    /* 5B44C 8006AC4C 00000000 */  nop
    /* 5B450 8006AC50 4C00A2A7 */  sh         $v0, 0x4C($sp)
    /* 5B454 8006AC54 06006296 */  lhu        $v0, 0x6($s3)
    /* 5B458 8006AC58 00000000 */  nop
    /* 5B45C 8006AC5C 4E00A2A7 */  sh         $v0, 0x4E($sp)
    /* 5B460 8006AC60 2000048E */  lw         $a0, 0x20($s0)
    /* 5B464 8006AC64 55F0010C */  jal        func_8007C154
    /* 5B468 8006AC68 4800A527 */   addiu     $a1, $sp, 0x48
    /* 5B46C 8006AC6C 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5B470 8006AC70 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5B474 8006AC74 2000058E */  lw         $a1, 0x20($s0)
    /* 5B478 8006AC78 2DEA010C */  jal        func_8007A8B4
    /* 5B47C 8006AC7C 28008424 */   addiu     $a0, $a0, 0x28
    /* 5B480 8006AC80 2000028E */  lw         $v0, 0x20($s0)
    /* 5B484 8006AC84 00000000 */  nop
    /* 5B488 8006AC88 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5B48C 8006AC8C 200002AE */  sw         $v0, 0x20($s0)
    /* 5B490 8006AC90 08006296 */  lhu        $v0, 0x8($s3)
    /* 5B494 8006AC94 00000000 */  nop
    /* 5B498 8006AC98 5000A2A7 */  sh         $v0, 0x50($sp)
    /* 5B49C 8006AC9C 0A006296 */  lhu        $v0, 0xA($s3)
    /* 5B4A0 8006ACA0 00000000 */  nop
    /* 5B4A4 8006ACA4 5200A2A7 */  sh         $v0, 0x52($sp)
    /* 5B4A8 8006ACA8 2400048E */  lw         $a0, 0x24($s0)
    /* 5B4AC 8006ACAC 76F0010C */  jal        func_8007C1D8
    /* 5B4B0 8006ACB0 5000A527 */   addiu     $a1, $sp, 0x50
    /* 5B4B4 8006ACB4 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5B4B8 8006ACB8 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5B4BC 8006ACBC 2400058E */  lw         $a1, 0x24($s0)
    /* 5B4C0 8006ACC0 2DEA010C */  jal        func_8007A8B4
    /* 5B4C4 8006ACC4 28008424 */   addiu     $a0, $a0, 0x28
    /* 5B4C8 8006ACC8 2400028E */  lw         $v0, 0x24($s0)
    /* 5B4CC 8006ACCC 00000000 */  nop
    /* 5B4D0 8006ACD0 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5B4D4 8006ACD4 240002AE */  sw         $v0, 0x24($s0)
    /* 5B4D8 8006ACD8 2C04838F */  lw         $v1, %gp_rel(D_800A34F8)($gp)
    /* 5B4DC 8006ACDC 76000224 */  addiu      $v0, $zero, 0x76
    /* 5B4E0 8006ACE0 3000A2AF */  sw         $v0, 0x30($sp)
    /* 5B4E4 8006ACE4 07000224 */  addiu      $v0, $zero, 0x7
    /* 5B4E8 8006ACE8 0F006330 */  andi       $v1, $v1, 0xF
    /* 5B4EC 8006ACEC 22006214 */  bne        $v1, $v0, .L8006AD78
    /* 5B4F0 8006ACF0 4000BEA3 */   sb        $fp, 0x40($sp)
    /* 5B4F4 8006ACF4 4804848F */  lw         $a0, %gp_rel(D_800A3514)($gp)
    /* 5B4F8 8006ACF8 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5B4FC 8006ACFC 1F008430 */  andi       $a0, $a0, 0x1F
    /* 5B500 8006AD00 C0210400 */  sll        $a0, $a0, 7
    /* 5B504 8006AD04 C8F7010C */  jal        func_8007DF20
    /* 5B508 8006AD08 FF018424 */   addiu     $a0, $a0, 0x1FF
    /* 5B50C 8006AD0C 80190200 */  sll        $v1, $v0, 6
    /* 5B510 8006AD10 23186200 */  subu       $v1, $v1, $v0
    /* 5B514 8006AD14 031B0300 */  sra        $v1, $v1, 12
    /* 5B518 8006AD18 4804848F */  lw         $a0, %gp_rel(D_800A3514)($gp)
    /* 5B51C 8006AD1C 9FFF6324 */  addiu      $v1, $v1, -0x61
    /* 5B520 8006AD20 4100A3A3 */  sb         $v1, 0x41($sp)
    /* 5B524 8006AD24 1F008430 */  andi       $a0, $a0, 0x1F
    /* 5B528 8006AD28 C0210400 */  sll        $a0, $a0, 7
    /* 5B52C 8006AD2C C8F7010C */  jal        func_8007DF20
    /* 5B530 8006AD30 FF018424 */   addiu     $a0, $a0, 0x1FF
    /* 5B534 8006AD34 40180200 */  sll        $v1, $v0, 1
    /* 5B538 8006AD38 21186200 */  addu       $v1, $v1, $v0
    /* 5B53C 8006AD3C 00110300 */  sll        $v0, $v1, 4
    /* 5B540 8006AD40 23104300 */  subu       $v0, $v0, $v1
    /* 5B544 8006AD44 03130200 */  sra        $v0, $v0, 12
    /* 5B548 8006AD48 3004838F */  lw         $v1, %gp_rel(D_800A34FC)($gp)
    /* 5B54C 8006AD4C 83FF4224 */  addiu      $v0, $v0, -0x7D
    /* 5B550 8006AD50 4200A2A3 */  sb         $v0, 0x42($sp)
    /* 5B554 8006AD54 30000224 */  addiu      $v0, $zero, 0x30
    /* 5B558 8006AD58 4300A2A3 */  sb         $v0, 0x43($sp)
    /* 5B55C 8006AD5C 4804828F */  lw         $v0, %gp_rel(D_800A3514)($gp)
    /* 5B560 8006AD60 0E006384 */  lh         $v1, 0xE($v1)
    /* 5B564 8006AD64 01004224 */  addiu      $v0, $v0, 0x1
    /* 5B568 8006AD68 480482AF */  sw         $v0, %gp_rel(D_800A3514)($gp)
    /* 5B56C 8006AD6C CF006324 */  addiu      $v1, $v1, 0xCF
    /* 5B570 8006AD70 7BAB0108 */  j          .L8006ADEC
    /* 5B574 8006AD74 3400A3AF */   sw        $v1, 0x34($sp)
  .L8006AD78:
    /* 5B578 8006AD78 C2111600 */  srl        $v0, $s6, 7
    /* 5B57C 8006AD7C 01004230 */  andi       $v0, $v0, 0x1
    /* 5B580 8006AD80 12004010 */  beqz       $v0, .L8006ADCC
    /* 5B584 8006AD84 80000224 */   addiu     $v0, $zero, 0x80
    /* 5B588 8006AD88 5804838F */  lw         $v1, %gp_rel(D_800A3524)($gp)
    /* 5B58C 8006AD8C 4100A2A3 */  sb         $v0, 0x41($sp)
    /* 5B590 8006AD90 6C000224 */  addiu      $v0, $zero, 0x6C
    /* 5B594 8006AD94 4200A2A3 */  sb         $v0, 0x42($sp)
    /* 5B598 8006AD98 30000224 */  addiu      $v0, $zero, 0x30
    /* 5B59C 8006AD9C 4300A2A3 */  sb         $v0, 0x43($sp)
    /* 5B5A0 8006ADA0 01000224 */  addiu      $v0, $zero, 0x1
    /* 5B5A4 8006ADA4 2800A2AF */  sw         $v0, 0x28($sp)
    /* 5B5A8 8006ADA8 CF000224 */  addiu      $v0, $zero, 0xCF
    /* 5B5AC 8006ADAC 3400A2AF */  sw         $v0, 0x34($sp)
    /* 5B5B0 8006ADB0 2000628C */  lw         $v0, 0x20($v1)
    /* 5B5B4 8006ADB4 00000000 */  nop
    /* 5B5B8 8006ADB8 01004230 */  andi       $v0, $v0, 0x1
    /* 5B5BC 8006ADBC 0C004010 */  beqz       $v0, .L8006ADF0
    /* 5B5C0 8006ADC0 09000224 */   addiu     $v0, $zero, 0x9
    /* 5B5C4 8006ADC4 7CAB0108 */  j          .L8006ADF0
    /* 5B5C8 8006ADC8 4000A0A3 */   sb        $zero, 0x40($sp)
  .L8006ADCC:
    /* 5B5CC 8006ADCC 30000224 */  addiu      $v0, $zero, 0x30
    /* 5B5D0 8006ADD0 4100A2A3 */  sb         $v0, 0x41($sp)
    /* 5B5D4 8006ADD4 4200A2A3 */  sb         $v0, 0x42($sp)
    /* 5B5D8 8006ADD8 4300A2A3 */  sb         $v0, 0x43($sp)
    /* 5B5DC 8006ADDC 01000224 */  addiu      $v0, $zero, 0x1
    /* 5B5E0 8006ADE0 2800A2AF */  sw         $v0, 0x28($sp)
    /* 5B5E4 8006ADE4 CF000224 */  addiu      $v0, $zero, 0xCF
    /* 5B5E8 8006ADE8 3400A2AF */  sw         $v0, 0x34($sp)
  .L8006ADEC:
    /* 5B5EC 8006ADEC 09000224 */  addiu      $v0, $zero, 0x9
  .L8006ADF0:
    /* 5B5F0 8006ADF0 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 5B5F4 8006ADF4 1C00428E */  lw         $v0, 0x1C($s2)
    /* 5B5F8 8006ADF8 00000000 */  nop
    /* 5B5FC 8006ADFC 0C004524 */  addiu      $a1, $v0, 0xC
    /* 5B600 8006AE00 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5B604 8006AE04 1C00A5AF */  sw         $a1, 0x1C($sp)
    /* 5B608 8006AE08 1400028E */  lw         $v0, 0x14($s0)
    /* 5B60C 8006AE0C 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5B610 8006AE10 4BCD010C */  jal        func_8007352C
    /* 5B614 8006AE14 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5B618 8006AE18 140002AE */  sw         $v0, 0x14($s0)
    /* 5B61C 8006AE1C 1800A48F */  lw         $a0, 0x18($sp)
    /* 5B620 8006AE20 20B9010C */  jal        func_8006E480
    /* 5B624 8006AE24 21280000 */   addu      $a1, $zero, $zero
    /* 5B628 8006AE28 01000524 */  addiu      $a1, $zero, 0x1
    /* 5B62C 8006AE2C 21300000 */  addu       $a2, $zero, $zero
    /* 5B630 8006AE30 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5B634 8006AE34 1C00048E */  lw         $a0, 0x1C($s0)
    /* 5B638 8006AE38 92F0010C */  jal        func_8007C248
    /* 5B63C 8006AE3C 21384000 */   addu      $a3, $v0, $zero
    /* 5B640 8006AE40 1C00058E */  lw         $a1, 0x1C($s0)
    /* 5B644 8006AE44 2C00A48F */  lw         $a0, 0x2C($sp)
    /* 5B648 8006AE48 0A80023C */  lui        $v0, %hi(D_800A374C)
    /* 5B64C 8006AE4C 4C37428C */  lw         $v0, %lo(D_800A374C)($v0)
    /* 5B650 8006AE50 80200400 */  sll        $a0, $a0, 2
    /* 5B654 8006AE54 2DEA010C */  jal        func_8007A8B4
    /* 5B658 8006AE58 21204400 */   addu      $a0, $v0, $a0
    /* 5B65C 8006AE5C 21200002 */  addu       $a0, $s0, $zero
    /* 5B660 8006AE60 1800A527 */  addiu      $a1, $sp, 0x18
    /* 5B664 8006AE64 1C00028E */  lw         $v0, 0x1C($s0)
    /* 5B668 8006AE68 07000624 */  addiu      $a2, $zero, 0x7
    /* 5B66C 8006AE6C 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5B670 8006AE70 1C0002AE */  sw         $v0, 0x1C($s0)
    /* 5B674 8006AE74 01000224 */  addiu      $v0, $zero, 0x1
    /* 5B678 8006AE78 59A9010C */  jal        func_8006A564
    /* 5B67C 8006AE7C 4000A2A3 */   sb        $v0, 0x40($sp)
    /* 5B680 8006AE80 21200002 */  addu       $a0, $s0, $zero
    /* 5B684 8006AE84 25A9010C */  jal        func_8006A494
    /* 5B688 8006AE88 1800A527 */   addiu     $a1, $sp, 0x18
    /* 5B68C 8006AE8C 1800A48F */  lw         $a0, 0x18($sp)
    /* 5B690 8006AE90 21280000 */  addu       $a1, $zero, $zero
    /* 5B694 8006AE94 20B9010C */  jal        func_8006E480
    /* 5B698 8006AE98 4000A0A3 */   sb        $zero, 0x40($sp)
    /* 5B69C 8006AE9C 01000524 */  addiu      $a1, $zero, 0x1
    /* 5B6A0 8006AEA0 21300000 */  addu       $a2, $zero, $zero
    /* 5B6A4 8006AEA4 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5B6A8 8006AEA8 1C00048E */  lw         $a0, 0x1C($s0)
    /* 5B6AC 8006AEAC 92F0010C */  jal        func_8007C248
    /* 5B6B0 8006AEB0 21384000 */   addu      $a3, $v0, $zero
    /* 5B6B4 8006AEB4 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5B6B8 8006AEB8 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5B6BC 8006AEBC 1C00058E */  lw         $a1, 0x1C($s0)
    /* 5B6C0 8006AEC0 2DEA010C */  jal        func_8007A8B4
    /* 5B6C4 8006AEC4 04008424 */   addiu     $a0, $a0, 0x4
    /* 5B6C8 8006AEC8 1C00028E */  lw         $v0, 0x1C($s0)
    /* 5B6CC 8006AECC 00000000 */  nop
    /* 5B6D0 8006AED0 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5B6D4 8006AED4 1C0002AE */  sw         $v0, 0x1C($s0)
    /* 5B6D8 8006AED8 1800A48F */  lw         $a0, 0x18($sp)
    /* 5B6DC 8006AEDC 20B9010C */  jal        func_8006E480
    /* 5B6E0 8006AEE0 21280000 */   addu      $a1, $zero, $zero
    /* 5B6E4 8006AEE4 01000524 */  addiu      $a1, $zero, 0x1
    /* 5B6E8 8006AEE8 21300000 */  addu       $a2, $zero, $zero
    /* 5B6EC 8006AEEC 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5B6F0 8006AEF0 1C00048E */  lw         $a0, 0x1C($s0)
    /* 5B6F4 8006AEF4 92F0010C */  jal        func_8007C248
    /* 5B6F8 8006AEF8 21384000 */   addu      $a3, $v0, $zero
    /* 5B6FC 8006AEFC 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5B700 8006AF00 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5B704 8006AF04 1C00058E */  lw         $a1, 0x1C($s0)
    /* 5B708 8006AF08 2DEA010C */  jal        func_8007A8B4
    /* 5B70C 8006AF0C 04008424 */   addiu     $a0, $a0, 0x4
    /* 5B710 8006AF10 1C00028E */  lw         $v0, 0x1C($s0)
    /* 5B714 8006AF14 0400038E */  lw         $v1, 0x4($s0)
    /* 5B718 8006AF18 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5B71C 8006AF1C 1C0002AE */  sw         $v0, 0x1C($s0)
    /* 5B720 8006AF20 2C04828F */  lw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5B724 8006AF24 2400728C */  lw         $s2, 0x24($v1)
    /* 5B728 8006AF28 0F004230 */  andi       $v0, $v0, 0xF
    /* 5B72C 8006AF2C 80100200 */  sll        $v0, $v0, 2
    /* 5B730 8006AF30 21105200 */  addu       $v0, $v0, $s2
    /* 5B734 8006AF34 2000438C */  lw         $v1, 0x20($v0)
    /* 5B738 8006AF38 19000224 */  addiu      $v0, $zero, 0x19
    /* 5B73C 8006AF3C 3000A0AF */  sw         $zero, 0x30($sp)
    /* 5B740 8006AF40 3400A2AF */  sw         $v0, 0x34($sp)
    /* 5B744 8006AF44 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5B748 8006AF48 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5B74C 8006AF4C 2C00A0AF */  sw         $zero, 0x2C($sp)
    /* 5B750 8006AF50 0C006524 */  addiu      $a1, $v1, 0xC
    /* 5B754 8006AF54 1800A3AF */  sw         $v1, 0x18($sp)
    /* 5B758 8006AF58 1C00A5AF */  sw         $a1, 0x1C($sp)
    /* 5B75C 8006AF5C 1400028E */  lw         $v0, 0x14($s0)
    /* 5B760 8006AF60 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5B764 8006AF64 4BCD010C */  jal        func_8007352C
    /* 5B768 8006AF68 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5B76C 8006AF6C 140002AE */  sw         $v0, 0x14($s0)
    /* 5B770 8006AF70 1800A48F */  lw         $a0, 0x18($sp)
    /* 5B774 8006AF74 20B9010C */  jal        func_8006E480
    /* 5B778 8006AF78 21280000 */   addu      $a1, $zero, $zero
    /* 5B77C 8006AF7C 01000524 */  addiu      $a1, $zero, 0x1
    /* 5B780 8006AF80 21300000 */  addu       $a2, $zero, $zero
    /* 5B784 8006AF84 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5B788 8006AF88 1C00048E */  lw         $a0, 0x1C($s0)
    /* 5B78C 8006AF8C 92F0010C */  jal        func_8007C248
    /* 5B790 8006AF90 21384000 */   addu      $a3, $v0, $zero
    /* 5B794 8006AF94 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5B798 8006AF98 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5B79C 8006AF9C 1C00058E */  lw         $a1, 0x1C($s0)
    /* 5B7A0 8006AFA0 2DEA010C */  jal        func_8007A8B4
    /* 5B7A4 8006AFA4 00000000 */   nop
    /* 5B7A8 8006AFA8 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5B7AC 8006AFAC 1C00028E */  lw         $v0, 0x1C($s0)
    /* 5B7B0 8006AFB0 0400038E */  lw         $v1, 0x4($s0)
    /* 5B7B4 8006AFB4 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5B7B8 8006AFB8 1C0002AE */  sw         $v0, 0x1C($s0)
    /* 5B7BC 8006AFBC 2C04828F */  lw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5B7C0 8006AFC0 2400728C */  lw         $s2, 0x24($v1)
    /* 5B7C4 8006AFC4 0F004230 */  andi       $v0, $v0, 0xF
    /* 5B7C8 8006AFC8 80100200 */  sll        $v0, $v0, 2
    /* 5B7CC 8006AFCC 21105200 */  addu       $v0, $v0, $s2
    /* 5B7D0 8006AFD0 0000438C */  lw         $v1, 0x0($v0)
    /* 5B7D4 8006AFD4 00020224 */  addiu      $v0, $zero, 0x200
    /* 5B7D8 8006AFD8 3800A2AF */  sw         $v0, 0x38($sp)
    /* 5B7DC 8006AFDC 00010224 */  addiu      $v0, $zero, 0x100
    /* 5B7E0 8006AFE0 3000A0AF */  sw         $zero, 0x30($sp)
    /* 5B7E4 8006AFE4 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5B7E8 8006AFE8 3C00A2AF */  sw         $v0, 0x3C($sp)
    /* 5B7EC 8006AFEC 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5B7F0 8006AFF0 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5B7F4 8006AFF4 2C00A0AF */  sw         $zero, 0x2C($sp)
    /* 5B7F8 8006AFF8 0C006524 */  addiu      $a1, $v1, 0xC
    /* 5B7FC 8006AFFC 1800A3AF */  sw         $v1, 0x18($sp)
    /* 5B800 8006B000 1C00A5AF */  sw         $a1, 0x1C($sp)
    /* 5B804 8006B004 0800028E */  lw         $v0, 0x8($s0)
    /* 5B808 8006B008 21280000 */  addu       $a1, $zero, $zero
    /* 5B80C 8006B00C CACD010C */  jal        func_80073728
    /* 5B810 8006B010 2400A2AF */   sw        $v0, 0x24($sp)
    /* 5B814 8006B014 1800048E */  lw         $a0, 0x18($s0)
    /* 5B818 8006B018 B4EA010C */  jal        func_8007AAD0
    /* 5B81C 8006B01C 080002AE */   sw        $v0, 0x8($s0)
    /* 5B820 8006B020 1800028E */  lw         $v0, 0x18($s0)
    /* 5B824 8006B024 00000000 */  nop
    /* 5B828 8006B028 040040A0 */  sb         $zero, 0x4($v0)
    /* 5B82C 8006B02C 1800028E */  lw         $v0, 0x18($s0)
    /* 5B830 8006B030 00000000 */  nop
    /* 5B834 8006B034 050040A0 */  sb         $zero, 0x5($v0)
    /* 5B838 8006B038 1800028E */  lw         $v0, 0x18($s0)
    /* 5B83C 8006B03C 00000000 */  nop
    /* 5B840 8006B040 060040A0 */  sb         $zero, 0x6($v0)
    /* 5B844 8006B044 1800038E */  lw         $v1, 0x18($s0)
    /* 5B848 8006B048 42010224 */  addiu      $v0, $zero, 0x142
    /* 5B84C 8006B04C 080062A4 */  sh         $v0, 0x8($v1)
    /* 5B850 8006B050 1800038E */  lw         $v1, 0x18($s0)
    /* 5B854 8006B054 51000224 */  addiu      $v0, $zero, 0x51
    /* 5B858 8006B058 0A0062A4 */  sh         $v0, 0xA($v1)
    /* 5B85C 8006B05C 1800038E */  lw         $v1, 0x18($s0)
    /* 5B860 8006B060 00010224 */  addiu      $v0, $zero, 0x100
    /* 5B864 8006B064 0C0062A4 */  sh         $v0, 0xC($v1)
    /* 5B868 8006B068 1800038E */  lw         $v1, 0x18($s0)
    /* 5B86C 8006B06C 59000224 */  addiu      $v0, $zero, 0x59
    /* 5B870 8006B070 0E0062A4 */  sh         $v0, 0xE($v1)
    /* 5B874 8006B074 1800048E */  lw         $a0, 0x18($s0)
    /* 5B878 8006B078 5AEA010C */  jal        func_8007A968
    /* 5B87C 8006B07C 21280000 */   addu      $a1, $zero, $zero
    /* 5B880 8006B080 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5B884 8006B084 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5B888 8006B088 1800058E */  lw         $a1, 0x18($s0)
    /* 5B88C 8006B08C 2DEA010C */  jal        func_8007A8B4
    /* 5B890 8006B090 04008424 */   addiu     $a0, $a0, 0x4
    /* 5B894 8006B094 1800028E */  lw         $v0, 0x18($s0)
    /* 5B898 8006B098 00000000 */  nop
    /* 5B89C 8006B09C 10004224 */  addiu      $v0, $v0, 0x10
    /* 5B8A0 8006B0A0 180002AE */  sw         $v0, 0x18($s0)
    /* 5B8A4 8006B0A4 1800A48F */  lw         $a0, 0x18($sp)
    /* 5B8A8 8006B0A8 20B9010C */  jal        func_8006E480
    /* 5B8AC 8006B0AC 21280000 */   addu      $a1, $zero, $zero
    /* 5B8B0 8006B0B0 01000524 */  addiu      $a1, $zero, 0x1
    /* 5B8B4 8006B0B4 21300000 */  addu       $a2, $zero, $zero
    /* 5B8B8 8006B0B8 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5B8BC 8006B0BC 1C00048E */  lw         $a0, 0x1C($s0)
    /* 5B8C0 8006B0C0 92F0010C */  jal        func_8007C248
    /* 5B8C4 8006B0C4 21384000 */   addu      $a3, $v0, $zero
    /* 5B8C8 8006B0C8 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5B8CC 8006B0CC 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5B8D0 8006B0D0 1C00058E */  lw         $a1, 0x1C($s0)
    /* 5B8D4 8006B0D4 2DEA010C */  jal        func_8007A8B4
    /* 5B8D8 8006B0D8 00000000 */   nop
    /* 5B8DC 8006B0DC 1C00028E */  lw         $v0, 0x1C($s0)
    /* 5B8E0 8006B0E0 00000000 */  nop
    /* 5B8E4 8006B0E4 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5B8E8 8006B0E8 1C0002AE */  sw         $v0, 0x1C($s0)
    /* 5B8EC 8006B0EC 7C00BF8F */  lw         $ra, 0x7C($sp)
    /* 5B8F0 8006B0F0 7800BE8F */  lw         $fp, 0x78($sp)
    /* 5B8F4 8006B0F4 7400B78F */  lw         $s7, 0x74($sp)
    /* 5B8F8 8006B0F8 7000B68F */  lw         $s6, 0x70($sp)
    /* 5B8FC 8006B0FC 6C00B58F */  lw         $s5, 0x6C($sp)
    /* 5B900 8006B100 6800B48F */  lw         $s4, 0x68($sp)
    /* 5B904 8006B104 6400B38F */  lw         $s3, 0x64($sp)
    /* 5B908 8006B108 6000B28F */  lw         $s2, 0x60($sp)
    /* 5B90C 8006B10C 5C00B18F */  lw         $s1, 0x5C($sp)
    /* 5B910 8006B110 5800B08F */  lw         $s0, 0x58($sp)
    /* 5B914 8006B114 8000BD27 */  addiu      $sp, $sp, 0x80
    /* 5B918 8006B118 0800E003 */  jr         $ra
    /* 5B91C 8006B11C 00000000 */   nop
endlabel func_8006A880
