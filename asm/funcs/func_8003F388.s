glabel func_8003F388
    /* 2FB88 8003F388 00008284 */  lh         $v0, 0x0($a0)
    /* 2FB8C 8003F38C 04008384 */  lh         $v1, 0x4($a0)
    /* 2FB90 8003F390 10004424 */  addiu      $a0, $v0, 0x10
    /* 2FB94 8003F394 2000822C */  sltiu      $v0, $a0, 0x20
    /* 2FB98 8003F398 0C004010 */  beqz       $v0, .L8003F3CC
    /* 2FB9C 8003F39C 10006324 */   addiu     $v1, $v1, 0x10
    /* 2FBA0 8003F3A0 2000622C */  sltiu      $v0, $v1, 0x20
    /* 2FBA4 8003F3A4 09004010 */  beqz       $v0, .L8003F3CC
    /* 2FBA8 8003F3A8 40190300 */   sll       $v1, $v1, 5
    /* 2FBAC 8003F3AC 0B80023C */  lui        $v0, %hi(D_800A8FB0)
    /* 2FBB0 8003F3B0 B08F4224 */  addiu      $v0, $v0, %lo(D_800A8FB0)
    /* 2FBB4 8003F3B4 21186400 */  addu       $v1, $v1, $a0
    /* 2FBB8 8003F3B8 21186200 */  addu       $v1, $v1, $v0
    /* 2FBBC 8003F3BC 00006290 */  lbu        $v0, 0x0($v1)
    /* 2FBC0 8003F3C0 00000000 */  nop
    /* 2FBC4 8003F3C4 04004234 */  ori        $v0, $v0, 0x4
    /* 2FBC8 8003F3C8 000062A0 */  sb         $v0, 0x0($v1)
  .L8003F3CC:
    /* 2FBCC 8003F3CC 0800E003 */  jr         $ra
    /* 2FBD0 8003F3D0 00000000 */   nop
endlabel func_8003F388
