glabel func_80016C80
    /* 7480 80016C80 4A068293 */  lbu        $v0, %gp_rel(D_800A3716)($gp)
    /* 7484 80016C84 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 7488 80016C88 1400BFAF */  sw         $ra, 0x14($sp)
    /* 748C 80016C8C 15004014 */  bnez       $v0, .L80016CE4
    /* 7490 80016C90 1000B0AF */   sw        $s0, 0x10($sp)
    /* 7494 80016C94 1D80043C */  lui        $a0, (0x801D8800 >> 16)
    /* 7498 80016C98 00888434 */  ori        $a0, $a0, (0x801D8800 & 0xFFFF)
    /* 749C 80016C9C 1080053C */  lui        $a1, (0x8010E800 >> 16)
    /* 74A0 80016CA0 2E83010C */  jal        func_80060CB8
    /* 74A4 80016CA4 00E8A534 */   ori       $a1, $a1, (0x8010E800 & 0xFFFF)
    /* 74A8 80016CA8 0180043C */  lui        $a0, %hi(D_8001000C)
    /* 74AC 80016CAC 0C008424 */  addiu      $a0, $a0, %lo(D_8001000C)
    /* 74B0 80016CB0 1080053C */  lui        $a1, (0x8010E800 >> 16)
    /* 74B4 80016CB4 00E8A534 */  ori        $a1, $a1, (0x8010E800 & 0xFFFF)
    /* 74B8 80016CB8 21804000 */  addu       $s0, $v0, $zero
    /* 74BC 80016CBC 82E4010C */  jal        func_80079208
    /* 74C0 80016CC0 21300002 */   addu      $a2, $s0, $zero
    /* 74C4 80016CC4 00A00234 */  ori        $v0, $zero, 0xA000
    /* 74C8 80016CC8 2A105000 */  slt        $v0, $v0, $s0
    /* 74CC 80016CCC 04004010 */  beqz       $v0, .L80016CE0
    /* 74D0 80016CD0 01000224 */   addiu     $v0, $zero, 0x1
    /* 74D4 80016CD4 0F5B000C */  jal        func_80016C3C
    /* 74D8 80016CD8 00000000 */   nop
    /* 74DC 80016CDC 01000224 */  addiu      $v0, $zero, 0x1
  .L80016CE0:
    /* 74E0 80016CE0 4A0682A3 */  sb         $v0, %gp_rel(D_800A3716)($gp)
  .L80016CE4:
    /* 74E4 80016CE4 1400BF8F */  lw         $ra, 0x14($sp)
    /* 74E8 80016CE8 1000B08F */  lw         $s0, 0x10($sp)
    /* 74EC 80016CEC 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 74F0 80016CF0 0800E003 */  jr         $ra
    /* 74F4 80016CF4 00000000 */   nop
endlabel func_80016C80
