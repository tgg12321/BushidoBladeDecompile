glabel func_8003D888
    /* 2E088 8003D888 21308000 */  addu       $a2, $a0, $zero
    /* 2E08C 8003D88C 2138A000 */  addu       $a3, $a1, $zero
    /* 2E090 8003D890 0800C58C */  lw         $a1, 0x8($a2)
    /* 2E094 8003D894 00000000 */  nop
    /* 2E098 8003D898 2A10A700 */  slt        $v0, $a1, $a3
    /* 2E09C 8003D89C 15004010 */  beqz       $v0, .L8003D8F4
    /* 2E0A0 8003D8A0 01000424 */   addiu     $a0, $zero, 0x1
    /* 2E0A4 8003D8A4 2338E500 */  subu       $a3, $a3, $a1
    /* 2E0A8 8003D8A8 0410A400 */  sllv       $v0, $a0, $a1
    /* 2E0AC 8003D8AC FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 2E0B0 8003D8B0 0420E400 */  sllv       $a0, $a0, $a3
    /* 2E0B4 8003D8B4 FFFF8424 */  addiu      $a0, $a0, -0x1
    /* 2E0B8 8003D8B8 0400C38C */  lw         $v1, 0x4($a2)
    /* 2E0BC 8003D8BC 0000C58C */  lw         $a1, 0x0($a2)
    /* 2E0C0 8003D8C0 24186200 */  and        $v1, $v1, $v0
    /* 2E0C4 8003D8C4 0400A224 */  addiu      $v0, $a1, 0x4
    /* 2E0C8 8003D8C8 0000C2AC */  sw         $v0, 0x0($a2)
    /* 2E0CC 8003D8CC 20000224 */  addiu      $v0, $zero, 0x20
    /* 2E0D0 8003D8D0 23104700 */  subu       $v0, $v0, $a3
    /* 2E0D4 8003D8D4 0000A58C */  lw         $a1, 0x0($a1)
    /* 2E0D8 8003D8D8 0418E300 */  sllv       $v1, $v1, $a3
    /* 2E0DC 8003D8DC 0800C2AC */  sw         $v0, 0x8($a2)
    /* 2E0E0 8003D8E0 06104500 */  srlv       $v0, $a1, $v0
    /* 2E0E4 8003D8E4 24104400 */  and        $v0, $v0, $a0
    /* 2E0E8 8003D8E8 25186200 */  or         $v1, $v1, $v0
    /* 2E0EC 8003D8EC 45F60008 */  j          .L8003D914
    /* 2E0F0 8003D8F0 0400C5AC */   sw        $a1, 0x4($a2)
  .L8003D8F4:
    /* 2E0F4 8003D8F4 0400C38C */  lw         $v1, 0x4($a2)
    /* 2E0F8 8003D8F8 2310A700 */  subu       $v0, $a1, $a3
    /* 2E0FC 8003D8FC 0800C2AC */  sw         $v0, 0x8($a2)
    /* 2E100 8003D900 06184300 */  srlv       $v1, $v1, $v0
    /* 2E104 8003D904 01000224 */  addiu      $v0, $zero, 0x1
    /* 2E108 8003D908 0410E200 */  sllv       $v0, $v0, $a3
    /* 2E10C 8003D90C FFFF4224 */  addiu      $v0, $v0, -0x1
    /* 2E110 8003D910 24186200 */  and        $v1, $v1, $v0
  .L8003D914:
    /* 2E114 8003D914 0800E003 */  jr         $ra
    /* 2E118 8003D918 21106000 */   addu      $v0, $v1, $zero
endlabel func_8003D888
