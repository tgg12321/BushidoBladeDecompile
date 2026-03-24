glabel func_8006D5D4
    /* 5DDD4 8006D5D4 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 5DDD8 8006D5D8 1800B0AF */  sw         $s0, 0x18($sp)
    /* 5DDDC 8006D5DC 2180A000 */  addu       $s0, $a1, $zero
    /* 5DDE0 8006D5E0 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 5DDE4 8006D5E4 21880000 */  addu       $s1, $zero, $zero
    /* 5DDE8 8006D5E8 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5DDEC 8006D5EC 21280000 */  addu       $a1, $zero, $zero
    /* 5DDF0 8006D5F0 FFFF0232 */  andi       $v0, $s0, 0xFFFF
    /* 5DDF4 8006D5F4 021C1000 */  srl        $v1, $s0, 16
    /* 5DDF8 8006D5F8 25104300 */  or         $v0, $v0, $v1
    /* 5DDFC 8006D5FC 3004868F */  lw         $a2, %gp_rel(D_800A34FC)($gp)
    /* 5DE00 8006D600 0A80073C */  lui        $a3, %hi(D_800A350C)
    /* 5DE04 8006D604 0C35E724 */  addiu      $a3, $a3, %lo(D_800A350C)
    /* 5DE08 8006D608 2000BFAF */  sw         $ra, 0x20($sp)
    /* 5DE0C 8006D60C 1000A2AF */  sw         $v0, 0x10($sp)
    /* 5DE10 8006D610 B0A4010C */  jal        func_800692C0
    /* 5DE14 8006D614 0C00C624 */   addiu     $a2, $a2, 0xC
    /* 5DE18 8006D618 031C0200 */  sra        $v1, $v0, 16
    /* 5DE1C 8006D61C 01000224 */  addiu      $v0, $zero, 0x1
    /* 5DE20 8006D620 06006214 */  bne        $v1, $v0, .L8006D63C
    /* 5DE24 8006D624 21200000 */   addu      $a0, $zero, $zero
    /* 5DE28 8006D628 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5DE2C 8006D62C 5C048297 */  lhu        $v0, %gp_rel(D_800A3528)($gp)
    /* 5DE30 8006D630 7F000624 */  addiu      $a2, $zero, 0x7F
    /* 5DE34 8006D634 95B50108 */  j          .L8006D654
    /* 5DE38 8006D638 01004224 */   addiu     $v0, $v0, 0x1
  .L8006D63C:
    /* 5DE3C 8006D63C 02000224 */  addiu      $v0, $zero, 0x2
    /* 5DE40 8006D640 07006214 */  bne        $v1, $v0, .L8006D660
    /* 5DE44 8006D644 7F000524 */   addiu     $a1, $zero, 0x7F
    /* 5DE48 8006D648 5C048297 */  lhu        $v0, %gp_rel(D_800A3528)($gp)
    /* 5DE4C 8006D64C 7F000624 */  addiu      $a2, $zero, 0x7F
    /* 5DE50 8006D650 FFFF4224 */  addiu      $v0, $v0, -0x1
  .L8006D654:
    /* 5DE54 8006D654 5C0482A7 */  sh         $v0, %gp_rel(D_800A3528)($gp)
    /* 5DE58 8006D658 9471010C */  jal        func_8005C650
    /* 5DE5C 8006D65C 00000000 */   nop
  .L8006D660:
    /* 5DE60 8006D660 5C048287 */  lh         $v0, %gp_rel(D_800A3528)($gp)
    /* 5DE64 8006D664 00000000 */  nop
    /* 5DE68 8006D668 04004104 */  bgez       $v0, .L8006D67C
    /* 5DE6C 8006D66C 5555023C */   lui       $v0, (0x55555556 >> 16)
    /* 5DE70 8006D670 02000224 */  addiu      $v0, $zero, 0x2
    /* 5DE74 8006D674 5C0482A7 */  sh         $v0, %gp_rel(D_800A3528)($gp)
    /* 5DE78 8006D678 5555023C */  lui        $v0, (0x55555556 >> 16)
  .L8006D67C:
    /* 5DE7C 8006D67C 5C048397 */  lhu        $v1, %gp_rel(D_800A3528)($gp)
    /* 5DE80 8006D680 56554234 */  ori        $v0, $v0, (0x55555556 & 0xFFFF)
    /* 5DE84 8006D684 001C0300 */  sll        $v1, $v1, 16
    /* 5DE88 8006D688 03240300 */  sra        $a0, $v1, 16
    /* 5DE8C 8006D68C 18008200 */  mult       $a0, $v0
    /* 5DE90 8006D690 1000053C */  lui        $a1, (0x100010 >> 16)
    /* 5DE94 8006D694 1000A534 */  ori        $a1, $a1, (0x100010 & 0xFFFF)
    /* 5DE98 8006D698 24280502 */  and        $a1, $s0, $a1
    /* 5DE9C 8006D69C C31F0300 */  sra        $v1, $v1, 31
    /* 5DEA0 8006D6A0 10400000 */  mfhi       $t0
    /* 5DEA4 8006D6A4 23180301 */  subu       $v1, $t0, $v1
    /* 5DEA8 8006D6A8 40100300 */  sll        $v0, $v1, 1
    /* 5DEAC 8006D6AC 21104300 */  addu       $v0, $v0, $v1
    /* 5DEB0 8006D6B0 23208200 */  subu       $a0, $a0, $v0
    /* 5DEB4 8006D6B4 5C0484A7 */  sh         $a0, %gp_rel(D_800A3528)($gp)
    /* 5DEB8 8006D6B8 0600A010 */  beqz       $a1, .L8006D6D4
    /* 5DEBC 8006D6BC 02000424 */   addiu     $a0, $zero, 0x2
    /* 5DEC0 8006D6C0 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5DEC4 8006D6C4 9471010C */  jal        func_8005C650
    /* 5DEC8 8006D6C8 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5DECC 8006D6CC CCB50108 */  j          .L8006D730
    /* 5DED0 8006D6D0 FFFF1124 */   addiu     $s1, $zero, -0x1
  .L8006D6D4:
    /* 5DED4 8006D6D4 4000023C */  lui        $v0, (0x400040 >> 16)
    /* 5DED8 8006D6D8 40004234 */  ori        $v0, $v0, (0x400040 & 0xFFFF)
    /* 5DEDC 8006D6DC 24100202 */  and        $v0, $s0, $v0
    /* 5DEE0 8006D6E0 13004010 */  beqz       $v0, .L8006D730
    /* 5DEE4 8006D6E4 01000424 */   addiu     $a0, $zero, 0x1
    /* 5DEE8 8006D6E8 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5DEEC 8006D6EC 9471010C */  jal        func_8005C650
    /* 5DEF0 8006D6F0 7F000624 */   addiu     $a2, $zero, 0x7F
    /* 5DEF4 8006D6F4 5C048587 */  lh         $a1, %gp_rel(D_800A3528)($gp)
    /* 5DEF8 8006D6F8 02000224 */  addiu      $v0, $zero, 0x2
    /* 5DEFC 8006D6FC 0300A214 */  bne        $a1, $v0, .L8006D70C
    /* 5DF00 8006D700 FDFF023C */   lui       $v0, (0xFFFDFFFF >> 16)
    /* 5DF04 8006D704 CCB50108 */  j          .L8006D730
    /* 5DF08 8006D708 FFFF1124 */   addiu     $s1, $zero, -0x1
  .L8006D70C:
    /* 5DF0C 8006D70C 5804848F */  lw         $a0, %gp_rel(D_800A3524)($gp)
    /* 5DF10 8006D710 FFFF4234 */  ori        $v0, $v0, (0xFFFDFFFF & 0xFFFF)
    /* 5DF14 8006D714 1400838C */  lw         $v1, 0x14($a0)
    /* 5DF18 8006D718 01001124 */  addiu      $s1, $zero, 0x1
    /* 5DF1C 8006D71C 24186200 */  and        $v1, $v1, $v0
    /* 5DF20 8006D720 0100A230 */  andi       $v0, $a1, 0x1
    /* 5DF24 8006D724 40140200 */  sll        $v0, $v0, 17
    /* 5DF28 8006D728 25186200 */  or         $v1, $v1, $v0
    /* 5DF2C 8006D72C 140083AC */  sw         $v1, 0x14($a0)
  .L8006D730:
    /* 5DF30 8006D730 21102002 */  addu       $v0, $s1, $zero
    /* 5DF34 8006D734 2000BF8F */  lw         $ra, 0x20($sp)
    /* 5DF38 8006D738 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 5DF3C 8006D73C 1800B08F */  lw         $s0, 0x18($sp)
    /* 5DF40 8006D740 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 5DF44 8006D744 0800E003 */  jr         $ra
    /* 5DF48 8006D748 00000000 */   nop
endlabel func_8006D5D4
