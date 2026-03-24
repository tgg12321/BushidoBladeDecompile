glabel func_800392C8
    /* 29AC8 800392C8 FF000424 */  addiu      $a0, $zero, 0xFF
    /* 29ACC 800392CC F0010324 */  addiu      $v1, $zero, 0x1F0
    /* 29AD0 800392D0 0F80023C */  lui        $v0, %hi(D_800F33D8)
    /* 29AD4 800392D4 D8334224 */  addiu      $v0, $v0, %lo(D_800F33D8)
    /* 29AD8 800392D8 200682AF */  sw         $v0, %gp_rel(D_800A36EC)($gp)
    /* 29ADC 800392DC 2C0680A3 */  sb         $zero, %gp_rel(D_800A36F8)($gp)
    /* 29AE0 800392E0 B60680A3 */  sb         $zero, %gp_rel(D_800A3782)($gp)
  .L800392E4:
    /* 29AE4 800392E4 1080013C */  lui        $at, %hi(D_80101BF0)
    /* 29AE8 800392E8 21082300 */  addu       $at, $at, $v1
    /* 29AEC 800392EC F01B24A0 */  sb         $a0, %lo(D_80101BF0)($at)
    /* 29AF0 800392F0 F0FF6324 */  addiu      $v1, $v1, -0x10
    /* 29AF4 800392F4 FBFF6104 */  bgez       $v1, .L800392E4
    /* 29AF8 800392F8 300B0224 */   addiu     $v0, $zero, 0xB30
    /* 29AFC 800392FC FFFF0324 */  addiu      $v1, $zero, -0x1
  .L80039300:
    /* 29B00 80039300 0F80013C */  lui        $at, %hi(D_800F68E0)
    /* 29B04 80039304 21082200 */  addu       $at, $at, $v0
    /* 29B08 80039308 E06823A4 */  sh         $v1, %lo(D_800F68E0)($at)
    /* 29B0C 8003930C F0FF4224 */  addiu      $v0, $v0, -0x10
    /* 29B10 80039310 FBFF4104 */  bgez       $v0, .L80039300
    /* 29B14 80039314 00000000 */   nop
    /* 29B18 80039318 0800E003 */  jr         $ra
    /* 29B1C 8003931C 00000000 */   nop
endlabel func_800392C8
