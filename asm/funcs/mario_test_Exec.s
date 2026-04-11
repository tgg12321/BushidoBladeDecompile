glabel mario_test_Exec
    /* E4B0 8001DCB0 D8FFBD27 */  addiu      $sp, $sp, -0x28
    /* E4B4 8001DCB4 2400BFAF */  sw         $ra, 0x24($sp)
    /* E4B8 8001DCB8 2000B2AF */  sw         $s2, 0x20($sp)
    /* E4BC 8001DCBC 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* E4C0 8001DCC0 6B6D010C */  jal        obj_InitChars
    /* E4C4 8001DCC4 1800B0AF */   sw        $s0, 0x18($sp)
    /* E4C8 8001DCC8 0A80033C */  lui        $v1, %hi(D_800A3768)
    /* E4CC 8001DCCC 68376390 */  lbu        $v1, %lo(D_800A3768)($v1)
    /* E4D0 8001DCD0 14000224 */  addiu      $v0, $zero, 0x14
    /* E4D4 8001DCD4 05006210 */  beq        $v1, $v0, .L8001DCEC
    /* E4D8 8001DCD8 00000000 */   nop
    /* E4DC 8001DCDC 225A000C */  jal        gpu_InitDisplay
    /* E4E0 8001DCE0 00000000 */   nop
    /* E4E4 8001DCE4 345A000C */  jal        gpu_DisableDisplay
    /* E4E8 8001DCE8 00000000 */   nop
  .L8001DCEC:
    /* E4EC 8001DCEC 3D5D000C */  jal        gnd_disp_loop_ctrl
    /* E4F0 8001DCF0 00000000 */   nop
    /* E4F4 8001DCF4 1A5A000C */  jal        gpu_EnableDisplay
    /* E4F8 8001DCF8 00000000 */   nop
    /* E4FC 8001DCFC 8BF8000C */  jal        func_8003E22C
    /* E500 8001DD00 00000000 */   nop
    /* E504 8001DD04 0FC1000C */  jal        func_8003043C
    /* E508 8001DD08 00000000 */   nop
    /* E50C 8001DD0C 10C8000C */  jal        func_80032040
    /* E510 8001DD10 00000000 */   nop
    /* E514 8001DD14 0A80043C */  lui        $a0, %hi(D_800A38BA)
    /* E518 8001DD18 BA388484 */  lh         $a0, %lo(D_800A38BA)($a0)
    /* E51C 8001DD1C 86FC000C */  jal        game_SetPlayerCount
    /* E520 8001DD20 00000000 */   nop
    /* E524 8001DD24 0A80023C */  lui        $v0, %hi(D_800A38BA)
    /* E528 8001DD28 BA384284 */  lh         $v0, %lo(D_800A38BA)($v0)
    /* E52C 8001DD2C 00000000 */  nop
    /* E530 8001DD30 02004010 */  beqz       $v0, .L8001DD3C
    /* E534 8001DD34 2D000424 */   addiu     $a0, $zero, 0x2D
    /* E538 8001DD38 50000424 */  addiu      $a0, $zero, 0x50
  .L8001DD3C:
    /* E53C 8001DD3C B159000C */  jal        disp_CalcFov
    /* E540 8001DD40 21800000 */   addu      $s0, $zero, $zero
    /* E544 8001DD44 FFFB010C */  jal        func_8007EFFC
    /* E548 8001DD48 21204000 */   addu      $a0, $v0, $zero
  .L8001DD4C:
    /* E54C 8001DD4C 0A80023C */  lui        $v0, %hi(D_800A38DC)
    /* E550 8001DD50 DC384284 */  lh         $v0, %lo(D_800A38DC)($v0)
    /* E554 8001DD54 00000000 */  nop
    /* E558 8001DD58 03004010 */  beqz       $v0, .L8001DD68
    /* E55C 8001DD5C 21200000 */   addu      $a0, $zero, $zero
    /* E560 8001DD60 8105010C */  jal        player_SetCharId
    /* E564 8001DD64 21280000 */   addu      $a1, $zero, $zero
  .L8001DD68:
    /* E568 8001DD68 1080013C */  lui        $at, %hi(D_80102780)
    /* E56C 8001DD6C 21083000 */  addu       $at, $at, $s0
    /* E570 8001DD70 80272580 */  lb         $a1, %lo(D_80102780)($at)
    /* E574 8001DD74 1080013C */  lui        $at, %hi(D_8010277C)
    /* E578 8001DD78 21083000 */  addu       $at, $at, $s0
    /* E57C 8001DD7C 7C272680 */  lb         $a2, %lo(D_8010277C)($at)
    /* E580 8001DD80 1080013C */  lui        $at, %hi(D_8010277E)
    /* E584 8001DD84 21083000 */  addu       $at, $at, $s0
    /* E588 8001DD88 7E272780 */  lb         $a3, %lo(D_8010277E)($at)
    /* E58C 8001DD8C 21200002 */  addu       $a0, $s0, $zero
    /* E590 8001DD90 6089000C */  jal        func_80022580
    /* E594 8001DD94 1000A0AF */   sw        $zero, 0x10($sp)
    /* E598 8001DD98 0A80023C */  lui        $v0, %hi(D_800A38BA)
    /* E59C 8001DD9C BA384284 */  lh         $v0, %lo(D_800A38BA)($v0)
    /* E5A0 8001DDA0 00000000 */  nop
    /* E5A4 8001DDA4 08004010 */  beqz       $v0, .L8001DDC8
    /* E5A8 8001DDA8 00000000 */   nop
    /* E5AC 8001DDAC 0A80023C */  lui        $v0, %hi(D_800A36F6)
    /* E5B0 8001DDB0 F6364284 */  lh         $v0, %lo(D_800A36F6)($v0)
    /* E5B4 8001DDB4 00000000 */  nop
    /* E5B8 8001DDB8 03005014 */  bne        $v0, $s0, .L8001DDC8
    /* E5BC 8001DDBC 00000000 */   nop
    /* E5C0 8001DDC0 59F8000C */  jal        func_8003E164
    /* E5C4 8001DDC4 0100042E */   sltiu     $a0, $s0, 0x1
  .L8001DDC8:
    /* E5C8 8001DDC8 01001026 */  addiu      $s0, $s0, 0x1
    /* E5CC 8001DDCC 0200022A */  slti       $v0, $s0, 0x2
    /* E5D0 8001DDD0 DEFF4014 */  bnez       $v0, .L8001DD4C
    /* E5D4 8001DDD4 00000000 */   nop
    /* E5D8 8001DDD8 F8FF000C */  jal        func_8003FFE0
    /* E5DC 8001DDDC 21200000 */   addu      $a0, $zero, $zero
    /* E5E0 8001DDE0 F8FF000C */  jal        func_8003FFE0
    /* E5E4 8001DDE4 01000424 */   addiu     $a0, $zero, 0x1
    /* E5E8 8001DDE8 0A80023C */  lui        $v0, %hi(D_800A3670)
    /* E5EC 8001DDEC 70364290 */  lbu        $v0, %lo(D_800A3670)($v0)
    /* E5F0 8001DDF0 00000000 */  nop
    /* E5F4 8001DDF4 DA004014 */  bnez       $v0, .L8001E160
    /* E5F8 8001DDF8 05000224 */   addiu     $v0, $zero, 0x5
    /* E5FC 8001DDFC E724010C */  jal        func_8004939C
    /* E600 8001DE00 1980123C */   lui       $s2, (0x80190800 >> 16)
    /* E604 8001DE04 4E83000C */  jal        func_80020D38
    /* E608 8001DE08 00085236 */   ori       $s2, $s2, (0x80190800 & 0xFFFF)
    /* E60C 8001DE0C 21800000 */  addu       $s0, $zero, $zero
    /* E610 8001DE10 21880000 */  addu       $s1, $zero, $zero
  .L8001DE14:
    /* E614 8001DE14 0A80023C */  lui        $v0, %hi(D_800A38BA)
    /* E618 8001DE18 BA384284 */  lh         $v0, %lo(D_800A38BA)($v0)
    /* E61C 8001DE1C 00000000 */  nop
    /* E620 8001DE20 12004010 */  beqz       $v0, .L8001DE6C
    /* E624 8001DE24 00000000 */   nop
    /* E628 8001DE28 0A80023C */  lui        $v0, %hi(D_800A36F6)
    /* E62C 8001DE2C F6364284 */  lh         $v0, %lo(D_800A36F6)($v0)
    /* E630 8001DE30 00000000 */  nop
    /* E634 8001DE34 0D005014 */  bne        $v0, $s0, .L8001DE6C
    /* E638 8001DE38 21200002 */   addu      $a0, $s0, $zero
    /* E63C 8001DE3C 1080013C */  lui        $at, %hi(D_8010277C)
    /* E640 8001DE40 21083000 */  addu       $at, $at, $s0
    /* E644 8001DE44 7C272280 */  lb         $v0, %lo(D_8010277C)($at)
    /* E648 8001DE48 0980013C */  lui        $at, %hi(D_8008D578)
    /* E64C 8001DE4C 21082200 */  addu       $at, $at, $v0
    /* E650 8001DE50 78D52590 */  lbu        $a1, %lo(D_8008D578)($at)
    /* E654 8001DE54 4401010C */  jal        func_80040510
    /* E658 8001DE58 21304002 */   addu      $a2, $s2, $zero
    /* E65C 8001DE5C B422010C */  jal        func_80048AD0
    /* E660 8001DE60 21200002 */   addu      $a0, $s0, $zero
    /* E664 8001DE64 B0770008 */  j          .L8001DEC0
    /* E668 8001DE68 00000000 */   nop
  .L8001DE6C:
    /* E66C 8001DE6C 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* E670 8001DE70 DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* E674 8001DE74 03000224 */  addiu      $v0, $zero, 0x3
    /* E678 8001DE78 07006214 */  bne        $v1, $v0, .L8001DE98
    /* E67C 8001DE7C 01000224 */   addiu     $v0, $zero, 0x1
    /* E680 8001DE80 05000216 */  bne        $s0, $v0, .L8001DE98
    /* E684 8001DE84 01000424 */   addiu     $a0, $zero, 0x1
    /* E688 8001DE88 0A80053C */  lui        $a1, %hi(D_800A38DE)
    /* E68C 8001DE8C DE38A590 */  lbu        $a1, %lo(D_800A38DE)($a1)
    /* E690 8001DE90 AE770008 */  j          .L8001DEB8
    /* E694 8001DE94 21300000 */   addu      $a2, $zero, $zero
  .L8001DE98:
    /* E698 8001DE98 1080013C */  lui        $at, %hi(D_8010277C)
    /* E69C 8001DE9C 21083000 */  addu       $at, $at, $s0
    /* E6A0 8001DEA0 7C272280 */  lb         $v0, %lo(D_8010277C)($at)
    /* E6A4 8001DEA4 21200002 */  addu       $a0, $s0, $zero
    /* E6A8 8001DEA8 0980013C */  lui        $at, %hi(D_8008D578)
    /* E6AC 8001DEAC 21082200 */  addu       $at, $at, $v0
    /* E6B0 8001DEB0 78D52590 */  lbu        $a1, %lo(D_8008D578)($at)
    /* E6B4 8001DEB4 21304002 */  addu       $a2, $s2, $zero
  .L8001DEB8:
    /* E6B8 8001DEB8 4401010C */  jal        func_80040510
    /* E6BC 8001DEBC 00000000 */   nop
  .L8001DEC0:
    /* E6C0 8001DEC0 1080013C */  lui        $at, %hi(D_80101EDA)
    /* E6C4 8001DEC4 21083100 */  addu       $at, $at, $s1
    /* E6C8 8001DEC8 DA1E2484 */  lh         $a0, %lo(D_80101EDA)($at)
    /* E6CC 8001DECC F924010C */  jal        func_800493E4
    /* E6D0 8001DED0 00000000 */   nop
    /* E6D4 8001DED4 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* E6D8 8001DED8 DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* E6DC 8001DEDC 03000224 */  addiu      $v0, $zero, 0x3
    /* E6E0 8001DEE0 04006214 */  bne        $v1, $v0, .L8001DEF4
    /* E6E4 8001DEE4 02000224 */   addiu     $v0, $zero, 0x2
    /* E6E8 8001DEE8 01000224 */  addiu      $v0, $zero, 0x1
    /* E6EC 8001DEEC 25000212 */  beq        $s0, $v0, .L8001DF84
    /* E6F0 8001DEF0 02000224 */   addiu     $v0, $zero, 0x2
  .L8001DEF4:
    /* E6F4 8001DEF4 07006214 */  bne        $v1, $v0, .L8001DF14
    /* E6F8 8001DEF8 05000224 */   addiu     $v0, $zero, 0x5
    /* E6FC 8001DEFC 0A80023C */  lui        $v0, %hi(D_800A389A)
    /* E700 8001DF00 9A384290 */  lbu        $v0, %lo(D_800A389A)($v0)
    /* E704 8001DF04 00000000 */  nop
    /* E708 8001DF08 04004010 */  beqz       $v0, .L8001DF1C
    /* E70C 8001DF0C 00000000 */   nop
    /* E710 8001DF10 05000224 */  addiu      $v0, $zero, 0x5
  .L8001DF14:
    /* E714 8001DF14 0D006214 */  bne        $v1, $v0, .L8001DF4C
    /* E718 8001DF18 00000000 */   nop
  .L8001DF1C:
    /* E71C 8001DF1C 1080013C */  lui        $at, %hi(D_80101ED2)
    /* E720 8001DF20 21083100 */  addu       $at, $at, $s1
    /* E724 8001DF24 D21E2384 */  lh         $v1, %lo(D_80101ED2)($at)
    /* E728 8001DF28 0980043C */  lui        $a0, %hi(D_8008E6A4)
    /* E72C 8001DF2C A4E68424 */  addiu      $a0, $a0, %lo(D_8008E6A4)
    /* E730 8001DF30 40100300 */  sll        $v0, $v1, 1
    /* E734 8001DF34 21104300 */  addu       $v0, $v0, $v1
    /* E738 8001DF38 1080013C */  lui        $at, %hi(D_80101ED6)
    /* E73C 8001DF3C 21083100 */  addu       $at, $at, $s1
    /* E740 8001DF40 D61E2384 */  lh         $v1, %lo(D_80101ED6)($at)
    /* E744 8001DF44 DC770008 */  j          .L8001DF70
    /* E748 8001DF48 40100200 */   sll       $v0, $v0, 1
  .L8001DF4C:
    /* E74C 8001DF4C 0980043C */  lui        $a0, %hi(D_8008E5CC)
    /* E750 8001DF50 CCE58424 */  addiu      $a0, $a0, %lo(D_8008E5CC)
    /* E754 8001DF54 1080013C */  lui        $at, %hi(D_80101ED2)
    /* E758 8001DF58 21083100 */  addu       $at, $at, $s1
    /* E75C 8001DF5C D21E2284 */  lh         $v0, %lo(D_80101ED2)($at)
    /* E760 8001DF60 1080013C */  lui        $at, %hi(D_80101ED6)
    /* E764 8001DF64 21083100 */  addu       $at, $at, $s1
    /* E768 8001DF68 D61E2384 */  lh         $v1, %lo(D_80101ED6)($at)
    /* E76C 8001DF6C C0100200 */  sll        $v0, $v0, 3
  .L8001DF70:
    /* E770 8001DF70 21104400 */  addu       $v0, $v0, $a0
    /* E774 8001DF74 21104300 */  addu       $v0, $v0, $v1
    /* E778 8001DF78 00004590 */  lbu        $a1, 0x0($v0)
    /* E77C 8001DF7C 3525010C */  jal        func_800494D4
    /* E780 8001DF80 21200002 */   addu      $a0, $s0, $zero
  .L8001DF84:
    /* E784 8001DF84 1080013C */  lui        $at, %hi(D_80101EDC)
    /* E788 8001DF88 21083100 */  addu       $at, $at, $s1
    /* E78C 8001DF8C DC1E2384 */  lh         $v1, %lo(D_80101EDC)($at)
    /* E790 8001DF90 FFFF0224 */  addiu      $v0, $zero, -0x1
    /* E794 8001DF94 10006210 */  beq        $v1, $v0, .L8001DFD8
    /* E798 8001DF98 00000000 */   nop
    /* E79C 8001DF9C 0980013C */  lui        $at, %hi(D_8008EB80)
    /* E7A0 8001DFA0 21082300 */  addu       $at, $at, $v1
    /* E7A4 8001DFA4 80EB2490 */  lbu        $a0, %lo(D_8008EB80)($at)
    /* E7A8 8001DFA8 F924010C */  jal        func_800493E4
    /* E7AC 8001DFAC 00000000 */   nop
    /* E7B0 8001DFB0 1080013C */  lui        $at, %hi(D_80101EDC)
    /* E7B4 8001DFB4 21083100 */  addu       $at, $at, $s1
    /* E7B8 8001DFB8 DC1E2384 */  lh         $v1, %lo(D_80101EDC)($at)
    /* E7BC 8001DFBC 0E000224 */  addiu      $v0, $zero, 0xE
    /* E7C0 8001DFC0 05006214 */  bne        $v1, $v0, .L8001DFD8
    /* E7C4 8001DFC4 00000000 */   nop
    /* E7C8 8001DFC8 0980043C */  lui        $a0, %hi(D_8008EB8E)
    /* E7CC 8001DFCC 8EEB8490 */  lbu        $a0, %lo(D_8008EB8E)($a0)
    /* E7D0 8001DFD0 F924010C */  jal        func_800493E4
    /* E7D4 8001DFD4 03008424 */   addiu     $a0, $a0, 0x3
  .L8001DFD8:
    /* E7D8 8001DFD8 01001026 */  addiu      $s0, $s0, 0x1
    /* E7DC 8001DFDC 0200022A */  slti       $v0, $s0, 0x2
    /* E7E0 8001DFE0 8CFF4014 */  bnez       $v0, .L8001DE14
    /* E7E4 8001DFE4 4C043126 */   addiu     $s1, $s1, 0x44C
    /* E7E8 8001DFE8 6125010C */  jal        func_80049584
    /* E7EC 8001DFEC 21204002 */   addu      $a0, $s2, $zero
    /* E7F0 8001DFF0 21200000 */  addu       $a0, $zero, $zero
    /* E7F4 8001DFF4 A205010C */  jal        func_80041688
    /* E7F8 8001DFF8 21280000 */   addu      $a1, $zero, $zero
    /* E7FC 8001DFFC 01000424 */  addiu      $a0, $zero, 0x1
    /* E800 8001E000 A205010C */  jal        func_80041688
    /* E804 8001E004 21280000 */   addu      $a1, $zero, $zero
    /* E808 8001E008 0A80023C */  lui        $v0, %hi(D_800A38DC)
    /* E80C 8001E00C DC384284 */  lh         $v0, %lo(D_800A38DC)($v0)
    /* E810 8001E010 00000000 */  nop
    /* E814 8001E014 0E004014 */  bnez       $v0, .L8001E050
    /* E818 8001E018 00000000 */   nop
    /* E81C 8001E01C 0A80023C */  lui        $v0, %hi(D_800A3712)
    /* E820 8001E020 12374290 */  lbu        $v0, %lo(D_800A3712)($v0)
    /* E824 8001E024 00000000 */  nop
    /* E828 8001E028 09004014 */  bnez       $v0, .L8001E050
    /* E82C 8001E02C 00000000 */   nop
    /* E830 8001E030 0A80043C */  lui        $a0, %hi(D_800A37B4)
    /* E834 8001E034 B4378490 */  lbu        $a0, %lo(D_800A37B4)($a0)
    /* E838 8001E038 0A80053C */  lui        $a1, %hi(D_800A37B5)
    /* E83C 8001E03C B537A590 */  lbu        $a1, %lo(D_800A37B5)($a1)
    /* E840 8001E040 0A80063C */  lui        $a2, %hi(D_800A37B6)
    /* E844 8001E044 B637C690 */  lbu        $a2, %lo(D_800A37B6)($a2)
    /* E848 8001E048 32780008 */  j          .L8001E0C8
    /* E84C 8001E04C 00000000 */   nop
  .L8001E050:
    /* E850 8001E050 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* E854 8001E054 DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* E858 8001E058 03000224 */  addiu      $v0, $zero, 0x3
    /* E85C 8001E05C 09006214 */  bne        $v1, $v0, .L8001E084
    /* E860 8001E060 02000224 */   addiu     $v0, $zero, 0x2
    /* E864 8001E064 0A80043C */  lui        $a0, %hi(D_800A38EC)
    /* E868 8001E068 EC388490 */  lbu        $a0, %lo(D_800A38EC)($a0)
    /* E86C 8001E06C 0A80053C */  lui        $a1, %hi(D_800A38ED)
    /* E870 8001E070 ED38A590 */  lbu        $a1, %lo(D_800A38ED)($a1)
    /* E874 8001E074 0A80063C */  lui        $a2, %hi(D_800A38EE)
    /* E878 8001E078 EE38C690 */  lbu        $a2, %lo(D_800A38EE)($a2)
    /* E87C 8001E07C 32780008 */  j          .L8001E0C8
    /* E880 8001E080 00000000 */   nop
  .L8001E084:
    /* E884 8001E084 12006214 */  bne        $v1, $v0, .L8001E0D0
    /* E888 8001E088 00000000 */   nop
    /* E88C 8001E08C 1080023C */  lui        $v0, %hi(D_80101ED2)
    /* E890 8001E090 D21E4284 */  lh         $v0, %lo(D_80101ED2)($v0)
    /* E894 8001E094 0A80033C */  lui        $v1, %hi(D_800A3100)
    /* E898 8001E098 00316324 */  addiu      $v1, $v1, %lo(D_800A3100)
    /* E89C 8001E09C 0980013C */  lui        $at, %hi(D_8008D9EC)
    /* E8A0 8001E0A0 21082200 */  addu       $at, $at, $v0
    /* E8A4 8001E0A4 ECD92290 */  lbu        $v0, %lo(D_8008D9EC)($at)
    /* E8A8 8001E0A8 0A80043C */  lui        $a0, %hi(D_800A389A)
    /* E8AC 8001E0AC 9A388490 */  lbu        $a0, %lo(D_800A389A)($a0)
    /* E8B0 8001E0B0 80100200 */  sll        $v0, $v0, 2
    /* E8B4 8001E0B4 06008014 */  bnez       $a0, .L8001E0D0
    /* E8B8 8001E0B8 21104300 */   addu      $v0, $v0, $v1
    /* E8BC 8001E0BC 00004490 */  lbu        $a0, 0x0($v0)
    /* E8C0 8001E0C0 01004590 */  lbu        $a1, 0x1($v0)
    /* E8C4 8001E0C4 02004690 */  lbu        $a2, 0x2($v0)
  .L8001E0C8:
    /* E8C8 8001E0C8 FD06010C */  jal        saTan4FireDisp
    /* E8CC 8001E0CC 00000000 */   nop
  .L8001E0D0:
    /* E8D0 8001E0D0 E475000C */  jal        se_data_set
    /* E8D4 8001E0D4 00000000 */   nop
    /* E8D8 8001E0D8 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* E8DC 8001E0DC DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* E8E0 8001E0E0 05000224 */  addiu      $v0, $zero, 0x5
    /* E8E4 8001E0E4 06006214 */  bne        $v1, $v0, .L8001E100
    /* E8E8 8001E0E8 03000224 */   addiu     $v0, $zero, 0x3
    /* E8EC 8001E0EC 4176000C */  jal        func_8001D904
    /* E8F0 8001E0F0 00000000 */   nop
    /* E8F4 8001E0F4 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* E8F8 8001E0F8 DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* E8FC 8001E0FC 03000224 */  addiu      $v0, $zero, 0x3
  .L8001E100:
    /* E900 8001E100 05006214 */  bne        $v1, $v0, .L8001E118
    /* E904 8001E104 00000000 */   nop
    /* E908 8001E108 6676000C */  jal        func_8001D998
    /* E90C 8001E10C 00000000 */   nop
    /* E910 8001E110 E776000C */  jal        func_8001DB9C
    /* E914 8001E114 00000000 */   nop
  .L8001E118:
    /* E918 8001E118 1080053C */  lui        $a1, %hi(D_8010277E)
    /* E91C 8001E11C 7E27A580 */  lb         $a1, %lo(D_8010277E)($a1)
    /* E920 8001E120 1080073C */  lui        $a3, %hi(D_8010277F)
    /* E924 8001E124 7F27E780 */  lb         $a3, %lo(D_8010277F)($a3)
    /* E928 8001E128 1080023C */  lui        $v0, %hi(D_8010277C)
    /* E92C 8001E12C 7C274280 */  lb         $v0, %lo(D_8010277C)($v0)
    /* E930 8001E130 1080033C */  lui        $v1, %hi(D_8010277D)
    /* E934 8001E134 7D276380 */  lb         $v1, %lo(D_8010277D)($v1)
    /* E938 8001E138 0980013C */  lui        $at, %hi(D_8008D538)
    /* E93C 8001E13C 21082200 */  addu       $at, $at, $v0
    /* E940 8001E140 38D52490 */  lbu        $a0, %lo(D_8008D538)($at)
    /* E944 8001E144 0980013C */  lui        $at, %hi(D_8008D538)
    /* E948 8001E148 21082300 */  addu       $at, $at, $v1
    /* E94C 8001E14C 38D52690 */  lbu        $a2, %lo(D_8008D538)($at)
    /* E950 8001E150 9D83000C */  jal        DispPracticeMenuTex_B
    /* E954 8001E154 00000000 */   nop
    /* E958 8001E158 5F780008 */  j          .L8001E17C
    /* E95C 8001E15C 00000000 */   nop
  .L8001E160:
    /* E960 8001E160 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* E964 8001E164 DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* E968 8001E168 00000000 */  nop
    /* E96C 8001E16C 03006214 */  bne        $v1, $v0, .L8001E17C
    /* E970 8001E170 01000224 */   addiu     $v0, $zero, 0x1
    /* E974 8001E174 0A80013C */  lui        $at, %hi(D_800A391E)
    /* E978 8001E178 1E3922A0 */  sb         $v0, %lo(D_800A391E)($at)
  .L8001E17C:
    /* E97C 8001E17C 8484000C */  jal        func_80021210
    /* E980 8001E180 02001024 */   addiu     $s0, $zero, 0x2
    /* E984 8001E184 A084000C */  jal        title_mv_exec
    /* E988 8001E188 21200000 */   addu      $a0, $zero, $zero
    /* E98C 8001E18C A084000C */  jal        title_mv_exec
    /* E990 8001E190 01000424 */   addiu     $a0, $zero, 0x1
    /* E994 8001E194 CD8B000C */  jal        func_80022F34
    /* E998 8001E198 00000000 */   nop
    /* E99C 8001E19C 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* E9A0 8001E1A0 DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* E9A4 8001E1A4 00000000 */  nop
    /* E9A8 8001E1A8 03007010 */  beq        $v1, $s0, .L8001E1B8
    /* E9AC 8001E1AC 05000224 */   addiu     $v0, $zero, 0x5
    /* E9B0 8001E1B0 41006214 */  bne        $v1, $v0, .L8001E2B8
    /* E9B4 8001E1B4 00000000 */   nop
  .L8001E1B8:
    /* E9B8 8001E1B8 0A80023C */  lui        $v0, %hi(D_800A3670)
    /* E9BC 8001E1BC 70364290 */  lbu        $v0, %lo(D_800A3670)($v0)
    /* E9C0 8001E1C0 00000000 */  nop
    /* E9C4 8001E1C4 27004010 */  beqz       $v0, .L8001E264
    /* E9C8 8001E1C8 00000000 */   nop
    /* E9CC 8001E1CC 3286000C */  jal        func_800218C8
    /* E9D0 8001E1D0 21200000 */   addu      $a0, $zero, $zero
    /* E9D4 8001E1D4 5D86000C */  jal        func_80021974
    /* E9D8 8001E1D8 21200000 */   addu      $a0, $zero, $zero
    /* E9DC 8001E1DC 21200000 */  addu       $a0, $zero, $zero
    /* E9E0 8001E1E0 21284000 */  addu       $a1, $v0, $zero
    /* E9E4 8001E1E4 1080013C */  lui        $at, %hi(D_80101F26)
    /* E9E8 8001E1E8 261F20A4 */  sh         $zero, %lo(D_80101F26)($at)
    /* E9EC 8001E1EC A686000C */  jal        func_80021A98
    /* E9F0 8001E1F0 21300000 */   addu      $a2, $zero, $zero
    /* E9F4 8001E1F4 0A80023C */  lui        $v0, %hi(D_800A38DC)
    /* E9F8 8001E1F8 DC384284 */  lh         $v0, %lo(D_800A38DC)($v0)
    /* E9FC 8001E1FC 00000000 */  nop
    /* EA00 8001E200 0E005014 */  bne        $v0, $s0, .L8001E23C
    /* EA04 8001E204 00000000 */   nop
    /* EA08 8001E208 0A80023C */  lui        $v0, %hi(D_800A389A)
    /* EA0C 8001E20C 9A384290 */  lbu        $v0, %lo(D_800A389A)($v0)
    /* EA10 8001E210 00000000 */  nop
    /* EA14 8001E214 09004014 */  bnez       $v0, .L8001E23C
    /* EA18 8001E218 00000000 */   nop
    /* EA1C 8001E21C 4186000C */  jal        func_80021904
    /* EA20 8001E220 01000424 */   addiu     $a0, $zero, 0x1
    /* EA24 8001E224 1080013C */  lui        $at, %hi(D_80102372)
    /* EA28 8001E228 722320A4 */  sh         $zero, %lo(D_80102372)($at)
    /* EA2C 8001E22C 01000424 */  addiu      $a0, $zero, 0x1
    /* EA30 8001E230 21284000 */  addu       $a1, $v0, $zero
    /* EA34 8001E234 C1780008 */  j          .L8001E304
    /* EA38 8001E238 21300000 */   addu      $a2, $zero, $zero
  .L8001E23C:
    /* EA3C 8001E23C 3286000C */  jal        func_800218C8
    /* EA40 8001E240 01000424 */   addiu     $a0, $zero, 0x1
    /* EA44 8001E244 5D86000C */  jal        func_80021974
    /* EA48 8001E248 01000424 */   addiu     $a0, $zero, 0x1
    /* EA4C 8001E24C 1080013C */  lui        $at, %hi(D_80102372)
    /* EA50 8001E250 722320A4 */  sh         $zero, %lo(D_80102372)($at)
    /* EA54 8001E254 01000424 */  addiu      $a0, $zero, 0x1
    /* EA58 8001E258 21284000 */  addu       $a1, $v0, $zero
    /* EA5C 8001E25C C1780008 */  j          .L8001E304
    /* EA60 8001E260 21300000 */   addu      $a2, $zero, $zero
  .L8001E264:
    /* EA64 8001E264 3286000C */  jal        func_800218C8
    /* EA68 8001E268 21200000 */   addu      $a0, $zero, $zero
    /* EA6C 8001E26C 3286000C */  jal        func_800218C8
    /* EA70 8001E270 01000424 */   addiu     $a0, $zero, 0x1
    /* EA74 8001E274 7986000C */  jal        func_800219E4
    /* EA78 8001E278 21200000 */   addu      $a0, $zero, $zero
    /* EA7C 8001E27C 21200000 */  addu       $a0, $zero, $zero
    /* EA80 8001E280 21284000 */  addu       $a1, $v0, $zero
    /* EA84 8001E284 01001024 */  addiu      $s0, $zero, 0x1
    /* EA88 8001E288 1080013C */  lui        $at, %hi(D_80101F26)
    /* EA8C 8001E28C 261F30A4 */  sh         $s0, %lo(D_80101F26)($at)
    /* EA90 8001E290 A686000C */  jal        func_80021A98
    /* EA94 8001E294 01000624 */   addiu     $a2, $zero, 0x1
    /* EA98 8001E298 7986000C */  jal        func_800219E4
    /* EA9C 8001E29C 01000424 */   addiu     $a0, $zero, 0x1
    /* EAA0 8001E2A0 01000424 */  addiu      $a0, $zero, 0x1
    /* EAA4 8001E2A4 21284000 */  addu       $a1, $v0, $zero
    /* EAA8 8001E2A8 1080013C */  lui        $at, %hi(D_80102372)
    /* EAAC 8001E2AC 722330A4 */  sh         $s0, %lo(D_80102372)($at)
    /* EAB0 8001E2B0 C1780008 */  j          .L8001E304
    /* EAB4 8001E2B4 01000624 */   addiu     $a2, $zero, 0x1
  .L8001E2B8:
    /* EAB8 8001E2B8 3286000C */  jal        func_800218C8
    /* EABC 8001E2BC 21200000 */   addu      $a0, $zero, $zero
    /* EAC0 8001E2C0 3286000C */  jal        func_800218C8
    /* EAC4 8001E2C4 01000424 */   addiu     $a0, $zero, 0x1
    /* EAC8 8001E2C8 5D86000C */  jal        func_80021974
    /* EACC 8001E2CC 21200000 */   addu      $a0, $zero, $zero
    /* EAD0 8001E2D0 21200000 */  addu       $a0, $zero, $zero
    /* EAD4 8001E2D4 21284000 */  addu       $a1, $v0, $zero
    /* EAD8 8001E2D8 1080013C */  lui        $at, %hi(D_80101F26)
    /* EADC 8001E2DC 261F20A4 */  sh         $zero, %lo(D_80101F26)($at)
    /* EAE0 8001E2E0 A686000C */  jal        func_80021A98
    /* EAE4 8001E2E4 21300000 */   addu      $a2, $zero, $zero
    /* EAE8 8001E2E8 5D86000C */  jal        func_80021974
    /* EAEC 8001E2EC 01000424 */   addiu     $a0, $zero, 0x1
    /* EAF0 8001E2F0 01000424 */  addiu      $a0, $zero, 0x1
    /* EAF4 8001E2F4 21284000 */  addu       $a1, $v0, $zero
    /* EAF8 8001E2F8 21300000 */  addu       $a2, $zero, $zero
    /* EAFC 8001E2FC 1080013C */  lui        $at, %hi(D_80102372)
    /* EB00 8001E300 722320A4 */  sh         $zero, %lo(D_80102372)($at)
  .L8001E304:
    /* EB04 8001E304 A686000C */  jal        func_80021A98
    /* EB08 8001E308 00000000 */   nop
    /* EB0C 8001E30C 1080103C */  lui        $s0, %hi(D_80101EC8)
    /* EB10 8001E310 C81E1026 */  addiu      $s0, $s0, %lo(D_80101EC8)
    /* EB14 8001E314 21200002 */  addu       $a0, $s0, $zero
    /* EB18 8001E318 4C041126 */  addiu      $s1, $s0, 0x44C
    /* EB1C 8001E31C FFFF0224 */  addiu      $v0, $zero, -0x1
    /* EB20 8001E320 0A80013C */  lui        $at, %hi(D_800A382E)
    /* EB24 8001E324 2E3820A4 */  sh         $zero, %lo(D_800A382E)($at)
    /* EB28 8001E328 0A80013C */  lui        $at, %hi(D_800A3748)
    /* EB2C 8001E32C 483722A0 */  sb         $v0, %lo(D_800A3748)($at)
    /* EB30 8001E330 A56C000C */  jal        func_8001B294
    /* EB34 8001E334 21282002 */   addu      $a1, $s1, $zero
    /* EB38 8001E338 0A80023C */  lui        $v0, %hi(D_800A38BA)
    /* EB3C 8001E33C BA384284 */  lh         $v0, %lo(D_800A38BA)($v0)
    /* EB40 8001E340 00000000 */  nop
    /* EB44 8001E344 03004010 */  beqz       $v0, .L8001E354
    /* EB48 8001E348 21200002 */   addu      $a0, $s0, $zero
    /* EB4C 8001E34C F06C000C */  jal        func_8001B3C0
    /* EB50 8001E350 21282002 */   addu      $a1, $s1, $zero
  .L8001E354:
    /* EB54 8001E354 B2E4000C */  jal        func_800392C8
    /* EB58 8001E358 00000000 */   nop
    /* EB5C 8001E35C 5E84010C */  jal        game_Cleanup
    /* EB60 8001E360 00000000 */   nop
    /* EB64 8001E364 F976000C */  jal        func_8001DBE4
    /* EB68 8001E368 00000000 */   nop
    /* EB6C 8001E36C FF000224 */  addiu      $v0, $zero, 0xFF
    /* EB70 8001E370 0A80013C */  lui        $at, %hi(D_800A3768)
    /* EB74 8001E374 683722A0 */  sb         $v0, %lo(D_800A3768)($at)
    /* EB78 8001E378 0A80013C */  lui        $at, %hi(D_800A36A8)
    /* EB7C 8001E37C A83620A0 */  sb         $zero, %lo(D_800A36A8)($at)
    /* EB80 8001E380 205B000C */  jal        file_LoadOverlay
    /* EB84 8001E384 00000000 */   nop
    /* EB88 8001E388 01000224 */  addiu      $v0, $zero, 0x1
    /* EB8C 8001E38C 0A80013C */  lui        $at, %hi(D_800A3670)
    /* EB90 8001E390 703620A0 */  sb         $zero, %lo(D_800A3670)($at)
    /* EB94 8001E394 0A80013C */  lui        $at, %hi(D_800A3834)
    /* EB98 8001E398 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* EB9C 8001E39C 0872000C */  jal        func_8001C820
    /* EBA0 8001E3A0 00000000 */   nop
    /* EBA4 8001E3A4 A376000C */  jal        func_8001DA8C
    /* EBA8 8001E3A8 00000000 */   nop
    /* EBAC 8001E3AC 44CD000C */  jal        func_80033510
    /* EBB0 8001E3B0 00000000 */   nop
    /* EBB4 8001E3B4 0A80043C */  lui        $a0, %hi(D_800A36A4)
    /* EBB8 8001E3B8 A4368484 */  lh         $a0, %lo(D_800A36A4)($a0)
    /* EBBC 8001E3BC A16F010C */  jal        func_8005BE84
    /* EBC0 8001E3C0 00000000 */   nop
    /* EBC4 8001E3C4 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* EBC8 8001E3C8 DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* EBCC 8001E3CC 06000224 */  addiu      $v0, $zero, 0x6
    /* EBD0 8001E3D0 05006214 */  bne        $v1, $v0, .L8001E3E8
    /* EBD4 8001E3D4 00000000 */   nop
    /* EBD8 8001E3D8 0A80043C */  lui        $a0, %hi(D_800A3904)
    /* EBDC 8001E3DC 04398494 */  lhu        $a0, %lo(D_800A3904)($a0)
    /* EBE0 8001E3E0 6B5C000C */  jal        rng_SetSeed
    /* EBE4 8001E3E4 00000000 */   nop
  .L8001E3E8:
    /* EBE8 8001E3E8 2400BF8F */  lw         $ra, 0x24($sp)
    /* EBEC 8001E3EC 2000B28F */  lw         $s2, 0x20($sp)
    /* EBF0 8001E3F0 1C00B18F */  lw         $s1, 0x1C($sp)
    /* EBF4 8001E3F4 1800B08F */  lw         $s0, 0x18($sp)
    /* EBF8 8001E3F8 2800BD27 */  addiu      $sp, $sp, 0x28
    /* EBFC 8001E3FC 0800E003 */  jr         $ra
    /* EC00 8001E400 00000000 */   nop
endlabel mario_test_Exec
