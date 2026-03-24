glabel func_8007ABB8
    /* 6B3B8 8007ABB8 21408000 */  addu       $t0, $a0, $zero
    /* 6B3BC 8007ABBC 0400A284 */  lh         $v0, 0x4($a1)
    /* 6B3C0 8007ABC0 00000000 */  nop
    /* 6B3C4 8007ABC4 05004010 */  beqz       $v0, .L8007ABDC
    /* 6B3C8 8007ABC8 05000424 */   addiu     $a0, $zero, 0x5
    /* 6B3CC 8007ABCC 0600A284 */  lh         $v0, 0x6($a1)
    /* 6B3D0 8007ABD0 00000000 */  nop
    /* 6B3D4 8007ABD4 03004014 */  bnez       $v0, .L8007ABE4
    /* 6B3D8 8007ABD8 0001023C */   lui       $v0, (0x1000000 >> 16)
  .L8007ABDC:
    /* 6B3DC 8007ABDC 21200000 */  addu       $a0, $zero, $zero
    /* 6B3E0 8007ABE0 0001023C */  lui        $v0, (0x1000000 >> 16)
  .L8007ABE4:
    /* 6B3E4 8007ABE4 040002AD */  sw         $v0, 0x4($t0)
    /* 6B3E8 8007ABE8 0080023C */  lui        $v0, (0x80000000 >> 16)
    /* 6B3EC 8007ABEC 080002AD */  sw         $v0, 0x8($t0)
    /* 6B3F0 8007ABF0 00140700 */  sll        $v0, $a3, 16
    /* 6B3F4 8007ABF4 FFFFC330 */  andi       $v1, $a2, 0xFFFF
    /* 6B3F8 8007ABF8 030004A1 */  sb         $a0, 0x3($t0)
    /* 6B3FC 8007ABFC 0000A48C */  lw         $a0, 0x0($a1)
    /* 6B400 8007AC00 25104300 */  or         $v0, $v0, $v1
    /* 6B404 8007AC04 100002AD */  sw         $v0, 0x10($t0)
    /* 6B408 8007AC08 0C0004AD */  sw         $a0, 0xC($t0)
    /* 6B40C 8007AC0C 0400A28C */  lw         $v0, 0x4($a1)
    /* 6B410 8007AC10 0800E003 */  jr         $ra
    /* 6B414 8007AC14 140002AD */   sw        $v0, 0x14($t0)
endlabel func_8007ABB8
