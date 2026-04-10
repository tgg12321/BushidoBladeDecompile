glabel func_8003F420
    /* 2FC20 8003F420 6210023C */  lui        $v0, (0x10624DD3 >> 16)
    /* 2FC24 8003F424 D34D4234 */  ori        $v0, $v0, (0x10624DD3 & 0xFFFF)
    /* 2FC28 8003F428 007D8424 */  addiu      $a0, $a0, 0x7D00
    /* 2FC2C 8003F42C 18008200 */  mult       $a0, $v0
    /* 2FC30 8003F430 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 2FC34 8003F434 007DA524 */  addiu      $a1, $a1, 0x7D00
    /* 2FC38 8003F438 2000BFAF */  sw         $ra, 0x20($sp)
    /* 2FC3C 8003F43C 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 2FC40 8003F440 10180000 */  mfhi       $v1
    /* 2FC44 8003F444 1800B2AF */  sw         $s2, 0x18($sp)
    /* 2FC48 8003F448 1400B1AF */  sw         $s1, 0x14($sp)
    /* 2FC4C 8003F44C 1800A200 */  mult       $a1, $v0
    /* 2FC50 8003F450 1000B0AF */  sw         $s0, 0x10($sp)
    /* 2FC54 8003F454 C3190300 */  sra        $v1, $v1, 7
    /* 2FC58 8003F458 C3170400 */  sra        $v0, $a0, 31
    /* 2FC5C 8003F45C 23986200 */  subu       $s3, $v1, $v0
    /* 2FC60 8003F460 40111300 */  sll        $v0, $s3, 5
    /* 2FC64 8003F464 23105300 */  subu       $v0, $v0, $s3
    /* 2FC68 8003F468 80100200 */  sll        $v0, $v0, 2
    /* 2FC6C 8003F46C 21105300 */  addu       $v0, $v0, $s3
    /* 2FC70 8003F470 00110200 */  sll        $v0, $v0, 4
    /* 2FC74 8003F474 23888200 */  subu       $s1, $a0, $v0
    /* 2FC78 8003F478 C3170500 */  sra        $v0, $a1, 31
    /* 2FC7C 8003F47C 10300000 */  mfhi       $a2
    /* 2FC80 8003F480 C3190600 */  sra        $v1, $a2, 7
    /* 2FC84 8003F484 23806200 */  subu       $s0, $v1, $v0
    /* 2FC88 8003F488 21900002 */  addu       $s2, $s0, $zero
    /* 2FC8C 8003F48C 40111200 */  sll        $v0, $s2, 5
    /* 2FC90 8003F490 23105200 */  subu       $v0, $v0, $s2
    /* 2FC94 8003F494 80100200 */  sll        $v0, $v0, 2
    /* 2FC98 8003F498 21105200 */  addu       $v0, $v0, $s2
    /* 2FC9C 8003F49C 00110200 */  sll        $v0, $v0, 4
    /* 2FCA0 8003F4A0 2380A200 */  subu       $s0, $a1, $v0
    /* 2FCA4 8003F4A4 E803222A */  slti       $v0, $s1, 0x3E8
    /* 2FCA8 8003F4A8 02004010 */  beqz       $v0, .L8003F4B4
    /* 2FCAC 8003F4AC 01001124 */   addiu     $s1, $zero, 0x1
    /* 2FCB0 8003F4B0 FFFF1124 */  addiu      $s1, $zero, -0x1
  .L8003F4B4:
    /* 2FCB4 8003F4B4 E803022A */  slti       $v0, $s0, 0x3E8
    /* 2FCB8 8003F4B8 02004010 */  beqz       $v0, .L8003F4C4
    /* 2FCBC 8003F4BC 01001024 */   addiu     $s0, $zero, 0x1
    /* 2FCC0 8003F4C0 FFFF1024 */  addiu      $s0, $zero, -0x1
  .L8003F4C4:
    /* 2FCC4 8003F4C4 21206002 */  addu       $a0, $s3, $zero
    /* 2FCC8 8003F4C8 21284002 */  addu       $a1, $s2, $zero
    /* 2FCCC 8003F4CC 4BFD000C */  jal        stage_SetCollision
    /* 2FCD0 8003F4D0 02000624 */   addiu     $a2, $zero, 0x2
    /* 2FCD4 8003F4D4 21887102 */  addu       $s1, $s3, $s1
    /* 2FCD8 8003F4D8 21202002 */  addu       $a0, $s1, $zero
    /* 2FCDC 8003F4DC 21284002 */  addu       $a1, $s2, $zero
    /* 2FCE0 8003F4E0 4BFD000C */  jal        stage_SetCollision
    /* 2FCE4 8003F4E4 02000624 */   addiu     $a2, $zero, 0x2
    /* 2FCE8 8003F4E8 21206002 */  addu       $a0, $s3, $zero
    /* 2FCEC 8003F4EC 21805002 */  addu       $s0, $s2, $s0
    /* 2FCF0 8003F4F0 21280002 */  addu       $a1, $s0, $zero
    /* 2FCF4 8003F4F4 4BFD000C */  jal        stage_SetCollision
    /* 2FCF8 8003F4F8 02000624 */   addiu     $a2, $zero, 0x2
    /* 2FCFC 8003F4FC 21202002 */  addu       $a0, $s1, $zero
    /* 2FD00 8003F500 21280002 */  addu       $a1, $s0, $zero
    /* 2FD04 8003F504 4BFD000C */  jal        stage_SetCollision
    /* 2FD08 8003F508 02000624 */   addiu     $a2, $zero, 0x2
    /* 2FD0C 8003F50C 2000BF8F */  lw         $ra, 0x20($sp)
    /* 2FD10 8003F510 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 2FD14 8003F514 1800B28F */  lw         $s2, 0x18($sp)
    /* 2FD18 8003F518 1400B18F */  lw         $s1, 0x14($sp)
    /* 2FD1C 8003F51C 1000B08F */  lw         $s0, 0x10($sp)
    /* 2FD20 8003F520 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 2FD24 8003F524 0800E003 */  jr         $ra
    /* 2FD28 8003F528 00000000 */   nop
endlabel func_8003F420
