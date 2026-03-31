glabel func_8001BBD8
    /* C3D8 8001BBD8 D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* C3DC 8001BBDC 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* C3E0 8001BBE0 21888000 */  addu       $s1, $a0, $zero
    /* C3E4 8001BBE4 2000B2AF */  sw         $s2, 0x20($sp)
    /* C3E8 8001BBE8 2190A000 */  addu       $s2, $a1, $zero
    /* C3EC 8001BBEC 2400B3AF */  sw         $s3, 0x24($sp)
    /* C3F0 8001BBF0 2198C000 */  addu       $s3, $a2, $zero
    /* C3F4 8001BBF4 2800BFAF */  sw         $ra, 0x28($sp)
    /* C3F8 8001BBF8 1800B0AF */  sw         $s0, 0x18($sp)
    /* C3FC 8001BBFC 04004286 */  lh         $v0, 0x4($s2)
    /* C400 8001BC00 04002486 */  lh         $a0, 0x4($s1)
    /* C404 8001BC04 08002586 */  lh         $a1, 0x8($s1)
    /* C408 8001BC08 0A80103C */  lui        $s0, %hi(D_800A387C)
    /* C40C 8001BC0C 7C38108E */  lw         $s0, %lo(D_800A387C)($s0)
    /* C410 8001BC10 23204400 */  subu       $a0, $v0, $a0
    /* C414 8001BC14 1127102A */  slti       $s0, $s0, 0x2711
    /* C418 8001BC18 08004286 */  lh         $v0, 0x8($s2)
    /* C41C 8001BC1C C0821000 */  sll        $s0, $s0, 11
    /* C420 8001BC20 57FF010C */  jal        func_8007FD5C
    /* C424 8001BC24 23284500 */   subu      $a1, $v0, $a1
    /* C428 8001BC28 0F80043C */  lui        $a0, %hi(D_800F5328)
    /* C42C 8001BC2C 28538424 */  addiu      $a0, $a0, %lo(D_800F5328)
    /* C430 8001BC30 21282002 */  addu       $a1, $s1, $zero
    /* C434 8001BC34 00FE0324 */  addiu      $v1, $zero, -0x200
    /* C438 8001BC38 23186200 */  subu       $v1, $v1, $v0
    /* C43C 8001BC3C 21304002 */  addu       $a2, $s2, $zero
    /* C440 8001BC40 21386002 */  addu       $a3, $s3, $zero
    /* C444 8001BC44 1000B0AF */  sw         $s0, 0x10($sp)
    /* C448 8001BC48 D26D000C */  jal        DispPracticeMenuTex_A
    /* C44C 8001BC4C 1400A3AF */   sw        $v1, 0x14($sp)
    /* C450 8001BC50 2800BF8F */  lw         $ra, 0x28($sp)
    /* C454 8001BC54 2400B38F */  lw         $s3, 0x24($sp)
    /* C458 8001BC58 2000B28F */  lw         $s2, 0x20($sp)
    /* C45C 8001BC5C 1C00B18F */  lw         $s1, 0x1C($sp)
    /* C460 8001BC60 1800B08F */  lw         $s0, 0x18($sp)
    /* C464 8001BC64 3000BD27 */  addiu      $sp, $sp, 0x30
    /* C468 8001BC68 0800E003 */  jr         $ra
    /* C46C 8001BC6C 00000000 */   nop
endlabel func_8001BBD8
