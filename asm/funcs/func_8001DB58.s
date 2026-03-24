glabel func_8001DB58
    /* E358 8001DB58 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* E35C 8001DB5C DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* E360 8001DB60 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* E364 8001DB64 05006228 */  slti       $v0, $v1, 0x5
    /* E368 8001DB68 04004010 */  beqz       $v0, .L8001DB7C
    /* E36C 8001DB6C 1000BFAF */   sw        $ra, 0x10($sp)
    /* E370 8001DB70 02006228 */  slti       $v0, $v1, 0x2
    /* E374 8001DB74 03004010 */  beqz       $v0, .L8001DB84
    /* E378 8001DB78 00000000 */   nop
  .L8001DB7C:
    /* E37C 8001DB7C E3760008 */  j          .L8001DB8C
    /* E380 8001DB80 01000224 */   addiu     $v0, $zero, 0x1
  .L8001DB84:
    /* E384 8001DB84 F559000C */  jal        func_800167D4
    /* E388 8001DB88 00000000 */   nop
  .L8001DB8C:
    /* E38C 8001DB8C 1000BF8F */  lw         $ra, 0x10($sp)
    /* E390 8001DB90 1800BD27 */  addiu      $sp, $sp, 0x18
    /* E394 8001DB94 0800E003 */  jr         $ra
    /* E398 8001DB98 00000000 */   nop
endlabel func_8001DB58
