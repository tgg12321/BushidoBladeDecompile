glabel func_8001B478
    /* BC78 8001B478 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* BC7C 8001B47C 2C00B1AF */  sw         $s1, 0x2C($sp)
    /* BC80 8001B480 21888000 */  addu       $s1, $a0, $zero
    /* BC84 8001B484 21200000 */  addu       $a0, $zero, $zero
    /* BC88 8001B488 3400BFAF */  sw         $ra, 0x34($sp)
    /* BC8C 8001B48C 3000B2AF */  sw         $s2, 0x30($sp)
    /* BC90 8001B490 79FC000C */  jal        func_8003F1E4
    /* BC94 8001B494 2800B0AF */   sw        $s0, 0x28($sp)
    /* BC98 8001B498 9C01228E */  lw         $v0, 0x19C($s1)
    /* BC9C 8001B49C A801238E */  lw         $v1, 0x1A8($s1)
    /* BCA0 8001B4A0 0F80123C */  lui        $s2, %hi(D_800F5328)
    /* BCA4 8001B4A4 28535226 */  addiu      $s2, $s2, %lo(D_800F5328)
    /* BCA8 8001B4A8 21104300 */  addu       $v0, $v0, $v1
    /* BCAC 8001B4AC C21F0200 */  srl        $v1, $v0, 31
    /* BCB0 8001B4B0 21104300 */  addu       $v0, $v0, $v1
    /* BCB4 8001B4B4 8401238E */  lw         $v1, 0x184($s1)
    /* BCB8 8001B4B8 43100200 */  sra        $v0, $v0, 1
    /* BCBC 8001B4BC 23804300 */  subu       $s0, $v0, $v1
    /* BCC0 8001B4C0 9103022A */  slti       $v0, $s0, 0x391
    /* BCC4 8001B4C4 01004438 */  xori       $a0, $v0, 0x1
    /* BCC8 8001B4C8 6A002396 */  lhu        $v1, 0x6A($s1)
    /* BCCC 8001B4CC 2A000224 */  addiu      $v0, $zero, 0x2A
    /* BCD0 8001B4D0 40006210 */  beq        $v1, $v0, .L8001B5D4
    /* BCD4 8001B4D4 00021024 */   addiu     $s0, $zero, 0x200
    /* BCD8 8001B4D8 8001228E */  lw         $v0, 0x180($s1)
    /* BCDC 8001B4DC 00000000 */  nop
    /* BCE0 8001B4E0 000042AE */  sw         $v0, 0x0($s2)
    /* BCE4 8001B4E4 8801228E */  lw         $v0, 0x188($s1)
    /* BCE8 8001B4E8 0F80013C */  lui        $at, %hi(D_800F5330)
    /* BCEC 8001B4EC 305322AC */  sw         $v0, %lo(D_800F5330)($at)
    /* BCF0 8001B4F0 8401308E */  lw         $s0, 0x184($s1)
    /* BCF4 8001B4F4 10008014 */  bnez       $a0, .L8001B538
    /* BCF8 8001B4F8 00000000 */   nop
    /* BCFC 8001B4FC 1A002386 */  lh         $v1, 0x1A($s1)
    /* BD00 8001B500 00000000 */  nop
    /* BD04 8001B504 00110300 */  sll        $v0, $v1, 4
    /* BD08 8001B508 23104300 */  subu       $v0, $v0, $v1
    /* BD0C 8001B50C C0100200 */  sll        $v0, $v0, 3
    /* BD10 8001B510 23104300 */  subu       $v0, $v0, $v1
    /* BD14 8001B514 80100200 */  sll        $v0, $v0, 2
    /* BD18 8001B518 23104300 */  subu       $v0, $v0, $v1
    /* BD1C 8001B51C 40100200 */  sll        $v0, $v0, 1
    /* BD20 8001B520 23100200 */  negu       $v0, $v0
    /* BD24 8001B524 02004104 */  bgez       $v0, .L8001B530
    /* BD28 8001B528 00000000 */   nop
    /* BD2C 8001B52C FF0F4224 */  addiu      $v0, $v0, 0xFFF
  .L8001B530:
    /* BD30 8001B530 03130200 */  sra        $v0, $v0, 12
    /* BD34 8001B534 21800202 */  addu       $s0, $s0, $v0
  .L8001B538:
    /* BD38 8001B538 0400438E */  lw         $v1, 0x4($s2)
    /* BD3C 8001B53C 00000000 */  nop
    /* BD40 8001B540 23100302 */  subu       $v0, $s0, $v1
    /* BD44 8001B544 02004104 */  bgez       $v0, .L8001B550
    /* BD48 8001B548 00000000 */   nop
    /* BD4C 8001B54C 03004224 */  addiu      $v0, $v0, 0x3
  .L8001B550:
    /* BD50 8001B550 83100200 */  sra        $v0, $v0, 2
    /* BD54 8001B554 21306200 */  addu       $a2, $v1, $v0
    /* BD58 8001B558 040046AE */  sw         $a2, 0x4($s2)
    /* BD5C 8001B55C 6A002396 */  lhu        $v1, 0x6A($s1)
    /* BD60 8001B560 2A000224 */  addiu      $v0, $zero, 0x2A
    /* BD64 8001B564 1B006210 */  beq        $v1, $v0, .L8001B5D4
    /* BD68 8001B568 00021024 */   addiu     $s0, $zero, 0x200
    /* BD6C 8001B56C D8012286 */  lh         $v0, 0x1D8($s1)
    /* BD70 8001B570 12004386 */  lh         $v1, 0x12($s2)
    /* BD74 8001B574 23100200 */  negu       $v0, $v0
    /* BD78 8001B578 23104300 */  subu       $v0, $v0, $v1
    /* BD7C 8001B57C FF0F5030 */  andi       $s0, $v0, 0xFFF
    /* BD80 8001B580 0008022A */  slti       $v0, $s0, 0x800
    /* BD84 8001B584 04004014 */  bnez       $v0, .L8001B598
    /* BD88 8001B588 0004022A */   slti      $v0, $s0, 0x400
    /* BD8C 8001B58C 00100224 */  addiu      $v0, $zero, 0x1000
    /* BD90 8001B590 23805000 */  subu       $s0, $v0, $s0
    /* BD94 8001B594 0004022A */  slti       $v0, $s0, 0x400
  .L8001B598:
    /* BD98 8001B598 02004014 */  bnez       $v0, .L8001B5A4
    /* BD9C 8001B59C 00000000 */   nop
    /* BDA0 8001B5A0 00041024 */  addiu      $s0, $zero, 0x400
  .L8001B5A4:
    /* BDA4 8001B5A4 0000228E */  lw         $v0, 0x0($s1)
    /* BDA8 8001B5A8 00000000 */  nop
    /* BDAC 8001B5AC F800448C */  lw         $a0, 0xF8($v0)
    /* BDB0 8001B5B0 0A80053C */  lui        $a1, %hi(D_800A387C)
    /* BDB4 8001B5B4 7C38A58C */  lw         $a1, %lo(D_800A387C)($a1)
    /* BDB8 8001B5B8 57FF010C */  jal        func_8007FD5C
    /* BDBC 8001B5BC 23208600 */   subu      $a0, $a0, $a2
    /* BDC0 8001B5C0 00040324 */  addiu      $v1, $zero, 0x400
    /* BDC4 8001B5C4 23187000 */  subu       $v1, $v1, $s0
    /* BDC8 8001B5C8 18004300 */  mult       $v0, $v1
    /* BDCC 8001B5CC 12380000 */  mflo       $a3
    /* BDD0 8001B5D0 83820700 */  sra        $s0, $a3, 10
  .L8001B5D4:
    /* BDD4 8001B5D4 10004286 */  lh         $v0, 0x10($s2)
    /* BDD8 8001B5D8 00000000 */  nop
    /* BDDC 8001B5DC 23180202 */  subu       $v1, $s0, $v0
    /* BDE0 8001B5E0 02006104 */  bgez       $v1, .L8001B5EC
    /* BDE4 8001B5E4 21204000 */   addu      $a0, $v0, $zero
    /* BDE8 8001B5E8 07006324 */  addiu      $v1, $v1, 0x7
  .L8001B5EC:
    /* BDEC 8001B5EC C3100300 */  sra        $v0, $v1, 3
    /* BDF0 8001B5F0 21108200 */  addu       $v0, $a0, $v0
    /* BDF4 8001B5F4 100042A6 */  sh         $v0, 0x10($s2)
    /* BDF8 8001B5F8 140040A6 */  sh         $zero, 0x14($s2)
    /* BDFC 8001B5FC CA012286 */  lh         $v0, 0x1CA($s1)
    /* BE00 8001B600 0A80033C */  lui        $v1, %hi(D_800A36FC)
    /* BE04 8001B604 FC366384 */  lh         $v1, %lo(D_800A36FC)($v1)
    /* BE08 8001B608 23800200 */  negu       $s0, $v0
    /* BE0C 8001B60C 16006010 */  beqz       $v1, .L8001B668
    /* BE10 8001B610 21206000 */   addu      $a0, $v1, $zero
    /* BE14 8001B614 12004286 */  lh         $v0, 0x12($s2)
    /* BE18 8001B618 00000000 */  nop
    /* BE1C 8001B61C 23180202 */  subu       $v1, $s0, $v0
    /* BE20 8001B620 02006104 */  bgez       $v1, .L8001B62C
    /* BE24 8001B624 21284000 */   addu      $a1, $v0, $zero
    /* BE28 8001B628 03006324 */  addiu      $v1, $v1, 0x3
  .L8001B62C:
    /* BE2C 8001B62C FFFF8424 */  addiu      $a0, $a0, -0x1
    /* BE30 8001B630 83100300 */  sra        $v0, $v1, 2
    /* BE34 8001B634 1C004386 */  lh         $v1, 0x1C($s2)
    /* BE38 8001B638 2110A200 */  addu       $v0, $a1, $v0
    /* BE3C 8001B63C 120042A6 */  sh         $v0, 0x12($s2)
    /* BE40 8001B640 0A80013C */  lui        $at, %hi(D_800A36FC)
    /* BE44 8001B644 FC3624A4 */  sh         $a0, %lo(D_800A36FC)($at)
    /* BE48 8001B648 40100300 */  sll        $v0, $v1, 1
    /* BE4C 8001B64C 21104300 */  addu       $v0, $v0, $v1
    /* BE50 8001B650 02004104 */  bgez       $v0, .L8001B65C
    /* BE54 8001B654 00000000 */   nop
    /* BE58 8001B658 03004224 */  addiu      $v0, $v0, 0x3
  .L8001B65C:
    /* BE5C 8001B65C 83100200 */  sra        $v0, $v0, 2
    /* BE60 8001B660 9C6D0008 */  j          .L8001B670
    /* BE64 8001B664 1C0042A6 */   sh        $v0, 0x1C($s2)
  .L8001B668:
    /* BE68 8001B668 120050A6 */  sh         $s0, 0x12($s2)
    /* BE6C 8001B66C 1C0040A6 */  sh         $zero, 0x1C($s2)
  .L8001B670:
    /* BE70 8001B670 180040AE */  sw         $zero, 0x18($s2)
    /* BE74 8001B674 3400BF8F */  lw         $ra, 0x34($sp)
    /* BE78 8001B678 3000B28F */  lw         $s2, 0x30($sp)
    /* BE7C 8001B67C 2C00B18F */  lw         $s1, 0x2C($sp)
    /* BE80 8001B680 2800B08F */  lw         $s0, 0x28($sp)
    /* BE84 8001B684 3800BD27 */  addiu      $sp, $sp, 0x38
    /* BE88 8001B688 0800E003 */  jr         $ra
    /* BE8C 8001B68C 00000000 */   nop
endlabel func_8001B478
