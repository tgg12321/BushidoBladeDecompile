glabel func_8005E098
    /* 4E898 8005E098 68FFBD27 */  addiu      $sp, $sp, -0x98
    /* 4E89C 8005E09C 8000B4AF */  sw         $s4, 0x80($sp)
    /* 4E8A0 8005E0A0 21A08000 */  addu       $s4, $a0, $zero
    /* 4E8A4 8005E0A4 9000BEAF */  sw         $fp, 0x90($sp)
    /* 4E8A8 8005E0A8 21F0A000 */  addu       $fp, $a1, $zero
    /* 4E8AC 8005E0AC 7000B0AF */  sw         $s0, 0x70($sp)
    /* 4E8B0 8005E0B0 21800000 */  addu       $s0, $zero, $zero
    /* 4E8B4 8005E0B4 7400B1AF */  sw         $s1, 0x74($sp)
    /* 4E8B8 8005E0B8 0A80113C */  lui        $s1, %hi(D_8009B488)
    /* 4E8BC 8005E0BC 88B43126 */  addiu      $s1, $s1, %lo(D_8009B488)
    /* 4E8C0 8005E0C0 8400B5AF */  sw         $s5, 0x84($sp)
    /* 4E8C4 8005E0C4 5000A6AF */  sw         $a2, 0x50($sp)
    /* 4E8C8 8005E0C8 5000B58F */  lw         $s5, 0x50($sp)
    /* 4E8CC 8005E0CC 0A80023C */  lui        $v0, %hi(D_8009B3A4)
    /* 4E8D0 8005E0D0 A4B34224 */  addiu      $v0, $v0, %lo(D_8009B3A4)
    /* 4E8D4 8005E0D4 9400BFAF */  sw         $ra, 0x94($sp)
    /* 4E8D8 8005E0D8 8C00B7AF */  sw         $s7, 0x8C($sp)
    /* 4E8DC 8005E0DC 8800B6AF */  sw         $s6, 0x88($sp)
    /* 4E8E0 8005E0E0 7C00B3AF */  sw         $s3, 0x7C($sp)
    /* 4E8E4 8005E0E4 7800B2AF */  sw         $s2, 0x78($sp)
    /* 4E8E8 8005E0E8 5800A7AF */  sw         $a3, 0x58($sp)
    /* 4E8EC 8005E0EC 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 4E8F0 8005E0F0 3400A0AF */  sw         $zero, 0x34($sp)
    /* 4E8F4 8005E0F4 3000A0AF */  sw         $zero, 0x30($sp)
    /* 4E8F8 8005E0F8 2800A0AF */  sw         $zero, 0x28($sp)
    /* 4E8FC 8005E0FC 1800A2AF */  sw         $v0, 0x18($sp)
    /* 4E900 8005E100 A000B226 */  addiu      $s2, $s5, 0xA0
    /* 4E904 8005E104 F802A926 */  addiu      $t1, $s5, 0x2F8
    /* 4E908 8005E108 6000A9AF */  sw         $t1, 0x60($sp)
    /* 4E90C 8005E10C 5800A98F */  lw         $t1, 0x58($sp)
    /* 4E910 8005E110 0403AA26 */  addiu      $t2, $s5, 0x304
    /* 4E914 8005E114 6800AAAF */  sw         $t2, 0x68($sp)
    /* 4E918 8005E118 2C00A9AF */  sw         $t1, 0x2C($sp)
  .L8005E11C:
    /* 4E91C 8005E11C 0E008106 */  bgez       $s4, .L8005E158
    /* 4E920 8005E120 00141000 */   sll       $v0, $s0, 16
    /* 4E924 8005E124 01000224 */  addiu      $v0, $zero, 0x1
    /* 4E928 8005E128 0600C217 */  bne        $fp, $v0, .L8005E144
    /* 4E92C 8005E12C 1C00B1AF */   sw        $s1, 0x1C($sp)
    /* 4E930 8005E130 2D000224 */  addiu      $v0, $zero, 0x2D
    /* 4E934 8005E134 0A80013C */  lui        $at, %hi(D_8009B48E)
    /* 4E938 8005E138 8EB422A0 */  sb         $v0, %lo(D_8009B48E)($at)
    /* 4E93C 8005E13C 5C780108 */  j          .L8005E170
    /* 4E940 8005E140 2000B2AF */   sw        $s2, 0x20($sp)
  .L8005E144:
    /* 4E944 8005E144 3C000224 */  addiu      $v0, $zero, 0x3C
    /* 4E948 8005E148 0A80013C */  lui        $at, %hi(D_8009B48E)
    /* 4E94C 8005E14C 8EB422A0 */  sb         $v0, %lo(D_8009B48E)($at)
    /* 4E950 8005E150 5C780108 */  j          .L8005E170
    /* 4E954 8005E154 2000B2AF */   sw        $s2, 0x20($sp)
  .L8005E158:
    /* 4E958 8005E158 43130200 */  sra        $v0, $v0, 13
    /* 4E95C 8005E15C 0A80033C */  lui        $v1, %hi(D_8009B458)
    /* 4E960 8005E160 58B46324 */  addiu      $v1, $v1, %lo(D_8009B458)
    /* 4E964 8005E164 21104300 */  addu       $v0, $v0, $v1
    /* 4E968 8005E168 1C00A2AF */  sw         $v0, 0x1C($sp)
    /* 4E96C 8005E16C 2000B2AF */  sw         $s2, 0x20($sp)
  .L8005E170:
    /* 4E970 8005E170 4BCD010C */  jal        func_8007352C
    /* 4E974 8005E174 1800A427 */   addiu     $a0, $sp, 0x18
    /* 4E978 8005E178 08008006 */  bltz       $s4, .L8005E19C
    /* 4E97C 8005E17C 21904000 */   addu      $s2, $v0, $zero
    /* 4E980 8005E180 01000226 */  addiu      $v0, $s0, 0x1
    /* 4E984 8005E184 21804000 */  addu       $s0, $v0, $zero
    /* 4E988 8005E188 00140200 */  sll        $v0, $v0, 16
    /* 4E98C 8005E18C 03140200 */  sra        $v0, $v0, 16
    /* 4E990 8005E190 02004228 */  slti       $v0, $v0, 0x2
    /* 4E994 8005E194 E1FF4014 */  bnez       $v0, .L8005E11C
    /* 4E998 8005E198 00000000 */   nop
  .L8005E19C:
    /* 4E99C 8005E19C 21980000 */  addu       $s3, $zero, $zero
    /* 4E9A0 8005E1A0 6666173C */  lui        $s7, (0x66666667 >> 16)
    /* 4E9A4 8005E1A4 6766F736 */  ori        $s7, $s7, (0x66666667 & 0xFFFF)
    /* 4E9A8 8005E1A8 1800B127 */  addiu      $s1, $sp, 0x18
    /* 4E9AC 8005E1AC 5800AA8F */  lw         $t2, 0x58($sp)
    /* 4E9B0 8005E1B0 0A80023C */  lui        $v0, %hi(D_8009B398)
    /* 4E9B4 8005E1B4 98B34224 */  addiu      $v0, $v0, %lo(D_8009B398)
    /* 4E9B8 8005E1B8 1800A2AF */  sw         $v0, 0x18($sp)
    /* 4E9BC 8005E1BC 16000224 */  addiu      $v0, $zero, 0x16
    /* 4E9C0 8005E1C0 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 4E9C4 8005E1C4 2800A0AF */  sw         $zero, 0x28($sp)
    /* 4E9C8 8005E1C8 3400A2AF */  sw         $v0, 0x34($sp)
    /* 4E9CC 8005E1CC 2C00AAAF */  sw         $t2, 0x2C($sp)
    /* 4E9D0 8005E1D0 00141300 */  sll        $v0, $s3, 16
  .L8005E1D4:
    /* 4E9D4 8005E1D4 02004014 */  bnez       $v0, .L8005E1E0
    /* 4E9D8 8005E1D8 21108002 */   addu      $v0, $s4, $zero
    /* 4E9DC 8005E1DC 2110C003 */  addu       $v0, $fp, $zero
  .L8005E1E0:
    /* 4E9E0 8005E1E0 4A00A2A7 */  sh         $v0, 0x4A($sp)
    /* 4E9E4 8005E1E4 4800A2A7 */  sh         $v0, 0x48($sp)
    /* 4E9E8 8005E1E8 4800A397 */  lhu        $v1, 0x48($sp)
    /* 4E9EC 8005E1EC 00000000 */  nop
    /* 4E9F0 8005E1F0 001C0300 */  sll        $v1, $v1, 16
    /* 4E9F4 8005E1F4 03140300 */  sra        $v0, $v1, 16
    /* 4E9F8 8005E1F8 18005700 */  mult       $v0, $s7
    /* 4E9FC 8005E1FC 4A00A597 */  lhu        $a1, 0x4A($sp)
    /* 4EA00 8005E200 10580000 */  mfhi       $t3
    /* 4EA04 8005E204 002C0500 */  sll        $a1, $a1, 16
    /* 4EA08 8005E208 03340500 */  sra        $a2, $a1, 16
    /* 4EA0C 8005E20C 1800D700 */  mult       $a2, $s7
    /* 4EA10 8005E210 21800000 */  addu       $s0, $zero, $zero
    /* 4EA14 8005E214 C31F0300 */  sra        $v1, $v1, 31
    /* 4EA18 8005E218 83200B00 */  sra        $a0, $t3, 2
    /* 4EA1C 8005E21C 23208300 */  subu       $a0, $a0, $v1
    /* 4EA20 8005E220 10400000 */  mfhi       $t0
    /* 4EA24 8005E224 00240400 */  sll        $a0, $a0, 16
    /* 4EA28 8005E228 033C0400 */  sra        $a3, $a0, 16
    /* 4EA2C 8005E22C 1800F700 */  mult       $a3, $s7
    /* 4EA30 8005E230 00141300 */  sll        $v0, $s3, 16
    /* 4EA34 8005E234 03B40200 */  sra        $s6, $v0, 16
    /* 4EA38 8005E238 C32F0500 */  sra        $a1, $a1, 31
    /* 4EA3C 8005E23C C3270400 */  sra        $a0, $a0, 31
    /* 4EA40 8005E240 83180800 */  sra        $v1, $t0, 2
    /* 4EA44 8005E244 23186500 */  subu       $v1, $v1, $a1
    /* 4EA48 8005E248 80100300 */  sll        $v0, $v1, 2
    /* 4EA4C 8005E24C 21104300 */  addu       $v0, $v0, $v1
    /* 4EA50 8005E250 40100200 */  sll        $v0, $v0, 1
    /* 4EA54 8005E254 2330C200 */  subu       $a2, $a2, $v0
    /* 4EA58 8005E258 4A00A6A7 */  sh         $a2, 0x4A($sp)
    /* 4EA5C 8005E25C 10580000 */  mfhi       $t3
    /* 4EA60 8005E260 83180B00 */  sra        $v1, $t3, 2
    /* 4EA64 8005E264 23186400 */  subu       $v1, $v1, $a0
    /* 4EA68 8005E268 80100300 */  sll        $v0, $v1, 2
    /* 4EA6C 8005E26C 21104300 */  addu       $v0, $v0, $v1
    /* 4EA70 8005E270 40100200 */  sll        $v0, $v0, 1
    /* 4EA74 8005E274 2338E200 */  subu       $a3, $a3, $v0
    /* 4EA78 8005E278 4800A7A7 */  sh         $a3, 0x48($sp)
  .L8005E27C:
    /* 4EA7C 8005E27C 00141000 */  sll        $v0, $s0, 16
    /* 4EA80 8005E280 031C0200 */  sra        $v1, $v0, 16
    /* 4EA84 8005E284 40100300 */  sll        $v0, $v1, 1
    /* 4EA88 8005E288 21105100 */  addu       $v0, $v0, $s1
    /* 4EA8C 8005E28C 30004284 */  lh         $v0, 0x30($v0)
    /* 4EA90 8005E290 00000000 */  nop
    /* 4EA94 8005E294 06004014 */  bnez       $v0, .L8005E2B0
    /* 4EA98 8005E298 00000000 */   nop
    /* 4EA9C 8005E29C 04006014 */  bnez       $v1, .L8005E2B0
    /* 4EAA0 8005E2A0 00000000 */   nop
    /* 4EAA4 8005E2A4 03008106 */  bgez       $s4, .L8005E2B4
    /* 4EAA8 8005E2A8 00141000 */   sll       $v0, $s0, 16
    /* 4EAAC 8005E2AC 01001026 */  addiu      $s0, $s0, 0x1
  .L8005E2B0:
    /* 4EAB0 8005E2B0 00141000 */  sll        $v0, $s0, 16
  .L8005E2B4:
    /* 4EAB4 8005E2B4 C3130200 */  sra        $v0, $v0, 15
    /* 4EAB8 8005E2B8 21105100 */  addu       $v0, $v0, $s1
    /* 4EABC 8005E2BC 30004284 */  lh         $v0, 0x30($v0)
    /* 4EAC0 8005E2C0 0A80033C */  lui        $v1, %hi(D_8009B400)
    /* 4EAC4 8005E2C4 00B46324 */  addiu      $v1, $v1, %lo(D_8009B400)
    /* 4EAC8 8005E2C8 C0100200 */  sll        $v0, $v0, 3
    /* 4EACC 8005E2CC 21184300 */  addu       $v1, $v0, $v1
    /* 4EAD0 8005E2D0 0300C012 */  beqz       $s6, .L8005E2E0
    /* 4EAD4 8005E2D4 1C00A3AF */   sw        $v1, 0x1C($sp)
    /* 4EAD8 8005E2D8 B9780108 */  j          .L8005E2E4
    /* 4EADC 8005E2DC 50000224 */   addiu     $v0, $zero, 0x50
  .L8005E2E0:
    /* 4EAE0 8005E2E0 09020224 */  addiu      $v0, $zero, 0x209
  .L8005E2E4:
    /* 4EAE4 8005E2E4 000062A4 */  sh         $v0, 0x0($v1)
    /* 4EAE8 8005E2E8 00141000 */  sll        $v0, $s0, 16
    /* 4EAEC 8005E2EC 03240200 */  sra        $a0, $v0, 16
    /* 4EAF0 8005E2F0 40100400 */  sll        $v0, $a0, 1
    /* 4EAF4 8005E2F4 21105100 */  addu       $v0, $v0, $s1
    /* 4EAF8 8005E2F8 30004384 */  lh         $v1, 0x30($v0)
    /* 4EAFC 8005E2FC 01000224 */  addiu      $v0, $zero, 0x1
    /* 4EB00 8005E300 05006214 */  bne        $v1, $v0, .L8005E318
    /* 4EB04 8005E304 80100400 */   sll       $v0, $a0, 2
    /* 4EB08 8005E308 21104400 */  addu       $v0, $v0, $a0
    /* 4EB0C 8005E30C 80100200 */  sll        $v0, $v0, 2
    /* 4EB10 8005E310 C8780108 */  j          .L8005E320
    /* 4EB14 8005E314 03004224 */   addiu     $v0, $v0, 0x3
  .L8005E318:
    /* 4EB18 8005E318 21104400 */  addu       $v0, $v0, $a0
    /* 4EB1C 8005E31C 80100200 */  sll        $v0, $v0, 2
  .L8005E320:
    /* 4EB20 8005E320 3000A2AF */  sw         $v0, 0x30($sp)
    /* 4EB24 8005E324 2000B2AF */  sw         $s2, 0x20($sp)
    /* 4EB28 8005E328 4BCD010C */  jal        func_8007352C
    /* 4EB2C 8005E32C 1800A427 */   addiu     $a0, $sp, 0x18
    /* 4EB30 8005E330 21904000 */  addu       $s2, $v0, $zero
    /* 4EB34 8005E334 01000226 */  addiu      $v0, $s0, 0x1
    /* 4EB38 8005E338 21804000 */  addu       $s0, $v0, $zero
    /* 4EB3C 8005E33C 00140200 */  sll        $v0, $v0, 16
    /* 4EB40 8005E340 03140200 */  sra        $v0, $v0, 16
    /* 4EB44 8005E344 02004228 */  slti       $v0, $v0, 0x2
    /* 4EB48 8005E348 CCFF4014 */  bnez       $v0, .L8005E27C
    /* 4EB4C 8005E34C 00000000 */   nop
    /* 4EB50 8005E350 07008006 */  bltz       $s4, .L8005E370
    /* 4EB54 8005E354 01006226 */   addiu     $v0, $s3, 0x1
    /* 4EB58 8005E358 21984000 */  addu       $s3, $v0, $zero
    /* 4EB5C 8005E35C 00140200 */  sll        $v0, $v0, 16
    /* 4EB60 8005E360 03140200 */  sra        $v0, $v0, 16
    /* 4EB64 8005E364 02004228 */  slti       $v0, $v0, 0x2
    /* 4EB68 8005E368 9AFF4014 */  bnez       $v0, .L8005E1D4
    /* 4EB6C 8005E36C 00141300 */   sll       $v0, $s3, 16
  .L8005E370:
    /* 4EB70 8005E370 21980000 */  addu       $s3, $zero, $zero
    /* 4EB74 8005E374 24001E24 */  addiu      $fp, $zero, 0x24
    /* 4EB78 8005E378 0A80173C */  lui        $s7, %hi(D_8009B3B0)
    /* 4EB7C 8005E37C B0B3F726 */  addiu      $s7, $s7, %lo(D_8009B3B0)
    /* 4EB80 8005E380 0E00B126 */  addiu      $s1, $s5, 0xE
    /* 4EB84 8005E384 5800A98F */  lw         $t1, 0x58($sp)
    /* 4EB88 8005E388 FF000224 */  addiu      $v0, $zero, 0xFF
    /* 4EB8C 8005E38C 4100A2A3 */  sb         $v0, 0x41($sp)
    /* 4EB90 8005E390 10000224 */  addiu      $v0, $zero, 0x10
    /* 4EB94 8005E394 4300A2A3 */  sb         $v0, 0x43($sp)
    /* 4EB98 8005E398 4200A2A3 */  sb         $v0, 0x42($sp)
    /* 4EB9C 8005E39C 01000224 */  addiu      $v0, $zero, 0x1
    /* 4EBA0 8005E3A0 4000A2A3 */  sb         $v0, 0x40($sp)
    /* 4EBA4 8005E3A4 3000A0AF */  sw         $zero, 0x30($sp)
    /* 4EBA8 8005E3A8 2800A0AF */  sw         $zero, 0x28($sp)
    /* 4EBAC 8005E3AC 80B00900 */  sll        $s6, $t1, 2
    /* 4EBB0 8005E3B0 2C00A9AF */  sw         $t1, 0x2C($sp)
  .L8005E3B4:
    /* 4EBB4 8005E3B4 B4EA010C */  jal        func_8007AAD0
    /* 4EBB8 8005E3B8 2120A002 */   addu      $a0, $s5, $zero
    /* 4EBBC 8005E3BC 2120A002 */  addu       $a0, $s5, $zero
    /* 4EBC0 8005E3C0 21280000 */  addu       $a1, $zero, $zero
    /* 4EBC4 8005E3C4 FF000224 */  addiu      $v0, $zero, 0xFF
    /* 4EBC8 8005E3C8 F6FF22A2 */  sb         $v0, -0xA($s1)
    /* 4EBCC 8005E3CC 10000224 */  addiu      $v0, $zero, 0x10
    /* 4EBD0 8005E3D0 00841300 */  sll        $s0, $s3, 16
    /* 4EBD4 8005E3D4 03841000 */  sra        $s0, $s0, 16
    /* 4EBD8 8005E3D8 F7FF22A2 */  sb         $v0, -0x9($s1)
    /* 4EBDC 8005E3DC F8FF22A2 */  sb         $v0, -0x8($s1)
    /* 4EBE0 8005E3E0 C0101000 */  sll        $v0, $s0, 3
    /* 4EBE4 8005E3E4 23105000 */  subu       $v0, $v0, $s0
    /* 4EBE8 8005E3E8 80110200 */  sll        $v0, $v0, 6
    /* 4EBEC 8005E3EC 21105000 */  addu       $v0, $v0, $s0
    /* 4EBF0 8005E3F0 09020324 */  addiu      $v1, $zero, 0x209
    /* 4EBF4 8005E3F4 23186200 */  subu       $v1, $v1, $v0
    /* 4EBF8 8005E3F8 30000224 */  addiu      $v0, $zero, 0x30
    /* 4EBFC 8005E3FC FEFF22A6 */  sh         $v0, -0x2($s1)
    /* 4EC00 8005E400 01000224 */  addiu      $v0, $zero, 0x1
    /* 4EC04 8005E404 FAFF23A6 */  sh         $v1, -0x6($s1)
    /* 4EC08 8005E408 FCFF3EA6 */  sh         $fp, -0x4($s1)
    /* 4EC0C 8005E40C 5AEA010C */  jal        func_8007A968
    /* 4EC10 8005E410 000022A6 */   sh        $v0, 0x0($s1)
    /* 4EC14 8005E414 2128A002 */  addu       $a1, $s5, $zero
    /* 4EC18 8005E418 10003126 */  addiu      $s1, $s1, 0x10
    /* 4EC1C 8005E41C 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 4EC20 8005E420 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 4EC24 8005E424 1000B526 */  addiu      $s5, $s5, 0x10
    /* 4EC28 8005E428 2DEA010C */  jal        func_8007A8B4
    /* 4EC2C 8005E42C 21209600 */   addu      $a0, $a0, $s6
    /* 4EC30 8005E430 1800A427 */  addiu      $a0, $sp, 0x18
    /* 4EC34 8005E434 00811000 */  sll        $s0, $s0, 4
    /* 4EC38 8005E438 0A800A3C */  lui        $t2, %hi(D_8009B468)
    /* 4EC3C 8005E43C 68B44A25 */  addiu      $t2, $t2, %lo(D_8009B468)
    /* 4EC40 8005E440 21100A02 */  addu       $v0, $s0, $t2
    /* 4EC44 8005E444 3400BEAF */  sw         $fp, 0x34($sp)
    /* 4EC48 8005E448 1800B7AF */  sw         $s7, 0x18($sp)
    /* 4EC4C 8005E44C 1C00A2AF */  sw         $v0, 0x1C($sp)
    /* 4EC50 8005E450 4BCD010C */  jal        func_8007352C
    /* 4EC54 8005E454 2000B2AF */   sw        $s2, 0x20($sp)
    /* 4EC58 8005E458 1800A427 */  addiu      $a0, $sp, 0x18
    /* 4EC5C 8005E45C 0C00E326 */  addiu      $v1, $s7, 0xC
    /* 4EC60 8005E460 0A80093C */  lui        $t1, %hi(D_8009B470)
    /* 4EC64 8005E464 70B42925 */  addiu      $t1, $t1, %lo(D_8009B470)
    /* 4EC68 8005E468 21800902 */  addu       $s0, $s0, $t1
    /* 4EC6C 8005E46C 1800A3AF */  sw         $v1, 0x18($sp)
    /* 4EC70 8005E470 1C00B0AF */  sw         $s0, 0x1C($sp)
    /* 4EC74 8005E474 4BCD010C */  jal        func_8007352C
    /* 4EC78 8005E478 2000A2AF */   sw        $v0, 0x20($sp)
    /* 4EC7C 8005E47C 08008006 */  bltz       $s4, .L8005E4A0
    /* 4EC80 8005E480 21904000 */   addu      $s2, $v0, $zero
    /* 4EC84 8005E484 01006226 */  addiu      $v0, $s3, 0x1
    /* 4EC88 8005E488 21984000 */  addu       $s3, $v0, $zero
    /* 4EC8C 8005E48C 00140200 */  sll        $v0, $v0, 16
    /* 4EC90 8005E490 03140200 */  sra        $v0, $v0, 16
    /* 4EC94 8005E494 02004228 */  slti       $v0, $v0, 0x2
    /* 4EC98 8005E498 C6FF4014 */  bnez       $v0, .L8005E3B4
    /* 4EC9C 8005E49C 00000000 */   nop
  .L8005E4A0:
    /* 4ECA0 8005E4A0 E8FFE426 */  addiu      $a0, $s7, -0x18
    /* 4ECA4 8005E4A4 20B9010C */  jal        func_8006E480
    /* 4ECA8 8005E4A8 21280000 */   addu      $a1, $zero, $zero
    /* 4ECAC 8005E4AC 01000524 */  addiu      $a1, $zero, 0x1
    /* 4ECB0 8005E4B0 21300000 */  addu       $a2, $zero, $zero
    /* 4ECB4 8005E4B4 6000A48F */  lw         $a0, 0x60($sp)
    /* 4ECB8 8005E4B8 21384000 */  addu       $a3, $v0, $zero
    /* 4ECBC 8005E4BC 92F0010C */  jal        func_8007C248
    /* 4ECC0 8005E4C0 1000A0AF */   sw        $zero, 0x10($sp)
    /* 4ECC4 8005E4C4 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 4ECC8 8005E4C8 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 4ECCC 8005E4CC 6000A58F */  lw         $a1, 0x60($sp)
    /* 4ECD0 8005E4D0 2DEA010C */  jal        func_8007A8B4
    /* 4ECD4 8005E4D4 21209600 */   addu      $a0, $a0, $s6
    /* 4ECD8 8005E4D8 6800AA8F */  lw         $t2, 0x68($sp)
    /* 4ECDC 8005E4DC 5000A98F */  lw         $t1, 0x50($sp)
    /* 4ECE0 8005E4E0 00000000 */  nop
    /* 4ECE4 8005E4E4 23104901 */  subu       $v0, $t2, $t1
    /* 4ECE8 8005E4E8 9400BF8F */  lw         $ra, 0x94($sp)
    /* 4ECEC 8005E4EC 9000BE8F */  lw         $fp, 0x90($sp)
    /* 4ECF0 8005E4F0 8C00B78F */  lw         $s7, 0x8C($sp)
    /* 4ECF4 8005E4F4 8800B68F */  lw         $s6, 0x88($sp)
    /* 4ECF8 8005E4F8 8400B58F */  lw         $s5, 0x84($sp)
    /* 4ECFC 8005E4FC 8000B48F */  lw         $s4, 0x80($sp)
    /* 4ED00 8005E500 7C00B38F */  lw         $s3, 0x7C($sp)
    /* 4ED04 8005E504 7800B28F */  lw         $s2, 0x78($sp)
    /* 4ED08 8005E508 7400B18F */  lw         $s1, 0x74($sp)
    /* 4ED0C 8005E50C 7000B08F */  lw         $s0, 0x70($sp)
    /* 4ED10 8005E510 9800BD27 */  addiu      $sp, $sp, 0x98
    /* 4ED14 8005E514 0800E003 */  jr         $ra
    /* 4ED18 8005E518 00000000 */   nop
endlabel func_8005E098
