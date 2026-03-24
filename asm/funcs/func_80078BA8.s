glabel func_80078BA8
    /* 693A8 80078BA8 FFFF8330 */  andi       $v1, $a0, 0xFFFF
    /* 693AC 80078BAC 03006228 */  slti       $v0, $v1, 0x3
    /* 693B0 80078BB0 08004010 */  beqz       $v0, .L80078BD4
    /* 693B4 80078BB4 01000224 */   addiu     $v0, $zero, 0x1
    /* 693B8 80078BB8 0A80043C */  lui        $a0, %hi(D_8009BD6C)
    /* 693BC 80078BBC 6CBD848C */  lw         $a0, %lo(D_8009BD6C)($a0)
    /* 693C0 80078BC0 00190300 */  sll        $v1, $v1, 4
    /* 693C4 80078BC4 21186400 */  addu       $v1, $v1, $a0
    /* 693C8 80078BC8 000060A4 */  sh         $zero, 0x0($v1)
    /* 693CC 80078BCC F6E20108 */  j          .L80078BD8
    /* 693D0 80078BD0 00000000 */   nop
  .L80078BD4:
    /* 693D4 80078BD4 21100000 */  addu       $v0, $zero, $zero
  .L80078BD8:
    /* 693D8 80078BD8 0800E003 */  jr         $ra
    /* 693DC 80078BDC 00000000 */   nop
endlabel func_80078BA8
