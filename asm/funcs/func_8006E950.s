glabel func_8006E950
    /* 5F150 8006E950 D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* 5F154 8006E954 1800B0AF */  sw         $s0, 0x18($sp)
    /* 5F158 8006E958 21808000 */  addu       $s0, $a0, $zero
    /* 5F15C 8006E95C 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* 5F160 8006E960 2188A000 */  addu       $s1, $a1, $zero
    /* 5F164 8006E964 2800BFAF */  sw         $ra, 0x28($sp)
    /* 5F168 8006E968 2400B3AF */  sw         $s3, 0x24($sp)
    /* 5F16C 8006E96C D0DB000C */  jal        func_80036F40
    /* 5F170 8006E970 2000B2AF */   sw        $s2, 0x20($sp)
    /* 5F174 8006E974 02000424 */  addiu      $a0, $zero, 0x2
    /* 5F178 8006E978 AADB000C */  jal        func_80036EA8
    /* 5F17C 8006E97C 21280002 */   addu      $a1, $s0, $zero
    /* 5F180 8006E980 21204000 */  addu       $a0, $v0, $zero
    /* 5F184 8006E984 66DB000C */  jal        func_80036D98
    /* 5F188 8006E988 21282002 */   addu      $a1, $s1, $zero
    /* 5F18C 8006E98C D0DB000C */  jal        func_80036F40
    /* 5F190 8006E990 80021224 */   addiu     $s2, $zero, 0x280
    /* 5F194 8006E994 10B9010C */  jal        func_8006E440
    /* 5F198 8006E998 21202002 */   addu      $a0, $s1, $zero
    /* 5F19C 8006E99C 21200000 */  addu       $a0, $zero, $zero
    /* 5F1A0 8006E9A0 80010224 */  addiu      $v0, $zero, 0x180
    /* 5F1A4 8006E9A4 0800338E */  lw         $s3, 0x8($s1)
    /* 5F1A8 8006E9A8 DC011024 */  addiu      $s0, $zero, 0x1DC
    /* 5F1AC 8006E9AC 1000B2A7 */  sh         $s2, 0x10($sp)
    /* 5F1B0 8006E9B0 1200A0A7 */  sh         $zero, 0x12($sp)
    /* 5F1B4 8006E9B4 1400A2A7 */  sh         $v0, 0x14($sp)
    /* 5F1B8 8006E9B8 CFEC010C */  jal        func_8007B33C
    /* 5F1BC 8006E9BC 1600B0A7 */   sh        $s0, 0x16($sp)
    /* 5F1C0 8006E9C0 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5F1C4 8006E9C4 80ED010C */  jal        func_8007B600
    /* 5F1C8 8006E9C8 21286002 */   addu      $a1, $s3, $zero
    /* 5F1CC 8006E9CC 21200000 */  addu       $a0, $zero, $zero
    /* 5F1D0 8006E9D0 70010224 */  addiu      $v0, $zero, 0x170
    /* 5F1D4 8006E9D4 1400A2A7 */  sh         $v0, 0x14($sp)
    /* 5F1D8 8006E9D8 24000224 */  addiu      $v0, $zero, 0x24
    /* 5F1DC 8006E9DC 1000B2A7 */  sh         $s2, 0x10($sp)
    /* 5F1E0 8006E9E0 1200B0A7 */  sh         $s0, 0x12($sp)
    /* 5F1E4 8006E9E4 CFEC010C */  jal        func_8007B33C
    /* 5F1E8 8006E9E8 1600A2A7 */   sh        $v0, 0x16($sp)
    /* 5F1EC 8006E9EC 0500053C */  lui        $a1, (0x59400 >> 16)
    /* 5F1F0 8006E9F0 0094A534 */  ori        $a1, $a1, (0x59400 & 0xFFFF)
    /* 5F1F4 8006E9F4 1000A427 */  addiu      $a0, $sp, 0x10
    /* 5F1F8 8006E9F8 80ED010C */  jal        func_8007B600
    /* 5F1FC 8006E9FC 21286502 */   addu      $a1, $s3, $a1
    /* 5F200 8006EA00 33BA010C */  jal        func_8006E8CC
    /* 5F204 8006EA04 21202002 */   addu      $a0, $s1, $zero
    /* 5F208 8006EA08 2800BF8F */  lw         $ra, 0x28($sp)
    /* 5F20C 8006EA0C 2400B38F */  lw         $s3, 0x24($sp)
    /* 5F210 8006EA10 2000B28F */  lw         $s2, 0x20($sp)
    /* 5F214 8006EA14 1C00B18F */  lw         $s1, 0x1C($sp)
    /* 5F218 8006EA18 1800B08F */  lw         $s0, 0x18($sp)
    /* 5F21C 8006EA1C 3000BD27 */  addiu      $sp, $sp, 0x30
    /* 5F220 8006EA20 0800E003 */  jr         $ra
    /* 5F224 8006EA24 00000000 */   nop
endlabel func_8006E950
