glabel func_8003E2D8
    /* 2EAD8 8003E2D8 007D8824 */  addiu      $t0, $a0, 0x7D00
    /* 2EADC 8003E2DC 6210023C */  lui        $v0, (0x10624DD3 >> 16)
    /* 2EAE0 8003E2E0 D34D4234 */  ori        $v0, $v0, (0x10624DD3 & 0xFFFF)
    /* 2EAE4 8003E2E4 18000201 */  mult       $t0, $v0
    /* 2EAE8 8003E2E8 B0FFBD27 */  addiu      $sp, $sp, -0x50
    /* 2EAEC 8003E2EC 10180000 */  mfhi       $v1
    /* 2EAF0 8003E2F0 3000B2AF */  sw         $s2, 0x30($sp)
    /* 2EAF4 8003E2F4 007DB224 */  addiu      $s2, $a1, 0x7D00
    /* 2EAF8 8003E2F8 18004202 */  mult       $s2, $v0
    /* 2EAFC 8003E2FC 10480000 */  mfhi       $t1
    /* 2EB00 8003E300 007DC624 */  addiu      $a2, $a2, 0x7D00
    /* 2EB04 8003E304 00000000 */  nop
    /* 2EB08 8003E308 1800C200 */  mult       $a2, $v0
    /* 2EB0C 8003E30C 007DE724 */  addiu      $a3, $a3, 0x7D00
    /* 2EB10 8003E310 2320C800 */  subu       $a0, $a2, $t0
    /* 2EB14 8003E314 2328F200 */  subu       $a1, $a3, $s2
    /* 2EB18 8003E318 4C00BFAF */  sw         $ra, 0x4C($sp)
    /* 2EB1C 8003E31C 4800BEAF */  sw         $fp, 0x48($sp)
    /* 2EB20 8003E320 4400B7AF */  sw         $s7, 0x44($sp)
    /* 2EB24 8003E324 4000B6AF */  sw         $s6, 0x40($sp)
    /* 2EB28 8003E328 3C00B5AF */  sw         $s5, 0x3C($sp)
    /* 2EB2C 8003E32C 3800B4AF */  sw         $s4, 0x38($sp)
    /* 2EB30 8003E330 10500000 */  mfhi       $t2
    /* 2EB34 8003E334 3400B3AF */  sw         $s3, 0x34($sp)
    /* 2EB38 8003E338 2C00B1AF */  sw         $s1, 0x2C($sp)
    /* 2EB3C 8003E33C 1800E200 */  mult       $a3, $v0
    /* 2EB40 8003E340 2800B0AF */  sw         $s0, 0x28($sp)
    /* 2EB44 8003E344 C3190300 */  sra        $v1, $v1, 7
    /* 2EB48 8003E348 C3170800 */  sra        $v0, $t0, 31
    /* 2EB4C 8003E34C 23886200 */  subu       $s1, $v1, $v0
    /* 2EB50 8003E350 C3190900 */  sra        $v1, $t1, 7
    /* 2EB54 8003E354 C3171200 */  sra        $v0, $s2, 31
    /* 2EB58 8003E358 23806200 */  subu       $s0, $v1, $v0
    /* 2EB5C 8003E35C C3190A00 */  sra        $v1, $t2, 7
    /* 2EB60 8003E360 C3170600 */  sra        $v0, $a2, 31
    /* 2EB64 8003E364 23F06200 */  subu       $fp, $v1, $v0
    /* 2EB68 8003E368 C3170700 */  sra        $v0, $a3, 31
    /* 2EB6C 8003E36C 10580000 */  mfhi       $t3
    /* 2EB70 8003E370 C3190B00 */  sra        $v1, $t3, 7
    /* 2EB74 8003E374 0F008014 */  bnez       $a0, .L8003E3B4
    /* 2EB78 8003E378 23B86200 */   subu      $s7, $v1, $v0
    /* 2EB7C 8003E37C 0D00A014 */  bnez       $a1, .L8003E3B4
    /* 2EB80 8003E380 00000000 */   nop
    /* 2EB84 8003E384 B9002006 */  bltz       $s1, .L8003E66C
    /* 2EB88 8003E388 00000000 */   nop
    /* 2EB8C 8003E38C B7000006 */  bltz       $s0, .L8003E66C
    /* 2EB90 8003E390 2000222A */   slti      $v0, $s1, 0x20
    /* 2EB94 8003E394 B5004010 */  beqz       $v0, .L8003E66C
    /* 2EB98 8003E398 2000022A */   slti      $v0, $s0, 0x20
    /* 2EB9C 8003E39C B3004010 */  beqz       $v0, .L8003E66C
    /* 2EBA0 8003E3A0 1000A427 */   addiu     $a0, $sp, 0x10
    /* 2EBA4 8003E3A4 F0FF2226 */  addiu      $v0, $s1, -0x10
    /* 2EBA8 8003E3A8 1000A2A7 */  sh         $v0, 0x10($sp)
    /* 2EBAC 8003E3AC 99F90008 */  j          .L8003E664
    /* 2EBB0 8003E3B0 F0FF0226 */   addiu     $v0, $s0, -0x10
  .L8003E3B4:
    /* 2EBB4 8003E3B4 40191100 */  sll        $v1, $s1, 5
    /* 2EBB8 8003E3B8 23187100 */  subu       $v1, $v1, $s1
    /* 2EBBC 8003E3BC 80180300 */  sll        $v1, $v1, 2
    /* 2EBC0 8003E3C0 21187100 */  addu       $v1, $v1, $s1
    /* 2EBC4 8003E3C4 00190300 */  sll        $v1, $v1, 4
    /* 2EBC8 8003E3C8 E8036324 */  addiu      $v1, $v1, 0x3E8
    /* 2EBCC 8003E3CC 23400301 */  subu       $t0, $t0, $v1
    /* 2EBD0 8003E3D0 40111000 */  sll        $v0, $s0, 5
    /* 2EBD4 8003E3D4 23105000 */  subu       $v0, $v0, $s0
    /* 2EBD8 8003E3D8 80100200 */  sll        $v0, $v0, 2
    /* 2EBDC 8003E3DC 21105000 */  addu       $v0, $v0, $s0
    /* 2EBE0 8003E3E0 00110200 */  sll        $v0, $v0, 4
    /* 2EBE4 8003E3E4 E8034224 */  addiu      $v0, $v0, 0x3E8
    /* 2EBE8 8003E3E8 23904202 */  subu       $s2, $s2, $v0
    /* 2EBEC 8003E3EC 2330C300 */  subu       $a2, $a2, $v1
    /* 2EBF0 8003E3F0 06008104 */  bgez       $a0, .L8003E40C
    /* 2EBF4 8003E3F4 2338E200 */   subu      $a3, $a3, $v0
    /* 2EBF8 8003E3F8 FFFF1624 */  addiu      $s6, $zero, -0x1
    /* 2EBFC 8003E3FC 23200400 */  negu       $a0, $a0
    /* 2EC00 8003E400 23400800 */  negu       $t0, $t0
    /* 2EC04 8003E404 04F90008 */  j          .L8003E410
    /* 2EC08 8003E408 23300600 */   negu      $a2, $a2
  .L8003E40C:
    /* 2EC0C 8003E40C 01001624 */  addiu      $s6, $zero, 0x1
  .L8003E410:
    /* 2EC10 8003E410 0500A104 */  bgez       $a1, .L8003E428
    /* 2EC14 8003E414 01001524 */   addiu     $s5, $zero, 0x1
    /* 2EC18 8003E418 FFFF1524 */  addiu      $s5, $zero, -0x1
    /* 2EC1C 8003E41C 23280500 */  negu       $a1, $a1
    /* 2EC20 8003E420 23901200 */  negu       $s2, $s2
    /* 2EC24 8003E424 23380700 */  negu       $a3, $a3
  .L8003E428:
    /* 2EC28 8003E428 2A108500 */  slt        $v0, $a0, $a1
    /* 2EC2C 8003E42C 09004010 */  beqz       $v0, .L8003E454
    /* 2EC30 8003E430 21108000 */   addu      $v0, $a0, $zero
    /* 2EC34 8003E434 2120A000 */  addu       $a0, $a1, $zero
    /* 2EC38 8003E438 21284000 */  addu       $a1, $v0, $zero
    /* 2EC3C 8003E43C 21100001 */  addu       $v0, $t0, $zero
    /* 2EC40 8003E440 21404002 */  addu       $t0, $s2, $zero
    /* 2EC44 8003E444 21904000 */  addu       $s2, $v0, $zero
    /* 2EC48 8003E448 2130E000 */  addu       $a2, $a3, $zero
    /* 2EC4C 8003E44C 16F90008 */  j          .L8003E458
    /* 2EC50 8003E450 01001424 */   addiu     $s4, $zero, 0x1
  .L8003E454:
    /* 2EC54 8003E454 21A00000 */  addu       $s4, $zero, $zero
  .L8003E458:
    /* 2EC58 8003E458 002B0500 */  sll        $a1, $a1, 12
    /* 2EC5C 8003E45C 1A00A400 */  div        $zero, $a1, $a0
    /* 2EC60 8003E460 02008014 */  bnez       $a0, .L8003E46C
    /* 2EC64 8003E464 00000000 */   nop
    /* 2EC68 8003E468 0D000700 */  break      7
  .L8003E46C:
    /* 2EC6C 8003E46C FFFF0124 */  addiu      $at, $zero, -0x1
    /* 2EC70 8003E470 04008114 */  bne        $a0, $at, .L8003E484
    /* 2EC74 8003E474 0080013C */   lui       $at, (0x80000000 >> 16)
    /* 2EC78 8003E478 0200A114 */  bne        $a1, $at, .L8003E484
    /* 2EC7C 8003E47C 00000000 */   nop
    /* 2EC80 8003E480 0D000600 */  break      6
  .L8003E484:
    /* 2EC84 8003E484 12280000 */  mflo       $a1
    /* 2EC88 8003E488 E803C624 */  addiu      $a2, $a2, 0x3E8
    /* 2EC8C 8003E48C 6210023C */  lui        $v0, (0x10624DD3 >> 16)
    /* 2EC90 8003E490 D34D4234 */  ori        $v0, $v0, (0x10624DD3 & 0xFFFF)
    /* 2EC94 8003E494 1800C200 */  mult       $a2, $v0
    /* 2EC98 8003E498 10180000 */  mfhi       $v1
    /* 2EC9C 8003E49C E8030825 */  addiu      $t0, $t0, 0x3E8
    /* 2ECA0 8003E4A0 00000000 */  nop
    /* 2ECA4 8003E4A4 18000201 */  mult       $t0, $v0
    /* 2ECA8 8003E4A8 10380000 */  mfhi       $a3
    /* 2ECAC 8003E4AC 00000000 */  nop
    /* 2ECB0 8003E4B0 00000000 */  nop
    /* 2ECB4 8003E4B4 18000501 */  mult       $t0, $a1
    /* 2ECB8 8003E4B8 E8035226 */  addiu      $s2, $s2, 0x3E8
    /* 2ECBC 8003E4BC C3210300 */  sra        $a0, $v1, 7
    /* 2ECC0 8003E4C0 C3170600 */  sra        $v0, $a2, 31
    /* 2ECC4 8003E4C4 23208200 */  subu       $a0, $a0, $v0
    /* 2ECC8 8003E4C8 C31F0800 */  sra        $v1, $t0, 31
    /* 2ECCC 8003E4CC C3110700 */  sra        $v0, $a3, 7
    /* 2ECD0 8003E4D0 23104300 */  subu       $v0, $v0, $v1
    /* 2ECD4 8003E4D4 23988200 */  subu       $s3, $a0, $v0
    /* 2ECD8 8003E4D8 12600000 */  mflo       $t4
    /* 2ECDC 8003E4DC 03130C00 */  sra        $v0, $t4, 12
    /* 2ECE0 8003E4E0 23904202 */  subu       $s2, $s2, $v0
    /* 2ECE4 8003E4E4 40110500 */  sll        $v0, $a1, 5
    /* 2ECE8 8003E4E8 23104500 */  subu       $v0, $v0, $a1
    /* 2ECEC 8003E4EC 80100200 */  sll        $v0, $v0, 2
    /* 2ECF0 8003E4F0 21104500 */  addu       $v0, $v0, $a1
    /* 2ECF4 8003E4F4 00110200 */  sll        $v0, $v0, 4
    /* 2ECF8 8003E4F8 03130200 */  sra        $v0, $v0, 12
    /* 2ECFC 8003E4FC 2000A2AF */  sw         $v0, 0x20($sp)
    /* 2ED00 8003E500 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 2ED04 8003E504 4C006212 */  beq        $s3, $v0, .L8003E638
    /* 2ED08 8003E508 00000000 */   nop
  .L8003E50C:
    /* 2ED0C 8003E50C 0D002006 */  bltz       $s1, .L8003E544
    /* 2ED10 8003E510 6210023C */   lui       $v0, (0x10624DD3 >> 16)
    /* 2ED14 8003E514 0A000006 */  bltz       $s0, .L8003E540
    /* 2ED18 8003E518 2000222A */   slti      $v0, $s1, 0x20
    /* 2ED1C 8003E51C 08004010 */  beqz       $v0, .L8003E540
    /* 2ED20 8003E520 2000022A */   slti      $v0, $s0, 0x20
    /* 2ED24 8003E524 06004010 */  beqz       $v0, .L8003E540
    /* 2ED28 8003E528 1000A427 */   addiu     $a0, $sp, 0x10
    /* 2ED2C 8003E52C F0FF2226 */  addiu      $v0, $s1, -0x10
    /* 2ED30 8003E530 1000A2A7 */  sh         $v0, 0x10($sp)
    /* 2ED34 8003E534 F0FF0226 */  addiu      $v0, $s0, -0x10
    /* 2ED38 8003E538 E2FC000C */  jal        func_8003F388
    /* 2ED3C 8003E53C 1400A2A7 */   sh        $v0, 0x14($sp)
  .L8003E540:
    /* 2ED40 8003E540 6210023C */  lui        $v0, (0x10624DD3 >> 16)
  .L8003E544:
    /* 2ED44 8003E544 D34D4234 */  ori        $v0, $v0, (0x10624DD3 & 0xFFFF)
    /* 2ED48 8003E548 18004202 */  mult       $s2, $v0
    /* 2ED4C 8003E54C C3171200 */  sra        $v0, $s2, 31
    /* 2ED50 8003E550 10600000 */  mfhi       $t4
    /* 2ED54 8003E554 C3190C00 */  sra        $v1, $t4, 7
    /* 2ED58 8003E558 23186200 */  subu       $v1, $v1, $v0
    /* 2ED5C 8003E55C 40110300 */  sll        $v0, $v1, 5
    /* 2ED60 8003E560 23104300 */  subu       $v0, $v0, $v1
    /* 2ED64 8003E564 80100200 */  sll        $v0, $v0, 2
    /* 2ED68 8003E568 21104300 */  addu       $v0, $v0, $v1
    /* 2ED6C 8003E56C 00110200 */  sll        $v0, $v0, 4
    /* 2ED70 8003E570 2000AC8F */  lw         $t4, 0x20($sp)
    /* 2ED74 8003E574 23904202 */  subu       $s2, $s2, $v0
    /* 2ED78 8003E578 2F006012 */  beqz       $s3, .L8003E638
    /* 2ED7C 8003E57C 21904C02 */   addu      $s2, $s2, $t4
    /* 2ED80 8003E580 D107422A */  slti       $v0, $s2, 0x7D1
    /* 2ED84 8003E584 1B004014 */  bnez       $v0, .L8003E5F4
    /* 2ED88 8003E588 00000000 */   nop
    /* 2ED8C 8003E58C 07008012 */  beqz       $s4, .L8003E5AC
    /* 2ED90 8003E590 00000000 */   nop
    /* 2ED94 8003E594 0300C106 */  bgez       $s6, .L8003E5A4
    /* 2ED98 8003E598 00000000 */   nop
    /* 2ED9C 8003E59C 70F90008 */  j          .L8003E5C0
    /* 2EDA0 8003E5A0 FFFF3126 */   addiu     $s1, $s1, -0x1
  .L8003E5A4:
    /* 2EDA4 8003E5A4 70F90008 */  j          .L8003E5C0
    /* 2EDA8 8003E5A8 01003126 */   addiu     $s1, $s1, 0x1
  .L8003E5AC:
    /* 2EDAC 8003E5AC 0300A106 */  bgez       $s5, .L8003E5BC
    /* 2EDB0 8003E5B0 00000000 */   nop
    /* 2EDB4 8003E5B4 70F90008 */  j          .L8003E5C0
    /* 2EDB8 8003E5B8 FFFF1026 */   addiu     $s0, $s0, -0x1
  .L8003E5BC:
    /* 2EDBC 8003E5BC 01001026 */  addiu      $s0, $s0, 0x1
  .L8003E5C0:
    /* 2EDC0 8003E5C0 0C002006 */  bltz       $s1, .L8003E5F4
    /* 2EDC4 8003E5C4 00000000 */   nop
    /* 2EDC8 8003E5C8 0A000006 */  bltz       $s0, .L8003E5F4
    /* 2EDCC 8003E5CC 2000222A */   slti      $v0, $s1, 0x20
    /* 2EDD0 8003E5D0 08004010 */  beqz       $v0, .L8003E5F4
    /* 2EDD4 8003E5D4 2000022A */   slti      $v0, $s0, 0x20
    /* 2EDD8 8003E5D8 06004010 */  beqz       $v0, .L8003E5F4
    /* 2EDDC 8003E5DC 1800A427 */   addiu     $a0, $sp, 0x18
    /* 2EDE0 8003E5E0 F0FF2226 */  addiu      $v0, $s1, -0x10
    /* 2EDE4 8003E5E4 1800A2A7 */  sh         $v0, 0x18($sp)
    /* 2EDE8 8003E5E8 F0FF0226 */  addiu      $v0, $s0, -0x10
    /* 2EDEC 8003E5EC E2FC000C */  jal        func_8003F388
    /* 2EDF0 8003E5F0 1C00A2A7 */   sh        $v0, 0x1C($sp)
  .L8003E5F4:
    /* 2EDF4 8003E5F4 07008012 */  beqz       $s4, .L8003E614
    /* 2EDF8 8003E5F8 00000000 */   nop
    /* 2EDFC 8003E5FC 0300A106 */  bgez       $s5, .L8003E60C
    /* 2EE00 8003E600 00000000 */   nop
    /* 2EE04 8003E604 8AF90008 */  j          .L8003E628
    /* 2EE08 8003E608 FFFF1026 */   addiu     $s0, $s0, -0x1
  .L8003E60C:
    /* 2EE0C 8003E60C 8AF90008 */  j          .L8003E628
    /* 2EE10 8003E610 01001026 */   addiu     $s0, $s0, 0x1
  .L8003E614:
    /* 2EE14 8003E614 0300C106 */  bgez       $s6, .L8003E624
    /* 2EE18 8003E618 00000000 */   nop
    /* 2EE1C 8003E61C 8AF90008 */  j          .L8003E628
    /* 2EE20 8003E620 FFFF3126 */   addiu     $s1, $s1, -0x1
  .L8003E624:
    /* 2EE24 8003E624 01003126 */  addiu      $s1, $s1, 0x1
  .L8003E628:
    /* 2EE28 8003E628 FFFF7326 */  addiu      $s3, $s3, -0x1
    /* 2EE2C 8003E62C FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 2EE30 8003E630 B6FF6216 */  bne        $s3, $v0, .L8003E50C
    /* 2EE34 8003E634 00000000 */   nop
  .L8003E638:
    /* 2EE38 8003E638 0C00C007 */  bltz       $fp, .L8003E66C
    /* 2EE3C 8003E63C 00000000 */   nop
    /* 2EE40 8003E640 0A00E006 */  bltz       $s7, .L8003E66C
    /* 2EE44 8003E644 2000C22B */   slti      $v0, $fp, 0x20
    /* 2EE48 8003E648 08004010 */  beqz       $v0, .L8003E66C
    /* 2EE4C 8003E64C 2000E22A */   slti      $v0, $s7, 0x20
    /* 2EE50 8003E650 06004010 */  beqz       $v0, .L8003E66C
    /* 2EE54 8003E654 1000A427 */   addiu     $a0, $sp, 0x10
    /* 2EE58 8003E658 F0FFC227 */  addiu      $v0, $fp, -0x10
    /* 2EE5C 8003E65C 1000A2A7 */  sh         $v0, 0x10($sp)
    /* 2EE60 8003E660 F0FFE226 */  addiu      $v0, $s7, -0x10
  .L8003E664:
    /* 2EE64 8003E664 E2FC000C */  jal        func_8003F388
    /* 2EE68 8003E668 1400A2A7 */   sh        $v0, 0x14($sp)
  .L8003E66C:
    /* 2EE6C 8003E66C 4C00BF8F */  lw         $ra, 0x4C($sp)
    /* 2EE70 8003E670 4800BE8F */  lw         $fp, 0x48($sp)
    /* 2EE74 8003E674 4400B78F */  lw         $s7, 0x44($sp)
    /* 2EE78 8003E678 4000B68F */  lw         $s6, 0x40($sp)
    /* 2EE7C 8003E67C 3C00B58F */  lw         $s5, 0x3C($sp)
    /* 2EE80 8003E680 3800B48F */  lw         $s4, 0x38($sp)
    /* 2EE84 8003E684 3400B38F */  lw         $s3, 0x34($sp)
    /* 2EE88 8003E688 3000B28F */  lw         $s2, 0x30($sp)
    /* 2EE8C 8003E68C 2C00B18F */  lw         $s1, 0x2C($sp)
    /* 2EE90 8003E690 2800B08F */  lw         $s0, 0x28($sp)
    /* 2EE94 8003E694 5000BD27 */  addiu      $sp, $sp, 0x50
    /* 2EE98 8003E698 0800E003 */  jr         $ra
    /* 2EE9C 8003E69C 00000000 */   nop
endlabel func_8003E2D8
