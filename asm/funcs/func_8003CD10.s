glabel func_8003CD10
    /* 2D510 8003CD10 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 2D514 8003CD14 1000BFAF */  sw         $ra, 0x10($sp)
    /* 2D518 8003CD18 79FC000C */  jal        game_SetControllerPorts
    /* 2D51C 8003CD1C 21200000 */   addu      $a0, $zero, $zero
    /* 2D520 8003CD20 0F80043C */  lui        $a0, %hi(D_800F6608)
    /* 2D524 8003CD24 08668424 */  addiu      $a0, $a0, %lo(D_800F6608)
    /* 2D528 8003CD28 10008524 */  addiu      $a1, $a0, 0x10
    /* 2D52C 8003CD2C 48F40224 */  addiu      $v0, $zero, -0xBB8
    /* 2D530 8003CD30 000080AC */  sw         $zero, 0x0($a0)
    /* 2D534 8003CD34 0F80013C */  lui        $at, %hi(D_800F660C)
    /* 2D538 8003CD38 0C6622AC */  sw         $v0, %lo(D_800F660C)($at)
    /* 2D53C 8003CD3C 20000224 */  addiu      $v0, $zero, 0x20
    /* 2D540 8003CD40 0F80013C */  lui        $at, %hi(D_800F6610)
    /* 2D544 8003CD44 106620AC */  sw         $zero, %lo(D_800F6610)($at)
    /* 2D548 8003CD48 100082A4 */  sh         $v0, 0x10($a0)
    /* 2D54C 8003CD4C 0A80023C */  lui        $v0, %hi(D_800A36AC)
    /* 2D550 8003CD50 AC36428C */  lw         $v0, %lo(D_800A36AC)($v0)
    /* 2D554 8003CD54 10270324 */  addiu      $v1, $zero, 0x2710
    /* 2D558 8003CD58 0F80013C */  lui        $at, %hi(D_800F661C)
    /* 2D55C 8003CD5C 1C6620A4 */  sh         $zero, %lo(D_800F661C)($at)
    /* 2D560 8003CD60 0F80013C */  lui        $at, %hi(D_800F6620)
    /* 2D564 8003CD64 206623AC */  sw         $v1, %lo(D_800F6620)($at)
    /* 2D568 8003CD68 80100200 */  sll        $v0, $v0, 2
    /* 2D56C 8003CD6C 0F80013C */  lui        $at, %hi(D_800F661A)
    /* 2D570 8003CD70 1A6622A4 */  sh         $v0, %lo(D_800F661A)($at)
    /* 2D574 8003CD74 FD1A010C */  jal        replay_camera_rob_back_win_near
    /* 2D578 8003CD78 10270624 */   addiu     $a2, $zero, 0x2710
    /* 2D57C 8003CD7C 6A1B010C */  jal        game_StageInit
    /* 2D580 8003CD80 01000424 */   addiu     $a0, $zero, 0x1
    /* 2D584 8003CD84 0A80043C */  lui        $a0, %hi(D_800A391F)
    /* 2D588 8003CD88 1F398490 */  lbu        $a0, %lo(D_800A391F)($a0)
    /* 2D58C 8003CD8C 0A80053C */  lui        $a1, %hi(D_800A38B4)
    /* 2D590 8003CD90 B438A58C */  lw         $a1, %lo(D_800A38B4)($a1)
    /* 2D594 8003CD94 3280010C */  jal        func_800600C8
    /* 2D598 8003CD98 01000624 */   addiu     $a2, $zero, 0x1
    /* 2D59C 8003CD9C 80100200 */  sll        $v0, $v0, 2
    /* 2D5A0 8003CDA0 0A80033C */  lui        $v1, %hi(D_800A38B4)
    /* 2D5A4 8003CDA4 B438638C */  lw         $v1, %lo(D_800A38B4)($v1)
    /* 2D5A8 8003CDA8 0A80043C */  lui        $a0, %hi(D_800A37B8)
    /* 2D5AC 8003CDAC B837848C */  lw         $a0, %lo(D_800A37B8)($a0)
    /* 2D5B0 8003CDB0 21186200 */  addu       $v1, $v1, $v0
    /* 2D5B4 8003CDB4 01008424 */  addiu      $a0, $a0, 0x1
    /* 2D5B8 8003CDB8 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* 2D5BC 8003CDBC B83724AC */  sw         $a0, %lo(D_800A37B8)($at)
    /* 2D5C0 8003CDC0 97008428 */  slti       $a0, $a0, 0x97
    /* 2D5C4 8003CDC4 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 2D5C8 8003CDC8 B43823AC */  sw         $v1, %lo(D_800A38B4)($at)
    /* 2D5CC 8003CDCC 07008010 */  beqz       $a0, .L8003CDEC
    /* 2D5D0 8003CDD0 4000033C */   lui       $v1, (0x400040 >> 16)
    /* 2D5D4 8003CDD4 1080023C */  lui        $v0, %hi(D_80102794)
    /* 2D5D8 8003CDD8 9427428C */  lw         $v0, %lo(D_80102794)($v0)
    /* 2D5DC 8003CDDC 40006334 */  ori        $v1, $v1, (0x400040 & 0xFFFF)
    /* 2D5E0 8003CDE0 24104300 */  and        $v0, $v0, $v1
    /* 2D5E4 8003CDE4 08004010 */  beqz       $v0, .L8003CE08
    /* 2D5E8 8003CDE8 00000000 */   nop
  .L8003CDEC:
    /* 2D5EC 8003CDEC B0DC000C */  jal        func_800372C0
    /* 2D5F0 8003CDF0 00000000 */   nop
    /* 2D5F4 8003CDF4 8B76000C */  jal        func_8001DA2C
    /* 2D5F8 8003CDF8 00000000 */   nop
    /* 2D5FC 8003CDFC 08000224 */  addiu      $v0, $zero, 0x8
    /* 2D600 8003CE00 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2D604 8003CE04 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
  .L8003CE08:
    /* 2D608 8003CE08 1000BF8F */  lw         $ra, 0x10($sp)
    /* 2D60C 8003CE0C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 2D610 8003CE10 0800E003 */  jr         $ra
    /* 2D614 8003CE14 00000000 */   nop
endlabel func_8003CD10
