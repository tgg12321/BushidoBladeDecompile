glabel func_8005C6D0
    /* 4CED0 8005C6D0 A8FFBD27 */  addiu      $sp, $sp, -0x58
    /* 4CED4 8005C6D4 2000A427 */  addiu      $a0, $sp, 0x20
    /* 4CED8 8005C6D8 5400BFAF */  sw         $ra, 0x54($sp)
    /* 4CEDC 8005C6DC 5000B6AF */  sw         $s6, 0x50($sp)
    /* 4CEE0 8005C6E0 4C00B5AF */  sw         $s5, 0x4C($sp)
    /* 4CEE4 8005C6E4 4800B4AF */  sw         $s4, 0x48($sp)
    /* 4CEE8 8005C6E8 4400B3AF */  sw         $s3, 0x44($sp)
    /* 4CEEC 8005C6EC 4000B2AF */  sw         $s2, 0x40($sp)
    /* 4CEF0 8005C6F0 3C00B1AF */  sw         $s1, 0x3C($sp)
    /* 4CEF4 8005C6F4 002D020C */  jal        func_8008B400
    /* 4CEF8 8005C6F8 3800B0AF */   sw        $s0, 0x38($sp)
    /* 4CEFC 8005C6FC 21A00000 */  addu       $s4, $zero, $zero
    /* 4CF00 8005C700 21980000 */  addu       $s3, $zero, $zero
    /* 4CF04 8005C704 0F80163C */  lui        $s6, %hi(D_800EFC38)
    /* 4CF08 8005C708 38FCD626 */  addiu      $s6, $s6, %lo(D_800EFC38)
    /* 4CF0C 8005C70C 01001524 */  addiu      $s5, $zero, 0x1
    /* 4CF10 8005C710 00141300 */  sll        $v0, $s3, 16
  .L8005C714:
    /* 4CF14 8005C714 431B0200 */  sra        $v1, $v0, 13
    /* 4CF18 8005C718 0F80013C */  lui        $at, %hi(D_800EFB78)
    /* 4CF1C 8005C71C 21082300 */  addu       $at, $at, $v1
    /* 4CF20 8005C720 78FB318C */  lw         $s1, %lo(D_800EFB78)($at)
    /* 4CF24 8005C724 00000000 */  nop
    /* 4CF28 8005C728 4A002012 */  beqz       $s1, .L8005C854
    /* 4CF2C 8005C72C 43130200 */   sra       $v0, $v0, 13
    /* 4CF30 8005C730 00002296 */  lhu        $v0, 0x0($s1)
    /* 4CF34 8005C734 00000000 */  nop
    /* 4CF38 8005C738 80100200 */  sll        $v0, $v0, 2
    /* 4CF3C 8005C73C 21105600 */  addu       $v0, $v0, $s6
    /* 4CF40 8005C740 0000428C */  lw         $v0, 0x0($v0)
    /* 4CF44 8005C744 00000000 */  nop
    /* 4CF48 8005C748 41004104 */  bgez       $v0, .L8005C850
    /* 4CF4C 8005C74C 00141300 */   sll       $v0, $s3, 16
    /* 4CF50 8005C750 21808002 */  addu       $s0, $s4, $zero
    /* 4CF54 8005C754 00141000 */  sll        $v0, $s0, 16
    /* 4CF58 8005C758 03140200 */  sra        $v0, $v0, 16
    /* 4CF5C 8005C75C 18004228 */  slti       $v0, $v0, 0x18
    /* 4CF60 8005C760 3B004010 */  beqz       $v0, .L8005C850
    /* 4CF64 8005C764 00141300 */   sll       $v0, $s3, 16
    /* 4CF68 8005C768 21906000 */  addu       $s2, $v1, $zero
  .L8005C76C:
    /* 4CF6C 8005C76C 342B020C */  jal        func_8008ACD0
    /* 4CF70 8005C770 04201502 */   sllv      $a0, $s5, $s0
    /* 4CF74 8005C774 30005510 */  beq        $v0, $s5, .L8005C838
    /* 4CF78 8005C778 01000226 */   addiu     $v0, $s0, 0x1
    /* 4CF7C 8005C77C 00002596 */  lhu        $a1, 0x0($s1)
    /* 4CF80 8005C780 06000224 */  addiu      $v0, $zero, 0x6
    /* 4CF84 8005C784 FFFFA330 */  andi       $v1, $a1, 0xFFFF
    /* 4CF88 8005C788 0A006214 */  bne        $v1, $v0, .L8005C7B4
    /* 4CF8C 8005C78C 80100500 */   sll       $v0, $a1, 2
    /* 4CF90 8005C790 0F80033C */  lui        $v1, %hi(D_800EFC50)
    /* 4CF94 8005C794 50FC638C */  lw         $v1, %lo(D_800EFC50)($v1)
    /* 4CF98 8005C798 0F80023C */  lui        $v0, %hi(D_800EFC44)
    /* 4CF9C 8005C79C 44FC428C */  lw         $v0, %lo(D_800EFC44)($v0)
    /* 4CFA0 8005C7A0 00000000 */  nop
    /* 4CFA4 8005C7A4 03006214 */  bne        $v1, $v0, .L8005C7B4
    /* 4CFA8 8005C7A8 80100500 */   sll       $v0, $a1, 2
    /* 4CFAC 8005C7AC 03000524 */  addiu      $a1, $zero, 0x3
    /* 4CFB0 8005C7B0 80100500 */  sll        $v0, $a1, 2
  .L8005C7B4:
    /* 4CFB4 8005C7B4 21105600 */  addu       $v0, $v0, $s6
    /* 4CFB8 8005C7B8 00241000 */  sll        $a0, $s0, 16
    /* 4CFBC 8005C7BC 03240400 */  sra        $a0, $a0, 16
    /* 4CFC0 8005C7C0 01001426 */  addiu      $s4, $s0, 0x1
    /* 4CFC4 8005C7C4 002C0500 */  sll        $a1, $a1, 16
    /* 4CFC8 8005C7C8 0000438C */  lw         $v1, 0x0($v0)
    /* 4CFCC 8005C7CC 02002296 */  lhu        $v0, 0x2($s1)
    /* 4CFD0 8005C7D0 0000638C */  lw         $v1, 0x0($v1)
    /* 4CFD4 8005C7D4 80100200 */  sll        $v0, $v0, 2
    /* 4CFD8 8005C7D8 21186200 */  addu       $v1, $v1, $v0
    /* 4CFDC 8005C7DC 0000668C */  lw         $a2, 0x0($v1)
    /* 4CFE0 8005C7E0 032C0500 */  sra        $a1, $a1, 16
    /* 4CFE4 8005C7E4 C2120600 */  srl        $v0, $a2, 11
    /* 4CFE8 8005C7E8 7F004230 */  andi       $v0, $v0, 0x7F
    /* 4CFEC 8005C7EC 1000A2AF */  sw         $v0, 0x10($sp)
    /* 4CFF0 8005C7F0 0000628C */  lw         $v0, 0x0($v1)
    /* 4CFF4 8005C7F4 C2390600 */  srl        $a3, $a2, 7
    /* 4CFF8 8005C7F8 82140200 */  srl        $v0, $v0, 18
    /* 4CFFC 8005C7FC 7F004230 */  andi       $v0, $v0, 0x7F
    /* 4D000 8005C800 1400A2AF */  sw         $v0, 0x14($sp)
    /* 4D004 8005C804 0F80013C */  lui        $at, %hi(D_800EFB7D)
    /* 4D008 8005C808 21083200 */  addu       $at, $at, $s2
    /* 4D00C 8005C80C 7DFB2290 */  lbu        $v0, %lo(D_800EFB7D)($at)
    /* 4D010 8005C810 7F00C630 */  andi       $a2, $a2, 0x7F
    /* 4D014 8005C814 1800A2AF */  sw         $v0, 0x18($sp)
    /* 4D018 8005C818 0F80013C */  lui        $at, %hi(D_800EFB7C)
    /* 4D01C 8005C81C 21083200 */  addu       $at, $at, $s2
    /* 4D020 8005C820 7CFB2290 */  lbu        $v0, %lo(D_800EFB7C)($at)
    /* 4D024 8005C824 0F00E730 */  andi       $a3, $a3, 0xF
    /* 4D028 8005C828 9016020C */  jal        func_80085A40
    /* 4D02C 8005C82C 1C00A2AF */   sw        $v0, 0x1C($sp)
    /* 4D030 8005C830 14720108 */  j          .L8005C850
    /* 4D034 8005C834 00141300 */   sll       $v0, $s3, 16
  .L8005C838:
    /* 4D038 8005C838 21804000 */  addu       $s0, $v0, $zero
    /* 4D03C 8005C83C 00140200 */  sll        $v0, $v0, 16
    /* 4D040 8005C840 03140200 */  sra        $v0, $v0, 16
    /* 4D044 8005C844 18004228 */  slti       $v0, $v0, 0x18
    /* 4D048 8005C848 C8FF4014 */  bnez       $v0, .L8005C76C
    /* 4D04C 8005C84C 00141300 */   sll       $v0, $s3, 16
  .L8005C850:
    /* 4D050 8005C850 43130200 */  sra        $v0, $v0, 13
  .L8005C854:
    /* 4D054 8005C854 0F80013C */  lui        $at, %hi(D_800EFB78)
    /* 4D058 8005C858 21082200 */  addu       $at, $at, $v0
    /* 4D05C 8005C85C 78FB20AC */  sw         $zero, %lo(D_800EFB78)($at)
    /* 4D060 8005C860 01006226 */  addiu      $v0, $s3, 0x1
    /* 4D064 8005C864 21984000 */  addu       $s3, $v0, $zero
    /* 4D068 8005C868 00140200 */  sll        $v0, $v0, 16
    /* 4D06C 8005C86C 03140200 */  sra        $v0, $v0, 16
    /* 4D070 8005C870 18004228 */  slti       $v0, $v0, 0x18
    /* 4D074 8005C874 A7FF4014 */  bnez       $v0, .L8005C714
    /* 4D078 8005C878 00141300 */   sll       $v0, $s3, 16
    /* 4D07C 8005C87C 5400BF8F */  lw         $ra, 0x54($sp)
    /* 4D080 8005C880 5000B68F */  lw         $s6, 0x50($sp)
    /* 4D084 8005C884 4C00B58F */  lw         $s5, 0x4C($sp)
    /* 4D088 8005C888 4800B48F */  lw         $s4, 0x48($sp)
    /* 4D08C 8005C88C 4400B38F */  lw         $s3, 0x44($sp)
    /* 4D090 8005C890 4000B28F */  lw         $s2, 0x40($sp)
    /* 4D094 8005C894 3C00B18F */  lw         $s1, 0x3C($sp)
    /* 4D098 8005C898 3800B08F */  lw         $s0, 0x38($sp)
    /* 4D09C 8005C89C 5800BD27 */  addiu      $sp, $sp, 0x58
    /* 4D0A0 8005C8A0 0800E003 */  jr         $ra
    /* 4D0A4 8005C8A4 00000000 */   nop
endlabel func_8005C6D0
