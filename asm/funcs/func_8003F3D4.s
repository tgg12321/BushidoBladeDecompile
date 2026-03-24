glabel func_8003F3D4
    /* 2FBD4 8003F3D4 00008284 */  lh         $v0, 0x0($a0)
    /* 2FBD8 8003F3D8 04008384 */  lh         $v1, 0x4($a0)
    /* 2FBDC 8003F3DC 10004424 */  addiu      $a0, $v0, 0x10
    /* 2FBE0 8003F3E0 2000822C */  sltiu      $v0, $a0, 0x20
    /* 2FBE4 8003F3E4 0C004010 */  beqz       $v0, .L8003F418
    /* 2FBE8 8003F3E8 10006324 */   addiu     $v1, $v1, 0x10
    /* 2FBEC 8003F3EC 2000622C */  sltiu      $v0, $v1, 0x20
    /* 2FBF0 8003F3F0 09004010 */  beqz       $v0, .L8003F418
    /* 2FBF4 8003F3F4 40190300 */   sll       $v1, $v1, 5
    /* 2FBF8 8003F3F8 0B80023C */  lui        $v0, %hi(D_800A8FB0)
    /* 2FBFC 8003F3FC B08F4224 */  addiu      $v0, $v0, %lo(D_800A8FB0)
    /* 2FC00 8003F400 21186400 */  addu       $v1, $v1, $a0
    /* 2FC04 8003F404 21186200 */  addu       $v1, $v1, $v0
    /* 2FC08 8003F408 00006290 */  lbu        $v0, 0x0($v1)
    /* 2FC0C 8003F40C 00000000 */  nop
    /* 2FC10 8003F410 08004234 */  ori        $v0, $v0, 0x8
    /* 2FC14 8003F414 000062A0 */  sb         $v0, 0x0($v1)
  .L8003F418:
    /* 2FC18 8003F418 0800E003 */  jr         $ra
    /* 2FC1C 8003F41C 00000000 */   nop
endlabel func_8003F3D4
