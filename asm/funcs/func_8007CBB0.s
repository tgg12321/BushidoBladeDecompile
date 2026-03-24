glabel func_8007CBB0
    /* 6D3B0 8007CBB0 C0FFBD27 */  addiu      $sp, $sp, -0x40
    /* 6D3B4 8007CBB4 21408000 */  addu       $t0, $a0, $zero
    /* 6D3B8 8007CBB8 3800BFAF */  sw         $ra, 0x38($sp)
    /* 6D3BC 8007CBBC 3400B1AF */  sw         $s1, 0x34($sp)
    /* 6D3C0 8007CBC0 3000B0AF */  sw         $s0, 0x30($sp)
    /* 6D3C4 8007CBC4 04000485 */  lh         $a0, 0x4($t0)
    /* 6D3C8 8007CBC8 2148A000 */  addu       $t1, $a1, $zero
    /* 6D3CC 8007CBCC 0B008004 */  bltz       $a0, .L8007CBFC
    /* 6D3D0 8007CBD0 21288000 */   addu      $a1, $a0, $zero
    /* 6D3D4 8007CBD4 0A80023C */  lui        $v0, %hi(D_8009BE78)
    /* 6D3D8 8007CBD8 78BE4284 */  lh         $v0, %lo(D_8009BE78)($v0)
    /* 6D3DC 8007CBDC 00000000 */  nop
    /* 6D3E0 8007CBE0 21184000 */  addu       $v1, $v0, $zero
    /* 6D3E4 8007CBE4 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6D3E8 8007CBE8 2A104400 */  slt        $v0, $v0, $a0
    /* 6D3EC 8007CBEC 04004014 */  bnez       $v0, .L8007CC00
    /* 6D3F0 8007CBF0 FFFF6224 */   addiu     $v0, $v1, -0x1
    /* 6D3F4 8007CBF4 00F30108 */  j          .L8007CC00
    /* 6D3F8 8007CBF8 2110A000 */   addu      $v0, $a1, $zero
  .L8007CBFC:
    /* 6D3FC 8007CBFC 21100000 */  addu       $v0, $zero, $zero
  .L8007CC00:
    /* 6D400 8007CC00 06000585 */  lh         $a1, 0x6($t0)
    /* 6D404 8007CC04 040002A5 */  sh         $v0, 0x4($t0)
    /* 6D408 8007CC08 0B00A004 */  bltz       $a1, .L8007CC38
    /* 6D40C 8007CC0C 2120A000 */   addu      $a0, $a1, $zero
    /* 6D410 8007CC10 0A80023C */  lui        $v0, %hi(D_8009BE7A)
    /* 6D414 8007CC14 7ABE4284 */  lh         $v0, %lo(D_8009BE7A)($v0)
    /* 6D418 8007CC18 00000000 */  nop
    /* 6D41C 8007CC1C 21184000 */  addu       $v1, $v0, $zero
    /* 6D420 8007CC20 FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 6D424 8007CC24 2A104500 */  slt        $v0, $v0, $a1
    /* 6D428 8007CC28 04004014 */  bnez       $v0, .L8007CC3C
    /* 6D42C 8007CC2C FFFF6324 */   addiu     $v1, $v1, -0x1
    /* 6D430 8007CC30 0FF30108 */  j          .L8007CC3C
    /* 6D434 8007CC34 21188000 */   addu      $v1, $a0, $zero
  .L8007CC38:
    /* 6D438 8007CC38 21180000 */  addu       $v1, $zero, $zero
  .L8007CC3C:
    /* 6D43C 8007CC3C 00000295 */  lhu        $v0, 0x0($t0)
    /* 6D440 8007CC40 00000000 */  nop
    /* 6D444 8007CC44 3F004230 */  andi       $v0, $v0, 0x3F
    /* 6D448 8007CC48 06004014 */  bnez       $v0, .L8007CC64
    /* 6D44C 8007CC4C 060003A5 */   sh        $v1, 0x6($t0)
    /* 6D450 8007CC50 04000295 */  lhu        $v0, 0x4($t0)
    /* 6D454 8007CC54 00000000 */  nop
    /* 6D458 8007CC58 3F004230 */  andi       $v0, $v0, 0x3F
    /* 6D45C 8007CC5C 42004010 */  beqz       $v0, .L8007CD68
    /* 6D460 8007CC60 FF05023C */   lui       $v0, (0x5FFFFFF >> 16)
  .L8007CC64:
    /* 6D464 8007CC64 FF00053C */  lui        $a1, (0xFFFFFF >> 16)
    /* 6D468 8007CC68 FFFFA534 */  ori        $a1, $a1, (0xFFFFFF & 0xFFFF)
    /* 6D46C 8007CC6C FFE4043C */  lui        $a0, (0xE4FFFFFF >> 16)
    /* 6D470 8007CC70 FFFF8434 */  ori        $a0, $a0, (0xE4FFFFFF & 0xFFFF)
    /* 6D474 8007CC74 FF03073C */  lui        $a3, (0x3FFFFFF >> 16)
    /* 6D478 8007CC78 0F80063C */  lui        $a2, %hi(D_800F187C)
    /* 6D47C 8007CC7C 7C18C624 */  addiu      $a2, $a2, %lo(D_800F187C)
    /* 6D480 8007CC80 2410C500 */  and        $v0, $a2, $a1
    /* 6D484 8007CC84 0008033C */  lui        $v1, (0x8000000 >> 16)
    /* 6D488 8007CC88 25104300 */  or         $v0, $v0, $v1
    /* 6D48C 8007CC8C 00E3103C */  lui        $s0, (0xE3000000 >> 16)
    /* 6D490 8007CC90 00E5113C */  lui        $s1, (0xE5000000 >> 16)
    /* 6D494 8007CC94 0F80013C */  lui        $at, %hi(D_800F1858)
    /* 6D498 8007CC98 581822AC */  sw         $v0, %lo(D_800F1858)($at)
    /* 6D49C 8007CC9C 00E6023C */  lui        $v0, (0xE6000000 >> 16)
    /* 6D4A0 8007CCA0 24282501 */  and        $a1, $t1, $a1
    /* 6D4A4 8007CCA4 0060033C */  lui        $v1, (0x60000000 >> 16)
    /* 6D4A8 8007CCA8 2528A300 */  or         $a1, $a1, $v1
    /* 6D4AC 8007CCAC 0F80013C */  lui        $at, %hi(D_800F1868)
    /* 6D4B0 8007CCB0 681822AC */  sw         $v0, %lo(D_800F1868)($at)
    /* 6D4B4 8007CCB4 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6D4B8 8007CCB8 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6D4BC 8007CCBC 00E1033C */  lui        $v1, (0xE1000000 >> 16)
    /* 6D4C0 8007CCC0 0F80013C */  lui        $at, %hi(D_800F185C)
    /* 6D4C4 8007CCC4 5C1830AC */  sw         $s0, %lo(D_800F185C)($at)
    /* 6D4C8 8007CCC8 0F80013C */  lui        $at, %hi(D_800F1860)
    /* 6D4CC 8007CCCC 601824AC */  sw         $a0, %lo(D_800F1860)($at)
    /* 6D4D0 8007CCD0 0F80013C */  lui        $at, %hi(D_800F1864)
    /* 6D4D4 8007CCD4 641831AC */  sw         $s1, %lo(D_800F1864)($at)
    /* 6D4D8 8007CCD8 0000448C */  lw         $a0, 0x0($v0)
    /* 6D4DC 8007CCDC C2170900 */  srl        $v0, $t1, 31
    /* 6D4E0 8007CCE0 80120200 */  sll        $v0, $v0, 10
    /* 6D4E4 8007CCE4 25104300 */  or         $v0, $v0, $v1
    /* 6D4E8 8007CCE8 0F80013C */  lui        $at, %hi(D_800F1870)
    /* 6D4EC 8007CCEC 701825AC */  sw         $a1, %lo(D_800F1870)($at)
    /* 6D4F0 8007CCF0 FF078430 */  andi       $a0, $a0, 0x7FF
    /* 6D4F4 8007CCF4 25208200 */  or         $a0, $a0, $v0
    /* 6D4F8 8007CCF8 0F80013C */  lui        $at, %hi(D_800F186C)
    /* 6D4FC 8007CCFC 6C1824AC */  sw         $a0, %lo(D_800F186C)($at)
    /* 6D500 8007CD00 0000028D */  lw         $v0, 0x0($t0)
    /* 6D504 8007CD04 FFFFE734 */  ori        $a3, $a3, (0x3FFFFFF & 0xFFFF)
    /* 6D508 8007CD08 0F80013C */  lui        $at, %hi(D_800F1874)
    /* 6D50C 8007CD0C 741822AC */  sw         $v0, %lo(D_800F1874)($at)
    /* 6D510 8007CD10 0400028D */  lw         $v0, 0x4($t0)
    /* 6D514 8007CD14 0000C7AC */  sw         $a3, 0x0($a2)
    /* 6D518 8007CD18 0F80013C */  lui        $at, %hi(D_800F1878)
    /* 6D51C 8007CD1C 781822AC */  sw         $v0, %lo(D_800F1878)($at)
    /* 6D520 8007CD20 E9F4010C */  jal        func_8007D3A4
    /* 6D524 8007CD24 03000424 */   addiu     $a0, $zero, 0x3
    /* 6D528 8007CD28 25105000 */  or         $v0, $v0, $s0
    /* 6D52C 8007CD2C 0F80013C */  lui        $at, %hi(D_800F1880)
    /* 6D530 8007CD30 801822AC */  sw         $v0, %lo(D_800F1880)($at)
    /* 6D534 8007CD34 E9F4010C */  jal        func_8007D3A4
    /* 6D538 8007CD38 04000424 */   addiu     $a0, $zero, 0x4
    /* 6D53C 8007CD3C 00E4033C */  lui        $v1, (0xE4000000 >> 16)
    /* 6D540 8007CD40 25104300 */  or         $v0, $v0, $v1
    /* 6D544 8007CD44 0F80013C */  lui        $at, %hi(D_800F1884)
    /* 6D548 8007CD48 841822AC */  sw         $v0, %lo(D_800F1884)($at)
    /* 6D54C 8007CD4C E9F4010C */  jal        func_8007D3A4
    /* 6D550 8007CD50 05000424 */   addiu     $a0, $zero, 0x5
    /* 6D554 8007CD54 25105100 */  or         $v0, $v0, $s1
    /* 6D558 8007CD58 0F80013C */  lui        $at, %hi(D_800F1888)
    /* 6D55C 8007CD5C 881822AC */  sw         $v0, %lo(D_800F1888)($at)
    /* 6D560 8007CD60 78F30108 */  j          .L8007CDE0
    /* 6D564 8007CD64 00000000 */   nop
  .L8007CD68:
    /* 6D568 8007CD68 FFFF4234 */  ori        $v0, $v0, (0x5FFFFFF & 0xFFFF)
    /* 6D56C 8007CD6C FF00033C */  lui        $v1, (0xFFFFFF >> 16)
    /* 6D570 8007CD70 FFFF6334 */  ori        $v1, $v1, (0xFFFFFF & 0xFFFF)
    /* 6D574 8007CD74 0F80013C */  lui        $at, %hi(D_800F1858)
    /* 6D578 8007CD78 581822AC */  sw         $v0, %lo(D_800F1858)($at)
    /* 6D57C 8007CD7C 00E6023C */  lui        $v0, (0xE6000000 >> 16)
    /* 6D580 8007CD80 24182301 */  and        $v1, $t1, $v1
    /* 6D584 8007CD84 0002053C */  lui        $a1, (0x2000000 >> 16)
    /* 6D588 8007CD88 0F80013C */  lui        $at, %hi(D_800F185C)
    /* 6D58C 8007CD8C 5C1822AC */  sw         $v0, %lo(D_800F185C)($at)
    /* 6D590 8007CD90 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6D594 8007CD94 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6D598 8007CD98 25186500 */  or         $v1, $v1, $a1
    /* 6D59C 8007CD9C 0000448C */  lw         $a0, 0x0($v0)
    /* 6D5A0 8007CDA0 C2170900 */  srl        $v0, $t1, 31
    /* 6D5A4 8007CDA4 80120200 */  sll        $v0, $v0, 10
    /* 6D5A8 8007CDA8 0F80013C */  lui        $at, %hi(D_800F1864)
    /* 6D5AC 8007CDAC 641823AC */  sw         $v1, %lo(D_800F1864)($at)
    /* 6D5B0 8007CDB0 00E1033C */  lui        $v1, (0xE1000000 >> 16)
    /* 6D5B4 8007CDB4 25104300 */  or         $v0, $v0, $v1
    /* 6D5B8 8007CDB8 FF078430 */  andi       $a0, $a0, 0x7FF
    /* 6D5BC 8007CDBC 25208200 */  or         $a0, $a0, $v0
    /* 6D5C0 8007CDC0 0F80013C */  lui        $at, %hi(D_800F1860)
    /* 6D5C4 8007CDC4 601824AC */  sw         $a0, %lo(D_800F1860)($at)
    /* 6D5C8 8007CDC8 0000028D */  lw         $v0, 0x0($t0)
    /* 6D5CC 8007CDCC 0F80013C */  lui        $at, %hi(D_800F1868)
    /* 6D5D0 8007CDD0 681822AC */  sw         $v0, %lo(D_800F1868)($at)
    /* 6D5D4 8007CDD4 0400028D */  lw         $v0, 0x4($t0)
    /* 6D5D8 8007CDD8 0F80013C */  lui        $at, %hi(D_800F186C)
    /* 6D5DC 8007CDDC 6C1822AC */  sw         $v0, %lo(D_800F186C)($at)
  .L8007CDE0:
    /* 6D5E0 8007CDE0 0F80043C */  lui        $a0, %hi(D_800F1858)
    /* 6D5E4 8007CDE4 58188424 */  addiu      $a0, $a0, %lo(D_800F1858)
    /* 6D5E8 8007CDE8 D6F4010C */  jal        func_8007D358
    /* 6D5EC 8007CDEC 00000000 */   nop
    /* 6D5F0 8007CDF0 21100000 */  addu       $v0, $zero, $zero
    /* 6D5F4 8007CDF4 3800BF8F */  lw         $ra, 0x38($sp)
    /* 6D5F8 8007CDF8 3400B18F */  lw         $s1, 0x34($sp)
    /* 6D5FC 8007CDFC 3000B08F */  lw         $s0, 0x30($sp)
    /* 6D600 8007CE00 4000BD27 */  addiu      $sp, $sp, 0x40
    /* 6D604 8007CE04 0800E003 */  jr         $ra
    /* 6D608 8007CE08 00000000 */   nop
endlabel func_8007CBB0
