glabel func_8001EA84
    /* F284 8001EA84 0A80023C */  lui        $v0, %hi(D_800A37B8)
    /* F288 8001EA88 B837428C */  lw         $v0, %lo(D_800A37B8)($v0)
    /* F28C 8001EA8C C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* F290 8001EA90 3400BFAF */  sw         $ra, 0x34($sp)
    /* F294 8001EA94 3000B0AF */  sw         $s0, 0x30($sp)
    /* F298 8001EA98 01004224 */  addiu      $v0, $v0, 0x1
    /* F29C 8001EA9C 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* F2A0 8001EAA0 B83722AC */  sw         $v0, %lo(D_800A37B8)($at)
    /* F2A4 8001EAA4 AC1C010C */  jal        camera_GetBoneData
    /* F2A8 8001EAA8 00000000 */   nop
    /* F2AC 8001EAAC 0A80033C */  lui        $v1, %hi(D_800A3748)
    /* F2B0 8001EAB0 48376380 */  lb         $v1, %lo(D_800A3748)($v1)
    /* F2B4 8001EAB4 1080043C */  lui        $a0, %hi(D_80101EC8)
    /* F2B8 8001EAB8 C81E8424 */  addiu      $a0, $a0, %lo(D_80101EC8)
    /* F2BC 8001EABC 0A80013C */  lui        $at, %hi(D_800A3778)
    /* F2C0 8001EAC0 783722AC */  sw         $v0, %lo(D_800A3778)($at)
    /* F2C4 8001EAC4 02006014 */  bnez       $v1, .L8001EAD0
    /* F2C8 8001EAC8 00000000 */   nop
    /* F2CC 8001EACC 4C048424 */  addiu      $a0, $a0, 0x44C
  .L8001EAD0:
    /* F2D0 8001EAD0 0A80053C */  lui        $a1, %hi(D_800A37B8)
    /* F2D4 8001EAD4 B837A58C */  lw         $a1, %lo(D_800A37B8)($a1)
    /* F2D8 8001EAD8 1C6F000C */  jal        func_8001BC70
    /* F2DC 8001EADC C0280500 */   sll       $a1, $a1, 3
    /* F2E0 8001EAE0 0179000C */  jal        func_8001E404
    /* F2E4 8001EAE4 00000000 */   nop
    /* F2E8 8001EAE8 C8E4000C */  jal        func_80039320
    /* F2EC 8001EAEC 00000000 */   nop
    /* F2F0 8001EAF0 1B80000C */  jal        func_8002006C
    /* F2F4 8001EAF4 00000000 */   nop
    /* F2F8 8001EAF8 826F000C */  jal        func_8001BE08
    /* F2FC 8001EAFC 1000A427 */   addiu     $a0, $sp, 0x10
    /* F300 8001EB00 21200000 */  addu       $a0, $zero, $zero
    /* F304 8001EB04 C28F000C */  jal        func_80023F08
    /* F308 8001EB08 1000A527 */   addiu     $a1, $sp, 0x10
    /* F30C 8001EB0C 01000424 */  addiu      $a0, $zero, 0x1
    /* F310 8001EB10 C28F000C */  jal        func_80023F08
    /* F314 8001EB14 1000A527 */   addiu     $a1, $sp, 0x10
    /* F318 8001EB18 87B1000C */  jal        func_8002C61C
    /* F31C 8001EB1C 00000000 */   nop
    /* F320 8001EB20 5FC3000C */  jal        func_80030D7C
    /* F324 8001EB24 00000000 */   nop
    /* F328 8001EB28 7AC8000C */  jal        func_800321E8
    /* F32C 8001EB2C 00000000 */   nop
    /* F330 8001EB30 E8E5000C */  jal        func_800397A0
    /* F334 8001EB34 00000000 */   nop
    /* F338 8001EB38 6A1B010C */  jal        game_StageInit
    /* F33C 8001EB3C 01000424 */   addiu     $a0, $zero, 0x1
    /* F340 8001EB40 76CD000C */  jal        func_800335D8
    /* F344 8001EB44 00000000 */   nop
    /* F348 8001EB48 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* F34C 8001EB4C DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* F350 8001EB50 03000224 */  addiu      $v0, $zero, 0x3
    /* F354 8001EB54 14006214 */  bne        $v1, $v0, .L8001EBA8
    /* F358 8001EB58 2800B027 */   addiu     $s0, $sp, 0x28
    /* F35C 8001EB5C 5A73000C */  jal        func_8001CD68
    /* F360 8001EB60 21200002 */   addu      $a0, $s0, $zero
    /* F364 8001EB64 21200002 */  addu       $a0, $s0, $zero
    /* F368 8001EB68 0A80053C */  lui        $a1, %hi(D_800A38E2)
    /* F36C 8001EB6C E238A590 */  lbu        $a1, %lo(D_800A38E2)($a1)
    /* F370 8001EB70 0A80063C */  lui        $a2, %hi(D_800A38B4)
    /* F374 8001EB74 B438C68C */  lw         $a2, %lo(D_800A38B4)($a2)
    /* F378 8001EB78 0576010C */  jal        func_8005D814
    /* F37C 8001EB7C 01000724 */   addiu     $a3, $zero, 0x1
    /* F380 8001EB80 02004104 */  bgez       $v0, .L8001EB8C
    /* F384 8001EB84 00000000 */   nop
    /* F388 8001EB88 03004224 */  addiu      $v0, $v0, 0x3
  .L8001EB8C:
    /* F38C 8001EB8C 83100200 */  sra        $v0, $v0, 2
    /* F390 8001EB90 0A80033C */  lui        $v1, %hi(D_800A38B4)
    /* F394 8001EB94 B438638C */  lw         $v1, %lo(D_800A38B4)($v1)
    /* F398 8001EB98 80100200 */  sll        $v0, $v0, 2
    /* F39C 8001EB9C 21186200 */  addu       $v1, $v1, $v0
    /* F3A0 8001EBA0 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* F3A4 8001EBA4 B43823AC */  sw         $v1, %lo(D_800A38B4)($at)
  .L8001EBA8:
    /* F3A8 8001EBA8 0A80023C */  lui        $v0, %hi(D_800A3929)
    /* F3AC 8001EBAC 29394290 */  lbu        $v0, %lo(D_800A3929)($v0)
    /* F3B0 8001EBB0 00000000 */  nop
    /* F3B4 8001EBB4 63004014 */  bnez       $v0, .L8001ED44
    /* F3B8 8001EBB8 01000424 */   addiu     $a0, $zero, 0x1
    /* F3BC 8001EBBC 0A80053C */  lui        $a1, %hi(D_800A3817)
    /* F3C0 8001EBC0 1738A590 */  lbu        $a1, %lo(D_800A3817)($a1)
    /* F3C4 8001EBC4 0A80063C */  lui        $a2, %hi(D_800A38B4)
    /* F3C8 8001EBC8 B438C68C */  lw         $a2, %lo(D_800A38B4)($a2)
    /* F3CC 8001EBCC 2A72010C */  jal        func_8005C8A8
    /* F3D0 8001EBD0 21380000 */   addu      $a3, $zero, $zero
    /* F3D4 8001EBD4 02004104 */  bgez       $v0, .L8001EBE0
    /* F3D8 8001EBD8 0010043C */   lui       $a0, (0x10001000 >> 16)
    /* F3DC 8001EBDC 03004224 */  addiu      $v0, $v0, 0x3
  .L8001EBE0:
    /* F3E0 8001EBE0 00108434 */  ori        $a0, $a0, (0x10001000 & 0xFFFF)
    /* F3E4 8001EBE4 83180200 */  sra        $v1, $v0, 2
    /* F3E8 8001EBE8 80180300 */  sll        $v1, $v1, 2
    /* F3EC 8001EBEC 0A80023C */  lui        $v0, %hi(D_800A38B4)
    /* F3F0 8001EBF0 B438428C */  lw         $v0, %lo(D_800A38B4)($v0)
    /* F3F4 8001EBF4 1080053C */  lui        $a1, %hi(D_80102794)
    /* F3F8 8001EBF8 9427A58C */  lw         $a1, %lo(D_80102794)($a1)
    /* F3FC 8001EBFC 21104300 */  addu       $v0, $v0, $v1
    /* F400 8001EC00 2420A400 */  and        $a0, $a1, $a0
    /* F404 8001EC04 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* F408 8001EC08 B43822AC */  sw         $v0, %lo(D_800A38B4)($at)
    /* F40C 8001EC0C 0D008010 */  beqz       $a0, .L8001EC44
    /* F410 8001EC10 21200000 */   addu      $a0, $zero, $zero
    /* F414 8001EC14 7F000524 */  addiu      $a1, $zero, 0x7F
    /* F418 8001EC18 9471010C */  jal        func_8005C650
    /* F41C 8001EC1C 7F000624 */   addiu     $a2, $zero, 0x7F
    /* F420 8001EC20 0A80033C */  lui        $v1, %hi(D_800A3817)
    /* F424 8001EC24 17386390 */  lbu        $v1, %lo(D_800A3817)($v1)
    /* F428 8001EC28 0A80023C */  lui        $v0, %hi(D_800A3804)
    /* F42C 8001EC2C 04384290 */  lbu        $v0, %lo(D_800A3804)($v0)
    /* F430 8001EC30 00000000 */  nop
    /* F434 8001EC34 12006214 */  bne        $v1, $v0, .L8001EC80
    /* F438 8001EC38 FFFF6224 */   addiu     $v0, $v1, -0x1
    /* F43C 8001EC3C 207B0008 */  j          .L8001EC80
    /* F440 8001EC40 02000224 */   addiu     $v0, $zero, 0x2
  .L8001EC44:
    /* F444 8001EC44 0040023C */  lui        $v0, (0x40004000 >> 16)
    /* F448 8001EC48 00404234 */  ori        $v0, $v0, (0x40004000 & 0xFFFF)
    /* F44C 8001EC4C 2410A200 */  and        $v0, $a1, $v0
    /* F450 8001EC50 0D004010 */  beqz       $v0, .L8001EC88
    /* F454 8001EC54 7F000524 */   addiu     $a1, $zero, 0x7F
    /* F458 8001EC58 9471010C */  jal        func_8005C650
    /* F45C 8001EC5C 7F000624 */   addiu     $a2, $zero, 0x7F
    /* F460 8001EC60 0A80043C */  lui        $a0, %hi(D_800A3817)
    /* F464 8001EC64 17388490 */  lbu        $a0, %lo(D_800A3817)($a0)
    /* F468 8001EC68 02000224 */  addiu      $v0, $zero, 0x2
    /* F46C 8001EC6C FF008330 */  andi       $v1, $a0, 0xFF
    /* F470 8001EC70 03006214 */  bne        $v1, $v0, .L8001EC80
    /* F474 8001EC74 01008224 */   addiu     $v0, $a0, 0x1
    /* F478 8001EC78 0A80023C */  lui        $v0, %hi(D_800A3804)
    /* F47C 8001EC7C 04384290 */  lbu        $v0, %lo(D_800A3804)($v0)
  .L8001EC80:
    /* F480 8001EC80 0A80013C */  lui        $at, %hi(D_800A3817)
    /* F484 8001EC84 173822A0 */  sb         $v0, %lo(D_800A3817)($at)
  .L8001EC88:
    /* F488 8001EC88 4000033C */  lui        $v1, (0x400040 >> 16)
    /* F48C 8001EC8C 1080023C */  lui        $v0, %hi(D_80102794)
    /* F490 8001EC90 9427428C */  lw         $v0, %lo(D_80102794)($v0)
    /* F494 8001EC94 40006334 */  ori        $v1, $v1, (0x400040 & 0xFFFF)
    /* F498 8001EC98 24104300 */  and        $v0, $v0, $v1
    /* F49C 8001EC9C 80004010 */  beqz       $v0, .L8001EEA0
    /* F4A0 8001ECA0 01000424 */   addiu     $a0, $zero, 0x1
    /* F4A4 8001ECA4 7F000524 */  addiu      $a1, $zero, 0x7F
    /* F4A8 8001ECA8 9471010C */  jal        func_8005C650
    /* F4AC 8001ECAC 7F000624 */   addiu     $a2, $zero, 0x7F
    /* F4B0 8001ECB0 0A80033C */  lui        $v1, %hi(D_800A3817)
    /* F4B4 8001ECB4 17386390 */  lbu        $v1, %lo(D_800A3817)($v1)
    /* F4B8 8001ECB8 00000000 */  nop
    /* F4BC 8001ECBC 02006014 */  bnez       $v1, .L8001ECC8
    /* F4C0 8001ECC0 3C000224 */   addiu     $v0, $zero, 0x3C
    /* F4C4 8001ECC4 01000224 */  addiu      $v0, $zero, 0x1
  .L8001ECC8:
    /* F4C8 8001ECC8 0A80013C */  lui        $at, %hi(D_800A3929)
    /* F4CC 8001ECCC 293922A0 */  sb         $v0, %lo(D_800A3929)($at)
    /* F4D0 8001ECD0 73006014 */  bnez       $v1, .L8001EEA0
    /* F4D4 8001ECD4 00000000 */   nop
    /* F4D8 8001ECD8 0A80023C */  lui        $v0, %hi(D_800A3748)
    /* F4DC 8001ECDC 48374280 */  lb         $v0, %lo(D_800A3748)($v0)
    /* F4E0 8001ECE0 00000000 */  nop
    /* F4E4 8001ECE4 0100422C */  sltiu      $v0, $v0, 0x1
    /* F4E8 8001ECE8 23100200 */  negu       $v0, $v0
    /* F4EC 8001ECEC 4C044230 */  andi       $v0, $v0, 0x44C
    /* F4F0 8001ECF0 1080013C */  lui        $at, %hi(D_80101F7B)
    /* F4F4 8001ECF4 21082200 */  addu       $at, $at, $v0
    /* F4F8 8001ECF8 7B1F20A0 */  sb         $zero, %lo(D_80101F7B)($at)
    /* F4FC 8001ECFC 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* F500 8001ED00 DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* F504 8001ED04 03000224 */  addiu      $v0, $zero, 0x3
    /* F508 8001ED08 65006214 */  bne        $v1, $v0, .L8001EEA0
    /* F50C 8001ED0C 0200033C */   lui       $v1, (0x2BF20 >> 16)
    /* F510 8001ED10 0A80023C */  lui        $v0, %hi(D_800A3858)
    /* F514 8001ED14 5838428C */  lw         $v0, %lo(D_800A3858)($v0)
    /* F518 8001ED18 20BF6334 */  ori        $v1, $v1, (0x2BF20 & 0xFFFF)
    /* F51C 8001ED1C 84034224 */  addiu      $v0, $v0, 0x384
    /* F520 8001ED20 0A80013C */  lui        $at, %hi(D_800A3858)
    /* F524 8001ED24 583822AC */  sw         $v0, %lo(D_800A3858)($at)
    /* F528 8001ED28 2A106200 */  slt        $v0, $v1, $v0
    /* F52C 8001ED2C 5C004010 */  beqz       $v0, .L8001EEA0
    /* F530 8001ED30 00000000 */   nop
    /* F534 8001ED34 0A80013C */  lui        $at, %hi(D_800A3858)
    /* F538 8001ED38 583823AC */  sw         $v1, %lo(D_800A3858)($at)
    /* F53C 8001ED3C A87B0008 */  j          .L8001EEA0
    /* F540 8001ED40 00000000 */   nop
  .L8001ED44:
    /* F544 8001ED44 0A80023C */  lui        $v0, %hi(D_800A3817)
    /* F548 8001ED48 17384290 */  lbu        $v0, %lo(D_800A3817)($v0)
    /* F54C 8001ED4C 00000000 */  nop
    /* F550 8001ED50 0F004014 */  bnez       $v0, .L8001ED90
    /* F554 8001ED54 21200000 */   addu      $a0, $zero, $zero
    /* F558 8001ED58 0A80053C */  lui        $a1, %hi(D_800A38B4)
    /* F55C 8001ED5C B438A58C */  lw         $a1, %lo(D_800A38B4)($a1)
    /* F560 8001ED60 A67E010C */  jal        func_8005FA98
    /* F564 8001ED64 01000624 */   addiu     $a2, $zero, 0x1
    /* F568 8001ED68 02004104 */  bgez       $v0, .L8001ED74
    /* F56C 8001ED6C 00000000 */   nop
    /* F570 8001ED70 03004224 */  addiu      $v0, $v0, 0x3
  .L8001ED74:
    /* F574 8001ED74 83100200 */  sra        $v0, $v0, 2
    /* F578 8001ED78 0A80033C */  lui        $v1, %hi(D_800A38B4)
    /* F57C 8001ED7C B438638C */  lw         $v1, %lo(D_800A38B4)($v1)
    /* F580 8001ED80 80100200 */  sll        $v0, $v0, 2
    /* F584 8001ED84 21186200 */  addu       $v1, $v1, $v0
    /* F588 8001ED88 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* F58C 8001ED8C B43823AC */  sw         $v1, %lo(D_800A38B4)($at)
  .L8001ED90:
    /* F590 8001ED90 0A80023C */  lui        $v0, %hi(D_800A3929)
    /* F594 8001ED94 29394290 */  lbu        $v0, %lo(D_800A3929)($v0)
    /* F598 8001ED98 00000000 */  nop
    /* F59C 8001ED9C 01004224 */  addiu      $v0, $v0, 0x1
    /* F5A0 8001EDA0 0A80013C */  lui        $at, %hi(D_800A3929)
    /* F5A4 8001EDA4 293922A0 */  sb         $v0, %lo(D_800A3929)($at)
    /* F5A8 8001EDA8 FF004230 */  andi       $v0, $v0, 0xFF
    /* F5AC 8001EDAC 3C00422C */  sltiu      $v0, $v0, 0x3C
    /* F5B0 8001EDB0 3B004014 */  bnez       $v0, .L8001EEA0
    /* F5B4 8001EDB4 00000000 */   nop
    /* F5B8 8001EDB8 0A80033C */  lui        $v1, %hi(D_800A3817)
    /* F5BC 8001EDBC 17386390 */  lbu        $v1, %lo(D_800A3817)($v1)
    /* F5C0 8001EDC0 00000000 */  nop
    /* F5C4 8001EDC4 22006014 */  bnez       $v1, .L8001EE50
    /* F5C8 8001EDC8 01000224 */   addiu     $v0, $zero, 0x1
    /* F5CC 8001EDCC 0A80033C */  lui        $v1, %hi(D_800A3748)
    /* F5D0 8001EDD0 48376380 */  lb         $v1, %lo(D_800A3748)($v1)
    /* F5D4 8001EDD4 01000224 */  addiu      $v0, $zero, 0x1
    /* F5D8 8001EDD8 0A80013C */  lui        $at, %hi(D_800A3670)
    /* F5DC 8001EDDC 703622A0 */  sb         $v0, %lo(D_800A3670)($at)
    /* F5E0 8001EDE0 0A80023C */  lui        $v0, %hi(D_800A380C)
    /* F5E4 8001EDE4 0C38428C */  lw         $v0, %lo(D_800A380C)($v0)
    /* F5E8 8001EDE8 00210300 */  sll        $a0, $v1, 4
    /* F5EC 8001EDEC 21208300 */  addu       $a0, $a0, $v1
    /* F5F0 8001EDF0 80200400 */  sll        $a0, $a0, 2
    /* F5F4 8001EDF4 21208300 */  addu       $a0, $a0, $v1
    /* F5F8 8001EDF8 80200400 */  sll        $a0, $a0, 2
    /* F5FC 8001EDFC 23208300 */  subu       $a0, $a0, $v1
    /* F600 8001EE00 80200400 */  sll        $a0, $a0, 2
    /* F604 8001EE04 1080033C */  lui        $v1, %hi(D_80101FBC)
    /* F608 8001EE08 BC1F6324 */  addiu      $v1, $v1, %lo(D_80101FBC)
    /* F60C 8001EE0C 01004224 */  addiu      $v0, $v0, 0x1
    /* F610 8001EE10 0A80013C */  lui        $at, %hi(D_800A380C)
    /* F614 8001EE14 0C3822AC */  sw         $v0, %lo(D_800A380C)($at)
    /* F618 8001EE18 0289000C */  jal        func_80022408
    /* F61C 8001EE1C 21208300 */   addu      $a0, $a0, $v1
    /* F620 8001EE20 1080033C */  lui        $v1, %hi(D_8010231A)
    /* F624 8001EE24 1A236384 */  lh         $v1, %lo(D_8010231A)($v1)
    /* F628 8001EE28 0A80013C */  lui        $at, %hi(D_800A38DF)
    /* F62C 8001EE2C DF3822A0 */  sb         $v0, %lo(D_800A38DF)($at)
    /* F630 8001EE30 03006010 */  beqz       $v1, .L8001EE40
    /* F634 8001EE34 00000000 */   nop
    /* F638 8001EE38 3A54010C */  jal        func_800550E8
    /* F63C 8001EE3C 01000424 */   addiu     $a0, $zero, 0x1
  .L8001EE40:
    /* F640 8001EE40 0A80013C */  lui        $at, %hi(D_800A3834)
    /* F644 8001EE44 343820A4 */  sh         $zero, %lo(D_800A3834)($at)
    /* F648 8001EE48 A87B0008 */  j          .L8001EEA0
    /* F64C 8001EE4C 00000000 */   nop
  .L8001EE50:
    /* F650 8001EE50 0A006214 */  bne        $v1, $v0, .L8001EE7C
    /* F654 8001EE54 02000224 */   addiu     $v0, $zero, 0x2
    /* F658 8001EE58 B0DC000C */  jal        func_800372C0
    /* F65C 8001EE5C 00000000 */   nop
    /* F660 8001EE60 8B76000C */  jal        func_8001DA2C
    /* F664 8001EE64 00000000 */   nop
    /* F668 8001EE68 01000224 */  addiu      $v0, $zero, 0x1
    /* F66C 8001EE6C 0A80013C */  lui        $at, %hi(D_800A31DA)
    /* F670 8001EE70 DA3122A0 */  sb         $v0, %lo(D_800A31DA)($at)
    /* F674 8001EE74 A67B0008 */  j          .L8001EE98
    /* F678 8001EE78 08000224 */   addiu     $v0, $zero, 0x8
  .L8001EE7C:
    /* F67C 8001EE7C 08006214 */  bne        $v1, $v0, .L8001EEA0
    /* F680 8001EE80 00000000 */   nop
    /* F684 8001EE84 B0DC000C */  jal        func_800372C0
    /* F688 8001EE88 00000000 */   nop
    /* F68C 8001EE8C 8B76000C */  jal        func_8001DA2C
    /* F690 8001EE90 00000000 */   nop
    /* F694 8001EE94 08000224 */  addiu      $v0, $zero, 0x8
  .L8001EE98:
    /* F698 8001EE98 0A80013C */  lui        $at, %hi(D_800A3834)
    /* F69C 8001EE9C 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
  .L8001EEA0:
    /* F6A0 8001EEA0 3400BF8F */  lw         $ra, 0x34($sp)
    /* F6A4 8001EEA4 3000B08F */  lw         $s0, 0x30($sp)
    /* F6A8 8001EEA8 3800BD27 */  addiu      $sp, $sp, 0x38
    /* F6AC 8001EEAC 0800E003 */  jr         $ra
    /* F6B0 8001EEB0 00000000 */   nop
endlabel func_8001EA84
