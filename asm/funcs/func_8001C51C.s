glabel func_8001C51C
    /* CD1C 8001C51C E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* CD20 8001C520 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* CD24 8001C524 3071000C */  jal        fight_StartFighter0DefaultVariantScriptIfMoveKind0x32Or0x11
    /* CD28 8001C528 1800B0AF */   sw        $s0, 0x18($sp)
    /* CD2C 8001C52C 0A80033C */  lui        $v1, %hi(g_fight_mode_selector)
    /* CD30 8001C530 DC386384 */  lh         $v1, %lo(g_fight_mode_selector)($v1)
    /* CD34 8001C534 03000224 */  addiu      $v0, $zero, 0x3
    /* CD38 8001C538 0E006214 */  bne        $v1, $v0, .L8001C574
    /* CD3C 8001C53C 01000424 */   addiu     $a0, $zero, 0x1
    /* CD40 8001C540 0A80023C */  lui        $v0, %hi(D_800A3728)
    /* CD44 8001C544 28374290 */  lbu        $v0, %lo(D_800A3728)($v0)
    /* CD48 8001C548 00000000 */  nop
    /* CD4C 8001C54C 09004010 */  beqz       $v0, .L8001C574
    /* CD50 8001C550 00000000 */   nop
    /* CD54 8001C554 1080053C */  lui        $a1, %hi(g_fight_config + 0x9)
    /* CD58 8001C558 8127A580 */  lb         $a1, %lo(g_fight_config + 0x9)($a1)
    /* CD5C 8001C55C 1080063C */  lui        $a2, %hi(g_fight_config + 0x5)
    /* CD60 8001C560 7D27C680 */  lb         $a2, %lo(g_fight_config + 0x5)($a2)
    /* CD64 8001C564 1080073C */  lui        $a3, %hi(g_fight_config + 0x7)
    /* CD68 8001C568 7F27E780 */  lb         $a3, %lo(g_fight_config + 0x7)($a3)
    /* CD6C 8001C56C 65710008 */  j          .L8001C594
    /* CD70 8001C570 1000A0AF */   sw        $zero, 0x10($sp)
  .L8001C574:
    /* CD74 8001C574 1080053C */  lui        $a1, %hi(g_fight_config + 0x9)
    /* CD78 8001C578 8127A580 */  lb         $a1, %lo(g_fight_config + 0x9)($a1)
    /* CD7C 8001C57C 1080063C */  lui        $a2, %hi(g_fight_config + 0x5)
    /* CD80 8001C580 7D27C680 */  lb         $a2, %lo(g_fight_config + 0x5)($a2)
    /* CD84 8001C584 1080073C */  lui        $a3, %hi(g_fight_config + 0x7)
    /* CD88 8001C588 7F27E780 */  lb         $a3, %lo(g_fight_config + 0x7)($a3)
    /* CD8C 8001C58C 01000224 */  addiu      $v0, $zero, 0x1
    /* CD90 8001C590 1000A2AF */  sw         $v0, 0x10($sp)
  .L8001C594:
    /* CD94 8001C594 6089000C */  jal        fighter_Init
    /* CD98 8001C598 00000000 */   nop
    /* CD9C 8001C59C CD8B000C */  jal        fighter_ConfigureCpuControl
    /* CDA0 8001C5A0 00000000 */   nop
    /* CDA4 8001C5A4 3286000C */  jal        motion_ResetVariantIndex
    /* CDA8 8001C5A8 01000424 */   addiu     $a0, $zero, 0x1
    /* CDAC 8001C5AC 5D86000C */  jal        motion_GetDefaultVariantScript
    /* CDB0 8001C5B0 01000424 */   addiu     $a0, $zero, 0x1
    /* CDB4 8001C5B4 01000424 */  addiu      $a0, $zero, 0x1
    /* CDB8 8001C5B8 21284000 */  addu       $a1, $v0, $zero
    /* CDBC 8001C5BC 21300000 */  addu       $a2, $zero, $zero
    /* CDC0 8001C5C0 1080103C */  lui        $s0, %hi(D_80101EC8 + 0x4AA)
    /* CDC4 8001C5C4 72231026 */  addiu      $s0, $s0, %lo(D_80101EC8 + 0x4AA)
    /* CDC8 8001C5C8 A686000C */  jal        fighter_StartMoveScript
    /* CDCC 8001C5CC 000000A6 */   sh        $zero, 0x0($s0)
    /* CDD0 8001C5D0 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* CDD4 8001C5D4 0A80013C */  lui        $at, %hi(D_800A382E)
    /* CDD8 8001C5D8 2E3820A4 */  sh         $zero, %lo(D_800A382E)($at)
    /* CDDC 8001C5DC 0A80013C */  lui        $at, %hi(g_fight_followup_fighter_index)
    /* CDE0 8001C5E0 483722A0 */  sb         $v0, %lo(g_fight_followup_fighter_index)($at)
    /* CDE4 8001C5E4 49C1000C */  jal        object_FreeAttachedObjects
    /* CDE8 8001C5E8 00000000 */   nop
    /* CDEC 8001C5EC 41C3000C */  jal        object_ClearFixedKindRange
    /* CDF0 8001C5F0 00000000 */   nop
    /* CDF4 8001C5F4 56FB0426 */  addiu      $a0, $s0, -0x4AA
    /* CDF8 8001C5F8 A56C000C */  jal        camera_InitTwoFighterView
    /* CDFC 8001C5FC A2FF0526 */   addiu     $a1, $s0, -0x5E
    /* CE00 8001C600 B2E4000C */  jal        replay_Reset
    /* CE04 8001C604 00000000 */   nop
    /* CE08 8001C608 A084000C */  jal        fighter_AssignMotionSlotAndScanDataIdNibbles
    /* CE0C 8001C60C 01000424 */   addiu     $a0, $zero, 0x1
    /* CE10 8001C610 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* CE14 8001C614 1800B08F */  lw         $s0, 0x18($sp)
    /* CE18 8001C618 2000BD27 */  addiu      $sp, $sp, 0x20
    /* CE1C 8001C61C 0800E003 */  jr         $ra
    /* CE20 8001C620 00000000 */   nop
endlabel func_8001C51C
