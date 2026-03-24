glabel func_8001EA04
    /* F204 8001EA04 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* F208 8001EA08 21200000 */  addu       $a0, $zero, $zero
    /* F20C 8001EA0C 1000BFAF */  sw         $ra, 0x10($sp)
    /* F210 8001EA10 A205010C */  jal        func_80041688
    /* F214 8001EA14 21280000 */   addu      $a1, $zero, $zero
    /* F218 8001EA18 01000424 */  addiu      $a0, $zero, 0x1
    /* F21C 8001EA1C A205010C */  jal        func_80041688
    /* F220 8001EA20 21280000 */   addu      $a1, $zero, $zero
    /* F224 8001EA24 5E84010C */  jal        func_80061178
    /* F228 8001EA28 00000000 */   nop
    /* F22C 8001EA2C 0A80023C */  lui        $v0, %hi(D_800A38D4)
    /* F230 8001EA30 D4384290 */  lbu        $v0, %lo(D_800A38D4)($v0)
    /* F234 8001EA34 0D000324 */  addiu      $v1, $zero, 0xD
    /* F238 8001EA38 1080013C */  lui        $at, %hi(D_8010262E)
    /* F23C 8001EA3C 2E2620A4 */  sh         $zero, %lo(D_8010262E)($at)
    /* F240 8001EA40 1080013C */  lui        $at, %hi(D_801021E2)
    /* F244 8001EA44 E22120A4 */  sh         $zero, %lo(D_801021E2)($at)
    /* F248 8001EA48 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* F24C 8001EA4C B83720AC */  sw         $zero, %lo(D_800A37B8)($at)
    /* F250 8001EA50 0A80013C */  lui        $at, %hi(D_800A3929)
    /* F254 8001EA54 293920A0 */  sb         $zero, %lo(D_800A3929)($at)
    /* F258 8001EA58 0A80013C */  lui        $at, %hi(D_800A3834)
    /* F25C 8001EA5C 343823A4 */  sh         $v1, %lo(D_800A3834)($at)
    /* F260 8001EA60 0100422C */  sltiu      $v0, $v0, 0x1
    /* F264 8001EA64 0A80013C */  lui        $at, %hi(D_800A3804)
    /* F268 8001EA68 043822A0 */  sb         $v0, %lo(D_800A3804)($at)
    /* F26C 8001EA6C 0A80013C */  lui        $at, %hi(D_800A3817)
    /* F270 8001EA70 173822A0 */  sb         $v0, %lo(D_800A3817)($at)
    /* F274 8001EA74 1000BF8F */  lw         $ra, 0x10($sp)
    /* F278 8001EA78 1800BD27 */  addiu      $sp, $sp, 0x18
    /* F27C 8001EA7C 0800E003 */  jr         $ra
    /* F280 8001EA80 00000000 */   nop
endlabel func_8001EA04
