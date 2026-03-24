glabel func_80054440
    /* 44C40 80054440 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 44C44 80054444 2800BFAF */  sw         $ra, 0x28($sp)
    /* 44C48 80054448 2400BEAF */  sw         $fp, 0x24($sp)
    /* 44C4C 8005444C 2000B5AF */  sw         $s5, 0x20($sp)
    /* 44C50 80054450 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 44C54 80054454 1800B3AF */  sw         $s3, 0x18($sp)
    /* 44C58 80054458 1400B2AF */  sw         $s2, 0x14($sp)
    /* 44C5C 8005445C 1000B1AF */  sw         $s1, 0x10($sp)
    /* 44C60 80054460 21888000 */  addu       $s1, $a0, $zero
    /* 44C64 80054464 2190A000 */  addu       $s2, $a1, $zero
    /* 44C68 80054468 0A80143C */  lui        $s4, %hi(D_800A3828)
    /* 44C6C 8005446C 2838948E */  lw         $s4, %lo(D_800A3828)($s4)
    /* 44C70 80054470 00000000 */  nop
    /* 44C74 80054474 18009422 */  addi       $s4, $s4, 0x18 /* handwritten instruction */
    /* 44C78 80054478 801F153C */  lui        $s5, (0x1F800030 >> 16)
    /* 44C7C 8005447C 00308048 */  mtc2       $zero, $6 /* handwritten instruction */
    /* 44C80 80054480 0F80133C */  lui        $s3, %hi(D_800F2B70)
    /* 44C84 80054484 702B7326 */  addiu      $s3, $s3, %lo(D_800F2B70)
    /* 44C88 80054488 4B000104 */  bgez       $zero, .L800545B8
    /* 44C8C 8005448C 00000000 */   nop
  .L80054490:
    /* 44C90 80054490 2000A622 */  addi       $a2, $s5, (0x1F800020 & 0xFFFF) /* handwritten instruction */
    /* 44C94 80054494 80400200 */  sll        $t0, $v0, 2
    /* 44C98 80054498 20284802 */  add        $a1, $s2, $t0 /* handwritten instruction */
    /* 44C9C 8005449C 0000A58C */  lw         $a1, 0x0($a1)
    /* 44CA0 800544A0 4C4A010C */  jal        func_80052930
    /* 44CA4 800544A4 21208002 */   addu      $a0, $s4, $zero
    /* 44CA8 800544A8 2000A88E */  lw         $t0, (0x1F800020 & 0xFFFF)($s5)
    /* 44CAC 800544AC 2400A98E */  lw         $t1, (0x1F800024 & 0xFFFF)($s5)
    /* 44CB0 800544B0 2800AA8E */  lw         $t2, (0x1F800028 & 0xFFFF)($s5)
    /* 44CB4 800544B4 2C00AB8E */  lw         $t3, (0x1F80002C & 0xFFFF)($s5)
    /* 44CB8 800544B8 3000AC8E */  lw         $t4, (0x1F800030 & 0xFFFF)($s5)
    /* 44CBC 800544BC 0040C848 */  ctc2       $t0, $8 /* handwritten instruction */
    /* 44CC0 800544C0 0048C948 */  ctc2       $t1, $9 /* handwritten instruction */
    /* 44CC4 800544C4 0050CA48 */  ctc2       $t2, $10 /* handwritten instruction */
    /* 44CC8 800544C8 0058CB48 */  ctc2       $t3, $11 /* handwritten instruction */
    /* 44CCC 800544CC 0060CC48 */  ctc2       $t4, $12 /* handwritten instruction */
    /* 44CD0 800544D0 00002286 */  lh         $v0, 0x0($s1)
    /* 44CD4 800544D4 02003122 */  addi       $s1, $s1, 0x2 /* handwritten instruction */
    /* 44CD8 800544D8 00002896 */  lhu        $t0, 0x0($s1)
    /* 44CDC 800544DC 02002A86 */  lh         $t2, 0x2($s1)
    /* 44CE0 800544E0 04002986 */  lh         $t1, 0x4($s1)
    /* 44CE4 800544E4 00540A00 */  sll        $t2, $t2, 16
    /* 44CE8 800544E8 25400A01 */  or         $t0, $t0, $t2
    /* 44CEC 800544EC 06002A96 */  lhu        $t2, 0x6($s1)
    /* 44CF0 800544F0 08002C86 */  lh         $t4, 0x8($s1)
    /* 44CF4 800544F4 0A002B86 */  lh         $t3, 0xA($s1)
    /* 44CF8 800544F8 00640C00 */  sll        $t4, $t4, 16
    /* 44CFC 800544FC 25504C01 */  or         $t2, $t2, $t4
    /* 44D00 80054500 0C002C96 */  lhu        $t4, 0xC($s1)
    /* 44D04 80054504 0E002E86 */  lh         $t6, 0xE($s1)
    /* 44D08 80054508 10002D86 */  lh         $t5, 0x10($s1)
    /* 44D0C 8005450C 00740E00 */  sll        $t6, $t6, 16
    /* 44D10 80054510 25608E01 */  or         $t4, $t4, $t6
    /* 44D14 80054514 14000104 */  bgez       $zero, .L80054568
    /* 44D18 80054518 00000000 */   nop
  .L8005451C:
    /* 44D1C 8005451C 00002896 */  lhu        $t0, 0x0($s1)
    /* 44D20 80054520 02002A86 */  lh         $t2, 0x2($s1)
    /* 44D24 80054524 04002986 */  lh         $t1, 0x4($s1)
    /* 44D28 80054528 00540A00 */  sll        $t2, $t2, 16
    /* 44D2C 8005452C 25400A01 */  or         $t0, $t0, $t2
    /* 44D30 80054530 06002A96 */  lhu        $t2, 0x6($s1)
    /* 44D34 80054534 08002C86 */  lh         $t4, 0x8($s1)
    /* 44D38 80054538 0A002B86 */  lh         $t3, 0xA($s1)
    /* 44D3C 8005453C 00640C00 */  sll        $t4, $t4, 16
    /* 44D40 80054540 25504C01 */  or         $t2, $t2, $t4
    /* 44D44 80054544 0C002C96 */  lhu        $t4, 0xC($s1)
    /* 44D48 80054548 0E002E86 */  lh         $t6, 0xE($s1)
    /* 44D4C 8005454C 10002D86 */  lh         $t5, 0x10($s1)
    /* 44D50 80054550 00740E00 */  sll        $t6, $t6, 16
    /* 44D54 80054554 25608E01 */  or         $t4, $t4, $t6
    /* 44D58 80054558 000074EA */  swc2       $20, 0x0($s3)
    /* 44D5C 8005455C 040075EA */  swc2       $21, 0x4($s3)
    /* 44D60 80054560 080076EA */  swc2       $22, 0x8($s3)
    /* 44D64 80054564 0C007322 */  addi       $s3, $s3, 0xC /* handwritten instruction */
  .L80054568:
    /* 44D68 80054568 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 44D6C 8005456C 00088948 */  mtc2       $t1, $1 /* handwritten instruction */
    /* 44D70 80054570 00108A48 */  mtc2       $t2, $2 /* handwritten instruction */
    /* 44D74 80054574 00188B48 */  mtc2       $t3, $3 /* handwritten instruction */
    /* 44D78 80054578 00208C48 */  mtc2       $t4, $4 /* handwritten instruction */
    /* 44D7C 8005457C 00288D48 */  mtc2       $t5, $5 /* handwritten instruction */
    /* 44D80 80054580 FDFF4220 */  addi       $v0, $v0, -0x3 /* handwritten instruction */
    /* 44D84 80054584 00000000 */  nop
    /* 44D88 80054588 2004D84A */  nct
    /* 44D8C 8005458C E3FF401C */  bgtz       $v0, .L8005451C
    /* 44D90 80054590 12003122 */   addi      $s1, $s1, 0x12 /* handwritten instruction */
    /* 44D94 80054594 000074EA */  swc2       $20, 0x0($s3)
    /* 44D98 80054598 040075EA */  swc2       $21, 0x4($s3)
    /* 44D9C 8005459C 080076EA */  swc2       $22, 0x8($s3)
    /* 44DA0 800545A0 0C007322 */  addi       $s3, $s3, 0xC /* handwritten instruction */
    /* 44DA4 800545A4 20104200 */  add        $v0, $v0, $v0 /* handwritten instruction */
    /* 44DA8 800545A8 20404200 */  add        $t0, $v0, $v0 /* handwritten instruction */
    /* 44DAC 800545AC 20986802 */  add        $s3, $s3, $t0 /* handwritten instruction */
    /* 44DB0 800545B0 20400201 */  add        $t0, $t0, $v0 /* handwritten instruction */
    /* 44DB4 800545B4 20882802 */  add        $s1, $s1, $t0 /* handwritten instruction */
  .L800545B8:
    /* 44DB8 800545B8 00002286 */  lh         $v0, 0x0($s1)
    /* 44DBC 800545BC 00000000 */  nop
    /* 44DC0 800545C0 01004820 */  addi       $t0, $v0, 0x1 /* handwritten instruction */
    /* 44DC4 800545C4 B2FF0015 */  bnez       $t0, .L80054490
    /* 44DC8 800545C8 02003122 */   addi      $s1, $s1, 0x2 /* handwritten instruction */
    /* 44DCC 800545CC 21102002 */  addu       $v0, $s1, $zero
    /* 44DD0 800545D0 2800BF8F */  lw         $ra, 0x28($sp)
    /* 44DD4 800545D4 2400BE8F */  lw         $fp, 0x24($sp)
    /* 44DD8 800545D8 2000B58F */  lw         $s5, 0x20($sp)
    /* 44DDC 800545DC 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 44DE0 800545E0 1800B38F */  lw         $s3, 0x18($sp)
    /* 44DE4 800545E4 1400B28F */  lw         $s2, 0x14($sp)
    /* 44DE8 800545E8 1000B18F */  lw         $s1, 0x10($sp)
    /* 44DEC 800545EC 0800E003 */  jr         $ra
    /* 44DF0 800545F0 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_80054440
