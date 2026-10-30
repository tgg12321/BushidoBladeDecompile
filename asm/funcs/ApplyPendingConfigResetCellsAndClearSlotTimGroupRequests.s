glabel ApplyPendingConfigResetCellsAndClearSlotTimGroupRequests
    /* 24BF0 800343F0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 24BF4 800343F4 21200000 */  addu       $a0, $zero, $zero
    /* 24BF8 800343F8 1080063C */  lui        $a2, %hi(g_fight_config + 0xD)
    /* 24BFC 800343FC 8527C690 */  lbu        $a2, %lo(g_fight_config + 0xD)($a2)
    /* 24C00 80034400 1080033C */  lui        $v1, %hi(g_fight_config + 0xE)
    /* 24C04 80034404 86276390 */  lbu        $v1, %lo(g_fight_config + 0xE)($v1)
    /* 24C08 80034408 1080023C */  lui        $v0, %hi(g_fight_config + 0xC)
    /* 24C0C 8003440C 84274290 */  lbu        $v0, %lo(g_fight_config + 0xC)($v0)
    /* 24C10 80034410 1080073C */  lui        $a3, %hi(g_fight_config + 0xF)
    /* 24C14 80034414 8727E780 */  lb         $a3, %lo(g_fight_config + 0xF)($a3)
    /* 24C18 80034418 1000BFAF */  sw         $ra, 0x10($sp)
    /* 24C1C 8003441C 0A80013C */  lui        $at, %hi(g_fight_tracked_fighter_index)
    /* 24C20 80034420 F63620A4 */  sh         $zero, %lo(g_fight_tracked_fighter_index)($at)
    /* 24C24 80034424 00360600 */  sll        $a2, $a2, 24
    /* 24C28 80034428 03360600 */  sra        $a2, $a2, 24
    /* 24C2C 8003442C 001E0300 */  sll        $v1, $v1, 24
    /* 24C30 80034430 031E0300 */  sra        $v1, $v1, 24
    /* 24C34 80034434 00160200 */  sll        $v0, $v0, 24
    /* 24C38 80034438 03160200 */  sra        $v0, $v0, 24
    /* 24C3C 8003443C 0A80013C */  lui        $at, %hi(g_fight_mode_selector)
    /* 24C40 80034440 DC3826A4 */  sh         $a2, %lo(g_fight_mode_selector)($at)
    /* 24C44 80034444 0A80013C */  lui        $at, %hi(g_fight_tracked_view_setting)
    /* 24C48 80034448 BA3823A4 */  sh         $v1, %lo(g_fight_tracked_view_setting)($at)
    /* 24C4C 8003444C 0A80013C */  lui        $at, %hi(g_fight_force_weapon_inner_bone_hits)
    /* 24C50 80034450 403127AC */  sw         $a3, %lo(g_fight_force_weapon_inner_bone_hits)($at)
    /* 24C54 80034454 0A80013C */  lui        $at, %hi(g_fight_stage_index)
    /* 24C58 80034458 A43622A4 */  sh         $v0, %lo(g_fight_stage_index)($at)
    /* 24C5C 8003445C 8105010C */  jal        slot_RequestTimGroupsAndMarkReloadOnChange
    /* 24C60 80034460 21280000 */   addu      $a1, $zero, $zero
    /* 24C64 80034464 01000424 */  addiu      $a0, $zero, 0x1
    /* 24C68 80034468 8105010C */  jal        slot_RequestTimGroupsAndMarkReloadOnChange
    /* 24C6C 8003446C 21280000 */   addu      $a1, $zero, $zero
    /* 24C70 80034470 02000224 */  addiu      $v0, $zero, 0x2
    /* 24C74 80034474 0A80013C */  lui        $at, %hi(g_fight_bone_group_tim_requests)
    /* 24C78 80034478 6A3720A0 */  sb         $zero, %lo(g_fight_bone_group_tim_requests)($at)
    /* 24C7C 8003447C 0A80013C */  lui        $at, %hi(D_800A376B)
    /* 24C80 80034480 6B3720A0 */  sb         $zero, %lo(D_800A376B)($at)
    /* 24C84 80034484 0A80013C */  lui        $at, %hi(g_fight_menu_restart_count)
    /* 24C88 80034488 0C3820AC */  sw         $zero, %lo(g_fight_menu_restart_count)($at)
    /* 24C8C 8003448C 0A80013C */  lui        $at, %hi(D_800A38D4)
    /* 24C90 80034490 D43822A0 */  sb         $v0, %lo(D_800A38D4)($at)
    /* 24C94 80034494 0A80013C */  lui        $at, %hi(D_800A37D3)
    /* 24C98 80034498 D33720A0 */  sb         $zero, %lo(D_800A37D3)($at)
    /* 24C9C 8003449C 0A80013C */  lui        $at, %hi(D_800A37D2)
    /* 24CA0 800344A0 D23720A0 */  sb         $zero, %lo(D_800A37D2)($at)
    /* 24CA4 800344A4 1000BF8F */  lw         $ra, 0x10($sp)
    /* 24CA8 800344A8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 24CAC 800344AC 0800E003 */  jr         $ra
    /* 24CB0 800344B0 00000000 */   nop
endlabel ApplyPendingConfigResetCellsAndClearSlotTimGroupRequests
