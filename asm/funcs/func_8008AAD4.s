glabel func_8008AAD4
    /* 7B2D4 8008AAD4 FF00023C */  lui        $v0, (0xFFFFFF >> 16)
    /* 7B2D8 8008AAD8 FFFF4234 */  ori        $v0, $v0, (0xFFFFFF & 0xFFFF)
    /* 7B2DC 8008AADC 2428A200 */  and        $a1, $a1, $v0
    /* 7B2E0 8008AAE0 2130A000 */  addu       $a2, $a1, $zero
    /* 7B2E4 8008AAE4 02440500 */  srl        $t0, $a1, 16
    /* 7B2E8 8008AAE8 3E008010 */  beqz       $a0, .L8008ABE4
    /* 7B2EC 8008AAEC 21380001 */   addu      $a3, $t0, $zero
    /* 7B2F0 8008AAF0 01000224 */  addiu      $v0, $zero, 0x1
    /* 7B2F4 8008AAF4 74008214 */  bne        $a0, $v0, .L8008ACC8
    /* 7B2F8 8008AAF8 00000000 */   nop
    /* 7B2FC 8008AAFC 0A80023C */  lui        $v0, %hi(D_800A2CD4)
    /* 7B300 8008AB00 D42C428C */  lw         $v0, %lo(D_800A2CD4)($v0)
    /* 7B304 8008AB04 00000000 */  nop
    /* 7B308 8008AB08 01004230 */  andi       $v0, $v0, 0x1
    /* 7B30C 8008AB0C 2C004010 */  beqz       $v0, .L8008ABC0
    /* 7B310 8008AB10 00000000 */   nop
    /* 7B314 8008AB14 0F80023C */  lui        $v0, %hi(D_800F7420)
    /* 7B318 8008AB18 20744224 */  addiu      $v0, $v0, %lo(D_800F7420)
    /* 7B31C 8008AB1C 000046A4 */  sh         $a2, 0x0($v0)
    /* 7B320 8008AB20 0F80013C */  lui        $at, %hi(D_800F7420 + 0x2)
    /* 7B324 8008AB24 227427A4 */  sh         $a3, %lo(D_800F7420 + 0x2)($at)
    /* 7B328 8008AB28 0A80023C */  lui        $v0, %hi(D_800A28A0)
    /* 7B32C 8008AB2C A028428C */  lw         $v0, %lo(D_800A28A0)($v0)
    /* 7B330 8008AB30 00000000 */  nop
    /* 7B334 8008AB34 01004234 */  ori        $v0, $v0, 0x1
    /* 7B338 8008AB38 0A80013C */  lui        $at, %hi(D_800A28A0)
    /* 7B33C 8008AB3C A02822AC */  sw         $v0, %lo(D_800A28A0)($at)
    /* 7B340 8008AB40 0A80023C */  lui        $v0, %hi(D_800A289C)
    /* 7B344 8008AB44 9C28428C */  lw         $v0, %lo(D_800A289C)($v0)
    /* 7B348 8008AB48 00000000 */  nop
    /* 7B34C 8008AB4C 25104500 */  or         $v0, $v0, $a1
    /* 7B350 8008AB50 0A80013C */  lui        $at, %hi(D_800A289C)
    /* 7B354 8008AB54 9C2822AC */  sw         $v0, %lo(D_800A289C)($at)
    /* 7B358 8008AB58 0F80023C */  lui        $v0, %hi(D_800F7424)
    /* 7B35C 8008AB5C 24744294 */  lhu        $v0, %lo(D_800F7424)($v0)
    /* 7B360 8008AB60 00000000 */  nop
    /* 7B364 8008AB64 24104500 */  and        $v0, $v0, $a1
    /* 7B368 8008AB68 07004010 */  beqz       $v0, .L8008AB88
    /* 7B36C 8008AB6C 00000000 */   nop
    /* 7B370 8008AB70 0F80023C */  lui        $v0, %hi(D_800F7424)
    /* 7B374 8008AB74 24744294 */  lhu        $v0, %lo(D_800F7424)($v0)
    /* 7B378 8008AB78 27180500 */  nor        $v1, $zero, $a1
    /* 7B37C 8008AB7C 24104300 */  and        $v0, $v0, $v1
    /* 7B380 8008AB80 0F80013C */  lui        $at, %hi(D_800F7424)
    /* 7B384 8008AB84 247422A4 */  sh         $v0, %lo(D_800F7424)($at)
  .L8008AB88:
    /* 7B388 8008AB88 0F80023C */  lui        $v0, %hi(D_800F7424 + 0x2)
    /* 7B38C 8008AB8C 26744294 */  lhu        $v0, %lo(D_800F7424 + 0x2)($v0)
    /* 7B390 8008AB90 00000000 */  nop
    /* 7B394 8008AB94 24104800 */  and        $v0, $v0, $t0
    /* 7B398 8008AB98 4B004010 */  beqz       $v0, .L8008ACC8
    /* 7B39C 8008AB9C 00000000 */   nop
    /* 7B3A0 8008ABA0 0F80023C */  lui        $v0, %hi(D_800F7424 + 0x2)
    /* 7B3A4 8008ABA4 26744294 */  lhu        $v0, %lo(D_800F7424 + 0x2)($v0)
    /* 7B3A8 8008ABA8 27180800 */  nor        $v1, $zero, $t0
    /* 7B3AC 8008ABAC 24104300 */  and        $v0, $v0, $v1
    /* 7B3B0 8008ABB0 0F80013C */  lui        $at, %hi(D_800F7424 + 0x2)
    /* 7B3B4 8008ABB4 267422A4 */  sh         $v0, %lo(D_800F7424 + 0x2)($at)
    /* 7B3B8 8008ABB8 322B0208 */  j          .L8008ACC8
    /* 7B3BC 8008ABBC 00000000 */   nop
  .L8008ABC0:
    /* 7B3C0 8008ABC0 0A80023C */  lui        $v0, %hi(D_800A2874)
    /* 7B3C4 8008ABC4 7428428C */  lw         $v0, %lo(D_800A2874)($v0)
    /* 7B3C8 8008ABC8 0A80033C */  lui        $v1, %hi(D_800A2CDC)
    /* 7B3CC 8008ABCC DC2C638C */  lw         $v1, %lo(D_800A2CDC)($v1)
    /* 7B3D0 8008ABD0 25104500 */  or         $v0, $v0, $a1
    /* 7B3D4 8008ABD4 880166A4 */  sh         $a2, 0x188($v1)
    /* 7B3D8 8008ABD8 8A0167A4 */  sh         $a3, 0x18A($v1)
    /* 7B3DC 8008ABDC 302B0208 */  j          .L8008ACC0
    /* 7B3E0 8008ABE0 00000000 */   nop
  .L8008ABE4:
    /* 7B3E4 8008ABE4 0A80023C */  lui        $v0, %hi(D_800A2CD4)
    /* 7B3E8 8008ABE8 D42C428C */  lw         $v0, %lo(D_800A2CD4)($v0)
    /* 7B3EC 8008ABEC 00000000 */  nop
    /* 7B3F0 8008ABF0 01004230 */  andi       $v0, $v0, 0x1
    /* 7B3F4 8008ABF4 29004010 */  beqz       $v0, .L8008AC9C
    /* 7B3F8 8008ABF8 00000000 */   nop
    /* 7B3FC 8008ABFC 0F80013C */  lui        $at, %hi(D_800F7424)
    /* 7B400 8008AC00 247426A4 */  sh         $a2, %lo(D_800F7424)($at)
    /* 7B404 8008AC04 0F80013C */  lui        $at, %hi(D_800F7424 + 0x2)
    /* 7B408 8008AC08 267427A4 */  sh         $a3, %lo(D_800F7424 + 0x2)($at)
    /* 7B40C 8008AC0C 0A80023C */  lui        $v0, %hi(D_800A28A0)
    /* 7B410 8008AC10 A028428C */  lw         $v0, %lo(D_800A28A0)($v0)
    /* 7B414 8008AC14 27200500 */  nor        $a0, $zero, $a1
    /* 7B418 8008AC18 01004234 */  ori        $v0, $v0, 0x1
    /* 7B41C 8008AC1C 0A80013C */  lui        $at, %hi(D_800A28A0)
    /* 7B420 8008AC20 A02822AC */  sw         $v0, %lo(D_800A28A0)($at)
    /* 7B424 8008AC24 0A80023C */  lui        $v0, %hi(D_800A289C)
    /* 7B428 8008AC28 9C28428C */  lw         $v0, %lo(D_800A289C)($v0)
    /* 7B42C 8008AC2C 0F80033C */  lui        $v1, %hi(D_800F7420)
    /* 7B430 8008AC30 20746324 */  addiu      $v1, $v1, %lo(D_800F7420)
    /* 7B434 8008AC34 24104400 */  and        $v0, $v0, $a0
    /* 7B438 8008AC38 0A80013C */  lui        $at, %hi(D_800A289C)
    /* 7B43C 8008AC3C 9C2822AC */  sw         $v0, %lo(D_800A289C)($at)
    /* 7B440 8008AC40 00006294 */  lhu        $v0, 0x0($v1)
    /* 7B444 8008AC44 00000000 */  nop
    /* 7B448 8008AC48 24104500 */  and        $v0, $v0, $a1
    /* 7B44C 8008AC4C 05004010 */  beqz       $v0, .L8008AC64
    /* 7B450 8008AC50 00000000 */   nop
    /* 7B454 8008AC54 00006294 */  lhu        $v0, 0x0($v1)
    /* 7B458 8008AC58 00000000 */  nop
    /* 7B45C 8008AC5C 24104400 */  and        $v0, $v0, $a0
    /* 7B460 8008AC60 000062A4 */  sh         $v0, 0x0($v1)
  .L8008AC64:
    /* 7B464 8008AC64 0F80023C */  lui        $v0, %hi(D_800F7420 + 0x2)
    /* 7B468 8008AC68 22744294 */  lhu        $v0, %lo(D_800F7420 + 0x2)($v0)
    /* 7B46C 8008AC6C 00000000 */  nop
    /* 7B470 8008AC70 24104800 */  and        $v0, $v0, $t0
    /* 7B474 8008AC74 14004010 */  beqz       $v0, .L8008ACC8
    /* 7B478 8008AC78 00000000 */   nop
    /* 7B47C 8008AC7C 0F80023C */  lui        $v0, %hi(D_800F7420 + 0x2)
    /* 7B480 8008AC80 22744294 */  lhu        $v0, %lo(D_800F7420 + 0x2)($v0)
    /* 7B484 8008AC84 27180800 */  nor        $v1, $zero, $t0
    /* 7B488 8008AC88 24104300 */  and        $v0, $v0, $v1
    /* 7B48C 8008AC8C 0F80013C */  lui        $at, %hi(D_800F7420 + 0x2)
    /* 7B490 8008AC90 227422A4 */  sh         $v0, %lo(D_800F7420 + 0x2)($at)
    /* 7B494 8008AC94 322B0208 */  j          .L8008ACC8
    /* 7B498 8008AC98 00000000 */   nop
  .L8008AC9C:
    /* 7B49C 8008AC9C 0A80023C */  lui        $v0, %hi(D_800A2CDC)
    /* 7B4A0 8008ACA0 DC2C428C */  lw         $v0, %lo(D_800A2CDC)($v0)
    /* 7B4A4 8008ACA4 00000000 */  nop
    /* 7B4A8 8008ACA8 8C0146A4 */  sh         $a2, 0x18C($v0)
    /* 7B4AC 8008ACAC 8E0147A4 */  sh         $a3, 0x18E($v0)
    /* 7B4B0 8008ACB0 0A80023C */  lui        $v0, %hi(D_800A2874)
    /* 7B4B4 8008ACB4 7428428C */  lw         $v0, %lo(D_800A2874)($v0)
    /* 7B4B8 8008ACB8 27180500 */  nor        $v1, $zero, $a1
    /* 7B4BC 8008ACBC 24104300 */  and        $v0, $v0, $v1
  .L8008ACC0:
    /* 7B4C0 8008ACC0 0A80013C */  lui        $at, %hi(D_800A2874)
    /* 7B4C4 8008ACC4 742822AC */  sw         $v0, %lo(D_800A2874)($at)
  .L8008ACC8:
    /* 7B4C8 8008ACC8 0800E003 */  jr         $ra
    /* 7B4CC 8008ACCC 00000000 */   nop
endlabel func_8008AAD4
