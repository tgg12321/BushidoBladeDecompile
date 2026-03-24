glabel func_800455AC
    /* 35DAC 800455AC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 35DB0 800455B0 1000B0AF */  sw         $s0, 0x10($sp)
    /* 35DB4 800455B4 1400BFAF */  sw         $ra, 0x14($sp)
    /* 35DB8 800455B8 F814010C */  jal        func_800453E0
    /* 35DBC 800455BC 21808000 */   addu      $s0, $a0, $zero
    /* 35DC0 800455C0 E002838F */  lw         $v1, %gp_rel(D_800A33AC)($gp)
    /* 35DC4 800455C4 0F80043C */  lui        $a0, %hi(D_800EED10)
    /* 35DC8 800455C8 10ED8424 */  addiu      $a0, $a0, %lo(D_800EED10)
    /* 35DCC 800455CC 01006224 */  addiu      $v0, $v1, 0x1
    /* 35DD0 800455D0 00190300 */  sll        $v1, $v1, 4
    /* 35DD4 800455D4 E00282AF */  sw         $v0, %gp_rel(D_800A33AC)($gp)
    /* 35DD8 800455D8 D402828F */  lw         $v0, %gp_rel(D_800A33A0)($gp)
    /* 35DDC 800455DC 21186400 */  addu       $v1, $v1, $a0
    /* 35DE0 800455E0 000070A4 */  sh         $s0, 0x0($v1)
    /* 35DE4 800455E4 0C0060AC */  sw         $zero, 0xC($v1)
    /* 35DE8 800455E8 040062AC */  sw         $v0, 0x4($v1)
    /* 35DEC 800455EC 1400BF8F */  lw         $ra, 0x14($sp)
    /* 35DF0 800455F0 1000B08F */  lw         $s0, 0x10($sp)
    /* 35DF4 800455F4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 35DF8 800455F8 0800E003 */  jr         $ra
    /* 35DFC 800455FC 00000000 */   nop
endlabel func_800455AC
