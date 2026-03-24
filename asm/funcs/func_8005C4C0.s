glabel func_8005C4C0
    /* 4CCC0 8005C4C0 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 4CCC4 8005C4C4 1400B1AF */  sw         $s1, 0x14($sp)
    /* 4CCC8 8005C4C8 21888000 */  addu       $s1, $a0, $zero
    /* 4CCCC 8005C4CC 1000B0AF */  sw         $s0, 0x10($sp)
    /* 4CCD0 8005C4D0 2180A000 */  addu       $s0, $a1, $zero
    /* 4CCD4 8005C4D4 21200000 */  addu       $a0, $zero, $zero
    /* 4CCD8 8005C4D8 2000BFAF */  sw         $ra, 0x20($sp)
    /* 4CCDC 8005C4DC 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 4CCE0 8005C4E0 3416020C */  jal        func_800858D0
    /* 4CCE4 8005C4E4 1800B2AF */   sw        $s2, 0x18($sp)
    /* 4CCE8 8005C4E8 00841000 */  sll        $s0, $s0, 16
    /* 4CCEC 8005C4EC 03941000 */  sra        $s2, $s0, 16
    /* 4CCF0 8005C4F0 0F80023C */  lui        $v0, %hi(D_800EFC38)
    /* 4CCF4 8005C4F4 38FC4224 */  addiu      $v0, $v0, %lo(D_800EFC38)
    /* 4CCF8 8005C4F8 80981200 */  sll        $s3, $s2, 2
    /* 4CCFC 8005C4FC 21806202 */  addu       $s0, $s3, $v0
    /* 4CD00 8005C500 0000038E */  lw         $v1, 0x0($s0)
    /* 4CD04 8005C504 00000000 */  nop
    /* 4CD08 8005C508 1C006010 */  beqz       $v1, .L8005C57C
    /* 4CD0C 8005C50C 21187100 */   addu      $v1, $v1, $s1
    /* 4CD10 8005C510 000003AE */  sw         $v1, 0x0($s0)
    /* 4CD14 8005C514 0000628C */  lw         $v0, 0x0($v1)
    /* 4CD18 8005C518 00000000 */  nop
    /* 4CD1C 8005C51C 21105100 */  addu       $v0, $v0, $s1
    /* 4CD20 8005C520 000062AC */  sw         $v0, 0x0($v1)
    /* 4CD24 8005C524 0000038E */  lw         $v1, 0x0($s0)
    /* 4CD28 8005C528 00000000 */  nop
    /* 4CD2C 8005C52C 0400628C */  lw         $v0, 0x4($v1)
    /* 4CD30 8005C530 21204002 */  addu       $a0, $s2, $zero
    /* 4CD34 8005C534 21105100 */  addu       $v0, $v0, $s1
    /* 4CD38 8005C538 D91F020C */  jal        func_80087F64
    /* 4CD3C 8005C53C 040062AC */   sw        $v0, 0x4($v1)
    /* 4CD40 8005C540 0000028E */  lw         $v0, 0x0($s0)
    /* 4CD44 8005C544 0F80013C */  lui        $at, %hi(D_800EFB38)
    /* 4CD48 8005C548 21083300 */  addu       $at, $at, $s3
    /* 4CD4C 8005C54C 38FB268C */  lw         $a2, %lo(D_800EFB38)($at)
    /* 4CD50 8005C550 0400448C */  lw         $a0, 0x4($v0)
    /* 4CD54 8005C554 2E20020C */  jal        func_800880B8
    /* 4CD58 8005C558 21284002 */   addu      $a1, $s2, $zero
    /* 4CD5C 8005C55C 00140200 */  sll        $v0, $v0, 16
    /* 4CD60 8005C560 03140200 */  sra        $v0, $v0, 16
    /* 4CD64 8005C564 08005214 */  bne        $v0, $s2, .L8005C588
    /* 4CD68 8005C568 00000000 */   nop
    /* 4CD6C 8005C56C FA1F020C */  jal        func_80087FE8
    /* 4CD70 8005C570 21204000 */   addu      $a0, $v0, $zero
    /* 4CD74 8005C574 61710108 */  j          .L8005C584
    /* 4CD78 8005C578 00140200 */   sll       $v0, $v0, 16
  .L8005C57C:
    /* 4CD7C 8005C57C FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 4CD80 8005C580 00140200 */  sll        $v0, $v0, 16
  .L8005C584:
    /* 4CD84 8005C584 03140200 */  sra        $v0, $v0, 16
  .L8005C588:
    /* 4CD88 8005C588 2000BF8F */  lw         $ra, 0x20($sp)
    /* 4CD8C 8005C58C 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 4CD90 8005C590 1800B28F */  lw         $s2, 0x18($sp)
    /* 4CD94 8005C594 1400B18F */  lw         $s1, 0x14($sp)
    /* 4CD98 8005C598 1000B08F */  lw         $s0, 0x10($sp)
    /* 4CD9C 8005C59C 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 4CDA0 8005C5A0 0800E003 */  jr         $ra
    /* 4CDA4 8005C5A4 00000000 */   nop
endlabel func_8005C4C0
