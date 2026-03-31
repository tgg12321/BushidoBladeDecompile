glabel func_80016A8C
    /* 728C 80016A8C C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 7290 80016A90 2C00B1AF */  sw         $s1, 0x2C($sp)
    /* 7294 80016A94 21888000 */  addu       $s1, $a0, $zero
    /* 7298 80016A98 3000BFAF */  sw         $ra, 0x30($sp)
    /* 729C 80016A9C 2800B0AF */  sw         $s0, 0x28($sp)
    /* 72A0 80016AA0 0A80053C */  lui        $a1, %hi(D_800A30D4)
    /* 72A4 80016AA4 D430A524 */  addiu      $a1, $a1, %lo(D_800A30D4)
    /* 72A8 80016AA8 0300A288 */  lwl        $v0, 0x3($a1)
    /* 72AC 80016AAC 0000A298 */  lwr        $v0, 0x0($a1)
    /* 72B0 80016AB0 0700A388 */  lwl        $v1, 0x7($a1)
    /* 72B4 80016AB4 0400A398 */  lwr        $v1, 0x4($a1)
    /* 72B8 80016AB8 1B00A2AB */  swl        $v0, 0x1B($sp)
    /* 72BC 80016ABC 1800A2BB */  swr        $v0, 0x18($sp)
    /* 72C0 80016AC0 1F00A3AB */  swl        $v1, 0x1F($sp)
    /* 72C4 80016AC4 1C00A3BB */  swr        $v1, 0x1C($sp)
    /* 72C8 80016AC8 A8EC010C */  jal        func_8007B2A0
    /* 72CC 80016ACC 21200000 */   addu      $a0, $zero, $zero
    /* 72D0 80016AD0 F0000224 */  addiu      $v0, $zero, 0xF0
    /* 72D4 80016AD4 1080103C */  lui        $s0, %hi(D_800FB524)
    /* 72D8 80016AD8 24B51026 */  addiu      $s0, $s0, %lo(D_800FB524)
    /* 72DC 80016ADC 21200002 */  addu       $a0, $s0, $zero
    /* 72E0 80016AE0 21280000 */  addu       $a1, $zero, $zero
    /* 72E4 80016AE4 21300000 */  addu       $a2, $zero, $zero
    /* 72E8 80016AE8 40010724 */  addiu      $a3, $zero, 0x140
    /* 72EC 80016AEC D3E9010C */  jal        func_8007A74C
    /* 72F0 80016AF0 1000A2AF */   sw        $v0, 0x10($sp)
    /* 72F4 80016AF4 D0DB000C */  jal        func_80036F40
    /* 72F8 80016AF8 00000000 */   nop
    /* 72FC 80016AFC 02000424 */  addiu      $a0, $zero, 0x2
    /* 7300 80016B00 AADB000C */  jal        func_80036EA8
    /* 7304 80016B04 61000524 */   addiu     $a1, $zero, 0x61
    /* 7308 80016B08 21204000 */  addu       $a0, $v0, $zero
    /* 730C 80016B0C 66DB000C */  jal        replay_camera_Init
    /* 7310 80016B10 21282002 */   addu      $a1, $s1, $zero
    /* 7314 80016B14 D0DB000C */  jal        func_80036F40
    /* 7318 80016B18 00000000 */   nop
    /* 731C 80016B1C 02EF010C */  jal        func_8007BC08
    /* 7320 80016B20 21200002 */   addu      $a0, $s0, $zero
    /* 7324 80016B24 CFEC010C */  jal        func_8007B33C
    /* 7328 80016B28 21200000 */   addu      $a0, $zero, $zero
    /* 732C 80016B2C 1800A427 */  addiu      $a0, $sp, 0x18
    /* 7330 80016B30 80ED010C */  jal        func_8007B600
    /* 7334 80016B34 14002526 */   addiu     $a1, $s1, 0x14
    /* 7338 80016B38 CFEC010C */  jal        func_8007B33C
    /* 733C 80016B3C 21200000 */   addu      $a0, $zero, $zero
    /* 7340 80016B40 A8EC010C */  jal        func_8007B2A0
    /* 7344 80016B44 01000424 */   addiu     $a0, $zero, 0x1
    /* 7348 80016B48 21800000 */  addu       $s0, $zero, $zero
    /* 734C 80016B4C 7900022A */  slti       $v0, $s0, 0x79
  .L80016B50:
    /* 7350 80016B50 20004014 */  bnez       $v0, .L80016BD4
    /* 7354 80016B54 28002526 */   addiu     $a1, $s1, 0x28
    /* 7358 80016B58 1C00A387 */  lh         $v1, 0x1C($sp)
    /* 735C 80016B5C 1E00A287 */  lh         $v0, 0x1E($sp)
    /* 7360 80016B60 00000000 */  nop
    /* 7364 80016B64 18006200 */  mult       $v1, $v0
    /* 7368 80016B68 12400000 */  mflo       $t0
    /* 736C 80016B6C 16000019 */  blez       $t0, .L80016BC8
    /* 7370 80016B70 21300000 */   addu      $a2, $zero, $zero
  .L80016B74:
    /* 7374 80016B74 0000A394 */  lhu        $v1, 0x0($a1)
    /* 7378 80016B78 00000000 */  nop
    /* 737C 80016B7C 1F006430 */  andi       $a0, $v1, 0x1F
    /* 7380 80016B80 43200400 */  sra        $a0, $a0, 1
    /* 7384 80016B84 FFFF6330 */  andi       $v1, $v1, 0xFFFF
    /* 7388 80016B88 42100300 */  srl        $v0, $v1, 1
    /* 738C 80016B8C E0014230 */  andi       $v0, $v0, 0x1E0
    /* 7390 80016B90 21208200 */  addu       $a0, $a0, $v0
    /* 7394 80016B94 42180300 */  srl        $v1, $v1, 1
    /* 7398 80016B98 003C6330 */  andi       $v1, $v1, 0x3C00
    /* 739C 80016B9C 21208300 */  addu       $a0, $a0, $v1
    /* 73A0 80016BA0 0000A4A4 */  sh         $a0, 0x0($a1)
    /* 73A4 80016BA4 1C00A387 */  lh         $v1, 0x1C($sp)
    /* 73A8 80016BA8 1E00A287 */  lh         $v0, 0x1E($sp)
    /* 73AC 80016BAC 00000000 */  nop
    /* 73B0 80016BB0 18006200 */  mult       $v1, $v0
    /* 73B4 80016BB4 0100C624 */  addiu      $a2, $a2, 0x1
    /* 73B8 80016BB8 12400000 */  mflo       $t0
    /* 73BC 80016BBC 2A10C800 */  slt        $v0, $a2, $t0
    /* 73C0 80016BC0 ECFF4014 */  bnez       $v0, .L80016B74
    /* 73C4 80016BC4 0200A524 */   addiu     $a1, $a1, 0x2
  .L80016BC8:
    /* 73C8 80016BC8 1800A427 */  addiu      $a0, $sp, 0x18
    /* 73CC 80016BCC 80ED010C */  jal        func_8007B600
    /* 73D0 80016BD0 14002526 */   addiu     $a1, $s1, 0x14
  .L80016BD4:
    /* 73D4 80016BD4 CFEC010C */  jal        func_8007B33C
    /* 73D8 80016BD8 21200000 */   addu      $a0, $zero, $zero
    /* 73DC 80016BDC 330A020C */  jal        func_800828CC
    /* 73E0 80016BE0 21200000 */   addu      $a0, $zero, $zero
    /* 73E4 80016BE4 01001026 */  addiu      $s0, $s0, 0x1
    /* 73E8 80016BE8 9600022A */  slti       $v0, $s0, 0x96
    /* 73EC 80016BEC D8FF4014 */  bnez       $v0, .L80016B50
    /* 73F0 80016BF0 7900022A */   slti      $v0, $s0, 0x79
    /* 73F4 80016BF4 A8EC010C */  jal        func_8007B2A0
    /* 73F8 80016BF8 21200000 */   addu      $a0, $zero, $zero
    /* 73FC 80016BFC F0000224 */  addiu      $v0, $zero, 0xF0
    /* 7400 80016C00 1000A2AF */  sw         $v0, 0x10($sp)
    /* 7404 80016C04 1080043C */  lui        $a0, %hi(D_800FB524)
    /* 7408 80016C08 24B58424 */  addiu      $a0, $a0, %lo(D_800FB524)
    /* 740C 80016C0C 21280000 */  addu       $a1, $zero, $zero
    /* 7410 80016C10 21300000 */  addu       $a2, $zero, $zero
    /* 7414 80016C14 D3E9010C */  jal        func_8007A74C
    /* 7418 80016C18 80020724 */   addiu     $a3, $zero, 0x280
    /* 741C 80016C1C A8EC010C */  jal        func_8007B2A0
    /* 7420 80016C20 01000424 */   addiu     $a0, $zero, 0x1
    /* 7424 80016C24 3000BF8F */  lw         $ra, 0x30($sp)
    /* 7428 80016C28 2C00B18F */  lw         $s1, 0x2C($sp)
    /* 742C 80016C2C 2800B08F */  lw         $s0, 0x28($sp)
    /* 7430 80016C30 3800BD27 */  addiu      $sp, $sp, 0x38
    /* 7434 80016C34 0800E003 */  jr         $ra
    /* 7438 80016C38 00000000 */   nop
endlabel func_80016A8C
