glabel func_8001DA2C
    /* E22C 8001DA2C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* E230 8001DA30 1000BFAF */  sw         $ra, 0x10($sp)
    /* E234 8001DA34 6B6D010C */  jal        obj_InitChars
    /* E238 8001DA38 00000000 */   nop
    /* E23C 8001DA3C CF6F010C */  jal        obj_Reset
    /* E240 8001DA40 00000000 */   nop
    /* E244 8001DA44 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* E248 8001DA48 DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* E24C 8001DA4C 05000224 */  addiu      $v0, $zero, 0x5
    /* E250 8001DA50 06006214 */  bne        $v1, $v0, .L8001DA6C
    /* E254 8001DA54 03000224 */   addiu     $v0, $zero, 0x3
    /* E258 8001DA58 716E010C */  jal        obj_InitTask
    /* E25C 8001DA5C 00000000 */   nop
    /* E260 8001DA60 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* E264 8001DA64 DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* E268 8001DA68 03000224 */  addiu      $v0, $zero, 0x3
  .L8001DA6C:
    /* E26C 8001DA6C 03006214 */  bne        $v1, $v0, .L8001DA7C
    /* E270 8001DA70 00000000 */   nop
    /* E274 8001DA74 1A6E010C */  jal        obj_InitPair
    /* E278 8001DA78 00000000 */   nop
  .L8001DA7C:
    /* E27C 8001DA7C 1000BF8F */  lw         $ra, 0x10($sp)
    /* E280 8001DA80 1800BD27 */  addiu      $sp, $sp, 0x18
    /* E284 8001DA84 0800E003 */  jr         $ra
    /* E288 8001DA88 00000000 */   nop
endlabel func_8001DA2C
