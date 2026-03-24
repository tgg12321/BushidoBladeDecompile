glabel func_8006EC0C
    /* 5F40C 8006EC0C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 5F410 8006EC10 AC048393 */  lbu        $v1, %gp_rel(D_800A3578)($gp)
    /* 5F414 8006EC14 02000224 */  addiu      $v0, $zero, 0x2
    /* 5F418 8006EC18 22006210 */  beq        $v1, $v0, .L8006ECA4
    /* 5F41C 8006EC1C 1000BFAF */   sw        $ra, 0x10($sp)
    /* 5F420 8006EC20 03006228 */  slti       $v0, $v1, 0x3
    /* 5F424 8006EC24 05004010 */  beqz       $v0, .L8006EC3C
    /* 5F428 8006EC28 01000224 */   addiu     $v0, $zero, 0x1
    /* 5F42C 8006EC2C 0A006210 */  beq        $v1, $v0, .L8006EC58
    /* 5F430 8006EC30 00000000 */   nop
    /* 5F434 8006EC34 39BB0108 */  j          .L8006ECE4
    /* 5F438 8006EC38 00000000 */   nop
  .L8006EC3C:
    /* 5F43C 8006EC3C 03000224 */  addiu      $v0, $zero, 0x3
    /* 5F440 8006EC40 05006210 */  beq        $v1, $v0, .L8006EC58
    /* 5F444 8006EC44 04000224 */   addiu     $v0, $zero, 0x4
    /* 5F448 8006EC48 16006210 */  beq        $v1, $v0, .L8006ECA4
    /* 5F44C 8006EC4C 00000000 */   nop
    /* 5F450 8006EC50 39BB0108 */  j          .L8006ECE4
    /* 5F454 8006EC54 00000000 */   nop
  .L8006EC58:
    /* 5F458 8006EC58 A4048297 */  lhu        $v0, %gp_rel(D_800A3570)($gp)
    /* 5F45C 8006EC5C 00000000 */  nop
    /* 5F460 8006EC60 20004224 */  addiu      $v0, $v0, 0x20
    /* 5F464 8006EC64 A40482A7 */  sh         $v0, %gp_rel(D_800A3570)($gp)
    /* 5F468 8006EC68 00140200 */  sll        $v0, $v0, 16
    /* 5F46C 8006EC6C 03140200 */  sra        $v0, $v0, 16
    /* 5F470 8006EC70 E8014228 */  slti       $v0, $v0, 0x1E8
    /* 5F474 8006EC74 1B004014 */  bnez       $v0, .L8006ECE4
    /* 5F478 8006EC78 E8010324 */   addiu     $v1, $zero, 0x1E8
    /* 5F47C 8006EC7C B8048297 */  lhu        $v0, %gp_rel(D_800A3584)($gp)
    /* 5F480 8006EC80 AC048497 */  lhu        $a0, %gp_rel(D_800A3578)($gp)
    /* 5F484 8006EC84 A40483A7 */  sh         $v1, %gp_rel(D_800A3570)($gp)
    /* 5F488 8006EC88 B40482A7 */  sh         $v0, %gp_rel(D_800A3580)($gp)
    /* 5F48C 8006EC8C 02120400 */  srl        $v0, $a0, 8
    /* 5F490 8006EC90 14004014 */  bnez       $v0, .L8006ECE4
    /* 5F494 8006EC94 01008224 */   addiu     $v0, $a0, 0x1
    /* 5F498 8006EC98 AC0482A7 */  sh         $v0, %gp_rel(D_800A3578)($gp)
    /* 5F49C 8006EC9C 39BB0108 */  j          .L8006ECE4
    /* 5F4A0 8006ECA0 00000000 */   nop
  .L8006ECA4:
    /* 5F4A4 8006ECA4 A4048387 */  lh         $v1, %gp_rel(D_800A3570)($gp)
    /* 5F4A8 8006ECA8 E8010224 */  addiu      $v0, $zero, 0x1E8
    /* 5F4AC 8006ECAC 04006214 */  bne        $v1, $v0, .L8006ECC0
    /* 5F4B0 8006ECB0 05000424 */   addiu     $a0, $zero, 0x5
    /* 5F4B4 8006ECB4 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 5F4B8 8006ECB8 9471010C */  jal        func_8005C650
    /* 5F4BC 8006ECBC 7F000624 */   addiu     $a2, $zero, 0x7F
  .L8006ECC0:
    /* 5F4C0 8006ECC0 A4048297 */  lhu        $v0, %gp_rel(D_800A3570)($gp)
    /* 5F4C4 8006ECC4 00000000 */  nop
    /* 5F4C8 8006ECC8 E0FF4224 */  addiu      $v0, $v0, -0x20
    /* 5F4CC 8006ECCC A40482A7 */  sh         $v0, %gp_rel(D_800A3570)($gp)
    /* 5F4D0 8006ECD0 00140200 */  sll        $v0, $v0, 16
    /* 5F4D4 8006ECD4 0300401C */  bgtz       $v0, .L8006ECE4
    /* 5F4D8 8006ECD8 00000000 */   nop
    /* 5F4DC 8006ECDC A40480A7 */  sh         $zero, %gp_rel(D_800A3570)($gp)
    /* 5F4E0 8006ECE0 AC0480A7 */  sh         $zero, %gp_rel(D_800A3578)($gp)
  .L8006ECE4:
    /* 5F4E4 8006ECE4 1000BF8F */  lw         $ra, 0x10($sp)
    /* 5F4E8 8006ECE8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 5F4EC 8006ECEC 0800E003 */  jr         $ra
    /* 5F4F0 8006ECF0 00000000 */   nop
endlabel func_8006EC0C
