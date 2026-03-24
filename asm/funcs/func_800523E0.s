glabel func_800523E0
    /* 42BE0 800523E0 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 42BE4 800523E4 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 42BE8 800523E8 2800BEAF */  sw         $fp, 0x28($sp)
    /* 42BEC 800523EC 2400B5AF */  sw         $s5, 0x24($sp)
    /* 42BF0 800523F0 2000B4AF */  sw         $s4, 0x20($sp)
    /* 42BF4 800523F4 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 42BF8 800523F8 1800B2AF */  sw         $s2, 0x18($sp)
    /* 42BFC 800523FC 1400B1AF */  sw         $s1, 0x14($sp)
    /* 42C00 80052400 1000B0AF */  sw         $s0, 0x10($sp)
    /* 42C04 80052404 21808000 */  addu       $s0, $a0, $zero
    /* 42C08 80052408 2188A000 */  addu       $s1, $a1, $zero
    /* 42C0C 8005240C 2198C000 */  addu       $s3, $a2, $zero
    /* 42C10 80052410 2190E000 */  addu       $s2, $a3, $zero
    /* 42C14 80052414 00101424 */  addiu      $s4, $zero, 0x1000
    /* 42C18 80052418 22A09302 */  sub        $s4, $s4, $s3 /* handwritten instruction */
    /* 42C1C 8005241C 00000886 */  lh         $t0, 0x0($s0)
    /* 42C20 80052420 02000986 */  lh         $t1, 0x2($s0)
    /* 42C24 80052424 04000A86 */  lh         $t2, 0x4($s0)
    /* 42C28 80052428 00409448 */  mtc2       $s4, $8 /* handwritten instruction */
    /* 42C2C 8005242C 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 42C30 80052430 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 42C34 80052434 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 42C38 80052438 00002B86 */  lh         $t3, 0x0($s1)
    /* 42C3C 8005243C 02002C86 */  lh         $t4, 0x2($s1)
    /* 42C40 80052440 3D00984B */  gpf        1
    /* 42C44 80052444 04002D86 */  lh         $t5, 0x4($s1)
    /* 42C48 80052448 06000486 */  lh         $a0, 0x6($s0)
    /* 42C4C 8005244C 08000586 */  lh         $a1, 0x8($s0)
    /* 42C50 80052450 0A000686 */  lh         $a2, 0xA($s0)
    /* 42C54 80052454 00409348 */  mtc2       $s3, $8 /* handwritten instruction */
    /* 42C58 80052458 00488B48 */  mtc2       $t3, $9 /* handwritten instruction */
    /* 42C5C 8005245C 00508C48 */  mtc2       $t4, $10 /* handwritten instruction */
    /* 42C60 80052460 00588D48 */  mtc2       $t5, $11 /* handwritten instruction */
    /* 42C64 80052464 06003586 */  lh         $s5, 0x6($s1)
    /* 42C68 80052468 08003E86 */  lh         $fp, 0x8($s1)
    /* 42C6C 8005246C 3E00A84B */  gpl        1
    /* 42C70 80052470 0A002786 */  lh         $a3, 0xA($s1)
    /* 42C74 80052474 00000000 */  nop
    /* 42C78 80052478 7649010C */  jal        func_800525D8
    /* 42C7C 8005247C 00000000 */   nop
    /* 42C80 80052480 00409448 */  mtc2       $s4, $8 /* handwritten instruction */
    /* 42C84 80052484 00488448 */  mtc2       $a0, $9 /* handwritten instruction */
    /* 42C88 80052488 00508548 */  mtc2       $a1, $10 /* handwritten instruction */
    /* 42C8C 8005248C 00588648 */  mtc2       $a2, $11 /* handwritten instruction */
    /* 42C90 80052490 21200001 */  addu       $a0, $t0, $zero
    /* 42C94 80052494 21282001 */  addu       $a1, $t1, $zero
    /* 42C98 80052498 3D00984B */  gpf        1
    /* 42C9C 8005249C 21304001 */  addu       $a2, $t2, $zero
    /* 42CA0 800524A0 000044A6 */  sh         $a0, 0x0($s2)
    /* 42CA4 800524A4 020045A6 */  sh         $a1, 0x2($s2)
    /* 42CA8 800524A8 040046A6 */  sh         $a2, 0x4($s2)
    /* 42CAC 800524AC 00409348 */  mtc2       $s3, $8 /* handwritten instruction */
    /* 42CB0 800524B0 00489548 */  mtc2       $s5, $9 /* handwritten instruction */
    /* 42CB4 800524B4 00509E48 */  mtc2       $fp, $10 /* handwritten instruction */
    /* 42CB8 800524B8 00588748 */  mtc2       $a3, $11 /* handwritten instruction */
    /* 42CBC 800524BC 00000000 */  nop
    /* 42CC0 800524C0 00000000 */  nop
    /* 42CC4 800524C4 3E00A84B */  gpl        1
    /* 42CC8 800524C8 00000000 */  nop
    /* 42CCC 800524CC 00000000 */  nop
    /* 42CD0 800524D0 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 42CD4 800524D4 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 42CD8 800524D8 00580A48 */  mfc2       $t2, $11 /* handwritten instruction */
    /* 42CDC 800524DC 18008800 */  mult       $a0, $t0
    /* 42CE0 800524E0 12580000 */  mflo       $t3
    /* 42CE4 800524E4 00000000 */  nop
    /* 42CE8 800524E8 00000000 */  nop
    /* 42CEC 800524EC 1800A900 */  mult       $a1, $t1
    /* 42CF0 800524F0 12600000 */  mflo       $t4
    /* 42CF4 800524F4 00000000 */  nop
    /* 42CF8 800524F8 00000000 */  nop
    /* 42CFC 800524FC 1800CA00 */  mult       $a2, $t2
    /* 42D00 80052500 12680000 */  mflo       $t5
    /* 42D04 80052504 00000000 */  nop
    /* 42D08 80052508 20586C01 */  add        $t3, $t3, $t4 /* handwritten instruction */
    /* 42D0C 8005250C 20586D01 */  add        $t3, $t3, $t5 /* handwritten instruction */
    /* 42D10 80052510 035B0B00 */  sra        $t3, $t3, 12
    /* 42D14 80052514 22580B00 */  neg        $t3, $t3 /* handwritten instruction */
    /* 42D18 80052518 00408B48 */  mtc2       $t3, $8 /* handwritten instruction */
    /* 42D1C 8005251C 00488448 */  mtc2       $a0, $9 /* handwritten instruction */
    /* 42D20 80052520 00508548 */  mtc2       $a1, $10 /* handwritten instruction */
    /* 42D24 80052524 00588648 */  mtc2       $a2, $11 /* handwritten instruction */
    /* 42D28 80052528 00C88848 */  mtc2       $t0, $25 /* handwritten instruction */
    /* 42D2C 8005252C 00D08948 */  mtc2       $t1, $26 /* handwritten instruction */
    /* 42D30 80052530 00D88A48 */  mtc2       $t2, $27 /* handwritten instruction */
    /* 42D34 80052534 00000000 */  nop
    /* 42D38 80052538 00000000 */  nop
    /* 42D3C 8005253C 3E00A84B */  gpl        1
    /* 42D40 80052540 00000000 */  nop
    /* 42D44 80052544 00000000 */  nop
    /* 42D48 80052548 7649010C */  jal        func_800525D8
    /* 42D4C 8005254C 00000000 */   nop
    /* 42D50 80052550 00004B48 */  cfc2       $t3, $0 /* handwritten instruction */
    /* 42D54 80052554 00104C48 */  cfc2       $t4, $2 /* handwritten instruction */
    /* 42D58 80052558 00204D48 */  cfc2       $t5, $4 /* handwritten instruction */
    /* 42D5C 8005255C 0000C448 */  ctc2       $a0, $0 /* handwritten instruction */
    /* 42D60 80052560 0010C548 */  ctc2       $a1, $2 /* handwritten instruction */
    /* 42D64 80052564 0020C648 */  ctc2       $a2, $4 /* handwritten instruction */
    /* 42D68 80052568 00488848 */  mtc2       $t0, $9 /* handwritten instruction */
    /* 42D6C 8005256C 00508948 */  mtc2       $t1, $10 /* handwritten instruction */
    /* 42D70 80052570 00588A48 */  mtc2       $t2, $11 /* handwritten instruction */
    /* 42D74 80052574 00000000 */  nop
    /* 42D78 80052578 00000000 */  nop
    /* 42D7C 8005257C 0C00784B */  op         1
    /* 42D80 80052580 060048A6 */  sh         $t0, 0x6($s2)
    /* 42D84 80052584 080049A6 */  sh         $t1, 0x8($s2)
    /* 42D88 80052588 0A004AA6 */  sh         $t2, 0xA($s2)
    /* 42D8C 8005258C 00C80848 */  mfc2       $t0, $25 /* handwritten instruction */
    /* 42D90 80052590 00D00948 */  mfc2       $t1, $26 /* handwritten instruction */
    /* 42D94 80052594 00D80A48 */  mfc2       $t2, $27 /* handwritten instruction */
    /* 42D98 80052598 0C0048A6 */  sh         $t0, 0xC($s2)
    /* 42D9C 8005259C 0E0049A6 */  sh         $t1, 0xE($s2)
    /* 42DA0 800525A0 10004AA6 */  sh         $t2, 0x10($s2)
    /* 42DA4 800525A4 0000CB48 */  ctc2       $t3, $0 /* handwritten instruction */
    /* 42DA8 800525A8 0010CC48 */  ctc2       $t4, $2 /* handwritten instruction */
    /* 42DAC 800525AC 0020CD48 */  ctc2       $t5, $4 /* handwritten instruction */
    /* 42DB0 800525B0 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 42DB4 800525B4 2800BE8F */  lw         $fp, 0x28($sp)
    /* 42DB8 800525B8 2400B58F */  lw         $s5, 0x24($sp)
    /* 42DBC 800525BC 2000B48F */  lw         $s4, 0x20($sp)
    /* 42DC0 800525C0 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 42DC4 800525C4 1800B28F */  lw         $s2, 0x18($sp)
    /* 42DC8 800525C8 1400B18F */  lw         $s1, 0x14($sp)
    /* 42DCC 800525CC 1000B08F */  lw         $s0, 0x10($sp)
    /* 42DD0 800525D0 0800E003 */  jr         $ra
    /* 42DD4 800525D4 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_800523E0
