glabel func_8001C8DC
    /* D0DC 8001C8DC 1080023C */  lui        $v0, %hi(D_80101F5E)
    /* D0E0 8001C8E0 5E1F4284 */  lh         $v0, %lo(D_80101F5E)($v0)
    /* D0E4 8001C8E4 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* D0E8 8001C8E8 1800BFAF */  sw         $ra, 0x18($sp)
    /* D0EC 8001C8EC 1400B1AF */  sw         $s1, 0x14($sp)
    /* D0F0 8001C8F0 06004014 */  bnez       $v0, .L8001C90C
    /* D0F4 8001C8F4 1000B0AF */   sw        $s0, 0x10($sp)
    /* D0F8 8001C8F8 1080023C */  lui        $v0, %hi(D_801023AA)
    /* D0FC 8001C8FC AA234284 */  lh         $v0, %lo(D_801023AA)($v0)
    /* D100 8001C900 00000000 */  nop
    /* D104 8001C904 09004010 */  beqz       $v0, .L8001C92C
    /* D108 8001C908 00000000 */   nop
  .L8001C90C:
    /* D10C 8001C90C 0A80023C */  lui        $v0, %hi(D_800A382E)
    /* D110 8001C910 2E384294 */  lhu        $v0, %lo(D_800A382E)($v0)
    /* D114 8001C914 00000000 */  nop
    /* D118 8001C918 01004224 */  addiu      $v0, $v0, 0x1
    /* D11C 8001C91C 0A80013C */  lui        $at, %hi(D_800A382E)
    /* D120 8001C920 2E3822A4 */  sh         $v0, %lo(D_800A382E)($at)
    /* D124 8001C924 5C720008 */  j          .L8001C970
    /* D128 8001C928 00000000 */   nop
  .L8001C92C:
    /* D12C 8001C92C 0A80023C */  lui        $v0, %hi(D_800A38DC)
    /* D130 8001C930 DC384284 */  lh         $v0, %lo(D_800A38DC)($v0)
    /* D134 8001C934 00000000 */  nop
    /* D138 8001C938 0D004014 */  bnez       $v0, .L8001C970
    /* D13C 8001C93C 00000000 */   nop
    /* D140 8001C940 0A80023C */  lui        $v0, %hi(D_800A385C)
    /* D144 8001C944 5C38428C */  lw         $v0, %lo(D_800A385C)($v0)
    /* D148 8001C948 00000000 */  nop
    /* D14C 8001C94C 08004010 */  beqz       $v0, .L8001C970
    /* D150 8001C950 01000224 */   addiu     $v0, $zero, 0x1
    /* D154 8001C954 1080033C */  lui        $v1, %hi(D_80101F7A)
    /* D158 8001C958 7A1F6390 */  lbu        $v1, %lo(D_80101F7A)($v1)
    /* D15C 8001C95C 00000000 */  nop
    /* D160 8001C960 03006214 */  bne        $v1, $v0, .L8001C970
    /* D164 8001C964 00000000 */   nop
    /* D168 8001C968 5BED000C */  jal        func_8003B56C
    /* D16C 8001C96C 03000424 */   addiu     $a0, $zero, 0x3
  .L8001C970:
    /* D170 8001C970 0A80023C */  lui        $v0, %hi(D_800A382E)
    /* D174 8001C974 2E384284 */  lh         $v0, %lo(D_800A382E)($v0)
    /* D178 8001C978 00000000 */  nop
    /* D17C 8001C97C 3D004228 */  slti       $v0, $v0, 0x3D
    /* D180 8001C980 E3004014 */  bnez       $v0, .L8001CD10
    /* D184 8001C984 00000000 */   nop
    /* D188 8001C988 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* D18C 8001C98C DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* D190 8001C990 00000000 */  nop
    /* D194 8001C994 0700622C */  sltiu      $v0, $v1, 0x7
    /* D198 8001C998 DA004010 */  beqz       $v0, .L8001CD04
    /* D19C 8001C99C 80100300 */   sll       $v0, $v1, 2
    /* D1A0 8001C9A0 0180013C */  lui        $at, %hi(jtbl_800100C4)
    /* D1A4 8001C9A4 21082200 */  addu       $at, $at, $v0
    /* D1A8 8001C9A8 C400228C */  lw         $v0, %lo(jtbl_800100C4)($at)
    /* D1AC 8001C9AC 00000000 */  nop
    /* D1B0 8001C9B0 08004000 */  jr         $v0
    /* D1B4 8001C9B4 00000000 */   nop
  jlabel .L8001C9B8
    /* D1B8 8001C9B8 1080023C */  lui        $v0, %hi(D_80101F5E)
    /* D1BC 8001C9BC 5E1F4284 */  lh         $v0, %lo(D_80101F5E)($v0)
    /* D1C0 8001C9C0 00000000 */  nop
    /* D1C4 8001C9C4 D0004014 */  bnez       $v0, .L8001CD08
    /* D1C8 8001C9C8 04000224 */   addiu     $v0, $zero, 0x4
    /* D1CC 8001C9CC 0A80023C */  lui        $v0, %hi(D_800A3680)
    /* D1D0 8001C9D0 80364290 */  lbu        $v0, %lo(D_800A3680)($v0)
    /* D1D4 8001C9D4 00000000 */  nop
    /* D1D8 8001C9D8 2F004010 */  beqz       $v0, .L8001CA98
    /* D1DC 8001C9DC FFFF4224 */   addiu     $v0, $v0, -0x1
    /* D1E0 8001C9E0 0A80013C */  lui        $at, %hi(D_800A3680)
    /* D1E4 8001C9E4 803622A0 */  sb         $v0, %lo(D_800A3680)($at)
    /* D1E8 8001C9E8 FF004230 */  andi       $v0, $v0, 0xFF
    /* D1EC 8001C9EC 24004014 */  bnez       $v0, .L8001CA80
    /* D1F0 8001C9F0 00000000 */   nop
    /* D1F4 8001C9F4 636D010C */  jal        func_8005B58C
    /* D1F8 8001C9F8 00000000 */   nop
    /* D1FC 8001C9FC 0A80023C */  lui        $v0, %hi(D_800A37C6)
    /* D200 8001CA00 C6374290 */  lbu        $v0, %lo(D_800A37C6)($v0)
    /* D204 8001CA04 00000000 */  nop
    /* D208 8001CA08 05004010 */  beqz       $v0, .L8001CA20
    /* D20C 8001CA0C 04000224 */   addiu     $v0, $zero, 0x4
    /* D210 8001CA10 0A80013C */  lui        $at, %hi(D_800A37C6)
    /* D214 8001CA14 C63720A0 */  sb         $zero, %lo(D_800A37C6)($at)
    /* D218 8001CA18 42730008 */  j          .L8001CD08
    /* D21C 8001CA1C 00000000 */   nop
  .L8001CA20:
    /* D220 8001CA20 0A80043C */  lui        $a0, %hi(D_800A3894)
    /* D224 8001CA24 9438848C */  lw         $a0, %lo(D_800A3894)($a0)
    /* D228 8001CA28 00000000 */  nop
    /* D22C 8001CA2C 10008010 */  beqz       $a0, .L8001CA70
    /* D230 8001CA30 00000000 */   nop
    /* D234 8001CA34 21ED000C */  jal        func_8003B484
    /* D238 8001CA38 06008424 */   addiu     $a0, $a0, 0x6
    /* D23C 8001CA3C 0A80033C */  lui        $v1, %hi(D_800A3836)
    /* D240 8001CA40 36386390 */  lbu        $v1, %lo(D_800A3836)($v1)
    /* D244 8001CA44 FF000224 */  addiu      $v0, $zero, 0xFF
    /* D248 8001CA48 05006210 */  beq        $v1, $v0, .L8001CA60
    /* D24C 8001CA4C 00000000 */   nop
    /* D250 8001CA50 4DED000C */  jal        func_8003B534
    /* D254 8001CA54 02000424 */   addiu     $a0, $zero, 0x2
    /* D258 8001CA58 44730008 */  j          .L8001CD10
    /* D25C 8001CA5C 00000000 */   nop
  .L8001CA60:
    /* D260 8001CA60 4DED000C */  jal        func_8003B534
    /* D264 8001CA64 05000424 */   addiu     $a0, $zero, 0x5
    /* D268 8001CA68 44730008 */  j          .L8001CD10
    /* D26C 8001CA6C 00000000 */   nop
  .L8001CA70:
    /* D270 8001CA70 69ED000C */  jal        func_8003B5A4
    /* D274 8001CA74 00000000 */   nop
    /* D278 8001CA78 44730008 */  j          .L8001CD10
    /* D27C 8001CA7C 00000000 */   nop
  .L8001CA80:
    /* D280 8001CA80 4771000C */  jal        func_8001C51C
    /* D284 8001CA84 00000000 */   nop
    /* D288 8001CA88 0872000C */  jal        func_8001C820
    /* D28C 8001CA8C 00000000 */   nop
    /* D290 8001CA90 44730008 */  j          .L8001CD10
    /* D294 8001CA94 00000000 */   nop
  .L8001CA98:
    /* D298 8001CA98 0A80023C */  lui        $v0, %hi(D_800A385C)
    /* D29C 8001CA9C 5C38428C */  lw         $v0, %lo(D_800A385C)($v0)
    /* D2A0 8001CAA0 00000000 */  nop
    /* D2A4 8001CAA4 98004010 */  beqz       $v0, .L8001CD08
    /* D2A8 8001CAA8 04000224 */   addiu     $v0, $zero, 0x4
    /* D2AC 8001CAAC 5BED000C */  jal        func_8003B56C
    /* D2B0 8001CAB0 02000424 */   addiu     $a0, $zero, 0x2
    /* D2B4 8001CAB4 44730008 */  j          .L8001CD10
    /* D2B8 8001CAB8 00000000 */   nop
  jlabel .L8001CABC
    /* D2BC 8001CABC 1080023C */  lui        $v0, %hi(D_80101F5E)
    /* D2C0 8001CAC0 5E1F4284 */  lh         $v0, %lo(D_80101F5E)($v0)
    /* D2C4 8001CAC4 00000000 */  nop
    /* D2C8 8001CAC8 8F004014 */  bnez       $v0, .L8001CD08
    /* D2CC 8001CACC 04000224 */   addiu     $v0, $zero, 0x4
    /* D2D0 8001CAD0 7DCF000C */  jal        func_80033DF4
    /* D2D4 8001CAD4 00000000 */   nop
    /* D2D8 8001CAD8 8D004010 */  beqz       $v0, .L8001CD10
    /* D2DC 8001CADC 01001024 */   addiu     $s0, $zero, 0x1
    /* D2E0 8001CAE0 0A80013C */  lui        $at, %hi(D_800A390D)
    /* D2E4 8001CAE4 0D3930A0 */  sb         $s0, %lo(D_800A390D)($at)
    /* D2E8 8001CAE8 1A5A000C */  jal        gpu_EnableDisplay
    /* D2EC 8001CAEC 00000000 */   nop
    /* D2F0 8001CAF0 01000424 */  addiu      $a0, $zero, 0x1
    /* D2F4 8001CAF4 0A80053C */  lui        $a1, %hi(D_800A38DE)
    /* D2F8 8001CAF8 DE38A590 */  lbu        $a1, %lo(D_800A38DE)($a1)
    /* D2FC 8001CAFC 4401010C */  jal        func_80040510
    /* D300 8001CB00 21300000 */   addu      $a2, $zero, $zero
    /* D304 8001CB04 33000224 */  addiu      $v0, $zero, 0x33
    /* D308 8001CB08 0A80033C */  lui        $v1, %hi(D_800A38E2)
    /* D30C 8001CB0C E2386390 */  lbu        $v1, %lo(D_800A38E2)($v1)
    /* D310 8001CB10 0A80113C */  lui        $s1, %hi(D_800A38E0)
    /* D314 8001CB14 E0383192 */  lbu        $s1, %lo(D_800A38E0)($s1)
    /* D318 8001CB18 10006210 */  beq        $v1, $v0, .L8001CB5C
    /* D31C 8001CB1C 34006228 */   slti      $v0, $v1, 0x34
    /* D320 8001CB20 05004010 */  beqz       $v0, .L8001CB38
    /* D324 8001CB24 1F000224 */   addiu     $v0, $zero, 0x1F
    /* D328 8001CB28 08006210 */  beq        $v1, $v0, .L8001CB4C
    /* D32C 8001CB2C 00000000 */   nop
    /* D330 8001CB30 E1720008 */  j          .L8001CB84
    /* D334 8001CB34 00000000 */   nop
  .L8001CB38:
    /* D338 8001CB38 51000224 */  addiu      $v0, $zero, 0x51
    /* D33C 8001CB3C 0C006210 */  beq        $v1, $v0, .L8001CB70
    /* D340 8001CB40 03000224 */   addiu     $v0, $zero, 0x3
    /* D344 8001CB44 E1720008 */  j          .L8001CB84
    /* D348 8001CB48 00000000 */   nop
  .L8001CB4C:
    /* D34C 8001CB4C 0A80013C */  lui        $at, %hi(D_800A38E0)
    /* D350 8001CB50 E03830A0 */  sb         $s0, %lo(D_800A38E0)($at)
    /* D354 8001CB54 DF720008 */  j          .L8001CB7C
    /* D358 8001CB58 01000424 */   addiu     $a0, $zero, 0x1
  .L8001CB5C:
    /* D35C 8001CB5C 02000224 */  addiu      $v0, $zero, 0x2
    /* D360 8001CB60 0A80013C */  lui        $at, %hi(D_800A38E0)
    /* D364 8001CB64 E03822A0 */  sb         $v0, %lo(D_800A38E0)($at)
    /* D368 8001CB68 DF720008 */  j          .L8001CB7C
    /* D36C 8001CB6C 02000424 */   addiu     $a0, $zero, 0x2
  .L8001CB70:
    /* D370 8001CB70 0A80013C */  lui        $at, %hi(D_800A38E0)
    /* D374 8001CB74 E03822A0 */  sb         $v0, %lo(D_800A38E0)($at)
    /* D378 8001CB78 03000424 */  addiu      $a0, $zero, 0x3
  .L8001CB7C:
    /* D37C 8001CB7C 6719010C */  jal        func_8004659C
    /* D380 8001CB80 00000000 */   nop
  .L8001CB84:
    /* D384 8001CB84 0A80023C */  lui        $v0, %hi(D_800A38E0)
    /* D388 8001CB88 E0384290 */  lbu        $v0, %lo(D_800A38E0)($v0)
    /* D38C 8001CB8C 00000000 */  nop
    /* D390 8001CB90 26102202 */  xor        $v0, $s1, $v0
    /* D394 8001CB94 2B100200 */  sltu       $v0, $zero, $v0
    /* D398 8001CB98 0A80013C */  lui        $at, %hi(D_800A3728)
    /* D39C 8001CB9C 283722A0 */  sb         $v0, %lo(D_800A3728)($at)
    /* D3A0 8001CBA0 FF004230 */  andi       $v0, $v0, 0xFF
    /* D3A4 8001CBA4 0B004010 */  beqz       $v0, .L8001CBD4
    /* D3A8 8001CBA8 00000000 */   nop
    /* D3AC 8001CBAC 8971000C */  jal        func_8001C624
    /* D3B0 8001CBB0 00000000 */   nop
    /* D3B4 8001CBB4 4186000C */  jal        func_80021904
    /* D3B8 8001CBB8 21200000 */   addu      $a0, $zero, $zero
    /* D3BC 8001CBBC 1080013C */  lui        $at, %hi(D_80101F26)
    /* D3C0 8001CBC0 261F20A4 */  sh         $zero, %lo(D_80101F26)($at)
    /* D3C4 8001CBC4 21200000 */  addu       $a0, $zero, $zero
    /* D3C8 8001CBC8 21284000 */  addu       $a1, $v0, $zero
    /* D3CC 8001CBCC A686000C */  jal        func_80021A98
    /* D3D0 8001CBD0 21300000 */   addu      $a2, $zero, $zero
  .L8001CBD4:
    /* D3D4 8001CBD4 4771000C */  jal        func_8001C51C
    /* D3D8 8001CBD8 00000000 */   nop
    /* D3DC 8001CBDC 0A80023C */  lui        $v0, %hi(D_800A384C)
    /* D3E0 8001CBE0 4C384290 */  lbu        $v0, %lo(D_800A384C)($v0)
    /* D3E4 8001CBE4 00000000 */  nop
    /* D3E8 8001CBE8 0400422C */  sltiu      $v0, $v0, 0x4
    /* D3EC 8001CBEC 48004010 */  beqz       $v0, .L8001CD10
    /* D3F0 8001CBF0 00000000 */   nop
    /* D3F4 8001CBF4 0A80043C */  lui        $a0, %hi(D_800A38EC)
    /* D3F8 8001CBF8 EC388490 */  lbu        $a0, %lo(D_800A38EC)($a0)
    /* D3FC 8001CBFC 0A80053C */  lui        $a1, %hi(D_800A38ED)
    /* D400 8001CC00 ED38A590 */  lbu        $a1, %lo(D_800A38ED)($a1)
    /* D404 8001CC04 0A80063C */  lui        $a2, %hi(D_800A38EE)
    /* D408 8001CC08 EE38C690 */  lbu        $a2, %lo(D_800A38EE)($a2)
    /* D40C 8001CC0C FD06010C */  jal        func_80041BF4
    /* D410 8001CC10 00000000 */   nop
    /* D414 8001CC14 44730008 */  j          .L8001CD10
    /* D418 8001CC18 00000000 */   nop
  jlabel .L8001CC1C
    /* D41C 8001CC1C 1080033C */  lui        $v1, %hi(D_80101F5E)
    /* D420 8001CC20 5E1F6384 */  lh         $v1, %lo(D_80101F5E)($v1)
    /* D424 8001CC24 00000000 */  nop
    /* D428 8001CC28 06006010 */  beqz       $v1, .L8001CC44
    /* D42C 8001CC2C 00000000 */   nop
    /* D430 8001CC30 1080023C */  lui        $v0, %hi(D_801023AA)
    /* D434 8001CC34 AA234284 */  lh         $v0, %lo(D_801023AA)($v0)
    /* D438 8001CC38 00000000 */  nop
    /* D43C 8001CC3C 09004014 */  bnez       $v0, .L8001CC64
    /* D440 8001CC40 00000000 */   nop
  .L8001CC44:
    /* D444 8001CC44 0A80023C */  lui        $v0, %hi(D_800A37D2)
    /* D448 8001CC48 D2374224 */  addiu      $v0, $v0, %lo(D_800A37D2)
    /* D44C 8001CC4C 2B180300 */  sltu       $v1, $zero, $v1
    /* D450 8001CC50 21186200 */  addu       $v1, $v1, $v0
    /* D454 8001CC54 00006290 */  lbu        $v0, 0x0($v1)
    /* D458 8001CC58 00000000 */  nop
    /* D45C 8001CC5C 01004224 */  addiu      $v0, $v0, 0x1
    /* D460 8001CC60 000062A0 */  sb         $v0, 0x0($v1)
  .L8001CC64:
    /* D464 8001CC64 0A80033C */  lui        $v1, %hi(D_800A3748)
    /* D468 8001CC68 48376380 */  lb         $v1, %lo(D_800A3748)($v1)
    /* D46C 8001CC6C 01000224 */  addiu      $v0, $zero, 0x1
    /* D470 8001CC70 0A80013C */  lui        $at, %hi(D_800A3670)
    /* D474 8001CC74 703622A0 */  sb         $v0, %lo(D_800A3670)($at)
    /* D478 8001CC78 1080023C */  lui        $v0, %hi(D_80101FBC)
    /* D47C 8001CC7C BC1F4224 */  addiu      $v0, $v0, %lo(D_80101FBC)
    /* D480 8001CC80 00210300 */  sll        $a0, $v1, 4
    /* D484 8001CC84 21208300 */  addu       $a0, $a0, $v1
    /* D488 8001CC88 80200400 */  sll        $a0, $a0, 2
    /* D48C 8001CC8C 21208300 */  addu       $a0, $a0, $v1
    /* D490 8001CC90 80200400 */  sll        $a0, $a0, 2
    /* D494 8001CC94 23208300 */  subu       $a0, $a0, $v1
    /* D498 8001CC98 80200400 */  sll        $a0, $a0, 2
    /* D49C 8001CC9C 0289000C */  jal        func_80022408
    /* D4A0 8001CCA0 21208200 */   addu      $a0, $a0, $v0
    /* D4A4 8001CCA4 0A80013C */  lui        $at, %hi(D_800A38DF)
    /* D4A8 8001CCA8 DF3822A0 */  sb         $v0, %lo(D_800A38DF)($at)
    /* D4AC 8001CCAC 0A80013C */  lui        $at, %hi(D_800A3834)
    /* D4B0 8001CCB0 343820A4 */  sh         $zero, %lo(D_800A3834)($at)
    /* D4B4 8001CCB4 44730008 */  j          .L8001CD10
    /* D4B8 8001CCB8 00000000 */   nop
  jlabel .L8001CCBC
    /* D4BC 8001CCBC 1080033C */  lui        $v1, %hi(D_80101F5E)
    /* D4C0 8001CCC0 5E1F6384 */  lh         $v1, %lo(D_80101F5E)($v1)
    /* D4C4 8001CCC4 00000000 */  nop
    /* D4C8 8001CCC8 06006010 */  beqz       $v1, .L8001CCE4
    /* D4CC 8001CCCC 00000000 */   nop
    /* D4D0 8001CCD0 1080023C */  lui        $v0, %hi(D_801023AA)
    /* D4D4 8001CCD4 AA234284 */  lh         $v0, %lo(D_801023AA)($v0)
    /* D4D8 8001CCD8 00000000 */  nop
    /* D4DC 8001CCDC 0A004014 */  bnez       $v0, .L8001CD08
    /* D4E0 8001CCE0 04000224 */   addiu     $v0, $zero, 0x4
  .L8001CCE4:
    /* D4E4 8001CCE4 0A80023C */  lui        $v0, %hi(D_800A37D2)
    /* D4E8 8001CCE8 D2374224 */  addiu      $v0, $v0, %lo(D_800A37D2)
    /* D4EC 8001CCEC 2B180300 */  sltu       $v1, $zero, $v1
    /* D4F0 8001CCF0 21186200 */  addu       $v1, $v1, $v0
    /* D4F4 8001CCF4 00006290 */  lbu        $v0, 0x0($v1)
    /* D4F8 8001CCF8 00000000 */  nop
    /* D4FC 8001CCFC 01004224 */  addiu      $v0, $v0, 0x1
    /* D500 8001CD00 000062A0 */  sb         $v0, 0x0($v1)
  jlabel .L8001CD04
    /* D504 8001CD04 04000224 */  addiu      $v0, $zero, 0x4
  .L8001CD08:
    /* D508 8001CD08 0A80013C */  lui        $at, %hi(D_800A3834)
    /* D50C 8001CD0C 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
  jlabel .L8001CD10
    /* D510 8001CD10 0A80023C */  lui        $v0, %hi(D_800A37D2)
    /* D514 8001CD14 D2374290 */  lbu        $v0, %lo(D_800A37D2)($v0)
    /* D518 8001CD18 00000000 */  nop
    /* D51C 8001CD1C 6400422C */  sltiu      $v0, $v0, 0x64
    /* D520 8001CD20 03004014 */  bnez       $v0, .L8001CD30
    /* D524 8001CD24 63000224 */   addiu     $v0, $zero, 0x63
    /* D528 8001CD28 0A80013C */  lui        $at, %hi(D_800A37D2)
    /* D52C 8001CD2C D23722A0 */  sb         $v0, %lo(D_800A37D2)($at)
  .L8001CD30:
    /* D530 8001CD30 0A80023C */  lui        $v0, %hi(D_800A37D3)
    /* D534 8001CD34 D3374290 */  lbu        $v0, %lo(D_800A37D3)($v0)
    /* D538 8001CD38 00000000 */  nop
    /* D53C 8001CD3C 6400422C */  sltiu      $v0, $v0, 0x64
    /* D540 8001CD40 03004014 */  bnez       $v0, .L8001CD50
    /* D544 8001CD44 63000224 */   addiu     $v0, $zero, 0x63
    /* D548 8001CD48 0A80013C */  lui        $at, %hi(D_800A37D3)
    /* D54C 8001CD4C D33722A0 */  sb         $v0, %lo(D_800A37D3)($at)
  .L8001CD50:
    /* D550 8001CD50 1800BF8F */  lw         $ra, 0x18($sp)
    /* D554 8001CD54 1400B18F */  lw         $s1, 0x14($sp)
    /* D558 8001CD58 1000B08F */  lw         $s0, 0x10($sp)
    /* D55C 8001CD5C 2000BD27 */  addiu      $sp, $sp, 0x20
    /* D560 8001CD60 0800E003 */  jr         $ra
    /* D564 8001CD64 00000000 */   nop
endlabel func_8001C8DC
