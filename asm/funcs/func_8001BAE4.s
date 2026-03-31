glabel func_8001BAE4
    /* C2E4 8001BAE4 0A80023C */  lui        $v0, %hi(D_800A387C)
    /* C2E8 8001BAE8 7C38428C */  lw         $v0, %lo(D_800A387C)($v0)
    /* C2EC 8001BAEC D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* C2F0 8001BAF0 1C00B1AF */  sw         $s1, 0x1C($sp)
    /* C2F4 8001BAF4 21888000 */  addu       $s1, $a0, $zero
    /* C2F8 8001BAF8 2000B2AF */  sw         $s2, 0x20($sp)
    /* C2FC 8001BAFC 2190A000 */  addu       $s2, $a1, $zero
    /* C300 8001BB00 1800B0AF */  sw         $s0, 0x18($sp)
    /* C304 8001BB04 2180C000 */  addu       $s0, $a2, $zero
    /* C308 8001BB08 2800BFAF */  sw         $ra, 0x28($sp)
    /* C30C 8001BB0C 11274228 */  slti       $v0, $v0, 0x2711
    /* C310 8001BB10 06004010 */  beqz       $v0, .L8001BB2C
    /* C314 8001BB14 2400B3AF */   sw        $s3, 0x24($sp)
    /* C318 8001BB18 C2171000 */  srl        $v0, $s0, 31
    /* C31C 8001BB1C 21100202 */  addu       $v0, $s0, $v0
    /* C320 8001BB20 43100200 */  sra        $v0, $v0, 1
    /* C324 8001BB24 CC6E0008 */  j          .L8001BB30
    /* C328 8001BB28 00085324 */   addiu     $s3, $v0, 0x800
  .L8001BB2C:
    /* C32C 8001BB2C 00101324 */  addiu      $s3, $zero, 0x1000
  .L8001BB30:
    /* C330 8001BB30 04004386 */  lh         $v1, 0x4($s2)
    /* C334 8001BB34 04002486 */  lh         $a0, 0x4($s1)
    /* C338 8001BB38 08004286 */  lh         $v0, 0x8($s2)
    /* C33C 8001BB3C 08002586 */  lh         $a1, 0x8($s1)
    /* C340 8001BB40 23206400 */  subu       $a0, $v1, $a0
    /* C344 8001BB44 57FF010C */  jal        func_8007FD5C
    /* C348 8001BB48 23284500 */   subu      $a1, $v0, $a1
    /* C34C 8001BB4C 21180002 */  addu       $v1, $s0, $zero
    /* C350 8001BB50 02000106 */  bgez       $s0, .L8001BB5C
    /* C354 8001BB54 21304000 */   addu      $a2, $v0, $zero
    /* C358 8001BB58 03000326 */  addiu      $v1, $s0, 0x3
  .L8001BB5C:
    /* C35C 8001BB5C 43100300 */  sra        $v0, $v1, 1
    /* C360 8001BB60 FE1F4230 */  andi       $v0, $v0, 0x1FFE
    /* C364 8001BB64 0980013C */  lui        $at, %hi(D_800973FC)
    /* C368 8001BB68 21082200 */  addu       $at, $at, $v0
    /* C36C 8001BB6C FC732384 */  lh         $v1, %lo(D_800973FC)($at)
    /* C370 8001BB70 00000000 */  nop
    /* C374 8001BB74 40100300 */  sll        $v0, $v1, 1
    /* C378 8001BB78 21104300 */  addu       $v0, $v0, $v1
    /* C37C 8001BB7C 02004104 */  bgez       $v0, .L8001BB88
    /* C380 8001BB80 00000000 */   nop
    /* C384 8001BB84 03004224 */  addiu      $v0, $v0, 0x3
  .L8001BB88:
    /* C388 8001BB88 0F80043C */  lui        $a0, %hi(D_800F6608)
    /* C38C 8001BB8C 08668424 */  addiu      $a0, $a0, %lo(D_800F6608)
    /* C390 8001BB90 21282002 */  addu       $a1, $s1, $zero
    /* C394 8001BB94 83180200 */  sra        $v1, $v0, 2
    /* C398 8001BB98 00050224 */  addiu      $v0, $zero, 0x500
    /* C39C 8001BB9C 23104300 */  subu       $v0, $v0, $v1
    /* C3A0 8001BBA0 23104600 */  subu       $v0, $v0, $a2
    /* C3A4 8001BBA4 21304002 */  addu       $a2, $s2, $zero
    /* C3A8 8001BBA8 21380002 */  addu       $a3, $s0, $zero
    /* C3AC 8001BBAC 1000B3AF */  sw         $s3, 0x10($sp)
    /* C3B0 8001BBB0 D26D000C */  jal        DispPracticeMenuTex_A
    /* C3B4 8001BBB4 1400A2AF */   sw        $v0, 0x14($sp)
    /* C3B8 8001BBB8 2800BF8F */  lw         $ra, 0x28($sp)
    /* C3BC 8001BBBC 2400B38F */  lw         $s3, 0x24($sp)
    /* C3C0 8001BBC0 2000B28F */  lw         $s2, 0x20($sp)
    /* C3C4 8001BBC4 1C00B18F */  lw         $s1, 0x1C($sp)
    /* C3C8 8001BBC8 1800B08F */  lw         $s0, 0x18($sp)
    /* C3CC 8001BBCC 3000BD27 */  addiu      $sp, $sp, 0x30
    /* C3D0 8001BBD0 0800E003 */  jr         $ra
    /* C3D4 8001BBD4 00000000 */   nop
endlabel func_8001BAE4
