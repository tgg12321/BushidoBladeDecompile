glabel func_8001E404
    /* EC04 8001E404 0A80023C */  lui        $v0, %hi(D_800A38BA)
    /* EC08 8001E408 BA384284 */  lh         $v0, %lo(D_800A38BA)($v0)
    /* EC0C 8001E40C 90FFBD27 */  addiu      $sp, $sp, -0x70
    /* EC10 8001E410 6C00BFAF */  sw         $ra, 0x6C($sp)
    /* EC14 8001E414 6800B2AF */  sw         $s2, 0x68($sp)
    /* EC18 8001E418 6400B1AF */  sw         $s1, 0x64($sp)
    /* EC1C 8001E41C 55004010 */  beqz       $v0, .L8001E574
    /* EC20 8001E420 6000B0AF */   sw        $s0, 0x60($sp)
    /* EC24 8001E424 0A80033C */  lui        $v1, %hi(D_800A36FA)
    /* EC28 8001E428 FA366390 */  lbu        $v1, %lo(D_800A36FA)($v1)
    /* EC2C 8001E42C 01000224 */  addiu      $v0, $zero, 0x1
    /* EC30 8001E430 10006214 */  bne        $v1, $v0, .L8001E474
    /* EC34 8001E434 02000224 */   addiu     $v0, $zero, 0x2
    /* EC38 8001E438 1080023C */  lui        $v0, %hi(D_80101F5E)
    /* EC3C 8001E43C 5E1F4284 */  lh         $v0, %lo(D_80101F5E)($v0)
    /* EC40 8001E440 00000000 */  nop
    /* EC44 8001E444 06004014 */  bnez       $v0, .L8001E460
    /* EC48 8001E448 02000224 */   addiu     $v0, $zero, 0x2
    /* EC4C 8001E44C 1080023C */  lui        $v0, %hi(D_801023AA)
    /* EC50 8001E450 AA234284 */  lh         $v0, %lo(D_801023AA)($v0)
    /* EC54 8001E454 00000000 */  nop
    /* EC58 8001E458 05004010 */  beqz       $v0, .L8001E470
    /* EC5C 8001E45C 02000224 */   addiu     $v0, $zero, 0x2
  .L8001E460:
    /* EC60 8001E460 0A80013C */  lui        $at, %hi(D_800A36FA)
    /* EC64 8001E464 FA3622A0 */  sb         $v0, %lo(D_800A36FA)($at)
    /* EC68 8001E468 0A80033C */  lui        $v1, %hi(D_800A36FA)
    /* EC6C 8001E46C FA366390 */  lbu        $v1, %lo(D_800A36FA)($v1)
  .L8001E470:
    /* EC70 8001E470 02000224 */  addiu      $v0, $zero, 0x2
  .L8001E474:
    /* EC74 8001E474 18006210 */  beq        $v1, $v0, .L8001E4D8
    /* EC78 8001E478 11000324 */   addiu     $v1, $zero, 0x11
    /* EC7C 8001E47C 1080023C */  lui        $v0, %hi(D_80101F32)
    /* EC80 8001E480 321F4294 */  lhu        $v0, %lo(D_80101F32)($v0)
    /* EC84 8001E484 00000000 */  nop
    /* EC88 8001E488 06004310 */  beq        $v0, $v1, .L8001E4A4
    /* EC8C 8001E48C 00000000 */   nop
    /* EC90 8001E490 1080023C */  lui        $v0, %hi(D_8010237E)
    /* EC94 8001E494 7E234294 */  lhu        $v0, %lo(D_8010237E)($v0)
    /* EC98 8001E498 00000000 */  nop
    /* EC9C 8001E49C 08004314 */  bne        $v0, $v1, .L8001E4C0
    /* ECA0 8001E4A0 00000000 */   nop
  .L8001E4A4:
    /* ECA4 8001E4A4 0F80123C */  lui        $s2, %hi(D_800F6608)
    /* ECA8 8001E4A8 08665226 */  addiu      $s2, $s2, %lo(D_800F6608)
    /* ECAC 8001E4AC 01000224 */  addiu      $v0, $zero, 0x1
    /* ECB0 8001E4B0 0A80013C */  lui        $at, %hi(D_800A36FA)
    /* ECB4 8001E4B4 FA3622A0 */  sb         $v0, %lo(D_800A36FA)($at)
    /* ECB8 8001E4B8 38790008 */  j          .L8001E4E0
    /* ECBC 8001E4BC 00000000 */   nop
  .L8001E4C0:
    /* ECC0 8001E4C0 0F80123C */  lui        $s2, %hi(D_800F5328)
    /* ECC4 8001E4C4 28535226 */  addiu      $s2, $s2, %lo(D_800F5328)
    /* ECC8 8001E4C8 0A80013C */  lui        $at, %hi(D_800A36FA)
    /* ECCC 8001E4CC FA3620A0 */  sb         $zero, %lo(D_800A36FA)($at)
    /* ECD0 8001E4D0 38790008 */  j          .L8001E4E0
    /* ECD4 8001E4D4 00000000 */   nop
  .L8001E4D8:
    /* ECD8 8001E4D8 0F80123C */  lui        $s2, %hi(D_800F6608)
    /* ECDC 8001E4DC 08665226 */  addiu      $s2, $s2, %lo(D_800F6608)
  .L8001E4E0:
    /* ECE0 8001E4E0 0A80043C */  lui        $a0, %hi(D_800A36FA)
    /* ECE4 8001E4E4 FA368490 */  lbu        $a0, %lo(D_800A36FA)($a0)
    /* ECE8 8001E4E8 86FC000C */  jal        func_8003F218
    /* ECEC 8001E4EC 0100842C */   sltiu     $a0, $a0, 0x1
    /* ECF0 8001E4F0 0A80023C */  lui        $v0, %hi(D_800A36FA)
    /* ECF4 8001E4F4 FA364290 */  lbu        $v0, %lo(D_800A36FA)($v0)
    /* ECF8 8001E4F8 00000000 */  nop
    /* ECFC 8001E4FC 02004014 */  bnez       $v0, .L8001E508
    /* ED00 8001E500 2D000424 */   addiu     $a0, $zero, 0x2D
    /* ED04 8001E504 50000424 */  addiu      $a0, $zero, 0x50
  .L8001E508:
    /* ED08 8001E508 B159000C */  jal        func_800166C4
    /* ED0C 8001E50C 00000000 */   nop
    /* ED10 8001E510 FFFB010C */  jal        func_8007EFFC
    /* ED14 8001E514 21204000 */   addu      $a0, $v0, $zero
    /* ED18 8001E518 0A80023C */  lui        $v0, %hi(D_800A36FA)
    /* ED1C 8001E51C FA364290 */  lbu        $v0, %lo(D_800A36FA)($v0)
    /* ED20 8001E520 00000000 */  nop
    /* ED24 8001E524 0C004014 */  bnez       $v0, .L8001E558
    /* ED28 8001E528 21200000 */   addu      $a0, $zero, $zero
    /* ED2C 8001E52C 0A80043C */  lui        $a0, %hi(D_800A36F6)
    /* ED30 8001E530 F6368484 */  lh         $a0, %lo(D_800A36F6)($a0)
    /* ED34 8001E534 A205010C */  jal        func_80041688
    /* ED38 8001E538 01000524 */   addiu     $a1, $zero, 0x1
    /* ED3C 8001E53C 0A80043C */  lui        $a0, %hi(D_800A36F6)
    /* ED40 8001E540 F6368484 */  lh         $a0, %lo(D_800A36F6)($a0)
    /* ED44 8001E544 21280000 */  addu       $a1, $zero, $zero
    /* ED48 8001E548 A205010C */  jal        func_80041688
    /* ED4C 8001E54C 0100842C */   sltiu     $a0, $a0, 0x1
    /* ED50 8001E550 5F790008 */  j          .L8001E57C
    /* ED54 8001E554 00000000 */   nop
  .L8001E558:
    /* ED58 8001E558 A205010C */  jal        func_80041688
    /* ED5C 8001E55C 21280000 */   addu      $a1, $zero, $zero
    /* ED60 8001E560 01000424 */  addiu      $a0, $zero, 0x1
    /* ED64 8001E564 A205010C */  jal        func_80041688
    /* ED68 8001E568 21280000 */   addu      $a1, $zero, $zero
    /* ED6C 8001E56C 5F790008 */  j          .L8001E57C
    /* ED70 8001E570 00000000 */   nop
  .L8001E574:
    /* ED74 8001E574 0F80123C */  lui        $s2, %hi(D_800F6608)
    /* ED78 8001E578 08665226 */  addiu      $s2, $s2, %lo(D_800F6608)
  .L8001E57C:
    /* ED7C 8001E57C 0A80033C */  lui        $v1, %hi(D_800A3834)
    /* ED80 8001E580 34386384 */  lh         $v1, %lo(D_800A3834)($v1)
    /* ED84 8001E584 01000224 */  addiu      $v0, $zero, 0x1
    /* ED88 8001E588 2C006214 */  bne        $v1, $v0, .L8001E63C
    /* ED8C 8001E58C 1800A727 */   addiu     $a3, $sp, 0x18
    /* ED90 8001E590 0000428E */  lw         $v0, 0x0($s2)
    /* ED94 8001E594 1080033C */  lui        $v1, %hi(D_800FF5C8)
    /* ED98 8001E598 C8F5638C */  lw         $v1, %lo(D_800FF5C8)($v1)
    /* ED9C 8001E59C 00000000 */  nop
    /* EDA0 8001E5A0 21104300 */  addu       $v0, $v0, $v1
    /* EDA4 8001E5A4 1800A2AF */  sw         $v0, 0x18($sp)
    /* EDA8 8001E5A8 0400428E */  lw         $v0, 0x4($s2)
    /* EDAC 8001E5AC 1080033C */  lui        $v1, %hi(D_800FF5CC)
    /* EDB0 8001E5B0 CCF5638C */  lw         $v1, %lo(D_800FF5CC)($v1)
    /* EDB4 8001E5B4 00000000 */  nop
    /* EDB8 8001E5B8 21104300 */  addu       $v0, $v0, $v1
    /* EDBC 8001E5BC 1C00A2AF */  sw         $v0, 0x1C($sp)
    /* EDC0 8001E5C0 0800428E */  lw         $v0, 0x8($s2)
    /* EDC4 8001E5C4 1080033C */  lui        $v1, %hi(D_800FF5D0)
    /* EDC8 8001E5C8 D0F5638C */  lw         $v1, %lo(D_800FF5D0)($v1)
    /* EDCC 8001E5CC 00000000 */  nop
    /* EDD0 8001E5D0 21104300 */  addu       $v0, $v0, $v1
    /* EDD4 8001E5D4 2000A2AF */  sw         $v0, 0x20($sp)
    /* EDD8 8001E5D8 10004296 */  lhu        $v0, 0x10($s2)
    /* EDDC 8001E5DC 1080033C */  lui        $v1, %hi(D_800FF5D8)
    /* EDE0 8001E5E0 D8F56394 */  lhu        $v1, %lo(D_800FF5D8)($v1)
    /* EDE4 8001E5E4 00000000 */  nop
    /* EDE8 8001E5E8 21104300 */  addu       $v0, $v0, $v1
    /* EDEC 8001E5EC 2800A2A7 */  sh         $v0, 0x28($sp)
    /* EDF0 8001E5F0 12004296 */  lhu        $v0, 0x12($s2)
    /* EDF4 8001E5F4 1080033C */  lui        $v1, %hi(D_800FF5DA)
    /* EDF8 8001E5F8 DAF56394 */  lhu        $v1, %lo(D_800FF5DA)($v1)
    /* EDFC 8001E5FC 00000000 */  nop
    /* EE00 8001E600 21104300 */  addu       $v0, $v0, $v1
    /* EE04 8001E604 2A00A2A7 */  sh         $v0, 0x2A($sp)
    /* EE08 8001E608 14004296 */  lhu        $v0, 0x14($s2)
    /* EE0C 8001E60C 1080033C */  lui        $v1, %hi(D_800FF5DC)
    /* EE10 8001E610 DCF56394 */  lhu        $v1, %lo(D_800FF5DC)($v1)
    /* EE14 8001E614 00000000 */  nop
    /* EE18 8001E618 21104300 */  addu       $v0, $v0, $v1
    /* EE1C 8001E61C 2C00A2A7 */  sh         $v0, 0x2C($sp)
    /* EE20 8001E620 1800428E */  lw         $v0, 0x18($s2)
    /* EE24 8001E624 1080033C */  lui        $v1, %hi(D_800FF5E0)
    /* EE28 8001E628 E0F5638C */  lw         $v1, %lo(D_800FF5E0)($v1)
    /* EE2C 8001E62C 00000000 */  nop
    /* EE30 8001E630 21104300 */  addu       $v0, $v0, $v1
    /* EE34 8001E634 9F790008 */  j          .L8001E67C
    /* EE38 8001E638 3000A2AF */   sw        $v0, 0x30($sp)
  .L8001E63C:
    /* EE3C 8001E63C 21304002 */  addu       $a2, $s2, $zero
    /* EE40 8001E640 40004826 */  addiu      $t0, $s2, 0x40
  .L8001E644:
    /* EE44 8001E644 0000C28C */  lw         $v0, 0x0($a2)
    /* EE48 8001E648 0400C38C */  lw         $v1, 0x4($a2)
    /* EE4C 8001E64C 0800C48C */  lw         $a0, 0x8($a2)
    /* EE50 8001E650 0C00C58C */  lw         $a1, 0xC($a2)
    /* EE54 8001E654 0000E2AC */  sw         $v0, 0x0($a3)
    /* EE58 8001E658 0400E3AC */  sw         $v1, 0x4($a3)
    /* EE5C 8001E65C 0800E4AC */  sw         $a0, 0x8($a3)
    /* EE60 8001E660 0C00E5AC */  sw         $a1, 0xC($a3)
    /* EE64 8001E664 1000C624 */  addiu      $a2, $a2, 0x10
    /* EE68 8001E668 F6FFC814 */  bne        $a2, $t0, .L8001E644
    /* EE6C 8001E66C 1000E724 */   addiu     $a3, $a3, 0x10
    /* EE70 8001E670 0000C28C */  lw         $v0, 0x0($a2)
    /* EE74 8001E674 00000000 */  nop
    /* EE78 8001E678 0000E2AC */  sw         $v0, 0x0($a3)
  .L8001E67C:
    /* EE7C 8001E67C 1800B027 */  addiu      $s0, $sp, 0x18
    /* EE80 8001E680 21200002 */  addu       $a0, $s0, $zero
    /* EE84 8001E684 2800B127 */  addiu      $s1, $sp, 0x28
    /* EE88 8001E688 3000A68F */  lw         $a2, 0x30($sp)
    /* EE8C 8001E68C FD1A010C */  jal        func_80046BF4
    /* EE90 8001E690 21282002 */   addu      $a1, $s1, $zero
    /* EE94 8001E694 21200002 */  addu       $a0, $s0, $zero
    /* EE98 8001E698 20005026 */  addiu      $s0, $s2, 0x20
    /* EE9C 8001E69C 4E69000C */  jal        func_8001A538
    /* EEA0 8001E6A0 21280002 */   addu      $a1, $s0, $zero
    /* EEA4 8001E6A4 21202002 */  addu       $a0, $s1, $zero
    /* EEA8 8001E6A8 1984010C */  jal        func_80061064
    /* EEAC 8001E6AC 21280002 */   addu      $a1, $s0, $zero
    /* EEB0 8001E6B0 F5FC000C */  jal        func_8003F3D4
    /* EEB4 8001E6B4 30004426 */   addiu     $a0, $s2, 0x30
    /* EEB8 8001E6B8 F5FC000C */  jal        func_8003F3D4
    /* EEBC 8001E6BC 38004426 */   addiu     $a0, $s2, 0x38
    /* EEC0 8001E6C0 0A80013C */  lui        $at, %hi(D_800A36B4)
    /* EEC4 8001E6C4 B43632AC */  sw         $s2, %lo(D_800A36B4)($at)
    /* EEC8 8001E6C8 6C00BF8F */  lw         $ra, 0x6C($sp)
    /* EECC 8001E6CC 6800B28F */  lw         $s2, 0x68($sp)
    /* EED0 8001E6D0 6400B18F */  lw         $s1, 0x64($sp)
    /* EED4 8001E6D4 6000B08F */  lw         $s0, 0x60($sp)
    /* EED8 8001E6D8 7000BD27 */  addiu      $sp, $sp, 0x70
    /* EEDC 8001E6DC 0800E003 */  jr         $ra
    /* EEE0 8001E6E0 00000000 */   nop
endlabel func_8001E404
