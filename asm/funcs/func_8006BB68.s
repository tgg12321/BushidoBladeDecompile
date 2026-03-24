glabel func_8006BB68
    /* 5C368 8006BB68 98FFBD27 */  addiu      $sp, $sp, -0x68
    /* 5C36C 8006BB6C 5000B0AF */  sw         $s0, 0x50($sp)
    /* 5C370 8006BB70 21808000 */  addu       $s0, $a0, $zero
    /* 5C374 8006BB74 0A000224 */  addiu      $v0, $zero, 0xA
    /* 5C378 8006BB78 6000BFAF */  sw         $ra, 0x60($sp)
    /* 5C37C 8006BB7C 5C00B3AF */  sw         $s3, 0x5C($sp)
    /* 5C380 8006BB80 5800B2AF */  sw         $s2, 0x58($sp)
    /* 5C384 8006BB84 5400B1AF */  sw         $s1, 0x54($sp)
    /* 5C388 8006BB88 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5C38C 8006BB8C 3000A0AF */  sw         $zero, 0x30($sp)
    /* 5C390 8006BB90 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 5C394 8006BB94 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5C398 8006BB98 0400028E */  lw         $v0, 0x4($s0)
    /* 5C39C 8006BB9C 00000000 */  nop
    /* 5C3A0 8006BBA0 2C00528C */  lw         $s2, 0x2C($v0)
    /* 5C3A4 8006BBA4 00000000 */  nop
    /* 5C3A8 8006BBA8 0000428E */  lw         $v0, 0x0($s2)
    /* 5C3AC 8006BBAC 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5C3B0 8006BBB0 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5C3B4 8006BBB4 0C004524 */  addiu      $a1, $v0, 0xC
    /* 5C3B8 8006BBB8 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5C3BC 8006BBBC 1C00A5AF */  sw         $a1, 0x1C($sp)
    /* 5C3C0 8006BBC0 1400028E */  lw         $v0, 0x14($s0)
    /* 5C3C4 8006BBC4 21880000 */  addu       $s1, $zero, $zero
    /* 5C3C8 8006BBC8 4BCD010C */  jal        func_8007352C
    /* 5C3CC 8006BBCC 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5C3D0 8006BBD0 140002AE */  sw         $v0, 0x14($s0)
    /* 5C3D4 8006BBD4 1800A48F */  lw         $a0, 0x18($sp)
    /* 5C3D8 8006BBD8 20B9010C */  jal        func_8006E480
    /* 5C3DC 8006BBDC 21280000 */   addu      $a1, $zero, $zero
    /* 5C3E0 8006BBE0 01000524 */  addiu      $a1, $zero, 0x1
    /* 5C3E4 8006BBE4 21300000 */  addu       $a2, $zero, $zero
    /* 5C3E8 8006BBE8 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5C3EC 8006BBEC 1C00048E */  lw         $a0, 0x1C($s0)
    /* 5C3F0 8006BBF0 92F0010C */  jal        func_8007C248
    /* 5C3F4 8006BBF4 21384000 */   addu      $a3, $v0, $zero
    /* 5C3F8 8006BBF8 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5C3FC 8006BBFC 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5C400 8006BC00 1C00058E */  lw         $a1, 0x1C($s0)
    /* 5C404 8006BC04 2DEA010C */  jal        func_8007A8B4
    /* 5C408 8006BC08 28008424 */   addiu     $a0, $a0, 0x28
    /* 5C40C 8006BC0C 1C00028E */  lw         $v0, 0x1C($s0)
    /* 5C410 8006BC10 01001324 */  addiu      $s3, $zero, 0x1
    /* 5C414 8006BC14 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5C418 8006BC18 1C0002AE */  sw         $v0, 0x1C($s0)
  .L8006BC1C:
    /* 5C41C 8006BC1C 0400438E */  lw         $v1, 0x4($s2)
    /* 5C420 8006BC20 2C04828F */  lw         $v0, %gp_rel(D_800A34F8)($gp)
    /* 5C424 8006BC24 0C006524 */  addiu      $a1, $v1, 0xC
    /* 5C428 8006BC28 42130200 */  srl        $v0, $v0, 13
    /* 5C42C 8006BC2C 07004230 */  andi       $v0, $v0, 0x7
    /* 5C430 8006BC30 1800A3AF */  sw         $v1, 0x18($sp)
    /* 5C434 8006BC34 07005114 */  bne        $v0, $s1, .L8006BC54
    /* 5C438 8006BC38 1C00A5AF */   sw        $a1, 0x1C($sp)
    /* 5C43C 8006BC3C 3004828F */  lw         $v0, %gp_rel(D_800A34FC)($gp)
    /* 5C440 8006BC40 00000000 */  nop
    /* 5C444 8006BC44 0E004284 */  lh         $v0, 0xE($v0)
    /* 5C448 8006BC48 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5C44C 8006BC4C 17AF0108 */  j          .L8006BC5C
    /* 5C450 8006BC50 3400A2AF */   sw        $v0, 0x34($sp)
  .L8006BC54:
    /* 5C454 8006BC54 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5C458 8006BC58 2800B3AF */  sw         $s3, 0x28($sp)
  .L8006BC5C:
    /* 5C45C 8006BC5C 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5C460 8006BC60 04005226 */  addiu      $s2, $s2, 0x4
    /* 5C464 8006BC64 1400028E */  lw         $v0, 0x14($s0)
    /* 5C468 8006BC68 01003126 */  addiu      $s1, $s1, 0x1
    /* 5C46C 8006BC6C 4BCD010C */  jal        func_8007352C
    /* 5C470 8006BC70 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5C474 8006BC74 140002AE */  sw         $v0, 0x14($s0)
    /* 5C478 8006BC78 0300222A */  slti       $v0, $s1, 0x3
    /* 5C47C 8006BC7C E7FF4014 */  bnez       $v0, .L8006BC1C
    /* 5C480 8006BC80 21280000 */   addu      $a1, $zero, $zero
    /* 5C484 8006BC84 0400028E */  lw         $v0, 0x4($s0)
    /* 5C488 8006BC88 00000000 */  nop
    /* 5C48C 8006BC8C 2800528C */  lw         $s2, 0x28($v0)
    /* 5C490 8006BC90 00000000 */  nop
    /* 5C494 8006BC94 0400448E */  lw         $a0, 0x4($s2)
    /* 5C498 8006BC98 20B9010C */  jal        func_8006E480
    /* 5C49C 8006BC9C 1800A4AF */   sw        $a0, 0x18($sp)
    /* 5C4A0 8006BCA0 01000524 */  addiu      $a1, $zero, 0x1
    /* 5C4A4 8006BCA4 21300000 */  addu       $a2, $zero, $zero
    /* 5C4A8 8006BCA8 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5C4AC 8006BCAC 1C00048E */  lw         $a0, 0x1C($s0)
    /* 5C4B0 8006BCB0 92F0010C */  jal        func_8007C248
    /* 5C4B4 8006BCB4 21384000 */   addu      $a3, $v0, $zero
    /* 5C4B8 8006BCB8 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5C4BC 8006BCBC 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5C4C0 8006BCC0 1C00058E */  lw         $a1, 0x1C($s0)
    /* 5C4C4 8006BCC4 2DEA010C */  jal        func_8007A8B4
    /* 5C4C8 8006BCC8 28008424 */   addiu     $a0, $a0, 0x28
    /* 5C4CC 8006BCCC 21200002 */  addu       $a0, $s0, $zero
    /* 5C4D0 8006BCD0 4800A527 */  addiu      $a1, $sp, 0x48
    /* 5C4D4 8006BCD4 1C00828C */  lw         $v0, 0x1C($a0)
    /* 5C4D8 8006BCD8 11000624 */  addiu      $a2, $zero, 0x11
    /* 5C4DC 8006BCDC 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5C4E0 8006BCE0 1C0082AC */  sw         $v0, 0x1C($a0)
    /* 5C4E4 8006BCE4 AF000224 */  addiu      $v0, $zero, 0xAF
    /* 5C4E8 8006BCE8 4C00A2A7 */  sh         $v0, 0x4C($sp)
    /* 5C4EC 8006BCEC E8000224 */  addiu      $v0, $zero, 0xE8
    /* 5C4F0 8006BCF0 4800A2A7 */  sh         $v0, 0x48($sp)
    /* 5C4F4 8006BCF4 25000224 */  addiu      $v0, $zero, 0x25
    /* 5C4F8 8006BCF8 4A00A2A7 */  sh         $v0, 0x4A($sp)
    /* 5C4FC 8006BCFC 01000224 */  addiu      $v0, $zero, 0x1
    /* 5C500 8006BD00 26A6010C */  jal        func_80069898
    /* 5C504 8006BD04 4E00A2A7 */   sh        $v0, 0x4E($sp)
    /* 5C508 8006BD08 6000BF8F */  lw         $ra, 0x60($sp)
    /* 5C50C 8006BD0C 5C00B38F */  lw         $s3, 0x5C($sp)
    /* 5C510 8006BD10 5800B28F */  lw         $s2, 0x58($sp)
    /* 5C514 8006BD14 5400B18F */  lw         $s1, 0x54($sp)
    /* 5C518 8006BD18 5000B08F */  lw         $s0, 0x50($sp)
    /* 5C51C 8006BD1C 6800BD27 */  addiu      $sp, $sp, 0x68
    /* 5C520 8006BD20 0800E003 */  jr         $ra
    /* 5C524 8006BD24 00000000 */   nop
endlabel func_8006BB68
