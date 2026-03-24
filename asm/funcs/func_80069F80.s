glabel func_80069F80
    /* 5A780 80069F80 90FFBD27 */  addiu      $sp, $sp, -0x70
    /* 5A784 80069F84 5800B0AF */  sw         $s0, 0x58($sp)
    /* 5A788 80069F88 21808000 */  addu       $s0, $a0, $zero
    /* 5A78C 80069F8C 5C00B1AF */  sw         $s1, 0x5C($sp)
    /* 5A790 80069F90 2188A000 */  addu       $s1, $a1, $zero
    /* 5A794 80069F94 02002232 */  andi       $v0, $s1, 0x2
    /* 5A798 80069F98 6800BFAF */  sw         $ra, 0x68($sp)
    /* 5A79C 80069F9C 6400B3AF */  sw         $s3, 0x64($sp)
    /* 5A7A0 80069FA0 77004010 */  beqz       $v0, .L8006A180
    /* 5A7A4 80069FA4 6000B2AF */   sw        $s2, 0x60($sp)
    /* 5A7A8 80069FA8 0400028E */  lw         $v0, 0x4($s0)
    /* 5A7AC 80069FAC 00000000 */  nop
    /* 5A7B0 80069FB0 1C00528C */  lw         $s2, 0x1C($v0)
    /* 5A7B4 80069FB4 00000000 */  nop
    /* 5A7B8 80069FB8 0000428E */  lw         $v0, 0x0($s2)
    /* 5A7BC 80069FBC 00000000 */  nop
    /* 5A7C0 80069FC0 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5A7C4 80069FC4 01002232 */  andi       $v0, $s1, 0x1
    /* 5A7C8 80069FC8 05004010 */  beqz       $v0, .L80069FE0
    /* 5A7CC 80069FCC 9C000224 */   addiu     $v0, $zero, 0x9C
    /* 5A7D0 80069FD0 3000A2AF */  sw         $v0, 0x30($sp)
    /* 5A7D4 80069FD4 01000224 */  addiu      $v0, $zero, 0x1
    /* 5A7D8 80069FD8 FBA70108 */  j          .L80069FEC
    /* 5A7DC 80069FDC 4000A2A3 */   sb        $v0, 0x40($sp)
  .L80069FE0:
    /* 5A7E0 80069FE0 4E000224 */  addiu      $v0, $zero, 0x4E
    /* 5A7E4 80069FE4 3000A2AF */  sw         $v0, 0x30($sp)
    /* 5A7E8 80069FE8 4000A0A3 */  sb         $zero, 0x40($sp)
  .L80069FEC:
    /* 5A7EC 80069FEC 5804828F */  lw         $v0, %gp_rel(D_800A3524)($gp)
    /* 5A7F0 80069FF0 00000000 */  nop
    /* 5A7F4 80069FF4 2000428C */  lw         $v0, 0x20($v0)
    /* 5A7F8 80069FF8 3000B38F */  lw         $s3, 0x30($sp)
    /* 5A7FC 80069FFC 08004230 */  andi       $v0, $v0, 0x8
    /* 5A800 8006A000 19004010 */  beqz       $v0, .L8006A068
    /* 5A804 8006A004 01002232 */   andi      $v0, $s1, 0x1
    /* 5A808 8006A008 13004010 */  beqz       $v0, .L8006A058
    /* 5A80C 8006A00C 00010224 */   addiu     $v0, $zero, 0x100
    /* 5A810 8006A010 4804848F */  lw         $a0, %gp_rel(D_800A3514)($gp)
    /* 5A814 8006A014 3004828F */  lw         $v0, %gp_rel(D_800A34FC)($gp)
    /* 5A818 8006A018 1F008430 */  andi       $a0, $a0, 0x1F
    /* 5A81C 8006A01C 0C004284 */  lh         $v0, 0xC($v0)
    /* 5A820 8006A020 C0210400 */  sll        $a0, $a0, 7
    /* 5A824 8006A024 21106202 */  addu       $v0, $s3, $v0
    /* 5A828 8006A028 C8F7010C */  jal        func_8007DF20
    /* 5A82C 8006A02C 3000A2AF */   sw        $v0, 0x30($sp)
    /* 5A830 8006A030 40180200 */  sll        $v1, $v0, 1
    /* 5A834 8006A034 21186200 */  addu       $v1, $v1, $v0
    /* 5A838 8006A038 00190300 */  sll        $v1, $v1, 4
    /* 5A83C 8006A03C 23186200 */  subu       $v1, $v1, $v0
    /* 5A840 8006A040 031B0300 */  sra        $v1, $v1, 12
    /* 5A844 8006A044 80FF6324 */  addiu      $v1, $v1, -0x80
    /* 5A848 8006A048 4300A3A3 */  sb         $v1, 0x43($sp)
    /* 5A84C 8006A04C 4200A3A3 */  sb         $v1, 0x42($sp)
    /* 5A850 8006A050 4100A3A3 */  sb         $v1, 0x41($sp)
    /* 5A854 8006A054 00010224 */  addiu      $v0, $zero, 0x100
  .L8006A058:
    /* 5A858 8006A058 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5A85C 8006A05C 3C00A2AF */  sw         $v0, 0x3C($sp)
    /* 5A860 8006A060 23A80108 */  j          .L8006A08C
    /* 5A864 8006A064 3800A2AF */   sw        $v0, 0x38($sp)
  .L8006A068:
    /* 5A868 8006A068 70000224 */  addiu      $v0, $zero, 0x70
    /* 5A86C 8006A06C 4300A2A3 */  sb         $v0, 0x43($sp)
    /* 5A870 8006A070 4200A2A3 */  sb         $v0, 0x42($sp)
    /* 5A874 8006A074 4100A2A3 */  sb         $v0, 0x41($sp)
    /* 5A878 8006A078 80000224 */  addiu      $v0, $zero, 0x80
    /* 5A87C 8006A07C 3C00A2AF */  sw         $v0, 0x3C($sp)
    /* 5A880 8006A080 3800A2AF */  sw         $v0, 0x38($sp)
    /* 5A884 8006A084 0A000224 */  addiu      $v0, $zero, 0xA
    /* 5A888 8006A088 3400A2AF */  sw         $v0, 0x34($sp)
  .L8006A08C:
    /* 5A88C 8006A08C 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5A890 8006A090 1800A38F */  lw         $v1, 0x18($sp)
    /* 5A894 8006A094 03000224 */  addiu      $v0, $zero, 0x3
    /* 5A898 8006A098 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5A89C 8006A09C 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 5A8A0 8006A0A0 0C006324 */  addiu      $v1, $v1, 0xC
    /* 5A8A4 8006A0A4 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 5A8A8 8006A0A8 0800028E */  lw         $v0, 0x8($s0)
    /* 5A8AC 8006A0AC 21280000 */  addu       $a1, $zero, $zero
    /* 5A8B0 8006A0B0 CACD010C */  jal        func_80073728
    /* 5A8B4 8006A0B4 2400A2AF */   sw        $v0, 0x24($sp)
    /* 5A8B8 8006A0B8 080002AE */  sw         $v0, 0x8($s0)
    /* 5A8BC 8006A0BC 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5A8C0 8006A0C0 3000B3AF */  sw         $s3, 0x30($sp)
    /* 5A8C4 8006A0C4 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5A8C8 8006A0C8 0400428E */  lw         $v0, 0x4($s2)
    /* 5A8CC 8006A0CC 00000000 */  nop
    /* 5A8D0 8006A0D0 0C004324 */  addiu      $v1, $v0, 0xC
    /* 5A8D4 8006A0D4 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5A8D8 8006A0D8 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 5A8DC 8006A0DC 1400028E */  lw         $v0, 0x14($s0)
    /* 5A8E0 8006A0E0 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5A8E4 8006A0E4 4BCD010C */  jal        func_8007352C
    /* 5A8E8 8006A0E8 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5A8EC 8006A0EC 140002AE */  sw         $v0, 0x14($s0)
    /* 5A8F0 8006A0F0 01002232 */  andi       $v0, $s1, 0x1
    /* 5A8F4 8006A0F4 10004010 */  beqz       $v0, .L8006A138
    /* 5A8F8 8006A0F8 02000224 */   addiu     $v0, $zero, 0x2
    /* 5A8FC 8006A0FC 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 5A900 8006A100 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5A904 8006A104 1800438E */  lw         $v1, 0x18($s2)
    /* 5A908 8006A108 01000224 */  addiu      $v0, $zero, 0x1
    /* 5A90C 8006A10C 3000A0AF */  sw         $zero, 0x30($sp)
    /* 5A910 8006A110 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5A914 8006A114 4000A2A3 */  sb         $v0, 0x40($sp)
    /* 5A918 8006A118 1800A3AF */  sw         $v1, 0x18($sp)
    /* 5A91C 8006A11C 14006324 */  addiu      $v1, $v1, 0x14
    /* 5A920 8006A120 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 5A924 8006A124 1400028E */  lw         $v0, 0x14($s0)
    /* 5A928 8006A128 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5A92C 8006A12C 4BCD010C */  jal        func_8007352C
    /* 5A930 8006A130 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5A934 8006A134 140002AE */  sw         $v0, 0x14($s0)
  .L8006A138:
    /* 5A938 8006A138 1800A48F */  lw         $a0, 0x18($sp)
    /* 5A93C 8006A13C 20B9010C */  jal        func_8006E480
    /* 5A940 8006A140 21280000 */   addu      $a1, $zero, $zero
    /* 5A944 8006A144 01000524 */  addiu      $a1, $zero, 0x1
    /* 5A948 8006A148 21300000 */  addu       $a2, $zero, $zero
    /* 5A94C 8006A14C 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5A950 8006A150 1C00048E */  lw         $a0, 0x1C($s0)
    /* 5A954 8006A154 92F0010C */  jal        func_8007C248
    /* 5A958 8006A158 21384000 */   addu      $a3, $v0, $zero
    /* 5A95C 8006A15C 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5A960 8006A160 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5A964 8006A164 1C00058E */  lw         $a1, 0x1C($s0)
    /* 5A968 8006A168 2DEA010C */  jal        func_8007A8B4
    /* 5A96C 8006A16C 0C008424 */   addiu     $a0, $a0, 0xC
    /* 5A970 8006A170 1C00028E */  lw         $v0, 0x1C($s0)
    /* 5A974 8006A174 00000000 */  nop
    /* 5A978 8006A178 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5A97C 8006A17C 1C0002AE */  sw         $v0, 0x1C($s0)
  .L8006A180:
    /* 5A980 8006A180 6800BF8F */  lw         $ra, 0x68($sp)
    /* 5A984 8006A184 6400B38F */  lw         $s3, 0x64($sp)
    /* 5A988 8006A188 6000B28F */  lw         $s2, 0x60($sp)
    /* 5A98C 8006A18C 5C00B18F */  lw         $s1, 0x5C($sp)
    /* 5A990 8006A190 5800B08F */  lw         $s0, 0x58($sp)
    /* 5A994 8006A194 7000BD27 */  addiu      $sp, $sp, 0x70
    /* 5A998 8006A198 0800E003 */  jr         $ra
    /* 5A99C 8006A19C 00000000 */   nop
endlabel func_80069F80
