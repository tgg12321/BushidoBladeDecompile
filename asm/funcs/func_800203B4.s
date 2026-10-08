glabel func_800203B4
    /* 10BB4 800203B4 B0FFBD27 */  addiu      $sp, $sp, -0x50
    /* 10BB8 800203B8 4000B0AF */  sw         $s0, 0x40($sp)
    /* 10BBC 800203BC 21808000 */  addu       $s0, $a0, $zero
    /* 10BC0 800203C0 01000224 */  addiu      $v0, $zero, 0x1
    /* 10BC4 800203C4 4800BFAF */  sw         $ra, 0x48($sp)
    /* 10BC8 800203C8 4400B1AF */  sw         $s1, 0x44($sp)
    /* 10BCC 800203CC 500302A6 */  sh         $v0, 0x350($s0)
    /* 10BD0 800203D0 80100500 */  sll        $v0, $a1, 2
    /* 10BD4 800203D4 21104500 */  addu       $v0, $v0, $a1
    /* 10BD8 800203D8 80100200 */  sll        $v0, $v0, 2
    /* 10BDC 800203DC 04000486 */  lh         $a0, 0x4($s0)
    /* 10BE0 800203E0 0980013C */  lui        $at, %hi(D_8008D59E)
    /* 10BE4 800203E4 21082200 */  addu       $at, $at, $v0
    /* 10BE8 800203E8 9ED52294 */  lhu        $v0, %lo(D_8008D59E)($at)
    /* 10BEC 800203EC 2188C000 */  addu       $s1, $a2, $zero
    /* 10BF0 800203F0 7B1B010C */  jal        func_80046DEC
    /* 10BF4 800203F4 520302A6 */   sh        $v0, 0x352($s0)
    /* 10BF8 800203F8 52030386 */  lh         $v1, 0x352($s0)
    /* 10BFC 800203FC 00000000 */  nop
    /* 10C00 80020400 80180300 */  sll        $v1, $v1, 2
    /* 10C04 80020404 21186200 */  addu       $v1, $v1, $v0
    /* 10C08 80020408 0000648C */  lw         $a0, 0x0($v1)
    /* 10C0C 8002040C B3BB000C */  jal        func_8002EECC
    /* 10C10 80020410 1000A527 */   addiu     $a1, $sp, 0x10
    /* 10C14 80020414 1000A227 */  addiu      $v0, $sp, 0x10
    /* 10C18 80020418 21604000 */  addu       $t4, $v0, $zero
    /* 10C1C 8002041C 00008D8D */  lw         $t5, 0x0($t4)
    /* 10C20 80020420 04008E8D */  lw         $t6, 0x4($t4)
    /* 10C24 80020424 0000CD48 */  ctc2       $t5, $0 /* handwritten instruction */
    /* 10C28 80020428 0008CE48 */  ctc2       $t6, $1 /* handwritten instruction */
    /* 10C2C 8002042C 08008D8D */  lw         $t5, 0x8($t4)
    /* 10C30 80020430 0C008E8D */  lw         $t6, 0xC($t4)
    /* 10C34 80020434 10008F8D */  lw         $t7, 0x10($t4)
    /* 10C38 80020438 0010CD48 */  ctc2       $t5, $2 /* handwritten instruction */
    /* 10C3C 8002043C 0018CE48 */  ctc2       $t6, $3 /* handwritten instruction */
    /* 10C40 80020440 0020CF48 */  ctc2       $t7, $4 /* handwritten instruction */
    /* 10C44 80020444 00002286 */  lh         $v0, 0x0($s1)
    /* 10C48 80020448 00000000 */  nop
    /* 10C4C 8002044C 3000A2AF */  sw         $v0, 0x30($sp)
    /* 10C50 80020450 02002286 */  lh         $v0, 0x2($s1)
    /* 10C54 80020454 00000000 */  nop
    /* 10C58 80020458 3400A2AF */  sw         $v0, 0x34($sp)
    /* 10C5C 8002045C 04002286 */  lh         $v0, 0x4($s1)
    /* 10C60 80020460 00000000 */  nop
    /* 10C64 80020464 3800A2AF */  sw         $v0, 0x38($sp)
    /* 10C68 80020468 3000A227 */  addiu      $v0, $sp, 0x30
    /* 10C6C 8002046C 21604000 */  addu       $t4, $v0, $zero
    /* 10C70 80020470 04008E95 */  lhu        $t6, 0x4($t4)
    /* 10C74 80020474 00008D95 */  lhu        $t5, 0x0($t4)
    /* 10C78 80020478 00740E00 */  sll        $t6, $t6, 16
    /* 10C7C 8002047C 2568AE01 */  or         $t5, $t5, $t6
    /* 10C80 80020480 00008D48 */  mtc2       $t5, $0 /* handwritten instruction */
    /* 10C84 80020484 080081C9 */  lwc2       $1, 0x8($t4)
    /* 10C88 80020488 00000000 */  nop
    /* 10C8C 8002048C 00000000 */  nop
    /* 10C90 80020490 1260484A */  mvmva      1, 0, 0, 3, 0
    /* 10C94 80020494 54031026 */  addiu      $s0, $s0, 0x354
    /* 10C98 80020498 21600002 */  addu       $t4, $s0, $zero
    /* 10C9C 8002049C 000099E9 */  swc2       $25, 0x0($t4)
    /* 10CA0 800204A0 04009AE9 */  swc2       $26, 0x4($t4) /* handwritten instruction */
    /* 10CA4 800204A4 08009BE9 */  swc2       $27, 0x8($t4) /* handwritten instruction */
    /* 10CA8 800204A8 4800BF8F */  lw         $ra, 0x48($sp)
    /* 10CAC 800204AC 4400B18F */  lw         $s1, 0x44($sp)
    /* 10CB0 800204B0 4000B08F */  lw         $s0, 0x40($sp)
    /* 10CB4 800204B4 5000BD27 */  addiu      $sp, $sp, 0x50
    /* 10CB8 800204B8 0800E003 */  jr         $ra
    /* 10CBC 800204BC 00000000 */   nop
endlabel func_800203B4
