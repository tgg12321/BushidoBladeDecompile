glabel func_8007D3F8
    /* 6DBF8 8007D3F8 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* 6DBFC 8007D3FC 1C00B3AF */  sw         $s3, 0x1C($sp)
    /* 6DC00 8007D400 21988000 */  addu       $s3, $a0, $zero
    /* 6DC04 8007D404 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6DC08 8007D408 2180A000 */  addu       $s0, $a1, $zero
    /* 6DC0C 8007D40C 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6DC10 8007D410 2188C000 */  addu       $s1, $a2, $zero
    /* 6DC14 8007D414 1800B2AF */  sw         $s2, 0x18($sp)
    /* 6DC18 8007D418 2000BFAF */  sw         $ra, 0x20($sp)
    /* 6DC1C 8007D41C 1AF7010C */  jal        func_8007DC68
    /* 6DC20 8007D420 2190E000 */   addu      $s2, $a3, $zero
    /* 6DC24 8007D424 11F50108 */  j          .L8007D444
    /* 6DC28 8007D428 00000000 */   nop
  .L8007D42C:
    /* 6DC2C 8007D42C 27F7010C */  jal        func_8007DC9C
    /* 6DC30 8007D430 00000000 */   nop
    /* 6DC34 8007D434 A0004014 */  bnez       $v0, .L8007D6B8
    /* 6DC38 8007D438 FFFF0224 */   addiu     $v0, $zero, -0x1
    /* 6DC3C 8007D43C B6F5010C */  jal        func_8007D6D8
    /* 6DC40 8007D440 00000000 */   nop
  .L8007D444:
    /* 6DC44 8007D444 0A80023C */  lui        $v0, %hi(D_8009BF78)
    /* 6DC48 8007D448 78BF428C */  lw         $v0, %lo(D_8009BF78)($v0)
    /* 6DC4C 8007D44C 0A80033C */  lui        $v1, %hi(D_8009BF7C)
    /* 6DC50 8007D450 7CBF638C */  lw         $v1, %lo(D_8009BF7C)($v1)
    /* 6DC54 8007D454 01004224 */  addiu      $v0, $v0, 0x1
    /* 6DC58 8007D458 3F004230 */  andi       $v0, $v0, 0x3F
    /* 6DC5C 8007D45C F3FF4310 */  beq        $v0, $v1, .L8007D42C
    /* 6DC60 8007D460 00000000 */   nop
    /* 6DC64 8007D464 0F0B020C */  jal        func_80082C3C
    /* 6DC68 8007D468 21200000 */   addu      $a0, $zero, $zero
    /* 6DC6C 8007D46C 0A80033C */  lui        $v1, %hi(D_8009BE75)
    /* 6DC70 8007D470 75BE6390 */  lbu        $v1, %lo(D_8009BE75)($v1)
    /* 6DC74 8007D474 0A80013C */  lui        $at, %hi(D_8009BF80)
    /* 6DC78 8007D478 80BF22AC */  sw         $v0, %lo(D_8009BF80)($at)
    /* 6DC7C 8007D47C 01000224 */  addiu      $v0, $zero, 0x1
    /* 6DC80 8007D480 0A80013C */  lui        $at, %hi(D_8009BE7C)
    /* 6DC84 8007D484 7CBE22AC */  sw         $v0, %lo(D_8009BE7C)($at)
    /* 6DC88 8007D488 15006010 */  beqz       $v1, .L8007D4E0
    /* 6DC8C 8007D48C 00000000 */   nop
    /* 6DC90 8007D490 0A80033C */  lui        $v1, %hi(D_8009BF78)
    /* 6DC94 8007D494 78BF638C */  lw         $v1, %lo(D_8009BF78)($v1)
    /* 6DC98 8007D498 0A80023C */  lui        $v0, %hi(D_8009BF7C)
    /* 6DC9C 8007D49C 7CBF428C */  lw         $v0, %lo(D_8009BF7C)($v0)
    /* 6DCA0 8007D4A0 00000000 */  nop
    /* 6DCA4 8007D4A4 26006214 */  bne        $v1, $v0, .L8007D540
    /* 6DCA8 8007D4A8 00000000 */   nop
    /* 6DCAC 8007D4AC 0A80023C */  lui        $v0, %hi(D_8009BF54)
    /* 6DCB0 8007D4B0 54BF428C */  lw         $v0, %lo(D_8009BF54)($v0)
    /* 6DCB4 8007D4B4 00000000 */  nop
    /* 6DCB8 8007D4B8 0000428C */  lw         $v0, 0x0($v0)
    /* 6DCBC 8007D4BC 0001033C */  lui        $v1, (0x1000000 >> 16)
    /* 6DCC0 8007D4C0 24104300 */  and        $v0, $v0, $v1
    /* 6DCC4 8007D4C4 1E004014 */  bnez       $v0, .L8007D540
    /* 6DCC8 8007D4C8 00000000 */   nop
    /* 6DCCC 8007D4CC 0A80023C */  lui        $v0, %hi(D_8009BE80)
    /* 6DCD0 8007D4D0 80BE428C */  lw         $v0, %lo(D_8009BE80)($v0)
    /* 6DCD4 8007D4D4 00000000 */  nop
    /* 6DCD8 8007D4D8 19004014 */  bnez       $v0, .L8007D540
    /* 6DCDC 8007D4DC 00000000 */   nop
  .L8007D4E0:
    /* 6DCE0 8007D4E0 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6DCE4 8007D4E4 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6DCE8 8007D4E8 0004043C */  lui        $a0, (0x4000000 >> 16)
  .L8007D4EC:
    /* 6DCEC 8007D4EC 0000628C */  lw         $v0, 0x0($v1)
    /* 6DCF0 8007D4F0 00000000 */  nop
    /* 6DCF4 8007D4F4 24104400 */  and        $v0, $v0, $a0
    /* 6DCF8 8007D4F8 FCFF4010 */  beqz       $v0, .L8007D4EC
    /* 6DCFC 8007D4FC 00000000 */   nop
    /* 6DD00 8007D500 21200002 */  addu       $a0, $s0, $zero
    /* 6DD04 8007D504 09F86002 */  jalr       $s3
    /* 6DD08 8007D508 21284002 */   addu      $a1, $s2, $zero
    /* 6DD0C 8007D50C 0A80043C */  lui        $a0, %hi(D_8009BF80)
    /* 6DD10 8007D510 80BF848C */  lw         $a0, %lo(D_8009BF80)($a0)
    /* 6DD14 8007D514 0A80023C */  lui        $v0, %hi(D_8009BF68)
    /* 6DD18 8007D518 68BF4224 */  addiu      $v0, $v0, %lo(D_8009BF68)
    /* 6DD1C 8007D51C 000053AC */  sw         $s3, 0x0($v0)
    /* 6DD20 8007D520 0A80013C */  lui        $at, %hi(D_8009BF6C)
    /* 6DD24 8007D524 6CBF30AC */  sw         $s0, %lo(D_8009BF6C)($at)
    /* 6DD28 8007D528 0A80013C */  lui        $at, %hi(D_8009BF70)
    /* 6DD2C 8007D52C 70BF32AC */  sw         $s2, %lo(D_8009BF70)($at)
    /* 6DD30 8007D530 0F0B020C */  jal        func_80082C3C
    /* 6DD34 8007D534 00000000 */   nop
    /* 6DD38 8007D538 AEF50108 */  j          .L8007D6B8
    /* 6DD3C 8007D53C 21100000 */   addu      $v0, $zero, $zero
  .L8007D540:
    /* 6DD40 8007D540 0880053C */  lui        $a1, %hi(func_8007D6D8)
    /* 6DD44 8007D544 D8D6A524 */  addiu      $a1, $a1, %lo(func_8007D6D8)
    /* 6DD48 8007D548 C80A020C */  jal        irq_AcknowledgeVblank
    /* 6DD4C 8007D54C 02000424 */   addiu     $a0, $zero, 0x2
    /* 6DD50 8007D550 2B002012 */  beqz       $s1, .L8007D600
    /* 6DD54 8007D554 21300000 */   addu      $a2, $zero, $zero
    /* 6DD58 8007D558 1080083C */  lui        $t0, %hi(D_8010368C)
    /* 6DD5C 8007D55C 8C360825 */  addiu      $t0, $t0, %lo(D_8010368C)
    /* 6DD60 8007D560 21380002 */  addu       $a3, $s0, $zero
    /* 6DD64 8007D564 21102002 */  addu       $v0, $s1, $zero
  .L8007D568:
    /* 6DD68 8007D568 02004104 */  bgez       $v0, .L8007D574
    /* 6DD6C 8007D56C 00000000 */   nop
    /* 6DD70 8007D570 03004224 */  addiu      $v0, $v0, 0x3
  .L8007D574:
    /* 6DD74 8007D574 83100200 */  sra        $v0, $v0, 2
    /* 6DD78 8007D578 2A10C200 */  slt        $v0, $a2, $v0
    /* 6DD7C 8007D57C 0E004010 */  beqz       $v0, .L8007D5B8
    /* 6DD80 8007D580 80200600 */   sll       $a0, $a2, 2
    /* 6DD84 8007D584 0000E58C */  lw         $a1, 0x0($a3)
    /* 6DD88 8007D588 0400E724 */  addiu      $a3, $a3, 0x4
    /* 6DD8C 8007D58C 0A80033C */  lui        $v1, %hi(D_8009BF78)
    /* 6DD90 8007D590 78BF638C */  lw         $v1, %lo(D_8009BF78)($v1)
    /* 6DD94 8007D594 0100C624 */  addiu      $a2, $a2, 0x1
    /* 6DD98 8007D598 40100300 */  sll        $v0, $v1, 1
    /* 6DD9C 8007D59C 21104300 */  addu       $v0, $v0, $v1
    /* 6DDA0 8007D5A0 40110200 */  sll        $v0, $v0, 5
    /* 6DDA4 8007D5A4 21104800 */  addu       $v0, $v0, $t0
    /* 6DDA8 8007D5A8 21208200 */  addu       $a0, $a0, $v0
    /* 6DDAC 8007D5AC 000085AC */  sw         $a1, 0x0($a0)
    /* 6DDB0 8007D5B0 5AF50108 */  j          .L8007D568
    /* 6DDB4 8007D5B4 21102002 */   addu      $v0, $s1, $zero
  .L8007D5B8:
    /* 6DDB8 8007D5B8 0A80023C */  lui        $v0, %hi(D_8009BF78)
    /* 6DDBC 8007D5BC 78BF428C */  lw         $v0, %lo(D_8009BF78)($v0)
    /* 6DDC0 8007D5C0 0A80033C */  lui        $v1, %hi(D_8009BF78)
    /* 6DDC4 8007D5C4 78BF638C */  lw         $v1, %lo(D_8009BF78)($v1)
    /* 6DDC8 8007D5C8 40200200 */  sll        $a0, $v0, 1
    /* 6DDCC 8007D5CC 21208200 */  addu       $a0, $a0, $v0
    /* 6DDD0 8007D5D0 40210400 */  sll        $a0, $a0, 5
    /* 6DDD4 8007D5D4 40100300 */  sll        $v0, $v1, 1
    /* 6DDD8 8007D5D8 21104300 */  addu       $v0, $v0, $v1
    /* 6DDDC 8007D5DC 40110200 */  sll        $v0, $v0, 5
    /* 6DDE0 8007D5E0 1080033C */  lui        $v1, %hi(D_8010368C)
    /* 6DDE4 8007D5E4 8C366324 */  addiu      $v1, $v1, %lo(D_8010368C)
    /* 6DDE8 8007D5E8 21104300 */  addu       $v0, $v0, $v1
    /* 6DDEC 8007D5EC 1080013C */  lui        $at, %hi(D_80103684)
    /* 6DDF0 8007D5F0 21082400 */  addu       $at, $at, $a0
    /* 6DDF4 8007D5F4 843622AC */  sw         $v0, %lo(D_80103684)($at)
    /* 6DDF8 8007D5F8 89F50108 */  j          .L8007D624
    /* 6DDFC 8007D5FC 00000000 */   nop
  .L8007D600:
    /* 6DE00 8007D600 0A80033C */  lui        $v1, %hi(D_8009BF78)
    /* 6DE04 8007D604 78BF638C */  lw         $v1, %lo(D_8009BF78)($v1)
    /* 6DE08 8007D608 00000000 */  nop
    /* 6DE0C 8007D60C 40100300 */  sll        $v0, $v1, 1
    /* 6DE10 8007D610 21104300 */  addu       $v0, $v0, $v1
    /* 6DE14 8007D614 40110200 */  sll        $v0, $v0, 5
    /* 6DE18 8007D618 1080013C */  lui        $at, %hi(D_80103684)
    /* 6DE1C 8007D61C 21082200 */  addu       $at, $at, $v0
    /* 6DE20 8007D620 843630AC */  sw         $s0, %lo(D_80103684)($at)
  .L8007D624:
    /* 6DE24 8007D624 0A80033C */  lui        $v1, %hi(D_8009BF78)
    /* 6DE28 8007D628 78BF638C */  lw         $v1, %lo(D_8009BF78)($v1)
    /* 6DE2C 8007D62C 00000000 */  nop
    /* 6DE30 8007D630 40100300 */  sll        $v0, $v1, 1
    /* 6DE34 8007D634 21104300 */  addu       $v0, $v0, $v1
    /* 6DE38 8007D638 40110200 */  sll        $v0, $v0, 5
    /* 6DE3C 8007D63C 1080013C */  lui        $at, %hi(D_80103688)
    /* 6DE40 8007D640 21082200 */  addu       $at, $at, $v0
    /* 6DE44 8007D644 883632AC */  sw         $s2, %lo(D_80103688)($at)
    /* 6DE48 8007D648 0A80033C */  lui        $v1, %hi(D_8009BF78)
    /* 6DE4C 8007D64C 78BF638C */  lw         $v1, %lo(D_8009BF78)($v1)
    /* 6DE50 8007D650 00000000 */  nop
    /* 6DE54 8007D654 40100300 */  sll        $v0, $v1, 1
    /* 6DE58 8007D658 21104300 */  addu       $v0, $v0, $v1
    /* 6DE5C 8007D65C 40110200 */  sll        $v0, $v0, 5
    /* 6DE60 8007D660 1080013C */  lui        $at, %hi(D_80103680)
    /* 6DE64 8007D664 21082200 */  addu       $at, $at, $v0
    /* 6DE68 8007D668 803633AC */  sw         $s3, %lo(D_80103680)($at)
    /* 6DE6C 8007D66C 0A80023C */  lui        $v0, %hi(D_8009BF78)
    /* 6DE70 8007D670 78BF428C */  lw         $v0, %lo(D_8009BF78)($v0)
    /* 6DE74 8007D674 0A80043C */  lui        $a0, %hi(D_8009BF80)
    /* 6DE78 8007D678 80BF848C */  lw         $a0, %lo(D_8009BF80)($a0)
    /* 6DE7C 8007D67C 01004224 */  addiu      $v0, $v0, 0x1
    /* 6DE80 8007D680 3F004230 */  andi       $v0, $v0, 0x3F
    /* 6DE84 8007D684 0A80013C */  lui        $at, %hi(D_8009BF78)
    /* 6DE88 8007D688 78BF22AC */  sw         $v0, %lo(D_8009BF78)($at)
    /* 6DE8C 8007D68C 0F0B020C */  jal        func_80082C3C
    /* 6DE90 8007D690 00000000 */   nop
    /* 6DE94 8007D694 B6F5010C */  jal        func_8007D6D8
    /* 6DE98 8007D698 00000000 */   nop
    /* 6DE9C 8007D69C 0A80023C */  lui        $v0, %hi(D_8009BF78)
    /* 6DEA0 8007D6A0 78BF428C */  lw         $v0, %lo(D_8009BF78)($v0)
    /* 6DEA4 8007D6A4 0A80033C */  lui        $v1, %hi(D_8009BF7C)
    /* 6DEA8 8007D6A8 7CBF638C */  lw         $v1, %lo(D_8009BF7C)($v1)
    /* 6DEAC 8007D6AC 00000000 */  nop
    /* 6DEB0 8007D6B0 23104300 */  subu       $v0, $v0, $v1
    /* 6DEB4 8007D6B4 3F004230 */  andi       $v0, $v0, 0x3F
  .L8007D6B8:
    /* 6DEB8 8007D6B8 2000BF8F */  lw         $ra, 0x20($sp)
    /* 6DEBC 8007D6BC 1C00B38F */  lw         $s3, 0x1C($sp)
    /* 6DEC0 8007D6C0 1800B28F */  lw         $s2, 0x18($sp)
    /* 6DEC4 8007D6C4 1400B18F */  lw         $s1, 0x14($sp)
    /* 6DEC8 8007D6C8 1000B08F */  lw         $s0, 0x10($sp)
    /* 6DECC 8007D6CC 2800BD27 */  addiu      $sp, $sp, 0x28
    /* 6DED0 8007D6D0 0800E003 */  jr         $ra
    /* 6DED4 8007D6D4 00000000 */   nop
endlabel func_8007D3F8
