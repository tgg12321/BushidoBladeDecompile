glabel func_8004D244
    /* 3DA44 8004D244 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 3DA48 8004D248 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* 3DA4C 8004D24C 2800BEAF */  sw         $fp, 0x28($sp)
    /* 3DA50 8004D250 2400B6AF */  sw         $s6, 0x24($sp)
    /* 3DA54 8004D254 2000B5AF */  sw         $s5, 0x20($sp)
    /* 3DA58 8004D258 1C00B4AF */  sw         $s4, 0x1C($sp)
    /* 3DA5C 8004D25C 1800B3AF */  sw         $s3, 0x18($sp)
    /* 3DA60 8004D260 1400B2AF */  sw         $s2, 0x14($sp)
    /* 3DA64 8004D264 1000B1AF */  sw         $s1, 0x10($sp)
    /* 3DA68 8004D268 62008010 */  beqz       $a0, .L8004D3F4
    /* 3DA6C 8004D26C FFFF9324 */   addiu     $s3, $a0, -0x1
    /* 3DA70 8004D270 801F113C */  lui        $s1, (0x1F800020 >> 16)
    /* 3DA74 8004D274 20003136 */  ori        $s1, $s1, (0x1F800020 & 0xFFFF)
    /* 3DA78 8004D278 801F123C */  lui        $s2, (0x1F8002B4 >> 16)
    /* 3DA7C 8004D27C B4025236 */  ori        $s2, $s2, (0x1F8002B4 & 0xFFFF)
    /* 3DA80 8004D280 801F143C */  lui        $s4, (0x1F80000C >> 16)
    /* 3DA84 8004D284 0C00948E */  lw         $s4, (0x1F80000C & 0xFFFF)($s4)
    /* 3DA88 8004D288 0007153C */  lui        $s5, (0x7000000 >> 16)
    /* 3DA8C 8004D28C FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3DA90 8004D290 FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3DA94 8004D294 0A80163C */  lui        $s6, %hi(D_800A38B4)
    /* 3DA98 8004D298 B438D68E */  lw         $s6, %lo(D_800A38B4)($s6)
    /* 3DA9C 8004D29C 801F033C */  lui        $v1, (0x1F800008 >> 16)
    /* 3DAA0 8004D2A0 0800638C */  lw         $v1, (0x1F800008 & 0xFFFF)($v1)
    /* 3DAA4 8004D2A4 000006CA */  lwc2       $6, 0x0($s0)
  .L8004D2A8:
    /* 3DAA8 8004D2A8 0C00048E */  lw         $a0, 0xC($s0)
    /* 3DAAC 8004D2AC 00000000 */  nop
    /* 3DAB0 8004D2B0 80280400 */  sll        $a1, $a0, 2
    /* 3DAB4 8004D2B4 FC03AB30 */  andi       $t3, $a1, 0x3FC
    /* 3DAB8 8004D2B8 21287101 */  addu       $a1, $t3, $s1
    /* 3DABC 8004D2BC 0000A88C */  lw         $t0, 0x0($a1)
    /* 3DAC0 8004D2C0 82290400 */  srl        $a1, $a0, 6
    /* 3DAC4 8004D2C4 FC03AC30 */  andi       $t4, $a1, 0x3FC
    /* 3DAC8 8004D2C8 21289101 */  addu       $a1, $t4, $s1
    /* 3DACC 8004D2CC 0000A98C */  lw         $t1, 0x0($a1)
    /* 3DAD0 8004D2D0 822B0400 */  srl        $a1, $a0, 14
    /* 3DAD4 8004D2D4 FC03AD30 */  andi       $t5, $a1, 0x3FC
    /* 3DAD8 8004D2D8 2128B101 */  addu       $a1, $t5, $s1
    /* 3DADC 8004D2DC 0000AA8C */  lw         $t2, 0x0($a1)
    /* 3DAE0 8004D2E0 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3DAE4 8004D2E4 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3DAE8 8004D2E8 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3DAEC 8004D2EC 24700901 */  and        $t6, $t0, $t1
    /* 3DAF0 8004D2F0 2470CA01 */  and        $t6, $t6, $t2
    /* 3DAF4 8004D2F4 0600404B */  nclip
    /* 3DAF8 8004D2F8 3B00C005 */  bltz       $t6, .L8004D3E8
    /* 3DAFC 8004D2FC 00C00248 */   mfc2      $v0, $24 /* handwritten instruction */
    /* 3DB00 8004D300 0080CE31 */  andi       $t6, $t6, 0x8000
    /* 3DB04 8004D304 38004104 */  bgez       $v0, .L8004D3E8
    /* 3DB08 8004D308 00000000 */   nop
    /* 3DB0C 8004D30C 3600C015 */  bnez       $t6, .L8004D3E8
    /* 3DB10 8004D310 42100B00 */   srl       $v0, $t3, 1
    /* 3DB14 8004D314 21105200 */  addu       $v0, $v0, $s2
    /* 3DB18 8004D318 00004484 */  lh         $a0, 0x0($v0)
    /* 3DB1C 8004D31C 42100C00 */  srl        $v0, $t4, 1
    /* 3DB20 8004D320 21105200 */  addu       $v0, $v0, $s2
    /* 3DB24 8004D324 00004584 */  lh         $a1, 0x0($v0)
    /* 3DB28 8004D328 42100D00 */  srl        $v0, $t5, 1
    /* 3DB2C 8004D32C 21105200 */  addu       $v0, $v0, $s2
    /* 3DB30 8004D330 00004684 */  lh         $a2, 0x0($v0)
    /* 3DB34 8004D334 00888448 */  mtc2       $a0, $17 /* handwritten instruction */
    /* 3DB38 8004D338 00908548 */  mtc2       $a1, $18 /* handwritten instruction */
    /* 3DB3C 8004D33C 00988648 */  mtc2       $a2, $19 /* handwritten instruction */
    /* 3DB40 8004D340 0800C8AE */  sw         $t0, 0x8($s6)
    /* 3DB44 8004D344 00000000 */  nop
    /* 3DB48 8004D348 2D00584B */  avsz3
    /* 3DB4C 8004D34C 1000C9AE */  sw         $t1, 0x10($s6)
    /* 3DB50 8004D350 00380248 */  mfc2       $v0, $7 /* handwritten instruction */
    /* 3DB54 8004D354 1800CAAE */  sw         $t2, 0x18($s6)
    /* 3DB58 8004D358 23004018 */  blez       $v0, .L8004D3E8
    /* 3DB5C 8004D35C 80100200 */   sll       $v0, $v0, 2
    /* 3DB60 8004D360 100000CA */  lwc2       $0, 0x10($s0)
    /* 3DB64 8004D364 140001CA */  lwc2       $1, 0x14($s0)
    /* 3DB68 8004D368 06106200 */  srlv       $v0, $v0, $v1
    /* 3DB6C 8004D36C C2420200 */  srl        $t0, $v0, 11
    /* 3DB70 8004D370 FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3DB74 8004D374 1E04C84A */  ncs
    /* 3DB78 8004D378 801F043C */  lui        $a0, (0x1F800018 >> 16)
    /* 3DB7C 8004D37C 1800848C */  lw         $a0, (0x1F800018 & 0xFFFF)($a0)
    /* 3DB80 8004D380 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3DB84 8004D384 07100201 */  srav       $v0, $v0, $t0
    /* 3DB88 8004D388 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3DB8C 8004D38C 2A084400 */  slt        $at, $v0, $a0
    /* 3DB90 8004D390 15002014 */  bnez       $at, .L8004D3E8
    /* 3DB94 8004D394 04000C86 */   lh        $t4, 0x4($s0)
    /* 3DB98 8004D398 00000D86 */  lh         $t5, 0x0($s0)
    /* 3DB9C 8004D39C 00640C00 */  sll        $t4, $t4, 16
    /* 3DBA0 8004D3A0 006C0D00 */  sll        $t5, $t5, 16
    /* 3DBA4 8004D3A4 06000896 */  lhu        $t0, 0x6($s0)
    /* 3DBA8 8004D3A8 08000996 */  lhu        $t1, 0x8($s0)
    /* 3DBAC 8004D3AC 0A000A96 */  lhu        $t2, 0xA($s0)
    /* 3DBB0 8004D3B0 25400C01 */  or         $t0, $t0, $t4
    /* 3DBB4 8004D3B4 25482D01 */  or         $t1, $t1, $t5
    /* 3DBB8 8004D3B8 0C00C8AE */  sw         $t0, 0xC($s6)
    /* 3DBBC 8004D3BC 1400C9AE */  sw         $t1, 0x14($s6)
    /* 3DBC0 8004D3C0 1C00CAA6 */  sh         $t2, 0x1C($s6)
    /* 3DBC4 8004D3C4 0400D6EA */  swc2       $22, 0x4($s6)
    /* 3DBC8 8004D3C8 80100200 */  sll        $v0, $v0, 2
    /* 3DBCC 8004D3CC 21105400 */  addu       $v0, $v0, $s4
    /* 3DBD0 8004D3D0 0000498C */  lw         $t1, 0x0($v0)
    /* 3DBD4 8004D3D4 2440DE02 */  and        $t0, $s6, $fp
    /* 3DBD8 8004D3D8 000048AC */  sw         $t0, 0x0($v0)
    /* 3DBDC 8004D3DC 25483501 */  or         $t1, $t1, $s5
    /* 3DBE0 8004D3E0 0000C9AE */  sw         $t1, 0x0($s6)
    /* 3DBE4 8004D3E4 2000D626 */  addiu      $s6, $s6, 0x20
  .L8004D3E8:
    /* 3DBE8 8004D3E8 18001026 */  addiu      $s0, $s0, 0x18
    /* 3DBEC 8004D3EC AEFF6016 */  bnez       $s3, .L8004D2A8
    /* 3DBF0 8004D3F0 FFFF7326 */   addiu     $s3, $s3, -0x1
  .L8004D3F4:
    /* 3DBF4 8004D3F4 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 3DBF8 8004D3F8 B43836AC */  sw         $s6, %lo(D_800A38B4)($at)
    /* 3DBFC 8004D3FC 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* 3DC00 8004D400 2800BE8F */  lw         $fp, 0x28($sp)
    /* 3DC04 8004D404 2400B68F */  lw         $s6, 0x24($sp)
    /* 3DC08 8004D408 2000B58F */  lw         $s5, 0x20($sp)
    /* 3DC0C 8004D40C 1C00B48F */  lw         $s4, 0x1C($sp)
    /* 3DC10 8004D410 1800B38F */  lw         $s3, 0x18($sp)
    /* 3DC14 8004D414 1400B28F */  lw         $s2, 0x14($sp)
    /* 3DC18 8004D418 1000B18F */  lw         $s1, 0x10($sp)
    /* 3DC1C 8004D41C 0800E003 */  jr         $ra
    /* 3DC20 8004D420 3800BD27 */   addiu     $sp, $sp, 0x38
endlabel func_8004D244
