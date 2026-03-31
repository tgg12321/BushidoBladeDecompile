glabel func_8003B10C
    /* 2B90C 8003B10C E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 2B910 8003B110 1400B1AF */  sw         $s1, 0x14($sp)
    /* 2B914 8003B114 21888000 */  addu       $s1, $a0, $zero
    /* 2B918 8003B118 1800B2AF */  sw         $s2, 0x18($sp)
    /* 2B91C 8003B11C 1980123C */  lui        $s2, (0x80190800 >> 16)
    /* 2B920 8003B120 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* 2B924 8003B124 1A5A000C */  jal        gpu_EnableDisplay
    /* 2B928 8003B128 1000B0AF */   sw        $s0, 0x10($sp)
    /* 2B92C 8003B12C 4E83000C */  jal        func_80020D38
    /* 2B930 8003B130 00085236 */   ori       $s2, $s2, (0x80190800 & 0xFFFF)
    /* 2B934 8003B134 E724010C */  jal        func_8004939C
    /* 2B938 8003B138 00000000 */   nop
    /* 2B93C 8003B13C 00111100 */  sll        $v0, $s1, 4
    /* 2B940 8003B140 21105100 */  addu       $v0, $v0, $s1
    /* 2B944 8003B144 80100200 */  sll        $v0, $v0, 2
    /* 2B948 8003B148 21105100 */  addu       $v0, $v0, $s1
    /* 2B94C 8003B14C 80100200 */  sll        $v0, $v0, 2
    /* 2B950 8003B150 23105100 */  subu       $v0, $v0, $s1
    /* 2B954 8003B154 80800200 */  sll        $s0, $v0, 2
    /* 2B958 8003B158 1080013C */  lui        $at, %hi(D_80101EDA)
    /* 2B95C 8003B15C 21083000 */  addu       $at, $at, $s0
    /* 2B960 8003B160 DA1E2484 */  lh         $a0, %lo(D_80101EDA)($at)
    /* 2B964 8003B164 F924010C */  jal        func_800493E4
    /* 2B968 8003B168 00000000 */   nop
    /* 2B96C 8003B16C 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* 2B970 8003B170 DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* 2B974 8003B174 05000224 */  addiu      $v0, $zero, 0x5
    /* 2B978 8003B178 0D006214 */  bne        $v1, $v0, .L8003B1B0
    /* 2B97C 8003B17C 00000000 */   nop
    /* 2B980 8003B180 1080013C */  lui        $at, %hi(D_80101ED2)
    /* 2B984 8003B184 21083000 */  addu       $at, $at, $s0
    /* 2B988 8003B188 D21E2384 */  lh         $v1, %lo(D_80101ED2)($at)
    /* 2B98C 8003B18C 0980043C */  lui        $a0, %hi(D_8008E6A4)
    /* 2B990 8003B190 A4E68424 */  addiu      $a0, $a0, %lo(D_8008E6A4)
    /* 2B994 8003B194 40100300 */  sll        $v0, $v1, 1
    /* 2B998 8003B198 21104300 */  addu       $v0, $v0, $v1
    /* 2B99C 8003B19C 1080013C */  lui        $at, %hi(D_80101ED6)
    /* 2B9A0 8003B1A0 21083000 */  addu       $at, $at, $s0
    /* 2B9A4 8003B1A4 D61E2384 */  lh         $v1, %lo(D_80101ED6)($at)
    /* 2B9A8 8003B1A8 75EC0008 */  j          .L8003B1D4
    /* 2B9AC 8003B1AC 40100200 */   sll       $v0, $v0, 1
  .L8003B1B0:
    /* 2B9B0 8003B1B0 0980043C */  lui        $a0, %hi(D_8008E5CC)
    /* 2B9B4 8003B1B4 CCE58424 */  addiu      $a0, $a0, %lo(D_8008E5CC)
    /* 2B9B8 8003B1B8 1080013C */  lui        $at, %hi(D_80101ED2)
    /* 2B9BC 8003B1BC 21083000 */  addu       $at, $at, $s0
    /* 2B9C0 8003B1C0 D21E2284 */  lh         $v0, %lo(D_80101ED2)($at)
    /* 2B9C4 8003B1C4 1080013C */  lui        $at, %hi(D_80101ED6)
    /* 2B9C8 8003B1C8 21083000 */  addu       $at, $at, $s0
    /* 2B9CC 8003B1CC D61E2384 */  lh         $v1, %lo(D_80101ED6)($at)
    /* 2B9D0 8003B1D0 C0100200 */  sll        $v0, $v0, 3
  .L8003B1D4:
    /* 2B9D4 8003B1D4 21104400 */  addu       $v0, $v0, $a0
    /* 2B9D8 8003B1D8 21104300 */  addu       $v0, $v0, $v1
    /* 2B9DC 8003B1DC 00004590 */  lbu        $a1, 0x0($v0)
    /* 2B9E0 8003B1E0 3525010C */  jal        func_800494D4
    /* 2B9E4 8003B1E4 21202002 */   addu      $a0, $s1, $zero
    /* 2B9E8 8003B1E8 6125010C */  jal        func_80049584
    /* 2B9EC 8003B1EC 21204002 */   addu      $a0, $s2, $zero
    /* 2B9F0 8003B1F0 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* 2B9F4 8003B1F4 1800B28F */  lw         $s2, 0x18($sp)
    /* 2B9F8 8003B1F8 1400B18F */  lw         $s1, 0x14($sp)
    /* 2B9FC 8003B1FC 1000B08F */  lw         $s0, 0x10($sp)
    /* 2BA00 8003B200 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 2BA04 8003B204 0800E003 */  jr         $ra
    /* 2BA08 8003B208 00000000 */   nop
endlabel func_8003B10C
