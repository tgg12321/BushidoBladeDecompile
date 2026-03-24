glabel func_8004D424
    /* 3DC24 8004D424 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3DC28 8004D428 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 3DC2C 8004D42C 2800BEAF */  sw         $fp, 0x28($sp)
    /* 3DC30 8004D430 2400B6AF */  sw         $s6, 0x24($sp)
    /* 3DC34 8004D434 2000B5AF */  sw         $s5, 0x20($sp)
    /* 3DC38 8004D438 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 3DC3C 8004D43C 1800B3AF */  sw         $s3, 0x18($sp)
    /* 3DC40 8004D440 1400B2AF */  sw         $s2, 0x14($sp)
    /* 3DC44 8004D444 1000B1AF */  sw         $s1, 0x10($sp)
    /* 3DC48 8004D448 6E008010 */  beqz       $a0, .L8004D604
    /* 3DC4C 8004D44C FFFF9324 */   addiu     $s3, $a0, -0x1
    /* 3DC50 8004D450 801F113C */  lui        $s1, (0x1F800020 >> 16)
    /* 3DC54 8004D454 20003136 */  ori        $s1, $s1, (0x1F800020 & 0xFFFF)
    /* 3DC58 8004D458 801F123C */  lui        $s2, (0x1F8002B4 >> 16)
    /* 3DC5C 8004D45C B4025236 */  ori        $s2, $s2, (0x1F8002B4 & 0xFFFF)
    /* 3DC60 8004D460 801F143C */  lui        $s4, (0x1F80000C >> 16)
    /* 3DC64 8004D464 0C00948E */  lw         $s4, (0x1F80000C & 0xFFFF)($s4)
    /* 3DC68 8004D468 0009153C */  lui        $s5, (0x9000000 >> 16)
    /* 3DC6C 8004D46C FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3DC70 8004D470 FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3DC74 8004D474 0A80163C */  lui        $s6, %hi(D_800A38B4)
    /* 3DC78 8004D478 B438D68E */  lw         $s6, %lo(D_800A38B4)($s6)
    /* 3DC7C 8004D47C 801F033C */  lui        $v1, (0x1F800008 >> 16)
    /* 3DC80 8004D480 0800638C */  lw         $v1, (0x1F800008 & 0xFFFF)($v1)
  .L8004D484:
    /* 3DC84 8004D484 0E000496 */  lhu        $a0, 0xE($s0)
    /* 3DC88 8004D488 10000596 */  lhu        $a1, 0x10($s0)
    /* 3DC8C 8004D48C 000006CA */  lwc2       $6, 0x0($s0)
    /* 3DC90 8004D490 002C0500 */  sll        $a1, $a1, 16
    /* 3DC94 8004D494 25208500 */  or         $a0, $a0, $a1
    /* 3DC98 8004D498 80280400 */  sll        $a1, $a0, 2
    /* 3DC9C 8004D49C FC03AB30 */  andi       $t3, $a1, 0x3FC
    /* 3DCA0 8004D4A0 21287101 */  addu       $a1, $t3, $s1
    /* 3DCA4 8004D4A4 0000A88C */  lw         $t0, 0x0($a1)
    /* 3DCA8 8004D4A8 82290400 */  srl        $a1, $a0, 6
    /* 3DCAC 8004D4AC FC03AC30 */  andi       $t4, $a1, 0x3FC
    /* 3DCB0 8004D4B0 21289101 */  addu       $a1, $t4, $s1
    /* 3DCB4 8004D4B4 0000A98C */  lw         $t1, 0x0($a1)
    /* 3DCB8 8004D4B8 822B0400 */  srl        $a1, $a0, 14
    /* 3DCBC 8004D4BC FC03AD30 */  andi       $t5, $a1, 0x3FC
    /* 3DCC0 8004D4C0 2128B101 */  addu       $a1, $t5, $s1
    /* 3DCC4 8004D4C4 0000AA8C */  lw         $t2, 0x0($a1)
    /* 3DCC8 8004D4C8 822D0400 */  srl        $a1, $a0, 22
    /* 3DCCC 8004D4CC FC03AE30 */  andi       $t6, $a1, 0x3FC
    /* 3DCD0 8004D4D0 2128D101 */  addu       $a1, $t6, $s1
    /* 3DCD4 8004D4D4 0000AF8C */  lw         $t7, 0x0($a1)
    /* 3DCD8 8004D4D8 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3DCDC 8004D4DC 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3DCE0 8004D4E0 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3DCE4 8004D4E4 24C00901 */  and        $t8, $t0, $t1
    /* 3DCE8 8004D4E8 24C00A03 */  and        $t8, $t8, $t2
    /* 3DCEC 8004D4EC 24C00F03 */  and        $t8, $t8, $t7
    /* 3DCF0 8004D4F0 0600404B */  nclip
    /* 3DCF4 8004D4F4 40000007 */  bltz       $t8, .L8004D5F8
    /* 3DCF8 8004D4F8 00C00248 */   mfc2      $v0, $24 /* handwritten instruction */
    /* 3DCFC 8004D4FC 00801833 */  andi       $t8, $t8, 0x8000
    /* 3DD00 8004D500 3D004104 */  bgez       $v0, .L8004D5F8
    /* 3DD04 8004D504 00000000 */   nop
    /* 3DD08 8004D508 3B000017 */  bnez       $t8, .L8004D5F8
    /* 3DD0C 8004D50C 42100B00 */   srl       $v0, $t3, 1
    /* 3DD10 8004D510 21105200 */  addu       $v0, $v0, $s2
    /* 3DD14 8004D514 00004484 */  lh         $a0, 0x0($v0)
    /* 3DD18 8004D518 42100C00 */  srl        $v0, $t4, 1
    /* 3DD1C 8004D51C 21105200 */  addu       $v0, $v0, $s2
    /* 3DD20 8004D520 00004584 */  lh         $a1, 0x0($v0)
    /* 3DD24 8004D524 42100D00 */  srl        $v0, $t5, 1
    /* 3DD28 8004D528 21105200 */  addu       $v0, $v0, $s2
    /* 3DD2C 8004D52C 00004684 */  lh         $a2, 0x0($v0)
    /* 3DD30 8004D530 42100E00 */  srl        $v0, $t6, 1
    /* 3DD34 8004D534 21105200 */  addu       $v0, $v0, $s2
    /* 3DD38 8004D538 00004784 */  lh         $a3, 0x0($v0)
    /* 3DD3C 8004D53C 20208500 */  add        $a0, $a0, $a1 /* handwritten instruction */
    /* 3DD40 8004D540 20208600 */  add        $a0, $a0, $a2 /* handwritten instruction */
    /* 3DD44 8004D544 20208700 */  add        $a0, $a0, $a3 /* handwritten instruction */
    /* 3DD48 8004D548 83100400 */  sra        $v0, $a0, 2
    /* 3DD4C 8004D54C 2A004018 */  blez       $v0, .L8004D5F8
    /* 3DD50 8004D550 00000000 */   nop
    /* 3DD54 8004D554 0800C8AE */  sw         $t0, 0x8($s6)
    /* 3DD58 8004D558 1000C9AE */  sw         $t1, 0x10($s6)
    /* 3DD5C 8004D55C 1800CAAE */  sw         $t2, 0x18($s6)
    /* 3DD60 8004D560 2000CFAE */  sw         $t7, 0x20($s6)
    /* 3DD64 8004D564 12000896 */  lhu        $t0, 0x12($s0)
    /* 3DD68 8004D568 140000CA */  lwc2       $0, 0x14($s0)
    /* 3DD6C 8004D56C 00088848 */  mtc2       $t0, $1 /* handwritten instruction */
    /* 3DD70 8004D570 06106200 */  srlv       $v0, $v0, $v1
    /* 3DD74 8004D574 C2420200 */  srl        $t0, $v0, 11
    /* 3DD78 8004D578 FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3DD7C 8004D57C 1E04C84A */  ncs
    /* 3DD80 8004D580 801F043C */  lui        $a0, (0x1F800018 >> 16)
    /* 3DD84 8004D584 1800848C */  lw         $a0, (0x1F800018 & 0xFFFF)($a0)
    /* 3DD88 8004D588 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3DD8C 8004D58C 07100201 */  srav       $v0, $v0, $t0
    /* 3DD90 8004D590 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3DD94 8004D594 2A084400 */  slt        $at, $v0, $a0
    /* 3DD98 8004D598 17002014 */  bnez       $at, .L8004D5F8
    /* 3DD9C 8004D59C 04000C86 */   lh        $t4, 0x4($s0)
    /* 3DDA0 8004D5A0 00000D86 */  lh         $t5, 0x0($s0)
    /* 3DDA4 8004D5A4 00640C00 */  sll        $t4, $t4, 16
    /* 3DDA8 8004D5A8 006C0D00 */  sll        $t5, $t5, 16
    /* 3DDAC 8004D5AC 06000896 */  lhu        $t0, 0x6($s0)
    /* 3DDB0 8004D5B0 08000996 */  lhu        $t1, 0x8($s0)
    /* 3DDB4 8004D5B4 0A000A96 */  lhu        $t2, 0xA($s0)
    /* 3DDB8 8004D5B8 0C000B96 */  lhu        $t3, 0xC($s0)
    /* 3DDBC 8004D5BC 25400C01 */  or         $t0, $t0, $t4
    /* 3DDC0 8004D5C0 25482D01 */  or         $t1, $t1, $t5
    /* 3DDC4 8004D5C4 0C00C8AE */  sw         $t0, 0xC($s6)
    /* 3DDC8 8004D5C8 1400C9AE */  sw         $t1, 0x14($s6)
    /* 3DDCC 8004D5CC 1C00CAA6 */  sh         $t2, 0x1C($s6)
    /* 3DDD0 8004D5D0 2400CBA6 */  sh         $t3, 0x24($s6)
    /* 3DDD4 8004D5D4 0400D6EA */  swc2       $22, 0x4($s6)
    /* 3DDD8 8004D5D8 80100200 */  sll        $v0, $v0, 2
    /* 3DDDC 8004D5DC 21105400 */  addu       $v0, $v0, $s4
    /* 3DDE0 8004D5E0 0000498C */  lw         $t1, 0x0($v0)
    /* 3DDE4 8004D5E4 2440DE02 */  and        $t0, $s6, $fp
    /* 3DDE8 8004D5E8 000048AC */  sw         $t0, 0x0($v0)
    /* 3DDEC 8004D5EC 25483501 */  or         $t1, $t1, $s5
    /* 3DDF0 8004D5F0 0000C9AE */  sw         $t1, 0x0($s6)
    /* 3DDF4 8004D5F4 2800D626 */  addiu      $s6, $s6, 0x28
  .L8004D5F8:
    /* 3DDF8 8004D5F8 18001026 */  addiu      $s0, $s0, 0x18
    /* 3DDFC 8004D5FC A1FF6016 */  bnez       $s3, .L8004D484
    /* 3DE00 8004D600 FFFF7326 */   addiu     $s3, $s3, -0x1
  .L8004D604:
    /* 3DE04 8004D604 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 3DE08 8004D608 B43836AC */  sw         $s6, %lo(D_800A38B4)($at)
    /* 3DE0C 8004D60C 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 3DE10 8004D610 2800BE8F */  lw         $fp, 0x28($sp)
    /* 3DE14 8004D614 2400B68F */  lw         $s6, 0x24($sp)
    /* 3DE18 8004D618 2000B58F */  lw         $s5, 0x20($sp)
    /* 3DE1C 8004D61C 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 3DE20 8004D620 1800B38F */  lw         $s3, 0x18($sp)
    /* 3DE24 8004D624 1400B28F */  lw         $s2, 0x14($sp)
    /* 3DE28 8004D628 1000B18F */  lw         $s1, 0x10($sp)
    /* 3DE2C 8004D62C 0800E003 */  jr         $ra
    /* 3DE30 8004D630 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004D424
