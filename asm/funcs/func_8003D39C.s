glabel func_8003D39C
    /* 2DB9C 8003D39C 21408000 */  addu       $t0, $a0, $zero
    /* 2DBA0 8003D3A0 2148A000 */  addu       $t1, $a1, $zero
    /* 2DBA4 8003D3A4 8C02838F */  lw         $v1, %gp_rel(D_800A3358)($gp)
    /* 2DBA8 8003D3A8 20000224 */  addiu      $v0, $zero, 0x20
    /* 2DBAC 8003D3AC 30006210 */  beq        $v1, $v0, .L8003D470
    /* 2DBB0 8003D3B0 2150E000 */   addu      $t2, $a3, $zero
    /* 2DBB4 8003D3B4 FF00073C */  lui        $a3, (0xFFFFFF >> 16)
    /* 2DBB8 8003D3B8 FFFFE734 */  ori        $a3, $a3, (0xFFFFFF & 0xFFFF)
    /* 2DBBC 8003D3BC 01006224 */  addiu      $v0, $v1, 0x1
    /* 2DBC0 8003D3C0 8C0282AF */  sw         $v0, %gp_rel(D_800A3358)($gp)
    /* 2DBC4 8003D3C4 00110300 */  sll        $v0, $v1, 4
    /* 2DBC8 8003D3C8 0A80033C */  lui        $v1, %hi(D_800A3930)
    /* 2DBCC 8003D3CC 30396324 */  addiu      $v1, $v1, %lo(D_800A3930)
    /* 2DBD0 8003D3D0 21104300 */  addu       $v0, $v0, $v1
    /* 2DBD4 8003D3D4 4C01848F */  lw         $a0, %gp_rel(D_800A3218)($gp)
    /* 2DBD8 8003D3D8 0074033C */  lui        $v1, (0x74000000 >> 16)
    /* 2DBDC 8003D3DC 40220400 */  sll        $a0, $a0, 9
    /* 2DBE0 8003D3E0 21208200 */  addu       $a0, $a0, $v0
    /* 2DBE4 8003D3E4 03000224 */  addiu      $v0, $zero, 0x3
    /* 2DBE8 8003D3E8 030082A0 */  sb         $v0, 0x3($a0)
    /* 2DBEC 8003D3EC 74000224 */  addiu      $v0, $zero, 0x74
    /* 2DBF0 8003D3F0 070082A0 */  sb         $v0, 0x7($a0)
    /* 2DBF4 8003D3F4 0700C230 */  andi       $v0, $a2, 0x7
    /* 2DBF8 8003D3F8 C0100200 */  sll        $v0, $v0, 3
    /* 2DBFC 8003D3FC C0FF4224 */  addiu      $v0, $v0, -0x40
    /* 2DC00 8003D400 0C0082A0 */  sb         $v0, 0xC($a0)
    /* 2DC04 8003D404 43110600 */  sra        $v0, $a2, 5
    /* 2DC08 8003D408 C0100200 */  sll        $v0, $v0, 3
    /* 2DC0C 8003D40C E0FF4224 */  addiu      $v0, $v0, -0x20
    /* 2DC10 8003D410 0D0082A0 */  sb         $v0, 0xD($a0)
    /* 2DC14 8003D414 C0100600 */  sll        $v0, $a2, 3
    /* 2DC18 8003D418 C0004230 */  andi       $v0, $v0, 0xC0
    /* 2DC1C 8003D41C 3F774234 */  ori        $v0, $v0, 0x773F
    /* 2DC20 8003D420 0E0082A4 */  sh         $v0, 0xE($a0)
    /* 2DC24 8003D424 43100A00 */  sra        $v0, $t2, 1
    /* 2DC28 8003D428 0000858C */  lw         $a1, 0x0($a0)
    /* 2DC2C 8003D42C 25104300 */  or         $v0, $v0, $v1
    /* 2DC30 8003D430 040082AC */  sw         $v0, 0x4($a0)
    /* 2DC34 8003D434 0A80033C */  lui        $v1, %hi(D_800A374C)
    /* 2DC38 8003D438 4C37638C */  lw         $v1, %lo(D_800A374C)($v1)
    /* 2DC3C 8003D43C 00FF063C */  lui        $a2, (0xFF000000 >> 16)
    /* 2DC40 8003D440 080088A4 */  sh         $t0, 0x8($a0)
    /* 2DC44 8003D444 0A0089A4 */  sh         $t1, 0xA($a0)
    /* 2DC48 8003D448 0000628C */  lw         $v0, 0x0($v1)
    /* 2DC4C 8003D44C 2428A600 */  and        $a1, $a1, $a2
    /* 2DC50 8003D450 24104700 */  and        $v0, $v0, $a3
    /* 2DC54 8003D454 2528A200 */  or         $a1, $a1, $v0
    /* 2DC58 8003D458 000085AC */  sw         $a1, 0x0($a0)
    /* 2DC5C 8003D45C 0000628C */  lw         $v0, 0x0($v1)
    /* 2DC60 8003D460 24208700 */  and        $a0, $a0, $a3
    /* 2DC64 8003D464 24104600 */  and        $v0, $v0, $a2
    /* 2DC68 8003D468 25104400 */  or         $v0, $v0, $a0
    /* 2DC6C 8003D46C 000062AC */  sw         $v0, 0x0($v1)
  .L8003D470:
    /* 2DC70 8003D470 0800E003 */  jr         $ra
    /* 2DC74 8003D474 00000000 */   nop
endlabel func_8003D39C
