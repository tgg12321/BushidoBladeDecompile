glabel func_8007E1AC
    /* 6E9AC 8007E1AC 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6E9B0 8007E1B0 000089C8 */  lwc2       $9, 0x0($a0)
    /* 6E9B4 8007E1B4 04008AC8 */  lwc2       $10, 0x4($a0)
    /* 6E9B8 8007E1B8 08008BC8 */  lwc2       $11, 0x8($a0)
    /* 6E9BC 8007E1BC 00000000 */  nop
    /* 6E9C0 8007E1C0 3D00984B */  gpf        1
    /* 6E9C4 8007E1C4 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6E9C8 8007E1C8 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6E9CC 8007E1CC 0000A9C8 */  lwc2       $9, 0x0($a1)
    /* 6E9D0 8007E1D0 0400AAC8 */  lwc2       $10, 0x4($a1)
    /* 6E9D4 8007E1D4 0800ABC8 */  lwc2       $11, 0x8($a1)
    /* 6E9D8 8007E1D8 00000000 */  nop
    /* 6E9DC 8007E1DC 3E00A84B */  gpl        1
    /* 6E9E0 8007E1E0 1000A88F */  lw         $t0, 0x10($sp)
    /* 6E9E4 8007E1E4 00000000 */  nop
    /* 6E9E8 8007E1E8 000009E9 */  swc2       $9, 0x0($t0)
    /* 6E9EC 8007E1EC 04000AE9 */  swc2       $10, 0x4($t0)
    /* 6E9F0 8007E1F0 08000BE9 */  swc2       $11, 0x8($t0)
    /* 6E9F4 8007E1F4 0800E003 */  jr         $ra
    /* 6E9F8 8007E1F8 00000000 */   nop
    /* 6E9FC 8007E1FC 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6EA00 8007E200 000089C8 */  lwc2       $9, 0x0($a0)
    /* 6EA04 8007E204 04008AC8 */  lwc2       $10, 0x4($a0)
    /* 6EA08 8007E208 08008BC8 */  lwc2       $11, 0x8($a0)
    /* 6EA0C 8007E20C 00000000 */  nop
    /* 6EA10 8007E210 3D00904B */  gpf        0
    /* 6EA14 8007E214 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6EA18 8007E218 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6EA1C 8007E21C 0000A9C8 */  lwc2       $9, 0x0($a1)
    /* 6EA20 8007E220 0400AAC8 */  lwc2       $10, 0x4($a1)
    /* 6EA24 8007E224 0800ABC8 */  lwc2       $11, 0x8($a1)
    /* 6EA28 8007E228 00000000 */  nop
    /* 6EA2C 8007E22C 3E00A04B */  gpl        0
    /* 6EA30 8007E230 1000A88F */  lw         $t0, 0x10($sp)
    /* 6EA34 8007E234 00000000 */  nop
    /* 6EA38 8007E238 000009E9 */  swc2       $9, 0x0($t0)
    /* 6EA3C 8007E23C 04000AE9 */  swc2       $10, 0x4($t0)
    /* 6EA40 8007E240 08000BE9 */  swc2       $11, 0x8($t0)
    /* 6EA44 8007E244 0800E003 */  jr         $ra
    /* 6EA48 8007E248 00000000 */   nop
    /* 6EA4C 8007E24C 0000888C */  lw         $t0, 0x0($a0)
    /* 6EA50 8007E250 04008A8C */  lw         $t2, 0x4($a0)
    /* 6EA54 8007E254 034C0800 */  sra        $t1, $t0, 16
    /* 6EA58 8007E258 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EA5C 8007E25C FFFF4A31 */  andi       $t2, $t2, 0xFFFF
    /* 6EA60 8007E260 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6EA64 8007E264 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EA68 8007E268 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EA6C 8007E26C 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6EA70 8007E270 00000000 */  nop
    /* 6EA74 8007E274 3D00984B */  gpf        1
    /* 6EA78 8007E278 0000A88C */  lw         $t0, 0x0($a1)
    /* 6EA7C 8007E27C 0400AA8C */  lw         $t2, 0x4($a1)
    /* 6EA80 8007E280 034C0800 */  sra        $t1, $t0, 16
    /* 6EA84 8007E284 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EA88 8007E288 FFFF4A31 */  andi       $t2, $t2, 0xFFFF
    /* 6EA8C 8007E28C 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6EA90 8007E290 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6EA94 8007E294 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EA98 8007E298 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EA9C 8007E29C 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6EAA0 8007E2A0 00000000 */  nop
    /* 6EAA4 8007E2A4 3E00A84B */  gpl        1
    /* 6EAA8 8007E2A8 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 6EAAC 8007E2AC 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 6EAB0 8007E2B0 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EAB4 8007E2B4 004C0900 */  sll        $t1, $t1, 16
    /* 6EAB8 8007E2B8 25400901 */  or         $t0, $t0, $t1
    /* 6EABC 8007E2BC 1000AD8F */  lw         $t5, 0x10($sp)
    /* 6EAC0 8007E2C0 00580A48 */  mfc2       $t2, $11 /* handwritten instruction */
    /* 6EAC4 8007E2C4 0000A8AD */  sw         $t0, 0x0($t5)
    /* 6EAC8 8007E2C8 0400AAAD */  sw         $t2, 0x4($t5)
    /* 6EACC 8007E2CC 0800E003 */  jr         $ra
    /* 6EAD0 8007E2D0 00000000 */   nop
    /* 6EAD4 8007E2D4 0000888C */  lw         $t0, 0x0($a0)
    /* 6EAD8 8007E2D8 04008A8C */  lw         $t2, 0x4($a0)
    /* 6EADC 8007E2DC 034C0800 */  sra        $t1, $t0, 16
    /* 6EAE0 8007E2E0 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EAE4 8007E2E4 FFFF4A31 */  andi       $t2, $t2, 0xFFFF
    /* 6EAE8 8007E2E8 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6EAEC 8007E2EC 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EAF0 8007E2F0 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EAF4 8007E2F4 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6EAF8 8007E2F8 00000000 */  nop
    /* 6EAFC 8007E2FC 3D00904B */  gpf        0
    /* 6EB00 8007E300 0000A88C */  lw         $t0, 0x0($a1)
    /* 6EB04 8007E304 0400AA8C */  lw         $t2, 0x4($a1)
    /* 6EB08 8007E308 034C0800 */  sra        $t1, $t0, 16
    /* 6EB0C 8007E30C FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EB10 8007E310 FFFF4A31 */  andi       $t2, $t2, 0xFFFF
    /* 6EB14 8007E314 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6EB18 8007E318 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6EB1C 8007E31C 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EB20 8007E320 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EB24 8007E324 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6EB28 8007E328 00000000 */  nop
    /* 6EB2C 8007E32C 3E00A04B */  gpl        0
    /* 6EB30 8007E330 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 6EB34 8007E334 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 6EB38 8007E338 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 6EB3C 8007E33C 004C0900 */  sll        $t1, $t1, 16
    /* 6EB40 8007E340 25400901 */  or         $t0, $t0, $t1
    /* 6EB44 8007E344 1000AD8F */  lw         $t5, 0x10($sp)
    /* 6EB48 8007E348 00580A48 */  mfc2       $t2, $11 /* handwritten instruction */
    /* 6EB4C 8007E34C 0000A8AD */  sw         $t0, 0x0($t5)
    /* 6EB50 8007E350 0400AAAD */  sw         $t2, 0x4($t5)
    /* 6EB54 8007E354 0800E003 */  jr         $ra
    /* 6EB58 8007E358 00000000 */   nop
    /* 6EB5C 8007E35C 00008890 */  lbu        $t0, 0x0($a0)
    /* 6EB60 8007E360 01008990 */  lbu        $t1, 0x1($a0)
    /* 6EB64 8007E364 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6EB68 8007E368 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EB6C 8007E36C 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EB70 8007E370 00000000 */  nop
    /* 6EB74 8007E374 3D00904B */  gpf        0
    /* 6EB78 8007E378 0000A890 */  lbu        $t0, 0x0($a1)
    /* 6EB7C 8007E37C 0100A990 */  lbu        $t1, 0x1($a1)
    /* 6EB80 8007E380 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6EB84 8007E384 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6EB88 8007E388 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EB8C 8007E38C 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EB90 8007E390 0C000B24 */  addiu      $t3, $zero, 0xC
    /* 6EB94 8007E394 3E00A04B */  gpl        0
    /* 6EB98 8007E398 1000AD8F */  lw         $t5, 0x10($sp)
    /* 6EB9C 8007E39C 00C80848 */  mfc2       $t0, $25 /* handwritten instruction */
    /* 6EBA0 8007E3A0 00D00948 */  mfc2       $t1, $26 /* handwritten instruction */
    /* 6EBA4 8007E3A4 07406801 */  srav       $t0, $t0, $t3
    /* 6EBA8 8007E3A8 07486901 */  srav       $t1, $t1, $t3
    /* 6EBAC 8007E3AC 0000A8A1 */  sb         $t0, 0x0($t5)
    /* 6EBB0 8007E3B0 0100A9A1 */  sb         $t1, 0x1($t5)
    /* 6EBB4 8007E3B4 0800E003 */  jr         $ra
    /* 6EBB8 8007E3B8 00000000 */   nop
    /* 6EBBC 8007E3BC 00008890 */  lbu        $t0, 0x0($a0)
    /* 6EBC0 8007E3C0 01008990 */  lbu        $t1, 0x1($a0)
    /* 6EBC4 8007E3C4 02008A90 */  lbu        $t2, 0x2($a0)
    /* 6EBC8 8007E3C8 00408648 */  mtc2       $a2, $8 /* handwritten instruction */
    /* 6EBCC 8007E3CC 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EBD0 8007E3D0 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EBD4 8007E3D4 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6EBD8 8007E3D8 00000000 */  nop
    /* 6EBDC 8007E3DC 3D00904B */  gpf        0
    /* 6EBE0 8007E3E0 0000A890 */  lbu        $t0, 0x0($a1)
    /* 6EBE4 8007E3E4 0100A990 */  lbu        $t1, 0x1($a1)
    /* 6EBE8 8007E3E8 0200AA90 */  lbu        $t2, 0x2($a1)
    /* 6EBEC 8007E3EC 00F80248 */  mfc2       $v0, $31 /* handwritten instruction */
    /* 6EBF0 8007E3F0 00408748 */  mtc2       $a3, $8 /* handwritten instruction */
    /* 6EBF4 8007E3F4 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 6EBF8 8007E3F8 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 6EBFC 8007E3FC 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 6EC00 8007E400 0C000B24 */  addiu      $t3, $zero, 0xC
    /* 6EC04 8007E404 3E00A04B */  gpl        0
    /* 6EC08 8007E408 1000AD8F */  lw         $t5, 0x10($sp)
    /* 6EC0C 8007E40C 00C80848 */  mfc2       $t0, $25 /* handwritten instruction */
    /* 6EC10 8007E410 00D00948 */  mfc2       $t1, $26 /* handwritten instruction */
    /* 6EC14 8007E414 00D80A48 */  mfc2       $t2, $27 /* handwritten instruction */
    /* 6EC18 8007E418 07406801 */  srav       $t0, $t0, $t3
    /* 6EC1C 8007E41C 07486901 */  srav       $t1, $t1, $t3
    /* 6EC20 8007E420 07506A01 */  srav       $t2, $t2, $t3
    /* 6EC24 8007E424 0000A8A1 */  sb         $t0, 0x0($t5)
    /* 6EC28 8007E428 0100A9A1 */  sb         $t1, 0x1($t5)
    /* 6EC2C 8007E42C 0200AAA1 */  sb         $t2, 0x2($t5)
    /* 6EC30 8007E430 0800E003 */  jr         $ra
    /* 6EC34 8007E434 00000000 */   nop
endlabel func_8007E1AC
