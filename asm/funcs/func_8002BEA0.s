glabel func_8002BEA0
    /* 1C6A0 8002BEA0 1080033C */  lui        $v1, %hi(D_80101FBC)
    /* 1C6A4 8002BEA4 BC1F638C */  lw         $v1, %lo(D_80101FBC)($v1)
    /* 1C6A8 8002BEA8 1080023C */  lui        $v0, %hi(D_80102408)
    /* 1C6AC 8002BEAC 0824428C */  lw         $v0, %lo(D_80102408)($v0)
    /* 1C6B0 8002BEB0 00000000 */  nop
    /* 1C6B4 8002BEB4 23386200 */  subu       $a3, $v1, $v0
    /* 1C6B8 8002BEB8 1800E700 */  mult       $a3, $a3
    /* 1C6BC 8002BEBC 1080033C */  lui        $v1, %hi(D_80101FC4)
    /* 1C6C0 8002BEC0 C41F638C */  lw         $v1, %lo(D_80101FC4)($v1)
    /* 1C6C4 8002BEC4 1080023C */  lui        $v0, %hi(D_80102410)
    /* 1C6C8 8002BEC8 1024428C */  lw         $v0, %lo(D_80102410)($v0)
    /* 1C6CC 8002BECC 12200000 */  mflo       $a0
    /* 1C6D0 8002BED0 23486200 */  subu       $t1, $v1, $v0
    /* 1C6D4 8002BED4 00000000 */  nop
    /* 1C6D8 8002BED8 18002901 */  mult       $t1, $t1
    /* 1C6DC 8002BEDC F8FFBD27 */  addiu      $sp, $sp, -0x8
    /* 1C6E0 8002BEE0 10800A3C */  lui        $t2, %hi(D_80101EC8)
    /* 1C6E4 8002BEE4 C81E4A25 */  addiu      $t2, $t2, %lo(D_80101EC8)
    /* 1C6E8 8002BEE8 12180000 */  mflo       $v1
    /* 1C6EC 8002BEEC 21208300 */  addu       $a0, $a0, $v1
    /* 1C6F0 8002BEF0 0004822C */  sltiu      $v0, $a0, 0x400
    /* 1C6F4 8002BEF4 06004010 */  beqz       $v0, .L8002BF10
    /* 1C6F8 8002BEF8 4C044B25 */   addiu     $t3, $t2, 0x44C
    /* 1C6FC 8002BEFC 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1C700 8002BF00 21082400 */  addu       $at, $at, $a0
    /* 1C704 8002BF04 18D12290 */  lbu        $v0, %lo(D_8008D118)($at)
    /* 1C708 8002BF08 D8AF0008 */  j          .L8002BF60
    /* 1C70C 8002BF0C C2400200 */   srl       $t0, $v0, 3
  .L8002BF10:
    /* 1C710 8002BF10 21608000 */  addu       $t4, $a0, $zero
    /* 1C714 8002BF14 00F08C48 */  mtc2       $t4, $30 /* handwritten instruction */
    /* 1C718 8002BF18 00000000 */  nop
    /* 1C71C 8002BF1C 00000000 */  nop
    /* 1C720 8002BF20 2160A003 */  addu       $t4, $sp, $zero
    /* 1C724 8002BF24 00009FE9 */  swc2       $31, 0x0($t4)
    /* 1C728 8002BF28 0000A38F */  lw         $v1, 0x0($sp)
    /* 1C72C 8002BF2C FEFF0224 */  addiu      $v0, $zero, -0x2
    /* 1C730 8002BF30 24106200 */  and        $v0, $v1, $v0
    /* 1C734 8002BF34 16000324 */  addiu      $v1, $zero, 0x16
    /* 1C738 8002BF38 23186200 */  subu       $v1, $v1, $v0
    /* 1C73C 8002BF3C 06106400 */  srlv       $v0, $a0, $v1
    /* 1C740 8002BF40 42180300 */  srl        $v1, $v1, 1
    /* 1C744 8002BF44 0980013C */  lui        $at, %hi(D_8008D118)
    /* 1C748 8002BF48 21082200 */  addu       $at, $at, $v0
    /* 1C74C 8002BF4C 18D12490 */  lbu        $a0, %lo(D_8008D118)($at)
    /* 1C750 8002BF50 13000224 */  addiu      $v0, $zero, 0x13
    /* 1C754 8002BF54 23104300 */  subu       $v0, $v0, $v1
    /* 1C758 8002BF58 00240400 */  sll        $a0, $a0, 16
    /* 1C75C 8002BF5C 06404400 */  srlv       $t0, $a0, $v0
  .L8002BF60:
    /* 1C760 8002BF60 4C040229 */  slti       $v0, $t0, 0x44C
    /* 1C764 8002BF64 0D004010 */  beqz       $v0, .L8002BF9C
    /* 1C768 8002BF68 EB51043C */   lui       $a0, (0x51EB851F >> 16)
    /* 1C76C 8002BF6C 1F858434 */  ori        $a0, $a0, (0x51EB851F & 0xFFFF)
    /* 1C770 8002BF70 4C040324 */  addiu      $v1, $zero, 0x44C
    /* 1C774 8002BF74 23186800 */  subu       $v1, $v1, $t0
    /* 1C778 8002BF78 80100300 */  sll        $v0, $v1, 2
    /* 1C77C 8002BF7C 21104300 */  addu       $v0, $v0, $v1
    /* 1C780 8002BF80 00110200 */  sll        $v0, $v0, 4
    /* 1C784 8002BF84 18004400 */  mult       $v0, $a0
    /* 1C788 8002BF88 C3170200 */  sra        $v0, $v0, 31
    /* 1C78C 8002BF8C 10C00000 */  mfhi       $t8
    /* 1C790 8002BF90 43191800 */  sra        $v1, $t8, 5
    /* 1C794 8002BF94 EDAF0008 */  j          .L8002BFB4
    /* 1C798 8002BF98 23206200 */   subu      $a0, $v1, $v0
  .L8002BF9C:
    /* 1C79C 8002BF9C 4C040224 */  addiu      $v0, $zero, 0x44C
    /* 1C7A0 8002BFA0 23104800 */  subu       $v0, $v0, $t0
    /* 1C7A4 8002BFA4 03004104 */  bgez       $v0, .L8002BFB4
    /* 1C7A8 8002BFA8 03210200 */   sra       $a0, $v0, 4
    /* 1C7AC 8002BFAC 0F004224 */  addiu      $v0, $v0, 0xF
    /* 1C7B0 8002BFB0 03210200 */  sra        $a0, $v0, 4
  .L8002BFB4:
    /* 1C7B4 8002BFB4 1800E400 */  mult       $a3, $a0
    /* 1C7B8 8002BFB8 12180000 */  mflo       $v1
    /* 1C7BC 8002BFBC E8030224 */  addiu      $v0, $zero, 0x3E8
    /* 1C7C0 8002BFC0 00000000 */  nop
    /* 1C7C4 8002BFC4 1A006200 */  div        $zero, $v1, $v0
    /* 1C7C8 8002BFC8 02004014 */  bnez       $v0, .L8002BFD4
    /* 1C7CC 8002BFCC 00000000 */   nop
    /* 1C7D0 8002BFD0 0D000700 */  break      7
  .L8002BFD4:
    /* 1C7D4 8002BFD4 FFFF0124 */  addiu      $at, $zero, -0x1
    /* 1C7D8 8002BFD8 04004114 */  bne        $v0, $at, .L8002BFEC
    /* 1C7DC 8002BFDC 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 1C7E0 8002BFE0 02006114 */  bne        $v1, $at, .L8002BFEC
    /* 1C7E4 8002BFE4 00000000 */   nop
    /* 1C7E8 8002BFE8 0D000600 */  break      6
  .L8002BFEC:
    /* 1C7EC 8002BFEC 12300000 */  mflo       $a2
    /* 1C7F0 8002BFF0 00000000 */  nop
    /* 1C7F4 8002BFF4 00000000 */  nop
    /* 1C7F8 8002BFF8 18002401 */  mult       $t1, $a0
    /* 1C7FC 8002BFFC 12180000 */  mflo       $v1
    /* 1C800 8002C000 00000000 */  nop
    /* 1C804 8002C004 00000000 */  nop
    /* 1C808 8002C008 1A006200 */  div        $zero, $v1, $v0
    /* 1C80C 8002C00C 02004014 */  bnez       $v0, .L8002C018
    /* 1C810 8002C010 00000000 */   nop
    /* 1C814 8002C014 0D000700 */  break      7
  .L8002C018:
    /* 1C818 8002C018 FFFF0124 */  addiu      $at, $zero, -0x1
    /* 1C81C 8002C01C 04004114 */  bne        $v0, $at, .L8002C030
    /* 1C820 8002C020 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 1C824 8002C024 02006114 */  bne        $v1, $at, .L8002C030
    /* 1C828 8002C028 00000000 */   nop
    /* 1C82C 8002C02C 0D000600 */  break      6
  .L8002C030:
    /* 1C830 8002C030 12280000 */  mflo       $a1
    /* 1C834 8002C034 23180400 */  negu       $v1, $a0
    /* 1C838 8002C038 00000000 */  nop
    /* 1C83C 8002C03C 1800E300 */  mult       $a3, $v1
    /* 1C840 8002C040 12200000 */  mflo       $a0
    /* 1C844 8002C044 00000000 */  nop
    /* 1C848 8002C048 00000000 */  nop
    /* 1C84C 8002C04C 1A008200 */  div        $zero, $a0, $v0
    /* 1C850 8002C050 02004014 */  bnez       $v0, .L8002C05C
    /* 1C854 8002C054 00000000 */   nop
    /* 1C858 8002C058 0D000700 */  break      7
  .L8002C05C:
    /* 1C85C 8002C05C FFFF0124 */  addiu      $at, $zero, -0x1
    /* 1C860 8002C060 04004114 */  bne        $v0, $at, .L8002C074
    /* 1C864 8002C064 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 1C868 8002C068 02008114 */  bne        $a0, $at, .L8002C074
    /* 1C86C 8002C06C 00000000 */   nop
    /* 1C870 8002C070 0D000600 */  break      6
  .L8002C074:
    /* 1C874 8002C074 12200000 */  mflo       $a0
    /* 1C878 8002C078 00000000 */  nop
    /* 1C87C 8002C07C 00000000 */  nop
    /* 1C880 8002C080 18002301 */  mult       $t1, $v1
    /* 1C884 8002C084 12180000 */  mflo       $v1
    /* 1C888 8002C088 00000000 */  nop
    /* 1C88C 8002C08C 00000000 */  nop
    /* 1C890 8002C090 1A006200 */  div        $zero, $v1, $v0
    /* 1C894 8002C094 02004014 */  bnez       $v0, .L8002C0A0
    /* 1C898 8002C098 00000000 */   nop
    /* 1C89C 8002C09C 0D000700 */  break      7
  .L8002C0A0:
    /* 1C8A0 8002C0A0 FFFF0124 */  addiu      $at, $zero, -0x1
    /* 1C8A4 8002C0A4 04004114 */  bne        $v0, $at, .L8002C0B8
    /* 1C8A8 8002C0A8 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 1C8AC 8002C0AC 02006114 */  bne        $v1, $at, .L8002C0B8
    /* 1C8B0 8002C0B0 00000000 */   nop
    /* 1C8B4 8002C0B4 0D000600 */  break      6
  .L8002C0B8:
    /* 1C8B8 8002C0B8 12100000 */  mflo       $v0
    /* 1C8BC 8002C0BC 340146AD */  sw         $a2, 0x134($t2)
    /* 1C8C0 8002C0C0 3C0145AD */  sw         $a1, 0x13C($t2)
    /* 1C8C4 8002C0C4 340164AD */  sw         $a0, 0x134($t3)
    /* 1C8C8 8002C0C8 3C0162AD */  sw         $v0, 0x13C($t3)
    /* 1C8CC 8002C0CC B4FB0225 */  addiu      $v0, $t0, -0x44C
    /* 1C8D0 8002C0D0 0800BD27 */  addiu      $sp, $sp, 0x8
    /* 1C8D4 8002C0D4 0800E003 */  jr         $ra
    /* 1C8D8 8002C0D8 00000000 */   nop
endlabel func_8002BEA0
