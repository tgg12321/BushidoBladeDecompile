glabel func_8007CAC8
    /* 6D2C8 8007CAC8 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* 6D2CC 8007CACC 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6D2D0 8007CAD0 2180A000 */  addu       $s0, $a1, $zero
    /* 6D2D4 8007CAD4 0A80053C */  lui        $a1, %hi(D_8009BF64)
    /* 6D2D8 8007CAD8 64BFA58C */  lw         $a1, %lo(D_8009BF64)($a1)
    /* 6D2DC 8007CADC 1800BFAF */  sw         $ra, 0x18($sp)
    /* 6D2E0 8007CAE0 1400B1AF */  sw         $s1, 0x14($sp)
    /* 6D2E4 8007CAE4 0000A28C */  lw         $v0, 0x0($a1)
    /* 6D2E8 8007CAE8 0008033C */  lui        $v1, (0x8000000 >> 16)
    /* 6D2EC 8007CAEC 25104300 */  or         $v0, $v0, $v1
    /* 6D2F0 8007CAF0 0000A2AC */  sw         $v0, 0x0($a1)
    /* 6D2F4 8007CAF4 0A80023C */  lui        $v0, %hi(D_8009BF60)
    /* 6D2F8 8007CAF8 60BF428C */  lw         $v0, %lo(D_8009BF60)($v0)
    /* 6D2FC 8007CAFC 00000000 */  nop
    /* 6D300 8007CB00 000040AC */  sw         $zero, 0x0($v0)
    /* 6D304 8007CB04 80101000 */  sll        $v0, $s0, 2
    /* 6D308 8007CB08 FCFF4224 */  addiu      $v0, $v0, -0x4
    /* 6D30C 8007CB0C 0A80033C */  lui        $v1, %hi(D_8009BF58)
    /* 6D310 8007CB10 58BF638C */  lw         $v1, %lo(D_8009BF58)($v1)
    /* 6D314 8007CB14 21208200 */  addu       $a0, $a0, $v0
    /* 6D318 8007CB18 000064AC */  sw         $a0, 0x0($v1)
    /* 6D31C 8007CB1C 0A80023C */  lui        $v0, %hi(D_8009BF5C)
    /* 6D320 8007CB20 5CBF428C */  lw         $v0, %lo(D_8009BF5C)($v0)
    /* 6D324 8007CB24 0011033C */  lui        $v1, (0x11000002 >> 16)
    /* 6D328 8007CB28 000050AC */  sw         $s0, 0x0($v0)
    /* 6D32C 8007CB2C 0A80023C */  lui        $v0, %hi(D_8009BF60)
    /* 6D330 8007CB30 60BF428C */  lw         $v0, %lo(D_8009BF60)($v0)
    /* 6D334 8007CB34 02006334 */  ori        $v1, $v1, (0x11000002 & 0xFFFF)
    /* 6D338 8007CB38 000043AC */  sw         $v1, 0x0($v0)
    /* 6D33C 8007CB3C 1AF7010C */  jal        func_8007DC68
    /* 6D340 8007CB40 00000000 */   nop
    /* 6D344 8007CB44 0A80023C */  lui        $v0, %hi(D_8009BF60)
    /* 6D348 8007CB48 60BF428C */  lw         $v0, %lo(D_8009BF60)($v0)
    /* 6D34C 8007CB4C 00000000 */  nop
    /* 6D350 8007CB50 0000428C */  lw         $v0, 0x0($v0)
    /* 6D354 8007CB54 0001033C */  lui        $v1, (0x1000000 >> 16)
    /* 6D358 8007CB58 24104300 */  and        $v0, $v0, $v1
    /* 6D35C 8007CB5C 0E004010 */  beqz       $v0, .L8007CB98
    /* 6D360 8007CB60 21100002 */   addu      $v0, $s0, $zero
    /* 6D364 8007CB64 0001113C */  lui        $s1, (0x1000000 >> 16)
  .L8007CB68:
    /* 6D368 8007CB68 27F7010C */  jal        func_8007DC9C
    /* 6D36C 8007CB6C 00000000 */   nop
    /* 6D370 8007CB70 09004014 */  bnez       $v0, .L8007CB98
    /* 6D374 8007CB74 FFFF0224 */   addiu     $v0, $zero, -0x1
    /* 6D378 8007CB78 0A80023C */  lui        $v0, %hi(D_8009BF60)
    /* 6D37C 8007CB7C 60BF428C */  lw         $v0, %lo(D_8009BF60)($v0)
    /* 6D380 8007CB80 00000000 */  nop
    /* 6D384 8007CB84 0000428C */  lw         $v0, 0x0($v0)
    /* 6D388 8007CB88 00000000 */  nop
    /* 6D38C 8007CB8C 24105100 */  and        $v0, $v0, $s1
    /* 6D390 8007CB90 F5FF4014 */  bnez       $v0, .L8007CB68
    /* 6D394 8007CB94 21100002 */   addu      $v0, $s0, $zero
  .L8007CB98:
    /* 6D398 8007CB98 1800BF8F */  lw         $ra, 0x18($sp)
    /* 6D39C 8007CB9C 1400B18F */  lw         $s1, 0x14($sp)
    /* 6D3A0 8007CBA0 1000B08F */  lw         $s0, 0x10($sp)
    /* 6D3A4 8007CBA4 2000BD27 */  addiu      $sp, $sp, 0x20
    /* 6D3A8 8007CBA8 0800E003 */  jr         $ra
    /* 6D3AC 8007CBAC 00000000 */   nop
endlabel func_8007CAC8
