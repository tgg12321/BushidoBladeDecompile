glabel func_8006F100
    /* 5F900 8006F100 84048287 */  lh         $v0, %gp_rel(D_800A3550)($gp)
    /* 5F904 8006F104 80FFBD27 */  addiu      $sp, $sp, -0x80
    /* 5F908 8006F108 6000B2AF */  sw         $s2, 0x60($sp)
    /* 5F90C 8006F10C 21908000 */  addu       $s2, $a0, $zero
    /* 5F910 8006F110 7800BFAF */  sw         $ra, 0x78($sp)
    /* 5F914 8006F114 7400B7AF */  sw         $s7, 0x74($sp)
    /* 5F918 8006F118 7000B6AF */  sw         $s6, 0x70($sp)
    /* 5F91C 8006F11C 6C00B5AF */  sw         $s5, 0x6C($sp)
    /* 5F920 8006F120 6800B4AF */  sw         $s4, 0x68($sp)
    /* 5F924 8006F124 6400B3AF */  sw         $s3, 0x64($sp)
    /* 5F928 8006F128 5C00B1AF */  sw         $s1, 0x5C($sp)
    /* 5F92C 8006F12C 5800B0AF */  sw         $s0, 0x58($sp)
    /* 5F930 8006F130 18004010 */  beqz       $v0, .L8006F194
    /* 5F934 8006F134 21184000 */   addu      $v1, $v0, $zero
    /* 5F938 8006F138 10006224 */  addiu      $v0, $v1, 0x10
    /* 5F93C 8006F13C 840482A7 */  sh         $v0, %gp_rel(D_800A3550)($gp)
    /* 5F940 8006F140 00140200 */  sll        $v0, $v0, 16
    /* 5F944 8006F144 03140200 */  sra        $v0, $v0, 16
    /* 5F948 8006F148 00014228 */  slti       $v0, $v0, 0x100
    /* 5F94C 8006F14C 07004014 */  bnez       $v0, .L8006F16C
    /* 5F950 8006F150 FF000224 */   addiu     $v0, $zero, 0xFF
    /* 5F954 8006F154 840482A7 */  sh         $v0, %gp_rel(D_800A3550)($gp)
    /* 5F958 8006F158 0EBC010C */  jal        func_8006F038
    /* 5F95C 8006F15C 00000000 */   nop
    /* 5F960 8006F160 840480A7 */  sh         $zero, %gp_rel(D_800A3550)($gp)
    /* 5F964 8006F164 5DBC0108 */  j          .L8006F174
    /* 5F968 8006F168 00000000 */   nop
  .L8006F16C:
    /* 5F96C 8006F16C 0EBC010C */  jal        func_8006F038
    /* 5F970 8006F170 21204002 */   addu      $a0, $s2, $zero
  .L8006F174:
    /* 5F974 8006F174 3DBB010C */  jal        func_8006ECF4
    /* 5F978 8006F178 21204002 */   addu      $a0, $s2, $zero
    /* 5F97C 8006F17C 84CB010C */  jal        func_80072E10
    /* 5F980 8006F180 21204002 */   addu      $a0, $s2, $zero
    /* 5F984 8006F184 80CC010C */  jal        func_80073200
    /* 5F988 8006F188 21204002 */   addu      $a0, $s2, $zero
    /* 5F98C 8006F18C 3EBD0108 */  j          .L8006F4F8
    /* 5F990 8006F190 00000000 */   nop
  .L8006F194:
    /* 5F994 8006F194 90048287 */  lh         $v0, %gp_rel(D_800A355C)($gp)
    /* 5F998 8006F198 00000000 */  nop
    /* 5F99C 8006F19C 79004228 */  slti       $v0, $v0, 0x79
    /* 5F9A0 8006F1A0 02004014 */  bnez       $v0, .L8006F1AC
    /* 5F9A4 8006F1A4 03000224 */   addiu     $v0, $zero, 0x3
    /* 5F9A8 8006F1A8 B80482A7 */  sh         $v0, %gp_rel(D_800A3584)($gp)
  .L8006F1AC:
    /* 5F9AC 8006F1AC 8C048387 */  lh         $v1, %gp_rel(D_800A3558)($gp)
    /* 5F9B0 8006F1B0 00010224 */  addiu      $v0, $zero, 0x100
    /* 5F9B4 8006F1B4 3000A2AF */  sw         $v0, 0x30($sp)
    /* 5F9B8 8006F1B8 3400A2AF */  sw         $v0, 0x34($sp)
    /* 5F9BC 8006F1BC 13000224 */  addiu      $v0, $zero, 0x13
    /* 5F9C0 8006F1C0 2400A2AF */  sw         $v0, 0x24($sp)
    /* 5F9C4 8006F1C4 E404828F */  lw         $v0, %gp_rel(D_800A35B0)($gp)
    /* 5F9C8 8006F1C8 DC04848F */  lw         $a0, %gp_rel(D_800A35A8)($gp)
    /* 5F9CC 8006F1CC 2000A0AF */  sw         $zero, 0x20($sp)
    /* 5F9D0 8006F1D0 3800A0A3 */  sb         $zero, 0x38($sp)
    /* 5F9D4 8006F1D4 01006324 */  addiu      $v1, $v1, 0x1
    /* 5F9D8 8006F1D8 21104300 */  addu       $v0, $v0, $v1
    /* 5F9DC 8006F1DC 5800978C */  lw         $s7, 0x58($a0)
    /* 5F9E0 8006F1E0 C1004018 */  blez       $v0, .L8006F4E8
    /* 5F9E4 8006F1E4 21880000 */   addu      $s1, $zero, $zero
    /* 5F9E8 8006F1E8 6666163C */  lui        $s6, (0x66666667 >> 16)
    /* 5F9EC 8006F1EC 6766D636 */  ori        $s6, $s6, (0x66666667 & 0xFFFF)
    /* 5F9F0 8006F1F0 0A80103C */  lui        $s0, %hi(D_800A35C8)
    /* 5F9F4 8006F1F4 C8351026 */  addiu      $s0, $s0, %lo(D_800A35C8)
    /* 5F9F8 8006F1F8 21A80000 */  addu       $s5, $zero, $zero
    /* 5F9FC 8006F1FC 21A00000 */  addu       $s4, $zero, $zero
    /* 5FA00 8006F200 21980000 */  addu       $s3, $zero, $zero
  .L8006F204:
    /* 5FA04 8006F204 02000424 */  addiu      $a0, $zero, 0x2
    /* 5FA08 8006F208 0A80013C */  lui        $at, %hi(D_800A3562)
    /* 5FA0C 8006F20C 21083300 */  addu       $at, $at, $s3
    /* 5FA10 8006F210 62352290 */  lbu        $v0, %lo(D_800A3562)($at)
    /* 5FA14 8006F214 F004838F */  lw         $v1, %gp_rel(D_800A35BC)($gp)
    /* 5FA18 8006F218 80100200 */  sll        $v0, $v0, 2
    /* 5FA1C 8006F21C 21105700 */  addu       $v0, $v0, $s7
    /* 5FA20 8006F220 0000478C */  lw         $a3, 0x0($v0)
    /* 5FA24 8006F224 12006414 */  bne        $v1, $a0, .L8006F270
    /* 5FA28 8006F228 01000524 */   addiu     $a1, $zero, 0x1
    /* 5FA2C 8006F22C 9C04828F */  lw         $v0, %gp_rel(D_800A3568)($gp)
    /* 5FA30 8006F230 00000000 */  nop
    /* 5FA34 8006F234 1400428C */  lw         $v0, 0x14($v0)
    /* 5FA38 8006F238 0200033C */  lui        $v1, (0x20000 >> 16)
    /* 5FA3C 8006F23C 24104300 */  and        $v0, $v0, $v1
    /* 5FA40 8006F240 0B004014 */  bnez       $v0, .L8006F270
    /* 5FA44 8006F244 00000000 */   nop
    /* 5FA48 8006F248 09002516 */  bne        $s1, $a1, .L8006F270
    /* 5FA4C 8006F24C 00000000 */   nop
    /* 5FA50 8006F250 95048293 */  lbu        $v0, %gp_rel(D_800A3561)($gp)
    /* 5FA54 8006F254 0A80013C */  lui        $at, %hi(D_8009BC7C)
    /* 5FA58 8006F258 21082200 */  addu       $at, $at, $v0
    /* 5FA5C 8006F25C 7CBC2290 */  lbu        $v0, %lo(D_8009BC7C)($at)
    /* 5FA60 8006F260 00000000 */  nop
    /* 5FA64 8006F264 02004230 */  andi       $v0, $v0, 0x2
    /* 5FA68 8006F268 A7BC0108 */  j          .L8006F29C
    /* 5FA6C 8006F26C 2B280200 */   sltu      $a1, $zero, $v0
  .L8006F270:
    /* 5FA70 8006F270 0A80013C */  lui        $at, %hi(D_800A3561)
    /* 5FA74 8006F274 21083300 */  addu       $at, $at, $s3
    /* 5FA78 8006F278 61352290 */  lbu        $v0, %lo(D_800A3561)($at)
    /* 5FA7C 8006F27C 0A80013C */  lui        $at, %hi(D_8009BC7C)
    /* 5FA80 8006F280 21082200 */  addu       $at, $at, $v0
    /* 5FA84 8006F284 7CBC2290 */  lbu        $v0, %lo(D_8009BC7C)($at)
    /* 5FA88 8006F288 00000000 */  nop
    /* 5FA8C 8006F28C 02004230 */  andi       $v0, $v0, 0x2
    /* 5FA90 8006F290 03004010 */  beqz       $v0, .L8006F2A0
    /* 5FA94 8006F294 40100500 */   sll       $v0, $a1, 1
    /* 5FA98 8006F298 21280000 */  addu       $a1, $zero, $zero
  .L8006F29C:
    /* 5FA9C 8006F29C 40100500 */  sll        $v0, $a1, 1
  .L8006F2A0:
    /* 5FAA0 8006F2A0 21104500 */  addu       $v0, $v0, $a1
    /* 5FAA4 8006F2A4 80100200 */  sll        $v0, $v0, 2
    /* 5FAA8 8006F2A8 2118E200 */  addu       $v1, $a3, $v0
    /* 5FAAC 8006F2AC 04002012 */  beqz       $s1, .L8006F2C0
    /* 5FAB0 8006F2B0 1000A3AF */   sw        $v1, 0x10($sp)
    /* 5FAB4 8006F2B4 40000224 */  addiu      $v0, $zero, 0x40
    /* 5FAB8 8006F2B8 B1BC0108 */  j          .L8006F2C4
    /* 5FABC 8006F2BC 080062A4 */   sh        $v0, 0x8($v1)
  .L8006F2C0:
    /* 5FAC0 8006F2C0 080060A4 */  sh         $zero, 0x8($v1)
  .L8006F2C4:
    /* 5FAC4 8006F2C4 1800E624 */  addiu      $a2, $a3, 0x18
    /* 5FAC8 8006F2C8 1400A6AF */  sw         $a2, 0x14($sp)
    /* 5FACC 8006F2CC 00000586 */  lh         $a1, 0x0($s0)
    /* 5FAD0 8006F2D0 00000000 */  nop
    /* 5FAD4 8006F2D4 40100500 */  sll        $v0, $a1, 1
    /* 5FAD8 8006F2D8 21104500 */  addu       $v0, $v0, $a1
    /* 5FADC 8006F2DC C0100200 */  sll        $v0, $v0, 3
    /* 5FAE0 8006F2E0 21104500 */  addu       $v0, $v0, $a1
    /* 5FAE4 8006F2E4 40110200 */  sll        $v0, $v0, 5
    /* 5FAE8 8006F2E8 23100200 */  negu       $v0, $v0
    /* 5FAEC 8006F2EC 18005600 */  mult       $v0, $s6
    /* 5FAF0 8006F2F0 1000A38F */  lw         $v1, 0x10($sp)
    /* 5FAF4 8006F2F4 C3170200 */  sra        $v0, $v0, 31
    /* 5FAF8 8006F2F8 10580000 */  mfhi       $t3
    /* 5FAFC 8006F2FC C3200B00 */  sra        $a0, $t3, 3
    /* 5FB00 8006F300 23408200 */  subu       $t0, $a0, $v0
    /* 5FB04 8006F304 21480001 */  addu       $t1, $t0, $zero
    /* 5FB08 8006F308 80100500 */  sll        $v0, $a1, 2
    /* 5FB0C 8006F30C 21104500 */  addu       $v0, $v0, $a1
    /* 5FB10 8006F310 80100200 */  sll        $v0, $v0, 2
    /* 5FB14 8006F314 21104500 */  addu       $v0, $v0, $a1
    /* 5FB18 8006F318 80100200 */  sll        $v0, $v0, 2
    /* 5FB1C 8006F31C 23104500 */  subu       $v0, $v0, $a1
    /* 5FB20 8006F320 C0100200 */  sll        $v0, $v0, 3
    /* 5FB24 8006F324 23100200 */  negu       $v0, $v0
    /* 5FB28 8006F328 02006490 */  lbu        $a0, 0x2($v1)
    /* 5FB2C 8006F32C 18005600 */  mult       $v0, $s6
    /* 5FB30 8006F330 C3170200 */  sra        $v0, $v0, 31
    /* 5FB34 8006F334 FFFF8424 */  addiu      $a0, $a0, -0x1
    /* 5FB38 8006F338 C0200400 */  sll        $a0, $a0, 3
    /* 5FB3C 8006F33C 21208600 */  addu       $a0, $a0, $a2
    /* 5FB40 8006F340 06008690 */  lbu        $a2, 0x6($a0)
    /* 5FB44 8006F344 00008394 */  lhu        $v1, 0x0($a0)
    /* 5FB48 8006F348 1800E594 */  lhu        $a1, 0x18($a3)
    /* 5FB4C 8006F34C 21186600 */  addu       $v1, $v1, $a2
    /* 5FB50 8006F350 23306500 */  subu       $a2, $v1, $a1
    /* 5FB54 8006F354 07008590 */  lbu        $a1, 0x7($a0)
    /* 5FB58 8006F358 02008394 */  lhu        $v1, 0x2($a0)
    /* 5FB5C 8006F35C 1A00E494 */  lhu        $a0, 0x1A($a3)
    /* 5FB60 8006F360 21186500 */  addu       $v1, $v1, $a1
    /* 5FB64 8006F364 23386400 */  subu       $a3, $v1, $a0
    /* 5FB68 8006F368 10580000 */  mfhi       $t3
    /* 5FB6C 8006F36C C3180B00 */  sra        $v1, $t3, 3
    /* 5FB70 8006F370 02002012 */  beqz       $s1, .L8006F37C
    /* 5FB74 8006F374 23506200 */   subu      $t2, $v1, $v0
    /* 5FB78 8006F378 23480800 */  negu       $t1, $t0
  .L8006F37C:
    /* 5FB7C 8006F37C 00240600 */  sll        $a0, $a2, 16
    /* 5FB80 8006F380 03240400 */  sra        $a0, $a0, 16
    /* 5FB84 8006F384 3000A28F */  lw         $v0, 0x30($sp)
    /* 5FB88 8006F388 0A80063C */  lui        $a2, %hi(D_800A3590)
    /* 5FB8C 8006F38C 9035C624 */  addiu      $a2, $a2, %lo(D_800A3590)
    /* 5FB90 8006F390 18008200 */  mult       $a0, $v0
    /* 5FB94 8006F394 2130A602 */  addu       $a2, $s5, $a2
    /* 5FB98 8006F398 0000C384 */  lh         $v1, 0x0($a2)
    /* 5FB9C 8006F39C 00000000 */  nop
    /* 5FBA0 8006F3A0 80180300 */  sll        $v1, $v1, 2
    /* 5FBA4 8006F3A4 21187400 */  addu       $v1, $v1, $s4
    /* 5FBA8 8006F3A8 00140900 */  sll        $v0, $t1, 16
    /* 5FBAC 8006F3AC 03140200 */  sra        $v0, $v0, 16
    /* 5FBB0 8006F3B0 40014224 */  addiu      $v0, $v0, 0x140
    /* 5FBB4 8006F3B4 0A80013C */  lui        $at, %hi(D_8009BC94)
    /* 5FBB8 8006F3B8 21082300 */  addu       $at, $at, $v1
    /* 5FBBC 8006F3BC 94BC2584 */  lh         $a1, %lo(D_8009BC94)($at)
    /* 5FBC0 8006F3C0 3400A38F */  lw         $v1, 0x34($sp)
    /* 5FBC4 8006F3C4 2128A200 */  addu       $a1, $a1, $v0
    /* 5FBC8 8006F3C8 12200000 */  mflo       $a0
    /* 5FBCC 8006F3CC 03120400 */  sra        $v0, $a0, 8
    /* 5FBD0 8006F3D0 C2270400 */  srl        $a0, $a0, 31
    /* 5FBD4 8006F3D4 21104400 */  addu       $v0, $v0, $a0
    /* 5FBD8 8006F3D8 00240700 */  sll        $a0, $a3, 16
    /* 5FBDC 8006F3DC 03240400 */  sra        $a0, $a0, 16
    /* 5FBE0 8006F3E0 18008300 */  mult       $a0, $v1
    /* 5FBE4 8006F3E4 43100200 */  sra        $v0, $v0, 1
    /* 5FBE8 8006F3E8 2328A200 */  subu       $a1, $a1, $v0
    /* 5FBEC 8006F3EC 00140A00 */  sll        $v0, $t2, 16
    /* 5FBF0 8006F3F0 2800A5AF */  sw         $a1, 0x28($sp)
    /* 5FBF4 8006F3F4 0000C384 */  lh         $v1, 0x0($a2)
    /* 5FBF8 8006F3F8 03140200 */  sra        $v0, $v0, 16
    /* 5FBFC 8006F3FC 80180300 */  sll        $v1, $v1, 2
    /* 5FC00 8006F400 21187400 */  addu       $v1, $v1, $s4
    /* 5FC04 8006F404 0A80013C */  lui        $at, %hi(D_8009BC96)
    /* 5FC08 8006F408 21082300 */  addu       $at, $at, $v1
    /* 5FC0C 8006F40C 96BC2384 */  lh         $v1, %lo(D_8009BC96)($at)
    /* 5FC10 8006F410 9D004224 */  addiu      $v0, $v0, 0x9D
    /* 5FC14 8006F414 21186200 */  addu       $v1, $v1, $v0
    /* 5FC18 8006F418 12200000 */  mflo       $a0
    /* 5FC1C 8006F41C 03120400 */  sra        $v0, $a0, 8
    /* 5FC20 8006F420 C2270400 */  srl        $a0, $a0, 31
    /* 5FC24 8006F424 21104400 */  addu       $v0, $v0, $a0
    /* 5FC28 8006F428 43100200 */  sra        $v0, $v0, 1
    /* 5FC2C 8006F42C 23186200 */  subu       $v1, $v1, $v0
    /* 5FC30 8006F430 2C00A3AF */  sw         $v1, 0x2C($sp)
    /* 5FC34 8006F434 0400428E */  lw         $v0, 0x4($s2)
    /* 5FC38 8006F438 05002012 */  beqz       $s1, .L8006F450
    /* 5FC3C 8006F43C 1C00A2AF */   sw        $v0, 0x1C($sp)
    /* 5FC40 8006F440 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5FC44 8006F444 C0010524 */  addiu      $a1, $zero, 0x1C0
    /* 5FC48 8006F448 17BD0108 */  j          .L8006F45C
    /* 5FC4C 8006F44C 01000624 */   addiu     $a2, $zero, 0x1
  .L8006F450:
    /* 5FC50 8006F450 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5FC54 8006F454 400E0524 */  addiu      $a1, $zero, 0xE40
    /* 5FC58 8006F458 21300000 */  addu       $a2, $zero, $zero
  .L8006F45C:
    /* 5FC5C 8006F45C 1ECF010C */  jal        func_80073C78
    /* 5FC60 8006F460 00000000 */   nop
    /* 5FC64 8006F464 040042AE */  sw         $v0, 0x4($s2)
    /* 5FC68 8006F468 00000286 */  lh         $v0, 0x0($s0)
    /* 5FC6C 8006F46C 00000000 */  nop
    /* 5FC70 8006F470 03004018 */  blez       $v0, .L8006F480
    /* 5FC74 8006F474 21184000 */   addu      $v1, $v0, $zero
    /* 5FC78 8006F478 FFFF6224 */  addiu      $v0, $v1, -0x1
    /* 5FC7C 8006F47C 000002A6 */  sh         $v0, 0x0($s0)
  .L8006F480:
    /* 5FC80 8006F480 00000386 */  lh         $v1, 0x0($s0)
    /* 5FC84 8006F484 0A000224 */  addiu      $v0, $zero, 0xA
    /* 5FC88 8006F488 04006214 */  bne        $v1, $v0, .L8006F49C
    /* 5FC8C 8006F48C 07000424 */   addiu     $a0, $zero, 0x7
    /* 5FC90 8006F490 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5FC94 8006F494 9471010C */  jal        func_8005C650
    /* 5FC98 8006F498 7F000624 */   addiu     $a2, $zero, 0x7F
  .L8006F49C:
    /* 5FC9C 8006F49C 00000286 */  lh         $v0, 0x0($s0)
    /* 5FCA0 8006F4A0 00000000 */  nop
    /* 5FCA4 8006F4A4 05004104 */  bgez       $v0, .L8006F4BC
    /* 5FCA8 8006F4A8 08000424 */   addiu     $a0, $zero, 0x8
    /* 5FCAC 8006F4AC 000000A6 */  sh         $zero, 0x0($s0)
    /* 5FCB0 8006F4B0 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5FCB4 8006F4B4 9471010C */  jal        func_8005C650
    /* 5FCB8 8006F4B8 7F000624 */   addiu     $a2, $zero, 0x7F
  .L8006F4BC:
    /* 5FCBC 8006F4BC 02001026 */  addiu      $s0, $s0, 0x2
    /* 5FCC0 8006F4C0 0200B526 */  addiu      $s5, $s5, 0x2
    /* 5FCC4 8006F4C4 18009426 */  addiu      $s4, $s4, 0x18
    /* 5FCC8 8006F4C8 01003126 */  addiu      $s1, $s1, 0x1
    /* 5FCCC 8006F4CC 8C048387 */  lh         $v1, %gp_rel(D_800A3558)($gp)
    /* 5FCD0 8006F4D0 E404828F */  lw         $v0, %gp_rel(D_800A35B0)($gp)
    /* 5FCD4 8006F4D4 01006324 */  addiu      $v1, $v1, 0x1
    /* 5FCD8 8006F4D8 21104300 */  addu       $v0, $v0, $v1
    /* 5FCDC 8006F4DC 2A102202 */  slt        $v0, $s1, $v0
    /* 5FCE0 8006F4E0 48FF4014 */  bnez       $v0, .L8006F204
    /* 5FCE4 8006F4E4 03007326 */   addiu     $s3, $s3, 0x3
  .L8006F4E8:
    /* 5FCE8 8006F4E8 90048297 */  lhu        $v0, %gp_rel(D_800A355C)($gp)
    /* 5FCEC 8006F4EC 00000000 */  nop
    /* 5FCF0 8006F4F0 01004224 */  addiu      $v0, $v0, 0x1
    /* 5FCF4 8006F4F4 900482A7 */  sh         $v0, %gp_rel(D_800A355C)($gp)
  .L8006F4F8:
    /* 5FCF8 8006F4F8 7800BF8F */  lw         $ra, 0x78($sp)
    /* 5FCFC 8006F4FC 7400B78F */  lw         $s7, 0x74($sp)
    /* 5FD00 8006F500 7000B68F */  lw         $s6, 0x70($sp)
    /* 5FD04 8006F504 6C00B58F */  lw         $s5, 0x6C($sp)
    /* 5FD08 8006F508 6800B48F */  lw         $s4, 0x68($sp)
    /* 5FD0C 8006F50C 6400B38F */  lw         $s3, 0x64($sp)
    /* 5FD10 8006F510 6000B28F */  lw         $s2, 0x60($sp)
    /* 5FD14 8006F514 5C00B18F */  lw         $s1, 0x5C($sp)
    /* 5FD18 8006F518 5800B08F */  lw         $s0, 0x58($sp)
    /* 5FD1C 8006F51C 8000BD27 */  addiu      $sp, $sp, 0x80
    /* 5FD20 8006F520 0800E003 */  jr         $ra
    /* 5FD24 8006F524 00000000 */   nop
endlabel func_8006F100
