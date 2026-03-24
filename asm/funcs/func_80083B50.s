glabel func_80083B50
    /* 74350 80083B50 C0FFBD27 */  addiu      $sp, $sp, -0x40
    /* 74354 80083B54 FF008230 */  andi       $v0, $a0, 0xFF
    /* 74358 80083B58 0E004014 */  bnez       $v0, .L80083B94
    /* 7435C 80083B5C 3800BFAF */   sw        $ra, 0x38($sp)
    /* 74360 80083B60 FF00A330 */  andi       $v1, $a1, 0xFF
    /* 74364 80083B64 06006014 */  bnez       $v1, .L80083B80
    /* 74368 80083B68 01000224 */   addiu     $v0, $zero, 0x1
    /* 7436C 80083B6C 00020224 */  addiu      $v0, $zero, 0x200
    /* 74370 80083B70 1000A2AF */  sw         $v0, 0x10($sp)
    /* 74374 80083B74 FF00C230 */  andi       $v0, $a2, 0xFF
    /* 74378 80083B78 2800A2AF */  sw         $v0, 0x28($sp)
    /* 7437C 80083B7C 01000224 */  addiu      $v0, $zero, 0x1
  .L80083B80:
    /* 74380 80083B80 04006214 */  bne        $v1, $v0, .L80083B94
    /* 74384 80083B84 00010224 */   addiu     $v0, $zero, 0x100
    /* 74388 80083B88 1000A2AF */  sw         $v0, 0x10($sp)
    /* 7438C 80083B8C FF00C230 */  andi       $v0, $a2, 0xFF
    /* 74390 80083B90 2400A2AF */  sw         $v0, 0x24($sp)
  .L80083B94:
    /* 74394 80083B94 FF008430 */  andi       $a0, $a0, 0xFF
    /* 74398 80083B98 01000224 */  addiu      $v0, $zero, 0x1
    /* 7439C 80083B9C 0B008214 */  bne        $a0, $v0, .L80083BCC
    /* 743A0 80083BA0 FF00A530 */   andi      $a1, $a1, 0xFF
    /* 743A4 80083BA4 0400A014 */  bnez       $a1, .L80083BB8
    /* 743A8 80083BA8 00200224 */   addiu     $v0, $zero, 0x2000
    /* 743AC 80083BAC 1000A2AF */  sw         $v0, 0x10($sp)
    /* 743B0 80083BB0 FF00C230 */  andi       $v0, $a2, 0xFF
    /* 743B4 80083BB4 3400A2AF */  sw         $v0, 0x34($sp)
  .L80083BB8:
    /* 743B8 80083BB8 0400A414 */  bne        $a1, $a0, .L80083BCC
    /* 743BC 80083BBC 00100224 */   addiu     $v0, $zero, 0x1000
    /* 743C0 80083BC0 1000A2AF */  sw         $v0, 0x10($sp)
    /* 743C4 80083BC4 FF00C230 */  andi       $v0, $a2, 0xFF
    /* 743C8 80083BC8 3000A2AF */  sw         $v0, 0x30($sp)
  .L80083BCC:
    /* 743CC 80083BCC E72B020C */  jal        func_8008AF9C
    /* 743D0 80083BD0 1000A427 */   addiu     $a0, $sp, 0x10
    /* 743D4 80083BD4 3800BF8F */  lw         $ra, 0x38($sp)
    /* 743D8 80083BD8 4000BD27 */  addiu      $sp, $sp, 0x40
    /* 743DC 80083BDC 0800E003 */  jr         $ra
    /* 743E0 80083BE0 00000000 */   nop
endlabel func_80083B50
