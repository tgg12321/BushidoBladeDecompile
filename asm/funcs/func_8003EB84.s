glabel func_8003EB84
    /* 2F384 8003EB84 06008004 */  bltz       $a0, .L8003EBA0
    /* 2F388 8003EB88 78FFBD27 */   addiu     $sp, $sp, -0x88
    /* 2F38C 8003EB8C 20008228 */  slti       $v0, $a0, 0x20
    /* 2F390 8003EB90 13004010 */  beqz       $v0, .L8003EBE0
    /* 2F394 8003EB94 FFFF0324 */   addiu     $v1, $zero, -0x1
    /* 2F398 8003EB98 ECFA0008 */  j          .L8003EBB0
    /* 2F39C 8003EB9C 00000000 */   nop
  .L8003EBA0:
    /* 2F3A0 8003EBA0 23100400 */  negu       $v0, $a0
    /* 2F3A4 8003EBA4 20004228 */  slti       $v0, $v0, 0x20
    /* 2F3A8 8003EBA8 0D004010 */  beqz       $v0, .L8003EBE0
    /* 2F3AC 8003EBAC FFFF0324 */   addiu     $v1, $zero, -0x1
  .L8003EBB0:
    /* 2F3B0 8003EBB0 04008104 */  bgez       $a0, .L8003EBC4
    /* 2F3B4 8003EBB4 1F008324 */   addiu     $v1, $a0, 0x1F
    /* 2F3B8 8003EBB8 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 2F3BC 8003EBBC F8FA0008 */  j          .L8003EBE0
    /* 2F3C0 8003EBC0 06186200 */   srlv      $v1, $v0, $v1
  .L8003EBC4:
    /* 2F3C4 8003EBC4 03008014 */  bnez       $a0, .L8003EBD4
    /* 2F3C8 8003EBC8 20000224 */   addiu     $v0, $zero, 0x20
    /* 2F3CC 8003EBCC F8FA0008 */  j          .L8003EBE0
    /* 2F3D0 8003EBD0 21180000 */   addu      $v1, $zero, $zero
  .L8003EBD4:
    /* 2F3D4 8003EBD4 23104400 */  subu       $v0, $v0, $a0
    /* 2F3D8 8003EBD8 FFFF0324 */  addiu      $v1, $zero, -0x1
    /* 2F3DC 8003EBDC 04184300 */  sllv       $v1, $v1, $v0
  .L8003EBE0:
    /* 2F3E0 8003EBE0 FFFF0924 */  addiu      $t1, $zero, -0x1
    /* 2F3E4 8003EBE4 2120A003 */  addu       $a0, $sp, $zero
    /* 2F3E8 8003EBE8 80100500 */  sll        $v0, $a1, 2
    /* 2F3EC 8003EBEC 21405D00 */  addu       $t0, $v0, $sp
    /* 2F3F0 8003EBF0 1F00A224 */  addiu      $v0, $a1, 0x1F
    /* 2F3F4 8003EBF4 80100200 */  sll        $v0, $v0, 2
    /* 2F3F8 8003EBF8 21385D00 */  addu       $a3, $v0, $sp
    /* 2F3FC 8003EBFC 8000A527 */  addiu      $a1, $sp, 0x80
    /* 2F400 8003EC00 2A108800 */  slt        $v0, $a0, $t0
  .L8003EC04:
    /* 2F404 8003EC04 05004014 */  bnez       $v0, .L8003EC1C
    /* 2F408 8003EC08 2A108700 */   slt       $v0, $a0, $a3
    /* 2F40C 8003EC0C 03004010 */  beqz       $v0, .L8003EC1C
    /* 2F410 8003EC10 00000000 */   nop
    /* 2F414 8003EC14 08FB0008 */  j          .L8003EC20
    /* 2F418 8003EC18 000083AC */   sw        $v1, 0x0($a0)
  .L8003EC1C:
    /* 2F41C 8003EC1C 000089AC */  sw         $t1, 0x0($a0)
  .L8003EC20:
    /* 2F420 8003EC20 04008424 */  addiu      $a0, $a0, 0x4
    /* 2F424 8003EC24 2A108500 */  slt        $v0, $a0, $a1
    /* 2F428 8003EC28 F6FF4014 */  bnez       $v0, .L8003EC04
    /* 2F42C 8003EC2C 2A108800 */   slt       $v0, $a0, $t0
    /* 2F430 8003EC30 21600000 */  addu       $t4, $zero, $zero
    /* 2F434 8003EC34 0B80183C */  lui        $t8, %hi(D_800A8FB0)
    /* 2F438 8003EC38 B08F1827 */  addiu      $t8, $t8, %lo(D_800A8FB0)
    /* 2F43C 8003EC3C 0B800F3C */  lui        $t7, %hi(D_800A87E0)
    /* 2F440 8003EC40 E087EF25 */  addiu      $t7, $t7, %lo(D_800A87E0)
    /* 2F444 8003EC44 0A800E3C */  lui        $t6, %hi(D_800A7FE0)
    /* 2F448 8003EC48 E07FCE25 */  addiu      $t6, $t6, %lo(D_800A7FE0)
    /* 2F44C 8003EC4C 2168A003 */  addu       $t5, $sp, $zero
  .L8003EC50:
    /* 2F450 8003EC50 0000AA8D */  lw         $t2, 0x0($t5)
    /* 2F454 8003EC54 00000000 */  nop
    /* 2F458 8003EC58 50004011 */  beqz       $t2, .L8003ED9C
    /* 2F45C 8003EC5C 00000000 */   nop
    /* 2F460 8003EC60 21480000 */  addu       $t1, $zero, $zero
    /* 2F464 8003EC64 2158C001 */  addu       $t3, $t6, $zero
  .L8003EC68:
    /* 2F468 8003EC68 47004105 */  bgez       $t2, .L8003ED88
    /* 2F46C 8003EC6C 00000000 */   nop
    /* 2F470 8003EC70 00006285 */  lh         $v0, 0x0($t3)
    /* 2F474 8003EC74 00000000 */  nop
    /* 2F478 8003EC78 43004004 */  bltz       $v0, .L8003ED88
    /* 2F47C 8003EC7C 21284000 */   addu      $a1, $v0, $zero
    /* 2F480 8003EC80 40110C00 */  sll        $v0, $t4, 5
    /* 2F484 8003EC84 21104900 */  addu       $v0, $v0, $t1
    /* 2F488 8003EC88 21105800 */  addu       $v0, $v0, $t8
    /* 2F48C 8003EC8C 00004790 */  lbu        $a3, 0x0($v0)
    /* 2F490 8003EC90 2110A000 */  addu       $v0, $a1, $zero
  .L8003EC94:
    /* 2F494 8003EC94 00140200 */  sll        $v0, $v0, 16
    /* 2F498 8003EC98 C3130200 */  sra        $v0, $v0, 15
    /* 2F49C 8003EC9C 21104F00 */  addu       $v0, $v0, $t7
    /* 2F4A0 8003ECA0 00004894 */  lhu        $t0, 0x0($v0)
    /* 2F4A4 8003ECA4 9C02848F */  lw         $a0, %gp_rel(D_800A3368)($gp)
    /* 2F4A8 8003ECA8 FF7F0331 */  andi       $v1, $t0, 0x7FFF
    /* 2F4AC 8003ECAC 2A106400 */  slt        $v0, $v1, $a0
    /* 2F4B0 8003ECB0 21004010 */  beqz       $v0, .L8003ED38
    /* 2F4B4 8003ECB4 0100A524 */   addiu     $a1, $a1, 0x1
    /* 2F4B8 8003ECB8 00190300 */  sll        $v1, $v1, 4
    /* 2F4BC 8003ECBC 0A80023C */  lui        $v0, %hi(D_800A4750)
    /* 2F4C0 8003ECC0 50474224 */  addiu      $v0, $v0, %lo(D_800A4750)
    /* 2F4C4 8003ECC4 21206200 */  addu       $a0, $v1, $v0
    /* 2F4C8 8003ECC8 0300E230 */  andi       $v0, $a3, 0x3
    /* 2F4CC 8003ECCC 060082A0 */  sb         $v0, 0x6($a0)
    /* 2F4D0 8003ECD0 0800E230 */  andi       $v0, $a3, 0x8
    /* 2F4D4 8003ECD4 09004014 */  bnez       $v0, .L8003ECFC
    /* 2F4D8 8003ECD8 00000000 */   nop
    /* 2F4DC 8003ECDC 0400E230 */  andi       $v0, $a3, 0x4
    /* 2F4E0 8003ECE0 09004010 */  beqz       $v0, .L8003ED08
    /* 2F4E4 8003ECE4 00000000 */   nop
    /* 2F4E8 8003ECE8 07008290 */  lbu        $v0, 0x7($a0)
    /* 2F4EC 8003ECEC 00000000 */  nop
    /* 2F4F0 8003ECF0 08004230 */  andi       $v0, $v0, 0x8
    /* 2F4F4 8003ECF4 04004010 */  beqz       $v0, .L8003ED08
    /* 2F4F8 8003ECF8 00000000 */   nop
  .L8003ECFC:
    /* 2F4FC 8003ECFC 07008290 */  lbu        $v0, 0x7($a0)
    /* 2F500 8003ED00 45FB0008 */  j          .L8003ED14
    /* 2F504 8003ED04 01004234 */   ori       $v0, $v0, 0x1
  .L8003ED08:
    /* 2F508 8003ED08 07008290 */  lbu        $v0, 0x7($a0)
    /* 2F50C 8003ED0C 00000000 */  nop
    /* 2F510 8003ED10 FE004230 */  andi       $v0, $v0, 0xFE
  .L8003ED14:
    /* 2F514 8003ED14 070082A0 */  sb         $v0, 0x7($a0)
    /* 2F518 8003ED18 0A80033C */  lui        $v1, %hi(D_800A3820)
    /* 2F51C 8003ED1C 2038638C */  lw         $v1, %lo(D_800A3820)($v1)
    /* 2F520 8003ED20 00000000 */  nop
    /* 2F524 8003ED24 04006224 */  addiu      $v0, $v1, 0x4
    /* 2F528 8003ED28 0A80013C */  lui        $at, %hi(D_800A3820)
    /* 2F52C 8003ED2C 203822AC */  sw         $v0, %lo(D_800A3820)($at)
    /* 2F530 8003ED30 5FFB0008 */  j          .L8003ED7C
    /* 2F534 8003ED34 000064AC */   sw        $a0, 0x0($v1)
  .L8003ED38:
    /* 2F538 8003ED38 23186400 */  subu       $v1, $v1, $a0
    /* 2F53C 8003ED3C 40100300 */  sll        $v0, $v1, 1
    /* 2F540 8003ED40 21104300 */  addu       $v0, $v0, $v1
    /* 2F544 8003ED44 80100200 */  sll        $v0, $v0, 2
    /* 2F548 8003ED48 21104300 */  addu       $v0, $v0, $v1
    /* 2F54C 8003ED4C C0100200 */  sll        $v0, $v0, 3
    /* 2F550 8003ED50 0A80033C */  lui        $v1, %hi(D_800A6690)
    /* 2F554 8003ED54 90666324 */  addiu      $v1, $v1, %lo(D_800A6690)
    /* 2F558 8003ED58 21184300 */  addu       $v1, $v0, $v1
    /* 2F55C 8003ED5C 58006290 */  lbu        $v0, 0x58($v1)
    /* 2F560 8003ED60 00000000 */  nop
    /* 2F564 8003ED64 06004014 */  bnez       $v0, .L8003ED80
    /* 2F568 8003ED68 00800231 */   andi      $v0, $t0, 0x8000
    /* 2F56C 8003ED6C 0000C3AC */  sw         $v1, 0x0($a2)
    /* 2F570 8003ED70 0400C624 */  addiu      $a2, $a2, 0x4
    /* 2F574 8003ED74 01000224 */  addiu      $v0, $zero, 0x1
    /* 2F578 8003ED78 580062A0 */  sb         $v0, 0x58($v1)
  .L8003ED7C:
    /* 2F57C 8003ED7C 00800231 */  andi       $v0, $t0, 0x8000
  .L8003ED80:
    /* 2F580 8003ED80 C4FF4010 */  beqz       $v0, .L8003EC94
    /* 2F584 8003ED84 2110A000 */   addu      $v0, $a1, $zero
  .L8003ED88:
    /* 2F588 8003ED88 40500A00 */  sll        $t2, $t2, 1
    /* 2F58C 8003ED8C 01002925 */  addiu      $t1, $t1, 0x1
    /* 2F590 8003ED90 20002229 */  slti       $v0, $t1, 0x20
    /* 2F594 8003ED94 B4FF4014 */  bnez       $v0, .L8003EC68
    /* 2F598 8003ED98 02006B25 */   addiu     $t3, $t3, 0x2
  .L8003ED9C:
    /* 2F59C 8003ED9C 4000CE25 */  addiu      $t6, $t6, 0x40
    /* 2F5A0 8003EDA0 01008C25 */  addiu      $t4, $t4, 0x1
    /* 2F5A4 8003EDA4 20008229 */  slti       $v0, $t4, 0x20
    /* 2F5A8 8003EDA8 A9FF4014 */  bnez       $v0, .L8003EC50
    /* 2F5AC 8003EDAC 0400AD25 */   addiu     $t5, $t5, 0x4
    /* 2F5B0 8003EDB0 2110C000 */  addu       $v0, $a2, $zero
    /* 2F5B4 8003EDB4 8800BD27 */  addiu      $sp, $sp, 0x88
    /* 2F5B8 8003EDB8 0800E003 */  jr         $ra
    /* 2F5BC 8003EDBC 00000000 */   nop
endlabel func_8003EB84
