glabel func_800451D0
    /* 359D0 800451D0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 359D4 800451D4 1000BFAF */  sw         $ra, 0x10($sp)
    /* 359D8 800451D8 FFFF0324 */  addiu      $v1, $zero, -0x1
    /* 359DC 800451DC 90000224 */  addiu      $v0, $zero, 0x90
  .L800451E0:
    /* 359E0 800451E0 0F80013C */  lui        $at, %hi(D_800EED10)
    /* 359E4 800451E4 21082200 */  addu       $at, $at, $v0
    /* 359E8 800451E8 10ED23A4 */  sh         $v1, %lo(D_800EED10)($at)
    /* 359EC 800451EC F0FF4224 */  addiu      $v0, $v0, -0x10
    /* 359F0 800451F0 FBFF4104 */  bgez       $v0, .L800451E0
    /* 359F4 800451F4 00000000 */   nop
    /* 359F8 800451F8 0400033C */  lui        $v1, (0x45000 >> 16)
    /* 359FC 800451FC 00506334 */  ori        $v1, $v1, (0x45000 & 0xFFFF)
    /* 35A00 80045200 0B80023C */  lui        $v0, %hi(D_800A9D10)
    /* 35A04 80045204 109D4224 */  addiu      $v0, $v0, %lo(D_800A9D10)
    /* 35A08 80045208 D40282AF */  sw         $v0, %gp_rel(D_800A33A0)($gp)
    /* 35A0C 8004520C D80283AF */  sw         $v1, %gp_rel(D_800A33A4)($gp)
    /* 35A10 80045210 E00280AF */  sw         $zero, %gp_rel(D_800A33AC)($gp)
    /* 35A14 80045214 DC0280AF */  sw         $zero, %gp_rel(D_800A33A8)($gp)
    /* 35A18 80045218 8727010C */  jal        func_80049E1C
    /* 35A1C 8004521C 00000000 */   nop
    /* 35A20 80045220 1000BF8F */  lw         $ra, 0x10($sp)
    /* 35A24 80045224 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 35A28 80045228 0800E003 */  jr         $ra
    /* 35A2C 8004522C 00000000 */   nop
endlabel func_800451D0
