glabel func_80078DA0
    /* 695A0 80078DA0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 695A4 80078DA4 1400BFAF */  sw         $ra, 0x14($sp)
    /* 695A8 80078DA8 6EE2010C */  jal        EnterCriticalSection
    /* 695AC 80078DAC 1000B0AF */   sw        $s0, 0x10($sp)
    /* 695B0 80078DB0 01000424 */  addiu      $a0, $zero, 0x1
    /* 695B4 80078DB4 0F80033C */  lui        $v1, %hi(D_800F183C)
    /* 695B8 80078DB8 3C186324 */  addiu      $v1, $v1, %lo(D_800F183C)
    /* 695BC 80078DBC FCFF7024 */  addiu      $s0, $v1, -0x4
    /* 695C0 80078DC0 0880023C */  lui        $v0, %hi(func_80078E58)
    /* 695C4 80078DC4 588E4224 */  addiu      $v0, $v0, %lo(func_80078E58)
    /* 695C8 80078DC8 000062AC */  sw         $v0, 0x0($v1)
    /* 695CC 80078DCC 0880023C */  lui        $v0, %hi(func_80078EC0)
    /* 695D0 80078DD0 C08E4224 */  addiu      $v0, $v0, %lo(func_80078EC0)
    /* 695D4 80078DD4 0F80013C */  lui        $at, %hi(D_800F1840)
    /* 695D8 80078DD8 401822AC */  sw         $v0, %lo(D_800F1840)($at)
    /* 695DC 80078DDC 0F80013C */  lui        $at, %hi(D_800F1838)
    /* 695E0 80078DE0 381820AC */  sw         $zero, %lo(D_800F1838)($at)
    /* 695E4 80078DE4 0F80013C */  lui        $at, %hi(D_800F1844)
    /* 695E8 80078DE8 441820AC */  sw         $zero, %lo(D_800F1844)($at)
    /* 695EC 80078DEC D4E3010C */  jal        func_80078F50
    /* 695F0 80078DF0 21280002 */   addu      $a1, $s0, $zero
    /* 695F4 80078DF4 01000424 */  addiu      $a0, $zero, 0x1
    /* 695F8 80078DF8 D0E3010C */  jal        func_80078F40
    /* 695FC 80078DFC 21280002 */   addu      $a1, $s0, $zero
    /* 69600 80078E00 72E2010C */  jal        ExitCriticalSection
    /* 69604 80078E04 00000000 */   nop
    /* 69608 80078E08 01000224 */  addiu      $v0, $zero, 0x1
    /* 6960C 80078E0C 1400BF8F */  lw         $ra, 0x14($sp)
    /* 69610 80078E10 1000B08F */  lw         $s0, 0x10($sp)
    /* 69614 80078E14 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 69618 80078E18 0800E003 */  jr         $ra
    /* 6961C 80078E1C 00000000 */   nop
endlabel func_80078DA0
