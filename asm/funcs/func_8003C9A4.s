glabel func_8003C9A4
    /* 2D1A4 8003C9A4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2D1A8 8003C9A8 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2D1AC 8003C9AC 79FC000C */  jal        game_SetControllerPorts
    /* 2D1B0 8003C9B0 21200000 */   addu      $a0, $zero, $zero
    /* 2D1B4 8003C9B4 0F80043C */  lui        $a0, %hi(D_800F6608)
    /* 2D1B8 8003C9B8 08668424 */  addiu      $a0, $a0, %lo(D_800F6608)
    /* 2D1BC 8003C9BC 10008524 */  addiu      $a1, $a0, 0x10
    /* 2D1C0 8003C9C0 48F40224 */  addiu      $v0, $zero, -0xBB8
    /* 2D1C4 8003C9C4 000080AC */  sw         $zero, 0x0($a0)
    /* 2D1C8 8003C9C8 0F80013C */  lui        $at, %hi(D_800F660C)
    /* 2D1CC 8003C9CC 0C6622AC */  sw         $v0, %lo(D_800F660C)($at)
    /* 2D1D0 8003C9D0 20000224 */  addiu      $v0, $zero, 0x20
    /* 2D1D4 8003C9D4 0F80013C */  lui        $at, %hi(D_800F6610)
    /* 2D1D8 8003C9D8 106620AC */  sw         $zero, %lo(D_800F6610)($at)
    /* 2D1DC 8003C9DC 100082A4 */  sh         $v0, 0x10($a0)
    /* 2D1E0 8003C9E0 0A80023C */  lui        $v0, %hi(D_800A36AC)
    /* 2D1E4 8003C9E4 AC36428C */  lw         $v0, %lo(D_800A36AC)($v0)
    /* 2D1E8 8003C9E8 10270324 */  addiu      $v1, $zero, 0x2710
    /* 2D1EC 8003C9EC 0F80013C */  lui        $at, %hi(D_800F661C)
    /* 2D1F0 8003C9F0 1C6620A4 */  sh         $zero, %lo(D_800F661C)($at)
    /* 2D1F4 8003C9F4 0F80013C */  lui        $at, %hi(D_800F6620)
    /* 2D1F8 8003C9F8 206623AC */  sw         $v1, %lo(D_800F6620)($at)
    /* 2D1FC 8003C9FC 80100200 */  sll        $v0, $v0, 2
    /* 2D200 8003CA00 0F80013C */  lui        $at, %hi(D_800F661A)
    /* 2D204 8003CA04 1A6622A4 */  sh         $v0, %lo(D_800F661A)($at)
    /* 2D208 8003CA08 FD1A010C */  jal        func_80046BF4
    /* 2D20C 8003CA0C 10270624 */   addiu     $a2, $zero, 0x2710
    /* 2D210 8003CA10 6A1B010C */  jal        game_StageInit
    /* 2D214 8003CA14 01000424 */   addiu     $a0, $zero, 0x1
    /* 2D218 8003CA18 0A80023C */  lui        $v0, %hi(D_800A3929)
    /* 2D21C 8003CA1C 29394290 */  lbu        $v0, %lo(D_800A3929)($v0)
    /* 2D220 8003CA20 00000000 */  nop
    /* 2D224 8003CA24 48004014 */  bnez       $v0, .L8003CB48
    /* 2D228 8003CA28 01000424 */   addiu     $a0, $zero, 0x1
    /* 2D22C 8003CA2C 0A80053C */  lui        $a1, %hi(D_800A3817)
    /* 2D230 8003CA30 1738A590 */  lbu        $a1, %lo(D_800A3817)($a1)
    /* 2D234 8003CA34 0A80063C */  lui        $a2, %hi(D_800A38B4)
    /* 2D238 8003CA38 B438C68C */  lw         $a2, %lo(D_800A38B4)($a2)
    /* 2D23C 8003CA3C 2A72010C */  jal        func_8005C8A8
    /* 2D240 8003CA40 21380000 */   addu      $a3, $zero, $zero
    /* 2D244 8003CA44 02004104 */  bgez       $v0, .L8003CA50
    /* 2D248 8003CA48 0010043C */   lui       $a0, (0x10001000 >> 16)
    /* 2D24C 8003CA4C 03004224 */  addiu      $v0, $v0, 0x3
  .L8003CA50:
    /* 2D250 8003CA50 00108434 */  ori        $a0, $a0, (0x10001000 & 0xFFFF)
    /* 2D254 8003CA54 83180200 */  sra        $v1, $v0, 2
    /* 2D258 8003CA58 80180300 */  sll        $v1, $v1, 2
    /* 2D25C 8003CA5C 0A80023C */  lui        $v0, %hi(D_800A38B4)
    /* 2D260 8003CA60 B438428C */  lw         $v0, %lo(D_800A38B4)($v0)
    /* 2D264 8003CA64 1080053C */  lui        $a1, %hi(D_80102794)
    /* 2D268 8003CA68 9427A58C */  lw         $a1, %lo(D_80102794)($a1)
    /* 2D26C 8003CA6C 21104300 */  addu       $v0, $v0, $v1
    /* 2D270 8003CA70 2420A400 */  and        $a0, $a1, $a0
    /* 2D274 8003CA74 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 2D278 8003CA78 B43822AC */  sw         $v0, %lo(D_800A38B4)($at)
    /* 2D27C 8003CA7C 0B008010 */  beqz       $a0, .L8003CAAC
    /* 2D280 8003CA80 21200000 */   addu      $a0, $zero, $zero
    /* 2D284 8003CA84 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 2D288 8003CA88 9471010C */  jal        func_8005C650
    /* 2D28C 8003CA8C 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 2D290 8003CA90 0A80023C */  lui        $v0, %hi(D_800A3817)
    /* 2D294 8003CA94 17384290 */  lbu        $v0, %lo(D_800A3817)($v0)
    /* 2D298 8003CA98 00000000 */  nop
    /* 2D29C 8003CA9C 14004014 */  bnez       $v0, .L8003CAF0
    /* 2D2A0 8003CAA0 FFFF4224 */   addiu     $v0, $v0, -0x1
    /* 2D2A4 8003CAA4 BCF20008 */  j          .L8003CAF0
    /* 2D2A8 8003CAA8 02000224 */   addiu     $v0, $zero, 0x2
  .L8003CAAC:
    /* 2D2AC 8003CAAC 0040023C */  lui        $v0, (0x40004000 >> 16)
    /* 2D2B0 8003CAB0 00404234 */  ori        $v0, $v0, (0x40004000 & 0xFFFF)
    /* 2D2B4 8003CAB4 2410A200 */  and        $v0, $a1, $v0
    /* 2D2B8 8003CAB8 0F004010 */  beqz       $v0, .L8003CAF8
    /* 2D2BC 8003CABC 7F000524 */   addiu     $a1, $zero, 0x7F
    /* 2D2C0 8003CAC0 9471010C */  jal        func_8005C650
    /* 2D2C4 8003CAC4 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 2D2C8 8003CAC8 0A80043C */  lui        $a0, %hi(D_800A3817)
    /* 2D2CC 8003CACC 17388490 */  lbu        $a0, %lo(D_800A3817)($a0)
    /* 2D2D0 8003CAD0 02000224 */  addiu      $v0, $zero, 0x2
    /* 2D2D4 8003CAD4 FF008330 */  andi       $v1, $a0, 0xFF
    /* 2D2D8 8003CAD8 05006214 */  bne        $v1, $v0, .L8003CAF0
    /* 2D2DC 8003CADC 01008224 */   addiu     $v0, $a0, 0x1
    /* 2D2E0 8003CAE0 0A80013C */  lui        $at, %hi(D_800A3817)
    /* 2D2E4 8003CAE4 173820A0 */  sb         $zero, %lo(D_800A3817)($at)
    /* 2D2E8 8003CAE8 BFF20008 */  j          .L8003CAFC
    /* 2D2EC 8003CAEC 4000033C */   lui       $v1, (0x400040 >> 16)
  .L8003CAF0:
    /* 2D2F0 8003CAF0 0A80013C */  lui        $at, %hi(D_800A3817)
    /* 2D2F4 8003CAF4 173822A0 */  sb         $v0, %lo(D_800A3817)($at)
  .L8003CAF8:
    /* 2D2F8 8003CAF8 4000033C */  lui        $v1, (0x400040 >> 16)
  .L8003CAFC:
    /* 2D2FC 8003CAFC 1080023C */  lui        $v0, %hi(D_80102794)
    /* 2D300 8003CB00 9427428C */  lw         $v0, %lo(D_80102794)($v0)
    /* 2D304 8003CB04 40006334 */  ori        $v1, $v1, (0x400040 & 0xFFFF)
    /* 2D308 8003CB08 24104300 */  and        $v0, $v0, $v1
    /* 2D30C 8003CB0C 6B004010 */  beqz       $v0, .L8003CCBC
    /* 2D310 8003CB10 01000424 */   addiu     $a0, $zero, 0x1
    /* 2D314 8003CB14 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 2D318 8003CB18 9471010C */  jal        func_8005C650
    /* 2D31C 8003CB1C 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 2D320 8003CB20 0A80023C */  lui        $v0, %hi(D_800A3817)
    /* 2D324 8003CB24 17384290 */  lbu        $v0, %lo(D_800A3817)($v0)
    /* 2D328 8003CB28 00000000 */  nop
    /* 2D32C 8003CB2C 02004014 */  bnez       $v0, .L8003CB38
    /* 2D330 8003CB30 3C000324 */   addiu     $v1, $zero, 0x3C
    /* 2D334 8003CB34 01000324 */  addiu      $v1, $zero, 0x1
  .L8003CB38:
    /* 2D338 8003CB38 0A80013C */  lui        $at, %hi(D_800A3929)
    /* 2D33C 8003CB3C 293923A0 */  sb         $v1, %lo(D_800A3929)($at)
    /* 2D340 8003CB40 2FF30008 */  j          .L8003CCBC
    /* 2D344 8003CB44 00000000 */   nop
  .L8003CB48:
    /* 2D348 8003CB48 0A80023C */  lui        $v0, %hi(D_800A3817)
    /* 2D34C 8003CB4C 17384290 */  lbu        $v0, %lo(D_800A3817)($v0)
    /* 2D350 8003CB50 00000000 */  nop
    /* 2D354 8003CB54 0F004014 */  bnez       $v0, .L8003CB94
    /* 2D358 8003CB58 21200000 */   addu      $a0, $zero, $zero
    /* 2D35C 8003CB5C 0A80053C */  lui        $a1, %hi(D_800A38B4)
    /* 2D360 8003CB60 B438A58C */  lw         $a1, %lo(D_800A38B4)($a1)
    /* 2D364 8003CB64 A67E010C */  jal        func_8005FA98
    /* 2D368 8003CB68 01000624 */   addiu     $a2, $zero, 0x1
    /* 2D36C 8003CB6C 02004104 */  bgez       $v0, .L8003CB78
    /* 2D370 8003CB70 00000000 */   nop
    /* 2D374 8003CB74 03004224 */  addiu      $v0, $v0, 0x3
  .L8003CB78:
    /* 2D378 8003CB78 83100200 */  sra        $v0, $v0, 2
    /* 2D37C 8003CB7C 0A80033C */  lui        $v1, %hi(D_800A38B4)
    /* 2D380 8003CB80 B438638C */  lw         $v1, %lo(D_800A38B4)($v1)
    /* 2D384 8003CB84 80100200 */  sll        $v0, $v0, 2
    /* 2D388 8003CB88 21186200 */  addu       $v1, $v1, $v0
    /* 2D38C 8003CB8C 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 2D390 8003CB90 B43823AC */  sw         $v1, %lo(D_800A38B4)($at)
  .L8003CB94:
    /* 2D394 8003CB94 0A80023C */  lui        $v0, %hi(D_800A3929)
    /* 2D398 8003CB98 29394290 */  lbu        $v0, %lo(D_800A3929)($v0)
    /* 2D39C 8003CB9C 00000000 */  nop
    /* 2D3A0 8003CBA0 01004224 */  addiu      $v0, $v0, 0x1
    /* 2D3A4 8003CBA4 0A80013C */  lui        $at, %hi(D_800A3929)
    /* 2D3A8 8003CBA8 293922A0 */  sb         $v0, %lo(D_800A3929)($at)
    /* 2D3AC 8003CBAC FF004230 */  andi       $v0, $v0, 0xFF
    /* 2D3B0 8003CBB0 3C00422C */  sltiu      $v0, $v0, 0x3C
    /* 2D3B4 8003CBB4 41004014 */  bnez       $v0, .L8003CCBC
    /* 2D3B8 8003CBB8 00000000 */   nop
    /* 2D3BC 8003CBBC B0DC000C */  jal        func_800372C0
    /* 2D3C0 8003CBC0 00000000 */   nop
    /* 2D3C4 8003CBC4 0A80033C */  lui        $v1, %hi(D_800A3817)
    /* 2D3C8 8003CBC8 17386390 */  lbu        $v1, %lo(D_800A3817)($v1)
    /* 2D3CC 8003CBCC 00000000 */  nop
    /* 2D3D0 8003CBD0 2A006014 */  bnez       $v1, .L8003CC7C
    /* 2D3D4 8003CBD4 01000224 */   addiu     $v0, $zero, 0x1
    /* 2D3D8 8003CBD8 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* 2D3DC 8003CBDC DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* 2D3E0 8003CBE0 05000224 */  addiu      $v0, $zero, 0x5
    /* 2D3E4 8003CBE4 0B006214 */  bne        $v1, $v0, .L8003CC14
    /* 2D3E8 8003CBE8 01000224 */   addiu     $v0, $zero, 0x1
    /* 2D3EC 8003CBEC 1A5A000C */  jal        gpu_EnableDisplay
    /* 2D3F0 8003CBF0 00000000 */   nop
    /* 2D3F4 8003CBF4 3783000C */  jal        func_80020CDC
    /* 2D3F8 8003CBF8 00000000 */   nop
    /* 2D3FC 8003CBFC 0A80013C */  lui        $at, %hi(D_800A3874)
    /* 2D400 8003CC00 743820A0 */  sb         $zero, %lo(D_800A3874)($at)
    /* 2D404 8003CC04 A8D0000C */  jal        func_800342A0
    /* 2D408 8003CC08 00000000 */   nop
    /* 2D40C 8003CC0C 2FF30008 */  j          .L8003CCBC
    /* 2D410 8003CC10 00000000 */   nop
  .L8003CC14:
    /* 2D414 8003CC14 0A80033C */  lui        $v1, %hi(D_800A3748)
    /* 2D418 8003CC18 48376380 */  lb         $v1, %lo(D_800A3748)($v1)
    /* 2D41C 8003CC1C 0A80013C */  lui        $at, %hi(D_800A3670)
    /* 2D420 8003CC20 703622A0 */  sb         $v0, %lo(D_800A3670)($at)
    /* 2D424 8003CC24 0A80023C */  lui        $v0, %hi(D_800A380C)
    /* 2D428 8003CC28 0C38428C */  lw         $v0, %lo(D_800A380C)($v0)
    /* 2D42C 8003CC2C 00210300 */  sll        $a0, $v1, 4
    /* 2D430 8003CC30 21208300 */  addu       $a0, $a0, $v1
    /* 2D434 8003CC34 80200400 */  sll        $a0, $a0, 2
    /* 2D438 8003CC38 21208300 */  addu       $a0, $a0, $v1
    /* 2D43C 8003CC3C 80200400 */  sll        $a0, $a0, 2
    /* 2D440 8003CC40 23208300 */  subu       $a0, $a0, $v1
    /* 2D444 8003CC44 80200400 */  sll        $a0, $a0, 2
    /* 2D448 8003CC48 1080033C */  lui        $v1, %hi(D_80101FBC)
    /* 2D44C 8003CC4C BC1F6324 */  addiu      $v1, $v1, %lo(D_80101FBC)
    /* 2D450 8003CC50 01004224 */  addiu      $v0, $v0, 0x1
    /* 2D454 8003CC54 0A80013C */  lui        $at, %hi(D_800A380C)
    /* 2D458 8003CC58 0C3822AC */  sw         $v0, %lo(D_800A380C)($at)
    /* 2D45C 8003CC5C 0289000C */  jal        func_80022408
    /* 2D460 8003CC60 21208300 */   addu      $a0, $a0, $v1
    /* 2D464 8003CC64 0A80013C */  lui        $at, %hi(D_800A38DF)
    /* 2D468 8003CC68 DF3822A0 */  sb         $v0, %lo(D_800A38DF)($at)
    /* 2D46C 8003CC6C 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2D470 8003CC70 343820A4 */  sh         $zero, %lo(D_800A3834)($at)
    /* 2D474 8003CC74 2FF30008 */  j          .L8003CCBC
    /* 2D478 8003CC78 00000000 */   nop
  .L8003CC7C:
    /* 2D47C 8003CC7C 08006214 */  bne        $v1, $v0, .L8003CCA0
    /* 2D480 8003CC80 02000224 */   addiu     $v0, $zero, 0x2
    /* 2D484 8003CC84 8B76000C */  jal        func_8001DA2C
    /* 2D488 8003CC88 00000000 */   nop
    /* 2D48C 8003CC8C 01000224 */  addiu      $v0, $zero, 0x1
    /* 2D490 8003CC90 0A80013C */  lui        $at, %hi(D_800A31DA)
    /* 2D494 8003CC94 DA3122A0 */  sb         $v0, %lo(D_800A31DA)($at)
    /* 2D498 8003CC98 2DF30008 */  j          .L8003CCB4
    /* 2D49C 8003CC9C 08000224 */   addiu     $v0, $zero, 0x8
  .L8003CCA0:
    /* 2D4A0 8003CCA0 06006214 */  bne        $v1, $v0, .L8003CCBC
    /* 2D4A4 8003CCA4 00000000 */   nop
    /* 2D4A8 8003CCA8 8B76000C */  jal        func_8001DA2C
    /* 2D4AC 8003CCAC 00000000 */   nop
    /* 2D4B0 8003CCB0 08000224 */  addiu      $v0, $zero, 0x8
  .L8003CCB4:
    /* 2D4B4 8003CCB4 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2D4B8 8003CCB8 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
  .L8003CCBC:
    /* 2D4BC 8003CCBC 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2D4C0 8003CCC0 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2D4C4 8003CCC4 0800E003 */  jr         $ra
    /* 2D4C8 8003CCC8 00000000 */   nop
endlabel func_8003C9A4
