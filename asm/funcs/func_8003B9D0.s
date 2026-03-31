glabel func_8003B9D0
    /* 2C1D0 8003B9D0 C8FFBD27 */  addiu      $sp, $sp, -0x38
    /* 2C1D4 8003B9D4 2C00B3AF */  sw         $s3, 0x2C($sp)
    /* 2C1D8 8003B9D8 1980133C */  lui        $s3, (0x80190800 >> 16)
    /* 2C1DC 8003B9DC 00087336 */  ori        $s3, $s3, (0x80190800 & 0xFFFF)
    /* 2C1E0 8003B9E0 3000BFAF */  sw         $ra, 0x30($sp)
    /* 2C1E4 8003B9E4 2800B2AF */  sw         $s2, 0x28($sp)
    /* 2C1E8 8003B9E8 2400B1AF */  sw         $s1, 0x24($sp)
    /* 2C1EC 8003B9EC 8B76000C */  jal        func_8001DA2C
    /* 2C1F0 8003B9F0 2000B0AF */   sw        $s0, 0x20($sp)
    /* 2C1F4 8003B9F4 5E84010C */  jal        func_80061178
    /* 2C1F8 8003B9F8 00000000 */   nop
    /* 2C1FC 8003B9FC 0A80033C */  lui        $v1, %hi(D_800A3768)
    /* 2C200 8003BA00 68376390 */  lbu        $v1, %lo(D_800A3768)($v1)
    /* 2C204 8003BA04 14000224 */  addiu      $v0, $zero, 0x14
    /* 2C208 8003BA08 06006210 */  beq        $v1, $v0, .L8003BA24
    /* 2C20C 8003BA0C FF000224 */   addiu     $v0, $zero, 0xFF
    /* 2C210 8003BA10 225A000C */  jal        gpu_InitDisplay
    /* 2C214 8003BA14 00000000 */   nop
    /* 2C218 8003BA18 0A80033C */  lui        $v1, %hi(D_800A3768)
    /* 2C21C 8003BA1C 68376390 */  lbu        $v1, %lo(D_800A3768)($v1)
    /* 2C220 8003BA20 FF000224 */  addiu      $v0, $zero, 0xFF
  .L8003BA24:
    /* 2C224 8003BA24 03006210 */  beq        $v1, $v0, .L8003BA34
    /* 2C228 8003BA28 00000000 */   nop
    /* 2C22C 8003BA2C 345A000C */  jal        gpu_DisableDisplay
    /* 2C230 8003BA30 00000000 */   nop
  .L8003BA34:
    /* 2C234 8003BA34 3D5D000C */  jal        gnd_disp_loop_ctrl
    /* 2C238 8003BA38 00000000 */   nop
    /* 2C23C 8003BA3C 1A5A000C */  jal        gpu_EnableDisplay
    /* 2C240 8003BA40 00000000 */   nop
    /* 2C244 8003BA44 4E83000C */  jal        func_80020D38
    /* 2C248 8003BA48 00000000 */   nop
    /* 2C24C 8003BA4C 01000424 */  addiu      $a0, $zero, 0x1
    /* 2C250 8003BA50 21280000 */  addu       $a1, $zero, $zero
    /* 2C254 8003BA54 21300000 */  addu       $a2, $zero, $zero
    /* 2C258 8003BA58 DA59000C */  jal        disp_SetFramebufferMode
    /* 2C25C 8003BA5C 21380000 */   addu      $a3, $zero, $zero
    /* 2C260 8003BA60 0A80023C */  lui        $v0, %hi(D_800A3878)
    /* 2C264 8003BA64 7838428C */  lw         $v0, %lo(D_800A3878)($v0)
    /* 2C268 8003BA68 00000000 */  nop
    /* 2C26C 8003BA6C 03004290 */  lbu        $v0, 0x3($v0)
    /* 2C270 8003BA70 00000000 */  nop
    /* 2C274 8003BA74 80004230 */  andi       $v0, $v0, 0x80
    /* 2C278 8003BA78 04004010 */  beqz       $v0, .L8003BA8C
    /* 2C27C 8003BA7C 00000000 */   nop
    /* 2C280 8003BA80 3783000C */  jal        func_80020CDC
    /* 2C284 8003BA84 1180133C */   lui       $s3, (0x80118800 >> 16)
    /* 2C288 8003BA88 00887336 */  ori        $s3, $s3, (0x80118800 & 0xFFFF)
  .L8003BA8C:
    /* 2C28C 8003BA8C 0A80043C */  lui        $a0, %hi(D_800A3878)
    /* 2C290 8003BA90 7838848C */  lw         $a0, %lo(D_800A3878)($a0)
    /* 2C294 8003BA94 00000000 */  nop
    /* 2C298 8003BA98 03008390 */  lbu        $v1, 0x3($a0)
    /* 2C29C 8003BA9C 00000000 */  nop
    /* 2C2A0 8003BAA0 30006230 */  andi       $v0, $v1, 0x30
    /* 2C2A4 8003BAA4 12004010 */  beqz       $v0, .L8003BAF0
    /* 2C2A8 8003BAA8 10006230 */   andi      $v0, $v1, 0x10
    /* 2C2AC 8003BAAC 1080103C */  lui        $s0, %hi(D_80101EDA)
    /* 2C2B0 8003BAB0 DA1E1026 */  addiu      $s0, $s0, %lo(D_80101EDA)
    /* 2C2B4 8003BAB4 00001186 */  lh         $s1, 0x0($s0)
    /* 2C2B8 8003BAB8 4C041286 */  lh         $s2, 0x44C($s0)
    /* 2C2BC 8003BABC 02004010 */  beqz       $v0, .L8003BAC8
    /* 2C2C0 8003BAC0 32000224 */   addiu     $v0, $zero, 0x32
    /* 2C2C4 8003BAC4 000002A6 */  sh         $v0, 0x0($s0)
  .L8003BAC8:
    /* 2C2C8 8003BAC8 03008290 */  lbu        $v0, 0x3($a0)
    /* 2C2CC 8003BACC 00000000 */  nop
    /* 2C2D0 8003BAD0 20004230 */  andi       $v0, $v0, 0x20
    /* 2C2D4 8003BAD4 02004010 */  beqz       $v0, .L8003BAE0
    /* 2C2D8 8003BAD8 32000224 */   addiu     $v0, $zero, 0x32
    /* 2C2DC 8003BADC 4C0402A6 */  sh         $v0, 0x44C($s0)
  .L8003BAE0:
    /* 2C2E0 8003BAE0 FFEB000C */  jal        func_8003AFFC
    /* 2C2E4 8003BAE4 00000000 */   nop
    /* 2C2E8 8003BAE8 000011A6 */  sh         $s1, 0x0($s0)
    /* 2C2EC 8003BAEC 4C0412A6 */  sh         $s2, 0x44C($s0)
  .L8003BAF0:
    /* 2C2F0 8003BAF0 0A80023C */  lui        $v0, %hi(D_800A3878)
    /* 2C2F4 8003BAF4 7838428C */  lw         $v0, %lo(D_800A3878)($v0)
    /* 2C2F8 8003BAF8 00000000 */  nop
    /* 2C2FC 8003BAFC 03004290 */  lbu        $v0, 0x3($v0)
    /* 2C300 8003BB00 00000000 */  nop
    /* 2C304 8003BB04 01004230 */  andi       $v0, $v0, 0x1
    /* 2C308 8003BB08 03004010 */  beqz       $v0, .L8003BB18
    /* 2C30C 8003BB0C FFFF0724 */   addiu     $a3, $zero, -0x1
    /* 2C310 8003BB10 1080073C */  lui        $a3, %hi(D_80101EDA)
    /* 2C314 8003BB14 DA1EE784 */  lh         $a3, %lo(D_80101EDA)($a3)
  .L8003BB18:
    /* 2C318 8003BB18 0A80023C */  lui        $v0, %hi(D_800A3878)
    /* 2C31C 8003BB1C 7838428C */  lw         $v0, %lo(D_800A3878)($v0)
    /* 2C320 8003BB20 00000000 */  nop
    /* 2C324 8003BB24 03004290 */  lbu        $v0, 0x3($v0)
    /* 2C328 8003BB28 00000000 */  nop
    /* 2C32C 8003BB2C 02004230 */  andi       $v0, $v0, 0x2
    /* 2C330 8003BB30 03004010 */  beqz       $v0, .L8003BB40
    /* 2C334 8003BB34 FFFF0424 */   addiu     $a0, $zero, -0x1
    /* 2C338 8003BB38 1080043C */  lui        $a0, %hi(D_80102326)
    /* 2C33C 8003BB3C 26238484 */  lh         $a0, %lo(D_80102326)($a0)
  .L8003BB40:
    /* 2C340 8003BB40 0A80053C */  lui        $a1, %hi(D_800A3878)
    /* 2C344 8003BB44 7838A58C */  lw         $a1, %lo(D_800A3878)($a1)
    /* 2C348 8003BB48 00000000 */  nop
    /* 2C34C 8003BB4C 0300A390 */  lbu        $v1, 0x3($a1)
    /* 2C350 8003BB50 00000000 */  nop
    /* 2C354 8003BB54 10006230 */  andi       $v0, $v1, 0x10
    /* 2C358 8003BB58 02004010 */  beqz       $v0, .L8003BB64
    /* 2C35C 8003BB5C 20006230 */   andi      $v0, $v1, 0x20
    /* 2C360 8003BB60 32000724 */  addiu      $a3, $zero, 0x32
  .L8003BB64:
    /* 2C364 8003BB64 02004010 */  beqz       $v0, .L8003BB70
    /* 2C368 8003BB68 00000000 */   nop
    /* 2C36C 8003BB6C 32000424 */  addiu      $a0, $zero, 0x32
  .L8003BB70:
    /* 2C370 8003BB70 0A80013C */  lui        $at, %hi(D_800A390F)
    /* 2C374 8003BB74 0F3920A0 */  sb         $zero, %lo(D_800A390F)($at)
    /* 2C378 8003BB78 0000A590 */  lbu        $a1, 0x0($a1)
    /* 2C37C 8003BB7C 21300000 */  addu       $a2, $zero, $zero
    /* 2C380 8003BB80 1000A4AF */  sw         $a0, 0x10($sp)
    /* 2C384 8003BB84 0A80043C */  lui        $a0, %hi(D_800A376C)
    /* 2C388 8003BB88 6C378490 */  lbu        $a0, %lo(D_800A376C)($a0)
    /* 2C38C 8003BB8C FFFF0224 */  addiu      $v0, $zero, -0x1
    /* 2C390 8003BB90 1400A2AF */  sw         $v0, 0x14($sp)
    /* 2C394 8003BB94 1800A2AF */  sw         $v0, 0x18($sp)
    /* 2C398 8003BB98 2152010C */  jal        func_80054884
    /* 2C39C 8003BB9C 1C00B3AF */   sw        $s3, 0x1C($sp)
    /* 2C3A0 8003BBA0 21200000 */  addu       $a0, $zero, $zero
    /* 2C3A4 8003BBA4 A205010C */  jal        func_80041688
    /* 2C3A8 8003BBA8 21280000 */   addu      $a1, $zero, $zero
    /* 2C3AC 8003BBAC 01000424 */  addiu      $a0, $zero, 0x1
    /* 2C3B0 8003BBB0 A205010C */  jal        func_80041688
    /* 2C3B4 8003BBB4 21280000 */   addu      $a1, $zero, $zero
    /* 2C3B8 8003BBB8 0A80023C */  lui        $v0, %hi(D_800A3878)
    /* 2C3BC 8003BBBC 7838428C */  lw         $v0, %lo(D_800A3878)($v0)
    /* 2C3C0 8003BBC0 00000000 */  nop
    /* 2C3C4 8003BBC4 03004290 */  lbu        $v0, 0x3($v0)
    /* 2C3C8 8003BBC8 00000000 */  nop
    /* 2C3CC 8003BBCC 40004230 */  andi       $v0, $v0, 0x40
    /* 2C3D0 8003BBD0 03004010 */  beqz       $v0, .L8003BBE0
    /* 2C3D4 8003BBD4 00000000 */   nop
    /* 2C3D8 8003BBD8 6719010C */  jal        func_8004659C
    /* 2C3DC 8003BBDC FFFF0424 */   addiu     $a0, $zero, -0x1
  .L8003BBE0:
    /* 2C3E0 8003BBE0 1080033C */  lui        $v1, %hi(D_8010277D)
    /* 2C3E4 8003BBE4 7D276380 */  lb         $v1, %lo(D_8010277D)($v1)
    /* 2C3E8 8003BBE8 0E000224 */  addiu      $v0, $zero, 0xE
    /* 2C3EC 8003BBEC 03006210 */  beq        $v1, $v0, .L8003BBFC
    /* 2C3F0 8003BBF0 1D000224 */   addiu     $v0, $zero, 0x1D
    /* 2C3F4 8003BBF4 09006214 */  bne        $v1, $v0, .L8003BC1C
    /* 2C3F8 8003BBF8 00000000 */   nop
  .L8003BBFC:
    /* 2C3FC 8003BBFC 0A80043C */  lui        $a0, %hi(D_800A37B4)
    /* 2C400 8003BC00 B4378490 */  lbu        $a0, %lo(D_800A37B4)($a0)
    /* 2C404 8003BC04 0A80053C */  lui        $a1, %hi(D_800A37B5)
    /* 2C408 8003BC08 B537A590 */  lbu        $a1, %lo(D_800A37B5)($a1)
    /* 2C40C 8003BC0C 0A80063C */  lui        $a2, %hi(D_800A37B6)
    /* 2C410 8003BC10 B637C690 */  lbu        $a2, %lo(D_800A37B6)($a2)
    /* 2C414 8003BC14 FD06010C */  jal        func_80041BF4
    /* 2C418 8003BC18 00000000 */   nop
  .L8003BC1C:
    /* 2C41C 8003BC1C F976000C */  jal        func_8001DBE4
    /* 2C420 8003BC20 00000000 */   nop
    /* 2C424 8003BC24 FF000224 */  addiu      $v0, $zero, 0xFF
    /* 2C428 8003BC28 0A80013C */  lui        $at, %hi(D_800A3768)
    /* 2C42C 8003BC2C 683722A0 */  sb         $v0, %lo(D_800A3768)($at)
    /* 2C430 8003BC30 0A80013C */  lui        $at, %hi(D_800A36A8)
    /* 2C434 8003BC34 A83620A0 */  sb         $zero, %lo(D_800A36A8)($at)
    /* 2C438 8003BC38 EAD7000C */  jal        func_80035FA8
    /* 2C43C 8003BC3C 00000000 */   nop
    /* 2C440 8003BC40 0A80023C */  lui        $v0, %hi(D_800A3878)
    /* 2C444 8003BC44 7838428C */  lw         $v0, %lo(D_800A3878)($v0)
    /* 2C448 8003BC48 00000000 */  nop
    /* 2C44C 8003BC4C 01004590 */  lbu        $a1, 0x1($v0)
    /* 2C450 8003BC50 AADB000C */  jal        func_80036EA8
    /* 2C454 8003BC54 05000424 */   addiu     $a0, $zero, 0x5
    /* 2C458 8003BC58 0A80033C */  lui        $v1, %hi(D_800A3878)
    /* 2C45C 8003BC5C 7838638C */  lw         $v1, %lo(D_800A3878)($v1)
    /* 2C460 8003BC60 00000000 */  nop
    /* 2C464 8003BC64 02006590 */  lbu        $a1, 0x2($v1)
    /* 2C468 8003BC68 F5DB000C */  jal        func_80036FD4
    /* 2C46C 8003BC6C 21204000 */   addu      $a0, $v0, $zero
    /* 2C470 8003BC70 98DC000C */  jal        func_80037260
    /* 2C474 8003BC74 00000000 */   nop
    /* 2C478 8003BC78 07000224 */  addiu      $v0, $zero, 0x7
    /* 2C47C 8003BC7C 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* 2C480 8003BC80 B83720AC */  sw         $zero, %lo(D_800A37B8)($at)
    /* 2C484 8003BC84 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2C488 8003BC88 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
    /* 2C48C 8003BC8C 345A000C */  jal        gpu_DisableDisplay
    /* 2C490 8003BC90 00000000 */   nop
    /* 2C494 8003BC94 3000BF8F */  lw         $ra, 0x30($sp)
    /* 2C498 8003BC98 2C00B38F */  lw         $s3, 0x2C($sp)
    /* 2C49C 8003BC9C 2800B28F */  lw         $s2, 0x28($sp)
    /* 2C4A0 8003BCA0 2400B18F */  lw         $s1, 0x24($sp)
    /* 2C4A4 8003BCA4 2000B08F */  lw         $s0, 0x20($sp)
    /* 2C4A8 8003BCA8 3800BD27 */  addiu      $sp, $sp, 0x38
    /* 2C4AC 8003BCAC 0800E003 */  jr         $ra
    /* 2C4B0 8003BCB0 00000000 */   nop
endlabel func_8003B9D0
