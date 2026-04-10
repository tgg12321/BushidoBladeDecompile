glabel func_8001DB9C
    /* E39C 8001DB9C E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* E3A0 8001DBA0 1080023C */  lui        $v0, %hi(D_80101ED2)
    /* E3A4 8001DBA4 D21E4284 */  lh         $v0, %lo(D_80101ED2)($v0)
    /* E3A8 8001DBA8 1980053C */  lui        $a1, (0x80190800 >> 16)
    /* E3AC 8001DBAC 1000BFAF */  sw         $ra, 0x10($sp)
    /* E3B0 8001DBB0 0980013C */  lui        $at, %hi(D_8008D9EC)
    /* E3B4 8001DBB4 21082200 */  addu       $at, $at, $v0
    /* E3B8 8001DBB8 ECD92490 */  lbu        $a0, %lo(D_8008D9EC)($at)
    /* E3BC 8001DBBC 0008A534 */  ori        $a1, $a1, (0x80190800 & 0xFFFF)
    /* E3C0 8001DBC0 2F14010C */  jal        seq_Start
    /* E3C4 8001DBC4 0100842C */   sltiu     $a0, $a0, 0x1
    /* E3C8 8001DBC8 FFFF0234 */  ori        $v0, $zero, 0xFFFF
    /* E3CC 8001DBCC 0A80013C */  lui        $at, %hi(D_800A38C6)
    /* E3D0 8001DBD0 C63822A4 */  sh         $v0, %lo(D_800A38C6)($at)
    /* E3D4 8001DBD4 1000BF8F */  lw         $ra, 0x10($sp)
    /* E3D8 8001DBD8 1800BD27 */  addiu      $sp, $sp, 0x18
    /* E3DC 8001DBDC 0800E003 */  jr         $ra
    /* E3E0 8001DBE0 00000000 */   nop
endlabel func_8001DB9C
