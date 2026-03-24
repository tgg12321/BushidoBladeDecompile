glabel func_80069E18
    /* 5A618 80069E18 A8FFBD27 */  addiu      $sp, $sp, -0x58
    /* 5A61C 80069E1C 4C00B1AF */  sw         $s1, 0x4C($sp)
    /* 5A620 80069E20 21888000 */  addu       $s1, $a0, $zero
    /* 5A624 80069E24 5000BFAF */  sw         $ra, 0x50($sp)
    /* 5A628 80069E28 4800B0AF */  sw         $s0, 0x48($sp)
    /* 5A62C 80069E2C 1800308E */  lw         $s0, 0x18($s1)
    /* 5A630 80069E30 B4EA010C */  jal        func_8007AAD0
    /* 5A634 80069E34 21200002 */   addu      $a0, $s0, $zero
    /* 5A638 80069E38 21200002 */  addu       $a0, $s0, $zero
    /* 5A63C 80069E3C 21280000 */  addu       $a1, $zero, $zero
    /* 5A640 80069E40 FF000224 */  addiu      $v0, $zero, 0xFF
    /* 5A644 80069E44 040002A2 */  sb         $v0, 0x4($s0)
    /* 5A648 80069E48 050002A2 */  sb         $v0, 0x5($s0)
    /* 5A64C 80069E4C 060002A2 */  sb         $v0, 0x6($s0)
    /* 5A650 80069E50 80020224 */  addiu      $v0, $zero, 0x280
    /* 5A654 80069E54 0C0002A6 */  sh         $v0, 0xC($s0)
    /* 5A658 80069E58 F0000224 */  addiu      $v0, $zero, 0xF0
    /* 5A65C 80069E5C 080000A6 */  sh         $zero, 0x8($s0)
    /* 5A660 80069E60 0A0000A6 */  sh         $zero, 0xA($s0)
    /* 5A664 80069E64 5AEA010C */  jal        func_8007A968
    /* 5A668 80069E68 0E0002A6 */   sh        $v0, 0xE($s0)
    /* 5A66C 80069E6C 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5A670 80069E70 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5A674 80069E74 21280002 */  addu       $a1, $s0, $zero
    /* 5A678 80069E78 2DEA010C */  jal        func_8007A8B4
    /* 5A67C 80069E7C 50008424 */   addiu     $a0, $a0, 0x50
    /* 5A680 80069E80 0400228E */  lw         $v0, 0x4($s1)
    /* 5A684 80069E84 10001026 */  addiu      $s0, $s0, 0x10
    /* 5A688 80069E88 180030AE */  sw         $s0, 0x18($s1)
    /* 5A68C 80069E8C 1400508C */  lw         $s0, 0x14($v0)
    /* 5A690 80069E90 10000224 */  addiu      $v0, $zero, 0x10
    /* 5A694 80069E94 2C00A2AF */  sw         $v0, 0x2C($sp)
    /* 5A698 80069E98 2800A0AF */  sw         $zero, 0x28($sp)
    /* 5A69C 80069E9C 3000A0AF */  sw         $zero, 0x30($sp)
    /* 5A6A0 80069EA0 3400A0AF */  sw         $zero, 0x34($sp)
    /* 5A6A4 80069EA4 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 5A6A8 80069EA8 0000048E */  lw         $a0, 0x0($s0)
    /* 5A6AC 80069EAC 21280000 */  addu       $a1, $zero, $zero
    /* 5A6B0 80069EB0 20B9010C */  jal        func_8006E480
    /* 5A6B4 80069EB4 1800A4AF */   sw        $a0, 0x18($sp)
    /* 5A6B8 80069EB8 01000524 */  addiu      $a1, $zero, 0x1
    /* 5A6BC 80069EBC 21300000 */  addu       $a2, $zero, $zero
    /* 5A6C0 80069EC0 1000A0AF */  sw         $zero, 0x10($sp)
    /* 5A6C4 80069EC4 1C00248E */  lw         $a0, 0x1C($s1)
    /* 5A6C8 80069EC8 92F0010C */  jal        func_8007C248
    /* 5A6CC 80069ECC 21384000 */   addu      $a3, $v0, $zero
    /* 5A6D0 80069ED0 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5A6D4 80069ED4 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5A6D8 80069ED8 1C00258E */  lw         $a1, 0x1C($s1)
    /* 5A6DC 80069EDC 2DEA010C */  jal        func_8007A8B4
    /* 5A6E0 80069EE0 44008424 */   addiu     $a0, $a0, 0x44
    /* 5A6E4 80069EE4 1C00228E */  lw         $v0, 0x1C($s1)
    /* 5A6E8 80069EE8 00000000 */  nop
    /* 5A6EC 80069EEC 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5A6F0 80069EF0 1C0022AE */  sw         $v0, 0x1C($s1)
    /* 5A6F4 80069EF4 1800A28F */  lw         $v0, 0x18($sp)
    /* 5A6F8 80069EF8 00000000 */  nop
    /* 5A6FC 80069EFC 0C004324 */  addiu      $v1, $v0, 0xC
    /* 5A700 80069F00 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 5A704 80069F04 1400228E */  lw         $v0, 0x14($s1)
    /* 5A708 80069F08 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5A70C 80069F0C 4BCD010C */  jal        func_8007352C
    /* 5A710 80069F10 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5A714 80069F14 140022AE */  sw         $v0, 0x14($s1)
    /* 5A718 80069F18 0400028E */  lw         $v0, 0x4($s0)
    /* 5A71C 80069F1C 00000000 */  nop
    /* 5A720 80069F20 0C004324 */  addiu      $v1, $v0, 0xC
    /* 5A724 80069F24 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5A728 80069F28 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 5A72C 80069F2C 1400228E */  lw         $v0, 0x14($s1)
    /* 5A730 80069F30 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5A734 80069F34 4BCD010C */  jal        func_8007352C
    /* 5A738 80069F38 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5A73C 80069F3C 140022AE */  sw         $v0, 0x14($s1)
    /* 5A740 80069F40 0800028E */  lw         $v0, 0x8($s0)
    /* 5A744 80069F44 00000000 */  nop
    /* 5A748 80069F48 0C004324 */  addiu      $v1, $v0, 0xC
    /* 5A74C 80069F4C 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5A750 80069F50 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 5A754 80069F54 1400228E */  lw         $v0, 0x14($s1)
    /* 5A758 80069F58 1800A427 */  addiu      $a0, $sp, 0x18
    /* 5A75C 80069F5C 4BCD010C */  jal        func_8007352C
    /* 5A760 80069F60 2000A2AF */   sw        $v0, 0x20($sp)
    /* 5A764 80069F64 140022AE */  sw         $v0, 0x14($s1)
    /* 5A768 80069F68 5000BF8F */  lw         $ra, 0x50($sp)
    /* 5A76C 80069F6C 4C00B18F */  lw         $s1, 0x4C($sp)
    /* 5A770 80069F70 4800B08F */  lw         $s0, 0x48($sp)
    /* 5A774 80069F74 5800BD27 */  addiu      $sp, $sp, 0x58
    /* 5A778 80069F78 0800E003 */  jr         $ra
    /* 5A77C 80069F7C 00000000 */   nop
endlabel func_80069E18
