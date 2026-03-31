glabel md_game_check_change_sub_mode
    /* 2C4B4 8003BCB4 0A80023C */  lui        $v0, %hi(D_800A37B8)
    /* 2C4B8 8003BCB8 B837428C */  lw         $v0, %lo(D_800A37B8)($v0)
    /* 2C4BC 8003BCBC E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2C4C0 8003BCC0 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2C4C4 8003BCC4 01004224 */  addiu      $v0, $v0, 0x1
    /* 2C4C8 8003BCC8 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* 2C4CC 8003BCCC B83722AC */  sw         $v0, %lo(D_800A37B8)($at)
    /* 2C4D0 8003BCD0 DA53010C */  jal        func_80054F68
    /* 2C4D4 8003BCD4 00000000 */   nop
    /* 2C4D8 8003BCD8 07004010 */  beqz       $v0, .L8003BCF8
    /* 2C4DC 8003BCDC 4000033C */   lui       $v1, (0x400040 >> 16)
    /* 2C4E0 8003BCE0 1080023C */  lui        $v0, %hi(D_80102794)
    /* 2C4E4 8003BCE4 9427428C */  lw         $v0, %lo(D_80102794)($v0)
    /* 2C4E8 8003BCE8 40006334 */  ori        $v1, $v1, (0x400040 & 0xFFFF)
    /* 2C4EC 8003BCEC 24104300 */  and        $v0, $v0, $v1
    /* 2C4F0 8003BCF0 43004010 */  beqz       $v0, .L8003BE00
    /* 2C4F4 8003BCF4 00000000 */   nop
  .L8003BCF8:
    /* 2C4F8 8003BCF8 B0DC000C */  jal        func_800372C0
    /* 2C4FC 8003BCFC 00000000 */   nop
    /* 2C500 8003BD00 3752010C */  jal        func_800548DC
    /* 2C504 8003BD04 00000000 */   nop
    /* 2C508 8003BD08 0A80023C */  lui        $v0, %hi(D_800A38DC)
    /* 2C50C 8003BD0C DC384284 */  lh         $v0, %lo(D_800A38DC)($v0)
    /* 2C510 8003BD10 00000000 */  nop
    /* 2C514 8003BD14 3A004014 */  bnez       $v0, .L8003BE00
    /* 2C518 8003BD18 00000000 */   nop
    /* 2C51C 8003BD1C 0A80023C */  lui        $v0, %hi(D_800A3894)
    /* 2C520 8003BD20 9438428C */  lw         $v0, %lo(D_800A3894)($v0)
    /* 2C524 8003BD24 00000000 */  nop
    /* 2C528 8003BD28 1F004010 */  beqz       $v0, .L8003BDA8
    /* 2C52C 8003BD2C 00000000 */   nop
    /* 2C530 8003BD30 0A80023C */  lui        $v0, %hi(D_800A37B0)
    /* 2C534 8003BD34 B0374290 */  lbu        $v0, %lo(D_800A37B0)($v0)
    /* 2C538 8003BD38 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2C53C 8003BD3C 343820A4 */  sh         $zero, %lo(D_800A3834)($at)
    /* 2C540 8003BD40 FFFF4324 */  addiu      $v1, $v0, -0x1
    /* 2C544 8003BD44 0600622C */  sltiu      $v0, $v1, 0x6
    /* 2C548 8003BD48 2D004010 */  beqz       $v0, .L8003BE00
    /* 2C54C 8003BD4C 80100300 */   sll       $v0, $v1, 2
    /* 2C550 8003BD50 0180013C */  lui        $at, %hi(jtbl_80010D74)
    /* 2C554 8003BD54 21082200 */  addu       $at, $at, $v0
    /* 2C558 8003BD58 740D228C */  lw         $v0, %lo(jtbl_80010D74)($at)
    /* 2C55C 8003BD5C 00000000 */  nop
    /* 2C560 8003BD60 08004000 */  jr         $v0
    /* 2C564 8003BD64 00000000 */   nop
  jlabel .L8003BD68
    /* 2C568 8003BD68 0A80023C */  lui        $v0, %hi(D_800A3907)
    /* 2C56C 8003BD6C 07394290 */  lbu        $v0, %lo(D_800A3907)($v0)
    /* 2C570 8003BD70 00000000 */  nop
    /* 2C574 8003BD74 01004224 */  addiu      $v0, $v0, 0x1
    /* 2C578 8003BD78 0A80013C */  lui        $at, %hi(D_800A3907)
    /* 2C57C 8003BD7C 073922A0 */  sb         $v0, %lo(D_800A3907)($at)
    /* 2C580 8003BD80 80EF0008 */  j          .L8003BE00
    /* 2C584 8003BD84 00000000 */   nop
  jlabel .L8003BD88
    /* 2C588 8003BD88 D0EB000C */  jal        func_8003AF40
    /* 2C58C 8003BD8C 21200000 */   addu      $a0, $zero, $zero
    /* 2C590 8003BD90 FFEB000C */  jal        func_8003AFFC
    /* 2C594 8003BD94 00000000 */   nop
  jlabel .L8003BD98
    /* 2C598 8003BD98 0A80013C */  lui        $at, %hi(D_800A3894)
    /* 2C59C 8003BD9C 943820AC */  sw         $zero, %lo(D_800A3894)($at)
    /* 2C5A0 8003BDA0 7EEF0008 */  j          .L8003BDF8
    /* 2C5A4 8003BDA4 00000000 */   nop
  .L8003BDA8:
    /* 2C5A8 8003BDA8 0A80023C */  lui        $v0, %hi(D_800A385C)
    /* 2C5AC 8003BDAC 5C38428C */  lw         $v0, %lo(D_800A385C)($v0)
    /* 2C5B0 8003BDB0 00000000 */  nop
    /* 2C5B4 8003BDB4 10004010 */  beqz       $v0, .L8003BDF8
    /* 2C5B8 8003BDB8 01000224 */   addiu     $v0, $zero, 0x1
    /* 2C5BC 8003BDBC 0A80033C */  lui        $v1, %hi(D_800A390C)
    /* 2C5C0 8003BDC0 0C396390 */  lbu        $v1, %lo(D_800A390C)($v1)
    /* 2C5C4 8003BDC4 00000000 */  nop
    /* 2C5C8 8003BDC8 05006214 */  bne        $v1, $v0, .L8003BDE0
    /* 2C5CC 8003BDCC 00000000 */   nop
    /* 2C5D0 8003BDD0 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2C5D4 8003BDD4 343820A4 */  sh         $zero, %lo(D_800A3834)($at)
    /* 2C5D8 8003BDD8 80EF0008 */  j          .L8003BE00
    /* 2C5DC 8003BDDC 00000000 */   nop
  .L8003BDE0:
    /* 2C5E0 8003BDE0 07006010 */  beqz       $v1, .L8003BE00
    /* 2C5E4 8003BDE4 04006228 */   slti      $v0, $v1, 0x4
    /* 2C5E8 8003BDE8 05004010 */  beqz       $v0, .L8003BE00
    /* 2C5EC 8003BDEC 00000000 */   nop
    /* 2C5F0 8003BDF0 0A80013C */  lui        $at, %hi(D_800A385C)
    /* 2C5F4 8003BDF4 5C3820AC */  sw         $zero, %lo(D_800A385C)($at)
  .L8003BDF8:
    /* 2C5F8 8003BDF8 69ED000C */  jal        func_8003B5A4
    /* 2C5FC 8003BDFC 00000000 */   nop
  .L8003BE00:
    /* 2C600 8003BE00 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2C604 8003BE04 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2C608 8003BE08 0800E003 */  jr         $ra
    /* 2C60C 8003BE0C 00000000 */   nop
endlabel md_game_check_change_sub_mode
