glabel func_8001D790
    /* DF90 8001D790 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* DF94 8001D794 1800B2AF */  sw         $s2, 0x18($sp)
    /* DF98 8001D798 1980123C */  lui        $s2, (0x80190800 >> 16)
    /* DF9C 8001D79C 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* DFA0 8001D7A0 1400B1AF */  sw         $s1, 0x14($sp)
    /* DFA4 8001D7A4 1A5A000C */  jal        gpu_EnableDisplay
    /* DFA8 8001D7A8 1000B0AF */   sw        $s0, 0x10($sp)
    /* DFAC 8001D7AC 0A80033C */  lui        $v1, %hi(D_800A36A4)
    /* DFB0 8001D7B0 A4366384 */  lh         $v1, %lo(D_800A36A4)($v1)
    /* DFB4 8001D7B4 0A80023C */  lui        $v0, %hi(D_800A390E)
    /* DFB8 8001D7B8 0E394280 */  lb         $v0, %lo(D_800A390E)($v0)
    /* DFBC 8001D7BC 00000000 */  nop
    /* DFC0 8001D7C0 13006214 */  bne        $v1, $v0, .L8001D810
    /* DFC4 8001D7C4 00085236 */   ori       $s2, $s2, (0x80190800 & 0xFFFF)
    /* DFC8 8001D7C8 1080023C */  lui        $v0, %hi(D_8010277C)
    /* DFCC 8001D7CC 7C274280 */  lb         $v0, %lo(D_8010277C)($v0)
    /* DFD0 8001D7D0 0980013C */  lui        $at, %hi(D_8008E5A8)
    /* DFD4 8001D7D4 21082200 */  addu       $at, $at, $v0
    /* DFD8 8001D7D8 A8E52390 */  lbu        $v1, %lo(D_8008E5A8)($at)
    /* DFDC 8001D7DC 30008283 */  lb         $v0, %gp_rel(D_800A30FC)($gp)
    /* DFE0 8001D7E0 00000000 */  nop
    /* DFE4 8001D7E4 0A006214 */  bne        $v1, $v0, .L8001D810
    /* DFE8 8001D7E8 00000000 */   nop
    /* DFEC 8001D7EC 1080023C */  lui        $v0, %hi(D_8010277D)
    /* DFF0 8001D7F0 7D274280 */  lb         $v0, %lo(D_8010277D)($v0)
    /* DFF4 8001D7F4 0980013C */  lui        $at, %hi(D_8008E5A8)
    /* DFF8 8001D7F8 21082200 */  addu       $at, $at, $v0
    /* DFFC 8001D7FC A8E52390 */  lbu        $v1, %lo(D_8008E5A8)($at)
    /* E000 8001D800 31008283 */  lb         $v0, %gp_rel(D_800A30FD)($gp)
    /* E004 8001D804 00000000 */  nop
    /* E008 8001D808 37006210 */  beq        $v1, $v0, .L8001D8E8
    /* E00C 8001D80C 00000000 */   nop
  .L8001D810:
    /* E010 8001D810 4E83000C */  jal        func_80020D38
    /* E014 8001D814 00000000 */   nop
    /* E018 8001D818 0A80043C */  lui        $a0, %hi(D_800A36A4)
    /* E01C 8001D81C A4368484 */  lh         $a0, %lo(D_800A36A4)($a0)
    /* E020 8001D820 B71B010C */  jal        game_StageCleanup
    /* E024 8001D824 21284002 */   addu      $a1, $s2, $zero
    /* E028 8001D828 1BA4000C */  jal        func_8002906C
    /* E02C 8001D82C 00000000 */   nop
    /* E030 8001D830 7C6F010C */  jal        func_8005BDF0
    /* E034 8001D834 00000000 */   nop
    /* E038 8001D838 1080103C */  lui        $s0, %hi(D_8010277C)
    /* E03C 8001D83C 7C271026 */  addiu      $s0, $s0, %lo(D_8010277C)
    /* E040 8001D840 0A80053C */  lui        $a1, %hi(D_800A36A4)
    /* E044 8001D844 A436A584 */  lh         $a1, %lo(D_800A36A4)($a1)
    /* E048 8001D848 00000282 */  lb         $v0, 0x0($s0)
    /* E04C 8001D84C 1080033C */  lui        $v1, %hi(D_8010277D)
    /* E050 8001D850 7D276380 */  lb         $v1, %lo(D_8010277D)($v1)
    /* E054 8001D854 0980013C */  lui        $at, %hi(D_8008E5A8)
    /* E058 8001D858 21082200 */  addu       $at, $at, $v0
    /* E05C 8001D85C A8E52690 */  lbu        $a2, %lo(D_8008E5A8)($at)
    /* E060 8001D860 0980013C */  lui        $at, %hi(D_8008E5A8)
    /* E064 8001D864 21082300 */  addu       $at, $at, $v1
    /* E068 8001D868 A8E52790 */  lbu        $a3, %lo(D_8008E5A8)($at)
    /* E06C 8001D86C A36E010C */  jal        func_8005BA8C
    /* E070 8001D870 21204002 */   addu      $a0, $s2, $zero
    /* E074 8001D874 0A80033C */  lui        $v1, %hi(D_800A36A4)
    /* E078 8001D878 A4366394 */  lhu        $v1, %lo(D_800A36A4)($v1)
    /* E07C 8001D87C 00000482 */  lb         $a0, 0x0($s0)
    /* E080 8001D880 0A80013C */  lui        $at, %hi(D_800A390E)
    /* E084 8001D884 0E3923A0 */  sb         $v1, %lo(D_800A390E)($at)
    /* E088 8001D888 0980013C */  lui        $at, %hi(D_8008E5A8)
    /* E08C 8001D88C 21082400 */  addu       $at, $at, $a0
    /* E090 8001D890 A8E52390 */  lbu        $v1, %lo(D_8008E5A8)($at)
    /* E094 8001D894 1080043C */  lui        $a0, %hi(D_8010277D)
    /* E098 8001D898 7D278480 */  lb         $a0, %lo(D_8010277D)($a0)
    /* E09C 8001D89C 21884000 */  addu       $s1, $v0, $zero
    /* E0A0 8001D8A0 300083A3 */  sb         $v1, %gp_rel(D_800A30FC)($gp)
    /* E0A4 8001D8A4 0980013C */  lui        $at, %hi(D_8008E5A8)
    /* E0A8 8001D8A8 21082400 */  addu       $at, $at, $a0
    /* E0AC 8001D8AC A8E52390 */  lbu        $v1, %lo(D_8008E5A8)($at)
    /* E0B0 8001D8B0 1925222A */  slti       $v0, $s1, 0x2519
    /* E0B4 8001D8B4 310083A3 */  sb         $v1, %gp_rel(D_800A30FD)($gp)
    /* E0B8 8001D8B8 03004014 */  bnez       $v0, .L8001D8C8
    /* E0BC 8001D8BC 00000000 */   nop
    /* E0C0 8001D8C0 0F5B000C */  jal        sys_Panic
    /* E0C4 8001D8C4 00000000 */   nop
  .L8001D8C8:
    /* E0C8 8001D8C8 1080103C */  lui        $s0, %hi(D_800FF6A8)
    /* E0CC 8001D8CC A8F61026 */  addiu      $s0, $s0, %lo(D_800FF6A8)
    /* E0D0 8001D8D0 21200002 */  addu       $a0, $s0, $zero
    /* E0D4 8001D8D4 21284002 */  addu       $a1, $s2, $zero
    /* E0D8 8001D8D8 48E4010C */  jal        func_80079120
    /* E0DC 8001D8DC 21302002 */   addu      $a2, $s1, $zero
    /* E0E0 8001D8E0 4C6F010C */  jal        func_8005BD30
    /* E0E4 8001D8E4 23201202 */   subu      $a0, $s0, $s2
  .L8001D8E8:
    /* E0E8 8001D8E8 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* E0EC 8001D8EC 1800B28F */  lw         $s2, 0x18($sp)
    /* E0F0 8001D8F0 1400B18F */  lw         $s1, 0x14($sp)
    /* E0F4 8001D8F4 1000B08F */  lw         $s0, 0x10($sp)
    /* E0F8 8001D8F8 2000BD27 */  addiu      $sp, $sp, 0x20
    /* E0FC 8001D8FC 0800E003 */  jr         $ra
    /* E100 8001D900 00000000 */   nop
endlabel func_8001D790
