glabel func_8006C21C
    /* 5CA1C 8006C21C 40FFBD27 */  addiu      $sp, $sp, -0xC0
    /* 5CA20 8006C220 4800A4AF */  sw         $a0, 0x48($sp)
    /* 5CA24 8006C224 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CA28 8006C228 0A000224 */  addiu      $v0, $zero, 0xA
    /* 5CA2C 8006C22C BC00BFAF */  sw         $ra, 0xBC($sp)
    /* 5CA30 8006C230 B800BEAF */  sw         $fp, 0xB8($sp)
    /* 5CA34 8006C234 B400B7AF */  sw         $s7, 0xB4($sp)
    /* 5CA38 8006C238 B000B6AF */  sw         $s6, 0xB0($sp)
    /* 5CA3C 8006C23C AC00B5AF */  sw         $s5, 0xAC($sp)
    /* 5CA40 8006C240 A800B4AF */  sw         $s4, 0xA8($sp)
    /* 5CA44 8006C244 A400B3AF */  sw         $s3, 0xA4($sp)
    /* 5CA48 8006C248 A000B2AF */  sw         $s2, 0xA0($sp)
    /* 5CA4C 8006C24C 9C00B1AF */  sw         $s1, 0x9C($sp)
    /* 5CA50 8006C250 9800B0AF */  sw         $s0, 0x98($sp)
    /* 5CA54 8006C254 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 5CA58 8006C258 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5CA5C 8006C25C 0400028D */  lw         $v0, 0x4($t0)
    /* 5CA60 8006C260 00000000 */  nop
    /* 5CA64 8006C264 3000538C */  lw         $s3, 0x30($v0)
    /* 5CA68 8006C268 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5CA6C 8006C26C 3000A0AF */  sw         $zero, 0x30($sp)
    /* 5CA70 8006C270 0000628E */  lw         $v0, 0x0($s3)
    /* 5CA74 8006C274 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5CA78 8006C278 0C004524 */  addiu      $a1, $v0, 0xC
    /* 5CA7C 8006C27C 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5CA80 8006C280 1C00A5AF */  sw         $a1, 0x1C($sp)
    /* 5CA84 8006C284 1400028D */  lw         $v0, 0x14($t0)
    /* 5CA88 8006C288 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5CA8C 8006C28C 4BCD010C */  jal        func_8007352C
    /* 5CA90 8006C290 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5CA94 8006C294 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CA98 8006C298 00000000 */  nop
    /* 5CA9C 8006C29C 140002AD */  sw         $v0, 0x14($t0)
    /* 5CAA0 8006C2A0 1800A48F */  lw         $a0, 0x18($sp)
    /* 5CAA4 8006C2A4 20B9010C */  jal        func_8006E480
    /* 5CAA8 8006C2A8 21280000 */   addu      $a1, $zero, $zero
    /* 5CAAC 8006C2AC 01000524 */  addiu      $a1, $zero, 0x1
    /* 5CAB0 8006C2B0 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CAB4 8006C2B4 21300000 */  addu       $a2, $zero, $zero
    /* 5CAB8 8006C2B8 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5CABC 8006C2BC 1C00048D */  lw         $a0, 0x1C($t0)
    /* 5CAC0 8006C2C0 92F0010C */  jal        func_8007C248
    /* 5CAC4 8006C2C4 21384000 */   addu      $a3, $v0, $zero
    /* 5CAC8 8006C2C8 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CACC 8006C2CC 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5CAD0 8006C2D0 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5CAD4 8006C2D4 1C00058D */  lw         $a1, 0x1C($t0)
    /* 5CAD8 8006C2D8 2DEA010C */  jal        func_8007A8B4
    /* 5CADC 8006C2DC 28008424 */   addiu     $a0, $a0, 0x28
    /* 5CAE0 8006C2E0 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CAE4 8006C2E4 21A80000 */  addu       $s5, $zero, $zero
    /* 5CAE8 8006C2E8 1C00028D */  lw         $v0, 0x1C($t0)
    /* 5CAEC 8006C2EC 21900000 */  addu       $s2, $zero, $zero
    /* 5CAF0 8006C2F0 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5CAF4 8006C2F4 1C0002AD */  sw         $v0, 0x1C($t0)
    /* 5CAF8 8006C2F8 09000224 */  addiu      $v0, $zero, 0x9
    /* 5CAFC 8006C2FC 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 5CB00 8006C300 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5CB04 8006C304 0400028D */  lw         $v0, 0x4($t0)
    /* 5CB08 8006C308 21880000 */  addu       $s1, $zero, $zero
    /* 5CB0C 8006C30C 3000538C */  lw         $s3, 0x30($v0)
    /* 5CB10 8006C310 21A00000 */  addu       $s4, $zero, $zero
    /* 5CB14 8006C314 3400A0AF */  sw         $zero, 0x34($sp)
  .L8006C318:
    /* 5CB18 8006C318 3004828F */  lw         $v0, %gp_rel(D_800A34FC)($gp)
    /* 5CB1C 8006C31C 3000B4AF */  sw         $s4, 0x30($sp)
    /* 5CB20 8006C320 21102202 */  addu       $v0, $s1, $v0
    /* 5CB24 8006C324 28004284 */  lh         $v0, 0x28($v0)
    /* 5CB28 8006C328 00000000 */  nop
    /* 5CB2C 8006C32C 03004228 */  slti       $v0, $v0, 0x3
    /* 5CB30 8006C330 20004010 */  beqz       $v0, .L8006C3B4
    /* 5CB34 8006C334 00000000 */   nop
    /* 5CB38 8006C338 21F00000 */  addu       $fp, $zero, $zero
    /* 5CB3C 8006C33C 21806002 */  addu       $s0, $s3, $zero
  .L8006C340:
    /* 5CB40 8006C340 3004828F */  lw         $v0, %gp_rel(D_800A34FC)($gp)
    /* 5CB44 8006C344 5804838F */  lw         $v1, %gp_rel(D_800A3524)($gp)
    /* 5CB48 8006C348 21102202 */  addu       $v0, $s1, $v0
    /* 5CB4C 8006C34C 28004284 */  lh         $v0, 0x28($v0)
    /* 5CB50 8006C350 80201200 */  sll        $a0, $s2, 2
    /* 5CB54 8006C354 21186200 */  addu       $v1, $v1, $v0
    /* 5CB58 8006C358 01000224 */  addiu      $v0, $zero, 0x1
    /* 5CB5C 8006C35C 0410C203 */  sllv       $v0, $v0, $fp
    /* 5CB60 8006C360 17006390 */  lbu        $v1, 0x17($v1)
    /* 5CB64 8006C364 04108200 */  sllv       $v0, $v0, $a0
    /* 5CB68 8006C368 24186200 */  and        $v1, $v1, $v0
    /* 5CB6C 8006C36C 0D006010 */  beqz       $v1, .L8006C3A4
    /* 5CB70 8006C370 1800A427 */   addiu     $a0, $sp, 0x18
    /* 5CB74 8006C374 3400028E */  lw         $v0, 0x34($s0)
    /* 5CB78 8006C378 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CB7C 8006C37C 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5CB80 8006C380 0C004524 */  addiu      $a1, $v0, 0xC
    /* 5CB84 8006C384 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5CB88 8006C388 1C00A5AF */  sw         $a1, 0x1C($sp)
    /* 5CB8C 8006C38C 1400028D */  lw         $v0, 0x14($t0)
    /* 5CB90 8006C390 4BCD010C */  jal        func_8007352C
    /* 5CB94 8006C394 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5CB98 8006C398 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CB9C 8006C39C 00000000 */  nop
    /* 5CBA0 8006C3A0 140002AD */  sw         $v0, 0x14($t0)
  .L8006C3A4:
    /* 5CBA4 8006C3A4 0100DE27 */  addiu      $fp, $fp, 0x1
    /* 5CBA8 8006C3A8 0400C22B */  slti       $v0, $fp, 0x4
    /* 5CBAC 8006C3AC E4FF4014 */  bnez       $v0, .L8006C340
    /* 5CBB0 8006C3B0 04001026 */   addiu     $s0, $s0, 0x4
  .L8006C3B4:
    /* 5CBB4 8006C3B4 02003126 */  addiu      $s1, $s1, 0x2
    /* 5CBB8 8006C3B8 01005226 */  addiu      $s2, $s2, 0x1
    /* 5CBBC 8006C3BC 0200422A */  slti       $v0, $s2, 0x2
    /* 5CBC0 8006C3C0 D5FF4014 */  bnez       $v0, .L8006C318
    /* 5CBC4 8006C3C4 18019426 */   addiu     $s4, $s4, 0x118
    /* 5CBC8 8006C3C8 3400648E */  lw         $a0, 0x34($s3)
    /* 5CBCC 8006C3CC 2128A002 */  addu       $a1, $s5, $zero
    /* 5CBD0 8006C3D0 20B9010C */  jal        func_8006E480
    /* 5CBD4 8006C3D4 1800A4AF */   sw        $a0, 0x18($sp)
    /* 5CBD8 8006C3D8 01000524 */  addiu      $a1, $zero, 0x1
    /* 5CBDC 8006C3DC 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CBE0 8006C3E0 21300000 */  addu       $a2, $zero, $zero
    /* 5CBE4 8006C3E4 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5CBE8 8006C3E8 1C00048D */  lw         $a0, 0x1C($t0)
    /* 5CBEC 8006C3EC 92F0010C */  jal        func_8007C248
    /* 5CBF0 8006C3F0 21384000 */   addu      $a3, $v0, $zero
    /* 5CBF4 8006C3F4 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CBF8 8006C3F8 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5CBFC 8006C3FC 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5CC00 8006C400 1C00058D */  lw         $a1, 0x1C($t0)
    /* 5CC04 8006C404 2DEA010C */  jal        func_8007A8B4
    /* 5CC08 8006C408 24008424 */   addiu     $a0, $a0, 0x24
    /* 5CC0C 8006C40C 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CC10 8006C410 00000000 */  nop
    /* 5CC14 8006C414 1C00028D */  lw         $v0, 0x1C($t0)
    /* 5CC18 8006C418 00000000 */  nop
    /* 5CC1C 8006C41C 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5CC20 8006C420 1C0002AD */  sw         $v0, 0x1C($t0)
    /* 5CC24 8006C424 0A000224 */  addiu      $v0, $zero, 0xA
    /* 5CC28 8006C428 3000A0AF */  sw         $zero, 0x30($sp)
    /* 5CC2C 8006C42C 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5CC30 8006C430 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 5CC34 8006C434 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5CC38 8006C438 0400028D */  lw         $v0, 0x4($t0)
    /* 5CC3C 8006C43C 00000000 */  nop
    /* 5CC40 8006C440 3000538C */  lw         $s3, 0x30($v0)
    /* 5CC44 8006C444 21F00000 */  addu       $fp, $zero, $zero
    /* 5CC48 8006C448 0400628E */  lw         $v0, 0x4($s3)
    /* 5CC4C 8006C44C 18011124 */  addiu      $s1, $zero, 0x118
    /* 5CC50 8006C450 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5CC54 8006C454 0C004524 */  addiu      $a1, $v0, 0xC
    /* 5CC58 8006C458 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5CC5C 8006C45C 1C00A5AF */  sw         $a1, 0x1C($sp)
    /* 5CC60 8006C460 1400028D */  lw         $v0, 0x14($t0)
    /* 5CC64 8006C464 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5CC68 8006C468 4BCD010C */  jal        func_8007352C
    /* 5CC6C 8006C46C 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5CC70 8006C470 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CC74 8006C474 21806002 */  addu       $s0, $s3, $zero
    /* 5CC78 8006C478 140002AD */  sw         $v0, 0x14($t0)
  .L8006C47C:
    /* 5CC7C 8006C47C 5000A0AF */  sw         $zero, 0x50($sp)
    /* 5CC80 8006C480 0800038E */  lw         $v1, 0x8($s0)
    /* 5CC84 8006C484 0A000224 */  addiu      $v0, $zero, 0xA
    /* 5CC88 8006C488 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 5CC8C 8006C48C 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5CC90 8006C490 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5CC94 8006C494 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5CC98 8006C498 0C006524 */  addiu      $a1, $v1, 0xC
    /* 5CC9C 8006C49C 1800A3AF */  sw         $v1, 0x18($sp)
    /* 5CCA0 8006C4A0 1C00A5AF */  sw         $a1, 0x1C($sp)
    /* 5CCA4 8006C4A4 5000A88F */  lw         $t0, 0x50($sp)
  .L8006C4A8:
    /* 5CCA8 8006C4A8 00000000 */  nop
    /* 5CCAC 8006C4AC 03000011 */  beqz       $t0, .L8006C4BC
    /* 5CCB0 8006C4B0 00000000 */   nop
    /* 5CCB4 8006C4B4 30B10108 */  j          .L8006C4C0
    /* 5CCB8 8006C4B8 3000B1AF */   sw        $s1, 0x30($sp)
  .L8006C4BC:
    /* 5CCBC 8006C4BC 3000A0AF */  sw         $zero, 0x30($sp)
  .L8006C4C0:
    /* 5CCC0 8006C4C0 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CCC4 8006C4C4 00000000 */  nop
    /* 5CCC8 8006C4C8 1400028D */  lw         $v0, 0x14($t0)
    /* 5CCCC 8006C4CC 5000A88F */  lw         $t0, 0x50($sp)
    /* 5CCD0 8006C4D0 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5CCD4 8006C4D4 01000825 */  addiu      $t0, $t0, 0x1
    /* 5CCD8 8006C4D8 5000A8AF */  sw         $t0, 0x50($sp)
    /* 5CCDC 8006C4DC 4BCD010C */  jal        func_8007352C
    /* 5CCE0 8006C4E0 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5CCE4 8006C4E4 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CCE8 8006C4E8 00000000 */  nop
    /* 5CCEC 8006C4EC 140002AD */  sw         $v0, 0x14($t0)
    /* 5CCF0 8006C4F0 5000A88F */  lw         $t0, 0x50($sp)
    /* 5CCF4 8006C4F4 00000000 */  nop
    /* 5CCF8 8006C4F8 02000229 */  slti       $v0, $t0, 0x2
    /* 5CCFC 8006C4FC EAFF4014 */  bnez       $v0, .L8006C4A8
    /* 5CD00 8006C500 00000000 */   nop
    /* 5CD04 8006C504 0100DE27 */  addiu      $fp, $fp, 0x1
    /* 5CD08 8006C508 0600C22B */  slti       $v0, $fp, 0x6
    /* 5CD0C 8006C50C DBFF4014 */  bnez       $v0, .L8006C47C
    /* 5CD10 8006C510 04001026 */   addiu     $s0, $s0, 0x4
    /* 5CD14 8006C514 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CD18 8006C518 00000000 */  nop
    /* 5CD1C 8006C51C 0400028D */  lw         $v0, 0x4($t0)
    /* 5CD20 8006C520 00000000 */  nop
    /* 5CD24 8006C524 3000538C */  lw         $s3, 0x30($v0)
    /* 5CD28 8006C528 00000000 */  nop
    /* 5CD2C 8006C52C 0400648E */  lw         $a0, 0x4($s3)
    /* 5CD30 8006C530 2128A002 */  addu       $a1, $s5, $zero
    /* 5CD34 8006C534 20B9010C */  jal        func_8006E480
    /* 5CD38 8006C538 1800A4AF */   sw        $a0, 0x18($sp)
    /* 5CD3C 8006C53C 01000524 */  addiu      $a1, $zero, 0x1
    /* 5CD40 8006C540 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CD44 8006C544 21300000 */  addu       $a2, $zero, $zero
    /* 5CD48 8006C548 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5CD4C 8006C54C 1C00048D */  lw         $a0, 0x1C($t0)
    /* 5CD50 8006C550 92F0010C */  jal        func_8007C248
    /* 5CD54 8006C554 21384000 */   addu      $a3, $v0, $zero
    /* 5CD58 8006C558 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CD5C 8006C55C 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5CD60 8006C560 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5CD64 8006C564 1C00058D */  lw         $a1, 0x1C($t0)
    /* 5CD68 8006C568 2DEA010C */  jal        func_8007A8B4
    /* 5CD6C 8006C56C 28008424 */   addiu     $a0, $a0, 0x28
    /* 5CD70 8006C570 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CD74 8006C574 00000000 */  nop
    /* 5CD78 8006C578 1C00028D */  lw         $v0, 0x1C($t0)
    /* 5CD7C 8006C57C 3004838F */  lw         $v1, %gp_rel(D_800A34FC)($gp)
    /* 5CD80 8006C580 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5CD84 8006C584 1C0002AD */  sw         $v0, 0x1C($t0)
    /* 5CD88 8006C588 2400628C */  lw         $v0, 0x24($v1)
    /* 5CD8C 8006C58C 00000000 */  nop
    /* 5CD90 8006C590 4400428C */  lw         $v0, 0x44($v0)
    /* 5CD94 8006C594 21F00000 */  addu       $fp, $zero, $zero
    /* 5CD98 8006C598 5800A2AF */  sw         $v0, 0x58($sp)
    /* 5CD9C 8006C59C 1800128D */  lw         $s2, 0x18($t0)
    /* 5CDA0 8006C5A0 5800B18F */  lw         $s1, 0x58($sp)
  .L8006C5A4:
    /* 5CDA4 8006C5A4 5000A0AF */  sw         $zero, 0x50($sp)
    /* 5CDA8 8006C5A8 21980000 */  addu       $s3, $zero, $zero
    /* 5CDAC 8006C5AC 0E005026 */  addiu      $s0, $s2, 0xE
  .L8006C5B0:
    /* 5CDB0 8006C5B0 B4EA010C */  jal        func_8007AAD0
    /* 5CDB4 8006C5B4 21204002 */   addu      $a0, $s2, $zero
    /* 5CDB8 8006C5B8 08002292 */  lbu        $v0, 0x8($s1)
    /* 5CDBC 8006C5BC 00000000 */  nop
    /* 5CDC0 8006C5C0 F6FF02A2 */  sb         $v0, -0xA($s0)
    /* 5CDC4 8006C5C4 09002292 */  lbu        $v0, 0x9($s1)
    /* 5CDC8 8006C5C8 00000000 */  nop
    /* 5CDCC 8006C5CC F7FF02A2 */  sb         $v0, -0x9($s0)
    /* 5CDD0 8006C5D0 0A002292 */  lbu        $v0, 0xA($s1)
    /* 5CDD4 8006C5D4 00000000 */  nop
    /* 5CDD8 8006C5D8 F8FF02A2 */  sb         $v0, -0x8($s0)
    /* 5CDDC 8006C5DC 00002296 */  lhu        $v0, 0x0($s1)
    /* 5CDE0 8006C5E0 21204002 */  addu       $a0, $s2, $zero
    /* 5CDE4 8006C5E4 21105300 */  addu       $v0, $v0, $s3
    /* 5CDE8 8006C5E8 FAFF02A6 */  sh         $v0, -0x6($s0)
    /* 5CDEC 8006C5EC 02002296 */  lhu        $v0, 0x2($s1)
    /* 5CDF0 8006C5F0 21280000 */  addu       $a1, $zero, $zero
    /* 5CDF4 8006C5F4 FCFF02A6 */  sh         $v0, -0x4($s0)
    /* 5CDF8 8006C5F8 5000A88F */  lw         $t0, 0x50($sp)
    /* 5CDFC 8006C5FC 04002296 */  lhu        $v0, 0x4($s1)
    /* 5CE00 8006C600 01000825 */  addiu      $t0, $t0, 0x1
    /* 5CE04 8006C604 5000A8AF */  sw         $t0, 0x50($sp)
    /* 5CE08 8006C608 FEFF02A6 */  sh         $v0, -0x2($s0)
    /* 5CE0C 8006C60C 06002296 */  lhu        $v0, 0x6($s1)
    /* 5CE10 8006C610 18017326 */  addiu      $s3, $s3, 0x118
    /* 5CE14 8006C614 5AEA010C */  jal        func_8007A968
    /* 5CE18 8006C618 000002A6 */   sh        $v0, 0x0($s0)
    /* 5CE1C 8006C61C 21284002 */  addu       $a1, $s2, $zero
    /* 5CE20 8006C620 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5CE24 8006C624 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5CE28 8006C628 10001026 */  addiu      $s0, $s0, 0x10
    /* 5CE2C 8006C62C 2DEA010C */  jal        func_8007A8B4
    /* 5CE30 8006C630 30008424 */   addiu     $a0, $a0, 0x30
    /* 5CE34 8006C634 5000A88F */  lw         $t0, 0x50($sp)
    /* 5CE38 8006C638 00000000 */  nop
    /* 5CE3C 8006C63C 02000229 */  slti       $v0, $t0, 0x2
    /* 5CE40 8006C640 DBFF4014 */  bnez       $v0, .L8006C5B0
    /* 5CE44 8006C644 10005226 */   addiu     $s2, $s2, 0x10
    /* 5CE48 8006C648 0100DE27 */  addiu      $fp, $fp, 0x1
    /* 5CE4C 8006C64C 0B00C22B */  slti       $v0, $fp, 0xB
    /* 5CE50 8006C650 D4FF4014 */  bnez       $v0, .L8006C5A4
    /* 5CE54 8006C654 0C003126 */   addiu     $s1, $s1, 0xC
    /* 5CE58 8006C658 4C04848F */  lw         $a0, %gp_rel(D_800A3518)($gp)
    /* 5CE5C 8006C65C 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CE60 8006C660 21980000 */  addu       $s3, $zero, $zero
    /* 5CE64 8006C664 5000A0AF */  sw         $zero, 0x50($sp)
    /* 5CE68 8006C668 C0210400 */  sll        $a0, $a0, 7
    /* 5CE6C 8006C66C 800F8430 */  andi       $a0, $a0, 0xF80
    /* 5CE70 8006C670 FBF7010C */  jal        func_8007DFEC
    /* 5CE74 8006C674 180012AD */   sw        $s2, 0x18($t0)
    /* 5CE78 8006C678 40110200 */  sll        $v0, $v0, 5
    /* 5CE7C 8006C67C 03130200 */  sra        $v0, $v0, 12
    /* 5CE80 8006C680 4800A88F */  lw         $t0, 0x48($sp)
    /* 5CE84 8006C684 D0005424 */  addiu      $s4, $v0, 0xD0
    /* 5CE88 8006C688 1000158D */  lw         $s5, 0x10($t0)
  .L8006C68C:
    /* 5CE8C 8006C68C 21B00000 */  addu       $s6, $zero, $zero
    /* 5CE90 8006C690 FFFF1724 */  addiu      $s7, $zero, -0x1
    /* 5CE94 8006C694 3004838F */  lw         $v1, %gp_rel(D_800A34FC)($gp)
    /* 5CE98 8006C698 5000A88F */  lw         $t0, 0x50($sp)
    /* 5CE9C 8006C69C 2200B026 */  addiu      $s0, $s5, 0x22
    /* 5CEA0 8006C6A0 8000A0AF */  sw         $zero, 0x80($sp)
    /* 5CEA4 8006C6A4 8800A0AF */  sw         $zero, 0x88($sp)
    /* 5CEA8 8006C6A8 9000A0AF */  sw         $zero, 0x90($sp)
    /* 5CEAC 8006C6AC 40100800 */  sll        $v0, $t0, 1
    /* 5CEB0 8006C6B0 21104300 */  addu       $v0, $v0, $v1
    /* 5CEB4 8006C6B4 28005E84 */  lh         $fp, 0x28($v0)
    /* 5CEB8 8006C6B8 5800A88F */  lw         $t0, 0x58($sp)
    /* 5CEBC 8006C6BC 40101E00 */  sll        $v0, $fp, 1
    /* 5CEC0 8006C6C0 21105E00 */  addu       $v0, $v0, $fp
    /* 5CEC4 8006C6C4 80100200 */  sll        $v0, $v0, 2
    /* 5CEC8 8006C6C8 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5CECC 8006C6CC 21880201 */  addu       $s1, $t0, $v0
    /* 5CED0 8006C6D0 21902002 */  addu       $s2, $s1, $zero
  .L8006C6D4:
    /* 5CED4 8006C6D4 8CEA010C */  jal        func_8007AA30
    /* 5CED8 8006C6D8 2120A002 */   addu      $a0, $s5, $zero
    /* 5CEDC 8006C6DC 05000224 */  addiu      $v0, $zero, 0x5
    /* 5CEE0 8006C6E0 3500C217 */  bne        $fp, $v0, .L8006C7B8
    /* 5CEE4 8006C6E4 00000000 */   nop
    /* 5CEE8 8006C6E8 2120A002 */  addu       $a0, $s5, $zero
    /* 5CEEC 8006C6EC 5AEA010C */  jal        func_8007A968
    /* 5CEF0 8006C6F0 01000524 */   addiu     $a1, $zero, 0x1
    /* 5CEF4 8006C6F4 E2FF14A2 */  sb         $s4, -0x1E($s0)
    /* 5CEF8 8006C6F8 E3FF14A2 */  sb         $s4, -0x1D($s0)
    /* 5CEFC 8006C6FC E4FF14A2 */  sb         $s4, -0x1C($s0)
    /* 5CF00 8006C700 EAFF14A2 */  sb         $s4, -0x16($s0)
    /* 5CF04 8006C704 EBFF14A2 */  sb         $s4, -0x15($s0)
    /* 5CF08 8006C708 ECFF14A2 */  sb         $s4, -0x14($s0)
    /* 5CF0C 8006C70C F2FF00A2 */  sb         $zero, -0xE($s0)
    /* 5CF10 8006C710 F3FF00A2 */  sb         $zero, -0xD($s0)
    /* 5CF14 8006C714 F4FF00A2 */  sb         $zero, -0xC($s0)
    /* 5CF18 8006C718 FAFF00A2 */  sb         $zero, -0x6($s0)
    /* 5CF1C 8006C71C FBFF00A2 */  sb         $zero, -0x5($s0)
    /* 5CF20 8006C720 FCFF00A2 */  sb         $zero, -0x4($s0)
    /* 5CF24 8006C724 00004296 */  lhu        $v0, 0x0($s2)
    /* 5CF28 8006C728 00000000 */  nop
    /* 5CF2C 8006C72C 21105300 */  addu       $v0, $v0, $s3
    /* 5CF30 8006C730 E6FF02A6 */  sh         $v0, -0x1A($s0)
    /* 5CF34 8006C734 02004296 */  lhu        $v0, 0x2($s2)
    /* 5CF38 8006C738 00000000 */  nop
    /* 5CF3C 8006C73C 23105700 */  subu       $v0, $v0, $s7
    /* 5CF40 8006C740 E8FF02A6 */  sh         $v0, -0x18($s0)
    /* 5CF44 8006C744 00004296 */  lhu        $v0, 0x0($s2)
    /* 5CF48 8006C748 04004396 */  lhu        $v1, 0x4($s2)
    /* 5CF4C 8006C74C 21105300 */  addu       $v0, $v0, $s3
    /* 5CF50 8006C750 21186200 */  addu       $v1, $v1, $v0
    /* 5CF54 8006C754 EEFF03A6 */  sh         $v1, -0x12($s0)
    /* 5CF58 8006C758 02004296 */  lhu        $v0, 0x2($s2)
    /* 5CF5C 8006C75C 00000000 */  nop
    /* 5CF60 8006C760 23105700 */  subu       $v0, $v0, $s7
    /* 5CF64 8006C764 F0FF02A6 */  sh         $v0, -0x10($s0)
    /* 5CF68 8006C768 00004296 */  lhu        $v0, 0x0($s2)
    /* 5CF6C 8006C76C 00000000 */  nop
    /* 5CF70 8006C770 21105300 */  addu       $v0, $v0, $s3
    /* 5CF74 8006C774 F6FF02A6 */  sh         $v0, -0xA($s0)
    /* 5CF78 8006C778 02004296 */  lhu        $v0, 0x2($s2)
    /* 5CF7C 8006C77C 8000A88F */  lw         $t0, 0x80($sp)
    /* 5CF80 8006C780 23105700 */  subu       $v0, $v0, $s7
    /* 5CF84 8006C784 02004224 */  addiu      $v0, $v0, 0x2
    /* 5CF88 8006C788 21104800 */  addu       $v0, $v0, $t0
    /* 5CF8C 8006C78C F8FF02A6 */  sh         $v0, -0x8($s0)
    /* 5CF90 8006C790 00004296 */  lhu        $v0, 0x0($s2)
    /* 5CF94 8006C794 04004396 */  lhu        $v1, 0x4($s2)
    /* 5CF98 8006C798 21105300 */  addu       $v0, $v0, $s3
    /* 5CF9C 8006C79C 21186200 */  addu       $v1, $v1, $v0
    /* 5CFA0 8006C7A0 FEFF03A6 */  sh         $v1, -0x2($s0)
    /* 5CFA4 8006C7A4 02004296 */  lhu        $v0, 0x2($s2)
    /* 5CFA8 8006C7A8 00000000 */  nop
    /* 5CFAC 8006C7AC 23105700 */  subu       $v0, $v0, $s7
    /* 5CFB0 8006C7B0 22B20108 */  j          .L8006C888
    /* 5CFB4 8006C7B4 02004224 */   addiu     $v0, $v0, 0x2
  .L8006C7B8:
    /* 5CFB8 8006C7B8 2120A002 */  addu       $a0, $s5, $zero
    /* 5CFBC 8006C7BC 5AEA010C */  jal        func_8007A968
    /* 5CFC0 8006C7C0 21280000 */   addu      $a1, $zero, $zero
    /* 5CFC4 8006C7C4 80000224 */  addiu      $v0, $zero, 0x80
    /* 5CFC8 8006C7C8 E2FF14A2 */  sb         $s4, -0x1E($s0)
    /* 5CFCC 8006C7CC E3FF00A2 */  sb         $zero, -0x1D($s0)
    /* 5CFD0 8006C7D0 E4FF00A2 */  sb         $zero, -0x1C($s0)
    /* 5CFD4 8006C7D4 EAFF14A2 */  sb         $s4, -0x16($s0)
    /* 5CFD8 8006C7D8 EBFF00A2 */  sb         $zero, -0x15($s0)
    /* 5CFDC 8006C7DC ECFF00A2 */  sb         $zero, -0x14($s0)
    /* 5CFE0 8006C7E0 F2FF02A2 */  sb         $v0, -0xE($s0)
    /* 5CFE4 8006C7E4 F3FF00A2 */  sb         $zero, -0xD($s0)
    /* 5CFE8 8006C7E8 F4FF00A2 */  sb         $zero, -0xC($s0)
    /* 5CFEC 8006C7EC FAFF02A2 */  sb         $v0, -0x6($s0)
    /* 5CFF0 8006C7F0 FBFF00A2 */  sb         $zero, -0x5($s0)
    /* 5CFF4 8006C7F4 FCFF00A2 */  sb         $zero, -0x4($s0)
    /* 5CFF8 8006C7F8 00004296 */  lhu        $v0, 0x0($s2)
    /* 5CFFC 8006C7FC 00000000 */  nop
    /* 5D000 8006C800 21105300 */  addu       $v0, $v0, $s3
    /* 5D004 8006C804 E6FF02A6 */  sh         $v0, -0x1A($s0)
    /* 5D008 8006C808 02004296 */  lhu        $v0, 0x2($s2)
    /* 5D00C 8006C80C 00000000 */  nop
    /* 5D010 8006C810 23105700 */  subu       $v0, $v0, $s7
    /* 5D014 8006C814 E8FF02A6 */  sh         $v0, -0x18($s0)
    /* 5D018 8006C818 00004296 */  lhu        $v0, 0x0($s2)
    /* 5D01C 8006C81C 04004396 */  lhu        $v1, 0x4($s2)
    /* 5D020 8006C820 21105300 */  addu       $v0, $v0, $s3
    /* 5D024 8006C824 21186200 */  addu       $v1, $v1, $v0
    /* 5D028 8006C828 EEFF03A6 */  sh         $v1, -0x12($s0)
    /* 5D02C 8006C82C 02004296 */  lhu        $v0, 0x2($s2)
    /* 5D030 8006C830 00000000 */  nop
    /* 5D034 8006C834 23105700 */  subu       $v0, $v0, $s7
    /* 5D038 8006C838 F0FF02A6 */  sh         $v0, -0x10($s0)
    /* 5D03C 8006C83C 00004296 */  lhu        $v0, 0x0($s2)
    /* 5D040 8006C840 00000000 */  nop
    /* 5D044 8006C844 21105300 */  addu       $v0, $v0, $s3
    /* 5D048 8006C848 F6FF02A6 */  sh         $v0, -0xA($s0)
    /* 5D04C 8006C84C 02004296 */  lhu        $v0, 0x2($s2)
    /* 5D050 8006C850 9000A88F */  lw         $t0, 0x90($sp)
    /* 5D054 8006C854 23105700 */  subu       $v0, $v0, $s7
    /* 5D058 8006C858 01004224 */  addiu      $v0, $v0, 0x1
    /* 5D05C 8006C85C 21104800 */  addu       $v0, $v0, $t0
    /* 5D060 8006C860 F8FF02A6 */  sh         $v0, -0x8($s0)
    /* 5D064 8006C864 00004296 */  lhu        $v0, 0x0($s2)
    /* 5D068 8006C868 04004396 */  lhu        $v1, 0x4($s2)
    /* 5D06C 8006C86C 21105300 */  addu       $v0, $v0, $s3
    /* 5D070 8006C870 21186200 */  addu       $v1, $v1, $v0
    /* 5D074 8006C874 FEFF03A6 */  sh         $v1, -0x2($s0)
    /* 5D078 8006C878 02004296 */  lhu        $v0, 0x2($s2)
    /* 5D07C 8006C87C 00000000 */  nop
    /* 5D080 8006C880 23105700 */  subu       $v0, $v0, $s7
    /* 5D084 8006C884 01004224 */  addiu      $v0, $v0, 0x1
  .L8006C888:
    /* 5D088 8006C888 21104800 */  addu       $v0, $v0, $t0
    /* 5D08C 8006C88C 000002A6 */  sh         $v0, 0x0($s0)
    /* 5D090 8006C890 2128A002 */  addu       $a1, $s5, $zero
    /* 5D094 8006C894 24001026 */  addiu      $s0, $s0, 0x24
    /* 5D098 8006C898 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5D09C 8006C89C 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5D0A0 8006C8A0 2400B526 */  addiu      $s5, $s5, 0x24
    /* 5D0A4 8006C8A4 2DEA010C */  jal        func_8007A8B4
    /* 5D0A8 8006C8A8 20008424 */   addiu     $a0, $a0, 0x20
    /* 5D0AC 8006C8AC 8CEA010C */  jal        func_8007AA30
    /* 5D0B0 8006C8B0 2120A002 */   addu      $a0, $s5, $zero
    /* 5D0B4 8006C8B4 05000224 */  addiu      $v0, $zero, 0x5
    /* 5D0B8 8006C8B8 4000C217 */  bne        $fp, $v0, .L8006C9BC
    /* 5D0BC 8006C8BC 2120A002 */   addu      $a0, $s5, $zero
    /* 5D0C0 8006C8C0 5AEA010C */  jal        func_8007A968
    /* 5D0C4 8006C8C4 01000524 */   addiu     $a1, $zero, 0x1
    /* 5D0C8 8006C8C8 E2FF14A2 */  sb         $s4, -0x1E($s0)
    /* 5D0CC 8006C8CC E3FF14A2 */  sb         $s4, -0x1D($s0)
    /* 5D0D0 8006C8D0 E4FF14A2 */  sb         $s4, -0x1C($s0)
    /* 5D0D4 8006C8D4 F2FF14A2 */  sb         $s4, -0xE($s0)
    /* 5D0D8 8006C8D8 F3FF14A2 */  sb         $s4, -0xD($s0)
    /* 5D0DC 8006C8DC F4FF14A2 */  sb         $s4, -0xC($s0)
    /* 5D0E0 8006C8E0 EAFF00A2 */  sb         $zero, -0x16($s0)
    /* 5D0E4 8006C8E4 EBFF00A2 */  sb         $zero, -0x15($s0)
    /* 5D0E8 8006C8E8 ECFF00A2 */  sb         $zero, -0x14($s0)
    /* 5D0EC 8006C8EC FAFF00A2 */  sb         $zero, -0x6($s0)
    /* 5D0F0 8006C8F0 FBFF00A2 */  sb         $zero, -0x5($s0)
    /* 5D0F4 8006C8F4 FCFF00A2 */  sb         $zero, -0x4($s0)
    /* 5D0F8 8006C8F8 04002286 */  lh         $v0, 0x4($s1)
    /* 5D0FC 8006C8FC 00000000 */  nop
    /* 5D100 8006C900 18005600 */  mult       $v0, $s6
    /* 5D104 8006C904 00002296 */  lhu        $v0, 0x0($s1)
    /* 5D108 8006C908 12400000 */  mflo       $t0
    /* 5D10C 8006C90C 21104800 */  addu       $v0, $v0, $t0
    /* 5D110 8006C910 21105300 */  addu       $v0, $v0, $s3
    /* 5D114 8006C914 E6FF02A6 */  sh         $v0, -0x1A($s0)
    /* 5D118 8006C918 02002296 */  lhu        $v0, 0x2($s1)
    /* 5D11C 8006C91C 00000000 */  nop
    /* 5D120 8006C920 01004224 */  addiu      $v0, $v0, 0x1
    /* 5D124 8006C924 E8FF02A6 */  sh         $v0, -0x18($s0)
    /* 5D128 8006C928 04002286 */  lh         $v0, 0x4($s1)
    /* 5D12C 8006C92C 00000000 */  nop
    /* 5D130 8006C930 18005600 */  mult       $v0, $s6
    /* 5D134 8006C934 00002296 */  lhu        $v0, 0x0($s1)
    /* 5D138 8006C938 12400000 */  mflo       $t0
    /* 5D13C 8006C93C 21104800 */  addu       $v0, $v0, $t0
    /* 5D140 8006C940 21105300 */  addu       $v0, $v0, $s3
    /* 5D144 8006C944 8800A88F */  lw         $t0, 0x88($sp)
    /* 5D148 8006C948 04004224 */  addiu      $v0, $v0, 0x4
    /* 5D14C 8006C94C 21104800 */  addu       $v0, $v0, $t0
    /* 5D150 8006C950 EEFF02A6 */  sh         $v0, -0x12($s0)
    /* 5D154 8006C954 02002296 */  lhu        $v0, 0x2($s1)
    /* 5D158 8006C958 00000000 */  nop
    /* 5D15C 8006C95C 01004224 */  addiu      $v0, $v0, 0x1
    /* 5D160 8006C960 F0FF02A6 */  sh         $v0, -0x10($s0)
    /* 5D164 8006C964 04002286 */  lh         $v0, 0x4($s1)
    /* 5D168 8006C968 00000000 */  nop
    /* 5D16C 8006C96C 18005600 */  mult       $v0, $s6
    /* 5D170 8006C970 00002296 */  lhu        $v0, 0x0($s1)
    /* 5D174 8006C974 12400000 */  mflo       $t0
    /* 5D178 8006C978 21104800 */  addu       $v0, $v0, $t0
    /* 5D17C 8006C97C 21105300 */  addu       $v0, $v0, $s3
    /* 5D180 8006C980 F6FF02A6 */  sh         $v0, -0xA($s0)
    /* 5D184 8006C984 0E002296 */  lhu        $v0, 0xE($s1)
    /* 5D188 8006C988 00000000 */  nop
    /* 5D18C 8006C98C FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 5D190 8006C990 F8FF02A6 */  sh         $v0, -0x8($s0)
    /* 5D194 8006C994 04002286 */  lh         $v0, 0x4($s1)
    /* 5D198 8006C998 00000000 */  nop
    /* 5D19C 8006C99C 18005600 */  mult       $v0, $s6
    /* 5D1A0 8006C9A0 00002296 */  lhu        $v0, 0x0($s1)
    /* 5D1A4 8006C9A4 12400000 */  mflo       $t0
    /* 5D1A8 8006C9A8 21104800 */  addu       $v0, $v0, $t0
    /* 5D1AC 8006C9AC 21105300 */  addu       $v0, $v0, $s3
    /* 5D1B0 8006C9B0 8800A88F */  lw         $t0, 0x88($sp)
    /* 5D1B4 8006C9B4 AEB20108 */  j          .L8006CAB8
    /* 5D1B8 8006C9B8 04004224 */   addiu     $v0, $v0, 0x4
  .L8006C9BC:
    /* 5D1BC 8006C9BC 5AEA010C */  jal        func_8007A968
    /* 5D1C0 8006C9C0 21280000 */   addu      $a1, $zero, $zero
    /* 5D1C4 8006C9C4 80000224 */  addiu      $v0, $zero, 0x80
    /* 5D1C8 8006C9C8 E2FF14A2 */  sb         $s4, -0x1E($s0)
    /* 5D1CC 8006C9CC E3FF00A2 */  sb         $zero, -0x1D($s0)
    /* 5D1D0 8006C9D0 E4FF00A2 */  sb         $zero, -0x1C($s0)
    /* 5D1D4 8006C9D4 F2FF14A2 */  sb         $s4, -0xE($s0)
    /* 5D1D8 8006C9D8 F3FF00A2 */  sb         $zero, -0xD($s0)
    /* 5D1DC 8006C9DC F4FF00A2 */  sb         $zero, -0xC($s0)
    /* 5D1E0 8006C9E0 EAFF02A2 */  sb         $v0, -0x16($s0)
    /* 5D1E4 8006C9E4 EBFF00A2 */  sb         $zero, -0x15($s0)
    /* 5D1E8 8006C9E8 ECFF00A2 */  sb         $zero, -0x14($s0)
    /* 5D1EC 8006C9EC FAFF02A2 */  sb         $v0, -0x6($s0)
    /* 5D1F0 8006C9F0 FBFF00A2 */  sb         $zero, -0x5($s0)
    /* 5D1F4 8006C9F4 FCFF00A2 */  sb         $zero, -0x4($s0)
    /* 5D1F8 8006C9F8 04002286 */  lh         $v0, 0x4($s1)
    /* 5D1FC 8006C9FC 00000000 */  nop
    /* 5D200 8006CA00 18005600 */  mult       $v0, $s6
    /* 5D204 8006CA04 00002296 */  lhu        $v0, 0x0($s1)
    /* 5D208 8006CA08 12400000 */  mflo       $t0
    /* 5D20C 8006CA0C 21104800 */  addu       $v0, $v0, $t0
    /* 5D210 8006CA10 21105300 */  addu       $v0, $v0, $s3
    /* 5D214 8006CA14 E6FF02A6 */  sh         $v0, -0x1A($s0)
    /* 5D218 8006CA18 02002296 */  lhu        $v0, 0x2($s1)
    /* 5D21C 8006CA1C 00000000 */  nop
    /* 5D220 8006CA20 01004224 */  addiu      $v0, $v0, 0x1
    /* 5D224 8006CA24 E8FF02A6 */  sh         $v0, -0x18($s0)
    /* 5D228 8006CA28 04002286 */  lh         $v0, 0x4($s1)
    /* 5D22C 8006CA2C 00000000 */  nop
    /* 5D230 8006CA30 18005600 */  mult       $v0, $s6
    /* 5D234 8006CA34 00002296 */  lhu        $v0, 0x0($s1)
    /* 5D238 8006CA38 12400000 */  mflo       $t0
    /* 5D23C 8006CA3C 21104800 */  addu       $v0, $v0, $t0
    /* 5D240 8006CA40 21105300 */  addu       $v0, $v0, $s3
    /* 5D244 8006CA44 8000A88F */  lw         $t0, 0x80($sp)
    /* 5D248 8006CA48 02004224 */  addiu      $v0, $v0, 0x2
    /* 5D24C 8006CA4C 21104800 */  addu       $v0, $v0, $t0
    /* 5D250 8006CA50 EEFF02A6 */  sh         $v0, -0x12($s0)
    /* 5D254 8006CA54 02002296 */  lhu        $v0, 0x2($s1)
    /* 5D258 8006CA58 00000000 */  nop
    /* 5D25C 8006CA5C 01004224 */  addiu      $v0, $v0, 0x1
    /* 5D260 8006CA60 F0FF02A6 */  sh         $v0, -0x10($s0)
    /* 5D264 8006CA64 04002286 */  lh         $v0, 0x4($s1)
    /* 5D268 8006CA68 00000000 */  nop
    /* 5D26C 8006CA6C 18005600 */  mult       $v0, $s6
    /* 5D270 8006CA70 00002296 */  lhu        $v0, 0x0($s1)
    /* 5D274 8006CA74 12400000 */  mflo       $t0
    /* 5D278 8006CA78 21104800 */  addu       $v0, $v0, $t0
    /* 5D27C 8006CA7C 21105300 */  addu       $v0, $v0, $s3
    /* 5D280 8006CA80 F6FF02A6 */  sh         $v0, -0xA($s0)
    /* 5D284 8006CA84 0E002296 */  lhu        $v0, 0xE($s1)
    /* 5D288 8006CA88 00000000 */  nop
    /* 5D28C 8006CA8C FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 5D290 8006CA90 F8FF02A6 */  sh         $v0, -0x8($s0)
    /* 5D294 8006CA94 04002286 */  lh         $v0, 0x4($s1)
    /* 5D298 8006CA98 00000000 */  nop
    /* 5D29C 8006CA9C 18005600 */  mult       $v0, $s6
    /* 5D2A0 8006CAA0 00002296 */  lhu        $v0, 0x0($s1)
    /* 5D2A4 8006CAA4 12400000 */  mflo       $t0
    /* 5D2A8 8006CAA8 21104800 */  addu       $v0, $v0, $t0
    /* 5D2AC 8006CAAC 21105300 */  addu       $v0, $v0, $s3
    /* 5D2B0 8006CAB0 8000A88F */  lw         $t0, 0x80($sp)
    /* 5D2B4 8006CAB4 02004224 */  addiu      $v0, $v0, 0x2
  .L8006CAB8:
    /* 5D2B8 8006CAB8 21104800 */  addu       $v0, $v0, $t0
    /* 5D2BC 8006CABC FEFF02A6 */  sh         $v0, -0x2($s0)
    /* 5D2C0 8006CAC0 0E002296 */  lhu        $v0, 0xE($s1)
    /* 5D2C4 8006CAC4 00000000 */  nop
    /* 5D2C8 8006CAC8 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 5D2CC 8006CACC 000002A6 */  sh         $v0, 0x0($s0)
    /* 5D2D0 8006CAD0 2128A002 */  addu       $a1, $s5, $zero
    /* 5D2D4 8006CAD4 24001026 */  addiu      $s0, $s0, 0x24
    /* 5D2D8 8006CAD8 2400B526 */  addiu      $s5, $s5, 0x24
    /* 5D2DC 8006CADC 0C005226 */  addiu      $s2, $s2, 0xC
    /* 5D2E0 8006CAE0 0100F726 */  addiu      $s7, $s7, 0x1
    /* 5D2E4 8006CAE4 8000A88F */  lw         $t0, 0x80($sp)
    /* 5D2E8 8006CAE8 0100D626 */  addiu      $s6, $s6, 0x1
    /* 5D2EC 8006CAEC FCFF0825 */  addiu      $t0, $t0, -0x4
    /* 5D2F0 8006CAF0 8000A8AF */  sw         $t0, 0x80($sp)
    /* 5D2F4 8006CAF4 8800A88F */  lw         $t0, 0x88($sp)
    /* 5D2F8 8006CAF8 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5D2FC 8006CAFC 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5D300 8006CB00 F8FF0825 */  addiu      $t0, $t0, -0x8
    /* 5D304 8006CB04 8800A8AF */  sw         $t0, 0x88($sp)
    /* 5D308 8006CB08 9000A88F */  lw         $t0, 0x90($sp)
    /* 5D30C 8006CB0C 20008424 */  addiu      $a0, $a0, 0x20
    /* 5D310 8006CB10 FEFF0825 */  addiu      $t0, $t0, -0x2
    /* 5D314 8006CB14 2DEA010C */  jal        func_8007A8B4
    /* 5D318 8006CB18 9000A8AF */   sw        $t0, 0x90($sp)
    /* 5D31C 8006CB1C 0200C22A */  slti       $v0, $s6, 0x2
    /* 5D320 8006CB20 ECFE4014 */  bnez       $v0, .L8006C6D4
    /* 5D324 8006CB24 01000524 */   addiu     $a1, $zero, 0x1
    /* 5D328 8006CB28 4800A88F */  lw         $t0, 0x48($sp)
    /* 5D32C 8006CB2C 21300000 */  addu       $a2, $zero, $zero
    /* 5D330 8006CB30 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5D334 8006CB34 1C00048D */  lw         $a0, 0x1C($t0)
    /* 5D338 8006CB38 92F0010C */  jal        func_8007C248
    /* 5D33C 8006CB3C 40000724 */   addiu     $a3, $zero, 0x40
    /* 5D340 8006CB40 4800A88F */  lw         $t0, 0x48($sp)
    /* 5D344 8006CB44 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5D348 8006CB48 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5D34C 8006CB4C 1C00058D */  lw         $a1, 0x1C($t0)
    /* 5D350 8006CB50 2DEA010C */  jal        func_8007A8B4
    /* 5D354 8006CB54 20008424 */   addiu     $a0, $a0, 0x20
    /* 5D358 8006CB58 4800A88F */  lw         $t0, 0x48($sp)
    /* 5D35C 8006CB5C 00000000 */  nop
    /* 5D360 8006CB60 1C00028D */  lw         $v0, 0x1C($t0)
    /* 5D364 8006CB64 5000A88F */  lw         $t0, 0x50($sp)
    /* 5D368 8006CB68 00000000 */  nop
    /* 5D36C 8006CB6C 01000825 */  addiu      $t0, $t0, 0x1
    /* 5D370 8006CB70 5000A8AF */  sw         $t0, 0x50($sp)
    /* 5D374 8006CB74 4800A88F */  lw         $t0, 0x48($sp)
    /* 5D378 8006CB78 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5D37C 8006CB7C 1C0002AD */  sw         $v0, 0x1C($t0)
    /* 5D380 8006CB80 5000A88F */  lw         $t0, 0x50($sp)
    /* 5D384 8006CB84 00000000 */  nop
    /* 5D388 8006CB88 02000229 */  slti       $v0, $t0, 0x2
    /* 5D38C 8006CB8C BFFE4014 */  bnez       $v0, .L8006C68C
    /* 5D390 8006CB90 18017326 */   addiu     $s3, $s3, 0x118
    /* 5D394 8006CB94 4800A88F */  lw         $t0, 0x48($sp)
    /* 5D398 8006CB98 00000000 */  nop
    /* 5D39C 8006CB9C 100015AD */  sw         $s5, 0x10($t0)
    /* 5D3A0 8006CBA0 BC00BF8F */  lw         $ra, 0xBC($sp)
    /* 5D3A4 8006CBA4 B800BE8F */  lw         $fp, 0xB8($sp)
    /* 5D3A8 8006CBA8 B400B78F */  lw         $s7, 0xB4($sp)
    /* 5D3AC 8006CBAC B000B68F */  lw         $s6, 0xB0($sp)
    /* 5D3B0 8006CBB0 AC00B58F */  lw         $s5, 0xAC($sp)
    /* 5D3B4 8006CBB4 A800B48F */  lw         $s4, 0xA8($sp)
    /* 5D3B8 8006CBB8 A400B38F */  lw         $s3, 0xA4($sp)
    /* 5D3BC 8006CBBC A000B28F */  lw         $s2, 0xA0($sp)
    /* 5D3C0 8006CBC0 9C00B18F */  lw         $s1, 0x9C($sp)
    /* 5D3C4 8006CBC4 9800B08F */  lw         $s0, 0x98($sp)
    /* 5D3C8 8006CBC8 C000BD27 */  addiu      $sp, $sp, 0xC0
    /* 5D3CC 8006CBCC 0800E003 */  jr         $ra
    /* 5D3D0 8006CBD0 00000000 */   nop
endlabel func_8006C21C
