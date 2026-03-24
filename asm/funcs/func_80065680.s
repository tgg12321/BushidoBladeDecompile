glabel func_80065680
    /* 55E80 80065680 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 55E84 80065684 1000BFAF */  sw         $ra, 0x10($sp)
    /* 55E88 80065688 0096010C */  jal        func_80065800
    /* 55E8C 8006568C 0C000424 */   addiu     $a0, $zero, 0xC
    /* 55E90 80065690 0F80033C */  lui        $v1, %hi(D_800F0BC0)
    /* 55E94 80065694 C00B6324 */  addiu      $v1, $v1, %lo(D_800F0BC0)
    /* 55E98 80065698 00006294 */  lhu        $v0, 0x0($v1)
    /* 55E9C 8006569C 0E000424 */  addiu      $a0, $zero, 0xE
    /* 55EA0 800656A0 01004224 */  addiu      $v0, $v0, 0x1
    /* 55EA4 800656A4 0096010C */  jal        func_80065800
    /* 55EA8 800656A8 000062A4 */   sh        $v0, 0x0($v1)
    /* 55EAC 800656AC 0F80033C */  lui        $v1, %hi(D_800F0BC4)
    /* 55EB0 800656B0 C40B6394 */  lhu        $v1, %lo(D_800F0BC4)($v1)
    /* 55EB4 800656B4 00000000 */  nop
    /* 55EB8 800656B8 01006324 */  addiu      $v1, $v1, 0x1
    /* 55EBC 800656BC 0F80013C */  lui        $at, %hi(D_800F0BC4)
    /* 55EC0 800656C0 C40B23A4 */  sh         $v1, %lo(D_800F0BC4)($at)
    /* 55EC4 800656C4 001C0300 */  sll        $v1, $v1, 16
    /* 55EC8 800656C8 031C0300 */  sra        $v1, $v1, 16
    /* 55ECC 800656CC 0B006328 */  slti       $v1, $v1, 0xB
    /* 55ED0 800656D0 02006014 */  bnez       $v1, .L800656DC
    /* 55ED4 800656D4 FF004230 */   andi      $v0, $v0, 0xFF
    /* 55ED8 800656D8 21100000 */  addu       $v0, $zero, $zero
  .L800656DC:
    /* 55EDC 800656DC 1000BF8F */  lw         $ra, 0x10($sp)
    /* 55EE0 800656E0 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 55EE4 800656E4 0800E003 */  jr         $ra
    /* 55EE8 800656E8 00000000 */   nop
endlabel func_80065680
