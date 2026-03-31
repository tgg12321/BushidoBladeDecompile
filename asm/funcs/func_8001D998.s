glabel func_8001D998
    /* E198 8001D998 E0FFBD27 */  addiu      $sp, $sp, -0x20
    /* E19C 8001D99C 1800B2AF */  sw         $s2, 0x18($sp)
    /* E1A0 8001D9A0 1980123C */  lui        $s2, (0x80190800 >> 16)
    /* E1A4 8001D9A4 00085236 */  ori        $s2, $s2, (0x80190800 & 0xFFFF)
    /* E1A8 8001D9A8 1C00BFAF */  sw         $ra, 0x1C($sp)
    /* E1AC 8001D9AC 1400B1AF */  sw         $s1, 0x14($sp)
    /* E1B0 8001D9B0 1A5A000C */  jal        gpu_EnableDisplay
    /* E1B4 8001D9B4 1000B0AF */   sw        $s0, 0x10($sp)
    /* E1B8 8001D9B8 4E83000C */  jal        func_80020D38
    /* E1BC 8001D9BC 00000000 */   nop
    /* E1C0 8001D9C0 1A6E010C */  jal        func_8005B868
    /* E1C4 8001D9C4 00000000 */   nop
    /* E1C8 8001D9C8 1980043C */  lui        $a0, (0x80190800 >> 16)
    /* E1CC 8001D9CC 2E6E010C */  jal        func_8005B8B8
    /* E1D0 8001D9D0 00088434 */   ori       $a0, $a0, (0x80190800 & 0xFFFF)
    /* E1D4 8001D9D4 21884000 */  addu       $s1, $v0, $zero
    /* E1D8 8001D9D8 191B222A */  slti       $v0, $s1, 0x1B19
    /* E1DC 8001D9DC 03004014 */  bnez       $v0, .L8001D9EC
    /* E1E0 8001D9E0 00000000 */   nop
    /* E1E4 8001D9E4 0F5B000C */  jal        sys_Panic
    /* E1E8 8001D9E8 00000000 */   nop
  .L8001D9EC:
    /* E1EC 8001D9EC 1080103C */  lui        $s0, %hi(D_80104F38)
    /* E1F0 8001D9F0 384F1026 */  addiu      $s0, $s0, %lo(D_80104F38)
    /* E1F4 8001D9F4 21200002 */  addu       $a0, $s0, $zero
    /* E1F8 8001D9F8 1980053C */  lui        $a1, (0x80190800 >> 16)
    /* E1FC 8001D9FC 0008A534 */  ori        $a1, $a1, (0x80190800 & 0xFFFF)
    /* E200 8001DA00 48E4010C */  jal        func_80079120
    /* E204 8001DA04 21302002 */   addu      $a2, $s1, $zero
    /* E208 8001DA08 636E010C */  jal        func_8005B98C
    /* E20C 8001DA0C 23201202 */   subu      $a0, $s0, $s2
    /* E210 8001DA10 1C00BF8F */  lw         $ra, 0x1C($sp)
    /* E214 8001DA14 1800B28F */  lw         $s2, 0x18($sp)
    /* E218 8001DA18 1400B18F */  lw         $s1, 0x14($sp)
    /* E21C 8001DA1C 1000B08F */  lw         $s0, 0x10($sp)
    /* E220 8001DA20 2000BD27 */  addiu      $sp, $sp, 0x20
    /* E224 8001DA24 0800E003 */  jr         $ra
    /* E228 8001DA28 00000000 */   nop
endlabel func_8001D998
