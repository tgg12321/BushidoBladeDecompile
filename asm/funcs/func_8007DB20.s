glabel func_8007DB20
    /* 6E320 8007DB20 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6E324 8007DB24 1400BFAF */  sw         $ra, 0x14($sp)
    /* 6E328 8007DB28 2A008014 */  bnez       $a0, .L8007DBD4
    /* 6E32C 8007DB2C 1000B0AF */   sw        $s0, 0x10($sp)
    /* 6E330 8007DB30 1AF7010C */  jal        func_8007DC68
    /* 6E334 8007DB34 00000000 */   nop
    /* 6E338 8007DB38 D6F60108 */  j          .L8007DB58
    /* 6E33C 8007DB3C 00000000 */   nop
  .L8007DB40:
    /* 6E340 8007DB40 B6F5010C */  jal        func_8007D6D8
    /* 6E344 8007DB44 00000000 */   nop
    /* 6E348 8007DB48 27F7010C */  jal        func_8007DC9C
    /* 6E34C 8007DB4C 00000000 */   nop
    /* 6E350 8007DB50 40004014 */  bnez       $v0, .L8007DC54
    /* 6E354 8007DB54 FFFF0224 */   addiu     $v0, $zero, -0x1
  .L8007DB58:
    /* 6E358 8007DB58 0A80033C */  lui        $v1, %hi(D_8009BF78)
    /* 6E35C 8007DB5C 78BF638C */  lw         $v1, %lo(D_8009BF78)($v1)
    /* 6E360 8007DB60 0A80023C */  lui        $v0, %hi(D_8009BF7C)
    /* 6E364 8007DB64 7CBF428C */  lw         $v0, %lo(D_8009BF7C)($v0)
    /* 6E368 8007DB68 00000000 */  nop
    /* 6E36C 8007DB6C 07006210 */  beq        $v1, $v0, .L8007DB8C
    /* 6E370 8007DB70 00000000 */   nop
    /* 6E374 8007DB74 D0F60108 */  j          .L8007DB40
    /* 6E378 8007DB78 00000000 */   nop
  .L8007DB7C:
    /* 6E37C 8007DB7C 27F7010C */  jal        func_8007DC9C
    /* 6E380 8007DB80 00000000 */   nop
    /* 6E384 8007DB84 33004014 */  bnez       $v0, .L8007DC54
    /* 6E388 8007DB88 FFFF0224 */   addiu     $v0, $zero, -0x1
  .L8007DB8C:
    /* 6E38C 8007DB8C 0A80023C */  lui        $v0, %hi(D_8009BF54)
    /* 6E390 8007DB90 54BF428C */  lw         $v0, %lo(D_8009BF54)($v0)
    /* 6E394 8007DB94 00000000 */  nop
    /* 6E398 8007DB98 0000428C */  lw         $v0, 0x0($v0)
    /* 6E39C 8007DB9C 0001033C */  lui        $v1, (0x1000000 >> 16)
    /* 6E3A0 8007DBA0 24104300 */  and        $v0, $v0, $v1
    /* 6E3A4 8007DBA4 F5FF4014 */  bnez       $v0, .L8007DB7C
    /* 6E3A8 8007DBA8 00000000 */   nop
    /* 6E3AC 8007DBAC 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6E3B0 8007DBB0 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6E3B4 8007DBB4 00000000 */  nop
    /* 6E3B8 8007DBB8 0000428C */  lw         $v0, 0x0($v0)
    /* 6E3BC 8007DBBC 0004033C */  lui        $v1, (0x4000000 >> 16)
    /* 6E3C0 8007DBC0 24104300 */  and        $v0, $v0, $v1
    /* 6E3C4 8007DBC4 EDFF4010 */  beqz       $v0, .L8007DB7C
    /* 6E3C8 8007DBC8 21100000 */   addu      $v0, $zero, $zero
    /* 6E3CC 8007DBCC 15F70108 */  j          .L8007DC54
    /* 6E3D0 8007DBD0 00000000 */   nop
  .L8007DBD4:
    /* 6E3D4 8007DBD4 0A80023C */  lui        $v0, %hi(D_8009BF78)
    /* 6E3D8 8007DBD8 78BF428C */  lw         $v0, %lo(D_8009BF78)($v0)
    /* 6E3DC 8007DBDC 0A80033C */  lui        $v1, %hi(D_8009BF7C)
    /* 6E3E0 8007DBE0 7CBF638C */  lw         $v1, %lo(D_8009BF7C)($v1)
    /* 6E3E4 8007DBE4 00000000 */  nop
    /* 6E3E8 8007DBE8 23104300 */  subu       $v0, $v0, $v1
    /* 6E3EC 8007DBEC 3F005030 */  andi       $s0, $v0, 0x3F
    /* 6E3F0 8007DBF0 03000012 */  beqz       $s0, .L8007DC00
    /* 6E3F4 8007DBF4 00000000 */   nop
    /* 6E3F8 8007DBF8 B6F5010C */  jal        func_8007D6D8
    /* 6E3FC 8007DBFC 00000000 */   nop
  .L8007DC00:
    /* 6E400 8007DC00 0A80023C */  lui        $v0, %hi(D_8009BF54)
    /* 6E404 8007DC04 54BF428C */  lw         $v0, %lo(D_8009BF54)($v0)
    /* 6E408 8007DC08 00000000 */  nop
    /* 6E40C 8007DC0C 0000428C */  lw         $v0, 0x0($v0)
    /* 6E410 8007DC10 0001033C */  lui        $v1, (0x1000000 >> 16)
    /* 6E414 8007DC14 24104300 */  and        $v0, $v0, $v1
    /* 6E418 8007DC18 09004014 */  bnez       $v0, .L8007DC40
    /* 6E41C 8007DC1C 00000000 */   nop
    /* 6E420 8007DC20 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6E424 8007DC24 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6E428 8007DC28 00000000 */  nop
    /* 6E42C 8007DC2C 0000428C */  lw         $v0, 0x0($v0)
    /* 6E430 8007DC30 0004033C */  lui        $v1, (0x4000000 >> 16)
    /* 6E434 8007DC34 24104300 */  and        $v0, $v0, $v1
    /* 6E438 8007DC38 05004014 */  bnez       $v0, .L8007DC50
    /* 6E43C 8007DC3C 00000000 */   nop
  .L8007DC40:
    /* 6E440 8007DC40 04000016 */  bnez       $s0, .L8007DC54
    /* 6E444 8007DC44 21100002 */   addu      $v0, $s0, $zero
    /* 6E448 8007DC48 15F70108 */  j          .L8007DC54
    /* 6E44C 8007DC4C 01000224 */   addiu     $v0, $zero, 0x1
  .L8007DC50:
    /* 6E450 8007DC50 21100002 */  addu       $v0, $s0, $zero
  .L8007DC54:
    /* 6E454 8007DC54 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6E458 8007DC58 1000B08F */  lw         $s0, 0x10($sp)
    /* 6E45C 8007DC5C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6E460 8007DC60 0800E003 */  jr         $ra
    /* 6E464 8007DC64 00000000 */   nop
endlabel func_8007DB20
