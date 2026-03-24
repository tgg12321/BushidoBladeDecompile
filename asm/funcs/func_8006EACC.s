glabel func_8006EACC
    /* 5F2CC 8006EACC 0A80033C */  lui        $v1, %hi(D_800A36AC)
    /* 5F2D0 8006EAD0 AC36638C */  lw         $v1, %lo(D_800A36AC)($v1)
    /* 5F2D4 8006EAD4 00000000 */  nop
    /* 5F2D8 8006EAD8 01006330 */  andi       $v1, $v1, 0x1
    /* 5F2DC 8006EADC C0110300 */  sll        $v0, $v1, 7
    /* 5F2E0 8006EAE0 21104300 */  addu       $v0, $v0, $v1
    /* 5F2E4 8006EAE4 C0100200 */  sll        $v0, $v0, 3
    /* 5F2E8 8006EAE8 21104300 */  addu       $v0, $v0, $v1
    /* 5F2EC 8006EAEC 00110200 */  sll        $v0, $v0, 4
    /* 5F2F0 8006EAF0 0F80033C */  lui        $v1, %hi(D_800F7438)
    /* 5F2F4 8006EAF4 38746324 */  addiu      $v1, $v1, %lo(D_800F7438)
    /* 5F2F8 8006EAF8 21104300 */  addu       $v0, $v0, $v1
    /* 5F2FC 8006EAFC F004838F */  lw         $v1, %gp_rel(D_800A35BC)($gp)
    /* 5F300 8006EB00 C0FFBD27 */  addiu      $sp, $sp, -0x40
    /* 5F304 8006EB04 3800BFAF */  sw         $ra, 0x38($sp)
    /* 5F308 8006EB08 7C0484AF */  sw         $a0, %gp_rel(D_800A3548)($gp)
    /* 5F30C 8006EB0C 800485AF */  sw         $a1, %gp_rel(D_800A354C)($gp)
    /* 5F310 8006EB10 F40482AF */  sw         $v0, %gp_rel(D_800A35C0)($gp)
    /* 5F314 8006EB14 02000224 */  addiu      $v0, $zero, 0x2
    /* 5F318 8006EB18 02006214 */  bne        $v1, $v0, .L8006EB24
    /* 5F31C 8006EB1C FFFFA230 */   andi      $v0, $a1, 0xFFFF
    /* 5F320 8006EB20 800482AF */  sw         $v0, %gp_rel(D_800A354C)($gp)
  .L8006EB24:
    /* 5F324 8006EB24 03BB010C */  jal        func_8006EC0C
    /* 5F328 8006EB28 00000000 */   nop
    /* 5F32C 8006EB2C F804858F */  lw         $a1, %gp_rel(D_800A35C4)($gp)
    /* 5F330 8006EB30 00000000 */  nop
    /* 5F334 8006EB34 0C00A38C */  lw         $v1, 0xC($a1)
    /* 5F338 8006EB38 0800A28C */  lw         $v0, 0x8($a1)
    /* 5F33C 8006EB3C 01006324 */  addiu      $v1, $v1, 0x1
    /* 5F340 8006EB40 01006430 */  andi       $a0, $v1, 0x1
    /* 5F344 8006EB44 01004224 */  addiu      $v0, $v0, 0x1
    /* 5F348 8006EB48 0800A2AC */  sw         $v0, 0x8($a1)
    /* 5F34C 8006EB4C 2BBA010C */  jal        func_8006E8AC
    /* 5F350 8006EB50 0C00A3AC */   sw        $v1, 0xC($a1)
    /* 5F354 8006EB54 DC04838F */  lw         $v1, %gp_rel(D_800A35A8)($gp)
    /* 5F358 8006EB58 00000000 */  nop
    /* 5F35C 8006EB5C 1000A3AF */  sw         $v1, 0x10($sp)
    /* 5F360 8006EB60 0000438C */  lw         $v1, 0x0($v0)
    /* 5F364 8006EB64 00000000 */  nop
    /* 5F368 8006EB68 1400A3AF */  sw         $v1, 0x14($sp)
    /* 5F36C 8006EB6C 0800438C */  lw         $v1, 0x8($v0)
    /* 5F370 8006EB70 00000000 */  nop
    /* 5F374 8006EB74 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 5F378 8006EB78 1000438C */  lw         $v1, 0x10($v0)
    /* 5F37C 8006EB7C 00000000 */  nop
    /* 5F380 8006EB80 2000A3AF */  sw         $v1, 0x20($sp)
    /* 5F384 8006EB84 0C00438C */  lw         $v1, 0xC($v0)
    /* 5F388 8006EB88 00000000 */  nop
    /* 5F38C 8006EB8C 2400A3AF */  sw         $v1, 0x24($sp)
    /* 5F390 8006EB90 1400438C */  lw         $v1, 0x14($v0)
    /* 5F394 8006EB94 00000000 */  nop
    /* 5F398 8006EB98 2800A3AF */  sw         $v1, 0x28($sp)
    /* 5F39C 8006EB9C 1800438C */  lw         $v1, 0x18($v0)
    /* 5F3A0 8006EBA0 00000000 */  nop
    /* 5F3A4 8006EBA4 2C00A3AF */  sw         $v1, 0x2C($sp)
    /* 5F3A8 8006EBA8 1C00438C */  lw         $v1, 0x1C($v0)
    /* 5F3AC 8006EBAC D80482AF */  sw         $v0, %gp_rel(D_800A35A4)($gp)
    /* 5F3B0 8006EBB0 3000A3AF */  sw         $v1, 0x30($sp)
    /* 5F3B4 8006EBB4 B4048397 */  lhu        $v1, %gp_rel(D_800A3580)($gp)
    /* 5F3B8 8006EBB8 2000448C */  lw         $a0, 0x20($v0)
    /* 5F3BC 8006EBBC FEFF6324 */  addiu      $v1, $v1, -0x2
    /* 5F3C0 8006EBC0 0200632C */  sltiu      $v1, $v1, 0x2
    /* 5F3C4 8006EBC4 03006014 */  bnez       $v1, .L8006EBD4
    /* 5F3C8 8006EBC8 3400A4AF */   sw        $a0, 0x34($sp)
    /* 5F3CC 8006EBCC 4ABD010C */  jal        func_8006F528
    /* 5F3D0 8006EBD0 1000A427 */   addiu     $a0, $sp, 0x10
  .L8006EBD4:
    /* 5F3D4 8006EBD4 B4048287 */  lh         $v0, %gp_rel(D_800A3580)($gp)
    /* 5F3D8 8006EBD8 00000000 */  nop
    /* 5F3DC 8006EBDC 80100200 */  sll        $v0, $v0, 2
    /* 5F3E0 8006EBE0 0A80013C */  lui        $at, %hi(D_8009BC1C)
    /* 5F3E4 8006EBE4 21082200 */  addu       $at, $at, $v0
    /* 5F3E8 8006EBE8 1CBC228C */  lw         $v0, %lo(D_8009BC1C)($at)
    /* 5F3EC 8006EBEC 00000000 */  nop
    /* 5F3F0 8006EBF0 09F84000 */  jalr       $v0
    /* 5F3F4 8006EBF4 1000A427 */   addiu     $a0, $sp, 0x10
    /* 5F3F8 8006EBF8 D404828F */  lw         $v0, %gp_rel(D_800A35A0)($gp)
    /* 5F3FC 8006EBFC 3800BF8F */  lw         $ra, 0x38($sp)
    /* 5F400 8006EC00 4000BD27 */  addiu      $sp, $sp, 0x40
    /* 5F404 8006EC04 0800E003 */  jr         $ra
    /* 5F408 8006EC08 00000000 */   nop
endlabel func_8006EACC
