glabel func_8008ACD0
    /* 7B4D0 8008ACD0 FFFF0524 */  addiu      $a1, $zero, -0x1
    /* 7B4D4 8008ACD4 21180000 */  addu       $v1, $zero, $zero
    /* 7B4D8 8008ACD8 01000624 */  addiu      $a2, $zero, 0x1
    /* 7B4DC 8008ACDC 04106600 */  sllv       $v0, $a2, $v1
  .L8008ACE0:
    /* 7B4E0 8008ACE0 24108200 */  and        $v0, $a0, $v0
    /* 7B4E4 8008ACE4 0A004014 */  bnez       $v0, .L8008AD10
    /* 7B4E8 8008ACE8 00000000 */   nop
    /* 7B4EC 8008ACEC 01006324 */  addiu      $v1, $v1, 0x1
    /* 7B4F0 8008ACF0 18006228 */  slti       $v0, $v1, 0x18
    /* 7B4F4 8008ACF4 FAFF4014 */  bnez       $v0, .L8008ACE0
    /* 7B4F8 8008ACF8 04106600 */   sllv      $v0, $a2, $v1
  .L8008ACFC:
    /* 7B4FC 8008ACFC FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 7B500 8008AD00 0500A214 */  bne        $a1, $v0, .L8008AD18
    /* 7B504 8008AD04 00210500 */   sll       $a0, $a1, 4
    /* 7B508 8008AD08 572B0208 */  j          .L8008AD5C
    /* 7B50C 8008AD0C 00000000 */   nop
  .L8008AD10:
    /* 7B510 8008AD10 3F2B0208 */  j          .L8008ACFC
    /* 7B514 8008AD14 21286000 */   addu      $a1, $v1, $zero
  .L8008AD18:
    /* 7B518 8008AD18 0A80023C */  lui        $v0, %hi(D_800A2CDC)
    /* 7B51C 8008AD1C DC2C428C */  lw         $v0, %lo(D_800A2CDC)($v0)
    /* 7B520 8008AD20 0A80033C */  lui        $v1, %hi(D_800A2874)
    /* 7B524 8008AD24 7428638C */  lw         $v1, %lo(D_800A2874)($v1)
    /* 7B528 8008AD28 21208200 */  addu       $a0, $a0, $v0
    /* 7B52C 8008AD2C 01000224 */  addiu      $v0, $zero, 0x1
    /* 7B530 8008AD30 0410A200 */  sllv       $v0, $v0, $a1
    /* 7B534 8008AD34 24186200 */  and        $v1, $v1, $v0
    /* 7B538 8008AD38 0C008494 */  lhu        $a0, 0xC($a0)
    /* 7B53C 8008AD3C 05006010 */  beqz       $v1, .L8008AD54
    /* 7B540 8008AD40 00000000 */   nop
    /* 7B544 8008AD44 05008010 */  beqz       $a0, .L8008AD5C
    /* 7B548 8008AD48 03000224 */   addiu     $v0, $zero, 0x3
    /* 7B54C 8008AD4C 572B0208 */  j          .L8008AD5C
    /* 7B550 8008AD50 01000224 */   addiu     $v0, $zero, 0x1
  .L8008AD54:
    /* 7B554 8008AD54 2B100400 */  sltu       $v0, $zero, $a0
    /* 7B558 8008AD58 40100200 */  sll        $v0, $v0, 1
  .L8008AD5C:
    /* 7B55C 8008AD5C 0800E003 */  jr         $ra
    /* 7B560 8008AD60 00000000 */   nop
endlabel func_8008ACD0
