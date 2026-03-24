glabel func_8003FE40
    /* 30640 8003FE40 F8FFBD27 */  addiu      $sp, $sp, -0x8
    /* 30644 8003FE44 0800A018 */  blez       $a1, .L8003FE68
    /* 30648 8003FE48 21380000 */   addu      $a3, $zero, $zero
    /* 3064C 8003FE4C 00FF0824 */  addiu      $t0, $zero, -0x100
    /* 30650 8003FE50 21188000 */  addu       $v1, $a0, $zero
  .L8003FE54:
    /* 30654 8003FE54 060068A4 */  sh         $t0, 0x6($v1)
    /* 30658 8003FE58 0100E724 */  addiu      $a3, $a3, 0x1
    /* 3065C 8003FE5C 2A10E500 */  slt        $v0, $a3, $a1
    /* 30660 8003FE60 FCFF4014 */  bnez       $v0, .L8003FE54
    /* 30664 8003FE64 08006324 */   addiu     $v1, $v1, 0x8
  .L8003FE68:
    /* 30668 8003FE68 0000C384 */  lh         $v1, 0x0($a2)
    /* 3066C 8003FE6C 00000000 */  nop
    /* 30670 8003FE70 12006004 */  bltz       $v1, .L8003FEBC
    /* 30674 8003FE74 0200C624 */   addiu     $a2, $a2, 0x2
    /* 30678 8003FE78 FFFF0924 */  addiu      $t1, $zero, -0x1
  .L8003FE7C:
    /* 3067C 8003FE7C 0000C584 */  lh         $a1, 0x0($a2)
    /* 30680 8003FE80 FFFF6324 */  addiu      $v1, $v1, -0x1
    /* 30684 8003FE84 09006910 */  beq        $v1, $t1, .L8003FEAC
    /* 30688 8003FE88 0200C624 */   addiu     $a2, $a2, 0x2
    /* 3068C 8003FE8C FFFF0824 */  addiu      $t0, $zero, -0x1
  .L8003FE90:
    /* 30690 8003FE90 0000C784 */  lh         $a3, 0x0($a2)
    /* 30694 8003FE94 0200C624 */  addiu      $a2, $a2, 0x2
    /* 30698 8003FE98 FFFF6324 */  addiu      $v1, $v1, -0x1
    /* 3069C 8003FE9C C0100700 */  sll        $v0, $a3, 3
    /* 306A0 8003FEA0 21104400 */  addu       $v0, $v0, $a0
    /* 306A4 8003FEA4 FAFF6814 */  bne        $v1, $t0, .L8003FE90
    /* 306A8 8003FEA8 060045A4 */   sh        $a1, 0x6($v0)
  .L8003FEAC:
    /* 306AC 8003FEAC 0000C384 */  lh         $v1, 0x0($a2)
    /* 306B0 8003FEB0 00000000 */  nop
    /* 306B4 8003FEB4 F1FF6104 */  bgez       $v1, .L8003FE7C
    /* 306B8 8003FEB8 0200C624 */   addiu     $a2, $a2, 0x2
  .L8003FEBC:
    /* 306BC 8003FEBC 2110C000 */  addu       $v0, $a2, $zero
    /* 306C0 8003FEC0 0800BD27 */  addiu      $sp, $sp, 0x8
    /* 306C4 8003FEC4 0800E003 */  jr         $ra
    /* 306C8 8003FEC8 00000000 */   nop
endlabel func_8003FE40
