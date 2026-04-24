glabel func_8008BC60
    /* 7C460 8008BC60 21C08000 */  addu       $t8, $a0, $zero
    /* 7C464 8008BC64 2120C000 */  addu       $a0, $a2, $zero
    /* 7C468 8008BC68 27300600 */  nor        $a2, $zero, $a2
    /* 7C46C 8008BC6C 21180000 */  addu       $v1, $zero, $zero
    /* 7C470 8008BC70 0F000A24 */  addiu      $t2, $zero, 0xF
    /* 7C474 8008BC74 FFFFC630 */  andi       $a2, $a2, 0xFFFF
    /* 7C478 8008BC78 07104601 */  srav       $v0, $a2, $t2
  .L8008BC7C:
    /* 7C47C 8008BC7C 01004230 */  andi       $v0, $v0, 0x1
    /* 7C480 8008BC80 03004014 */  bnez       $v0, .L8008BC90
    /* 7C484 8008BC84 00000000 */   nop
    /* 7C488 8008BC88 272F0208 */  j          .L8008BC9C
    /* 7C48C 8008BC8C 21184001 */   addu      $v1, $t2, $zero
  .L8008BC90:
    /* 7C490 8008BC90 FFFF4A25 */  addiu      $t2, $t2, -0x1
    /* 7C494 8008BC94 F9FF4105 */  bgez       $t2, .L8008BC7C
    /* 7C498 8008BC98 07104601 */   srav      $v0, $a2, $t2
  .L8008BC9C:
    /* 7C49C 8008BC9C F4FF6F24 */  addiu      $t7, $v1, -0xC
    /* 7C4A0 8008BCA0 01000224 */  addiu      $v0, $zero, 0x1
    /* 7C4A4 8008BCA4 04706200 */  sllv       $t6, $v0, $v1
    /* 7C4A8 8008BCA8 00100824 */  addiu      $t0, $zero, 0x1000
    /* 7C4AC 8008BCAC 21500000 */  addu       $t2, $zero, $zero
    /* 7C4B0 8008BCB0 FFFF8630 */  andi       $a2, $a0, 0xFFFF
    /* 7C4B4 8008BCB4 1800C801 */  mult       $t6, $t0
  .L8008BCB8:
    /* 7C4B8 8008BCB8 80110800 */  sll        $v0, $t0, 6
    /* 7C4BC 8008BCBC 21104800 */  addu       $v0, $v0, $t0
    /* 7C4C0 8008BCC0 00110200 */  sll        $v0, $v0, 4
    /* 7C4C4 8008BCC4 23104800 */  subu       $v0, $v0, $t0
    /* 7C4C8 8008BCC8 80100200 */  sll        $v0, $v0, 2
    /* 7C4CC 8008BCCC 12600000 */  mflo       $t4
    /* 7C4D0 8008BCD0 23404800 */  subu       $t0, $v0, $t0
    /* 7C4D4 8008BCD4 02430800 */  srl        $t0, $t0, 12
    /* 7C4D8 8008BCD8 1800C801 */  mult       $t6, $t0
    /* 7C4DC 8008BCDC 21200000 */  addu       $a0, $zero, $zero
    /* 7C4E0 8008BCE0 40690A00 */  sll        $t5, $t2, 5
    /* 7C4E4 8008BCE4 21580000 */  addu       $t3, $zero, $zero
    /* 7C4E8 8008BCE8 12180000 */  mflo       $v1
    /* 7C4EC 8008BCEC 23106C00 */  subu       $v0, $v1, $t4
    /* 7C4F0 8008BCF0 42490200 */  srl        $t1, $v0, 5
    /* 7C4F4 8008BCF4 21382001 */  addu       $a3, $t1, $zero
  .L8008BCF8:
    /* 7C4F8 8008BCF8 21108B01 */  addu       $v0, $t4, $t3
    /* 7C4FC 8008BCFC 21188701 */  addu       $v1, $t4, $a3
    /* 7C500 8008BD00 02130200 */  srl        $v0, $v0, 12
    /* 7C504 8008BD04 2B10C200 */  sltu       $v0, $a2, $v0
    /* 7C508 8008BD08 04004014 */  bnez       $v0, .L8008BD1C
    /* 7C50C 8008BD0C 021B0300 */   srl       $v1, $v1, 12
    /* 7C510 8008BD10 2B10C300 */  sltu       $v0, $a2, $v1
    /* 7C514 8008BD14 0B004014 */  bnez       $v0, .L8008BD44
    /* 7C518 8008BD18 2110A401 */   addu      $v0, $t5, $a0
  .L8008BD1C:
    /* 7C51C 8008BD1C 2138E900 */  addu       $a3, $a3, $t1
    /* 7C520 8008BD20 01008424 */  addiu      $a0, $a0, 0x1
    /* 7C524 8008BD24 20008228 */  slti       $v0, $a0, 0x20
    /* 7C528 8008BD28 F3FF4014 */  bnez       $v0, .L8008BCF8
    /* 7C52C 8008BD2C 21586901 */   addu      $t3, $t3, $t1
    /* 7C530 8008BD30 01004A25 */  addiu      $t2, $t2, 0x1
    /* 7C534 8008BD34 30004229 */  slti       $v0, $t2, 0x30
    /* 7C538 8008BD38 DFFF4014 */  bnez       $v0, .L8008BCB8
    /* 7C53C 8008BD3C 1800C801 */   mult      $t6, $t0
    /* 7C540 8008BD40 00060224 */  addiu      $v0, $zero, 0x600
  .L8008BD44:
    /* 7C544 8008BD44 02004104 */  bgez       $v0, .L8008BD50
    /* 7C548 8008BD48 21184000 */   addu      $v1, $v0, $zero
    /* 7C54C 8008BD4C 7F004324 */  addiu      $v1, $v0, 0x7F
  .L8008BD50:
    /* 7C550 8008BD50 C3190300 */  sra        $v1, $v1, 7
    /* 7C554 8008BD54 C0210300 */  sll        $a0, $v1, 7
    /* 7C558 8008BD58 23204400 */  subu       $a0, $v0, $a0
    /* 7C55C 8008BD5C FFFF0233 */  andi       $v0, $t8, 0xFFFF
    /* 7C560 8008BD60 21104300 */  addu       $v0, $v0, $v1
    /* 7C564 8008BD64 40180F00 */  sll        $v1, $t7, 1
    /* 7C568 8008BD68 21186F00 */  addu       $v1, $v1, $t7
    /* 7C56C 8008BD6C 80180300 */  sll        $v1, $v1, 2
    /* 7C570 8008BD70 21104300 */  addu       $v0, $v0, $v1
    /* 7C574 8008BD74 FFFFA330 */  andi       $v1, $a1, 0xFFFF
    /* 7C578 8008BD78 21186400 */  addu       $v1, $v1, $a0
    /* 7C57C 8008BD7C 00120200 */  sll        $v0, $v0, 8
    /* 7C580 8008BD80 0800E003 */  jr         $ra
    /* 7C584 8008BD84 25104300 */   or        $v0, $v0, $v1
endlabel func_8008BC60
