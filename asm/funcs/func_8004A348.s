glabel func_8004A348
    /* 3AB48 8004A348 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3AB4C 8004A34C 3400BFAF */  sw         $ra, 0x34($sp)
    /* 3AB50 8004A350 3000BEAF */  sw         $fp, 0x30($sp)
    /* 3AB54 8004A354 2C00B7AF */  sw         $s7, 0x2C($sp)
    /* 3AB58 8004A358 2800B6AF */  sw         $s6, 0x28($sp)
    /* 3AB5C 8004A35C 2400B5AF */  sw         $s5, 0x24($sp)
    /* 3AB60 8004A360 2000B4AF */  sw         $s4, 0x20($sp)
    /* 3AB64 8004A364 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 3AB68 8004A368 1800B2AF */  sw         $s2, 0x18($sp)
    /* 3AB6C 8004A36C 1400B1AF */  sw         $s1, 0x14($sp)
    /* 3AB70 8004A370 1000B0AF */  sw         $s0, 0x10($sp)
    /* 3AB74 8004A374 0980023C */  lui        $v0, %hi(D_800973FC)
    /* 3AB78 8004A378 FC734224 */  addiu      $v0, $v0, %lo(D_800973FC)
    /* 3AB7C 8004A37C 00008894 */  lhu        $t0, 0x0($a0)
    /* 3AB80 8004A380 02008994 */  lhu        $t1, 0x2($a0)
    /* 3AB84 8004A384 04008A94 */  lhu        $t2, 0x4($a0)
    /* 3AB88 8004A388 FF0F0831 */  andi       $t0, $t0, 0xFFF
    /* 3AB8C 8004A38C FF0F2931 */  andi       $t1, $t1, 0xFFF
    /* 3AB90 8004A390 FF0F4A31 */  andi       $t2, $t2, 0xFFF
    /* 3AB94 8004A394 20400801 */  add        $t0, $t0, $t0 /* handwritten instruction */
    /* 3AB98 8004A398 20482901 */  add        $t1, $t1, $t1 /* handwritten instruction */
    /* 3AB9C 8004A39C 20504A01 */  add        $t2, $t2, $t2 /* handwritten instruction */
    /* 3ABA0 8004A3A0 20584800 */  add        $t3, $v0, $t0 /* handwritten instruction */
    /* 3ABA4 8004A3A4 20604900 */  add        $t4, $v0, $t1 /* handwritten instruction */
    /* 3ABA8 8004A3A8 20684A00 */  add        $t5, $v0, $t2 /* handwritten instruction */
    /* 3ABAC 8004A3AC 00007085 */  lh         $s0, 0x0($t3)
    /* 3ABB0 8004A3B0 00009185 */  lh         $s1, 0x0($t4)
    /* 3ABB4 8004A3B4 0000B285 */  lh         $s2, 0x0($t5)
    /* 3ABB8 8004A3B8 00080821 */  addi       $t0, $t0, 0x800 /* handwritten instruction */
    /* 3ABBC 8004A3BC 00082921 */  addi       $t1, $t1, 0x800 /* handwritten instruction */
    /* 3ABC0 8004A3C0 00084A21 */  addi       $t2, $t2, 0x800 /* handwritten instruction */
    /* 3ABC4 8004A3C4 FE1F0831 */  andi       $t0, $t0, 0x1FFE
    /* 3ABC8 8004A3C8 FE1F2931 */  andi       $t1, $t1, 0x1FFE
    /* 3ABCC 8004A3CC FE1F4A31 */  andi       $t2, $t2, 0x1FFE
    /* 3ABD0 8004A3D0 20404800 */  add        $t0, $v0, $t0 /* handwritten instruction */
    /* 3ABD4 8004A3D4 20484900 */  add        $t1, $v0, $t1 /* handwritten instruction */
    /* 3ABD8 8004A3D8 20504A00 */  add        $t2, $v0, $t2 /* handwritten instruction */
    /* 3ABDC 8004A3DC 00004D85 */  lh         $t5, 0x0($t2)
    /* 3ABE0 8004A3E0 00003485 */  lh         $s4, 0x0($t1)
    /* 3ABE4 8004A3E4 00001385 */  lh         $s3, 0x0($t0)
    /* 3ABE8 8004A3E8 00408D48 */  mtc2       $t5, $8 /* handwritten instruction */
    /* 3ABEC 8004A3EC 00489048 */  mtc2       $s0, $9 /* handwritten instruction */
    /* 3ABF0 8004A3F0 00509348 */  mtc2       $s3, $10 /* handwritten instruction */
    /* 3ABF4 8004A3F4 00589448 */  mtc2       $s4, $11 /* handwritten instruction */
    /* 3ABF8 8004A3F8 00481048 */  mfc2       $s0, $9 /* handwritten instruction */
    /* 3ABFC 8004A3FC 00000000 */  nop
    /* 3AC00 8004A400 3D00984B */  gpf        1
    /* 3AC04 8004A404 18001402 */  mult       $s0, $s4
    /* 3AC08 8004A408 22781100 */  neg        $t7, $s1 /* handwritten instruction */
    /* 3AC0C 8004A40C 0C00AFA4 */  sh         $t7, 0xC($a1)
    /* 3AC10 8004A410 00480E48 */  mfc2       $t6, $9 /* handwritten instruction */
    /* 3AC14 8004A414 00500F48 */  mfc2       $t7, $10 /* handwritten instruction */
    /* 3AC18 8004A418 00581848 */  mfc2       $t8, $11 /* handwritten instruction */
    /* 3AC1C 8004A41C 00409248 */  mtc2       $s2, $8 /* handwritten instruction */
    /* 3AC20 8004A420 00489048 */  mtc2       $s0, $9 /* handwritten instruction */
    /* 3AC24 8004A424 00509348 */  mtc2       $s3, $10 /* handwritten instruction */
    /* 3AC28 8004A428 00589448 */  mtc2       $s4, $11 /* handwritten instruction */
    /* 3AC2C 8004A42C 0000B8A4 */  sh         $t8, 0x0($a1)
    /* 3AC30 8004A430 12C80000 */  mflo       $t9
    /* 3AC34 8004A434 3D00984B */  gpf        1
    /* 3AC38 8004A438 03CB1900 */  sra        $t9, $t9, 12
    /* 3AC3C 8004A43C 18007402 */  mult       $s3, $s4
    /* 3AC40 8004A440 00409148 */  mtc2       $s1, $8 /* handwritten instruction */
    /* 3AC44 8004A444 0E00B9A4 */  sh         $t9, 0xE($a1)
    /* 3AC48 8004A448 00481848 */  mfc2       $t8, $9 /* handwritten instruction */
    /* 3AC4C 8004A44C 00501948 */  mfc2       $t9, $10 /* handwritten instruction */
    /* 3AC50 8004A450 00580648 */  mfc2       $a2, $11 /* handwritten instruction */
    /* 3AC54 8004A454 00401148 */  mfc2       $s1, $8 /* handwritten instruction */
    /* 3AC58 8004A458 00C88F48 */  mtc2       $t7, $25 /* handwritten instruction */
    /* 3AC5C 8004A45C 22380E00 */  neg        $a3, $t6 /* handwritten instruction */
    /* 3AC60 8004A460 00D08748 */  mtc2       $a3, $26 /* handwritten instruction */
    /* 3AC64 8004A464 00D89848 */  mtc2       $t8, $27 /* handwritten instruction */
    /* 3AC68 8004A468 00588F48 */  mtc2       $t7, $11 /* handwritten instruction */
    /* 3AC6C 8004A46C 12380000 */  mflo       $a3
    /* 3AC70 8004A470 00000000 */  nop
    /* 3AC74 8004A474 3E00A84B */  gpl        1
    /* 3AC78 8004A478 0600A6A4 */  sh         $a2, 0x6($a1)
    /* 3AC7C 8004A47C 18002E02 */  mult       $s1, $t6
    /* 3AC80 8004A480 033B0700 */  sra        $a3, $a3, 12
    /* 3AC84 8004A484 1000A7A4 */  sh         $a3, 0x10($a1)
    /* 3AC88 8004A488 00480E48 */  mfc2       $t6, $9 /* handwritten instruction */
    /* 3AC8C 8004A48C 00500F48 */  mfc2       $t7, $10 /* handwritten instruction */
    /* 3AC90 8004A490 00581848 */  mfc2       $t8, $11 /* handwritten instruction */
    /* 3AC94 8004A494 0800AEA4 */  sh         $t6, 0x8($a1)
    /* 3AC98 8004A498 0A00AFA4 */  sh         $t7, 0xA($a1)
    /* 3AC9C 8004A49C 12300000 */  mflo       $a2
    /* 3ACA0 8004A4A0 0400B8A4 */  sh         $t8, 0x4($a1)
    /* 3ACA4 8004A4A4 03330600 */  sra        $a2, $a2, 12
    /* 3ACA8 8004A4A8 2230D900 */  sub        $a2, $a2, $t9 /* handwritten instruction */
    /* 3ACAC 8004A4AC 0200A6A4 */  sh         $a2, 0x2($a1)
    /* 3ACB0 8004A4B0 3400BF8F */  lw         $ra, 0x34($sp)
    /* 3ACB4 8004A4B4 3000BE8F */  lw         $fp, 0x30($sp)
    /* 3ACB8 8004A4B8 2C00B78F */  lw         $s7, 0x2C($sp)
    /* 3ACBC 8004A4BC 2800B68F */  lw         $s6, 0x28($sp)
    /* 3ACC0 8004A4C0 2400B58F */  lw         $s5, 0x24($sp)
    /* 3ACC4 8004A4C4 2000B48F */  lw         $s4, 0x20($sp)
    /* 3ACC8 8004A4C8 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 3ACCC 8004A4CC 1800B28F */  lw         $s2, 0x18($sp)
    /* 3ACD0 8004A4D0 1400B18F */  lw         $s1, 0x14($sp)
    /* 3ACD4 8004A4D4 1000B08F */  lw         $s0, 0x10($sp)
    /* 3ACD8 8004A4D8 0800E003 */  jr         $ra
    /* 3ACDC 8004A4DC 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004A348
