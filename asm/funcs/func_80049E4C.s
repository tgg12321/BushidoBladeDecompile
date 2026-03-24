glabel func_80049E4C
    /* 3A64C 80049E4C E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 3A650 80049E50 1400B1AF */  sw         $s1, 0x14($sp)
    /* 3A654 80049E54 1080113C */  lui        $s1, %hi(D_80101DF0)
    /* 3A658 80049E58 F01D3126 */  addiu      $s1, $s1, %lo(D_80101DF0)
    /* 3A65C 80049E5C 64000224 */  addiu      $v0, $zero, 0x64
    /* 3A660 80049E60 1800BFAF */  sw         $ra, 0x18($sp)
    /* 3A664 80049E64 1000B0AF */  sw         $s0, 0x10($sp)
    /* 3A668 80049E68 000022A2 */  sb         $v0, 0x0($s1)
    /* 3A66C 80049E6C 05000224 */  addiu      $v0, $zero, 0x5
    /* 3A670 80049E70 1080013C */  lui        $at, %hi(D_80101DF1)
    /* 3A674 80049E74 F11D20A0 */  sb         $zero, %lo(D_80101DF1)($at)
    /* 3A678 80049E78 1080013C */  lui        $at, %hi(D_80101E00)
    /* 3A67C 80049E7C 001E20A4 */  sh         $zero, %lo(D_80101E00)($at)
    /* 3A680 80049E80 1080013C */  lui        $at, %hi(D_80101E02)
    /* 3A684 80049E84 021E20A4 */  sh         $zero, %lo(D_80101E02)($at)
    /* 3A688 80049E88 1080013C */  lui        $at, %hi(D_80101E04)
    /* 3A68C 80049E8C 041E20A4 */  sh         $zero, %lo(D_80101E04)($at)
    /* 3A690 80049E90 1080013C */  lui        $at, %hi(D_80101E3C)
    /* 3A694 80049E94 3C1E20AC */  sw         $zero, %lo(D_80101E3C)($at)
    /* 3A698 80049E98 1080013C */  lui        $at, %hi(D_80101E40)
    /* 3A69C 80049E9C 401E20AC */  sw         $zero, %lo(D_80101E40)($at)
    /* 3A6A0 80049EA0 1080013C */  lui        $at, %hi(D_80101E44)
    /* 3A6A4 80049EA4 441E20AC */  sw         $zero, %lo(D_80101E44)($at)
    /* 3A6A8 80049EA8 1080013C */  lui        $at, %hi(D_80101DFC)
    /* 3A6AC 80049EAC FC1D20AC */  sw         $zero, %lo(D_80101DFC)($at)
    /* 3A6B0 80049EB0 1080013C */  lui        $at, %hi(D_80101DF8)
    /* 3A6B4 80049EB4 F81D22A4 */  sh         $v0, %lo(D_80101DF8)($at)
    /* 3A6B8 80049EB8 3406010C */  jal        func_800418D0
    /* 3A6BC 80049EBC 21202002 */   addu      $a0, $s1, $zero
    /* 3A6C0 80049EC0 1080103C */  lui        $s0, %hi(D_800FF638)
    /* 3A6C4 80049EC4 38F61026 */  addiu      $s0, $s0, %lo(D_800FF638)
    /* 3A6C8 80049EC8 65000224 */  addiu      $v0, $zero, 0x65
    /* 3A6CC 80049ECC 000002A2 */  sb         $v0, 0x0($s0)
    /* 3A6D0 80049ED0 02000224 */  addiu      $v0, $zero, 0x2
    /* 3A6D4 80049ED4 1080013C */  lui        $at, %hi(D_800FF639)
    /* 3A6D8 80049ED8 39F620A0 */  sb         $zero, %lo(D_800FF639)($at)
    /* 3A6DC 80049EDC 1080013C */  lui        $at, %hi(D_800FF648)
    /* 3A6E0 80049EE0 48F620A4 */  sh         $zero, %lo(D_800FF648)($at)
    /* 3A6E4 80049EE4 1080013C */  lui        $at, %hi(D_800FF64A)
    /* 3A6E8 80049EE8 4AF620A4 */  sh         $zero, %lo(D_800FF64A)($at)
    /* 3A6EC 80049EEC 1080013C */  lui        $at, %hi(D_800FF64C)
    /* 3A6F0 80049EF0 4CF620A4 */  sh         $zero, %lo(D_800FF64C)($at)
    /* 3A6F4 80049EF4 1080013C */  lui        $at, %hi(D_800FF684)
    /* 3A6F8 80049EF8 84F620AC */  sw         $zero, %lo(D_800FF684)($at)
    /* 3A6FC 80049EFC 1080013C */  lui        $at, %hi(D_800FF688)
    /* 3A700 80049F00 88F620AC */  sw         $zero, %lo(D_800FF688)($at)
    /* 3A704 80049F04 1080013C */  lui        $at, %hi(D_800FF68C)
    /* 3A708 80049F08 8CF620AC */  sw         $zero, %lo(D_800FF68C)($at)
    /* 3A70C 80049F0C 1080013C */  lui        $at, %hi(D_800FF644)
    /* 3A710 80049F10 44F620AC */  sw         $zero, %lo(D_800FF644)($at)
    /* 3A714 80049F14 1080013C */  lui        $at, %hi(D_800FF640)
    /* 3A718 80049F18 40F622A4 */  sh         $v0, %lo(D_800FF640)($at)
    /* 3A71C 80049F1C 3406010C */  jal        func_800418D0
    /* 3A720 80049F20 21200002 */   addu      $a0, $s0, $zero
    /* 3A724 80049F24 0A80013C */  lui        $at, %hi(D_800A3708)
    /* 3A728 80049F28 083731AC */  sw         $s1, %lo(D_800A3708)($at)
    /* 3A72C 80049F2C 0A80013C */  lui        $at, %hi(D_800A370C)
    /* 3A730 80049F30 0C3730AC */  sw         $s0, %lo(D_800A370C)($at)
    /* 3A734 80049F34 1800BF8F */  lw         $ra, 0x18($sp)
    /* 3A738 80049F38 1400B18F */  lw         $s1, 0x14($sp)
    /* 3A73C 80049F3C 1000B08F */  lw         $s0, 0x10($sp)
    /* 3A740 80049F40 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 3A744 80049F44 0800E003 */  jr         $ra
    /* 3A748 80049F48 00000000 */   nop
endlabel func_80049E4C
