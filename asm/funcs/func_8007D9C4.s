glabel func_8007D9C4
    /* 6E1C4 8007D9C4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 6E1C8 8007D9C8 1000B0AF */  sw         $s0, 0x10($sp)
    /* 6E1CC 8007D9CC 21808000 */  addu       $s0, $a0, $zero
    /* 6E1D0 8007D9D0 1400BFAF */  sw         $ra, 0x14($sp)
    /* 6E1D4 8007D9D4 0F0B020C */  jal        motion_make_table
    /* 6E1D8 8007D9D8 21200000 */   addu      $a0, $zero, $zero
    /* 6E1DC 8007D9DC 0A80013C */  lui        $at, %hi(D_8009BF7C)
    /* 6E1E0 8007D9E0 7CBF20AC */  sw         $zero, %lo(D_8009BF7C)($at)
    /* 6E1E4 8007D9E4 0A80033C */  lui        $v1, %hi(D_8009BF7C)
    /* 6E1E8 8007D9E8 7CBF638C */  lw         $v1, %lo(D_8009BF7C)($v1)
    /* 6E1EC 8007D9EC 0A80013C */  lui        $at, %hi(D_8009BF88)
    /* 6E1F0 8007D9F0 88BF22AC */  sw         $v0, %lo(D_8009BF88)($at)
    /* 6E1F4 8007D9F4 01000224 */  addiu      $v0, $zero, 0x1
    /* 6E1F8 8007D9F8 0A80013C */  lui        $at, %hi(D_8009BF78)
    /* 6E1FC 8007D9FC 78BF23AC */  sw         $v1, %lo(D_8009BF78)($at)
    /* 6E200 8007DA00 07000332 */  andi       $v1, $s0, 0x7
    /* 6E204 8007DA04 25006210 */  beq        $v1, $v0, .L8007DA9C
    /* 6E208 8007DA08 02006228 */   slti      $v0, $v1, 0x2
    /* 6E20C 8007DA0C 05004010 */  beqz       $v0, .L8007DA24
    /* 6E210 8007DA10 00000000 */   nop
    /* 6E214 8007DA14 08006010 */  beqz       $v1, .L8007DA38
    /* 6E218 8007DA18 00000000 */   nop
    /* 6E21C 8007DA1C BAF60108 */  j          .L8007DAE8
    /* 6E220 8007DA20 00000000 */   nop
  .L8007DA24:
    /* 6E224 8007DA24 03000224 */  addiu      $v0, $zero, 0x3
    /* 6E228 8007DA28 1C006210 */  beq        $v1, $v0, .L8007DA9C
    /* 6E22C 8007DA2C 05000224 */   addiu     $v0, $zero, 0x5
    /* 6E230 8007DA30 2D006214 */  bne        $v1, $v0, .L8007DAE8
    /* 6E234 8007DA34 00000000 */   nop
  .L8007DA38:
    /* 6E238 8007DA38 0A80033C */  lui        $v1, %hi(D_8009BF54)
    /* 6E23C 8007DA3C 54BF638C */  lw         $v1, %lo(D_8009BF54)($v1)
    /* 6E240 8007DA40 01040224 */  addiu      $v0, $zero, 0x401
    /* 6E244 8007DA44 000062AC */  sw         $v0, 0x0($v1)
    /* 6E248 8007DA48 0A80033C */  lui        $v1, %hi(D_8009BF64)
    /* 6E24C 8007DA4C 64BF638C */  lw         $v1, %lo(D_8009BF64)($v1)
    /* 6E250 8007DA50 0F80043C */  lui        $a0, %hi(D_800F189C)
    /* 6E254 8007DA54 9C188424 */  addiu      $a0, $a0, %lo(D_800F189C)
    /* 6E258 8007DA58 0000628C */  lw         $v0, 0x0($v1)
    /* 6E25C 8007DA5C 21280000 */  addu       $a1, $zero, $zero
    /* 6E260 8007DA60 00084234 */  ori        $v0, $v0, 0x800
    /* 6E264 8007DA64 000062AC */  sw         $v0, 0x0($v1)
    /* 6E268 8007DA68 0A80023C */  lui        $v0, %hi(D_8009BF48)
    /* 6E26C 8007DA6C 48BF428C */  lw         $v0, %lo(D_8009BF48)($v0)
    /* 6E270 8007DA70 00010624 */  addiu      $a2, $zero, 0x100
    /* 6E274 8007DA74 000040AC */  sw         $zero, 0x0($v0)
    /* 6E278 8007DA78 B9F7010C */  jal        bb2_memset
    /* 6E27C 8007DA7C 00000000 */   nop
    /* 6E280 8007DA80 1080043C */  lui        $a0, %hi(D_80103680)
    /* 6E284 8007DA84 80368424 */  addiu      $a0, $a0, %lo(D_80103680)
    /* 6E288 8007DA88 21280000 */  addu       $a1, $zero, $zero
    /* 6E28C 8007DA8C B9F7010C */  jal        bb2_memset
    /* 6E290 8007DA90 00180624 */   addiu     $a2, $zero, 0x1800
    /* 6E294 8007DA94 BAF60108 */  j          .L8007DAE8
    /* 6E298 8007DA98 00000000 */   nop
  .L8007DA9C:
    /* 6E29C 8007DA9C 0A80033C */  lui        $v1, %hi(D_8009BF54)
    /* 6E2A0 8007DAA0 54BF638C */  lw         $v1, %lo(D_8009BF54)($v1)
    /* 6E2A4 8007DAA4 01040224 */  addiu      $v0, $zero, 0x401
    /* 6E2A8 8007DAA8 000062AC */  sw         $v0, 0x0($v1)
    /* 6E2AC 8007DAAC 0A80033C */  lui        $v1, %hi(D_8009BF64)
    /* 6E2B0 8007DAB0 64BF638C */  lw         $v1, %lo(D_8009BF64)($v1)
    /* 6E2B4 8007DAB4 00000000 */  nop
    /* 6E2B8 8007DAB8 0000628C */  lw         $v0, 0x0($v1)
    /* 6E2BC 8007DABC 00000000 */  nop
    /* 6E2C0 8007DAC0 00084234 */  ori        $v0, $v0, 0x800
    /* 6E2C4 8007DAC4 000062AC */  sw         $v0, 0x0($v1)
    /* 6E2C8 8007DAC8 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6E2CC 8007DACC 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6E2D0 8007DAD0 0002023C */  lui        $v0, (0x2000000 >> 16)
    /* 6E2D4 8007DAD4 000062AC */  sw         $v0, 0x0($v1)
    /* 6E2D8 8007DAD8 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6E2DC 8007DADC 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6E2E0 8007DAE0 0001023C */  lui        $v0, (0x1000000 >> 16)
    /* 6E2E4 8007DAE4 000062AC */  sw         $v0, 0x0($v1)
  .L8007DAE8:
    /* 6E2E8 8007DAE8 0A80043C */  lui        $a0, %hi(D_8009BF88)
    /* 6E2EC 8007DAEC 88BF848C */  lw         $a0, %lo(D_8009BF88)($a0)
    /* 6E2F0 8007DAF0 0F0B020C */  jal        motion_make_table
    /* 6E2F4 8007DAF4 00000000 */   nop
    /* 6E2F8 8007DAF8 07000232 */  andi       $v0, $s0, 0x7
    /* 6E2FC 8007DAFC 03004014 */  bnez       $v0, .L8007DB0C
    /* 6E300 8007DB00 21100000 */   addu      $v0, $zero, $zero
    /* 6E304 8007DB04 82F7010C */  jal        func_8007DE08
    /* 6E308 8007DB08 21200002 */   addu      $a0, $s0, $zero
  .L8007DB0C:
    /* 6E30C 8007DB0C 1400BF8F */  lw         $ra, 0x14($sp)
    /* 6E310 8007DB10 1000B08F */  lw         $s0, 0x10($sp)
    /* 6E314 8007DB14 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 6E318 8007DB18 0800E003 */  jr         $ra
    /* 6E31C 8007DB1C 00000000 */   nop
endlabel func_8007D9C4
