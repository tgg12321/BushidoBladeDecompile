glabel func_8001E6E4
    /* EEE4 8001E6E4 90FFBD27 */  addiu      $sp, $sp, -0x70
    /* EEE8 8001E6E8 6800B2AF */  sw         $s2, 0x68($sp)
    /* EEEC 8001E6EC 0F80123C */  lui        $s2, %hi(D_800F5328)
    /* EEF0 8001E6F0 28535226 */  addiu      $s2, $s2, %lo(D_800F5328)
    /* EEF4 8001E6F4 ABFA8424 */  addiu      $a0, $a0, -0x555
    /* EEF8 8001E6F8 5605842C */  sltiu      $a0, $a0, 0x556
    /* EEFC 8001E6FC 6C00BFAF */  sw         $ra, 0x6C($sp)
    /* EF00 8001E700 6400B1AF */  sw         $s1, 0x64($sp)
    /* EF04 8001E704 03008014 */  bnez       $a0, .L8001E714
    /* EF08 8001E708 6000B0AF */   sw        $s0, 0x60($sp)
    /* EF0C 8001E70C 0F80123C */  lui        $s2, %hi(D_800F6608)
    /* EF10 8001E710 08665226 */  addiu      $s2, $s2, %lo(D_800F6608)
  .L8001E714:
    /* EF14 8001E714 0000428E */  lw         $v0, 0x0($s2)
    /* EF18 8001E718 1080033C */  lui        $v1, %hi(D_800FF5C8)
    /* EF1C 8001E71C C8F5638C */  lw         $v1, %lo(D_800FF5C8)($v1)
    /* EF20 8001E720 00000000 */  nop
    /* EF24 8001E724 21104300 */  addu       $v0, $v0, $v1
    /* EF28 8001E728 1800A2AF */  sw         $v0, 0x18($sp)
    /* EF2C 8001E72C 0400428E */  lw         $v0, 0x4($s2)
    /* EF30 8001E730 1080033C */  lui        $v1, %hi(D_800FF5CC)
    /* EF34 8001E734 CCF5638C */  lw         $v1, %lo(D_800FF5CC)($v1)
    /* EF38 8001E738 00000000 */  nop
    /* EF3C 8001E73C 21104300 */  addu       $v0, $v0, $v1
    /* EF40 8001E740 1C00A2AF */  sw         $v0, 0x1C($sp)
    /* EF44 8001E744 0800428E */  lw         $v0, 0x8($s2)
    /* EF48 8001E748 1080033C */  lui        $v1, %hi(D_800FF5D0)
    /* EF4C 8001E74C D0F5638C */  lw         $v1, %lo(D_800FF5D0)($v1)
    /* EF50 8001E750 00000000 */  nop
    /* EF54 8001E754 21104300 */  addu       $v0, $v0, $v1
    /* EF58 8001E758 2000A2AF */  sw         $v0, 0x20($sp)
    /* EF5C 8001E75C 10004296 */  lhu        $v0, 0x10($s2)
    /* EF60 8001E760 1080033C */  lui        $v1, %hi(D_800FF5D8)
    /* EF64 8001E764 D8F56394 */  lhu        $v1, %lo(D_800FF5D8)($v1)
    /* EF68 8001E768 1800B027 */  addiu      $s0, $sp, 0x18
    /* EF6C 8001E76C 21104300 */  addu       $v0, $v0, $v1
    /* EF70 8001E770 2800A2A7 */  sh         $v0, 0x28($sp)
    /* EF74 8001E774 12004296 */  lhu        $v0, 0x12($s2)
    /* EF78 8001E778 1080033C */  lui        $v1, %hi(D_800FF5DA)
    /* EF7C 8001E77C DAF56394 */  lhu        $v1, %lo(D_800FF5DA)($v1)
    /* EF80 8001E780 21200002 */  addu       $a0, $s0, $zero
    /* EF84 8001E784 21104300 */  addu       $v0, $v0, $v1
    /* EF88 8001E788 2A00A2A7 */  sh         $v0, 0x2A($sp)
    /* EF8C 8001E78C 14004296 */  lhu        $v0, 0x14($s2)
    /* EF90 8001E790 1080033C */  lui        $v1, %hi(D_800FF5DC)
    /* EF94 8001E794 DCF56394 */  lhu        $v1, %lo(D_800FF5DC)($v1)
    /* EF98 8001E798 2800B127 */  addiu      $s1, $sp, 0x28
    /* EF9C 8001E79C 21104300 */  addu       $v0, $v0, $v1
    /* EFA0 8001E7A0 2C00A2A7 */  sh         $v0, 0x2C($sp)
    /* EFA4 8001E7A4 1800468E */  lw         $a2, 0x18($s2)
    /* EFA8 8001E7A8 1080023C */  lui        $v0, %hi(D_800FF5E0)
    /* EFAC 8001E7AC E0F5428C */  lw         $v0, %lo(D_800FF5E0)($v0)
    /* EFB0 8001E7B0 21282002 */  addu       $a1, $s1, $zero
    /* EFB4 8001E7B4 2130C200 */  addu       $a2, $a2, $v0
    /* EFB8 8001E7B8 FD1A010C */  jal        func_80046BF4
    /* EFBC 8001E7BC 3000A6AF */   sw        $a2, 0x30($sp)
    /* EFC0 8001E7C0 21200002 */  addu       $a0, $s0, $zero
    /* EFC4 8001E7C4 20005026 */  addiu      $s0, $s2, 0x20
    /* EFC8 8001E7C8 4E69000C */  jal        func_8001A538
    /* EFCC 8001E7CC 21280002 */   addu      $a1, $s0, $zero
    /* EFD0 8001E7D0 21202002 */  addu       $a0, $s1, $zero
    /* EFD4 8001E7D4 1984010C */  jal        func_80061064
    /* EFD8 8001E7D8 21280002 */   addu      $a1, $s0, $zero
    /* EFDC 8001E7DC 0A80013C */  lui        $at, %hi(D_800A36B4)
    /* EFE0 8001E7E0 B43632AC */  sw         $s2, %lo(D_800A36B4)($at)
    /* EFE4 8001E7E4 6C00BF8F */  lw         $ra, 0x6C($sp)
    /* EFE8 8001E7E8 6800B28F */  lw         $s2, 0x68($sp)
    /* EFEC 8001E7EC 6400B18F */  lw         $s1, 0x64($sp)
    /* EFF0 8001E7F0 6000B08F */  lw         $s0, 0x60($sp)
    /* EFF4 8001E7F4 7000BD27 */  addiu      $sp, $sp, 0x70
    /* EFF8 8001E7F8 0800E003 */  jr         $ra
    /* EFFC 8001E7FC 00000000 */   nop
endlabel func_8001E6E4
