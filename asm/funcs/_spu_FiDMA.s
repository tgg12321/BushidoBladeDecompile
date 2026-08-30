glabel _spu_FiDMA
  alabel D_80088BA0
    /* 793A0 80088BA0 0A80023C */  lui        $v0, %hi(D_800A2D2C)
    /* 793A4 80088BA4 2C2D428C */  lw         $v0, %lo(D_800A2D2C)($v0)
    /* 793A8 80088BA8 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 793AC 80088BAC 03004014 */  bnez       $v0, .L80088BBC
    /* 793B0 80088BB0 1000BFAF */   sw        $ra, 0x10($sp)
    /* 793B4 80088BB4 9B24020C */  jal        _spu_Fw1ts
    /* 793B8 80088BB8 00000000 */   nop
  .L80088BBC:
    /* 793BC 80088BBC 0A80043C */  lui        $a0, %hi(D_800A2CDC)
    /* 793C0 80088BC0 DC2C848C */  lw         $a0, %lo(D_800A2CDC)($a0)
    /* 793C4 80088BC4 00000000 */  nop
    /* 793C8 80088BC8 AA018294 */  lhu        $v0, 0x1AA($a0)
    /* 793CC 80088BCC 00000000 */  nop
    /* 793D0 80088BD0 CFFF4230 */  andi       $v0, $v0, 0xFFCF
    /* 793D4 80088BD4 AA0182A4 */  sh         $v0, 0x1AA($a0)
    /* 793D8 80088BD8 AA018294 */  lhu        $v0, 0x1AA($a0)
    /* 793DC 80088BDC 00000000 */  nop
    /* 793E0 80088BE0 30004230 */  andi       $v0, $v0, 0x30
    /* 793E4 80088BE4 0B004010 */  beqz       $v0, .L80088C14
    /* 793E8 80088BE8 21180000 */   addu      $v1, $zero, $zero
    /* 793EC 80088BEC 01006324 */  addiu      $v1, $v1, 0x1
  .L80088BF0:
    /* 793F0 80088BF0 010F622C */  sltiu      $v0, $v1, 0xF01
    /* 793F4 80088BF4 07004010 */  beqz       $v0, .L80088C14
    /* 793F8 80088BF8 00000000 */   nop
    /* 793FC 80088BFC AA018294 */  lhu        $v0, 0x1AA($a0)
    /* 79400 80088C00 00000000 */  nop
    /* 79404 80088C04 30004230 */  andi       $v0, $v0, 0x30
    /* 79408 80088C08 F9FF4014 */  bnez       $v0, .L80088BF0
    /* 7940C 80088C0C 01006324 */   addiu     $v1, $v1, 0x1
    /* 79410 80088C10 FFFF6324 */  addiu      $v1, $v1, -0x1
  .L80088C14:
    /* 79414 80088C14 0A80023C */  lui        $v0, %hi(_spu_transferCallback)
    /* 79418 80088C18 142D428C */  lw         $v0, %lo(_spu_transferCallback)($v0)
    /* 7941C 80088C1C 00000000 */  nop
    /* 79420 80088C20 08004010 */  beqz       $v0, .L80088C44
    /* 79424 80088C24 00F0043C */   lui       $a0, (0xF0000009 >> 16)
    /* 79428 80088C28 0A80023C */  lui        $v0, %hi(_spu_transferCallback)
    /* 7942C 80088C2C 142D428C */  lw         $v0, %lo(_spu_transferCallback)($v0)
    /* 79430 80088C30 00000000 */  nop
    /* 79434 80088C34 09F84000 */  jalr       $v0
    /* 79438 80088C38 00000000 */   nop
    /* 7943C 80088C3C 14230208 */  j          .L80088C50
    /* 79440 80088C40 00000000 */   nop
  .L80088C44:
    /* 79444 80088C44 09008434 */  ori        $a0, $a0, (0xF0000009 & 0xFFFF)
    /* 79448 80088C48 2300020C */  jal        DeliverEvent
    /* 7944C 80088C4C 20000524 */   addiu     $a1, $zero, 0x20
  .L80088C50:
    /* 79450 80088C50 1000BF8F */  lw         $ra, 0x10($sp)
    /* 79454 80088C54 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 79458 80088C58 0800E003 */  jr         $ra
    /* 7945C 80088C5C 00000000 */   nop
endlabel _spu_FiDMA
