glabel func_80037AA4
    /* 282A4 80037AA4 F8FFBD27 */  addiu      $sp, $sp, -0x8
    /* 282A8 80037AA8 21280000 */  addu       $a1, $zero, $zero
    /* 282AC 80037AAC FC07868F */  lw         $a2, %gp_rel(D_800A38C8)($gp)
    /* 282B0 80037AB0 00000000 */  nop
    /* 282B4 80037AB4 0900C018 */  blez       $a2, .L80037ADC
    /* 282B8 80037AB8 21200000 */   addu      $a0, $zero, $zero
    /* 282BC 80037ABC 1080033C */  lui        $v1, %hi(D_80102810)
    /* 282C0 80037AC0 10286324 */  addiu      $v1, $v1, %lo(D_80102810)
  .L80037AC4:
    /* 282C4 80037AC4 1800628C */  lw         $v0, 0x18($v1)
    /* 282C8 80037AC8 0100A524 */  addiu      $a1, $a1, 0x1
    /* 282CC 80037ACC 21208200 */  addu       $a0, $a0, $v0
    /* 282D0 80037AD0 2A10A600 */  slt        $v0, $a1, $a2
    /* 282D4 80037AD4 FBFF4014 */  bnez       $v0, .L80037AC4
    /* 282D8 80037AD8 28006324 */   addiu     $v1, $v1, 0x28
  .L80037ADC:
    /* 282DC 80037ADC 02008104 */  bgez       $a0, .L80037AE8
    /* 282E0 80037AE0 21108000 */   addu      $v0, $a0, $zero
    /* 282E4 80037AE4 FF1F8224 */  addiu      $v0, $a0, 0x1FFF
  .L80037AE8:
    /* 282E8 80037AE8 43230200 */  sra        $a0, $v0, 13
    /* 282EC 80037AEC 0F000224 */  addiu      $v0, $zero, 0xF
    /* 282F0 80037AF0 23104400 */  subu       $v0, $v0, $a0
    /* 282F4 80037AF4 0800BD27 */  addiu      $sp, $sp, 0x8
    /* 282F8 80037AF8 0800E003 */  jr         $ra
    /* 282FC 80037AFC 00000000 */   nop
endlabel func_80037AA4
