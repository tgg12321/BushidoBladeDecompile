glabel func_800520B8
    /* 428B8 800520B8 2A088500 */  slt        $at, $a0, $a1
    /* 428BC 800520BC 62002014 */  bnez       $at, .L80052248
    /* 428C0 800520C0 00000000 */   nop
    /* 428C4 800520C4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 428C8 800520C8 1400B1AF */  sw         $s1, 0x14($sp)
    /* 428CC 800520CC 1000B0AF */  sw         $s0, 0x10($sp)
    /* 428D0 800520D0 5900C010 */  beqz       $a2, .L80052238
    /* 428D4 800520D4 0300C830 */   andi      $t0, $a2, 0x3
    /* 428D8 800520D8 2A400800 */  slt        $t0, $zero, $t0
    /* 428DC 800520DC 82300600 */  srl        $a2, $a2, 2
    /* 428E0 800520E0 2030C800 */  add        $a2, $a2, $t0 /* handwritten instruction */
    /* 428E4 800520E4 0F00C230 */  andi       $v0, $a2, 0xF
    /* 428E8 800520E8 02310600 */  srl        $a2, $a2, 4
    /* 428EC 800520EC 2700C010 */  beqz       $a2, .L8005218C
    /* 428F0 800520F0 FFFFC620 */   addi      $a2, $a2, -0x1 /* handwritten instruction */
  .L800520F4:
    /* 428F4 800520F4 0000888C */  lw         $t0, 0x0($a0)
    /* 428F8 800520F8 0400898C */  lw         $t1, 0x4($a0)
    /* 428FC 800520FC 08008A8C */  lw         $t2, 0x8($a0)
    /* 42900 80052100 0C008B8C */  lw         $t3, 0xC($a0)
    /* 42904 80052104 10008C8C */  lw         $t4, 0x10($a0)
    /* 42908 80052108 14008D8C */  lw         $t5, 0x14($a0)
    /* 4290C 8005210C 18008E8C */  lw         $t6, 0x18($a0)
    /* 42910 80052110 1C008F8C */  lw         $t7, 0x1C($a0)
    /* 42914 80052114 0000A8AC */  sw         $t0, 0x0($a1)
    /* 42918 80052118 0400A9AC */  sw         $t1, 0x4($a1)
    /* 4291C 8005211C 0800AAAC */  sw         $t2, 0x8($a1)
    /* 42920 80052120 0C00ABAC */  sw         $t3, 0xC($a1)
    /* 42924 80052124 1000ACAC */  sw         $t4, 0x10($a1)
    /* 42928 80052128 1400ADAC */  sw         $t5, 0x14($a1)
    /* 4292C 8005212C 1800AEAC */  sw         $t6, 0x18($a1)
    /* 42930 80052130 1C00AFAC */  sw         $t7, 0x1C($a1)
    /* 42934 80052134 2000888C */  lw         $t0, 0x20($a0)
    /* 42938 80052138 2400898C */  lw         $t1, 0x24($a0)
    /* 4293C 8005213C 28008A8C */  lw         $t2, 0x28($a0)
    /* 42940 80052140 2C008B8C */  lw         $t3, 0x2C($a0)
    /* 42944 80052144 30008C8C */  lw         $t4, 0x30($a0)
    /* 42948 80052148 34008D8C */  lw         $t5, 0x34($a0)
    /* 4294C 8005214C 38008E8C */  lw         $t6, 0x38($a0)
    /* 42950 80052150 3C008F8C */  lw         $t7, 0x3C($a0)
    /* 42954 80052154 2000A8AC */  sw         $t0, 0x20($a1)
    /* 42958 80052158 2400A9AC */  sw         $t1, 0x24($a1)
    /* 4295C 8005215C 2800AAAC */  sw         $t2, 0x28($a1)
    /* 42960 80052160 2C00ABAC */  sw         $t3, 0x2C($a1)
    /* 42964 80052164 3000ACAC */  sw         $t4, 0x30($a1)
    /* 42968 80052168 3400ADAC */  sw         $t5, 0x34($a1)
    /* 4296C 8005216C 3800AEAC */  sw         $t6, 0x38($a1)
    /* 42970 80052170 3C00AFAC */  sw         $t7, 0x3C($a1)
    /* 42974 80052174 40008420 */  addi       $a0, $a0, 0x40 /* handwritten instruction */
    /* 42978 80052178 4000A520 */  addi       $a1, $a1, 0x40 /* handwritten instruction */
    /* 4297C 8005217C DDFFC014 */  bnez       $a2, .L800520F4
    /* 42980 80052180 FFFFC620 */   addi      $a2, $a2, -0x1 /* handwritten instruction */
    /* 42984 80052184 2C004010 */  beqz       $v0, .L80052238
    /* 42988 80052188 00000000 */   nop
  .L8005218C:
    /* 4298C 8005218C 0F000324 */  addiu      $v1, $zero, 0xF
    /* 42990 80052190 22186200 */  sub        $v1, $v1, $v0 /* handwritten instruction */
    /* 42994 80052194 80180300 */  sll        $v1, $v1, 2
    /* 42998 80052198 0580193C */  lui        $t9, %hi(D_800521AC)
    /* 4299C 8005219C AC213927 */  addiu      $t9, $t9, %lo(D_800521AC)
    /* 429A0 800521A0 20C82303 */  add        $t9, $t9, $v1 /* handwritten instruction */
    /* 429A4 800521A4 08002003 */  jr         $t9
    /* 429A8 800521A8 00000000 */   nop
  alabel D_800521AC
    /* 429AC 800521AC 3800888C */  lw         $t0, 0x38($a0)
    /* 429B0 800521B0 3400898C */  lw         $t1, 0x34($a0)
    /* 429B4 800521B4 30008A8C */  lw         $t2, 0x30($a0)
    /* 429B8 800521B8 2C008B8C */  lw         $t3, 0x2C($a0)
    /* 429BC 800521BC 28008C8C */  lw         $t4, 0x28($a0)
    /* 429C0 800521C0 24008D8C */  lw         $t5, 0x24($a0)
    /* 429C4 800521C4 20008E8C */  lw         $t6, 0x20($a0)
    /* 429C8 800521C8 1C008F8C */  lw         $t7, 0x1C($a0)
    /* 429CC 800521CC 1800988C */  lw         $t8, 0x18($a0)
    /* 429D0 800521D0 1400998C */  lw         $t9, 0x14($a0)
    /* 429D4 800521D4 1000878C */  lw         $a3, 0x10($a0)
    /* 429D8 800521D8 0C00868C */  lw         $a2, 0xC($a0)
    /* 429DC 800521DC 0800828C */  lw         $v0, 0x8($a0)
    /* 429E0 800521E0 0400918C */  lw         $s1, 0x4($a0)
    /* 429E4 800521E4 0000908C */  lw         $s0, 0x0($a0)
    /* 429E8 800521E8 0580043C */  lui        $a0, %hi(D_800521FC)
    /* 429EC 800521EC FC218424 */  addiu      $a0, $a0, %lo(D_800521FC)
    /* 429F0 800521F0 20208300 */  add        $a0, $a0, $v1 /* handwritten instruction */
    /* 429F4 800521F4 08008000 */  jr         $a0
    /* 429F8 800521F8 00000000 */   nop
  alabel D_800521FC
    /* 429FC 800521FC 3800A8AC */  sw         $t0, 0x38($a1)
    /* 42A00 80052200 3400A9AC */  sw         $t1, 0x34($a1)
    /* 42A04 80052204 3000AAAC */  sw         $t2, 0x30($a1)
    /* 42A08 80052208 2C00ABAC */  sw         $t3, 0x2C($a1)
    /* 42A0C 8005220C 2800ACAC */  sw         $t4, 0x28($a1)
    /* 42A10 80052210 2400ADAC */  sw         $t5, 0x24($a1)
    /* 42A14 80052214 2000AEAC */  sw         $t6, 0x20($a1)
    /* 42A18 80052218 1C00AFAC */  sw         $t7, 0x1C($a1)
    /* 42A1C 8005221C 1800B8AC */  sw         $t8, 0x18($a1)
    /* 42A20 80052220 1400B9AC */  sw         $t9, 0x14($a1)
    /* 42A24 80052224 1000A7AC */  sw         $a3, 0x10($a1)
    /* 42A28 80052228 0C00A6AC */  sw         $a2, 0xC($a1)
    /* 42A2C 8005222C 0800A2AC */  sw         $v0, 0x8($a1)
    /* 42A30 80052230 0400B1AC */  sw         $s1, 0x4($a1)
    /* 42A34 80052234 0000B0AC */  sw         $s0, 0x0($a1)
  .L80052238:
    /* 42A38 80052238 1400B18F */  lw         $s1, 0x14($sp)
    /* 42A3C 8005223C 1000B08F */  lw         $s0, 0x10($sp)
    /* 42A40 80052240 0800E003 */  jr         $ra
    /* 42A44 80052244 1800BD27 */   addiu     $sp, $sp, 0x18
  .L80052248:
    /* 42A48 80052248 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 42A4C 8005224C 1400B1AF */  sw         $s1, 0x14($sp)
    /* 42A50 80052250 1000B0AF */  sw         $s0, 0x10($sp)
    /* 42A54 80052254 5E00C010 */  beqz       $a2, .L800523D0
    /* 42A58 80052258 0300C830 */   andi      $t0, $a2, 0x3
    /* 42A5C 8005225C 2A400800 */  slt        $t0, $zero, $t0
    /* 42A60 80052260 82300600 */  srl        $a2, $a2, 2
    /* 42A64 80052264 2030C800 */  add        $a2, $a2, $t0 /* handwritten instruction */
    /* 42A68 80052268 80400600 */  sll        $t0, $a2, 2
    /* 42A6C 8005226C 20208800 */  add        $a0, $a0, $t0 /* handwritten instruction */
    /* 42A70 80052270 2028A800 */  add        $a1, $a1, $t0 /* handwritten instruction */
    /* 42A74 80052274 FCFF8420 */  addi       $a0, $a0, -0x4 /* handwritten instruction */
    /* 42A78 80052278 FCFFA520 */  addi       $a1, $a1, -0x4 /* handwritten instruction */
    /* 42A7C 8005227C 0F00C230 */  andi       $v0, $a2, 0xF
    /* 42A80 80052280 02310600 */  srl        $a2, $a2, 4
    /* 42A84 80052284 2700C010 */  beqz       $a2, .L80052324
    /* 42A88 80052288 FFFFC620 */   addi      $a2, $a2, -0x1 /* handwritten instruction */
  .L8005228C:
    /* 42A8C 8005228C 0000888C */  lw         $t0, 0x0($a0)
    /* 42A90 80052290 FCFF898C */  lw         $t1, -0x4($a0)
    /* 42A94 80052294 F8FF8A8C */  lw         $t2, -0x8($a0)
    /* 42A98 80052298 F4FF8B8C */  lw         $t3, -0xC($a0)
    /* 42A9C 8005229C F0FF8C8C */  lw         $t4, -0x10($a0)
    /* 42AA0 800522A0 ECFF8D8C */  lw         $t5, -0x14($a0)
    /* 42AA4 800522A4 E8FF8E8C */  lw         $t6, -0x18($a0)
    /* 42AA8 800522A8 E4FF8F8C */  lw         $t7, -0x1C($a0)
    /* 42AAC 800522AC 0000A8AC */  sw         $t0, 0x0($a1)
    /* 42AB0 800522B0 FCFFA9AC */  sw         $t1, -0x4($a1)
    /* 42AB4 800522B4 F8FFAAAC */  sw         $t2, -0x8($a1)
    /* 42AB8 800522B8 F4FFABAC */  sw         $t3, -0xC($a1)
    /* 42ABC 800522BC F0FFACAC */  sw         $t4, -0x10($a1)
    /* 42AC0 800522C0 ECFFADAC */  sw         $t5, -0x14($a1)
    /* 42AC4 800522C4 E8FFAEAC */  sw         $t6, -0x18($a1)
    /* 42AC8 800522C8 E4FFAFAC */  sw         $t7, -0x1C($a1)
    /* 42ACC 800522CC E0FF888C */  lw         $t0, -0x20($a0)
    /* 42AD0 800522D0 DCFF898C */  lw         $t1, -0x24($a0)
    /* 42AD4 800522D4 D8FF8A8C */  lw         $t2, -0x28($a0)
    /* 42AD8 800522D8 D4FF8B8C */  lw         $t3, -0x2C($a0)
    /* 42ADC 800522DC D0FF8C8C */  lw         $t4, -0x30($a0)
    /* 42AE0 800522E0 CCFF8D8C */  lw         $t5, -0x34($a0)
    /* 42AE4 800522E4 C8FF8E8C */  lw         $t6, -0x38($a0)
    /* 42AE8 800522E8 C4FF8F8C */  lw         $t7, -0x3C($a0)
    /* 42AEC 800522EC E0FFA8AC */  sw         $t0, -0x20($a1)
    /* 42AF0 800522F0 DCFFA9AC */  sw         $t1, -0x24($a1)
    /* 42AF4 800522F4 D8FFAAAC */  sw         $t2, -0x28($a1)
    /* 42AF8 800522F8 D4FFABAC */  sw         $t3, -0x2C($a1)
    /* 42AFC 800522FC D0FFACAC */  sw         $t4, -0x30($a1)
    /* 42B00 80052300 CCFFADAC */  sw         $t5, -0x34($a1)
    /* 42B04 80052304 C8FFAEAC */  sw         $t6, -0x38($a1)
    /* 42B08 80052308 C4FFAFAC */  sw         $t7, -0x3C($a1)
    /* 42B0C 8005230C C0FF8420 */  addi       $a0, $a0, -0x40 /* handwritten instruction */
    /* 42B10 80052310 C0FFA520 */  addi       $a1, $a1, -0x40 /* handwritten instruction */
    /* 42B14 80052314 DDFFC014 */  bnez       $a2, .L8005228C
    /* 42B18 80052318 FFFFC620 */   addi      $a2, $a2, -0x1 /* handwritten instruction */
    /* 42B1C 8005231C 2C004010 */  beqz       $v0, .L800523D0
    /* 42B20 80052320 00000000 */   nop
  .L80052324:
    /* 42B24 80052324 0F000324 */  addiu      $v1, $zero, 0xF
    /* 42B28 80052328 22186200 */  sub        $v1, $v1, $v0 /* handwritten instruction */
    /* 42B2C 8005232C 80180300 */  sll        $v1, $v1, 2
    /* 42B30 80052330 0580193C */  lui        $t9, %hi(D_80052344)
    /* 42B34 80052334 44233927 */  addiu      $t9, $t9, %lo(D_80052344)
    /* 42B38 80052338 20C82303 */  add        $t9, $t9, $v1 /* handwritten instruction */
    /* 42B3C 8005233C 08002003 */  jr         $t9
    /* 42B40 80052340 00000000 */   nop
  alabel D_80052344
    /* 42B44 80052344 C8FF888C */  lw         $t0, -0x38($a0)
    /* 42B48 80052348 CCFF898C */  lw         $t1, -0x34($a0)
    /* 42B4C 8005234C D0FF8A8C */  lw         $t2, -0x30($a0)
    /* 42B50 80052350 D4FF8B8C */  lw         $t3, -0x2C($a0)
    /* 42B54 80052354 D8FF8C8C */  lw         $t4, -0x28($a0)
    /* 42B58 80052358 DCFF8D8C */  lw         $t5, -0x24($a0)
    /* 42B5C 8005235C E0FF8E8C */  lw         $t6, -0x20($a0)
    /* 42B60 80052360 E4FF8F8C */  lw         $t7, -0x1C($a0)
    /* 42B64 80052364 E8FF988C */  lw         $t8, -0x18($a0)
    /* 42B68 80052368 ECFF998C */  lw         $t9, -0x14($a0)
    /* 42B6C 8005236C F0FF878C */  lw         $a3, -0x10($a0)
    /* 42B70 80052370 F4FF868C */  lw         $a2, -0xC($a0)
    /* 42B74 80052374 F8FF828C */  lw         $v0, -0x8($a0)
    /* 42B78 80052378 FCFF918C */  lw         $s1, -0x4($a0)
    /* 42B7C 8005237C 0000908C */  lw         $s0, 0x0($a0)
    /* 42B80 80052380 0580043C */  lui        $a0, %hi(D_80052394)
    /* 42B84 80052384 94238424 */  addiu      $a0, $a0, %lo(D_80052394)
    /* 42B88 80052388 20208300 */  add        $a0, $a0, $v1 /* handwritten instruction */
    /* 42B8C 8005238C 08008000 */  jr         $a0
    /* 42B90 80052390 00000000 */   nop
  alabel D_80052394
    /* 42B94 80052394 C8FFA8AC */  sw         $t0, -0x38($a1)
    /* 42B98 80052398 CCFFA9AC */  sw         $t1, -0x34($a1)
    /* 42B9C 8005239C D0FFAAAC */  sw         $t2, -0x30($a1)
    /* 42BA0 800523A0 D4FFABAC */  sw         $t3, -0x2C($a1)
    /* 42BA4 800523A4 D8FFACAC */  sw         $t4, -0x28($a1)
    /* 42BA8 800523A8 DCFFADAC */  sw         $t5, -0x24($a1)
    /* 42BAC 800523AC E0FFAEAC */  sw         $t6, -0x20($a1)
    /* 42BB0 800523B0 E4FFAFAC */  sw         $t7, -0x1C($a1)
    /* 42BB4 800523B4 E8FFB8AC */  sw         $t8, -0x18($a1)
    /* 42BB8 800523B8 ECFFB9AC */  sw         $t9, -0x14($a1)
    /* 42BBC 800523BC F0FFA7AC */  sw         $a3, -0x10($a1)
    /* 42BC0 800523C0 F4FFA6AC */  sw         $a2, -0xC($a1)
    /* 42BC4 800523C4 F8FFA2AC */  sw         $v0, -0x8($a1)
    /* 42BC8 800523C8 FCFFB1AC */  sw         $s1, -0x4($a1)
    /* 42BCC 800523CC 0000B0AC */  sw         $s0, 0x0($a1)
  .L800523D0:
    /* 42BD0 800523D0 1400B18F */  lw         $s1, 0x14($sp)
    /* 42BD4 800523D4 1000B08F */  lw         $s0, 0x10($sp)
    /* 42BD8 800523D8 0800E003 */  jr         $ra
    /* 42BDC 800523DC 1800BD27 */   addiu     $sp, $sp, 0x18
endlabel func_800520B8
