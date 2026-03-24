glabel func_8002EA24
    /* 1F224 8002EA24 21408000 */  addu       $t0, $a0, $zero
    /* 1F228 8002EA28 6000038D */  lw         $v1, 0x60($t0)
    /* 1F22C 8002EA2C 0000A28C */  lw         $v0, 0x0($a1)
    /* 1F230 8002EA30 0000638C */  lw         $v1, 0x0($v1)
    /* 1F234 8002EA34 00000000 */  nop
    /* 1F238 8002EA38 23104300 */  subu       $v0, $v0, $v1
    /* 1F23C 8002EA3C 6000038D */  lw         $v1, 0x60($t0)
    /* 1F240 8002EA40 F80002A5 */  sh         $v0, 0xF8($t0)
    /* 1F244 8002EA44 0400A28C */  lw         $v0, 0x4($a1)
    /* 1F248 8002EA48 0400638C */  lw         $v1, 0x4($v1)
    /* 1F24C 8002EA4C 00000000 */  nop
    /* 1F250 8002EA50 23104300 */  subu       $v0, $v0, $v1
    /* 1F254 8002EA54 6000038D */  lw         $v1, 0x60($t0)
    /* 1F258 8002EA58 FA0002A5 */  sh         $v0, 0xFA($t0)
    /* 1F25C 8002EA5C 0800A28C */  lw         $v0, 0x8($a1)
    /* 1F260 8002EA60 0800638C */  lw         $v1, 0x8($v1)
    /* 1F264 8002EA64 F8FFBD27 */  addiu      $sp, $sp, -0x8
    /* 1F268 8002EA68 23104300 */  subu       $v0, $v0, $v1
    /* 1F26C 8002EA6C FC0002A5 */  sh         $v0, 0xFC($t0)
    /* 1F270 8002EA70 F8000225 */  addiu      $v0, $t0, 0xF8
    /* 1F274 8002EA74 21604000 */  addu       $t4, $v0, $zero
    /* 1F278 8002EA78 000080C9 */  lwc2       $0, 0x0($t4)
    /* 1F27C 8002EA7C 040081C9 */  lwc2       $1, 0x4($t4)
    /* 1F280 8002EA80 00000000 */  nop
    /* 1F284 8002EA84 00000000 */  nop
    /* 1F288 8002EA88 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 1F28C 8002EA8C 00010225 */  addiu      $v0, $t0, 0x100
    /* 1F290 8002EA90 21604000 */  addu       $t4, $v0, $zero
    /* 1F294 8002EA94 000099E9 */  swc2       $25, 0x0($t4)
    /* 1F298 8002EA98 04009AE9 */  swc2       $26, 0x4($t4) /* handwritten instruction */
    /* 1F29C 8002EA9C 08009BE9 */  swc2       $27, 0x8($t4) /* handwritten instruction */
    /* 1F2A0 8002EAA0 0001058D */  lw         $a1, 0x100($t0)
    /* 1F2A4 8002EAA4 23480600 */  negu       $t1, $a2
    /* 1F2A8 8002EAA8 2A10A900 */  slt        $v0, $a1, $t1
    /* 1F2AC 8002EAAC 48004014 */  bnez       $v0, .L8002EBD0
    /* 1F2B0 8002EAB0 21100000 */   addu      $v0, $zero, $zero
    /* 1F2B4 8002EAB4 2A10C500 */  slt        $v0, $a2, $a1
    /* 1F2B8 8002EAB8 45004014 */  bnez       $v0, .L8002EBD0
    /* 1F2BC 8002EABC 21100000 */   addu      $v0, $zero, $zero
    /* 1F2C0 8002EAC0 0401038D */  lw         $v1, 0x104($t0)
    /* 1F2C4 8002EAC4 00000000 */  nop
    /* 1F2C8 8002EAC8 2A106900 */  slt        $v0, $v1, $t1
    /* 1F2CC 8002EACC 40004014 */  bnez       $v0, .L8002EBD0
    /* 1F2D0 8002EAD0 21100000 */   addu      $v0, $zero, $zero
    /* 1F2D4 8002EAD4 2A10C300 */  slt        $v0, $a2, $v1
    /* 1F2D8 8002EAD8 3D004014 */  bnez       $v0, .L8002EBD0
    /* 1F2DC 8002EADC 21100000 */   addu      $v0, $zero, $zero
    /* 1F2E0 8002EAE0 1800A500 */  mult       $a1, $a1
    /* 1F2E4 8002EAE4 12100000 */  mflo       $v0
    /* 1F2E8 8002EAE8 00000000 */  nop
    /* 1F2EC 8002EAEC 00000000 */  nop
    /* 1F2F0 8002EAF0 18006300 */  mult       $v1, $v1
    /* 1F2F4 8002EAF4 12180000 */  mflo       $v1
    /* 1F2F8 8002EAF8 21204300 */  addu       $a0, $v0, $v1
    /* 1F2FC 8002EAFC 2A10E400 */  slt        $v0, $a3, $a0
    /* 1F300 8002EB00 33004014 */  bnez       $v0, .L8002EBD0
    /* 1F304 8002EB04 21100000 */   addu      $v0, $zero, $zero
    /* 1F308 8002EB08 2320E400 */  subu       $a0, $a3, $a0
    /* 1F30C 8002EB0C 0004822C */  sltiu      $v0, $a0, 0x400
    /* 1F310 8002EB10 06004010 */  beqz       $v0, .L8002EB2C
    /* 1F314 8002EB14 00000000 */   nop
    /* 1F318 8002EB18 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1F31C 8002EB1C 21082400 */  addu       $at, $at, $a0
    /* 1F320 8002EB20 18D12290 */  lbu        $v0, %lo(D_8008D118)($at)
    /* 1F324 8002EB24 E1BA0008 */  j          .L8002EB84
    /* 1F328 8002EB28 C2200200 */   srl       $a0, $v0, 3
  .L8002EB2C:
    /* 1F32C 8002EB2C 08008004 */  bltz       $a0, .L8002EB50
    /* 1F330 8002EB30 21180000 */   addu      $v1, $zero, $zero
    /* 1F334 8002EB34 21608000 */  addu       $t4, $a0, $zero
    /* 1F338 8002EB38 00F08C48 */  mtc2       $t4, $30 /* handwritten instruction */
    /* 1F33C 8002EB3C 00000000 */  nop
    /* 1F340 8002EB40 00000000 */  nop
    /* 1F344 8002EB44 2160A003 */  addu       $t4, $sp, $zero
    /* 1F348 8002EB48 00009FE9 */  swc2       $31, 0x0($t4)
    /* 1F34C 8002EB4C 0000A38F */  lw         $v1, 0x0($sp)
  .L8002EB50:
    /* 1F350 8002EB50 FEFF0224 */  addiu      $v0, $zero, -0x2
    /* 1F354 8002EB54 24106200 */  and        $v0, $v1, $v0
    /* 1F358 8002EB58 16000324 */  addiu      $v1, $zero, 0x16
    /* 1F35C 8002EB5C 23186200 */  subu       $v1, $v1, $v0
    /* 1F360 8002EB60 06106400 */  srlv       $v0, $a0, $v1
    /* 1F364 8002EB64 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1F368 8002EB68 21082200 */  addu       $at, $at, $v0
    /* 1F36C 8002EB6C 18D12490 */  lbu        $a0, %lo(D_8008D118)($at)
    /* 1F370 8002EB70 42180300 */  srl        $v1, $v1, 1
    /* 1F374 8002EB74 13000224 */  addiu      $v0, $zero, 0x13
    /* 1F378 8002EB78 23104300 */  subu       $v0, $v0, $v1
    /* 1F37C 8002EB7C 00240400 */  sll        $a0, $a0, 16
    /* 1F380 8002EB80 06204400 */  srlv       $a0, $a0, $v0
  .L8002EB84:
    /* 1F384 8002EB84 21280000 */  addu       $a1, $zero, $zero
    /* 1F388 8002EB88 B000028D */  lw         $v0, 0xB0($t0)
    /* 1F38C 8002EB8C 00000000 */  nop
    /* 1F390 8002EB90 03004104 */  bgez       $v0, .L8002EBA0
    /* 1F394 8002EB94 21300000 */   addu      $a2, $zero, $zero
    /* 1F398 8002EB98 E9BA0008 */  j          .L8002EBA4
    /* 1F39C 8002EB9C 21304000 */   addu      $a2, $v0, $zero
  .L8002EBA0:
    /* 1F3A0 8002EBA0 21284000 */  addu       $a1, $v0, $zero
  .L8002EBA4:
    /* 1F3A4 8002EBA4 0801038D */  lw         $v1, 0x108($t0)
    /* 1F3A8 8002EBA8 00000000 */  nop
    /* 1F3AC 8002EBAC 23106400 */  subu       $v0, $v1, $a0
    /* 1F3B0 8002EBB0 2A10A200 */  slt        $v0, $a1, $v0
    /* 1F3B4 8002EBB4 06004014 */  bnez       $v0, .L8002EBD0
    /* 1F3B8 8002EBB8 21100000 */   addu      $v0, $zero, $zero
    /* 1F3BC 8002EBBC 21106400 */  addu       $v0, $v1, $a0
    /* 1F3C0 8002EBC0 2A104600 */  slt        $v0, $v0, $a2
    /* 1F3C4 8002EBC4 02004014 */  bnez       $v0, .L8002EBD0
    /* 1F3C8 8002EBC8 21100000 */   addu      $v0, $zero, $zero
    /* 1F3CC 8002EBCC 01000224 */  addiu      $v0, $zero, 0x1
  .L8002EBD0:
    /* 1F3D0 8002EBD0 0800BD27 */  addiu      $sp, $sp, 0x8
    /* 1F3D4 8002EBD4 0800E003 */  jr         $ra
    /* 1F3D8 8002EBD8 00000000 */   nop
endlabel func_8002EA24
