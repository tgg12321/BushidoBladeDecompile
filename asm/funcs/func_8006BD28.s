glabel func_8006BD28
    /* 5C528 8006BD28 A8FFBD27 */  addiu      $sp, $sp, -0x58
    /* 5C52C 8006BD2C 4000B4AF */  sw         $s4, 0x40($sp)
    /* 5C530 8006BD30 21A08000 */  addu       $s4, $a0, $zero
    /* 5C534 8006BD34 4400B5AF */  sw         $s5, 0x44($sp)
    /* 5C538 8006BD38 21A8A000 */  addu       $s5, $a1, $zero
    /* 5C53C 8006BD3C 3400B1AF */  sw         $s1, 0x34($sp)
    /* 5C540 8006BD40 2188C000 */  addu       $s1, $a2, $zero
    /* 5C544 8006BD44 4800B6AF */  sw         $s6, 0x48($sp)
    /* 5C548 8006BD48 21B0E000 */  addu       $s6, $a3, $zero
    /* 5C54C 8006BD4C 3C00B3AF */  sw         $s3, 0x3C($sp)
    /* 5C550 8006BD50 21980000 */  addu       $s3, $zero, $zero
    /* 5C554 8006BD54 3004828F */  lw         $v0, %gp_rel(D_800A34FC)($gp)
    /* 5C558 8006BD58 C0401400 */  sll        $t0, $s4, 3
    /* 5C55C 8006BD5C 5400BFAF */  sw         $ra, 0x54($sp)
    /* 5C560 8006BD60 5000BEAF */  sw         $fp, 0x50($sp)
    /* 5C564 8006BD64 4C00B7AF */  sw         $s7, 0x4C($sp)
    /* 5C568 8006BD68 3800B2AF */  sw         $s2, 0x38($sp)
    /* 5C56C 8006BD6C 3000B0AF */  sw         $s0, 0x30($sp)
    /* 5C570 8006BD70 2000A8AF */  sw         $t0, 0x20($sp)
    /* 5C574 8006BD74 2400428C */  lw         $v0, 0x24($v0)
    /* 5C578 8006BD78 12001724 */  addiu      $s7, $zero, 0x12
    /* 5C57C 8006BD7C 2000428C */  lw         $v0, 0x20($v0)
    /* 5C580 8006BD80 08001E24 */  addiu      $fp, $zero, 0x8
    /* 5C584 8006BD84 1800A2AF */  sw         $v0, 0x18($sp)
    /* 5C588 8006BD88 30000224 */  addiu      $v0, $zero, 0x30
    /* 5C58C 8006BD8C 2B0022A2 */  sb         $v0, 0x2B($s1)
    /* 5C590 8006BD90 2A0022A2 */  sb         $v0, 0x2A($s1)
    /* 5C594 8006BD94 290022A2 */  sb         $v0, 0x29($s1)
  .L8006BD98:
    /* 5C598 8006BD98 1800A88F */  lw         $t0, 0x18($sp)
    /* 5C59C 8006BD9C 80101300 */  sll        $v0, $s3, 2
    /* 5C5A0 8006BDA0 21104800 */  addu       $v0, $v0, $t0
    /* 5C5A4 8006BDA4 2000A88F */  lw         $t0, 0x20($sp)
    /* 5C5A8 8006BDA8 00000000 */  nop
    /* 5C5AC 8006BDAC 21100201 */  addu       $v0, $t0, $v0
    /* 5C5B0 8006BDB0 0000438C */  lw         $v1, 0x0($v0)
    /* 5C5B4 8006BDB4 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 5C5B8 8006BDB8 35006210 */  beq        $v1, $v0, .L8006BE90
    /* 5C5BC 8006BDBC 000023AE */   sw        $v1, 0x0($s1)
    /* 5C5C0 8006BDC0 02009716 */  bne        $s4, $s7, .L8006BDCC
    /* 5C5C4 8006BDC4 01001224 */   addiu     $s2, $zero, 0x1
    /* 5C5C8 8006BDC8 03001224 */  addiu      $s2, $zero, 0x3
  .L8006BDCC:
    /* 5C5CC 8006BDCC 1B004012 */  beqz       $s2, .L8006BE3C
    /* 5C5D0 8006BDD0 21800000 */   addu      $s0, $zero, $zero
  .L8006BDD4:
    /* 5C5D4 8006BDD4 180020AE */  sw         $zero, 0x18($s1)
    /* 5C5D8 8006BDD8 1C0035AE */  sw         $s5, 0x1C($s1)
    /* 5C5DC 8006BDDC 05009716 */  bne        $s4, $s7, .L8006BDF4
    /* 5C5E0 8006BDE0 14003EAE */   sw        $fp, 0x14($s1)
    /* 5C5E4 8006BDE4 03001612 */  beq        $s0, $s6, .L8006BDF4
    /* 5C5E8 8006BDE8 02000224 */   addiu     $v0, $zero, 0x2
    /* 5C5EC 8006BDEC 03000216 */  bne        $s0, $v0, .L8006BDFC
    /* 5C5F0 8006BDF0 01000224 */   addiu     $v0, $zero, 0x1
  .L8006BDF4:
    /* 5C5F4 8006BDF4 80AF0108 */  j          .L8006BE00
    /* 5C5F8 8006BDF8 280020A2 */   sb        $zero, 0x28($s1)
  .L8006BDFC:
    /* 5C5FC 8006BDFC 280022A2 */  sb         $v0, 0x28($s1)
  .L8006BE00:
    /* 5C600 8006BE00 21202002 */  addu       $a0, $s1, $zero
    /* 5C604 8006BE04 C0281000 */  sll        $a1, $s0, 3
    /* 5C608 8006BE08 1806838F */  lw         $v1, %gp_rel(D_800A36E4)($gp)
    /* 5C60C 8006BE0C 0000228E */  lw         $v0, 0x0($s1)
    /* 5C610 8006BE10 01001026 */  addiu      $s0, $s0, 0x1
    /* 5C614 8006BE14 100020AE */  sw         $zero, 0x10($s1)
    /* 5C618 8006BE18 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5C61C 8006BE1C 21104500 */  addu       $v0, $v0, $a1
    /* 5C620 8006BE20 080023AE */  sw         $v1, 0x8($s1)
    /* 5C624 8006BE24 4BCD010C */  jal        func_8007352C
    /* 5C628 8006BE28 040022AE */   sw        $v0, 0x4($s1)
    /* 5C62C 8006BE2C 180682AF */  sw         $v0, %gp_rel(D_800A36E4)($gp)
    /* 5C630 8006BE30 2A101202 */  slt        $v0, $s0, $s2
    /* 5C634 8006BE34 E7FF4014 */  bnez       $v0, .L8006BDD4
    /* 5C638 8006BE38 00000000 */   nop
  .L8006BE3C:
    /* 5C63C 8006BE3C 0000248E */  lw         $a0, 0x0($s1)
    /* 5C640 8006BE40 20B9010C */  jal        func_8006E480
    /* 5C644 8006BE44 21280000 */   addu      $a1, $zero, $zero
    /* 5C648 8006BE48 01000524 */  addiu      $a1, $zero, 0x1
    /* 5C64C 8006BE4C 21300000 */  addu       $a2, $zero, $zero
    /* 5C650 8006BE50 1406848F */  lw         $a0, %gp_rel(D_800A36E0)($gp)
    /* 5C654 8006BE54 21384000 */  addu       $a3, $v0, $zero
    /* 5C658 8006BE58 92F0010C */  jal        func_8007C248
    /* 5C65C 8006BE5C 1000A0AF */   sw        $zero, 0x10($sp)
    /* 5C660 8006BE60 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 5C664 8006BE64 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 5C668 8006BE68 1406858F */  lw         $a1, %gp_rel(D_800A36E0)($gp)
    /* 5C66C 8006BE6C 2DEA010C */  jal        func_8007A8B4
    /* 5C670 8006BE70 20008424 */   addiu     $a0, $a0, 0x20
    /* 5C674 8006BE74 1406828F */  lw         $v0, %gp_rel(D_800A36E0)($gp)
    /* 5C678 8006BE78 01007326 */  addiu      $s3, $s3, 0x1
    /* 5C67C 8006BE7C 0C004224 */  addiu      $v0, $v0, 0xC
    /* 5C680 8006BE80 140682AF */  sw         $v0, %gp_rel(D_800A36E0)($gp)
    /* 5C684 8006BE84 0200622A */  slti       $v0, $s3, 0x2
    /* 5C688 8006BE88 C3FF4014 */  bnez       $v0, .L8006BD98
    /* 5C68C 8006BE8C 00000000 */   nop
  .L8006BE90:
    /* 5C690 8006BE90 5400BF8F */  lw         $ra, 0x54($sp)
    /* 5C694 8006BE94 5000BE8F */  lw         $fp, 0x50($sp)
    /* 5C698 8006BE98 4C00B78F */  lw         $s7, 0x4C($sp)
    /* 5C69C 8006BE9C 4800B68F */  lw         $s6, 0x48($sp)
    /* 5C6A0 8006BEA0 4400B58F */  lw         $s5, 0x44($sp)
    /* 5C6A4 8006BEA4 4000B48F */  lw         $s4, 0x40($sp)
    /* 5C6A8 8006BEA8 3C00B38F */  lw         $s3, 0x3C($sp)
    /* 5C6AC 8006BEAC 3800B28F */  lw         $s2, 0x38($sp)
    /* 5C6B0 8006BEB0 3400B18F */  lw         $s1, 0x34($sp)
    /* 5C6B4 8006BEB4 3000B08F */  lw         $s0, 0x30($sp)
    /* 5C6B8 8006BEB8 5800BD27 */  addiu      $sp, $sp, 0x58
    /* 5C6BC 8006BEBC 0800E003 */  jr         $ra
    /* 5C6C0 8006BEC0 00000000 */   nop
endlabel func_8006BD28
