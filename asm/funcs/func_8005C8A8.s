glabel func_8005C8A8
    /* 4D0A8 8005C8A8 48FFBD27 */  addiu      $sp, $sp, -0xB8
    /* 4D0AC 8005C8AC 4800A4AF */  sw         $a0, 0x48($sp)
    /* 4D0B0 8005C8B0 4800A88F */  lw         $t0, 0x48($sp)
    /* 4D0B4 8005C8B4 9C00B3AF */  sw         $s3, 0x9C($sp)
    /* 4D0B8 8005C8B8 2198C000 */  addu       $s3, $a2, $zero
    /* 4D0BC 8005C8BC A400B5AF */  sw         $s5, 0xA4($sp)
    /* 4D0C0 8005C8C0 BC00A5AF */  sw         $a1, 0xBC($sp)
    /* 4D0C4 8005C8C4 80100800 */  sll        $v0, $t0, 2
    /* 4D0C8 8005C8C8 BC00A897 */  lhu        $t0, 0xBC($sp)
    /* 4D0CC 8005C8CC F0007526 */  addiu      $s5, $s3, 0xF0
    /* 4D0D0 8005C8D0 B400BFAF */  sw         $ra, 0xB4($sp)
    /* 4D0D4 8005C8D4 B000BEAF */  sw         $fp, 0xB0($sp)
    /* 4D0D8 8005C8D8 AC00B7AF */  sw         $s7, 0xAC($sp)
    /* 4D0DC 8005C8DC A800B6AF */  sw         $s6, 0xA8($sp)
    /* 4D0E0 8005C8E0 A000B4AF */  sw         $s4, 0xA0($sp)
    /* 4D0E4 8005C8E4 9800B2AF */  sw         $s2, 0x98($sp)
    /* 4D0E8 8005C8E8 9400B1AF */  sw         $s1, 0x94($sp)
    /* 4D0EC 8005C8EC 9000B0AF */  sw         $s0, 0x90($sp)
    /* 4D0F0 8005C8F0 6000A0A7 */  sh         $zero, 0x60($sp)
    /* 4D0F4 8005C8F4 5800A8A7 */  sh         $t0, 0x58($sp)
    /* 4D0F8 8005C8F8 0A80013C */  lui        $at, %hi(D_8009B2BE)
    /* 4D0FC 8005C8FC 21082200 */  addu       $at, $at, $v0
    /* 4D100 8005C900 BEB22384 */  lh         $v1, %lo(D_8009B2BE)($at)
    /* 4D104 8005C904 D8046826 */  addiu      $t0, $s3, 0x4D8
    /* 4D108 8005C908 6800A8AF */  sw         $t0, 0x68($sp)
    /* 4D10C 8005C90C F0040824 */  addiu      $t0, $zero, 0x4F0
    /* 4D110 8005C910 7000A8AF */  sw         $t0, 0x70($sp)
    /* 4D114 8005C914 4800A88F */  lw         $t0, 0x48($sp)
    /* 4D118 8005C918 40000224 */  addiu      $v0, $zero, 0x40
    /* 4D11C 8005C91C 4300A2A3 */  sb         $v0, 0x43($sp)
    /* 4D120 8005C920 4200A2A3 */  sb         $v0, 0x42($sp)
    /* 4D124 8005C924 4100A2A3 */  sb         $v0, 0x41($sp)
    /* 4D128 8005C928 F0000224 */  addiu      $v0, $zero, 0xF0
    /* 4D12C 8005C92C 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 4D130 8005C930 5000A7AF */  sw         $a3, 0x50($sp)
    /* 4D134 8005C934 23104300 */  subu       $v0, $v0, $v1
    /* 4D138 8005C938 C21F0200 */  srl        $v1, $v0, 31
    /* 4D13C 8005C93C 21104300 */  addu       $v0, $v0, $v1
    /* 4D140 8005C940 43180200 */  sra        $v1, $v0, 1
    /* 4D144 8005C944 01000224 */  addiu      $v0, $zero, 0x1
    /* 4D148 8005C948 46010211 */  beq        $t0, $v0, .L8005CE64
    /* 4D14C 8005C94C 21B86000 */   addu      $s7, $v1, $zero
    /* 4D150 8005C950 02000229 */  slti       $v0, $t0, 0x2
    /* 4D154 8005C954 05004010 */  beqz       $v0, .L8005C96C
    /* 4D158 8005C958 00000000 */   nop
    /* 4D15C 8005C95C BC000011 */  beqz       $t0, .L8005CC50
    /* 4D160 8005C960 00000000 */   nop
    /* 4D164 8005C964 B3730108 */  j          .L8005CECC
    /* 4D168 8005C968 02001624 */   addiu     $s6, $zero, 0x2
  .L8005C96C:
    /* 4D16C 8005C96C 4800A88F */  lw         $t0, 0x48($sp)
    /* 4D170 8005C970 02000224 */  addiu      $v0, $zero, 0x2
    /* 4D174 8005C974 55010215 */  bne        $t0, $v0, .L8005CECC
    /* 4D178 8005C978 02001624 */   addiu     $s6, $zero, 0x2
    /* 4D17C 8005C97C 33000824 */  addiu      $t0, $zero, 0x33
    /* 4D180 8005C980 1800A427 */  addiu      $a0, $sp, 0x18
    /* 4D184 8005C984 21B00000 */  addu       $s6, $zero, $zero
    /* 4D188 8005C988 0A80103C */  lui        $s0, %hi(D_8009B0F8)
    /* 4D18C 8005C98C F8B01026 */  addiu      $s0, $s0, %lo(D_8009B0F8)
    /* 4D190 8005C990 0A80063C */  lui        $a2, %hi(D_8009B184)
    /* 4D194 8005C994 84B1C624 */  addiu      $a2, $a2, %lo(D_8009B184)
    /* 4D198 8005C998 00140300 */  sll        $v0, $v1, 16
    /* 4D19C 8005C99C 03140200 */  sra        $v0, $v0, 16
    /* 4D1A0 8005C9A0 6000A8A7 */  sh         $t0, 0x60($sp)
    /* 4D1A4 8005C9A4 5000A88F */  lw         $t0, 0x50($sp)
    /* 4D1A8 8005C9A8 0A80033C */  lui        $v1, %hi(D_8009B2C4)
    /* 4D1AC 8005C9AC C4B26384 */  lh         $v1, %lo(D_8009B2C4)($v1)
    /* 4D1B0 8005C9B0 73004224 */  addiu      $v0, $v0, 0x73
    /* 4D1B4 8005C9B4 3400A2AF */  sw         $v0, 0x34($sp)
    /* 4D1B8 8005C9B8 80020224 */  addiu      $v0, $zero, 0x280
    /* 4D1BC 8005C9BC 1800B0AF */  sw         $s0, 0x18($sp)
    /* 4D1C0 8005C9C0 1C00A6AF */  sw         $a2, 0x1C($sp)
    /* 4D1C4 8005C9C4 3000A0AF */  sw         $zero, 0x30($sp)
    /* 4D1C8 8005C9C8 2800A0AF */  sw         $zero, 0x28($sp)
    /* 4D1CC 8005C9CC 2000B5AF */  sw         $s5, 0x20($sp)
    /* 4D1D0 8005C9D0 23104300 */  subu       $v0, $v0, $v1
    /* 4D1D4 8005C9D4 C22F0200 */  srl        $a1, $v0, 31
    /* 4D1D8 8005C9D8 21104500 */  addu       $v0, $v0, $a1
    /* 4D1DC 8005C9DC 43100200 */  sra        $v0, $v0, 1
    /* 4D1E0 8005C9E0 80026324 */  addiu      $v1, $v1, 0x280
    /* 4D1E4 8005C9E4 2C00A8AF */  sw         $t0, 0x2C($sp)
    /* 4D1E8 8005C9E8 0000C2A4 */  sh         $v0, 0x0($a2)
    /* 4D1EC 8005C9EC C2170300 */  srl        $v0, $v1, 31
    /* 4D1F0 8005C9F0 21186200 */  addu       $v1, $v1, $v0
    /* 4D1F4 8005C9F4 43180300 */  sra        $v1, $v1, 1
    /* 4D1F8 8005C9F8 F4FF6324 */  addiu      $v1, $v1, -0xC
    /* 4D1FC 8005C9FC 0A80013C */  lui        $at, %hi(D_8009B18C)
    /* 4D200 8005CA00 8CB123A4 */  sh         $v1, %lo(D_8009B18C)($at)
    /* 4D204 8005CA04 4BCD010C */  jal        func_8007352C
    /* 4D208 8005CA08 01001424 */   addiu     $s4, $zero, 0x1
    /* 4D20C 8005CA0C 1800A427 */  addiu      $a0, $sp, 0x18
    /* 4D210 8005CA10 48001026 */  addiu      $s0, $s0, 0x48
    /* 4D214 8005CA14 5000A88F */  lw         $t0, 0x50($sp)
    /* 4D218 8005CA18 0A80033C */  lui        $v1, %hi(D_8009B20C)
    /* 4D21C 8005CA1C 0CB26324 */  addiu      $v1, $v1, %lo(D_8009B20C)
    /* 4D220 8005CA20 1800B0AF */  sw         $s0, 0x18($sp)
    /* 4D224 8005CA24 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 4D228 8005CA28 3000A0AF */  sw         $zero, 0x30($sp)
    /* 4D22C 8005CA2C 3400A0AF */  sw         $zero, 0x34($sp)
    /* 4D230 8005CA30 2000A2AF */  sw         $v0, 0x20($sp)
    /* 4D234 8005CA34 4BCD010C */  jal        func_8007352C
    /* 4D238 8005CA38 2C00A8AF */   sw        $t0, 0x2C($sp)
    /* 4D23C 8005CA3C 5800A897 */  lhu        $t0, 0x58($sp)
    /* 4D240 8005CA40 21A84000 */  addu       $s5, $v0, $zero
    /* 4D244 8005CA44 00140800 */  sll        $v0, $t0, 16
    /* 4D248 8005CA48 03F40200 */  sra        $fp, $v0, 16
    /* 4D24C 8005CA4C 00141600 */  sll        $v0, $s6, 16
  .L8005CA50:
    /* 4D250 8005CA50 03140200 */  sra        $v0, $v0, 16
    /* 4D254 8005CA54 03004224 */  addiu      $v0, $v0, 0x3
    /* 4D258 8005CA58 0300C217 */  bne        $fp, $v0, .L8005CA68
    /* 4D25C 8005CA5C 00000000 */   nop
    /* 4D260 8005CA60 9B720108 */  j          .L8005CA6C
    /* 4D264 8005CA64 2800A0AF */   sw        $zero, 0x28($sp)
  .L8005CA68:
    /* 4D268 8005CA68 2800B4AF */  sw         $s4, 0x28($sp)
  .L8005CA6C:
    /* 4D26C 8005CA6C 00841600 */  sll        $s0, $s6, 16
    /* 4D270 8005CA70 0A80023C */  lui        $v0, %hi(D_8009B14E)
    /* 4D274 8005CA74 4EB14290 */  lbu        $v0, %lo(D_8009B14E)($v0)
    /* 4D278 8005CA78 03841000 */  sra        $s0, $s0, 16
    /* 4D27C 8005CA7C 18000202 */  mult       $s0, $v0
    /* 4D280 8005CA80 1800A427 */  addiu      $a0, $sp, 0x18
    /* 4D284 8005CA84 21900000 */  addu       $s2, $zero, $zero
    /* 4D288 8005CA88 10001126 */  addiu      $s1, $s0, 0x10
    /* 4D28C 8005CA8C 5000A88F */  lw         $t0, 0x50($sp)
    /* 4D290 8005CA90 0A80033C */  lui        $v1, %hi(D_8009B23C)
    /* 4D294 8005CA94 3CB26324 */  addiu      $v1, $v1, %lo(D_8009B23C)
    /* 4D298 8005CA98 3000A0AF */  sw         $zero, 0x30($sp)
    /* 4D29C 8005CA9C 3400A0AF */  sw         $zero, 0x34($sp)
    /* 4D2A0 8005CAA0 2000B5AF */  sw         $s5, 0x20($sp)
    /* 4D2A4 8005CAA4 0A80023C */  lui        $v0, %hi(D_8009B14C)
    /* 4D2A8 8005CAA8 4CB14224 */  addiu      $v0, $v0, %lo(D_8009B14C)
    /* 4D2AC 8005CAAC 1800A2AF */  sw         $v0, 0x18($sp)
    /* 4D2B0 8005CAB0 2C00A8AF */  sw         $t0, 0x2C($sp)
    /* 4D2B4 8005CAB4 12400000 */  mflo       $t0
    /* 4D2B8 8005CAB8 C0100800 */  sll        $v0, $t0, 3
    /* 4D2BC 8005CABC 21104300 */  addu       $v0, $v0, $v1
    /* 4D2C0 8005CAC0 4BCD010C */  jal        func_8007352C
    /* 4D2C4 8005CAC4 1C00A2AF */   sw        $v0, 0x1C($sp)
    /* 4D2C8 8005CAC8 21A84000 */  addu       $s5, $v0, $zero
    /* 4D2CC 8005CACC 00111000 */  sll        $v0, $s0, 4
    /* 4D2D0 8005CAD0 23805000 */  subu       $s0, $v0, $s0
  .L8005CAD4:
    /* 4D2D4 8005CAD4 BC00A38F */  lw         $v1, 0xBC($sp)
    /* 4D2D8 8005CAD8 00141200 */  sll        $v0, $s2, 16
    /* 4D2DC 8005CADC 03140200 */  sra        $v0, $v0, 16
    /* 4D2E0 8005CAE0 07182302 */  srav       $v1, $v1, $s1
    /* 4D2E4 8005CAE4 01006330 */  andi       $v1, $v1, 0x1
    /* 4D2E8 8005CAE8 03006214 */  bne        $v1, $v0, .L8005CAF8
    /* 4D2EC 8005CAEC 00000000 */   nop
    /* 4D2F0 8005CAF0 BF720108 */  j          .L8005CAFC
    /* 4D2F4 8005CAF4 2800A0AF */   sw        $zero, 0x28($sp)
  .L8005CAF8:
    /* 4D2F8 8005CAF8 2800B4AF */  sw         $s4, 0x28($sp)
  .L8005CAFC:
    /* 4D2FC 8005CAFC 1800A427 */  addiu      $a0, $sp, 0x18
    /* 4D300 8005CB00 0A80023C */  lui        $v0, %hi(D_8009B158)
    /* 4D304 8005CB04 58B14224 */  addiu      $v0, $v0, %lo(D_8009B158)
    /* 4D308 8005CB08 1800A2AF */  sw         $v0, 0x18($sp)
    /* 4D30C 8005CB0C 00141200 */  sll        $v0, $s2, 16
    /* 4D310 8005CB10 43130200 */  sra        $v0, $v0, 13
    /* 4D314 8005CB14 0A80033C */  lui        $v1, %hi(D_8009B29C)
    /* 4D318 8005CB18 9CB26324 */  addiu      $v1, $v1, %lo(D_8009B29C)
    /* 4D31C 8005CB1C 5000A88F */  lw         $t0, 0x50($sp)
    /* 4D320 8005CB20 21104300 */  addu       $v0, $v0, $v1
    /* 4D324 8005CB24 1C00A2AF */  sw         $v0, 0x1C($sp)
    /* 4D328 8005CB28 3000A0AF */  sw         $zero, 0x30($sp)
    /* 4D32C 8005CB2C 3400B0AF */  sw         $s0, 0x34($sp)
    /* 4D330 8005CB30 2000B5AF */  sw         $s5, 0x20($sp)
    /* 4D334 8005CB34 4BCD010C */  jal        func_8007352C
    /* 4D338 8005CB38 2C00A8AF */   sw        $t0, 0x2C($sp)
    /* 4D33C 8005CB3C 21A84000 */  addu       $s5, $v0, $zero
    /* 4D340 8005CB40 01004226 */  addiu      $v0, $s2, 0x1
    /* 4D344 8005CB44 21904000 */  addu       $s2, $v0, $zero
    /* 4D348 8005CB48 00140200 */  sll        $v0, $v0, 16
    /* 4D34C 8005CB4C 03140200 */  sra        $v0, $v0, 16
    /* 4D350 8005CB50 02004228 */  slti       $v0, $v0, 0x2
    /* 4D354 8005CB54 DFFF4014 */  bnez       $v0, .L8005CAD4
    /* 4D358 8005CB58 0100C226 */   addiu     $v0, $s6, 0x1
    /* 4D35C 8005CB5C 21B04000 */  addu       $s6, $v0, $zero
    /* 4D360 8005CB60 00140200 */  sll        $v0, $v0, 16
    /* 4D364 8005CB64 03140200 */  sra        $v0, $v0, 16
    /* 4D368 8005CB68 03004228 */  slti       $v0, $v0, 0x3
    /* 4D36C 8005CB6C B8FF4014 */  bnez       $v0, .L8005CA50
    /* 4D370 8005CB70 00141600 */   sll       $v0, $s6, 16
    /* 4D374 8005CB74 21900000 */  addu       $s2, $zero, $zero
    /* 4D378 8005CB78 3C001624 */  addiu      $s6, $zero, 0x3C
    /* 4D37C 8005CB7C 52001424 */  addiu      $s4, $zero, 0x52
    /* 4D380 8005CB80 4800A88F */  lw         $t0, 0x48($sp)
    /* 4D384 8005CB84 0E007026 */  addiu      $s0, $s3, 0xE
    /* 4D388 8005CB88 80880800 */  sll        $s1, $t0, 2
  .L8005CB8C:
    /* 4D38C 8005CB8C B4EA010C */  jal        initTile
    /* 4D390 8005CB90 21206002 */   addu      $a0, $s3, $zero
    /* 4D394 8005CB94 00141200 */  sll        $v0, $s2, 16
    /* 4D398 8005CB98 05004010 */  beqz       $v0, .L8005CBB0
    /* 4D39C 8005CB9C 00000000 */   nop
    /* 4D3A0 8005CBA0 F6FF16A2 */  sb         $s6, -0xA($s0)
    /* 4D3A4 8005CBA4 F7FF16A2 */  sb         $s6, -0x9($s0)
    /* 4D3A8 8005CBA8 EF720108 */  j          .L8005CBBC
    /* 4D3AC 8005CBAC F8FF16A2 */   sb        $s6, -0x8($s0)
  .L8005CBB0:
    /* 4D3B0 8005CBB0 F6FF14A2 */  sb         $s4, -0xA($s0)
    /* 4D3B4 8005CBB4 F7FF14A2 */  sb         $s4, -0x9($s0)
    /* 4D3B8 8005CBB8 F8FF14A2 */  sb         $s4, -0x8($s0)
  .L8005CBBC:
    /* 4D3BC 8005CBBC 21206002 */  addu       $a0, $s3, $zero
    /* 4D3C0 8005CBC0 21280000 */  addu       $a1, $zero, $zero
    /* 4D3C4 8005CBC4 73004226 */  addiu      $v0, $s2, 0x73
    /* 4D3C8 8005CBC8 0A80013C */  lui        $at, %hi(D_8009B2BC)
    /* 4D3CC 8005CBCC 21083100 */  addu       $at, $at, $s1
    /* 4D3D0 8005CBD0 BCB22384 */  lh         $v1, %lo(D_8009B2BC)($at)
    /* 4D3D4 8005CBD4 2110E202 */  addu       $v0, $s7, $v0
    /* 4D3D8 8005CBD8 FCFF02A6 */  sh         $v0, -0x4($s0)
    /* 4D3DC 8005CBDC 80020224 */  addiu      $v0, $zero, 0x280
    /* 4D3E0 8005CBE0 23104300 */  subu       $v0, $v0, $v1
    /* 4D3E4 8005CBE4 C21F0200 */  srl        $v1, $v0, 31
    /* 4D3E8 8005CBE8 21104300 */  addu       $v0, $v0, $v1
    /* 4D3EC 8005CBEC 43100200 */  sra        $v0, $v0, 1
    /* 4D3F0 8005CBF0 FAFF02A6 */  sh         $v0, -0x6($s0)
    /* 4D3F4 8005CBF4 0A80013C */  lui        $at, %hi(D_8009B2BC)
    /* 4D3F8 8005CBF8 21083100 */  addu       $at, $at, $s1
    /* 4D3FC 8005CBFC BCB22394 */  lhu        $v1, %lo(D_8009B2BC)($at)
    /* 4D400 8005CC00 01000224 */  addiu      $v0, $zero, 0x1
    /* 4D404 8005CC04 000002A6 */  sh         $v0, 0x0($s0)
    /* 4D408 8005CC08 5AEA010C */  jal        gpu_SetSemiTransp
    /* 4D40C 8005CC0C FEFF03A6 */   sh        $v1, -0x2($s0)
    /* 4D410 8005CC10 21286002 */  addu       $a1, $s3, $zero
    /* 4D414 8005CC14 10001026 */  addiu      $s0, $s0, 0x10
    /* 4D418 8005CC18 10007326 */  addiu      $s3, $s3, 0x10
    /* 4D41C 8005CC1C 5000A88F */  lw         $t0, 0x50($sp)
    /* 4D420 8005CC20 0A80023C */  lui        $v0, %hi(D_800A374C)
    /* 4D424 8005CC24 4C37428C */  lw         $v0, %lo(D_800A374C)($v0)
    /* 4D428 8005CC28 80200800 */  sll        $a0, $t0, 2
    /* 4D42C 8005CC2C 2DEA010C */  jal        ot_Link
    /* 4D430 8005CC30 21204400 */   addu      $a0, $v0, $a0
    /* 4D434 8005CC34 01004226 */  addiu      $v0, $s2, 0x1
    /* 4D438 8005CC38 21904000 */  addu       $s2, $v0, $zero
    /* 4D43C 8005CC3C 00140200 */  sll        $v0, $v0, 16
    /* 4D440 8005CC40 03140200 */  sra        $v0, $v0, 16
    /* 4D444 8005CC44 02004228 */  slti       $v0, $v0, 0x2
    /* 4D448 8005CC48 D0FF4014 */  bnez       $v0, .L8005CB8C
    /* 4D44C 8005CC4C 00000000 */   nop
  .L8005CC50:
    /* 4D450 8005CC50 5800A897 */  lhu        $t0, 0x58($sp)
    /* 4D454 8005CC54 00000000 */  nop
    /* 4D458 8005CC58 03000015 */  bnez       $t0, .L8005CC68
    /* 4D45C 8005CC5C 01000224 */   addiu     $v0, $zero, 0x1
    /* 4D460 8005CC60 1B730108 */  j          .L8005CC6C
    /* 4D464 8005CC64 2800A0AF */   sw        $zero, 0x28($sp)
  .L8005CC68:
    /* 4D468 8005CC68 2800A2AF */  sw         $v0, 0x28($sp)
  .L8005CC6C:
    /* 4D46C 8005CC6C 1800A427 */  addiu      $a0, $sp, 0x18
    /* 4D470 8005CC70 0A80023C */  lui        $v0, %hi(D_8009B1AC)
    /* 4D474 8005CC74 ACB14224 */  addiu      $v0, $v0, %lo(D_8009B1AC)
    /* 4D478 8005CC78 00841700 */  sll        $s0, $s7, 16
    /* 4D47C 8005CC7C 6000A897 */  lhu        $t0, 0x60($sp)
    /* 4D480 8005CC80 03841000 */  sra        $s0, $s0, 16
    /* 4D484 8005CC84 1C00A2AF */  sw         $v0, 0x1C($sp)
    /* 4D488 8005CC88 21101001 */  addu       $v0, $t0, $s0
    /* 4D48C 8005CC8C 5000A88F */  lw         $t0, 0x50($sp)
    /* 4D490 8005CC90 0A80113C */  lui        $s1, %hi(D_8009B110)
    /* 4D494 8005CC94 10B13126 */  addiu      $s1, $s1, %lo(D_8009B110)
    /* 4D498 8005CC98 1800B1AF */  sw         $s1, 0x18($sp)
    /* 4D49C 8005CC9C 3000A0AF */  sw         $zero, 0x30($sp)
    /* 4D4A0 8005CCA0 2000B5AF */  sw         $s5, 0x20($sp)
    /* 4D4A4 8005CCA4 3400A2AF */  sw         $v0, 0x34($sp)
    /* 4D4A8 8005CCA8 4BCD010C */  jal        func_8007352C
    /* 4D4AC 8005CCAC 2C00A8AF */   sw        $t0, 0x2C($sp)
    /* 4D4B0 8005CCB0 1800A427 */  addiu      $a0, $sp, 0x18
    /* 4D4B4 8005CCB4 5000A88F */  lw         $t0, 0x50($sp)
    /* 4D4B8 8005CCB8 F4FF2326 */  addiu      $v1, $s1, -0xC
    /* 4D4BC 8005CCBC 1800A3AF */  sw         $v1, 0x18($sp)
    /* 4D4C0 8005CCC0 0A80033C */  lui        $v1, %hi(D_8009B194)
    /* 4D4C4 8005CCC4 94B16324 */  addiu      $v1, $v1, %lo(D_8009B194)
    /* 4D4C8 8005CCC8 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 4D4CC 8005CCCC 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 4D4D0 8005CCD0 3000A0AF */  sw         $zero, 0x30($sp)
    /* 4D4D4 8005CCD4 3400B0AF */  sw         $s0, 0x34($sp)
    /* 4D4D8 8005CCD8 2800A0AF */  sw         $zero, 0x28($sp)
    /* 4D4DC 8005CCDC 2000A2AF */  sw         $v0, 0x20($sp)
    /* 4D4E0 8005CCE0 4BCD010C */  jal        func_8007352C
    /* 4D4E4 8005CCE4 2C00A8AF */   sw        $t0, 0x2C($sp)
    /* 4D4E8 8005CCE8 0E00E526 */  addiu      $a1, $s7, 0xE
    /* 4D4EC 8005CCEC 21B8A000 */  addu       $s7, $a1, $zero
    /* 4D4F0 8005CCF0 1800A427 */  addiu      $a0, $sp, 0x18
    /* 4D4F4 8005CCF4 21900000 */  addu       $s2, $zero, $zero
    /* 4D4F8 8005CCF8 3C001E24 */  addiu      $fp, $zero, 0x3C
    /* 4D4FC 8005CCFC 52001624 */  addiu      $s6, $zero, 0x52
    /* 4D500 8005CD00 E8FF3126 */  addiu      $s1, $s1, -0x18
    /* 4D504 8005CD04 0A80063C */  lui        $a2, %hi(D_8009B184)
    /* 4D508 8005CD08 84B1C624 */  addiu      $a2, $a2, %lo(D_8009B184)
    /* 4D50C 8005CD0C 002C0500 */  sll        $a1, $a1, 16
    /* 4D510 8005CD10 4800A88F */  lw         $t0, 0x48($sp)
    /* 4D514 8005CD14 032C0500 */  sra        $a1, $a1, 16
    /* 4D518 8005CD18 1800B1AF */  sw         $s1, 0x18($sp)
    /* 4D51C 8005CD1C 1C00A6AF */  sw         $a2, 0x1C($sp)
    /* 4D520 8005CD20 3000A0AF */  sw         $zero, 0x30($sp)
    /* 4D524 8005CD24 3400A5AF */  sw         $a1, 0x34($sp)
    /* 4D528 8005CD28 2800A0AF */  sw         $zero, 0x28($sp)
    /* 4D52C 8005CD2C 2000A2AF */  sw         $v0, 0x20($sp)
    /* 4D530 8005CD30 80180800 */  sll        $v1, $t0, 2
    /* 4D534 8005CD34 5000A88F */  lw         $t0, 0x50($sp)
    /* 4D538 8005CD38 21A06000 */  addu       $s4, $v1, $zero
    /* 4D53C 8005CD3C 2C00A8AF */  sw         $t0, 0x2C($sp)
    /* 4D540 8005CD40 0A80013C */  lui        $at, %hi(D_8009B2BC)
    /* 4D544 8005CD44 21083400 */  addu       $at, $at, $s4
    /* 4D548 8005CD48 BCB22384 */  lh         $v1, %lo(D_8009B2BC)($at)
    /* 4D54C 8005CD4C 80020224 */  addiu      $v0, $zero, 0x280
    /* 4D550 8005CD50 23104300 */  subu       $v0, $v0, $v1
    /* 4D554 8005CD54 C21F0200 */  srl        $v1, $v0, 31
    /* 4D558 8005CD58 21104300 */  addu       $v0, $v0, $v1
    /* 4D55C 8005CD5C 43100200 */  sra        $v0, $v0, 1
    /* 4D560 8005CD60 0000C2A4 */  sh         $v0, 0x0($a2)
    /* 4D564 8005CD64 0A80013C */  lui        $at, %hi(D_8009B2BC)
    /* 4D568 8005CD68 21083400 */  addu       $at, $at, $s4
    /* 4D56C 8005CD6C BCB22284 */  lh         $v0, %lo(D_8009B2BC)($at)
    /* 4D570 8005CD70 00000000 */  nop
    /* 4D574 8005CD74 80024224 */  addiu      $v0, $v0, 0x280
    /* 4D578 8005CD78 C21F0200 */  srl        $v1, $v0, 31
    /* 4D57C 8005CD7C 21104300 */  addu       $v0, $v0, $v1
    /* 4D580 8005CD80 43100200 */  sra        $v0, $v0, 1
    /* 4D584 8005CD84 F4FF4224 */  addiu      $v0, $v0, -0xC
    /* 4D588 8005CD88 0A80013C */  lui        $at, %hi(D_8009B18C)
    /* 4D58C 8005CD8C 8CB122A4 */  sh         $v0, %lo(D_8009B18C)($at)
    /* 4D590 8005CD90 4BCD010C */  jal        func_8007352C
    /* 4D594 8005CD94 0E007026 */   addiu     $s0, $s3, 0xE
    /* 4D598 8005CD98 21A84000 */  addu       $s5, $v0, $zero
  .L8005CD9C:
    /* 4D59C 8005CD9C B4EA010C */  jal        initTile
    /* 4D5A0 8005CDA0 21206002 */   addu      $a0, $s3, $zero
    /* 4D5A4 8005CDA4 00141200 */  sll        $v0, $s2, 16
    /* 4D5A8 8005CDA8 05004010 */  beqz       $v0, .L8005CDC0
    /* 4D5AC 8005CDAC 00000000 */   nop
    /* 4D5B0 8005CDB0 F6FF1EA2 */  sb         $fp, -0xA($s0)
    /* 4D5B4 8005CDB4 F7FF1EA2 */  sb         $fp, -0x9($s0)
    /* 4D5B8 8005CDB8 73730108 */  j          .L8005CDCC
    /* 4D5BC 8005CDBC F8FF1EA2 */   sb        $fp, -0x8($s0)
  .L8005CDC0:
    /* 4D5C0 8005CDC0 F6FF16A2 */  sb         $s6, -0xA($s0)
    /* 4D5C4 8005CDC4 F7FF16A2 */  sb         $s6, -0x9($s0)
    /* 4D5C8 8005CDC8 F8FF16A2 */  sb         $s6, -0x8($s0)
  .L8005CDCC:
    /* 4D5CC 8005CDCC 21206002 */  addu       $a0, $s3, $zero
    /* 4D5D0 8005CDD0 21280000 */  addu       $a1, $zero, $zero
    /* 4D5D4 8005CDD4 0A80013C */  lui        $at, %hi(D_8009B2BC)
    /* 4D5D8 8005CDD8 21083400 */  addu       $at, $at, $s4
    /* 4D5DC 8005CDDC BCB22384 */  lh         $v1, %lo(D_8009B2BC)($at)
    /* 4D5E0 8005CDE0 2110F202 */  addu       $v0, $s7, $s2
    /* 4D5E4 8005CDE4 FCFF02A6 */  sh         $v0, -0x4($s0)
    /* 4D5E8 8005CDE8 80020224 */  addiu      $v0, $zero, 0x280
    /* 4D5EC 8005CDEC 23104300 */  subu       $v0, $v0, $v1
    /* 4D5F0 8005CDF0 C21F0200 */  srl        $v1, $v0, 31
    /* 4D5F4 8005CDF4 21104300 */  addu       $v0, $v0, $v1
    /* 4D5F8 8005CDF8 43100200 */  sra        $v0, $v0, 1
    /* 4D5FC 8005CDFC FAFF02A6 */  sh         $v0, -0x6($s0)
    /* 4D600 8005CE00 0A80013C */  lui        $at, %hi(D_8009B2BC)
    /* 4D604 8005CE04 21083400 */  addu       $at, $at, $s4
    /* 4D608 8005CE08 BCB22394 */  lhu        $v1, %lo(D_8009B2BC)($at)
    /* 4D60C 8005CE0C 01000224 */  addiu      $v0, $zero, 0x1
    /* 4D610 8005CE10 000002A6 */  sh         $v0, 0x0($s0)
    /* 4D614 8005CE14 5AEA010C */  jal        gpu_SetSemiTransp
    /* 4D618 8005CE18 FEFF03A6 */   sh        $v1, -0x2($s0)
    /* 4D61C 8005CE1C 21286002 */  addu       $a1, $s3, $zero
    /* 4D620 8005CE20 10001026 */  addiu      $s0, $s0, 0x10
    /* 4D624 8005CE24 10007326 */  addiu      $s3, $s3, 0x10
    /* 4D628 8005CE28 5000A88F */  lw         $t0, 0x50($sp)
    /* 4D62C 8005CE2C 0A80023C */  lui        $v0, %hi(D_800A374C)
    /* 4D630 8005CE30 4C37428C */  lw         $v0, %lo(D_800A374C)($v0)
    /* 4D634 8005CE34 80200800 */  sll        $a0, $t0, 2
    /* 4D638 8005CE38 2DEA010C */  jal        ot_Link
    /* 4D63C 8005CE3C 21204400 */   addu      $a0, $v0, $a0
    /* 4D640 8005CE40 01004226 */  addiu      $v0, $s2, 0x1
    /* 4D644 8005CE44 21904000 */  addu       $s2, $v0, $zero
    /* 4D648 8005CE48 00140200 */  sll        $v0, $v0, 16
    /* 4D64C 8005CE4C 03140200 */  sra        $v0, $v0, 16
    /* 4D650 8005CE50 02004228 */  slti       $v0, $v0, 0x2
    /* 4D654 8005CE54 D1FF4014 */  bnez       $v0, .L8005CD9C
    /* 4D658 8005CE58 00000000 */   nop
    /* 4D65C 8005CE5C B3730108 */  j          .L8005CECC
    /* 4D660 8005CE60 02001624 */   addiu     $s6, $zero, 0x2
  .L8005CE64:
    /* 4D664 8005CE64 5800A897 */  lhu        $t0, 0x58($sp)
    /* 4D668 8005CE68 00000000 */  nop
    /* 4D66C 8005CE6C 03000015 */  bnez       $t0, .L8005CE7C
    /* 4D670 8005CE70 00000000 */   nop
    /* 4D674 8005CE74 A2730108 */  j          .L8005CE88
    /* 4D678 8005CE78 2800A0AF */   sw        $zero, 0x28($sp)
  .L8005CE7C:
    /* 4D67C 8005CE7C 4800A88F */  lw         $t0, 0x48($sp)
    /* 4D680 8005CE80 00000000 */  nop
    /* 4D684 8005CE84 2800A8AF */  sw         $t0, 0x28($sp)
  .L8005CE88:
    /* 4D688 8005CE88 1800A427 */  addiu      $a0, $sp, 0x18
    /* 4D68C 8005CE8C 0A80023C */  lui        $v0, %hi(D_8009B11C)
    /* 4D690 8005CE90 1CB14224 */  addiu      $v0, $v0, %lo(D_8009B11C)
    /* 4D694 8005CE94 1800A2AF */  sw         $v0, 0x18($sp)
    /* 4D698 8005CE98 0A80023C */  lui        $v0, %hi(D_8009B1BC)
    /* 4D69C 8005CE9C BCB14224 */  addiu      $v0, $v0, %lo(D_8009B1BC)
    /* 4D6A0 8005CEA0 1C00A2AF */  sw         $v0, 0x1C($sp)
    /* 4D6A4 8005CEA4 00141700 */  sll        $v0, $s7, 16
    /* 4D6A8 8005CEA8 5000A88F */  lw         $t0, 0x50($sp)
    /* 4D6AC 8005CEAC 03140200 */  sra        $v0, $v0, 16
    /* 4D6B0 8005CEB0 3000A0AF */  sw         $zero, 0x30($sp)
    /* 4D6B4 8005CEB4 3400A2AF */  sw         $v0, 0x34($sp)
    /* 4D6B8 8005CEB8 2000B5AF */  sw         $s5, 0x20($sp)
    /* 4D6BC 8005CEBC 4BCD010C */  jal        func_8007352C
    /* 4D6C0 8005CEC0 2C00A8AF */   sw        $t0, 0x2C($sp)
    /* 4D6C4 8005CEC4 21A84000 */  addu       $s5, $v0, $zero
    /* 4D6C8 8005CEC8 02001624 */  addiu      $s6, $zero, 0x2
  .L8005CECC:
    /* 4D6CC 8005CECC 5800A897 */  lhu        $t0, 0x58($sp)
    /* 4D6D0 8005CED0 0A80143C */  lui        $s4, %hi(D_8009B2AC)
    /* 4D6D4 8005CED4 ACB29426 */  addiu      $s4, $s4, %lo(D_8009B2AC)
    /* 4D6D8 8005CED8 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 4D6DC 8005CEDC 00140800 */  sll        $v0, $t0, 16
    /* 4D6E0 8005CEE0 038C0200 */  sra        $s1, $v0, 16
    /* 4D6E4 8005CEE4 00141700 */  sll        $v0, $s7, 16
    /* 4D6E8 8005CEE8 6000A897 */  lhu        $t0, 0x60($sp)
    /* 4D6EC 8005CEEC 03140200 */  sra        $v0, $v0, 16
    /* 4D6F0 8005CEF0 21800201 */  addu       $s0, $t0, $v0
    /* 4D6F4 8005CEF4 00141600 */  sll        $v0, $s6, 16
  .L8005CEF8:
    /* 4D6F8 8005CEF8 03140200 */  sra        $v0, $v0, 16
    /* 4D6FC 8005CEFC FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 4D700 8005CF00 03002216 */  bne        $s1, $v0, .L8005CF10
    /* 4D704 8005CF04 01000224 */   addiu     $v0, $zero, 0x1
    /* 4D708 8005CF08 C5730108 */  j          .L8005CF14
    /* 4D70C 8005CF0C 2800A0AF */   sw        $zero, 0x28($sp)
  .L8005CF10:
    /* 4D710 8005CF10 2800A2AF */  sw         $v0, 0x28($sp)
  .L8005CF14:
    /* 4D714 8005CF14 001C1600 */  sll        $v1, $s6, 16
    /* 4D718 8005CF18 031C0300 */  sra        $v1, $v1, 16
    /* 4D71C 8005CF1C 40100300 */  sll        $v0, $v1, 1
    /* 4D720 8005CF20 21104300 */  addu       $v0, $v0, $v1
    /* 4D724 8005CF24 80100200 */  sll        $v0, $v0, 2
    /* 4D728 8005CF28 0A80123C */  lui        $s2, %hi(D_8009B110)
    /* 4D72C 8005CF2C 10B15226 */  addiu      $s2, $s2, %lo(D_8009B110)
    /* 4D730 8005CF30 21105200 */  addu       $v0, $v0, $s2
    /* 4D734 8005CF34 80180300 */  sll        $v1, $v1, 2
    /* 4D738 8005CF38 21187400 */  addu       $v1, $v1, $s4
    /* 4D73C 8005CF3C 1800A2AF */  sw         $v0, 0x18($sp)
    /* 4D740 8005CF40 0000628C */  lw         $v0, 0x0($v1)
    /* 4D744 8005CF44 5000A88F */  lw         $t0, 0x50($sp)
    /* 4D748 8005CF48 1800A427 */  addiu      $a0, $sp, 0x18
    /* 4D74C 8005CF4C 3000A0AF */  sw         $zero, 0x30($sp)
    /* 4D750 8005CF50 3400B0AF */  sw         $s0, 0x34($sp)
    /* 4D754 8005CF54 2000B5AF */  sw         $s5, 0x20($sp)
    /* 4D758 8005CF58 2C00A8AF */  sw         $t0, 0x2C($sp)
    /* 4D75C 8005CF5C 4BCD010C */  jal        func_8007352C
    /* 4D760 8005CF60 1C00A2AF */   sw        $v0, 0x1C($sp)
    /* 4D764 8005CF64 21A84000 */  addu       $s5, $v0, $zero
    /* 4D768 8005CF68 0100C226 */  addiu      $v0, $s6, 0x1
    /* 4D76C 8005CF6C 21B04000 */  addu       $s6, $v0, $zero
    /* 4D770 8005CF70 00140200 */  sll        $v0, $v0, 16
    /* 4D774 8005CF74 03140200 */  sra        $v0, $v0, 16
    /* 4D778 8005CF78 04004228 */  slti       $v0, $v0, 0x4
    /* 4D77C 8005CF7C DEFF4014 */  bnez       $v0, .L8005CEF8
    /* 4D780 8005CF80 00141600 */   sll       $v0, $s6, 16
    /* 4D784 8005CF84 1800A427 */  addiu      $a0, $sp, 0x18
    /* 4D788 8005CF88 21B00000 */  addu       $s6, $zero, $zero
    /* 4D78C 8005CF8C D0FF4226 */  addiu      $v0, $s2, -0x30
    /* 4D790 8005CF90 4800A88F */  lw         $t0, 0x48($sp)
    /* 4D794 8005CF94 0A80113C */  lui        $s1, %hi(D_8009B164)
    /* 4D798 8005CF98 64B13126 */  addiu      $s1, $s1, %lo(D_8009B164)
    /* 4D79C 8005CF9C 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 4D7A0 8005CFA0 1800A2AF */  sw         $v0, 0x18($sp)
    /* 4D7A4 8005CFA4 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 4D7A8 8005CFA8 3000A0AF */  sw         $zero, 0x30($sp)
    /* 4D7AC 8005CFAC 80100800 */  sll        $v0, $t0, 2
    /* 4D7B0 8005CFB0 21A04000 */  addu       $s4, $v0, $zero
    /* 4D7B4 8005CFB4 0A80013C */  lui        $at, %hi(D_8009B2BE)
    /* 4D7B8 8005CFB8 21082200 */  addu       $at, $at, $v0
    /* 4D7BC 8005CFBC BEB22384 */  lh         $v1, %lo(D_8009B2BE)($at)
    /* 4D7C0 8005CFC0 5000A88F */  lw         $t0, 0x50($sp)
    /* 4D7C4 8005CFC4 F0000224 */  addiu      $v0, $zero, 0xF0
    /* 4D7C8 8005CFC8 2800A0AF */  sw         $zero, 0x28($sp)
    /* 4D7CC 8005CFCC 2000B5AF */  sw         $s5, 0x20($sp)
    /* 4D7D0 8005CFD0 23104300 */  subu       $v0, $v0, $v1
    /* 4D7D4 8005CFD4 C21F0200 */  srl        $v1, $v0, 31
    /* 4D7D8 8005CFD8 21104300 */  addu       $v0, $v0, $v1
    /* 4D7DC 8005CFDC 43100200 */  sra        $v0, $v0, 1
    /* 4D7E0 8005CFE0 2C00A8AF */  sw         $t0, 0x2C($sp)
    /* 4D7E4 8005CFE4 3400A2AF */  sw         $v0, 0x34($sp)
    /* 4D7E8 8005CFE8 0A80013C */  lui        $at, %hi(D_8009B2BC)
    /* 4D7EC 8005CFEC 21083400 */  addu       $at, $at, $s4
    /* 4D7F0 8005CFF0 BCB22284 */  lh         $v0, %lo(D_8009B2BC)($at)
    /* 4D7F4 8005CFF4 80021024 */  addiu      $s0, $zero, 0x280
    /* 4D7F8 8005CFF8 23100202 */  subu       $v0, $s0, $v0
    /* 4D7FC 8005CFFC C21F0200 */  srl        $v1, $v0, 31
    /* 4D800 8005D000 21104300 */  addu       $v0, $v0, $v1
    /* 4D804 8005D004 43100200 */  sra        $v0, $v0, 1
    /* 4D808 8005D008 000022A6 */  sh         $v0, 0x0($s1)
    /* 4D80C 8005D00C 0A80013C */  lui        $at, %hi(D_8009B2BC)
    /* 4D810 8005D010 21083400 */  addu       $at, $at, $s4
    /* 4D814 8005D014 BCB22284 */  lh         $v0, %lo(D_8009B2BC)($at)
    /* 4D818 8005D018 00000000 */  nop
    /* 4D81C 8005D01C 80024224 */  addiu      $v0, $v0, 0x280
    /* 4D820 8005D020 C21F0200 */  srl        $v1, $v0, 31
    /* 4D824 8005D024 21104300 */  addu       $v0, $v0, $v1
    /* 4D828 8005D028 43100200 */  sra        $v0, $v0, 1
    /* 4D82C 8005D02C F4FF4224 */  addiu      $v0, $v0, -0xC
    /* 4D830 8005D030 0A80013C */  lui        $at, %hi(D_8009B16C)
    /* 4D834 8005D034 6CB122A4 */  sh         $v0, %lo(D_8009B16C)($at)
    /* 4D838 8005D038 4BCD010C */  jal        func_8007352C
    /* 4D83C 8005D03C 3C001724 */   addiu     $s7, $zero, 0x3C
    /* 4D840 8005D040 DCFF4326 */  addiu      $v1, $s2, -0x24
    /* 4D844 8005D044 1800A3AF */  sw         $v1, 0x18($sp)
    /* 4D848 8005D048 10002326 */  addiu      $v1, $s1, 0x10
    /* 4D84C 8005D04C 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 4D850 8005D050 0A80013C */  lui        $at, %hi(D_8009B2BE)
    /* 4D854 8005D054 21083400 */  addu       $at, $at, $s4
    /* 4D858 8005D058 BEB22384 */  lh         $v1, %lo(D_8009B2BE)($at)
    /* 4D85C 8005D05C 5000A88F */  lw         $t0, 0x50($sp)
    /* 4D860 8005D060 52001524 */  addiu      $s5, $zero, 0x52
    /* 4D864 8005D064 2000A2AF */  sw         $v0, 0x20($sp)
    /* 4D868 8005D068 F0006324 */  addiu      $v1, $v1, 0xF0
    /* 4D86C 8005D06C C2170300 */  srl        $v0, $v1, 31
    /* 4D870 8005D070 21186200 */  addu       $v1, $v1, $v0
    /* 4D874 8005D074 43180300 */  sra        $v1, $v1, 1
    /* 4D878 8005D078 3400A3AF */  sw         $v1, 0x34($sp)
    /* 4D87C 8005D07C 0A80013C */  lui        $at, %hi(D_8009B2BC)
    /* 4D880 8005D080 21083400 */  addu       $at, $at, $s4
    /* 4D884 8005D084 BCB22284 */  lh         $v0, %lo(D_8009B2BC)($at)
    /* 4D888 8005D088 80400800 */  sll        $t0, $t0, 2
    /* 4D88C 8005D08C 8000A8AF */  sw         $t0, 0x80($sp)
    /* 4D890 8005D090 23800202 */  subu       $s0, $s0, $v0
    /* 4D894 8005D094 C2171000 */  srl        $v0, $s0, 31
    /* 4D898 8005D098 21800202 */  addu       $s0, $s0, $v0
    /* 4D89C 8005D09C 43801000 */  sra        $s0, $s0, 1
    /* 4D8A0 8005D0A0 100030A6 */  sh         $s0, 0x10($s1)
    /* 4D8A4 8005D0A4 0A80013C */  lui        $at, %hi(D_8009B2BC)
    /* 4D8A8 8005D0A8 21083400 */  addu       $at, $at, $s4
    /* 4D8AC 8005D0AC BCB22284 */  lh         $v0, %lo(D_8009B2BC)($at)
    /* 4D8B0 8005D0B0 00000000 */  nop
    /* 4D8B4 8005D0B4 80024224 */  addiu      $v0, $v0, 0x280
    /* 4D8B8 8005D0B8 C21F0200 */  srl        $v1, $v0, 31
    /* 4D8BC 8005D0BC 21104300 */  addu       $v0, $v0, $v1
    /* 4D8C0 8005D0C0 43100200 */  sra        $v0, $v0, 1
    /* 4D8C4 8005D0C4 F4FF4224 */  addiu      $v0, $v0, -0xC
    /* 4D8C8 8005D0C8 0A80013C */  lui        $at, %hi(D_8009B17C)
    /* 4D8CC 8005D0CC 7CB122A4 */  sh         $v0, %lo(D_8009B17C)($at)
    /* 4D8D0 8005D0D0 4BCD010C */  jal        func_8007352C
    /* 4D8D4 8005D0D4 1800A427 */   addiu     $a0, $sp, 0x18
  .L8005D0D8:
    /* 4D8D8 8005D0D8 0A80013C */  lui        $at, %hi(D_8009B2BC)
    /* 4D8DC 8005D0DC 21083400 */  addu       $at, $at, $s4
    /* 4D8E0 8005D0E0 BCB22384 */  lh         $v1, %lo(D_8009B2BC)($at)
    /* 4D8E4 8005D0E4 80020224 */  addiu      $v0, $zero, 0x280
    /* 4D8E8 8005D0E8 23104300 */  subu       $v0, $v0, $v1
    /* 4D8EC 8005D0EC C21F0200 */  srl        $v1, $v0, 31
    /* 4D8F0 8005D0F0 21104300 */  addu       $v0, $v0, $v1
    /* 4D8F4 8005D0F4 42F00200 */  srl        $fp, $v0, 1
    /* 4D8F8 8005D0F8 00141600 */  sll        $v0, $s6, 16
    /* 4D8FC 8005D0FC 0A004010 */  beqz       $v0, .L8005D128
    /* 4D900 8005D100 F0000324 */   addiu     $v1, $zero, 0xF0
    /* 4D904 8005D104 0A80013C */  lui        $at, %hi(D_8009B2BE)
    /* 4D908 8005D108 21083400 */  addu       $at, $at, $s4
    /* 4D90C 8005D10C BEB22284 */  lh         $v0, %lo(D_8009B2BE)($at)
    /* 4D910 8005D110 00000000 */  nop
    /* 4D914 8005D114 23186200 */  subu       $v1, $v1, $v0
    /* 4D918 8005D118 C2170300 */  srl        $v0, $v1, 31
    /* 4D91C 8005D11C 21186200 */  addu       $v1, $v1, $v0
    /* 4D920 8005D120 52740108 */  j          .L8005D148
    /* 4D924 8005D124 42880300 */   srl       $s1, $v1, 1
  .L8005D128:
    /* 4D928 8005D128 0A80013C */  lui        $at, %hi(D_8009B2BE)
    /* 4D92C 8005D12C 21083400 */  addu       $at, $at, $s4
    /* 4D930 8005D130 BEB22284 */  lh         $v0, %lo(D_8009B2BE)($at)
    /* 4D934 8005D134 00000000 */  nop
    /* 4D938 8005D138 F0004224 */  addiu      $v0, $v0, 0xF0
    /* 4D93C 8005D13C C21F0200 */  srl        $v1, $v0, 31
    /* 4D940 8005D140 21104300 */  addu       $v0, $v0, $v1
    /* 4D944 8005D144 42880200 */  srl        $s1, $v0, 1
  .L8005D148:
    /* 4D948 8005D148 21900000 */  addu       $s2, $zero, $zero
    /* 4D94C 8005D14C 0E007026 */  addiu      $s0, $s3, 0xE
  .L8005D150:
    /* 4D950 8005D150 B4EA010C */  jal        initTile
    /* 4D954 8005D154 21206002 */   addu      $a0, $s3, $zero
    /* 4D958 8005D158 00141200 */  sll        $v0, $s2, 16
    /* 4D95C 8005D15C 05004010 */  beqz       $v0, .L8005D174
    /* 4D960 8005D160 00000000 */   nop
    /* 4D964 8005D164 F6FF17A2 */  sb         $s7, -0xA($s0)
    /* 4D968 8005D168 F7FF17A2 */  sb         $s7, -0x9($s0)
    /* 4D96C 8005D16C 60740108 */  j          .L8005D180
    /* 4D970 8005D170 F8FF17A2 */   sb        $s7, -0x8($s0)
  .L8005D174:
    /* 4D974 8005D174 F6FF15A2 */  sb         $s5, -0xA($s0)
    /* 4D978 8005D178 F7FF15A2 */  sb         $s5, -0x9($s0)
    /* 4D97C 8005D17C F8FF15A2 */  sb         $s5, -0x8($s0)
  .L8005D180:
    /* 4D980 8005D180 21206002 */  addu       $a0, $s3, $zero
    /* 4D984 8005D184 21280000 */  addu       $a1, $zero, $zero
    /* 4D988 8005D188 21103202 */  addu       $v0, $s1, $s2
    /* 4D98C 8005D18C FAFF1EA6 */  sh         $fp, -0x6($s0)
    /* 4D990 8005D190 FCFF02A6 */  sh         $v0, -0x4($s0)
    /* 4D994 8005D194 0A80013C */  lui        $at, %hi(D_8009B2BC)
    /* 4D998 8005D198 21083400 */  addu       $at, $at, $s4
    /* 4D99C 8005D19C BCB22394 */  lhu        $v1, %lo(D_8009B2BC)($at)
    /* 4D9A0 8005D1A0 01000224 */  addiu      $v0, $zero, 0x1
    /* 4D9A4 8005D1A4 000002A6 */  sh         $v0, 0x0($s0)
    /* 4D9A8 8005D1A8 5AEA010C */  jal        gpu_SetSemiTransp
    /* 4D9AC 8005D1AC FEFF03A6 */   sh        $v1, -0x2($s0)
    /* 4D9B0 8005D1B0 21286002 */  addu       $a1, $s3, $zero
    /* 4D9B4 8005D1B4 10001026 */  addiu      $s0, $s0, 0x10
    /* 4D9B8 8005D1B8 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 4D9BC 8005D1BC 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 4D9C0 8005D1C0 8000A88F */  lw         $t0, 0x80($sp)
    /* 4D9C4 8005D1C4 10007326 */  addiu      $s3, $s3, 0x10
    /* 4D9C8 8005D1C8 2DEA010C */  jal        ot_Link
    /* 4D9CC 8005D1CC 21208800 */   addu      $a0, $a0, $t0
    /* 4D9D0 8005D1D0 01004226 */  addiu      $v0, $s2, 0x1
    /* 4D9D4 8005D1D4 21904000 */  addu       $s2, $v0, $zero
    /* 4D9D8 8005D1D8 00140200 */  sll        $v0, $v0, 16
    /* 4D9DC 8005D1DC 03140200 */  sra        $v0, $v0, 16
    /* 4D9E0 8005D1E0 02004228 */  slti       $v0, $v0, 0x2
    /* 4D9E4 8005D1E4 DAFF4014 */  bnez       $v0, .L8005D150
    /* 4D9E8 8005D1E8 00141600 */   sll       $v0, $s6, 16
    /* 4D9EC 8005D1EC 0A004010 */  beqz       $v0, .L8005D218
    /* 4D9F0 8005D1F0 80020324 */   addiu     $v1, $zero, 0x280
    /* 4D9F4 8005D1F4 0A80013C */  lui        $at, %hi(D_8009B2BC)
    /* 4D9F8 8005D1F8 21083400 */  addu       $at, $at, $s4
    /* 4D9FC 8005D1FC BCB22284 */  lh         $v0, %lo(D_8009B2BC)($at)
    /* 4DA00 8005D200 00000000 */  nop
    /* 4DA04 8005D204 23186200 */  subu       $v1, $v1, $v0
    /* 4DA08 8005D208 C2170300 */  srl        $v0, $v1, 31
    /* 4DA0C 8005D20C 21186200 */  addu       $v1, $v1, $v0
    /* 4DA10 8005D210 8E740108 */  j          .L8005D238
    /* 4DA14 8005D214 42F00300 */   srl       $fp, $v1, 1
  .L8005D218:
    /* 4DA18 8005D218 0A80013C */  lui        $at, %hi(D_8009B2BC)
    /* 4DA1C 8005D21C 21083400 */  addu       $at, $at, $s4
    /* 4DA20 8005D220 BCB22284 */  lh         $v0, %lo(D_8009B2BC)($at)
    /* 4DA24 8005D224 00000000 */  nop
    /* 4DA28 8005D228 80024224 */  addiu      $v0, $v0, 0x280
    /* 4DA2C 8005D22C C21F0200 */  srl        $v1, $v0, 31
    /* 4DA30 8005D230 21104300 */  addu       $v0, $v0, $v1
    /* 4DA34 8005D234 42F00200 */  srl        $fp, $v0, 1
  .L8005D238:
    /* 4DA38 8005D238 21900000 */  addu       $s2, $zero, $zero
    /* 4DA3C 8005D23C 00141600 */  sll        $v0, $s6, 16
    /* 4DA40 8005D240 03140200 */  sra        $v0, $v0, 16
    /* 4DA44 8005D244 0E007026 */  addiu      $s0, $s3, 0xE
    /* 4DA48 8005D248 8800A2AF */  sw         $v0, 0x88($sp)
    /* 4DA4C 8005D24C 0A80013C */  lui        $at, %hi(D_8009B2BE)
    /* 4DA50 8005D250 21083400 */  addu       $at, $at, $s4
    /* 4DA54 8005D254 BEB22384 */  lh         $v1, %lo(D_8009B2BE)($at)
    /* 4DA58 8005D258 F0000224 */  addiu      $v0, $zero, 0xF0
    /* 4DA5C 8005D25C 23104300 */  subu       $v0, $v0, $v1
    /* 4DA60 8005D260 C21F0200 */  srl        $v1, $v0, 31
    /* 4DA64 8005D264 21104300 */  addu       $v0, $v0, $v1
    /* 4DA68 8005D268 42880200 */  srl        $s1, $v0, 1
  .L8005D26C:
    /* 4DA6C 8005D26C B4EA010C */  jal        initTile
    /* 4DA70 8005D270 21206002 */   addu      $a0, $s3, $zero
    /* 4DA74 8005D274 8800A88F */  lw         $t0, 0x88($sp)
    /* 4DA78 8005D278 00000000 */  nop
    /* 4DA7C 8005D27C 05000011 */  beqz       $t0, .L8005D294
    /* 4DA80 8005D280 00141200 */   sll       $v0, $s2, 16
    /* 4DA84 8005D284 09004014 */  bnez       $v0, .L8005D2AC
    /* 4DA88 8005D288 00000000 */   nop
    /* 4DA8C 8005D28C A8740108 */  j          .L8005D2A0
    /* 4DA90 8005D290 F6FF15A2 */   sb        $s5, -0xA($s0)
  .L8005D294:
    /* 4DA94 8005D294 05004010 */  beqz       $v0, .L8005D2AC
    /* 4DA98 8005D298 00000000 */   nop
    /* 4DA9C 8005D29C F6FF15A2 */  sb         $s5, -0xA($s0)
  .L8005D2A0:
    /* 4DAA0 8005D2A0 F7FF15A2 */  sb         $s5, -0x9($s0)
    /* 4DAA4 8005D2A4 AE740108 */  j          .L8005D2B8
    /* 4DAA8 8005D2A8 F8FF15A2 */   sb        $s5, -0x8($s0)
  .L8005D2AC:
    /* 4DAAC 8005D2AC F6FF17A2 */  sb         $s7, -0xA($s0)
    /* 4DAB0 8005D2B0 F7FF17A2 */  sb         $s7, -0x9($s0)
    /* 4DAB4 8005D2B4 F8FF17A2 */  sb         $s7, -0x8($s0)
  .L8005D2B8:
    /* 4DAB8 8005D2B8 21206002 */  addu       $a0, $s3, $zero
    /* 4DABC 8005D2BC 00141200 */  sll        $v0, $s2, 16
    /* 4DAC0 8005D2C0 C3130200 */  sra        $v0, $v0, 15
    /* 4DAC4 8005D2C4 2110C203 */  addu       $v0, $fp, $v0
    /* 4DAC8 8005D2C8 FAFF02A6 */  sh         $v0, -0x6($s0)
    /* 4DACC 8005D2CC 02000224 */  addiu      $v0, $zero, 0x2
    /* 4DAD0 8005D2D0 FCFF11A6 */  sh         $s1, -0x4($s0)
    /* 4DAD4 8005D2D4 FEFF02A6 */  sh         $v0, -0x2($s0)
    /* 4DAD8 8005D2D8 0A80013C */  lui        $at, %hi(D_8009B2BE)
    /* 4DADC 8005D2DC 21083400 */  addu       $at, $at, $s4
    /* 4DAE0 8005D2E0 BEB22294 */  lhu        $v0, %lo(D_8009B2BE)($at)
    /* 4DAE4 8005D2E4 21280000 */  addu       $a1, $zero, $zero
    /* 4DAE8 8005D2E8 02004224 */  addiu      $v0, $v0, 0x2
    /* 4DAEC 8005D2EC 5AEA010C */  jal        gpu_SetSemiTransp
    /* 4DAF0 8005D2F0 000002A6 */   sh        $v0, 0x0($s0)
    /* 4DAF4 8005D2F4 21286002 */  addu       $a1, $s3, $zero
    /* 4DAF8 8005D2F8 10001026 */  addiu      $s0, $s0, 0x10
    /* 4DAFC 8005D2FC 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 4DB00 8005D300 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 4DB04 8005D304 8000A88F */  lw         $t0, 0x80($sp)
    /* 4DB08 8005D308 10007326 */  addiu      $s3, $s3, 0x10
    /* 4DB0C 8005D30C 2DEA010C */  jal        ot_Link
    /* 4DB10 8005D310 21208800 */   addu      $a0, $a0, $t0
    /* 4DB14 8005D314 01004226 */  addiu      $v0, $s2, 0x1
    /* 4DB18 8005D318 21904000 */  addu       $s2, $v0, $zero
    /* 4DB1C 8005D31C 00140200 */  sll        $v0, $v0, 16
    /* 4DB20 8005D320 03140200 */  sra        $v0, $v0, 16
    /* 4DB24 8005D324 02004228 */  slti       $v0, $v0, 0x2
    /* 4DB28 8005D328 D0FF4014 */  bnez       $v0, .L8005D26C
    /* 4DB2C 8005D32C 0100C226 */   addiu     $v0, $s6, 0x1
    /* 4DB30 8005D330 21B04000 */  addu       $s6, $v0, $zero
    /* 4DB34 8005D334 00140200 */  sll        $v0, $v0, 16
    /* 4DB38 8005D338 03140200 */  sra        $v0, $v0, 16
    /* 4DB3C 8005D33C 02004228 */  slti       $v0, $v0, 0x2
    /* 4DB40 8005D340 65FF4014 */  bnez       $v0, .L8005D0D8
    /* 4DB44 8005D344 00000000 */   nop
    /* 4DB48 8005D348 B4EA010C */  jal        initTile
    /* 4DB4C 8005D34C 21206002 */   addu      $a0, $s3, $zero
    /* 4DB50 8005D350 040060A2 */  sb         $zero, 0x4($s3)
    /* 4DB54 8005D354 050060A2 */  sb         $zero, 0x5($s3)
    /* 4DB58 8005D358 060060A2 */  sb         $zero, 0x6($s3)
    /* 4DB5C 8005D35C 4800A88F */  lw         $t0, 0x48($sp)
    /* 4DB60 8005D360 00000000 */  nop
    /* 4DB64 8005D364 80280800 */  sll        $a1, $t0, 2
    /* 4DB68 8005D368 0A80013C */  lui        $at, %hi(D_8009B2BC)
    /* 4DB6C 8005D36C 21082500 */  addu       $at, $at, $a1
    /* 4DB70 8005D370 BCB22384 */  lh         $v1, %lo(D_8009B2BC)($at)
    /* 4DB74 8005D374 80020224 */  addiu      $v0, $zero, 0x280
    /* 4DB78 8005D378 23104300 */  subu       $v0, $v0, $v1
    /* 4DB7C 8005D37C C21F0200 */  srl        $v1, $v0, 31
    /* 4DB80 8005D380 21104300 */  addu       $v0, $v0, $v1
    /* 4DB84 8005D384 43100200 */  sra        $v0, $v0, 1
    /* 4DB88 8005D388 080062A6 */  sh         $v0, 0x8($s3)
    /* 4DB8C 8005D38C 0A80013C */  lui        $at, %hi(D_8009B2BE)
    /* 4DB90 8005D390 21082500 */  addu       $at, $at, $a1
    /* 4DB94 8005D394 BEB22384 */  lh         $v1, %lo(D_8009B2BE)($at)
    /* 4DB98 8005D398 F0000224 */  addiu      $v0, $zero, 0xF0
    /* 4DB9C 8005D39C 23104300 */  subu       $v0, $v0, $v1
    /* 4DBA0 8005D3A0 C21F0200 */  srl        $v1, $v0, 31
    /* 4DBA4 8005D3A4 21104300 */  addu       $v0, $v0, $v1
    /* 4DBA8 8005D3A8 43100200 */  sra        $v0, $v0, 1
    /* 4DBAC 8005D3AC 0A0062A6 */  sh         $v0, 0xA($s3)
    /* 4DBB0 8005D3B0 0A80013C */  lui        $at, %hi(D_8009B2BC)
    /* 4DBB4 8005D3B4 21082500 */  addu       $at, $at, $a1
    /* 4DBB8 8005D3B8 BCB22294 */  lhu        $v0, %lo(D_8009B2BC)($at)
    /* 4DBBC 8005D3BC 21206002 */  addu       $a0, $s3, $zero
    /* 4DBC0 8005D3C0 0C0062A6 */  sh         $v0, 0xC($s3)
    /* 4DBC4 8005D3C4 0A80013C */  lui        $at, %hi(D_8009B2BE)
    /* 4DBC8 8005D3C8 21082500 */  addu       $at, $at, $a1
    /* 4DBCC 8005D3CC BEB22294 */  lhu        $v0, %lo(D_8009B2BE)($at)
    /* 4DBD0 8005D3D0 01000524 */  addiu      $a1, $zero, 0x1
    /* 4DBD4 8005D3D4 5AEA010C */  jal        gpu_SetSemiTransp
    /* 4DBD8 8005D3D8 0E0062A6 */   sh        $v0, 0xE($s3)
    /* 4DBDC 8005D3DC 21286002 */  addu       $a1, $s3, $zero
    /* 4DBE0 8005D3E0 5000A88F */  lw         $t0, 0x50($sp)
    /* 4DBE4 8005D3E4 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 4DBE8 8005D3E8 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 4DBEC 8005D3EC 80800800 */  sll        $s0, $t0, 2
    /* 4DBF0 8005D3F0 2DEA010C */  jal        ot_Link
    /* 4DBF4 8005D3F4 21209000 */   addu      $a0, $a0, $s0
    /* 4DBF8 8005D3F8 0A80043C */  lui        $a0, %hi(D_8009B0E0)
    /* 4DBFC 8005D3FC E0B08424 */  addiu      $a0, $a0, %lo(D_8009B0E0)
    /* 4DC00 8005D400 20B9010C */  jal        func_8006E480
    /* 4DC04 8005D404 21280000 */   addu      $a1, $zero, $zero
    /* 4DC08 8005D408 01000524 */  addiu      $a1, $zero, 0x1
    /* 4DC0C 8005D40C 21300000 */  addu       $a2, $zero, $zero
    /* 4DC10 8005D410 6800A48F */  lw         $a0, 0x68($sp)
    /* 4DC14 8005D414 21384000 */  addu       $a3, $v0, $zero
    /* 4DC18 8005D418 92F0010C */  jal        initTexPage
    /* 4DC1C 8005D41C 1000A0AF */   sw        $zero, 0x10($sp)
    /* 4DC20 8005D420 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 4DC24 8005D424 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 4DC28 8005D428 6800A58F */  lw         $a1, 0x68($sp)
    /* 4DC2C 8005D42C 2DEA010C */  jal        ot_Link
    /* 4DC30 8005D430 21209000 */   addu      $a0, $a0, $s0
    /* 4DC34 8005D434 7000A28F */  lw         $v0, 0x70($sp)
    /* 4DC38 8005D438 B400BF8F */  lw         $ra, 0xB4($sp)
    /* 4DC3C 8005D43C B000BE8F */  lw         $fp, 0xB0($sp)
    /* 4DC40 8005D440 AC00B78F */  lw         $s7, 0xAC($sp)
    /* 4DC44 8005D444 A800B68F */  lw         $s6, 0xA8($sp)
    /* 4DC48 8005D448 A400B58F */  lw         $s5, 0xA4($sp)
    /* 4DC4C 8005D44C A000B48F */  lw         $s4, 0xA0($sp)
    /* 4DC50 8005D450 9C00B38F */  lw         $s3, 0x9C($sp)
    /* 4DC54 8005D454 9800B28F */  lw         $s2, 0x98($sp)
    /* 4DC58 8005D458 9400B18F */  lw         $s1, 0x94($sp)
    /* 4DC5C 8005D45C 9000B08F */  lw         $s0, 0x90($sp)
    /* 4DC60 8005D460 B800BD27 */  addiu      $sp, $sp, 0xB8
    /* 4DC64 8005D464 0800E003 */  jr         $ra
    /* 4DC68 8005D468 00000000 */   nop
endlabel func_8005C8A8
