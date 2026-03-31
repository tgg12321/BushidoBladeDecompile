glabel func_8001DA8C
    /* E28C 8001DA8C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* E290 8001DA90 1000BFAF */  sw         $ra, 0x10($sp)
    /* E294 8001DA94 EAD7000C */  jal        func_80035FA8
    /* E298 8001DA98 00000000 */   nop
    /* E29C 8001DA9C F559000C */  jal        file_GetFlag2
    /* E2A0 8001DAA0 00000000 */   nop
    /* E2A4 8001DAA4 28004014 */  bnez       $v0, .L8001DB48
    /* E2A8 8001DAA8 00000000 */   nop
    /* E2AC 8001DAAC 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* E2B0 8001DAB0 DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* E2B4 8001DAB4 00000000 */  nop
    /* E2B8 8001DAB8 0600622C */  sltiu      $v0, $v1, 0x6
    /* E2BC 8001DABC 20004010 */  beqz       $v0, .L8001DB40
    /* E2C0 8001DAC0 80100300 */   sll       $v0, $v1, 2
    /* E2C4 8001DAC4 0180013C */  lui        $at, %hi(jtbl_800100E4)
    /* E2C8 8001DAC8 21082200 */  addu       $at, $at, $v0
    /* E2CC 8001DACC E400228C */  lw         $v0, %lo(jtbl_800100E4)($at)
    /* E2D0 8001DAD0 00000000 */  nop
    /* E2D4 8001DAD4 08004000 */  jr         $v0
    /* E2D8 8001DAD8 00000000 */   nop
  jlabel .L8001DADC
    /* E2DC 8001DADC 0A80023C */  lui        $v0, %hi(D_800A36A4)
    /* E2E0 8001DAE0 A4364284 */  lh         $v0, %lo(D_800A36A4)($v0)
    /* E2E4 8001DAE4 0980013C */  lui        $at, %hi(D_8008D518)
    /* E2E8 8001DAE8 21082200 */  addu       $at, $at, $v0
    /* E2EC 8001DAEC 18D52490 */  lbu        $a0, %lo(D_8008D518)($at)
    /* E2F0 8001DAF0 CE760008 */  j          .L8001DB38
    /* E2F4 8001DAF4 00000000 */   nop
  jlabel .L8001DAF8
    /* E2F8 8001DAF8 1080023C */  lui        $v0, %hi(D_80101ED2)
    /* E2FC 8001DAFC D21E4284 */  lh         $v0, %lo(D_80101ED2)($v0)
    /* E300 8001DB00 0980013C */  lui        $at, %hi(D_8008D9EC)
    /* E304 8001DB04 21082200 */  addu       $at, $at, $v0
    /* E308 8001DB08 ECD92290 */  lbu        $v0, %lo(D_8008D9EC)($at)
    /* E30C 8001DB0C 00000000 */  nop
    /* E310 8001DB10 09004010 */  beqz       $v0, .L8001DB38
    /* E314 8001DB14 08000424 */   addiu     $a0, $zero, 0x8
    /* E318 8001DB18 CE760008 */  j          .L8001DB38
    /* E31C 8001DB1C 09000424 */   addiu     $a0, $zero, 0x9
  jlabel .L8001DB20
    /* E320 8001DB20 0A80023C */  lui        $v0, %hi(D_800A389A)
    /* E324 8001DB24 9A384290 */  lbu        $v0, %lo(D_800A389A)($v0)
    /* E328 8001DB28 00000000 */  nop
    /* E32C 8001DB2C 02004010 */  beqz       $v0, .L8001DB38
    /* E330 8001DB30 0B000424 */   addiu     $a0, $zero, 0xB
    /* E334 8001DB34 0A000424 */  addiu      $a0, $zero, 0xA
  .L8001DB38:
    /* E338 8001DB38 44DC000C */  jal        func_80037110
    /* E33C 8001DB3C 00000000 */   nop
  jlabel .L8001DB40
    /* E340 8001DB40 7ADC000C */  jal        func_800371E8
    /* E344 8001DB44 01000424 */   addiu     $a0, $zero, 0x1
  .L8001DB48:
    /* E348 8001DB48 1000BF8F */  lw         $ra, 0x10($sp)
    /* E34C 8001DB4C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* E350 8001DB50 0800E003 */  jr         $ra
    /* E354 8001DB54 00000000 */   nop
endlabel func_8001DA8C
