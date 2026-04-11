glabel func_8005BD30
    /* 4C530 8005BD30 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 4C534 8005BD34 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 4C538 8005BD38 21988000 */  addu       $s3, $a0, $zero
    /* 4C53C 8005BD3C 21200000 */  addu       $a0, $zero, $zero
    /* 4C540 8005BD40 2000BFAF */  sw         $ra, 0x20($sp)
    /* 4C544 8005BD44 1800B2AF */  sw         $s2, 0x18($sp)
    /* 4C548 8005BD48 1400B1AF */  sw         $s1, 0x14($sp)
    /* 4C54C 8005BD4C 3416020C */  jal        title_mv_exec2
    /* 4C550 8005BD50 1000B0AF */   sw        $s0, 0x10($sp)
    /* 4C554 8005BD54 0F80033C */  lui        $v1, %hi(D_800EFC50)
    /* 4C558 8005BD58 50FC638C */  lw         $v1, %lo(D_800EFC50)($v1)
    /* 4C55C 8005BD5C 0F80023C */  lui        $v0, %hi(D_800EFC44)
    /* 4C560 8005BD60 44FC428C */  lw         $v0, %lo(D_800EFC44)($v0)
    /* 4C564 8005BD64 00000000 */  nop
    /* 4C568 8005BD68 02006214 */  bne        $v1, $v0, .L8005BD74
    /* 4C56C 8005BD6C 03001124 */   addiu     $s1, $zero, 0x3
    /* 4C570 8005BD70 02001124 */  addiu      $s1, $zero, 0x2
  .L8005BD74:
    /* 4C574 8005BD74 FF002232 */  andi       $v0, $s1, 0xFF
    /* 4C578 8005BD78 0D004010 */  beqz       $v0, .L8005BDB0
    /* 4C57C 8005BD7C 21800000 */   addu      $s0, $zero, $zero
    /* 4C580 8005BD80 21904000 */  addu       $s2, $v0, $zero
    /* 4C584 8005BD84 21206002 */  addu       $a0, $s3, $zero
  .L8005BD88:
    /* 4C588 8005BD88 FF000232 */  andi       $v0, $s0, 0xFF
    /* 4C58C 8005BD8C 0A80013C */  lui        $at, %hi(D_8009AD18)
    /* 4C590 8005BD90 21082200 */  addu       $at, $at, $v0
    /* 4C594 8005BD94 18AD2590 */  lbu        $a1, %lo(D_8009AD18)($at)
    /* 4C598 8005BD98 3071010C */  jal        func_8005C4C0
    /* 4C59C 8005BD9C 01001026 */   addiu     $s0, $s0, 0x1
    /* 4C5A0 8005BDA0 FF000232 */  andi       $v0, $s0, 0xFF
    /* 4C5A4 8005BDA4 2B105200 */  sltu       $v0, $v0, $s2
    /* 4C5A8 8005BDA8 F7FF4014 */  bnez       $v0, .L8005BD88
    /* 4C5AC 8005BDAC 21206002 */   addu      $a0, $s3, $zero
  .L8005BDB0:
    /* 4C5B0 8005BDB0 FF002332 */  andi       $v1, $s1, 0xFF
    /* 4C5B4 8005BDB4 02000224 */  addiu      $v0, $zero, 0x2
    /* 4C5B8 8005BDB8 05006214 */  bne        $v1, $v0, .L8005BDD0
    /* 4C5BC 8005BDBC 00000000 */   nop
    /* 4C5C0 8005BDC0 0F80023C */  lui        $v0, %hi(D_800EFC44)
    /* 4C5C4 8005BDC4 44FC428C */  lw         $v0, %lo(D_800EFC44)($v0)
    /* 4C5C8 8005BDC8 0F80013C */  lui        $at, %hi(D_800EFC50)
    /* 4C5CC 8005BDCC 50FC22AC */  sw         $v0, %lo(D_800EFC50)($at)
  .L8005BDD0:
    /* 4C5D0 8005BDD0 2000BF8F */  lw         $ra, 0x20($sp)
    /* 4C5D4 8005BDD4 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 4C5D8 8005BDD8 1800B28F */  lw         $s2, 0x18($sp)
    /* 4C5DC 8005BDDC 1400B18F */  lw         $s1, 0x14($sp)
    /* 4C5E0 8005BDE0 1000B08F */  lw         $s0, 0x10($sp)
    /* 4C5E4 8005BDE4 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 4C5E8 8005BDE8 0800E003 */  jr         $ra
    /* 4C5EC 8005BDEC 00000000 */   nop
endlabel func_8005BD30
