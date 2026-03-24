glabel func_8007D3A4
    /* 6DBA4 8007D3A4 0010023C */  lui        $v0, (0x10000000 >> 16)
    /* 6DBA8 8007D3A8 0A80033C */  lui        $v1, %hi(D_8009BF48)
    /* 6DBAC 8007D3AC 48BF638C */  lw         $v1, %lo(D_8009BF48)($v1)
    /* 6DBB0 8007D3B0 25208200 */  or         $a0, $a0, $v0
    /* 6DBB4 8007D3B4 000064AC */  sw         $a0, 0x0($v1)
    /* 6DBB8 8007D3B8 0A80023C */  lui        $v0, %hi(D_8009BF44)
    /* 6DBBC 8007D3BC 44BF428C */  lw         $v0, %lo(D_8009BF44)($v0)
    /* 6DBC0 8007D3C0 FF00033C */  lui        $v1, (0xFFFFFF >> 16)
    /* 6DBC4 8007D3C4 0000428C */  lw         $v0, 0x0($v0)
    /* 6DBC8 8007D3C8 FFFF6334 */  ori        $v1, $v1, (0xFFFFFF & 0xFFFF)
    /* 6DBCC 8007D3CC 0800E003 */  jr         $ra
    /* 6DBD0 8007D3D0 24104300 */   and       $v0, $v0, $v1
endlabel func_8007D3A4
