glabel func_8003DBE4
    /* 2E3E4 8003DBE4 C0FFBD27 */  addiu      $sp, $sp, -0x40
    /* 2E3E8 8003DBE8 3400B7AF */  sw         $s7, 0x34($sp)
    /* 2E3EC 8003DBEC 5000B78F */  lw         $s7, 0x50($sp)
    /* 2E3F0 8003DBF0 2800B4AF */  sw         $s4, 0x28($sp)
    /* 2E3F4 8003DBF4 21A08000 */  addu       $s4, $a0, $zero
    /* 2E3F8 8003DBF8 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 2E3FC 8003DBFC 2188A000 */  addu       $s1, $a1, $zero
    /* 2E400 8003DC00 1800B0AF */  sw         $s0, 0x18($sp)
    /* 2E404 8003DC04 2180C000 */  addu       $s0, $a2, $zero
    /* 2E408 8003DC08 2400B3AF */  sw         $s3, 0x24($sp)
    /* 2E40C 8003DC0C 2198E000 */  addu       $s3, $a3, $zero
    /* 2E410 8003DC10 3800BFAF */  sw         $ra, 0x38($sp)
    /* 2E414 8003DC14 3000B6AF */  sw         $s6, 0x30($sp)
    /* 2E418 8003DC18 2C00B5AF */  sw         $s5, 0x2C($sp)
    /* 2E41C 8003DC1C 0300E012 */  beqz       $s7, .L8003DC2C
    /* 2E420 8003DC20 2000B2AF */   sw        $s2, 0x20($sp)
    /* 2E424 8003DC24 0CF70008 */  j          .L8003DC30
    /* 2E428 8003DC28 21A82002 */   addu      $s5, $s1, $zero
  .L8003DC2C:
    /* 2E42C 8003DC2C FFFF3526 */  addiu      $s5, $s1, -0x1
  .L8003DC30:
    /* 2E430 8003DC30 0F80023C */  lui        $v0, %hi(D_800F6656)
    /* 2E434 8003DC34 56664294 */  lhu        $v0, %lo(D_800F6656)($v0)
    /* 2E438 8003DC38 00000000 */  nop
    /* 2E43C 8003DC3C 01004230 */  andi       $v0, $v0, 0x1
    /* 2E440 8003DC40 05004010 */  beqz       $v0, .L8003DC58
    /* 2E444 8003DC44 00000000 */   nop
    /* 2E448 8003DC48 0980123C */  lui        $s2, %hi(D_80090600)
    /* 2E44C 8003DC4C 0006528E */  lw         $s2, %lo(D_80090600)($s2)
    /* 2E450 8003DC50 1CF70008 */  j          .L8003DC70
    /* 2E454 8003DC54 00000000 */   nop
  .L8003DC58:
    /* 2E458 8003DC58 9AFC000C */  jal        game_GetPlayerCount
    /* 2E45C 8003DC5C 00000000 */   nop
    /* 2E460 8003DC60 02004014 */  bnez       $v0, .L8003DC6C
    /* 2E464 8003DC64 F0550224 */   addiu     $v0, $zero, 0x55F0
    /* 2E468 8003DC68 90650224 */  addiu      $v0, $zero, 0x6590
  .L8003DC6C:
    /* 2E46C 8003DC6C 23905400 */  subu       $s2, $v0, $s4
  .L8003DC70:
    /* 2E470 8003DC70 55004006 */  bltz       $s2, .L8003DDC8
    /* 2E474 8003DC74 2B107202 */   sltu      $v0, $s3, $s2
    /* 2E478 8003DC78 08004010 */  beqz       $v0, .L8003DC9C
    /* 2E47C 8003DC7C 00000000 */   nop
    /* 2E480 8003DC80 1B007102 */  divu       $zero, $s3, $s1
    /* 2E484 8003DC84 02002016 */  bnez       $s1, .L8003DC90
    /* 2E488 8003DC88 00000000 */   nop
    /* 2E48C 8003DC8C 0D000700 */  break      7
  .L8003DC90:
    /* 2E490 8003DC90 12900000 */  mflo       $s2
    /* 2E494 8003DC94 32F70008 */  j          .L8003DCC8
    /* 2E498 8003DC98 00000000 */   nop
  .L8003DC9C:
    /* 2E49C 8003DC9C 1A005102 */  div        $zero, $s2, $s1
    /* 2E4A0 8003DCA0 02002016 */  bnez       $s1, .L8003DCAC
    /* 2E4A4 8003DCA4 00000000 */   nop
    /* 2E4A8 8003DCA8 0D000700 */  break      7
  .L8003DCAC:
    /* 2E4AC 8003DCAC FFFF0124 */  addiu      $at, $zero, -0x1
    /* 2E4B0 8003DCB0 04002116 */  bne        $s1, $at, .L8003DCC4
    /* 2E4B4 8003DCB4 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 2E4B8 8003DCB8 02004116 */  bne        $s2, $at, .L8003DCC4
    /* 2E4BC 8003DCBC 00000000 */   nop
    /* 2E4C0 8003DCC0 0D000600 */  break      6
  .L8003DCC4:
    /* 2E4C4 8003DCC4 12900000 */  mflo       $s2
  .L8003DCC8:
    /* 2E4C8 8003DCC8 0A80033C */  lui        $v1, %hi(D_800A36AC)
    /* 2E4CC 8003DCCC AC36638C */  lw         $v1, %lo(D_800A36AC)($v1)
    /* 2E4D0 8003DCD0 21880000 */  addu       $s1, $zero, $zero
    /* 2E4D4 8003DCD4 01006330 */  andi       $v1, $v1, 0x1
    /* 2E4D8 8003DCD8 40100300 */  sll        $v0, $v1, 1
    /* 2E4DC 8003DCDC 21104300 */  addu       $v0, $v0, $v1
    /* 2E4E0 8003DCE0 C0100200 */  sll        $v0, $v0, 3
    /* 2E4E4 8003DCE4 2300A01A */  blez       $s5, .L8003DD74
    /* 2E4E8 8003DCE8 21800202 */   addu      $s0, $s0, $v0
    /* 2E4EC 8003DCEC FF00133C */  lui        $s3, (0xFFFFFF >> 16)
    /* 2E4F0 8003DCF0 FFFF7336 */  ori        $s3, $s3, (0xFFFFFF & 0xFFFF)
    /* 2E4F4 8003DCF4 00FF163C */  lui        $s6, (0xFF000000 >> 16)
  .L8003DCF8:
    /* 2E4F8 8003DCF8 82201400 */  srl        $a0, $s4, 2
    /* 2E4FC 8003DCFC 0A4B010C */  jal        func_80052C28
    /* 2E500 8003DD00 02000524 */   addiu     $a1, $zero, 0x2
    /* 2E504 8003DD04 21284000 */  addu       $a1, $v0, $zero
    /* 2E508 8003DD08 0010A228 */  slti       $v0, $a1, 0x1000
    /* 2E50C 8003DD0C 15004010 */  beqz       $v0, .L8003DD64
    /* 2E510 8003DD10 80200500 */   sll       $a0, $a1, 2
    /* 2E514 8003DD14 0A80023C */  lui        $v0, %hi(D_800A378C)
    /* 2E518 8003DD18 8C37428C */  lw         $v0, %lo(D_800A378C)($v0)
    /* 2E51C 8003DD1C 0000038E */  lw         $v1, 0x0($s0)
    /* 2E520 8003DD20 21208200 */  addu       $a0, $a0, $v0
    /* 2E524 8003DD24 0000828C */  lw         $v0, 0x0($a0)
    /* 2E528 8003DD28 24187600 */  and        $v1, $v1, $s6
    /* 2E52C 8003DD2C 24105300 */  and        $v0, $v0, $s3
    /* 2E530 8003DD30 25186200 */  or         $v1, $v1, $v0
    /* 2E534 8003DD34 000003AE */  sw         $v1, 0x0($s0)
    /* 2E538 8003DD38 24181302 */  and        $v1, $s0, $s3
    /* 2E53C 8003DD3C 0000828C */  lw         $v0, 0x0($a0)
    /* 2E540 8003DD40 00000000 */  nop
    /* 2E544 8003DD44 24105600 */  and        $v0, $v0, $s6
    /* 2E548 8003DD48 25104300 */  or         $v0, $v0, $v1
    /* 2E54C 8003DD4C 000082AC */  sw         $v0, 0x0($a0)
    /* 2E550 8003DD50 FFFFA226 */  addiu      $v0, $s5, -0x1
    /* 2E554 8003DD54 03002216 */  bne        $s1, $v0, .L8003DD64
    /* 2E558 8003DD58 30001026 */   addiu     $s0, $s0, 0x30
    /* 2E55C 8003DD5C 0980013C */  lui        $at, %hi(D_800905F8)
    /* 2E560 8003DD60 F80525AC */  sw         $a1, %lo(D_800905F8)($at)
  .L8003DD64:
    /* 2E564 8003DD64 01003126 */  addiu      $s1, $s1, 0x1
    /* 2E568 8003DD68 2A103502 */  slt        $v0, $s1, $s5
    /* 2E56C 8003DD6C E2FF4014 */  bnez       $v0, .L8003DCF8
    /* 2E570 8003DD70 21A09202 */   addu      $s4, $s4, $s2
  .L8003DD74:
    /* 2E574 8003DD74 0500E012 */  beqz       $s7, .L8003DD8C
    /* 2E578 8003DD78 FF00043C */   lui       $a0, (0xFFFFFF >> 16)
    /* 2E57C 8003DD7C 7EF7000C */  jal        func_8003DDF8
    /* 2E580 8003DD80 21200002 */   addu      $a0, $s0, $zero
    /* 2E584 8003DD84 72F70008 */  j          .L8003DDC8
    /* 2E588 8003DD88 00000000 */   nop
  .L8003DD8C:
    /* 2E58C 8003DD8C FFFF8434 */  ori        $a0, $a0, (0xFFFFFF & 0xFFFF)
    /* 2E590 8003DD90 00FF063C */  lui        $a2, (0xFF000000 >> 16)
    /* 2E594 8003DD94 0A80053C */  lui        $a1, %hi(D_800A378C)
    /* 2E598 8003DD98 8C37A58C */  lw         $a1, %lo(D_800A378C)($a1)
    /* 2E59C 8003DD9C 0000038E */  lw         $v1, 0x0($s0)
    /* 2E5A0 8003DDA0 EC3FA28C */  lw         $v0, 0x3FEC($a1)
    /* 2E5A4 8003DDA4 24186600 */  and        $v1, $v1, $a2
    /* 2E5A8 8003DDA8 24104400 */  and        $v0, $v0, $a0
    /* 2E5AC 8003DDAC 25186200 */  or         $v1, $v1, $v0
    /* 2E5B0 8003DDB0 000003AE */  sw         $v1, 0x0($s0)
    /* 2E5B4 8003DDB4 EC3FA28C */  lw         $v0, 0x3FEC($a1)
    /* 2E5B8 8003DDB8 24200402 */  and        $a0, $s0, $a0
    /* 2E5BC 8003DDBC 24104600 */  and        $v0, $v0, $a2
    /* 2E5C0 8003DDC0 25104400 */  or         $v0, $v0, $a0
    /* 2E5C4 8003DDC4 EC3FA2AC */  sw         $v0, 0x3FEC($a1)
  .L8003DDC8:
    /* 2E5C8 8003DDC8 3800BF8F */  lw         $ra, 0x38($sp)
    /* 2E5CC 8003DDCC 3400B78F */  lw         $s7, 0x34($sp)
    /* 2E5D0 8003DDD0 3000B68F */  lw         $s6, 0x30($sp)
    /* 2E5D4 8003DDD4 2C00B58F */  lw         $s5, 0x2C($sp)
    /* 2E5D8 8003DDD8 2800B48F */  lw         $s4, 0x28($sp)
    /* 2E5DC 8003DDDC 2400B38F */  lw         $s3, 0x24($sp)
    /* 2E5E0 8003DDE0 2000B28F */  lw         $s2, 0x20($sp)
    /* 2E5E4 8003DDE4 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 2E5E8 8003DDE8 1800B08F */  lw         $s0, 0x18($sp)
    /* 2E5EC 8003DDEC 4000BD27 */  addiu      $sp, $sp, 0x40
    /* 2E5F0 8003DDF0 0800E003 */  jr         $ra
    /* 2E5F4 8003DDF4 00000000 */   nop
endlabel func_8003DBE4
