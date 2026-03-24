glabel func_8006D808
    /* 5E008 8006D808 78FFBD27 */  addiu      $sp, $sp, -0x88
    /* 5E00C 8006D80C 6800B2AF */  sw         $s2, 0x68($sp)
    /* 5E010 8006D810 21908000 */  addu       $s2, $a0, $zero
    /* 5E014 8006D814 7800B6AF */  sw         $s6, 0x78($sp)
    /* 5E018 8006D818 21B0A000 */  addu       $s6, $a1, $zero
    /* 5E01C 8006D81C 7000B4AF */  sw         $s4, 0x70($sp)
    /* 5E020 8006D820 21A0C000 */  addu       $s4, $a2, $zero
    /* 5E024 8006D824 8000BEAF */  sw         $fp, 0x80($sp)
    /* 5E028 8006D828 21F0E000 */  addu       $fp, $a3, $zero
    /* 5E02C 8006D82C A0000224 */  addiu      $v0, $zero, 0xA0
    /* 5E030 8006D830 8400BFAF */  sw         $ra, 0x84($sp)
    /* 5E034 8006D834 7C00B7AF */  sw         $s7, 0x7C($sp)
    /* 5E038 8006D838 7400B5AF */  sw         $s5, 0x74($sp)
    /* 5E03C 8006D83C 6C00B3AF */  sw         $s3, 0x6C($sp)
    /* 5E040 8006D840 6400B1AF */  sw         $s1, 0x64($sp)
    /* 5E044 8006D844 6000B0AF */  sw         $s0, 0x60($sp)
    /* 5E048 8006D848 2C00BEAF */  sw         $fp, 0x2C($sp)
    /* 5E04C 8006D84C 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5E050 8006D850 3000A0AF */  sw         $zero, 0x30($sp)
    /* 5E054 8006D854 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5E058 8006D858 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5E05C 8006D85C 4300A2A3 */  sb         $v0, 0x43($sp)
    /* 5E060 8006D860 4200A2A3 */  sb         $v0, 0x42($sp)
    /* 5E064 8006D864 4100A2A3 */  sb         $v0, 0x41($sp)
    /* 5E068 8006D868 0800828E */  lw         $v0, 0x8($s4)
    /* 5E06C 8006D86C 21980000 */  addu       $s3, $zero, $zero
    /* 5E070 8006D870 1800A2AF */  sw         $v0, 0x18($sp)
  .L8006D874:
    /* 5E074 8006D874 00141300 */  sll        $v0, $s3, 16
    /* 5E078 8006D878 83130200 */  sra        $v0, $v0, 14
    /* 5E07C 8006D87C 21105400 */  addu       $v0, $v0, $s4
    /* 5E080 8006D880 0000428C */  lw         $v0, 0x0($v0)
    /* 5E084 8006D884 00000000 */  nop
    /* 5E088 8006D888 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5E08C 8006D88C 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5E090 8006D890 1C00A2AF */  sw         $v0, 0x1C($sp)
    /* 5E094 8006D894 0000428E */  lw         $v0, 0x0($s2)
    /* 5E098 8006D898 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5E09C 8006D89C 4BCD010C */  jal        func_8007352C
    /* 5E0A0 8006D8A0 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5E0A4 8006D8A4 01006326 */  addiu      $v1, $s3, 0x1
    /* 5E0A8 8006D8A8 21986000 */  addu       $s3, $v1, $zero
    /* 5E0AC 8006D8AC 001C0300 */  sll        $v1, $v1, 16
    /* 5E0B0 8006D8B0 031C0300 */  sra        $v1, $v1, 16
    /* 5E0B4 8006D8B4 03006328 */  slti       $v1, $v1, 0x3
    /* 5E0B8 8006D8B8 EEFF6014 */  bnez       $v1, .L8006D874
    /* 5E0BC 8006D8BC 000042AE */   sw        $v0, 0x0($s2)
    /* 5E0C0 8006D8C0 0000848E */  lw         $a0, 0x0($s4)
    /* 5E0C4 8006D8C4 21280000 */  addu       $a1, $zero, $zero
    /* 5E0C8 8006D8C8 20B9010C */  jal        func_8006E480
    /* 5E0CC 8006D8CC 1800A4AF */   sw        $a0, 0x18($sp)
    /* 5E0D0 8006D8D0 01000524 */  addiu      $a1, $zero, 0x1
    /* 5E0D4 8006D8D4 21300000 */  addu       $a2, $zero, $zero
    /* 5E0D8 8006D8D8 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5E0DC 8006D8DC 0000C48E */  lw         $a0, 0x0($s6)
    /* 5E0E0 8006D8E0 92F0010C */  jal        func_8007C248
    /* 5E0E4 8006D8E4 21384000 */   addu      $a3, $v0, $zero
    /* 5E0E8 8006D8E8 80201E00 */  sll        $a0, $fp, 2
    /* 5E0EC 8006D8EC 0A80023C */  lui        $v0, %hi(D_800A374C)
    /* 5E0F0 8006D8F0 4C37428C */  lw         $v0, %lo(D_800A374C)($v0)
    /* 5E0F4 8006D8F4 0000C58E */  lw         $a1, 0x0($s6)
    /* 5E0F8 8006D8F8 2DEA010C */  jal        func_8007A8B4
    /* 5E0FC 8006D8FC 21204400 */   addu      $a0, $v0, $a0
    /* 5E100 8006D900 0000C28E */  lw         $v0, 0x0($s6)
    /* 5E104 8006D904 00000000 */  nop
    /* 5E108 8006D908 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5E10C 8006D90C 0000C2AE */  sw         $v0, 0x0($s6)
    /* 5E110 8006D910 9800A88F */  lw         $t0, 0x98($sp)
    /* 5E114 8006D914 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 5E118 8006D918 02000215 */  bne        $t0, $v0, .L8006D924
    /* 5E11C 8006D91C 04001124 */   addiu     $s1, $zero, 0x4
    /* 5E120 8006D920 03001124 */  addiu      $s1, $zero, 0x3
  .L8006D924:
    /* 5E124 8006D924 21102002 */  addu       $v0, $s1, $zero
    /* 5E128 8006D928 42004010 */  beqz       $v0, .L8006DA34
    /* 5E12C 8006D92C 21980000 */   addu      $s3, $zero, $zero
    /* 5E130 8006D930 21A84000 */  addu       $s5, $v0, $zero
    /* 5E134 8006D934 00141300 */  sll        $v0, $s3, 16
  .L8006D938:
    /* 5E138 8006D938 032C0200 */  sra        $a1, $v0, 16
    /* 5E13C 8006D93C 5804828F */  lw         $v0, %gp_rel(D_800A3524)($gp)
    /* 5E140 8006D940 80180500 */  sll        $v1, $a1, 2
    /* 5E144 8006D944 21104300 */  addu       $v0, $v0, $v1
    /* 5E148 8006D948 24004390 */  lbu        $v1, 0x24($v0)
    /* 5E14C 8006D94C 00000000 */  nop
    /* 5E150 8006D950 F4FF6224 */  addiu      $v0, $v1, -0xC
    /* 5E154 8006D954 0A00422C */  sltiu      $v0, $v0, 0xA
    /* 5E158 8006D958 02004010 */  beqz       $v0, .L8006D964
    /* 5E15C 8006D95C 00000000 */   nop
    /* 5E160 8006D960 FEFF6324 */  addiu      $v1, $v1, -0x2
  .L8006D964:
    /* 5E164 8006D964 0C00828E */  lw         $v0, 0xC($s4)
    /* 5E168 8006D968 00000000 */  nop
    /* 5E16C 8006D96C 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5E170 8006D970 00140300 */  sll        $v0, $v1, 16
    /* 5E174 8006D974 03840200 */  sra        $s0, $v0, 16
    /* 5E178 8006D978 40201000 */  sll        $a0, $s0, 1
    /* 5E17C 8006D97C 21109000 */  addu       $v0, $a0, $s0
    /* 5E180 8006D980 C0100200 */  sll        $v0, $v0, 3
    /* 5E184 8006D984 1000838E */  lw         $v1, 0x10($s4)
    /* 5E188 8006D988 18004224 */  addiu      $v0, $v0, 0x18
    /* 5E18C 8006D98C 21186200 */  addu       $v1, $v1, $v0
    /* 5E190 8006D990 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 5E194 8006D994 1C00828E */  lw         $v0, 0x1C($s4)
    /* 5E198 8006D998 00000000 */  nop
    /* 5E19C 8006D99C 21208200 */  addu       $a0, $a0, $v0
    /* 5E1A0 8006D9A0 40100500 */  sll        $v0, $a1, 1
    /* 5E1A4 8006D9A4 21104500 */  addu       $v0, $v0, $a1
    /* 5E1A8 8006D9A8 80100200 */  sll        $v0, $v0, 2
    /* 5E1AC 8006D9AC 21104500 */  addu       $v0, $v0, $a1
    /* 5E1B0 8006D9B0 00008384 */  lh         $v1, 0x0($a0)
    /* 5E1B4 8006D9B4 40100200 */  sll        $v0, $v0, 1
    /* 5E1B8 8006D9B8 3400A2AF */  sw         $v0, 0x34($sp)
    /* 5E1BC 8006D9BC 03000224 */  addiu      $v0, $zero, 0x3
    /* 5E1C0 8006D9C0 0500A214 */  bne        $a1, $v0, .L8006D9D8
    /* 5E1C4 8006D9C4 3000A3AF */   sw        $v1, 0x30($sp)
    /* 5E1C8 8006D9C8 CEFF6224 */  addiu      $v0, $v1, -0x32
    /* 5E1CC 8006D9CC 3000A2AF */  sw         $v0, 0x30($sp)
    /* 5E1D0 8006D9D0 62000224 */  addiu      $v0, $zero, 0x62
    /* 5E1D4 8006D9D4 3400A2AF */  sw         $v0, 0x34($sp)
  .L8006D9D8:
    /* 5E1D8 8006D9D8 0000428E */  lw         $v0, 0x0($s2)
    /* 5E1DC 8006D9DC 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5E1E0 8006D9E0 4BCD010C */  jal        func_8007352C
    /* 5E1E4 8006D9E4 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5E1E8 8006D9E8 000042AE */  sw         $v0, 0x0($s2)
    /* 5E1EC 8006D9EC 08000224 */  addiu      $v0, $zero, 0x8
    /* 5E1F0 8006D9F0 0A000216 */  bne        $s0, $v0, .L8006DA1C
    /* 5E1F4 8006D9F4 01006226 */   addiu     $v0, $s3, 0x1
    /* 5E1F8 8006D9F8 1000828E */  lw         $v0, 0x10($s4)
    /* 5E1FC 8006D9FC 00000000 */  nop
    /* 5E200 8006DA00 1C00A2AF */  sw         $v0, 0x1C($sp)
    /* 5E204 8006DA04 0000428E */  lw         $v0, 0x0($s2)
    /* 5E208 8006DA08 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5E20C 8006DA0C 4BCD010C */  jal        func_8007352C
    /* 5E210 8006DA10 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5E214 8006DA14 000042AE */  sw         $v0, 0x0($s2)
    /* 5E218 8006DA18 01006226 */  addiu      $v0, $s3, 0x1
  .L8006DA1C:
    /* 5E21C 8006DA1C 21984000 */  addu       $s3, $v0, $zero
    /* 5E220 8006DA20 00140200 */  sll        $v0, $v0, 16
    /* 5E224 8006DA24 03140200 */  sra        $v0, $v0, 16
    /* 5E228 8006DA28 2A105500 */  slt        $v0, $v0, $s5
    /* 5E22C 8006DA2C C2FF4014 */  bnez       $v0, .L8006D938
    /* 5E230 8006DA30 00141300 */   sll       $v0, $s3, 16
  .L8006DA34:
    /* 5E234 8006DA34 21280000 */  addu       $a1, $zero, $zero
    /* 5E238 8006DA38 21980000 */  addu       $s3, $zero, $zero
    /* 5E23C 8006DA3C 21B82002 */  addu       $s7, $s1, $zero
    /* 5E240 8006DA40 2A401700 */  slt        $t0, $zero, $s7
    /* 5E244 8006DA44 5800A8AF */  sw         $t0, 0x58($sp)
    /* 5E248 8006DA48 0C00848E */  lw         $a0, 0xC($s4)
    /* 5E24C 8006DA4C 6666153C */  lui        $s5, (0x66666667 >> 16)
    /* 5E250 8006DA50 20B9010C */  jal        func_8006E480
    /* 5E254 8006DA54 1800A4AF */   sw        $a0, 0x18($sp)
    /* 5E258 8006DA58 01000524 */  addiu      $a1, $zero, 0x1
    /* 5E25C 8006DA5C 21300000 */  addu       $a2, $zero, $zero
    /* 5E260 8006DA60 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5E264 8006DA64 0000C48E */  lw         $a0, 0x0($s6)
    /* 5E268 8006DA68 92F0010C */  jal        func_8007C248
    /* 5E26C 8006DA6C 21384000 */   addu      $a3, $v0, $zero
    /* 5E270 8006DA70 80201E00 */  sll        $a0, $fp, 2
    /* 5E274 8006DA74 0A80023C */  lui        $v0, %hi(D_800A374C)
    /* 5E278 8006DA78 4C37428C */  lw         $v0, %lo(D_800A374C)($v0)
    /* 5E27C 8006DA7C 0000C58E */  lw         $a1, 0x0($s6)
    /* 5E280 8006DA80 2DEA010C */  jal        func_8007A8B4
    /* 5E284 8006DA84 21204400 */   addu      $a0, $v0, $a0
    /* 5E288 8006DA88 0000C28E */  lw         $v0, 0x0($s6)
    /* 5E28C 8006DA8C 6766B536 */  ori        $s5, $s5, (0x66666667 & 0xFFFF)
    /* 5E290 8006DA90 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5E294 8006DA94 0000C2AE */  sw         $v0, 0x0($s6)
  .L8006DA98:
    /* 5E298 8006DA98 5800A88F */  lw         $t0, 0x58($sp)
    /* 5E29C 8006DA9C 00000000 */  nop
    /* 5E2A0 8006DAA0 95000011 */  beqz       $t0, .L8006DCF8
    /* 5E2A4 8006DAA4 21880000 */   addu      $s1, $zero, $zero
    /* 5E2A8 8006DAA8 00141300 */  sll        $v0, $s3, 16
    /* 5E2AC 8006DAAC 03840200 */  sra        $s0, $v0, 16
  .L8006DAB0:
    /* 5E2B0 8006DAB0 0A80023C */  lui        $v0, %hi(D_800A36AC)
    /* 5E2B4 8006DAB4 AC36428C */  lw         $v0, %lo(D_800A36AC)($v0)
    /* 5E2B8 8006DAB8 00000000 */  nop
    /* 5E2BC 8006DABC 01004230 */  andi       $v0, $v0, 0x1
    /* 5E2C0 8006DAC0 0D004010 */  beqz       $v0, .L8006DAF8
    /* 5E2C4 8006DAC4 00141100 */   sll       $v0, $s1, 16
    /* 5E2C8 8006DAC8 9800A88F */  lw         $t0, 0x98($sp)
    /* 5E2CC 8006DACC 031C0200 */  sra        $v1, $v0, 16
    /* 5E2D0 8006DAD0 05006814 */  bne        $v1, $t0, .L8006DAE8
    /* 5E2D4 8006DAD4 03000224 */   addiu     $v0, $zero, 0x3
    /* 5E2D8 8006DAD8 03006228 */  slti       $v0, $v1, 0x3
    /* 5E2DC 8006DADC 04004014 */  bnez       $v0, .L8006DAF0
    /* 5E2E0 8006DAE0 01000224 */   addiu     $v0, $zero, 0x1
    /* 5E2E4 8006DAE4 03000224 */  addiu      $v0, $zero, 0x3
  .L8006DAE8:
    /* 5E2E8 8006DAE8 03006214 */  bne        $v1, $v0, .L8006DAF8
    /* 5E2EC 8006DAEC 01000224 */   addiu     $v0, $zero, 0x1
  .L8006DAF0:
    /* 5E2F0 8006DAF0 BFB60108 */  j          .L8006DAFC
    /* 5E2F4 8006DAF4 4000A2A3 */   sb        $v0, 0x40($sp)
  .L8006DAF8:
    /* 5E2F8 8006DAF8 4000A0A3 */  sb         $zero, 0x40($sp)
  .L8006DAFC:
    /* 5E2FC 8006DAFC 01000224 */  addiu      $v0, $zero, 0x1
    /* 5E300 8006DB00 12000212 */  beq        $s0, $v0, .L8006DB4C
    /* 5E304 8006DB04 0200022A */   slti      $v0, $s0, 0x2
    /* 5E308 8006DB08 05004010 */  beqz       $v0, .L8006DB20
    /* 5E30C 8006DB0C 00000000 */   nop
    /* 5E310 8006DB10 08000012 */  beqz       $s0, .L8006DB34
    /* 5E314 8006DB14 00141100 */   sll       $v0, $s1, 16
    /* 5E318 8006DB18 E1B60108 */  j          .L8006DB84
    /* 5E31C 8006DB1C 00000000 */   nop
  .L8006DB20:
    /* 5E320 8006DB20 02000224 */  addiu      $v0, $zero, 0x2
    /* 5E324 8006DB24 10000212 */  beq        $s0, $v0, .L8006DB68
    /* 5E328 8006DB28 00141100 */   sll       $v0, $s1, 16
    /* 5E32C 8006DB2C E1B60108 */  j          .L8006DB84
    /* 5E330 8006DB30 00000000 */   nop
  .L8006DB34:
    /* 5E334 8006DB34 5804838F */  lw         $v1, %gp_rel(D_800A3524)($gp)
    /* 5E338 8006DB38 83130200 */  sra        $v0, $v0, 14
    /* 5E33C 8006DB3C 21186200 */  addu       $v1, $v1, $v0
    /* 5E340 8006DB40 21006290 */  lbu        $v0, 0x21($v1)
    /* 5E344 8006DB44 E0B60108 */  j          .L8006DB80
    /* 5E348 8006DB48 4A00A2A7 */   sh        $v0, 0x4A($sp)
  .L8006DB4C:
    /* 5E34C 8006DB4C 00141100 */  sll        $v0, $s1, 16
    /* 5E350 8006DB50 5804838F */  lw         $v1, %gp_rel(D_800A3524)($gp)
    /* 5E354 8006DB54 83130200 */  sra        $v0, $v0, 14
    /* 5E358 8006DB58 21186200 */  addu       $v1, $v1, $v0
    /* 5E35C 8006DB5C 22006290 */  lbu        $v0, 0x22($v1)
    /* 5E360 8006DB60 E0B60108 */  j          .L8006DB80
    /* 5E364 8006DB64 4A00A2A7 */   sh        $v0, 0x4A($sp)
  .L8006DB68:
    /* 5E368 8006DB68 5804838F */  lw         $v1, %gp_rel(D_800A3524)($gp)
    /* 5E36C 8006DB6C 83130200 */  sra        $v0, $v0, 14
    /* 5E370 8006DB70 21186200 */  addu       $v1, $v1, $v0
    /* 5E374 8006DB74 23006290 */  lbu        $v0, 0x23($v1)
    /* 5E378 8006DB78 00000000 */  nop
    /* 5E37C 8006DB7C 4A00A2A7 */  sh         $v0, 0x4A($sp)
  .L8006DB80:
    /* 5E380 8006DB80 4800A2A7 */  sh         $v0, 0x48($sp)
  .L8006DB84:
    /* 5E384 8006DB84 4A00A397 */  lhu        $v1, 0x4A($sp)
    /* 5E388 8006DB88 00000000 */  nop
    /* 5E38C 8006DB8C 001C0300 */  sll        $v1, $v1, 16
    /* 5E390 8006DB90 03140300 */  sra        $v0, $v1, 16
    /* 5E394 8006DB94 18005500 */  mult       $v0, $s5
    /* 5E398 8006DB98 4800A297 */  lhu        $v0, 0x48($sp)
    /* 5E39C 8006DB9C 10480000 */  mfhi       $t1
    /* 5E3A0 8006DBA0 00140200 */  sll        $v0, $v0, 16
    /* 5E3A4 8006DBA4 032C0200 */  sra        $a1, $v0, 16
    /* 5E3A8 8006DBA8 1800B500 */  mult       $a1, $s5
    /* 5E3AC 8006DBAC C31F0300 */  sra        $v1, $v1, 31
    /* 5E3B0 8006DBB0 83200900 */  sra        $a0, $t1, 2
    /* 5E3B4 8006DBB4 23208300 */  subu       $a0, $a0, $v1
    /* 5E3B8 8006DBB8 10380000 */  mfhi       $a3
    /* 5E3BC 8006DBBC 00240400 */  sll        $a0, $a0, 16
    /* 5E3C0 8006DBC0 03340400 */  sra        $a2, $a0, 16
    /* 5E3C4 8006DBC4 1800D500 */  mult       $a2, $s5
    /* 5E3C8 8006DBC8 C3170200 */  sra        $v0, $v0, 31
    /* 5E3CC 8006DBCC C3270400 */  sra        $a0, $a0, 31
    /* 5E3D0 8006DBD0 83180700 */  sra        $v1, $a3, 2
    /* 5E3D4 8006DBD4 23186200 */  subu       $v1, $v1, $v0
    /* 5E3D8 8006DBD8 80100300 */  sll        $v0, $v1, 2
    /* 5E3DC 8006DBDC 21104300 */  addu       $v0, $v0, $v1
    /* 5E3E0 8006DBE0 40100200 */  sll        $v0, $v0, 1
    /* 5E3E4 8006DBE4 2328A200 */  subu       $a1, $a1, $v0
    /* 5E3E8 8006DBE8 4800A5A7 */  sh         $a1, 0x48($sp)
    /* 5E3EC 8006DBEC 002C0500 */  sll        $a1, $a1, 16
    /* 5E3F0 8006DBF0 032C0500 */  sra        $a1, $a1, 16
    /* 5E3F4 8006DBF4 10480000 */  mfhi       $t1
    /* 5E3F8 8006DBF8 83180900 */  sra        $v1, $t1, 2
    /* 5E3FC 8006DBFC 23186400 */  subu       $v1, $v1, $a0
    /* 5E400 8006DC00 80100300 */  sll        $v0, $v1, 2
    /* 5E404 8006DC04 21104300 */  addu       $v0, $v0, $v1
    /* 5E408 8006DC08 40100200 */  sll        $v0, $v0, 1
    /* 5E40C 8006DC0C 2330C200 */  subu       $a2, $a2, $v0
    /* 5E410 8006DC10 40100500 */  sll        $v0, $a1, 1
    /* 5E414 8006DC14 21104500 */  addu       $v0, $v0, $a1
    /* 5E418 8006DC18 4A00A6A7 */  sh         $a2, 0x4A($sp)
    /* 5E41C 8006DC1C 1400838E */  lw         $v1, 0x14($s4)
    /* 5E420 8006DC20 C0100200 */  sll        $v0, $v0, 3
    /* 5E424 8006DC24 1800A3AF */  sw         $v1, 0x18($sp)
    /* 5E428 8006DC28 080062A4 */  sh         $v0, 0x8($v1)
    /* 5E42C 8006DC2C C0101000 */  sll        $v0, $s0, 3
    /* 5E430 8006DC30 23105000 */  subu       $v0, $v0, $s0
    /* 5E434 8006DC34 80100200 */  sll        $v0, $v0, 2
    /* 5E438 8006DC38 21105000 */  addu       $v0, $v0, $s0
    /* 5E43C 8006DC3C 40280200 */  sll        $a1, $v0, 1
    /* 5E440 8006DC40 1800A224 */  addiu      $v0, $a1, 0x18
    /* 5E444 8006DC44 001C1100 */  sll        $v1, $s1, 16
    /* 5E448 8006DC48 1800848E */  lw         $a0, 0x18($s4)
    /* 5E44C 8006DC4C 031C0300 */  sra        $v1, $v1, 16
    /* 5E450 8006DC50 3000A2AF */  sw         $v0, 0x30($sp)
    /* 5E454 8006DC54 40100300 */  sll        $v0, $v1, 1
    /* 5E458 8006DC58 21104300 */  addu       $v0, $v0, $v1
    /* 5E45C 8006DC5C 80100200 */  sll        $v0, $v0, 2
    /* 5E460 8006DC60 21104300 */  addu       $v0, $v0, $v1
    /* 5E464 8006DC64 40100200 */  sll        $v0, $v0, 1
    /* 5E468 8006DC68 02004224 */  addiu      $v0, $v0, 0x2
    /* 5E46C 8006DC6C 3400A2AF */  sw         $v0, 0x34($sp)
    /* 5E470 8006DC70 03000224 */  addiu      $v0, $zero, 0x3
    /* 5E474 8006DC74 05006214 */  bne        $v1, $v0, .L8006DC8C
    /* 5E478 8006DC78 1C00A4AF */   sw        $a0, 0x1C($sp)
    /* 5E47C 8006DC7C 0300A224 */  addiu      $v0, $a1, 0x3
    /* 5E480 8006DC80 3000A2AF */  sw         $v0, 0x30($sp)
    /* 5E484 8006DC84 64000224 */  addiu      $v0, $zero, 0x64
    /* 5E488 8006DC88 3400A2AF */  sw         $v0, 0x34($sp)
  .L8006DC8C:
    /* 5E48C 8006DC8C 0000428E */  lw         $v0, 0x0($s2)
    /* 5E490 8006DC90 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5E494 8006DC94 4BCD010C */  jal        func_8007352C
    /* 5E498 8006DC98 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5E49C 8006DC9C 000042AE */  sw         $v0, 0x0($s2)
    /* 5E4A0 8006DCA0 4A00A387 */  lh         $v1, 0x4A($sp)
    /* 5E4A4 8006DCA4 00000000 */  nop
    /* 5E4A8 8006DCA8 40100300 */  sll        $v0, $v1, 1
    /* 5E4AC 8006DCAC 21104300 */  addu       $v0, $v0, $v1
    /* 5E4B0 8006DCB0 1800A38F */  lw         $v1, 0x18($sp)
    /* 5E4B4 8006DCB4 C0100200 */  sll        $v0, $v0, 3
    /* 5E4B8 8006DCB8 080062A4 */  sh         $v0, 0x8($v1)
    /* 5E4BC 8006DCBC 3000A28F */  lw         $v0, 0x30($sp)
    /* 5E4C0 8006DCC0 00000000 */  nop
    /* 5E4C4 8006DCC4 E8FF4224 */  addiu      $v0, $v0, -0x18
    /* 5E4C8 8006DCC8 3000A2AF */  sw         $v0, 0x30($sp)
    /* 5E4CC 8006DCCC 0000428E */  lw         $v0, 0x0($s2)
    /* 5E4D0 8006DCD0 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5E4D4 8006DCD4 4BCD010C */  jal        func_8007352C
    /* 5E4D8 8006DCD8 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5E4DC 8006DCDC 01002326 */  addiu      $v1, $s1, 0x1
    /* 5E4E0 8006DCE0 21886000 */  addu       $s1, $v1, $zero
    /* 5E4E4 8006DCE4 001C0300 */  sll        $v1, $v1, 16
    /* 5E4E8 8006DCE8 031C0300 */  sra        $v1, $v1, 16
    /* 5E4EC 8006DCEC 2A187700 */  slt        $v1, $v1, $s7
    /* 5E4F0 8006DCF0 6FFF6014 */  bnez       $v1, .L8006DAB0
    /* 5E4F4 8006DCF4 000042AE */   sw        $v0, 0x0($s2)
  .L8006DCF8:
    /* 5E4F8 8006DCF8 01006226 */  addiu      $v0, $s3, 0x1
    /* 5E4FC 8006DCFC 21984000 */  addu       $s3, $v0, $zero
    /* 5E500 8006DD00 00140200 */  sll        $v0, $v0, 16
    /* 5E504 8006DD04 03140200 */  sra        $v0, $v0, 16
    /* 5E508 8006DD08 03004228 */  slti       $v0, $v0, 0x3
    /* 5E50C 8006DD0C 62FF4014 */  bnez       $v0, .L8006DA98
    /* 5E510 8006DD10 21280000 */   addu      $a1, $zero, $zero
    /* 5E514 8006DD14 1400848E */  lw         $a0, 0x14($s4)
    /* 5E518 8006DD18 20B9010C */  jal        func_8006E480
    /* 5E51C 8006DD1C 1800A4AF */   sw        $a0, 0x18($sp)
    /* 5E520 8006DD20 01000524 */  addiu      $a1, $zero, 0x1
    /* 5E524 8006DD24 21300000 */  addu       $a2, $zero, $zero
    /* 5E528 8006DD28 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5E52C 8006DD2C 0000C48E */  lw         $a0, 0x0($s6)
    /* 5E530 8006DD30 92F0010C */  jal        func_8007C248
    /* 5E534 8006DD34 21384000 */   addu      $a3, $v0, $zero
    /* 5E538 8006DD38 80201E00 */  sll        $a0, $fp, 2
    /* 5E53C 8006DD3C 0A80023C */  lui        $v0, %hi(D_800A374C)
    /* 5E540 8006DD40 4C37428C */  lw         $v0, %lo(D_800A374C)($v0)
    /* 5E544 8006DD44 0000C58E */  lw         $a1, 0x0($s6)
    /* 5E548 8006DD48 2DEA010C */  jal        func_8007A8B4
    /* 5E54C 8006DD4C 21204400 */   addu      $a0, $v0, $a0
    /* 5E550 8006DD50 0000C28E */  lw         $v0, 0x0($s6)
    /* 5E554 8006DD54 00000000 */  nop
    /* 5E558 8006DD58 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5E55C 8006DD5C 0000C2AE */  sw         $v0, 0x0($s6)
    /* 5E560 8006DD60 8400BF8F */  lw         $ra, 0x84($sp)
    /* 5E564 8006DD64 8000BE8F */  lw         $fp, 0x80($sp)
    /* 5E568 8006DD68 7C00B78F */  lw         $s7, 0x7C($sp)
    /* 5E56C 8006DD6C 7800B68F */  lw         $s6, 0x78($sp)
    /* 5E570 8006DD70 7400B58F */  lw         $s5, 0x74($sp)
    /* 5E574 8006DD74 7000B48F */  lw         $s4, 0x70($sp)
    /* 5E578 8006DD78 6C00B38F */  lw         $s3, 0x6C($sp)
    /* 5E57C 8006DD7C 6800B28F */  lw         $s2, 0x68($sp)
    /* 5E580 8006DD80 6400B18F */  lw         $s1, 0x64($sp)
    /* 5E584 8006DD84 6000B08F */  lw         $s0, 0x60($sp)
    /* 5E588 8006DD88 8800BD27 */  addiu      $sp, $sp, 0x88
    /* 5E58C 8006DD8C 0800E003 */  jr         $ra
    /* 5E590 8006DD90 00000000 */   nop
endlabel func_8006D808
