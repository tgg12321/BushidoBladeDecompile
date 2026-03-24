glabel func_8007B78C
    /* 6BF8C 8007B78C 0A80023C */  lui        $v0, %hi(D_8009BE76)
    /* 6BF90 8007B790 76BE4290 */  lbu        $v0, %lo(D_8009BE76)($v0)
    /* 6BF94 8007B794 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6BF98 8007B798 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6BF9C 8007B79C 21808000 */  addu       $s0, $a0, $zero
    /* 6BFA0 8007B7A0 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6BFA4 8007B7A4 2188A000 */  addu       $s1, $a1, $zero
    /* 6BFA8 8007B7A8 0200422C */  sltiu      $v0, $v0, 0x2
    /* 6BFAC 8007B7AC 08004014 */  bnez       $v0, .L8007B7D0
    /* 6BFB0 8007B7B0 1800BFAF */   sw        $ra, 0x18($sp)
    /* 6BFB4 8007B7B4 0A80023C */  lui        $v0, %hi(D_8009BE70)
    /* 6BFB8 8007B7B8 70BE428C */  lw         $v0, %lo(D_8009BE70)($v0)
    /* 6BFBC 8007B7BC 0180043C */  lui        $a0, %hi(D_80015F80)
    /* 6BFC0 8007B7C0 805F8424 */  addiu      $a0, $a0, %lo(D_80015F80)
    /* 6BFC4 8007B7C4 21280002 */  addu       $a1, $s0, $zero
    /* 6BFC8 8007B7C8 09F84000 */  jalr       $v0
    /* 6BFCC 8007B7CC 21302002 */   addu      $a2, $s1, $zero
  .L8007B7D0:
    /* 6BFD0 8007B7D0 FFFF3126 */  addiu      $s1, $s1, -0x1
    /* 6BFD4 8007B7D4 0F002012 */  beqz       $s1, .L8007B814
    /* 6BFD8 8007B7D8 FF00033C */   lui       $v1, (0xFFFFFF >> 16)
    /* 6BFDC 8007B7DC FF00053C */  lui        $a1, (0xFFFFFF >> 16)
    /* 6BFE0 8007B7E0 FFFFA534 */  ori        $a1, $a1, (0xFFFFFF & 0xFFFF)
    /* 6BFE4 8007B7E4 00FF063C */  lui        $a2, (0xFF000000 >> 16)
  .L8007B7E8:
    /* 6BFE8 8007B7E8 FFFF3126 */  addiu      $s1, $s1, -0x1
    /* 6BFEC 8007B7EC 04000426 */  addiu      $a0, $s0, 0x4
    /* 6BFF0 8007B7F0 030000A2 */  sb         $zero, 0x3($s0)
    /* 6BFF4 8007B7F4 0000028E */  lw         $v0, 0x0($s0)
    /* 6BFF8 8007B7F8 24188500 */  and        $v1, $a0, $a1
    /* 6BFFC 8007B7FC 24104600 */  and        $v0, $v0, $a2
    /* 6C000 8007B800 25104300 */  or         $v0, $v0, $v1
    /* 6C004 8007B804 000002AE */  sw         $v0, 0x0($s0)
    /* 6C008 8007B808 F7FF2016 */  bnez       $s1, .L8007B7E8
    /* 6C00C 8007B80C 21808000 */   addu      $s0, $a0, $zero
    /* 6C010 8007B810 FF00033C */  lui        $v1, (0xFFFFFF >> 16)
  .L8007B814:
    /* 6C014 8007B814 FFFF6334 */  ori        $v1, $v1, (0xFFFFFF & 0xFFFF)
    /* 6C018 8007B818 0A80023C */  lui        $v0, %hi(D_8009BF30)
    /* 6C01C 8007B81C 30BF4224 */  addiu      $v0, $v0, %lo(D_8009BF30)
    /* 6C020 8007B820 24104300 */  and        $v0, $v0, $v1
    /* 6C024 8007B824 000002AE */  sw         $v0, 0x0($s0)
    /* 6C028 8007B828 21100002 */  addu       $v0, $s0, $zero
    /* 6C02C 8007B82C 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6C030 8007B830 1400B18F */  lw         $s1, 0x14($sp)
    /* 6C034 8007B834 1000B08F */  lw         $s0, 0x10($sp)
    /* 6C038 8007B838 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6C03C 8007B83C 0800E003 */  jr         $ra
    /* 6C040 8007B840 00000000 */   nop
endlabel func_8007B78C
