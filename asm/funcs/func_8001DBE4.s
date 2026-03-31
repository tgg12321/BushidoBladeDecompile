glabel func_8001DBE4
    /* E3E4 8001DBE4 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* E3E8 8001DBE8 0A80033C */  lui        $v1, %hi(D_800A3768)
    /* E3EC 8001DBEC 68376390 */  lbu        $v1, %lo(D_800A3768)($v1)
    /* E3F0 8001DBF0 14000224 */  addiu      $v0, $zero, 0x14
    /* E3F4 8001DBF4 1400BFAF */  sw         $ra, 0x14($sp)
    /* E3F8 8001DBF8 28006214 */  bne        $v1, $v0, .L8001DC9C
    /* E3FC 8001DBFC 1000B0AF */   sw        $s0, 0x10($sp)
    /* E400 8001DC00 9EEA000C */  jal        func_8003AA78
    /* E404 8001DC04 00000000 */   nop
    /* E408 8001DC08 0A80033C */  lui        $v1, %hi(D_800A38F8)
    /* E40C 8001DC0C F8386390 */  lbu        $v1, %lo(D_800A38F8)($v1)
    /* E410 8001DC10 0A80023C */  lui        $v0, %hi(D_800A37A0)
    /* E414 8001DC14 A0374290 */  lbu        $v0, %lo(D_800A37A0)($v0)
    /* E418 8001DC18 00000000 */  nop
    /* E41C 8001DC1C 2B104300 */  sltu       $v0, $v0, $v1
    /* E420 8001DC20 18004014 */  bnez       $v0, .L8001DC84
    /* E424 8001DC24 00000000 */   nop
  .L8001DC28:
    /* E428 8001DC28 92EA000C */  jal        func_8003AA48
    /* E42C 8001DC2C 00000000 */   nop
    /* E430 8001DC30 3D5D000C */  jal        gnd_disp_loop_ctrl
    /* E434 8001DC34 00000000 */   nop
    /* E438 8001DC38 330A020C */  jal        func_800828CC
    /* E43C 8001DC3C 02000424 */   addiu     $a0, $zero, 0x2
    /* E440 8001DC40 0A80033C */  lui        $v1, %hi(D_800A38F8)
    /* E444 8001DC44 F8386390 */  lbu        $v1, %lo(D_800A38F8)($v1)
    /* E448 8001DC48 0A80023C */  lui        $v0, %hi(D_800A37A0)
    /* E44C 8001DC4C A0374290 */  lbu        $v0, %lo(D_800A37A0)($v0)
    /* E450 8001DC50 00000000 */  nop
    /* E454 8001DC54 2B104300 */  sltu       $v0, $v0, $v1
    /* E458 8001DC58 F3FF4010 */  beqz       $v0, .L8001DC28
    /* E45C 8001DC5C 21800000 */   addu      $s0, $zero, $zero
  .L8001DC60:
    /* E460 8001DC60 92EA000C */  jal        func_8003AA48
    /* E464 8001DC64 01001026 */   addiu     $s0, $s0, 0x1
    /* E468 8001DC68 3D5D000C */  jal        gnd_disp_loop_ctrl
    /* E46C 8001DC6C 00000000 */   nop
    /* E470 8001DC70 330A020C */  jal        func_800828CC
    /* E474 8001DC74 02000424 */   addiu     $a0, $zero, 0x2
    /* E478 8001DC78 0F00022A */  slti       $v0, $s0, 0xF
    /* E47C 8001DC7C F8FF4014 */  bnez       $v0, .L8001DC60
    /* E480 8001DC80 00000000 */   nop
  .L8001DC84:
    /* E484 8001DC84 ACEA000C */  jal        func_8003AAB0
    /* E488 8001DC88 00000000 */   nop
    /* E48C 8001DC8C 225A000C */  jal        gpu_InitDisplay
    /* E490 8001DC90 00000000 */   nop
    /* E494 8001DC94 345A000C */  jal        gpu_DisableDisplay
    /* E498 8001DC98 00000000 */   nop
  .L8001DC9C:
    /* E49C 8001DC9C 1400BF8F */  lw         $ra, 0x14($sp)
    /* E4A0 8001DCA0 1000B08F */  lw         $s0, 0x10($sp)
    /* E4A4 8001DCA4 1800BD27 */  addiu      $sp, $sp, 0x18
    /* E4A8 8001DCA8 0800E003 */  jr         $ra
    /* E4AC 8001DCAC 00000000 */   nop
endlabel func_8001DBE4
