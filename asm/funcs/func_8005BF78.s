glabel func_8005BF78
    /* 4C778 8005BF78 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 4C77C 8005BF7C 2000B4AF */  sw         $s4, 0x20($sp)
    /* 4C780 8005BF80 21A08000 */  addu       $s4, $a0, $zero
    /* 4C784 8005BF84 1800B2AF */  sw         $s2, 0x18($sp)
    /* 4C788 8005BF88 2190A000 */  addu       $s2, $a1, $zero
    /* 4C78C 8005BF8C 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 4C790 8005BF90 2198C000 */  addu       $s3, $a2, $zero
    /* 4C794 8005BF94 1000B0AF */  sw         $s0, 0x10($sp)
    /* 4C798 8005BF98 2180E000 */  addu       $s0, $a3, $zero
    /* 4C79C 8005BF9C 21200000 */  addu       $a0, $zero, $zero
    /* 4C7A0 8005BFA0 2400BFAF */  sw         $ra, 0x24($sp)
    /* 4C7A4 8005BFA4 3416020C */  jal        title_mv_exec2
    /* 4C7A8 8005BFA8 1400B1AF */   sw        $s1, 0x14($sp)
    /* 4C7AC 8005BFAC 008C1200 */  sll        $s1, $s2, 16
    /* 4C7B0 8005BFB0 038C1100 */  sra        $s1, $s1, 16
    /* 4C7B4 8005BFB4 D91F020C */  jal        func_80087F64
    /* 4C7B8 8005BFB8 21202002 */   addu      $a0, $s1, $zero
    /* 4C7BC 8005BFBC 892B020C */  jal        func_8008AE24
    /* 4C7C0 8005BFC0 21200002 */   addu      $a0, $s0, $zero
    /* 4C7C4 8005BFC4 0F80103C */  lui        $s0, %hi(D_800EFC38)
    /* 4C7C8 8005BFC8 38FC1026 */  addiu      $s0, $s0, %lo(D_800EFC38)
    /* 4C7CC 8005BFCC 80901200 */  sll        $s2, $s2, 2
    /* 4C7D0 8005BFD0 21805002 */  addu       $s0, $s2, $s0
    /* 4C7D4 8005BFD4 0000028E */  lw         $v0, 0x0($s0)
    /* 4C7D8 8005BFD8 00000000 */  nop
    /* 4C7DC 8005BFDC 0C00458C */  lw         $a1, 0xC($v0)
    /* 4C7E0 8005BFE0 592B020C */  jal        func_8008AD64
    /* 4C7E4 8005BFE4 21208002 */   addu      $a0, $s4, $zero
    /* 4C7E8 8005BFE8 AC2B020C */  jal        func_8008AEB0
    /* 4C7EC 8005BFEC 01000424 */   addiu     $a0, $zero, 0x1
    /* 4C7F0 8005BFF0 892B020C */  jal        func_8008AE24
    /* 4C7F4 8005BFF4 21206002 */   addu      $a0, $s3, $zero
    /* 4C7F8 8005BFF8 0000028E */  lw         $v0, 0x0($s0)
    /* 4C7FC 8005BFFC 00000000 */  nop
    /* 4C800 8005C000 0C00458C */  lw         $a1, 0xC($v0)
    /* 4C804 8005C004 712B020C */  jal        func_8008ADC4
    /* 4C808 8005C008 21208002 */   addu      $a0, $s4, $zero
    /* 4C80C 8005C00C AC2B020C */  jal        func_8008AEB0
    /* 4C810 8005C010 01000424 */   addiu     $a0, $zero, 0x1
    /* 4C814 8005C014 0000028E */  lw         $v0, 0x0($s0)
    /* 4C818 8005C018 21282002 */  addu       $a1, $s1, $zero
    /* 4C81C 8005C01C 0400448C */  lw         $a0, 0x4($v0)
    /* 4C820 8005C020 2E20020C */  jal        func_800880B8
    /* 4C824 8005C024 21306002 */   addu      $a2, $s3, $zero
    /* 4C828 8005C028 FA1F020C */  jal        func_80087FE8
    /* 4C82C 8005C02C 21202002 */   addu      $a0, $s1, $zero
    /* 4C830 8005C030 0F80013C */  lui        $at, %hi(D_800EFB38)
    /* 4C834 8005C034 21083200 */  addu       $at, $at, $s2
    /* 4C838 8005C038 38FB33AC */  sw         $s3, %lo(D_800EFB38)($at)
    /* 4C83C 8005C03C 0000028E */  lw         $v0, 0x0($s0)
    /* 4C840 8005C040 00000000 */  nop
    /* 4C844 8005C044 0C00428C */  lw         $v0, 0xC($v0)
    /* 4C848 8005C048 00000000 */  nop
    /* 4C84C 8005C04C 21106202 */  addu       $v0, $s3, $v0
    /* 4C850 8005C050 2400BF8F */  lw         $ra, 0x24($sp)
    /* 4C854 8005C054 2000B48F */  lw         $s4, 0x20($sp)
    /* 4C858 8005C058 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 4C85C 8005C05C 1800B28F */  lw         $s2, 0x18($sp)
    /* 4C860 8005C060 1400B18F */  lw         $s1, 0x14($sp)
    /* 4C864 8005C064 1000B08F */  lw         $s0, 0x10($sp)
    /* 4C868 8005C068 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 4C86C 8005C06C 0800E003 */  jr         $ra
    /* 4C870 8005C070 00000000 */   nop
endlabel func_8005BF78
