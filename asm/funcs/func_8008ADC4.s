glabel func_8008ADC4
    /* 7B5C4 8008ADC4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 7B5C8 8008ADC8 1000B0AF */  sw         $s0, 0x10($sp)
    /* 7B5CC 8008ADCC 2180A000 */  addu       $s0, $a1, $zero
    /* 7B5D0 8008ADD0 0700023C */  lui        $v0, (0x7EFF0 >> 16)
    /* 7B5D4 8008ADD4 F0EF4234 */  ori        $v0, $v0, (0x7EFF0 & 0xFFFF)
    /* 7B5D8 8008ADD8 2B105000 */  sltu       $v0, $v0, $s0
    /* 7B5DC 8008ADDC 03004010 */  beqz       $v0, .L8008ADEC
    /* 7B5E0 8008ADE0 1400BFAF */   sw        $ra, 0x14($sp)
    /* 7B5E4 8008ADE4 0700103C */  lui        $s0, (0x7EFF0 >> 16)
    /* 7B5E8 8008ADE8 F0EF1036 */  ori        $s0, $s0, (0x7EFF0 & 0xFFFF)
  .L8008ADEC:
    /* 7B5EC 8008ADEC E723020C */  jal        func_80088F9C
    /* 7B5F0 8008ADF0 21280002 */   addu      $a1, $s0, $zero
    /* 7B5F4 8008ADF4 0A80023C */  lui        $v0, %hi(D_800A2D14)
    /* 7B5F8 8008ADF8 142D428C */  lw         $v0, %lo(D_800A2D14)($v0)
    /* 7B5FC 8008ADFC 00000000 */  nop
    /* 7B600 8008AE00 03004014 */  bnez       $v0, .L8008AE10
    /* 7B604 8008AE04 21100002 */   addu      $v0, $s0, $zero
    /* 7B608 8008AE08 0A80013C */  lui        $at, %hi(D_800A2D10)
    /* 7B60C 8008AE0C 102D20AC */  sw         $zero, %lo(D_800A2D10)($at)
  .L8008AE10:
    /* 7B610 8008AE10 1400BF8F */  lw         $ra, 0x14($sp)
    /* 7B614 8008AE14 1000B08F */  lw         $s0, 0x10($sp)
    /* 7B618 8008AE18 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 7B61C 8008AE1C 0800E003 */  jr         $ra
    /* 7B620 8008AE20 00000000 */   nop
endlabel func_8008ADC4
