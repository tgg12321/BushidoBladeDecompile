glabel func_8002BC68
    /* 1C468 8002BC68 1080033C */  lui        $v1, %hi(D_80101FA0)
    /* 1C46C 8002BC6C A01F638C */  lw         $v1, %lo(D_80101FA0)($v1)
    /* 1C470 8002BC70 1080023C */  lui        $v0, %hi(D_801023EC)
    /* 1C474 8002BC74 EC23428C */  lw         $v0, %lo(D_801023EC)($v0)
    /* 1C478 8002BC78 00000000 */  nop
    /* 1C47C 8002BC7C 23386200 */  subu       $a3, $v1, $v0
    /* 1C480 8002BC80 1800E700 */  mult       $a3, $a3
    /* 1C484 8002BC84 21288000 */  addu       $a1, $a0, $zero
    /* 1C488 8002BC88 1080033C */  lui        $v1, %hi(D_80101FA8)
    /* 1C48C 8002BC8C A81F638C */  lw         $v1, %lo(D_80101FA8)($v1)
    /* 1C490 8002BC90 1080023C */  lui        $v0, %hi(D_801023F4)
    /* 1C494 8002BC94 F423428C */  lw         $v0, %lo(D_801023F4)($v0)
    /* 1C498 8002BC98 12200000 */  mflo       $a0
    /* 1C49C 8002BC9C 23486200 */  subu       $t1, $v1, $v0
    /* 1C4A0 8002BCA0 00000000 */  nop
    /* 1C4A4 8002BCA4 18002901 */  mult       $t1, $t1
    /* 1C4A8 8002BCA8 F8FFBD27 */  addiu      $sp, $sp, -0x8
    /* 1C4AC 8002BCAC 10800A3C */  lui        $t2, %hi(D_80101EC8)
    /* 1C4B0 8002BCB0 C81E4A25 */  addiu      $t2, $t2, %lo(D_80101EC8)
    /* 1C4B4 8002BCB4 12180000 */  mflo       $v1
    /* 1C4B8 8002BCB8 21208300 */  addu       $a0, $a0, $v1
    /* 1C4BC 8002BCBC 0004822C */  sltiu      $v0, $a0, 0x400
    /* 1C4C0 8002BCC0 06004010 */  beqz       $v0, .L8002BCDC
    /* 1C4C4 8002BCC4 4C044B25 */   addiu     $t3, $t2, 0x44C
    /* 1C4C8 8002BCC8 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1C4CC 8002BCCC 21082400 */  addu       $at, $at, $a0
    /* 1C4D0 8002BCD0 18D12290 */  lbu        $v0, %lo(D_8008D118)($at)
    /* 1C4D4 8002BCD4 4BAF0008 */  j          .L8002BD2C
    /* 1C4D8 8002BCD8 C2400200 */   srl       $t0, $v0, 3
  .L8002BCDC:
    /* 1C4DC 8002BCDC 21608000 */  addu       $t4, $a0, $zero
    /* 1C4E0 8002BCE0 00F08C48 */  mtc2       $t4, $30 /* handwritten instruction */
    /* 1C4E4 8002BCE4 00000000 */  nop
    /* 1C4E8 8002BCE8 00000000 */  nop
    /* 1C4EC 8002BCEC 2160A003 */  addu       $t4, $sp, $zero
    /* 1C4F0 8002BCF0 00009FE9 */  swc2       $31, 0x0($t4)
    /* 1C4F4 8002BCF4 0000A38F */  lw         $v1, 0x0($sp)
    /* 1C4F8 8002BCF8 FEFF0224 */  addiu      $v0, $zero, -0x2
    /* 1C4FC 8002BCFC 24106200 */  and        $v0, $v1, $v0
    /* 1C500 8002BD00 16000324 */  addiu      $v1, $zero, 0x16
    /* 1C504 8002BD04 23186200 */  subu       $v1, $v1, $v0
    /* 1C508 8002BD08 06106400 */  srlv       $v0, $a0, $v1
    /* 1C50C 8002BD0C 42180300 */  srl        $v1, $v1, 1
    /* 1C510 8002BD10 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1C514 8002BD14 21082200 */  addu       $at, $at, $v0
    /* 1C518 8002BD18 18D12490 */  lbu        $a0, %lo(D_8008D118)($at)
    /* 1C51C 8002BD1C 13000224 */  addiu      $v0, $zero, 0x13
    /* 1C520 8002BD20 23104300 */  subu       $v0, $v0, $v1
    /* 1C524 8002BD24 00240400 */  sll        $a0, $a0, 16
    /* 1C528 8002BD28 06404400 */  srlv       $t0, $a0, $v0
  .L8002BD2C:
    /* 1C52C 8002BD2C 2A100501 */  slt        $v0, $t0, $a1
    /* 1C530 8002BD30 0C004010 */  beqz       $v0, .L8002BD64
    /* 1C534 8002BD34 EB51043C */   lui       $a0, (0x51EB851F >> 16)
    /* 1C538 8002BD38 1F858434 */  ori        $a0, $a0, (0x51EB851F & 0xFFFF)
    /* 1C53C 8002BD3C 2318A800 */  subu       $v1, $a1, $t0
    /* 1C540 8002BD40 80100300 */  sll        $v0, $v1, 2
    /* 1C544 8002BD44 21104300 */  addu       $v0, $v0, $v1
    /* 1C548 8002BD48 00110200 */  sll        $v0, $v0, 4
    /* 1C54C 8002BD4C 18004400 */  mult       $v0, $a0
    /* 1C550 8002BD50 C3170200 */  sra        $v0, $v0, 31
    /* 1C554 8002BD54 10C00000 */  mfhi       $t8
    /* 1C558 8002BD58 43191800 */  sra        $v1, $t8, 5
    /* 1C55C 8002BD5C 5EAF0008 */  j          .L8002BD78
    /* 1C560 8002BD60 23206200 */   subu      $a0, $v1, $v0
  .L8002BD64:
    /* 1C564 8002BD64 2318A800 */  subu       $v1, $a1, $t0
    /* 1C568 8002BD68 03006104 */  bgez       $v1, .L8002BD78
    /* 1C56C 8002BD6C 03210300 */   sra       $a0, $v1, 4
    /* 1C570 8002BD70 0F006324 */  addiu      $v1, $v1, 0xF
    /* 1C574 8002BD74 03210300 */  sra        $a0, $v1, 4
  .L8002BD78:
    /* 1C578 8002BD78 1800E400 */  mult       $a3, $a0
    /* 1C57C 8002BD7C 12180000 */  mflo       $v1
    /* 1C580 8002BD80 9CFFA224 */  addiu      $v0, $a1, -0x64
    /* 1C584 8002BD84 00000000 */  nop
    /* 1C588 8002BD88 1A006200 */  div        $zero, $v1, $v0
    /* 1C58C 8002BD8C 02004014 */  bnez       $v0, .L8002BD98
    /* 1C590 8002BD90 00000000 */   nop
    /* 1C594 8002BD94 0D000700 */  break      7
  .L8002BD98:
    /* 1C598 8002BD98 FFFF0124 */  addiu      $at, $zero, -0x1
    /* 1C59C 8002BD9C 04004114 */  bne        $v0, $at, .L8002BDB0
    /* 1C5A0 8002BDA0 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 1C5A4 8002BDA4 02006114 */  bne        $v1, $at, .L8002BDB0
    /* 1C5A8 8002BDA8 00000000 */   nop
    /* 1C5AC 8002BDAC 0D000600 */  break      6
  .L8002BDB0:
    /* 1C5B0 8002BDB0 12300000 */  mflo       $a2
    /* 1C5B4 8002BDB4 00000000 */  nop
    /* 1C5B8 8002BDB8 00000000 */  nop
    /* 1C5BC 8002BDBC 18002401 */  mult       $t1, $a0
    /* 1C5C0 8002BDC0 12180000 */  mflo       $v1
    /* 1C5C4 8002BDC4 00000000 */  nop
    /* 1C5C8 8002BDC8 00000000 */  nop
    /* 1C5CC 8002BDCC 1A006200 */  div        $zero, $v1, $v0
    /* 1C5D0 8002BDD0 02004014 */  bnez       $v0, .L8002BDDC
    /* 1C5D4 8002BDD4 00000000 */   nop
    /* 1C5D8 8002BDD8 0D000700 */  break      7
  .L8002BDDC:
    /* 1C5DC 8002BDDC FFFF0124 */  addiu      $at, $zero, -0x1
    /* 1C5E0 8002BDE0 04004114 */  bne        $v0, $at, .L8002BDF4
    /* 1C5E4 8002BDE4 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 1C5E8 8002BDE8 02006114 */  bne        $v1, $at, .L8002BDF4
    /* 1C5EC 8002BDEC 00000000 */   nop
    /* 1C5F0 8002BDF0 0D000600 */  break      6
  .L8002BDF4:
    /* 1C5F4 8002BDF4 12280000 */  mflo       $a1
    /* 1C5F8 8002BDF8 23180400 */  negu       $v1, $a0
    /* 1C5FC 8002BDFC 00000000 */  nop
    /* 1C600 8002BE00 1800E300 */  mult       $a3, $v1
    /* 1C604 8002BE04 12200000 */  mflo       $a0
    /* 1C608 8002BE08 00000000 */  nop
    /* 1C60C 8002BE0C 00000000 */  nop
    /* 1C610 8002BE10 1A008200 */  div        $zero, $a0, $v0
    /* 1C614 8002BE14 02004014 */  bnez       $v0, .L8002BE20
    /* 1C618 8002BE18 00000000 */   nop
    /* 1C61C 8002BE1C 0D000700 */  break      7
  .L8002BE20:
    /* 1C620 8002BE20 FFFF0124 */  addiu      $at, $zero, -0x1
    /* 1C624 8002BE24 04004114 */  bne        $v0, $at, .L8002BE38
    /* 1C628 8002BE28 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 1C62C 8002BE2C 02008114 */  bne        $a0, $at, .L8002BE38
    /* 1C630 8002BE30 00000000 */   nop
    /* 1C634 8002BE34 0D000600 */  break      6
  .L8002BE38:
    /* 1C638 8002BE38 12200000 */  mflo       $a0
    /* 1C63C 8002BE3C 00000000 */  nop
    /* 1C640 8002BE40 00000000 */  nop
    /* 1C644 8002BE44 18002301 */  mult       $t1, $v1
    /* 1C648 8002BE48 12180000 */  mflo       $v1
    /* 1C64C 8002BE4C 00000000 */  nop
    /* 1C650 8002BE50 00000000 */  nop
    /* 1C654 8002BE54 1A006200 */  div        $zero, $v1, $v0
    /* 1C658 8002BE58 02004014 */  bnez       $v0, .L8002BE64
    /* 1C65C 8002BE5C 00000000 */   nop
    /* 1C660 8002BE60 0D000700 */  break      7
  .L8002BE64:
    /* 1C664 8002BE64 FFFF0124 */  addiu      $at, $zero, -0x1
    /* 1C668 8002BE68 04004114 */  bne        $v0, $at, .L8002BE7C
    /* 1C66C 8002BE6C 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 1C670 8002BE70 02006114 */  bne        $v1, $at, .L8002BE7C
    /* 1C674 8002BE74 00000000 */   nop
    /* 1C678 8002BE78 0D000600 */  break      6
  .L8002BE7C:
    /* 1C67C 8002BE7C 12100000 */  mflo       $v0
    /* 1C680 8002BE80 340146AD */  sw         $a2, 0x134($t2)
    /* 1C684 8002BE84 3C0145AD */  sw         $a1, 0x13C($t2)
    /* 1C688 8002BE88 340164AD */  sw         $a0, 0x134($t3)
    /* 1C68C 8002BE8C 3C0162AD */  sw         $v0, 0x13C($t3)
    /* 1C690 8002BE90 21100001 */  addu       $v0, $t0, $zero
    /* 1C694 8002BE94 0800BD27 */  addiu      $sp, $sp, 0x8
    /* 1C698 8002BE98 0800E003 */  jr         $ra
    /* 1C69C 8002BE9C 00000000 */   nop
endlabel func_8002BC68
