glabel tslLineG5Init
    /* 2E28C 8003DA8C D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* 2E290 8003DA90 2400B1AF */  sw         $s1, 0x24($sp)
    /* 2E294 8003DA94 21888000 */  addu       $s1, $a0, $zero
    /* 2E298 8003DA98 FFFF0234 */  ori        $v0, $zero, 0xFFFF
    /* 2E29C 8003DA9C 0980013C */  lui        $at, %hi(D_800905F8)
    /* 2E2A0 8003DAA0 F80522AC */  sw         $v0, %lo(D_800905F8)($at)
    /* 2E2A4 8003DAA4 80101100 */  sll        $v0, $s1, 2
    /* 2E2A8 8003DAA8 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 2E2AC 8003DAAC 2800B2AF */  sw         $s2, 0x28($sp)
    /* 2E2B0 8003DAB0 2000B0AF */  sw         $s0, 0x20($sp)
    /* 2E2B4 8003DAB4 0980013C */  lui        $at, %hi(D_800906A4)
    /* 2E2B8 8003DAB8 21082200 */  addu       $at, $at, $v0
    /* 2E2BC 8003DABC A4062284 */  lh         $v0, %lo(D_800906A4)($at)
    /* 2E2C0 8003DAC0 00000000 */  nop
    /* 2E2C4 8003DAC4 40004010 */  beqz       $v0, .L8003DBC8
    /* 2E2C8 8003DAC8 2190A000 */   addu      $s2, $a1, $zero
    /* 2E2CC 8003DACC 9AFC000C */  jal        game_GetPlayerCount
    /* 2E2D0 8003DAD0 00000000 */   nop
    /* 2E2D4 8003DAD4 02004014 */  bnez       $v0, .L8003DAE0
    /* 2E2D8 8003DAD8 F0550224 */   addiu     $v0, $zero, 0x55F0
    /* 2E2DC 8003DADC 90650224 */  addiu      $v0, $zero, 0x6590
  .L8003DAE0:
    /* 2E2E0 8003DAE0 23205200 */  subu       $a0, $v0, $s2
    /* 2E2E4 8003DAE4 70178228 */  slti       $v0, $a0, 0x1770
    /* 2E2E8 8003DAE8 04004010 */  beqz       $v0, .L8003DAFC
    /* 2E2EC 8003DAEC 70170224 */   addiu     $v0, $zero, 0x1770
    /* 2E2F0 8003DAF0 23284400 */  subu       $a1, $v0, $a0
    /* 2E2F4 8003DAF4 C0F60008 */  j          .L8003DB00
    /* 2E2F8 8003DAF8 70170424 */   addiu     $a0, $zero, 0x1770
  .L8003DAFC:
    /* 2E2FC 8003DAFC 21280000 */  addu       $a1, $zero, $zero
  .L8003DB00:
    /* 2E300 8003DB00 0F80033C */  lui        $v1, %hi(D_800F6656)
    /* 2E304 8003DB04 56666384 */  lh         $v1, %lo(D_800F6656)($v1)
    /* 2E308 8003DB08 0980023C */  lui        $v0, %hi(D_80090608)
    /* 2E30C 8003DB0C 08064284 */  lh         $v0, %lo(D_80090608)($v0)
    /* 2E310 8003DB10 21306000 */  addu       $a2, $v1, $zero
    /* 2E314 8003DB14 27100200 */  nor        $v0, $zero, $v0
    /* 2E318 8003DB18 24186200 */  and        $v1, $v1, $v0
    /* 2E31C 8003DB1C 01006330 */  andi       $v1, $v1, 0x1
    /* 2E320 8003DB20 05006010 */  beqz       $v1, .L8003DB38
    /* 2E324 8003DB24 0100C230 */   andi      $v0, $a2, 0x1
    /* 2E328 8003DB28 0980013C */  lui        $at, %hi(D_80090600)
    /* 2E32C 8003DB2C 000624AC */  sw         $a0, %lo(D_80090600)($at)
    /* 2E330 8003DB30 0980013C */  lui        $at, %hi(D_80090604)
    /* 2E334 8003DB34 040625AC */  sw         $a1, %lo(D_80090604)($at)
  .L8003DB38:
    /* 2E338 8003DB38 05004010 */  beqz       $v0, .L8003DB50
    /* 2E33C 8003DB3C 00000000 */   nop
    /* 2E340 8003DB40 0980023C */  lui        $v0, %hi(D_80090604)
    /* 2E344 8003DB44 0406428C */  lw         $v0, %lo(D_80090604)($v0)
    /* 2E348 8003DB48 D5F60008 */  j          .L8003DB54
    /* 2E34C 8003DB4C 23904202 */   subu      $s2, $s2, $v0
  .L8003DB50:
    /* 2E350 8003DB50 23904502 */  subu       $s2, $s2, $a1
  .L8003DB54:
    /* 2E354 8003DB54 21204002 */  addu       $a0, $s2, $zero
    /* 2E358 8003DB58 80881100 */  sll        $s1, $s1, 2
    /* 2E35C 8003DB5C 0F80023C */  lui        $v0, %hi(D_800F6656)
    /* 2E360 8003DB60 56664294 */  lhu        $v0, %lo(D_800F6656)($v0)
    /* 2E364 8003DB64 0980103C */  lui        $s0, %hi(D_8009060C)
    /* 2E368 8003DB68 0C061026 */  addiu      $s0, $s0, %lo(D_8009060C)
    /* 2E36C 8003DB6C 0980013C */  lui        $at, %hi(D_80090608)
    /* 2E370 8003DB70 080622A4 */  sh         $v0, %lo(D_80090608)($at)
    /* 2E374 8003DB74 0980013C */  lui        $at, %hi(StatusUpBuf)
    /* 2E378 8003DB78 21083100 */  addu       $at, $at, $s1
    /* 2E37C 8003DB7C A6062284 */  lh         $v0, %lo(StatusUpBuf)($at)
    /* 2E380 8003DB80 21803002 */  addu       $s0, $s1, $s0
    /* 2E384 8003DB84 1000A2AF */  sw         $v0, 0x10($sp)
    /* 2E388 8003DB88 0000078E */  lw         $a3, 0x0($s0)
    /* 2E38C 8003DB8C 0A80063C */  lui        $a2, %hi(light_effect_col)
    /* 2E390 8003DB90 703DC624 */  addiu      $a2, $a2, %lo(light_effect_col)
    /* 2E394 8003DB94 F9F6000C */  jal        func_8003DBE4
    /* 2E398 8003DB98 1F000524 */   addiu     $a1, $zero, 0x1F
    /* 2E39C 8003DB9C 21204002 */  addu       $a0, $s2, $zero
    /* 2E3A0 8003DBA0 0980013C */  lui        $at, %hi(StatusUpBuf)
    /* 2E3A4 8003DBA4 21083100 */  addu       $at, $at, $s1
    /* 2E3A8 8003DBA8 A6062284 */  lh         $v0, %lo(StatusUpBuf)($at)
    /* 2E3AC 8003DBAC 00000000 */  nop
    /* 2E3B0 8003DBB0 1000A2AF */  sw         $v0, 0x10($sp)
    /* 2E3B4 8003DBB4 0000078E */  lw         $a3, 0x0($s0)
    /* 2E3B8 8003DBB8 0A80063C */  lui        $a2, %hi(D_800A4340)
    /* 2E3BC 8003DBBC 4043C624 */  addiu      $a2, $a2, %lo(D_800A4340)
    /* 2E3C0 8003DBC0 F9F6000C */  jal        func_8003DBE4
    /* 2E3C4 8003DBC4 13000524 */   addiu     $a1, $zero, 0x13
  .L8003DBC8:
    /* 2E3C8 8003DBC8 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 2E3CC 8003DBCC 2800B28F */  lw         $s2, 0x28($sp)
    /* 2E3D0 8003DBD0 2400B18F */  lw         $s1, 0x24($sp)
    /* 2E3D4 8003DBD4 2000B08F */  lw         $s0, 0x20($sp)
    /* 2E3D8 8003DBD8 3000BD27 */  addiu      $sp, $sp, 0x30
    /* 2E3DC 8003DBDC 0800E003 */  jr         $ra
    /* 2E3E0 8003DBE0 00000000 */   nop
endlabel tslLineG5Init
