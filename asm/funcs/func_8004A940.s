glabel func_8004A940
    /* 3B140 8004A940 04003FAE */  sw         $ra, 0x4($s1)
    /* 3B144 8004A944 00000496 */  lhu        $a0, 0x0($s0)
    /* 3B148 8004A948 02001026 */  addiu      $s0, $s0, 0x2
    /* 3B14C 8004A94C 0C008010 */  beqz       $a0, .L8004A980
  .L8004A950:
    /* 3B150 8004A950 00000896 */   lhu       $t0, 0x0($s0)
    /* 3B154 8004A954 02001026 */  addiu      $s0, $s0, 0x2
    /* 3B158 8004A958 80400800 */  sll        $t0, $t0, 2
    /* 3B15C 8004A95C 21401401 */  addu       $t0, $t0, $s4
    /* 3B160 8004A960 0000088D */  lw         $t0, 0x0($t0)
    /* 3B164 8004A964 00000000 */  nop
    /* 3B168 8004A968 09F80001 */  jalr       $t0
    /* 3B16C 8004A96C 00000000 */   nop
    /* 3B170 8004A970 00000496 */  lhu        $a0, 0x0($s0)
    /* 3B174 8004A974 02001026 */  addiu      $s0, $s0, 0x2
    /* 3B178 8004A978 F5FF8014 */  bnez       $a0, .L8004A950
    /* 3B17C 8004A97C 00000000 */   nop
  .L8004A980:
    /* 3B180 8004A980 04003F8E */  lw         $ra, 0x4($s1)
    /* 3B184 8004A984 00000000 */  nop
    /* 3B188 8004A988 0800E003 */  jr         $ra
    /* 3B18C 8004A98C 00000000 */   nop
  jlabel .L8004A990
    /* 3B190 8004A990 0A80023C */  lui        $v0, %hi(D_800A3708)
    /* 3B194 8004A994 0837428C */  lw         $v0, %lo(D_800A3708)($v0)
    /* 3B198 8004A998 2C00488E */  lw         $t0, 0x2C($s2)
    /* 3B19C 8004A99C 3000498E */  lw         $t1, 0x30($s2)
    /* 3B1A0 8004A9A0 34004A8E */  lw         $t2, 0x34($s2)
    /* 3B1A4 8004A9A4 2C004B8C */  lw         $t3, 0x2C($v0)
    /* 3B1A8 8004A9A8 30004C8C */  lw         $t4, 0x30($v0)
    /* 3B1AC 8004A9AC 34004D8C */  lw         $t5, 0x34($v0)
    /* 3B1B0 8004A9B0 23400B01 */  subu       $t0, $t0, $t3
    /* 3B1B4 8004A9B4 23482C01 */  subu       $t1, $t1, $t4
    /* 3B1B8 8004A9B8 23504D01 */  subu       $t2, $t2, $t5
    /* 3B1BC 8004A9BC 2A700001 */  slt        $t6, $t0, $zero
    /* 3B1C0 8004A9C0 22700E00 */  neg        $t6, $t6 /* handwritten instruction */
    /* 3B1C4 8004A9C4 26580E01 */  xor        $t3, $t0, $t6
    /* 3B1C8 8004A9C8 22586E01 */  sub        $t3, $t3, $t6 /* handwritten instruction */
    /* 3B1CC 8004A9CC 2A702001 */  slt        $t6, $t1, $zero
    /* 3B1D0 8004A9D0 22700E00 */  neg        $t6, $t6 /* handwritten instruction */
    /* 3B1D4 8004A9D4 26602E01 */  xor        $t4, $t1, $t6
    /* 3B1D8 8004A9D8 22608E01 */  sub        $t4, $t4, $t6 /* handwritten instruction */
    /* 3B1DC 8004A9DC 2A704001 */  slt        $t6, $t2, $zero
    /* 3B1E0 8004A9E0 22700E00 */  neg        $t6, $t6 /* handwritten instruction */
    /* 3B1E4 8004A9E4 26684E01 */  xor        $t5, $t2, $t6
    /* 3B1E8 8004A9E8 2268AE01 */  sub        $t5, $t5, $t6 /* handwritten instruction */
    /* 3B1EC 8004A9EC 25586C01 */  or         $t3, $t3, $t4
    /* 3B1F0 8004A9F0 25586D01 */  or         $t3, $t3, $t5
    /* 3B1F4 8004A9F4 02001524 */  addiu      $s5, $zero, 0x2
    /* 3B1F8 8004A9F8 004A6C29 */  slti       $t4, $t3, 0x4A00
    /* 3B1FC 8004A9FC 00A50134 */  ori        $at, $zero, 0xA500
    /* 3B200 8004AA00 2A686101 */  slt        $t5, $t3, $at
    /* 3B204 8004AA04 22A8AC02 */  sub        $s5, $s5, $t4 /* handwritten instruction */
    /* 3B208 8004AA08 22A8AD02 */  sub        $s5, $s5, $t5 /* handwritten instruction */
    /* 3B20C 8004AA0C 00000000 */  nop
    /* 3B210 8004AA10 0740A802 */  srav       $t0, $t0, $s5
    /* 3B214 8004AA14 200028A6 */  sh         $t0, 0x20($s1)
    /* 3B218 8004AA18 0748A902 */  srav       $t1, $t1, $s5
    /* 3B21C 8004AA1C 220029A6 */  sh         $t1, 0x22($s1)
    /* 3B220 8004AA20 0750AA02 */  srav       $t2, $t2, $s5
    /* 3B224 8004AA24 24002AA6 */  sh         $t2, 0x24($s1)
    /* 3B228 8004AA28 0A004886 */  lh         $t0, 0xA($s2)
    /* 3B22C 8004AA2C 0F800A3C */  lui        $t2, %hi(D_800F62E0)
    /* 3B230 8004AA30 E0624A25 */  addiu      $t2, $t2, %lo(D_800F62E0)
    /* 3B234 8004AA34 40480800 */  sll        $t1, $t0, 1
    /* 3B238 8004AA38 20482801 */  add        $t1, $t1, $t0 /* handwritten instruction */
    /* 3B23C 8004AA3C 40490900 */  sll        $t1, $t1, 5
    /* 3B240 8004AA40 21982A01 */  addu       $s3, $t1, $t2
    /* 3B244 8004AA44 18006426 */  addiu      $a0, $s3, 0x18
    /* 3B248 8004AA48 18004526 */  addiu      $a1, $s2, 0x18
    /* 3B24C 8004AA4C 4C4A010C */  jal        func_80052930
    /* 3B250 8004AA50 48002626 */   addiu     $a2, $s1, 0x48
    /* 3B254 8004AA54 4800288E */  lw         $t0, 0x48($s1)
    /* 3B258 8004AA58 4C00298E */  lw         $t1, 0x4C($s1)
    /* 3B25C 8004AA5C 50002A8E */  lw         $t2, 0x50($s1)
    /* 3B260 8004AA60 54002B8E */  lw         $t3, 0x54($s1)
    /* 3B264 8004AA64 58002C8E */  lw         $t4, 0x58($s1)
    /* 3B268 8004AA68 0040C848 */  ctc2       $t0, $8 /* handwritten instruction */
    /* 3B26C 8004AA6C 0048C948 */  ctc2       $t1, $9 /* handwritten instruction */
    /* 3B270 8004AA70 0050CA48 */  ctc2       $t2, $10 /* handwritten instruction */
    /* 3B274 8004AA74 0058CB48 */  ctc2       $t3, $11 /* handwritten instruction */
    /* 3B278 8004AA78 0060CC48 */  ctc2       $t4, $12 /* handwritten instruction */
    /* 3B27C 8004AA7C 3800688E */  lw         $t0, 0x38($s3)
    /* 3B280 8004AA80 3C00698E */  lw         $t1, 0x3C($s3)
    /* 3B284 8004AA84 40006A8E */  lw         $t2, 0x40($s3)
    /* 3B288 8004AA88 44006B8E */  lw         $t3, 0x44($s3)
    /* 3B28C 8004AA8C 48006C8E */  lw         $t4, 0x48($s3)
    /* 3B290 8004AA90 0080C848 */  ctc2       $t0, $16 /* handwritten instruction */
    /* 3B294 8004AA94 0088C948 */  ctc2       $t1, $17 /* handwritten instruction */
    /* 3B298 8004AA98 0090CA48 */  ctc2       $t2, $18 /* handwritten instruction */
    /* 3B29C 8004AA9C 0098CB48 */  ctc2       $t3, $19 /* handwritten instruction */
    /* 3B2A0 8004AAA0 00A0CC48 */  ctc2       $t4, $20 /* handwritten instruction */
    /* 3B2A4 8004AAA4 2120C002 */  addu       $a0, $s6, $zero
    /* 3B2A8 8004AAA8 18004526 */  addiu      $a1, $s2, 0x18
    /* 3B2AC 8004AAAC 4C4A010C */  jal        func_80052930
    /* 3B2B0 8004AAB0 28002626 */   addiu     $a2, $s1, 0x28
    /* 3B2B4 8004AAB4 0000C88E */  lw         $t0, 0x0($s6)
    /* 3B2B8 8004AAB8 0400C98E */  lw         $t1, 0x4($s6)
    /* 3B2BC 8004AABC 0800CA8E */  lw         $t2, 0x8($s6)
    /* 3B2C0 8004AAC0 0C00CB8E */  lw         $t3, 0xC($s6)
    /* 3B2C4 8004AAC4 1000CC8E */  lw         $t4, 0x10($s6)
    /* 3B2C8 8004AAC8 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3B2CC 8004AACC 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 3B2D0 8004AAD0 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 3B2D4 8004AAD4 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 3B2D8 8004AAD8 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 3B2DC 8004AADC 0028C048 */  ctc2       $zero, $5 /* handwritten instruction */
    /* 3B2E0 8004AAE0 0030C048 */  ctc2       $zero, $6 /* handwritten instruction */
    /* 3B2E4 8004AAE4 0038C048 */  ctc2       $zero, $7 /* handwritten instruction */
    /* 3B2E8 8004AAE8 200020CA */  lwc2       $0, 0x20($s1)
    /* 3B2EC 8004AAEC 240021CA */  lwc2       $1, 0x24($s1)
    /* 3B2F0 8004AAF0 5800648E */  lw         $a0, 0x58($s3)
    /* 3B2F4 8004AAF4 F00F0924 */  addiu      $t1, $zero, 0xFF0
    /* 3B2F8 8004AAF8 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 3B2FC 8004AAFC 02290400 */  srl        $a1, $a0, 4
    /* 3B300 8004AB00 02330400 */  srl        $a2, $a0, 12
    /* 3B304 8004AB04 00210400 */  sll        $a0, $a0, 4
    /* 3B308 8004AB08 24208900 */  and        $a0, $a0, $t1
    /* 3B30C 8004AB0C 2428A900 */  and        $a1, $a1, $t1
    /* 3B310 8004AB10 2430C900 */  and        $a2, $a2, $t1
    /* 3B314 8004AB14 0068C448 */  ctc2       $a0, $13 /* handwritten instruction */
    /* 3B318 8004AB18 0070C548 */  ctc2       $a1, $14 /* handwritten instruction */
    /* 3B31C 8004AB1C 0078C648 */  ctc2       $a2, $15 /* handwritten instruction */
    /* 3B320 8004AB20 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 3B324 8004AB24 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 3B328 8004AB28 00580A48 */  mfc2       $t2, $11 /* handwritten instruction */
    /* 3B32C 8004AB2C 43480900 */  sra        $t1, $t1, 1
    /* 3B330 8004AB30 0028C848 */  ctc2       $t0, $5 /* handwritten instruction */
    /* 3B334 8004AB34 0030C948 */  ctc2       $t1, $6 /* handwritten instruction */
    /* 3B338 8004AB38 0038CA48 */  ctc2       $t2, $7 /* handwritten instruction */
    /* 3B33C 8004AB3C 28002886 */  lh         $t0, 0x28($s1)
    /* 3B340 8004AB40 2A002986 */  lh         $t1, 0x2A($s1)
    /* 3B344 8004AB44 2C002A86 */  lh         $t2, 0x2C($s1)
    /* 3B348 8004AB48 2E002B86 */  lh         $t3, 0x2E($s1)
    /* 3B34C 8004AB4C 30002C86 */  lh         $t4, 0x30($s1)
    /* 3B350 8004AB50 32002D86 */  lh         $t5, 0x32($s1)
    /* 3B354 8004AB54 34002E86 */  lh         $t6, 0x34($s1)
    /* 3B358 8004AB58 36002F86 */  lh         $t7, 0x36($s1)
    /* 3B35C 8004AB5C 38003886 */  lh         $t8, 0x38($s1)
    /* 3B360 8004AB60 0740A802 */  srav       $t0, $t0, $s5
    /* 3B364 8004AB64 0748A902 */  srav       $t1, $t1, $s5
    /* 3B368 8004AB68 0750AA02 */  srav       $t2, $t2, $s5
    /* 3B36C 8004AB6C 0100B522 */  addi       $s5, $s5, 0x1 /* handwritten instruction */
    /* 3B370 8004AB70 0758AB02 */  srav       $t3, $t3, $s5
    /* 3B374 8004AB74 0760AC02 */  srav       $t4, $t4, $s5
    /* 3B378 8004AB78 0768AD02 */  srav       $t5, $t5, $s5
    /* 3B37C 8004AB7C FFFFB522 */  addi       $s5, $s5, -0x1 /* handwritten instruction */
    /* 3B380 8004AB80 0770AE02 */  srav       $t6, $t6, $s5
    /* 3B384 8004AB84 0778AF02 */  srav       $t7, $t7, $s5
    /* 3B388 8004AB88 07C0B802 */  srav       $t8, $t8, $s5
    /* 3B38C 8004AB8C FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 3B390 8004AB90 004C0900 */  sll        $t1, $t1, 16
    /* 3B394 8004AB94 FFFF4A31 */  andi       $t2, $t2, 0xFFFF
    /* 3B398 8004AB98 005C0B00 */  sll        $t3, $t3, 16
    /* 3B39C 8004AB9C FFFF8C31 */  andi       $t4, $t4, 0xFFFF
    /* 3B3A0 8004ABA0 006C0D00 */  sll        $t5, $t5, 16
    /* 3B3A4 8004ABA4 FFFFCE31 */  andi       $t6, $t6, 0xFFFF
    /* 3B3A8 8004ABA8 007C0F00 */  sll        $t7, $t7, 16
    /* 3B3AC 8004ABAC 25400901 */  or         $t0, $t0, $t1
    /* 3B3B0 8004ABB0 25504B01 */  or         $t2, $t2, $t3
    /* 3B3B4 8004ABB4 25608D01 */  or         $t4, $t4, $t5
    /* 3B3B8 8004ABB8 2570CF01 */  or         $t6, $t6, $t7
    /* 3B3BC 8004ABBC 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3B3C0 8004ABC0 0008CA48 */  ctc2       $t2, $1 /* handwritten instruction */
    /* 3B3C4 8004ABC4 0010CC48 */  ctc2       $t4, $2 /* handwritten instruction */
    /* 3B3C8 8004ABC8 0018CE48 */  ctc2       $t6, $3 /* handwritten instruction */
    /* 3B3CC 8004ABCC 0020D848 */  ctc2       $t8, $4 /* handwritten instruction */
    /* 3B3D0 8004ABD0 58290108 */  j          .L8004A560
    /* 3B3D4 8004ABD4 00000000 */   nop
  jlabel .L8004ABD8
    /* 3B3D8 8004ABD8 0A80023C */  lui        $v0, %hi(D_800A3708)
    /* 3B3DC 8004ABDC 0837428C */  lw         $v0, %lo(D_800A3708)($v0)
    /* 3B3E0 8004ABE0 2C00488E */  lw         $t0, 0x2C($s2)
    /* 3B3E4 8004ABE4 3000498E */  lw         $t1, 0x30($s2)
    /* 3B3E8 8004ABE8 34004A8E */  lw         $t2, 0x34($s2)
    /* 3B3EC 8004ABEC 2C004B8C */  lw         $t3, 0x2C($v0)
    /* 3B3F0 8004ABF0 30004C8C */  lw         $t4, 0x30($v0)
    /* 3B3F4 8004ABF4 34004D8C */  lw         $t5, 0x34($v0)
    /* 3B3F8 8004ABF8 23400B01 */  subu       $t0, $t0, $t3
    /* 3B3FC 8004ABFC 23482C01 */  subu       $t1, $t1, $t4
    /* 3B400 8004AC00 23504D01 */  subu       $t2, $t2, $t5
    /* 3B404 8004AC04 2A700001 */  slt        $t6, $t0, $zero
    /* 3B408 8004AC08 22700E00 */  neg        $t6, $t6 /* handwritten instruction */
    /* 3B40C 8004AC0C 26580E01 */  xor        $t3, $t0, $t6
    /* 3B410 8004AC10 22586E01 */  sub        $t3, $t3, $t6 /* handwritten instruction */
    /* 3B414 8004AC14 2A702001 */  slt        $t6, $t1, $zero
    /* 3B418 8004AC18 22700E00 */  neg        $t6, $t6 /* handwritten instruction */
    /* 3B41C 8004AC1C 26602E01 */  xor        $t4, $t1, $t6
    /* 3B420 8004AC20 22608E01 */  sub        $t4, $t4, $t6 /* handwritten instruction */
    /* 3B424 8004AC24 2A704001 */  slt        $t6, $t2, $zero
    /* 3B428 8004AC28 22700E00 */  neg        $t6, $t6 /* handwritten instruction */
    /* 3B42C 8004AC2C 26684E01 */  xor        $t5, $t2, $t6
    /* 3B430 8004AC30 2268AE01 */  sub        $t5, $t5, $t6 /* handwritten instruction */
    /* 3B434 8004AC34 25586C01 */  or         $t3, $t3, $t4
    /* 3B438 8004AC38 25586D01 */  or         $t3, $t3, $t5
    /* 3B43C 8004AC3C 03001524 */  addiu      $s5, $zero, 0x3
    /* 3B440 8004AC40 002D6C29 */  slti       $t4, $t3, 0x2D00
    /* 3B444 8004AC44 805A6D29 */  slti       $t5, $t3, 0x5A80
    /* 3B448 8004AC48 04B50134 */  ori        $at, $zero, 0xB504
    /* 3B44C 8004AC4C 2A706101 */  slt        $t6, $t3, $at
    /* 3B450 8004AC50 22A8AE02 */  sub        $s5, $s5, $t6 /* handwritten instruction */
    /* 3B454 8004AC54 22A8AD02 */  sub        $s5, $s5, $t5 /* handwritten instruction */
    /* 3B458 8004AC58 22A8AC02 */  sub        $s5, $s5, $t4 /* handwritten instruction */
    /* 3B45C 8004AC5C 02001524 */  addiu      $s5, $zero, 0x2
    /* 3B460 8004AC60 0740A802 */  srav       $t0, $t0, $s5
    /* 3B464 8004AC64 200028A6 */  sh         $t0, 0x20($s1)
    /* 3B468 8004AC68 0748A902 */  srav       $t1, $t1, $s5
    /* 3B46C 8004AC6C 220029A6 */  sh         $t1, 0x22($s1)
    /* 3B470 8004AC70 0750AA02 */  srav       $t2, $t2, $s5
    /* 3B474 8004AC74 24002AA6 */  sh         $t2, 0x24($s1)
    /* 3B478 8004AC78 0000C88E */  lw         $t0, 0x0($s6)
    /* 3B47C 8004AC7C 0400C98E */  lw         $t1, 0x4($s6)
    /* 3B480 8004AC80 0800CA8E */  lw         $t2, 0x8($s6)
    /* 3B484 8004AC84 0C00CB8E */  lw         $t3, 0xC($s6)
    /* 3B488 8004AC88 1000CC8E */  lw         $t4, 0x10($s6)
    /* 3B48C 8004AC8C 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3B490 8004AC90 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 3B494 8004AC94 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 3B498 8004AC98 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 3B49C 8004AC9C 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 3B4A0 8004ACA0 0028C048 */  ctc2       $zero, $5 /* handwritten instruction */
    /* 3B4A4 8004ACA4 0030C048 */  ctc2       $zero, $6 /* handwritten instruction */
    /* 3B4A8 8004ACA8 0038C048 */  ctc2       $zero, $7 /* handwritten instruction */
    /* 3B4AC 8004ACAC 200020CA */  lwc2       $0, 0x20($s1)
    /* 3B4B0 8004ACB0 240021CA */  lwc2       $1, 0x24($s1)
    /* 3B4B4 8004ACB4 0A004886 */  lh         $t0, 0xA($s2)
    /* 3B4B8 8004ACB8 00000000 */  nop
    /* 3B4BC 8004ACBC 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 3B4C0 8004ACC0 40480800 */  sll        $t1, $t0, 1
    /* 3B4C4 8004ACC4 20482801 */  add        $t1, $t1, $t0 /* handwritten instruction */
    /* 3B4C8 8004ACC8 40490900 */  sll        $t1, $t1, 5
    /* 3B4CC 8004ACCC 0F80083C */  lui        $t0, %hi(D_800F62E0)
    /* 3B4D0 8004ACD0 E0620825 */  addiu      $t0, $t0, %lo(D_800F62E0)
    /* 3B4D4 8004ACD4 21982801 */  addu       $s3, $t1, $t0
    /* 3B4D8 8004ACD8 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 3B4DC 8004ACDC 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 3B4E0 8004ACE0 00580A48 */  mfc2       $t2, $11 /* handwritten instruction */
    /* 3B4E4 8004ACE4 43480900 */  sra        $t1, $t1, 1
    /* 3B4E8 8004ACE8 0028C848 */  ctc2       $t0, $5 /* handwritten instruction */
    /* 3B4EC 8004ACEC 0030C948 */  ctc2       $t1, $6 /* handwritten instruction */
    /* 3B4F0 8004ACF0 0038CA48 */  ctc2       $t2, $7 /* handwritten instruction */
    /* 3B4F4 8004ACF4 1800688E */  lw         $t0, 0x18($s3)
    /* 3B4F8 8004ACF8 1C00698E */  lw         $t1, 0x1C($s3)
    /* 3B4FC 8004ACFC 20006A8E */  lw         $t2, 0x20($s3)
    /* 3B500 8004AD00 24006B8E */  lw         $t3, 0x24($s3)
    /* 3B504 8004AD04 28006C8E */  lw         $t4, 0x28($s3)
    /* 3B508 8004AD08 0040C848 */  ctc2       $t0, $8 /* handwritten instruction */
    /* 3B50C 8004AD0C 0048C948 */  ctc2       $t1, $9 /* handwritten instruction */
    /* 3B510 8004AD10 0050CA48 */  ctc2       $t2, $10 /* handwritten instruction */
    /* 3B514 8004AD14 0058CB48 */  ctc2       $t3, $11 /* handwritten instruction */
    /* 3B518 8004AD18 0060CC48 */  ctc2       $t4, $12 /* handwritten instruction */
    /* 3B51C 8004AD1C 3800688E */  lw         $t0, 0x38($s3)
    /* 3B520 8004AD20 3C00698E */  lw         $t1, 0x3C($s3)
    /* 3B524 8004AD24 40006A8E */  lw         $t2, 0x40($s3)
    /* 3B528 8004AD28 44006B8E */  lw         $t3, 0x44($s3)
    /* 3B52C 8004AD2C 48006C8E */  lw         $t4, 0x48($s3)
    /* 3B530 8004AD30 0080C848 */  ctc2       $t0, $16 /* handwritten instruction */
    /* 3B534 8004AD34 0088C948 */  ctc2       $t1, $17 /* handwritten instruction */
    /* 3B538 8004AD38 0090CA48 */  ctc2       $t2, $18 /* handwritten instruction */
    /* 3B53C 8004AD3C 0098CB48 */  ctc2       $t3, $19 /* handwritten instruction */
    /* 3B540 8004AD40 00A0CC48 */  ctc2       $t4, $20 /* handwritten instruction */
    /* 3B544 8004AD44 5800648E */  lw         $a0, 0x58($s3)
    /* 3B548 8004AD48 F00F0924 */  addiu      $t1, $zero, 0xFF0
    /* 3B54C 8004AD4C 02290400 */  srl        $a1, $a0, 4
    /* 3B550 8004AD50 02330400 */  srl        $a2, $a0, 12
    /* 3B554 8004AD54 00210400 */  sll        $a0, $a0, 4
    /* 3B558 8004AD58 24208900 */  and        $a0, $a0, $t1
    /* 3B55C 8004AD5C 2428A900 */  and        $a1, $a1, $t1
    /* 3B560 8004AD60 2430C900 */  and        $a2, $a2, $t1
    /* 3B564 8004AD64 0068C448 */  ctc2       $a0, $13 /* handwritten instruction */
    /* 3B568 8004AD68 0070C548 */  ctc2       $a1, $14 /* handwritten instruction */
    /* 3B56C 8004AD6C 0078C648 */  ctc2       $a2, $15 /* handwritten instruction */
    /* 3B570 8004AD70 0000C886 */  lh         $t0, 0x0($s6)
    /* 3B574 8004AD74 0200C986 */  lh         $t1, 0x2($s6)
    /* 3B578 8004AD78 0400CA86 */  lh         $t2, 0x4($s6)
    /* 3B57C 8004AD7C 0600CB86 */  lh         $t3, 0x6($s6)
    /* 3B580 8004AD80 0800CC86 */  lh         $t4, 0x8($s6)
    /* 3B584 8004AD84 0A00CD86 */  lh         $t5, 0xA($s6)
    /* 3B588 8004AD88 0C00CE86 */  lh         $t6, 0xC($s6)
    /* 3B58C 8004AD8C 0E00CF86 */  lh         $t7, 0xE($s6)
    /* 3B590 8004AD90 1000D886 */  lh         $t8, 0x10($s6)
    /* 3B594 8004AD94 0740A802 */  srav       $t0, $t0, $s5
    /* 3B598 8004AD98 0748A902 */  srav       $t1, $t1, $s5
    /* 3B59C 8004AD9C 0750AA02 */  srav       $t2, $t2, $s5
    /* 3B5A0 8004ADA0 0100B522 */  addi       $s5, $s5, 0x1 /* handwritten instruction */
    /* 3B5A4 8004ADA4 0758AB02 */  srav       $t3, $t3, $s5
    /* 3B5A8 8004ADA8 0760AC02 */  srav       $t4, $t4, $s5
    /* 3B5AC 8004ADAC 0768AD02 */  srav       $t5, $t5, $s5
    /* 3B5B0 8004ADB0 FFFFB522 */  addi       $s5, $s5, -0x1 /* handwritten instruction */
    /* 3B5B4 8004ADB4 0770AE02 */  srav       $t6, $t6, $s5
    /* 3B5B8 8004ADB8 0778AF02 */  srav       $t7, $t7, $s5
    /* 3B5BC 8004ADBC 07C0B802 */  srav       $t8, $t8, $s5
    /* 3B5C0 8004ADC0 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 3B5C4 8004ADC4 004C0900 */  sll        $t1, $t1, 16
    /* 3B5C8 8004ADC8 FFFF4A31 */  andi       $t2, $t2, 0xFFFF
    /* 3B5CC 8004ADCC 005C0B00 */  sll        $t3, $t3, 16
    /* 3B5D0 8004ADD0 FFFF8C31 */  andi       $t4, $t4, 0xFFFF
    /* 3B5D4 8004ADD4 006C0D00 */  sll        $t5, $t5, 16
    /* 3B5D8 8004ADD8 FFFFCE31 */  andi       $t6, $t6, 0xFFFF
    /* 3B5DC 8004ADDC 007C0F00 */  sll        $t7, $t7, 16
    /* 3B5E0 8004ADE0 25400901 */  or         $t0, $t0, $t1
    /* 3B5E4 8004ADE4 25504B01 */  or         $t2, $t2, $t3
    /* 3B5E8 8004ADE8 25608D01 */  or         $t4, $t4, $t5
    /* 3B5EC 8004ADEC 2570CF01 */  or         $t6, $t6, $t7
    /* 3B5F0 8004ADF0 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3B5F4 8004ADF4 0008CA48 */  ctc2       $t2, $1 /* handwritten instruction */
    /* 3B5F8 8004ADF8 0010CC48 */  ctc2       $t4, $2 /* handwritten instruction */
    /* 3B5FC 8004ADFC 0018CE48 */  ctc2       $t6, $3 /* handwritten instruction */
    /* 3B600 8004AE00 0020D848 */  ctc2       $t8, $4 /* handwritten instruction */
    /* 3B604 8004AE04 58290108 */  j          .L8004A560
    /* 3B608 8004AE08 00000000 */   nop
  jlabel .L8004AE0C
    /* 3B60C 8004AE0C 0C00448E */  lw         $a0, 0xC($s2)
    /* 3B610 8004AE10 28002526 */  addiu      $a1, $s1, 0x28
    /* 3B614 8004AE14 5800468E */  lw         $a2, 0x58($s2)
    /* 3B618 8004AE18 031C010C */  jal        camera_Transform
    /* 3B61C 8004AE1C 18008424 */   addiu     $a0, $a0, 0x18
    /* 3B620 8004AE20 0A80023C */  lui        $v0, %hi(D_800A3708)
    /* 3B624 8004AE24 0837428C */  lw         $v0, %lo(D_800A3708)($v0)
    /* 3B628 8004AE28 3C00288E */  lw         $t0, 0x3C($s1)
    /* 3B62C 8004AE2C 4000298E */  lw         $t1, 0x40($s1)
    /* 3B630 8004AE30 44002A8E */  lw         $t2, 0x44($s1)
    /* 3B634 8004AE34 2C004B8C */  lw         $t3, 0x2C($v0)
    /* 3B638 8004AE38 30004C8C */  lw         $t4, 0x30($v0)
    /* 3B63C 8004AE3C 34004D8C */  lw         $t5, 0x34($v0)
    /* 3B640 8004AE40 23400B01 */  subu       $t0, $t0, $t3
    /* 3B644 8004AE44 23482C01 */  subu       $t1, $t1, $t4
    /* 3B648 8004AE48 23504D01 */  subu       $t2, $t2, $t5
    /* 3B64C 8004AE4C 83400800 */  sra        $t0, $t0, 2
    /* 3B650 8004AE50 83480900 */  sra        $t1, $t1, 2
    /* 3B654 8004AE54 83500A00 */  sra        $t2, $t2, 2
    /* 3B658 8004AE58 200028A6 */  sh         $t0, 0x20($s1)
    /* 3B65C 8004AE5C 220029A6 */  sh         $t1, 0x22($s1)
    /* 3B660 8004AE60 24002AA6 */  sh         $t2, 0x24($s1)
    /* 3B664 8004AE64 0000C88E */  lw         $t0, 0x0($s6)
    /* 3B668 8004AE68 0400C98E */  lw         $t1, 0x4($s6)
    /* 3B66C 8004AE6C 0800CA8E */  lw         $t2, 0x8($s6)
    /* 3B670 8004AE70 0C00CB8E */  lw         $t3, 0xC($s6)
    /* 3B674 8004AE74 1000CC8E */  lw         $t4, 0x10($s6)
    /* 3B678 8004AE78 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3B67C 8004AE7C 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 3B680 8004AE80 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 3B684 8004AE84 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 3B688 8004AE88 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 3B68C 8004AE8C 0028C048 */  ctc2       $zero, $5 /* handwritten instruction */
    /* 3B690 8004AE90 0030C048 */  ctc2       $zero, $6 /* handwritten instruction */
    /* 3B694 8004AE94 0038C048 */  ctc2       $zero, $7 /* handwritten instruction */
    /* 3B698 8004AE98 2000288E */  lw         $t0, 0x20($s1)
    /* 3B69C 8004AE9C 2400298E */  lw         $t1, 0x24($s1)
    /* 3B6A0 8004AEA0 00008848 */  mtc2       $t0, $0 /* handwritten instruction */
    /* 3B6A4 8004AEA4 00088948 */  mtc2       $t1, $1 /* handwritten instruction */
    /* 3B6A8 8004AEA8 00000000 */  nop
    /* 3B6AC 8004AEAC 00000000 */  nop
    /* 3B6B0 8004AEB0 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 3B6B4 8004AEB4 00000000 */  nop
    /* 3B6B8 8004AEB8 00000000 */  nop
    /* 3B6BC 8004AEBC 1400C9EA */  swc2       $9, 0x14($s6)
    /* 3B6C0 8004AEC0 1800CAEA */  swc2       $10, 0x18($s6)
    /* 3B6C4 8004AEC4 1C00CBEA */  swc2       $11, 0x1C($s6)
    /* 3B6C8 8004AEC8 2120C002 */  addu       $a0, $s6, $zero
    /* 3B6CC 8004AECC 17FB010C */  jal        func_8007EC5C
    /* 3B6D0 8004AED0 28002526 */   addiu     $a1, $s1, 0x28
    /* 3B6D4 8004AED4 1400C88E */  lw         $t0, 0x14($s6)
    /* 3B6D8 8004AED8 1800C98E */  lw         $t1, 0x18($s6)
    /* 3B6DC 8004AEDC 1C00CA8E */  lw         $t2, 0x1C($s6)
    /* 3B6E0 8004AEE0 43480900 */  sra        $t1, $t1, 1
    /* 3B6E4 8004AEE4 0028C848 */  ctc2       $t0, $5 /* handwritten instruction */
    /* 3B6E8 8004AEE8 0030C948 */  ctc2       $t1, $6 /* handwritten instruction */
    /* 3B6EC 8004AEEC 0038CA48 */  ctc2       $t2, $7 /* handwritten instruction */
    /* 3B6F0 8004AEF0 28002886 */  lh         $t0, 0x28($s1)
    /* 3B6F4 8004AEF4 2A002986 */  lh         $t1, 0x2A($s1)
    /* 3B6F8 8004AEF8 2C002A86 */  lh         $t2, 0x2C($s1)
    /* 3B6FC 8004AEFC 2E002B86 */  lh         $t3, 0x2E($s1)
    /* 3B700 8004AF00 30002C86 */  lh         $t4, 0x30($s1)
    /* 3B704 8004AF04 32002D86 */  lh         $t5, 0x32($s1)
    /* 3B708 8004AF08 34002E86 */  lh         $t6, 0x34($s1)
    /* 3B70C 8004AF0C 36002F86 */  lh         $t7, 0x36($s1)
    /* 3B710 8004AF10 38003886 */  lh         $t8, 0x38($s1)
    /* 3B714 8004AF14 83400800 */  sra        $t0, $t0, 2
    /* 3B718 8004AF18 83480900 */  sra        $t1, $t1, 2
    /* 3B71C 8004AF1C 83500A00 */  sra        $t2, $t2, 2
    /* 3B720 8004AF20 C3580B00 */  sra        $t3, $t3, 3
    /* 3B724 8004AF24 C3600C00 */  sra        $t4, $t4, 3
    /* 3B728 8004AF28 C3680D00 */  sra        $t5, $t5, 3
    /* 3B72C 8004AF2C 83700E00 */  sra        $t6, $t6, 2
    /* 3B730 8004AF30 83780F00 */  sra        $t7, $t7, 2
    /* 3B734 8004AF34 83C01800 */  sra        $t8, $t8, 2
    /* 3B738 8004AF38 FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 3B73C 8004AF3C 004C0900 */  sll        $t1, $t1, 16
    /* 3B740 8004AF40 FFFF4A31 */  andi       $t2, $t2, 0xFFFF
    /* 3B744 8004AF44 005C0B00 */  sll        $t3, $t3, 16
    /* 3B748 8004AF48 FFFF8C31 */  andi       $t4, $t4, 0xFFFF
    /* 3B74C 8004AF4C 006C0D00 */  sll        $t5, $t5, 16
    /* 3B750 8004AF50 FFFFCE31 */  andi       $t6, $t6, 0xFFFF
    /* 3B754 8004AF54 007C0F00 */  sll        $t7, $t7, 16
    /* 3B758 8004AF58 25400901 */  or         $t0, $t0, $t1
    /* 3B75C 8004AF5C 25504B01 */  or         $t2, $t2, $t3
    /* 3B760 8004AF60 25608D01 */  or         $t4, $t4, $t5
    /* 3B764 8004AF64 2570CF01 */  or         $t6, $t6, $t7
    /* 3B768 8004AF68 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3B76C 8004AF6C 0008CA48 */  ctc2       $t2, $1 /* handwritten instruction */
    /* 3B770 8004AF70 0010CC48 */  ctc2       $t4, $2 /* handwritten instruction */
    /* 3B774 8004AF74 0018CE48 */  ctc2       $t6, $3 /* handwritten instruction */
    /* 3B778 8004AF78 0020D848 */  ctc2       $t8, $4 /* handwritten instruction */
    /* 3B77C 8004AF7C 58290108 */  j          .L8004A560
    /* 3B780 8004AF80 02001524 */   addiu     $s5, $zero, 0x2
  jlabel .L8004AF84
    /* 3B784 8004AF84 0A80023C */  lui        $v0, %hi(D_800A3708)
    /* 3B788 8004AF88 0837428C */  lw         $v0, %lo(D_800A3708)($v0)
    /* 3B78C 8004AF8C 2C00488E */  lw         $t0, 0x2C($s2)
    /* 3B790 8004AF90 3000498E */  lw         $t1, 0x30($s2)
    /* 3B794 8004AF94 34004A8E */  lw         $t2, 0x34($s2)
    /* 3B798 8004AF98 2C004B8C */  lw         $t3, 0x2C($v0)
    /* 3B79C 8004AF9C 30004C8C */  lw         $t4, 0x30($v0)
    /* 3B7A0 8004AFA0 34004D8C */  lw         $t5, 0x34($v0)
    /* 3B7A4 8004AFA4 23400B01 */  subu       $t0, $t0, $t3
    /* 3B7A8 8004AFA8 23482C01 */  subu       $t1, $t1, $t4
    /* 3B7AC 8004AFAC 23504D01 */  subu       $t2, $t2, $t5
    /* 3B7B0 8004AFB0 2A700001 */  slt        $t6, $t0, $zero
    /* 3B7B4 8004AFB4 22700E00 */  neg        $t6, $t6 /* handwritten instruction */
    /* 3B7B8 8004AFB8 26580E01 */  xor        $t3, $t0, $t6
    /* 3B7BC 8004AFBC 22586E01 */  sub        $t3, $t3, $t6 /* handwritten instruction */
    /* 3B7C0 8004AFC0 2A702001 */  slt        $t6, $t1, $zero
    /* 3B7C4 8004AFC4 22700E00 */  neg        $t6, $t6 /* handwritten instruction */
    /* 3B7C8 8004AFC8 26602E01 */  xor        $t4, $t1, $t6
    /* 3B7CC 8004AFCC 22608E01 */  sub        $t4, $t4, $t6 /* handwritten instruction */
    /* 3B7D0 8004AFD0 2A704001 */  slt        $t6, $t2, $zero
    /* 3B7D4 8004AFD4 22700E00 */  neg        $t6, $t6 /* handwritten instruction */
    /* 3B7D8 8004AFD8 26684E01 */  xor        $t5, $t2, $t6
    /* 3B7DC 8004AFDC 2268AE01 */  sub        $t5, $t5, $t6 /* handwritten instruction */
    /* 3B7E0 8004AFE0 25586C01 */  or         $t3, $t3, $t4
    /* 3B7E4 8004AFE4 25586D01 */  or         $t3, $t3, $t5
    /* 3B7E8 8004AFE8 02001524 */  addiu      $s5, $zero, 0x2
    /* 3B7EC 8004AFEC 005A6C29 */  slti       $t4, $t3, 0x5A00
    /* 3B7F0 8004AFF0 00B50134 */  ori        $at, $zero, 0xB500
    /* 3B7F4 8004AFF4 2A686101 */  slt        $t5, $t3, $at
    /* 3B7F8 8004AFF8 22A8AC02 */  sub        $s5, $s5, $t4 /* handwritten instruction */
    /* 3B7FC 8004AFFC 22A8AD02 */  sub        $s5, $s5, $t5 /* handwritten instruction */
    /* 3B800 8004B000 00000000 */  nop
    /* 3B804 8004B004 0A004886 */  lh         $t0, 0xA($s2)
    /* 3B808 8004B008 00000000 */  nop
    /* 3B80C 8004B00C 40480800 */  sll        $t1, $t0, 1
    /* 3B810 8004B010 20482801 */  add        $t1, $t1, $t0 /* handwritten instruction */
    /* 3B814 8004B014 40490900 */  sll        $t1, $t1, 5
    /* 3B818 8004B018 0F80083C */  lui        $t0, %hi(D_800F62E0)
    /* 3B81C 8004B01C E0620825 */  addiu      $t0, $t0, %lo(D_800F62E0)
    /* 3B820 8004B020 21982801 */  addu       $s3, $t1, $t0
    /* 3B824 8004B024 3800688E */  lw         $t0, 0x38($s3)
    /* 3B828 8004B028 3C00698E */  lw         $t1, 0x3C($s3)
    /* 3B82C 8004B02C 40006A8E */  lw         $t2, 0x40($s3)
    /* 3B830 8004B030 44006B8E */  lw         $t3, 0x44($s3)
    /* 3B834 8004B034 48006C8E */  lw         $t4, 0x48($s3)
    /* 3B838 8004B038 0080C848 */  ctc2       $t0, $16 /* handwritten instruction */
    /* 3B83C 8004B03C 0088C948 */  ctc2       $t1, $17 /* handwritten instruction */
    /* 3B840 8004B040 0090CA48 */  ctc2       $t2, $18 /* handwritten instruction */
    /* 3B844 8004B044 0098CB48 */  ctc2       $t3, $19 /* handwritten instruction */
    /* 3B848 8004B048 00A0CC48 */  ctc2       $t4, $20 /* handwritten instruction */
    /* 3B84C 8004B04C 5800648E */  lw         $a0, 0x58($s3)
    /* 3B850 8004B050 F00F0924 */  addiu      $t1, $zero, 0xFF0
    /* 3B854 8004B054 02290400 */  srl        $a1, $a0, 4
    /* 3B858 8004B058 02330400 */  srl        $a2, $a0, 12
    /* 3B85C 8004B05C 00210400 */  sll        $a0, $a0, 4
    /* 3B860 8004B060 24208900 */  and        $a0, $a0, $t1
    /* 3B864 8004B064 2428A900 */  and        $a1, $a1, $t1
    /* 3B868 8004B068 2430C900 */  and        $a2, $a2, $t1
    /* 3B86C 8004B06C 0068C448 */  ctc2       $a0, $13 /* handwritten instruction */
    /* 3B870 8004B070 0070C548 */  ctc2       $a1, $14 /* handwritten instruction */
    /* 3B874 8004B074 0078C648 */  ctc2       $a2, $15 /* handwritten instruction */
    /* 3B878 8004B078 801F013C */  lui        $at, (0x1F800008 >> 16)
    /* 3B87C 8004B07C 080035AC */  sw         $s5, (0x1F800008 & 0xFFFF)($at)
    /* 3B880 8004B080 00000000 */  nop
    /* 3B884 8004B084 04004886 */  lh         $t0, 0x4($s2)
    /* 3B888 8004B088 02004986 */  lh         $t1, 0x2($s2)
    /* 3B88C 8004B08C 80400800 */  sll        $t0, $t0, 2
    /* 3B890 8004B090 10800A3C */  lui        $t2, %hi(D_80103608)
    /* 3B894 8004B094 08364A25 */  addiu      $t2, $t2, %lo(D_80103608)
    /* 3B898 8004B098 21400A01 */  addu       $t0, $t0, $t2
    /* 3B89C 8004B09C 0000088D */  lw         $t0, 0x0($t0)
    /* 3B8A0 8004B0A0 80480900 */  sll        $t1, $t1, 2
    /* 3B8A4 8004B0A4 21400901 */  addu       $t0, $t0, $t1
    /* 3B8A8 8004B0A8 0000058D */  lw         $a1, 0x0($t0)
    /* 3B8AC 8004B0AC 5800448E */  lw         $a0, 0x58($s2)
    /* 3B8B0 8004B0B0 0A004686 */  lh         $a2, 0xA($s2)
    /* 3B8B4 8004B0B4 390C010C */  jal        func_800430E4
    /* 3B8B8 8004B0B8 21384002 */   addu      $a3, $s2, $zero
    /* 3B8BC 8004B0BC 21804000 */  addu       $s0, $v0, $zero
    /* 3B8C0 8004B0C0 74290108 */  j          .L8004A5D0
    /* 3B8C4 8004B0C4 00000000 */   nop
  jlabel .L8004B0C8
    /* 3B8C8 8004B0C8 2120E002 */  addu       $a0, $s7, $zero
    /* 3B8CC 8004B0CC 21284002 */  addu       $a1, $s2, $zero
    /* 3B8D0 8004B0D0 0131010C */  jal        func_8004C404
    /* 3B8D4 8004B0D4 2130C002 */   addu      $a2, $s6, $zero
    /* 3B8D8 8004B0D8 21B84000 */  addu       $s7, $v0, $zero
    /* 3B8DC 8004B0DC 74290108 */  j          .L8004A5D0
    /* 3B8E0 8004B0E0 00000000 */   nop
  jlabel .L8004B0E4
    /* 3B8E4 8004B0E4 0A80023C */  lui        $v0, %hi(D_800A3708)
    /* 3B8E8 8004B0E8 0837428C */  lw         $v0, %lo(D_800A3708)($v0)
    /* 3B8EC 8004B0EC 2C00488E */  lw         $t0, 0x2C($s2)
    /* 3B8F0 8004B0F0 3000498E */  lw         $t1, 0x30($s2)
    /* 3B8F4 8004B0F4 34004A8E */  lw         $t2, 0x34($s2)
    /* 3B8F8 8004B0F8 2C004B8C */  lw         $t3, 0x2C($v0)
    /* 3B8FC 8004B0FC 30004C8C */  lw         $t4, 0x30($v0)
    /* 3B900 8004B100 34004D8C */  lw         $t5, 0x34($v0)
    /* 3B904 8004B104 23400B01 */  subu       $t0, $t0, $t3
    /* 3B908 8004B108 23482C01 */  subu       $t1, $t1, $t4
    /* 3B90C 8004B10C 23504D01 */  subu       $t2, $t2, $t5
    /* 3B910 8004B110 2A700001 */  slt        $t6, $t0, $zero
    /* 3B914 8004B114 22700E00 */  neg        $t6, $t6 /* handwritten instruction */
    /* 3B918 8004B118 26580E01 */  xor        $t3, $t0, $t6
    /* 3B91C 8004B11C 22586E01 */  sub        $t3, $t3, $t6 /* handwritten instruction */
    /* 3B920 8004B120 2A702001 */  slt        $t6, $t1, $zero
    /* 3B924 8004B124 22700E00 */  neg        $t6, $t6 /* handwritten instruction */
    /* 3B928 8004B128 26602E01 */  xor        $t4, $t1, $t6
    /* 3B92C 8004B12C 22608E01 */  sub        $t4, $t4, $t6 /* handwritten instruction */
    /* 3B930 8004B130 2A704001 */  slt        $t6, $t2, $zero
    /* 3B934 8004B134 22700E00 */  neg        $t6, $t6 /* handwritten instruction */
    /* 3B938 8004B138 26684E01 */  xor        $t5, $t2, $t6
    /* 3B93C 8004B13C 2268AE01 */  sub        $t5, $t5, $t6 /* handwritten instruction */
    /* 3B940 8004B140 25586C01 */  or         $t3, $t3, $t4
    /* 3B944 8004B144 25586D01 */  or         $t3, $t3, $t5
    /* 3B948 8004B148 02001524 */  addiu      $s5, $zero, 0x2
    /* 3B94C 8004B14C 004A6C29 */  slti       $t4, $t3, 0x4A00
    /* 3B950 8004B150 00A50134 */  ori        $at, $zero, 0xA500
    /* 3B954 8004B154 2A686101 */  slt        $t5, $t3, $at
    /* 3B958 8004B158 22A8AC02 */  sub        $s5, $s5, $t4 /* handwritten instruction */
    /* 3B95C 8004B15C 22A8AD02 */  sub        $s5, $s5, $t5 /* handwritten instruction */
    /* 3B960 8004B160 00000000 */  nop
    /* 3B964 8004B164 0A004886 */  lh         $t0, 0xA($s2)
    /* 3B968 8004B168 0F800A3C */  lui        $t2, %hi(D_800F62E0)
    /* 3B96C 8004B16C E0624A25 */  addiu      $t2, $t2, %lo(D_800F62E0)
    /* 3B970 8004B170 40480800 */  sll        $t1, $t0, 1
    /* 3B974 8004B174 20482801 */  add        $t1, $t1, $t0 /* handwritten instruction */
    /* 3B978 8004B178 40490900 */  sll        $t1, $t1, 5
    /* 3B97C 8004B17C 21982A01 */  addu       $s3, $t1, $t2
    /* 3B980 8004B180 18006426 */  addiu      $a0, $s3, 0x18
    /* 3B984 8004B184 18004526 */  addiu      $a1, $s2, 0x18
    /* 3B988 8004B188 4C4A010C */  jal        func_80052930
    /* 3B98C 8004B18C 48002626 */   addiu     $a2, $s1, 0x48
    /* 3B990 8004B190 4800288E */  lw         $t0, 0x48($s1)
    /* 3B994 8004B194 4C00298E */  lw         $t1, 0x4C($s1)
    /* 3B998 8004B198 50002A8E */  lw         $t2, 0x50($s1)
    /* 3B99C 8004B19C 54002B8E */  lw         $t3, 0x54($s1)
    /* 3B9A0 8004B1A0 58002C8E */  lw         $t4, 0x58($s1)
    /* 3B9A4 8004B1A4 0040C848 */  ctc2       $t0, $8 /* handwritten instruction */
    /* 3B9A8 8004B1A8 0048C948 */  ctc2       $t1, $9 /* handwritten instruction */
    /* 3B9AC 8004B1AC 0050CA48 */  ctc2       $t2, $10 /* handwritten instruction */
    /* 3B9B0 8004B1B0 0058CB48 */  ctc2       $t3, $11 /* handwritten instruction */
    /* 3B9B4 8004B1B4 0060CC48 */  ctc2       $t4, $12 /* handwritten instruction */
    /* 3B9B8 8004B1B8 3800688E */  lw         $t0, 0x38($s3)
    /* 3B9BC 8004B1BC 3C00698E */  lw         $t1, 0x3C($s3)
    /* 3B9C0 8004B1C0 40006A8E */  lw         $t2, 0x40($s3)
    /* 3B9C4 8004B1C4 44006B8E */  lw         $t3, 0x44($s3)
    /* 3B9C8 8004B1C8 48006C8E */  lw         $t4, 0x48($s3)
    /* 3B9CC 8004B1CC 0080C848 */  ctc2       $t0, $16 /* handwritten instruction */
    /* 3B9D0 8004B1D0 0088C948 */  ctc2       $t1, $17 /* handwritten instruction */
    /* 3B9D4 8004B1D4 0090CA48 */  ctc2       $t2, $18 /* handwritten instruction */
    /* 3B9D8 8004B1D8 0098CB48 */  ctc2       $t3, $19 /* handwritten instruction */
    /* 3B9DC 8004B1DC 00A0CC48 */  ctc2       $t4, $20 /* handwritten instruction */
    /* 3B9E0 8004B1E0 5800648E */  lw         $a0, 0x58($s3)
    /* 3B9E4 8004B1E4 F00F0924 */  addiu      $t1, $zero, 0xFF0
    /* 3B9E8 8004B1E8 02290400 */  srl        $a1, $a0, 4
    /* 3B9EC 8004B1EC 02330400 */  srl        $a2, $a0, 12
    /* 3B9F0 8004B1F0 00210400 */  sll        $a0, $a0, 4
    /* 3B9F4 8004B1F4 24208900 */  and        $a0, $a0, $t1
    /* 3B9F8 8004B1F8 2428A900 */  and        $a1, $a1, $t1
    /* 3B9FC 8004B1FC 2430C900 */  and        $a2, $a2, $t1
    /* 3BA00 8004B200 0068C448 */  ctc2       $a0, $13 /* handwritten instruction */
    /* 3BA04 8004B204 0070C548 */  ctc2       $a1, $14 /* handwritten instruction */
    /* 3BA08 8004B208 0078C648 */  ctc2       $a2, $15 /* handwritten instruction */
    /* 3BA0C 8004B20C 0400CA96 */  lhu        $t2, 0x4($s6)
    /* 3BA10 8004B210 0600CB86 */  lh         $t3, 0x6($s6)
    /* 3BA14 8004B214 0800CC86 */  lh         $t4, 0x8($s6)
    /* 3BA18 8004B218 0A00CD86 */  lh         $t5, 0xA($s6)
    /* 3BA1C 8004B21C 43580B00 */  sra        $t3, $t3, 1
    /* 3BA20 8004B220 43600C00 */  sra        $t4, $t4, 1
    /* 3BA24 8004B224 43680D00 */  sra        $t5, $t5, 1
    /* 3BA28 8004B228 0000C88E */  lw         $t0, 0x0($s6)
    /* 3BA2C 8004B22C 005C0B00 */  sll        $t3, $t3, 16
    /* 3BA30 8004B230 FFFF8C31 */  andi       $t4, $t4, 0xFFFF
    /* 3BA34 8004B234 006C0D00 */  sll        $t5, $t5, 16
    /* 3BA38 8004B238 25504B01 */  or         $t2, $t2, $t3
    /* 3BA3C 8004B23C 25608D01 */  or         $t4, $t4, $t5
    /* 3BA40 8004B240 0C00CE8E */  lw         $t6, 0xC($s6)
    /* 3BA44 8004B244 1000D88E */  lw         $t8, 0x10($s6)
    /* 3BA48 8004B248 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3BA4C 8004B24C 0008CA48 */  ctc2       $t2, $1 /* handwritten instruction */
    /* 3BA50 8004B250 0010CC48 */  ctc2       $t4, $2 /* handwritten instruction */
    /* 3BA54 8004B254 0018CE48 */  ctc2       $t6, $3 /* handwritten instruction */
    /* 3BA58 8004B258 0020D848 */  ctc2       $t8, $4 /* handwritten instruction */
    /* 3BA5C 8004B25C 0028C048 */  ctc2       $zero, $5 /* handwritten instruction */
    /* 3BA60 8004B260 0030C048 */  ctc2       $zero, $6 /* handwritten instruction */
    /* 3BA64 8004B264 0038C048 */  ctc2       $zero, $7 /* handwritten instruction */
    /* 3BA68 8004B268 2128A002 */  addu       $a1, $s5, $zero
    /* 3BA6C 8004B26C 9D36010C */  jal        func_8004DA74
    /* 3BA70 8004B270 21204002 */   addu      $a0, $s2, $zero
    /* 3BA74 8004B274 74290108 */  j          .L8004A5D0
    /* 3BA78 8004B278 00000000 */   nop
  jlabel .L8004B27C
    /* 3BA7C 8004B27C 1080023C */  lui        $v0, %hi(D_800FF558)
    /* 3BA80 8004B280 58F54224 */  addiu      $v0, $v0, %lo(D_800FF558)
    /* 3BA84 8004B284 2C00488E */  lw         $t0, 0x2C($s2)
    /* 3BA88 8004B288 3000498E */  lw         $t1, 0x30($s2)
    /* 3BA8C 8004B28C 34004A8E */  lw         $t2, 0x34($s2)
    /* 3BA90 8004B290 14004B8C */  lw         $t3, 0x14($v0)
    /* 3BA94 8004B294 18004C8C */  lw         $t4, 0x18($v0)
    /* 3BA98 8004B298 1C004D8C */  lw         $t5, 0x1C($v0)
    /* 3BA9C 8004B29C 23400B01 */  subu       $t0, $t0, $t3
    /* 3BAA0 8004B2A0 23482C01 */  subu       $t1, $t1, $t4
    /* 3BAA4 8004B2A4 23504D01 */  subu       $t2, $t2, $t5
    /* 3BAA8 8004B2A8 2A700001 */  slt        $t6, $t0, $zero
    /* 3BAAC 8004B2AC 22700E00 */  neg        $t6, $t6 /* handwritten instruction */
    /* 3BAB0 8004B2B0 26580E01 */  xor        $t3, $t0, $t6
    /* 3BAB4 8004B2B4 22586E01 */  sub        $t3, $t3, $t6 /* handwritten instruction */
    /* 3BAB8 8004B2B8 2A702001 */  slt        $t6, $t1, $zero
    /* 3BABC 8004B2BC 22700E00 */  neg        $t6, $t6 /* handwritten instruction */
    /* 3BAC0 8004B2C0 26602E01 */  xor        $t4, $t1, $t6
    /* 3BAC4 8004B2C4 22608E01 */  sub        $t4, $t4, $t6 /* handwritten instruction */
    /* 3BAC8 8004B2C8 2A704001 */  slt        $t6, $t2, $zero
    /* 3BACC 8004B2CC 22700E00 */  neg        $t6, $t6 /* handwritten instruction */
    /* 3BAD0 8004B2D0 26684E01 */  xor        $t5, $t2, $t6
    /* 3BAD4 8004B2D4 2268AE01 */  sub        $t5, $t5, $t6 /* handwritten instruction */
    /* 3BAD8 8004B2D8 25586C01 */  or         $t3, $t3, $t4
    /* 3BADC 8004B2DC 25586D01 */  or         $t3, $t3, $t5
    /* 3BAE0 8004B2E0 02001524 */  addiu      $s5, $zero, 0x2
    /* 3BAE4 8004B2E4 004A6C29 */  slti       $t4, $t3, 0x4A00
    /* 3BAE8 8004B2E8 00A50134 */  ori        $at, $zero, 0xA500
    /* 3BAEC 8004B2EC 2A686101 */  slt        $t5, $t3, $at
    /* 3BAF0 8004B2F0 22A8AC02 */  sub        $s5, $s5, $t4 /* handwritten instruction */
    /* 3BAF4 8004B2F4 22A8AD02 */  sub        $s5, $s5, $t5 /* handwritten instruction */
    /* 3BAF8 8004B2F8 00000000 */  nop
    /* 3BAFC 8004B2FC 01001524 */  addiu      $s5, $zero, 0x1
    /* 3BB00 8004B300 0740A802 */  srav       $t0, $t0, $s5
    /* 3BB04 8004B304 200028A6 */  sh         $t0, 0x20($s1)
    /* 3BB08 8004B308 0748A902 */  srav       $t1, $t1, $s5
    /* 3BB0C 8004B30C 220029A6 */  sh         $t1, 0x22($s1)
    /* 3BB10 8004B310 0750AA02 */  srav       $t2, $t2, $s5
    /* 3BB14 8004B314 24002AA6 */  sh         $t2, 0x24($s1)
    /* 3BB18 8004B318 0A004886 */  lh         $t0, 0xA($s2)
    /* 3BB1C 8004B31C 0F800A3C */  lui        $t2, %hi(D_800F62E0)
    /* 3BB20 8004B320 E0624A25 */  addiu      $t2, $t2, %lo(D_800F62E0)
    /* 3BB24 8004B324 40480800 */  sll        $t1, $t0, 1
    /* 3BB28 8004B328 20482801 */  add        $t1, $t1, $t0 /* handwritten instruction */
    /* 3BB2C 8004B32C 40490900 */  sll        $t1, $t1, 5
    /* 3BB30 8004B330 21982A01 */  addu       $s3, $t1, $t2
    /* 3BB34 8004B334 18006426 */  addiu      $a0, $s3, 0x18
    /* 3BB38 8004B338 18004526 */  addiu      $a1, $s2, 0x18
    /* 3BB3C 8004B33C 4C4A010C */  jal        func_80052930
    /* 3BB40 8004B340 48002626 */   addiu     $a2, $s1, 0x48
    /* 3BB44 8004B344 4800288E */  lw         $t0, 0x48($s1)
    /* 3BB48 8004B348 4C00298E */  lw         $t1, 0x4C($s1)
    /* 3BB4C 8004B34C 50002A8E */  lw         $t2, 0x50($s1)
    /* 3BB50 8004B350 54002B8E */  lw         $t3, 0x54($s1)
    /* 3BB54 8004B354 58002C8E */  lw         $t4, 0x58($s1)
    /* 3BB58 8004B358 0040C848 */  ctc2       $t0, $8 /* handwritten instruction */
    /* 3BB5C 8004B35C 0048C948 */  ctc2       $t1, $9 /* handwritten instruction */
    /* 3BB60 8004B360 0050CA48 */  ctc2       $t2, $10 /* handwritten instruction */
    /* 3BB64 8004B364 0058CB48 */  ctc2       $t3, $11 /* handwritten instruction */
    /* 3BB68 8004B368 0060CC48 */  ctc2       $t4, $12 /* handwritten instruction */
    /* 3BB6C 8004B36C 3800688E */  lw         $t0, 0x38($s3)
    /* 3BB70 8004B370 3C00698E */  lw         $t1, 0x3C($s3)
    /* 3BB74 8004B374 40006A8E */  lw         $t2, 0x40($s3)
    /* 3BB78 8004B378 44006B8E */  lw         $t3, 0x44($s3)
    /* 3BB7C 8004B37C 48006C8E */  lw         $t4, 0x48($s3)
    /* 3BB80 8004B380 0080C848 */  ctc2       $t0, $16 /* handwritten instruction */
    /* 3BB84 8004B384 0088C948 */  ctc2       $t1, $17 /* handwritten instruction */
    /* 3BB88 8004B388 0090CA48 */  ctc2       $t2, $18 /* handwritten instruction */
    /* 3BB8C 8004B38C 0098CB48 */  ctc2       $t3, $19 /* handwritten instruction */
    /* 3BB90 8004B390 00A0CC48 */  ctc2       $t4, $20 /* handwritten instruction */
    /* 3BB94 8004B394 1080043C */  lui        $a0, %hi(D_800FF558)
    /* 3BB98 8004B398 58F58424 */  addiu      $a0, $a0, %lo(D_800FF558)
    /* 3BB9C 8004B39C 18004526 */  addiu      $a1, $s2, 0x18
    /* 3BBA0 8004B3A0 4C4A010C */  jal        func_80052930
    /* 3BBA4 8004B3A4 28002626 */   addiu     $a2, $s1, 0x28
    /* 3BBA8 8004B3A8 1080023C */  lui        $v0, %hi(D_800FF558)
    /* 3BBAC 8004B3AC 58F54224 */  addiu      $v0, $v0, %lo(D_800FF558)
    /* 3BBB0 8004B3B0 00000000 */  nop
    /* 3BBB4 8004B3B4 0000488C */  lw         $t0, 0x0($v0)
    /* 3BBB8 8004B3B8 0400498C */  lw         $t1, 0x4($v0)
    /* 3BBBC 8004B3BC 08004A8C */  lw         $t2, 0x8($v0)
    /* 3BBC0 8004B3C0 0C004B8C */  lw         $t3, 0xC($v0)
    /* 3BBC4 8004B3C4 10004C8C */  lw         $t4, 0x10($v0)
    /* 3BBC8 8004B3C8 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3BBCC 8004B3CC 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 3BBD0 8004B3D0 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 3BBD4 8004B3D4 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 3BBD8 8004B3D8 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 3BBDC 8004B3DC 0028C048 */  ctc2       $zero, $5 /* handwritten instruction */
    /* 3BBE0 8004B3E0 0030C048 */  ctc2       $zero, $6 /* handwritten instruction */
    /* 3BBE4 8004B3E4 0038C048 */  ctc2       $zero, $7 /* handwritten instruction */
    /* 3BBE8 8004B3E8 200020CA */  lwc2       $0, 0x20($s1)
    /* 3BBEC 8004B3EC 240021CA */  lwc2       $1, 0x24($s1)
    /* 3BBF0 8004B3F0 5800648E */  lw         $a0, 0x58($s3)
    /* 3BBF4 8004B3F4 F00F0924 */  addiu      $t1, $zero, 0xFF0
    /* 3BBF8 8004B3F8 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 3BBFC 8004B3FC 02290400 */  srl        $a1, $a0, 4
    /* 3BC00 8004B400 02330400 */  srl        $a2, $a0, 12
    /* 3BC04 8004B404 00210400 */  sll        $a0, $a0, 4
    /* 3BC08 8004B408 24208900 */  and        $a0, $a0, $t1
    /* 3BC0C 8004B40C 2428A900 */  and        $a1, $a1, $t1
    /* 3BC10 8004B410 2430C900 */  and        $a2, $a2, $t1
    /* 3BC14 8004B414 0068C448 */  ctc2       $a0, $13 /* handwritten instruction */
    /* 3BC18 8004B418 0070C548 */  ctc2       $a1, $14 /* handwritten instruction */
    /* 3BC1C 8004B41C 0078C648 */  ctc2       $a2, $15 /* handwritten instruction */
    /* 3BC20 8004B420 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 3BC24 8004B424 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 3BC28 8004B428 00580A48 */  mfc2       $t2, $11 /* handwritten instruction */
    /* 3BC2C 8004B42C 43480900 */  sra        $t1, $t1, 1
    /* 3BC30 8004B430 0028C848 */  ctc2       $t0, $5 /* handwritten instruction */
    /* 3BC34 8004B434 0030C948 */  ctc2       $t1, $6 /* handwritten instruction */
    /* 3BC38 8004B438 0038CA48 */  ctc2       $t2, $7 /* handwritten instruction */
    /* 3BC3C 8004B43C 28002886 */  lh         $t0, 0x28($s1)
    /* 3BC40 8004B440 2A002986 */  lh         $t1, 0x2A($s1)
    /* 3BC44 8004B444 2C002A86 */  lh         $t2, 0x2C($s1)
    /* 3BC48 8004B448 2E002B86 */  lh         $t3, 0x2E($s1)
    /* 3BC4C 8004B44C 30002C86 */  lh         $t4, 0x30($s1)
    /* 3BC50 8004B450 32002D86 */  lh         $t5, 0x32($s1)
    /* 3BC54 8004B454 34002E86 */  lh         $t6, 0x34($s1)
    /* 3BC58 8004B458 36002F86 */  lh         $t7, 0x36($s1)
    /* 3BC5C 8004B45C 38003886 */  lh         $t8, 0x38($s1)
    /* 3BC60 8004B460 0740A802 */  srav       $t0, $t0, $s5
    /* 3BC64 8004B464 0748A902 */  srav       $t1, $t1, $s5
    /* 3BC68 8004B468 0750AA02 */  srav       $t2, $t2, $s5
    /* 3BC6C 8004B46C 0100B522 */  addi       $s5, $s5, 0x1 /* handwritten instruction */
    /* 3BC70 8004B470 0758AB02 */  srav       $t3, $t3, $s5
    /* 3BC74 8004B474 0760AC02 */  srav       $t4, $t4, $s5
    /* 3BC78 8004B478 0768AD02 */  srav       $t5, $t5, $s5
    /* 3BC7C 8004B47C FFFFB522 */  addi       $s5, $s5, -0x1 /* handwritten instruction */
    /* 3BC80 8004B480 0770AE02 */  srav       $t6, $t6, $s5
    /* 3BC84 8004B484 0778AF02 */  srav       $t7, $t7, $s5
    /* 3BC88 8004B488 07C0B802 */  srav       $t8, $t8, $s5
    /* 3BC8C 8004B48C FFFF0831 */  andi       $t0, $t0, 0xFFFF
    /* 3BC90 8004B490 004C0900 */  sll        $t1, $t1, 16
    /* 3BC94 8004B494 FFFF4A31 */  andi       $t2, $t2, 0xFFFF
    /* 3BC98 8004B498 005C0B00 */  sll        $t3, $t3, 16
    /* 3BC9C 8004B49C FFFF8C31 */  andi       $t4, $t4, 0xFFFF
    /* 3BCA0 8004B4A0 006C0D00 */  sll        $t5, $t5, 16
    /* 3BCA4 8004B4A4 FFFFCE31 */  andi       $t6, $t6, 0xFFFF
    /* 3BCA8 8004B4A8 007C0F00 */  sll        $t7, $t7, 16
    /* 3BCAC 8004B4AC 25400901 */  or         $t0, $t0, $t1
    /* 3BCB0 8004B4B0 25504B01 */  or         $t2, $t2, $t3
    /* 3BCB4 8004B4B4 25608D01 */  or         $t4, $t4, $t5
    /* 3BCB8 8004B4B8 2570CF01 */  or         $t6, $t6, $t7
    /* 3BCBC 8004B4BC 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3BCC0 8004B4C0 0008CA48 */  ctc2       $t2, $1 /* handwritten instruction */
    /* 3BCC4 8004B4C4 0010CC48 */  ctc2       $t4, $2 /* handwritten instruction */
    /* 3BCC8 8004B4C8 0018CE48 */  ctc2       $t6, $3 /* handwritten instruction */
    /* 3BCCC 8004B4CC 0020D848 */  ctc2       $t8, $4 /* handwritten instruction */
    /* 3BCD0 8004B4D0 58290108 */  j          .L8004A560
    /* 3BCD4 8004B4D4 00000000 */   nop
  jlabel .L8004B4D8
    /* 3BCD8 8004B4D8 0A80023C */  lui        $v0, %hi(D_800A3708)
    /* 3BCDC 8004B4DC 0837428C */  lw         $v0, %lo(D_800A3708)($v0)
    /* 3BCE0 8004B4E0 2C00488E */  lw         $t0, 0x2C($s2)
    /* 3BCE4 8004B4E4 3000498E */  lw         $t1, 0x30($s2)
    /* 3BCE8 8004B4E8 34004A8E */  lw         $t2, 0x34($s2)
    /* 3BCEC 8004B4EC 2C004B8C */  lw         $t3, 0x2C($v0)
    /* 3BCF0 8004B4F0 30004C8C */  lw         $t4, 0x30($v0)
    /* 3BCF4 8004B4F4 34004D8C */  lw         $t5, 0x34($v0)
    /* 3BCF8 8004B4F8 23400B01 */  subu       $t0, $t0, $t3
    /* 3BCFC 8004B4FC 23482C01 */  subu       $t1, $t1, $t4
    /* 3BD00 8004B500 23504D01 */  subu       $t2, $t2, $t5
    /* 3BD04 8004B504 0F80153C */  lui        $s5, %hi(D_800F6658)
    /* 3BD08 8004B508 5866B586 */  lh         $s5, %lo(D_800F6658)($s5)
    /* 3BD0C 8004B50C 00000000 */  nop
    /* 3BD10 8004B510 0740A802 */  srav       $t0, $t0, $s5
    /* 3BD14 8004B514 200028A6 */  sh         $t0, 0x20($s1)
    /* 3BD18 8004B518 0748A902 */  srav       $t1, $t1, $s5
    /* 3BD1C 8004B51C 220029A6 */  sh         $t1, 0x22($s1)
    /* 3BD20 8004B520 0750AA02 */  srav       $t2, $t2, $s5
    /* 3BD24 8004B524 24002AA6 */  sh         $t2, 0x24($s1)
    /* 3BD28 8004B528 0000C88E */  lw         $t0, 0x0($s6)
    /* 3BD2C 8004B52C 0400C98E */  lw         $t1, 0x4($s6)
    /* 3BD30 8004B530 0800CA8E */  lw         $t2, 0x8($s6)
    /* 3BD34 8004B534 0C00CB8E */  lw         $t3, 0xC($s6)
    /* 3BD38 8004B538 1000CC8E */  lw         $t4, 0x10($s6)
    /* 3BD3C 8004B53C 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3BD40 8004B540 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 3BD44 8004B544 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 3BD48 8004B548 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 3BD4C 8004B54C 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 3BD50 8004B550 0028C048 */  ctc2       $zero, $5 /* handwritten instruction */
    /* 3BD54 8004B554 0030C048 */  ctc2       $zero, $6 /* handwritten instruction */
    /* 3BD58 8004B558 0038C048 */  ctc2       $zero, $7 /* handwritten instruction */
    /* 3BD5C 8004B55C 200020CA */  lwc2       $0, 0x20($s1)
    /* 3BD60 8004B560 240021CA */  lwc2       $1, 0x24($s1)
    /* 3BD64 8004B564 0A004886 */  lh         $t0, 0xA($s2)
    /* 3BD68 8004B568 00000000 */  nop
    /* 3BD6C 8004B56C 1200484A */  mvmva      1, 0, 0, 0, 0
    /* 3BD70 8004B570 40480800 */  sll        $t1, $t0, 1
    /* 3BD74 8004B574 20482801 */  add        $t1, $t1, $t0 /* handwritten instruction */
    /* 3BD78 8004B578 40490900 */  sll        $t1, $t1, 5
    /* 3BD7C 8004B57C 0F800A3C */  lui        $t2, %hi(D_800F62E0)
    /* 3BD80 8004B580 E0624A25 */  addiu      $t2, $t2, %lo(D_800F62E0)
    /* 3BD84 8004B584 21982A01 */  addu       $s3, $t1, $t2
    /* 3BD88 8004B588 00480848 */  mfc2       $t0, $9 /* handwritten instruction */
    /* 3BD8C 8004B58C 00500948 */  mfc2       $t1, $10 /* handwritten instruction */
    /* 3BD90 8004B590 00580A48 */  mfc2       $t2, $11 /* handwritten instruction */
    /* 3BD94 8004B594 43480900 */  sra        $t1, $t1, 1
    /* 3BD98 8004B598 0028C848 */  ctc2       $t0, $5 /* handwritten instruction */
    /* 3BD9C 8004B59C 0030C948 */  ctc2       $t1, $6 /* handwritten instruction */
    /* 3BDA0 8004B5A0 0038CA48 */  ctc2       $t2, $7 /* handwritten instruction */
    /* 3BDA4 8004B5A4 1800688E */  lw         $t0, 0x18($s3)
    /* 3BDA8 8004B5A8 1C00698E */  lw         $t1, 0x1C($s3)
    /* 3BDAC 8004B5AC 20006A8E */  lw         $t2, 0x20($s3)
    /* 3BDB0 8004B5B0 24006B8E */  lw         $t3, 0x24($s3)
    /* 3BDB4 8004B5B4 28006C8E */  lw         $t4, 0x28($s3)
    /* 3BDB8 8004B5B8 0040C848 */  ctc2       $t0, $8 /* handwritten instruction */
    /* 3BDBC 8004B5BC 0048C948 */  ctc2       $t1, $9 /* handwritten instruction */
    /* 3BDC0 8004B5C0 0050CA48 */  ctc2       $t2, $10 /* handwritten instruction */
    /* 3BDC4 8004B5C4 0058CB48 */  ctc2       $t3, $11 /* handwritten instruction */
    /* 3BDC8 8004B5C8 0060CC48 */  ctc2       $t4, $12 /* handwritten instruction */
    /* 3BDCC 8004B5CC 3800688E */  lw         $t0, 0x38($s3)
    /* 3BDD0 8004B5D0 3C00698E */  lw         $t1, 0x3C($s3)
    /* 3BDD4 8004B5D4 40006A8E */  lw         $t2, 0x40($s3)
    /* 3BDD8 8004B5D8 44006B8E */  lw         $t3, 0x44($s3)
    /* 3BDDC 8004B5DC 48006C8E */  lw         $t4, 0x48($s3)
    /* 3BDE0 8004B5E0 0080C848 */  ctc2       $t0, $16 /* handwritten instruction */
    /* 3BDE4 8004B5E4 0088C948 */  ctc2       $t1, $17 /* handwritten instruction */
    /* 3BDE8 8004B5E8 0090CA48 */  ctc2       $t2, $18 /* handwritten instruction */
    /* 3BDEC 8004B5EC 0098CB48 */  ctc2       $t3, $19 /* handwritten instruction */
    /* 3BDF0 8004B5F0 00A0CC48 */  ctc2       $t4, $20 /* handwritten instruction */
    /* 3BDF4 8004B5F4 5800648E */  lw         $a0, 0x58($s3)
    /* 3BDF8 8004B5F8 F00F0924 */  addiu      $t1, $zero, 0xFF0
    /* 3BDFC 8004B5FC 02290400 */  srl        $a1, $a0, 4
    /* 3BE00 8004B600 02330400 */  srl        $a2, $a0, 12
    /* 3BE04 8004B604 00210400 */  sll        $a0, $a0, 4
    /* 3BE08 8004B608 24208900 */  and        $a0, $a0, $t1
    /* 3BE0C 8004B60C 2428A900 */  and        $a1, $a1, $t1
    /* 3BE10 8004B610 2430C900 */  and        $a2, $a2, $t1
    /* 3BE14 8004B614 0068C448 */  ctc2       $a0, $13 /* handwritten instruction */
    /* 3BE18 8004B618 0070C548 */  ctc2       $a1, $14 /* handwritten instruction */
    /* 3BE1C 8004B61C 0078C648 */  ctc2       $a2, $15 /* handwritten instruction */
    /* 3BE20 8004B620 0000C886 */  lh         $t0, 0x0($s6)
    /* 3BE24 8004B624 0200C986 */  lh         $t1, 0x2($s6)
    /* 3BE28 8004B628 0400CA86 */  lh         $t2, 0x4($s6)
    /* 3BE2C 8004B62C 0600CB86 */  lh         $t3, 0x6($s6)
    /* 3BE30 8004B630 0800CC86 */  lh         $t4, 0x8($s6)
    /* 3BE34 8004B634 0A00CD86 */  lh         $t5, 0xA($s6)
    /* 3BE38 8004B638 0C00CE86 */  lh         $t6, 0xC($s6)
    /* 3BE3C 8004B63C 0E00CF86 */  lh         $t7, 0xE($s6)
    /* 3BE40 8004B640 1000D886 */  lh         $t8, 0x10($s6)
    /* 3BE44 8004B644 0740A802 */  srav       $t0, $t0, $s5
    /* 3BE48 8004B648 0748A902 */  srav       $t1, $t1, $s5
    /* 3BE4C 8004B64C 0750AA02 */  srav       $t2, $t2, $s5
    /* 3BE50 8004B650 0100B522 */  addi       $s5, $s5, 0x1 /* handwritten instruction */
    /* 3BE54 8004B654 0758AB02 */  srav       $t3, $t3, $s5
    /* 3BE58 8004B658 0760AC02 */  srav       $t4, $t4, $s5
    /* 3BE5C 8004B65C 0768AD02 */  srav       $t5, $t5, $s5
    /* 3BE60 8004B660 FFFFB522 */  addi       $s5, $s5, -0x1 /* handwritten instruction */
    /* 3BE64 8004B664 0770AE02 */  srav       $t6, $t6, $s5
    /* 3BE68 8004B668 0778AF02 */  srav       $t7, $t7, $s5
    /* 3BE6C 8004B66C 07C0B802 */  srav       $t8, $t8, $s5
    /* 3BE70 8004B670 280028A6 */  sh         $t0, 0x28($s1)
    /* 3BE74 8004B674 2A0029A6 */  sh         $t1, 0x2A($s1)
    /* 3BE78 8004B678 2C002AA6 */  sh         $t2, 0x2C($s1)
    /* 3BE7C 8004B67C 2E002BA6 */  sh         $t3, 0x2E($s1)
    /* 3BE80 8004B680 30002CA6 */  sh         $t4, 0x30($s1)
    /* 3BE84 8004B684 32002DA6 */  sh         $t5, 0x32($s1)
    /* 3BE88 8004B688 34002EA6 */  sh         $t6, 0x34($s1)
    /* 3BE8C 8004B68C 36002FA6 */  sh         $t7, 0x36($s1)
    /* 3BE90 8004B690 380038A6 */  sh         $t8, 0x38($s1)
    /* 3BE94 8004B694 2800288E */  lw         $t0, 0x28($s1)
    /* 3BE98 8004B698 2C00298E */  lw         $t1, 0x2C($s1)
    /* 3BE9C 8004B69C 30002A8E */  lw         $t2, 0x30($s1)
    /* 3BEA0 8004B6A0 34002B8E */  lw         $t3, 0x34($s1)
    /* 3BEA4 8004B6A4 38002C8E */  lw         $t4, 0x38($s1)
    /* 3BEA8 8004B6A8 0000C848 */  ctc2       $t0, $0 /* handwritten instruction */
    /* 3BEAC 8004B6AC 0008C948 */  ctc2       $t1, $1 /* handwritten instruction */
    /* 3BEB0 8004B6B0 0010CA48 */  ctc2       $t2, $2 /* handwritten instruction */
    /* 3BEB4 8004B6B4 0018CB48 */  ctc2       $t3, $3 /* handwritten instruction */
    /* 3BEB8 8004B6B8 0020CC48 */  ctc2       $t4, $4 /* handwritten instruction */
    /* 3BEBC 8004B6BC 0A80053C */  lui        $a1, %hi(D_800A3790)
    /* 3BEC0 8004B6C0 9037A58C */  lw         $a1, %lo(D_800A3790)($a1)
    /* 3BEC4 8004B6C4 00000000 */  nop
    /* 3BEC8 8004B6C8 C00125AE */  sw         $a1, 0x1C0($s1)
    /* 3BECC 8004B6CC F81E010C */  jal        func_80047BE0
    /* 3BED0 8004B6D0 21204002 */   addu      $a0, $s2, $zero
    /* 3BED4 8004B6D4 74290108 */  j          .L8004A5D0
    /* 3BED8 8004B6D8 00000000 */   nop
    /* 3BEDC 8004B6DC 58290108 */  j          .L8004A560
    /* 3BEE0 8004B6E0 00000000 */   nop
    /* 3BEE4 8004B6E4 801F083C */  lui        $t0, (0x1F8001D4 >> 16)
    /* 3BEE8 8004B6E8 E8011FAD */  sw         $ra, (0x1F8001E8 & 0xFFFF)($t0)
    /* 3BEEC 8004B6EC EC011EAD */  sw         $fp, (0x1F8001EC & 0xFFFF)($t0)
    /* 3BEF0 8004B6F0 F00117AD */  sw         $s7, (0x1F8001F0 & 0xFFFF)($t0)
    /* 3BEF4 8004B6F4 F40116AD */  sw         $s6, (0x1F8001F4 & 0xFFFF)($t0)
    /* 3BEF8 8004B6F8 F80115AD */  sw         $s5, (0x1F8001F8 & 0xFFFF)($t0)
    /* 3BEFC 8004B6FC FC0114AD */  sw         $s4, (0x1F8001FC & 0xFFFF)($t0)
    /* 3BF00 8004B700 000213AD */  sw         $s3, (0x1F800200 & 0xFFFF)($t0)
    /* 3BF04 8004B704 040212AD */  sw         $s2, (0x1F800204 & 0xFFFF)($t0)
    /* 3BF08 8004B708 080211AD */  sw         $s1, (0x1F800208 & 0xFFFF)($t0)
    /* 3BF0C 8004B70C 0C0210AD */  sw         $s0, (0x1F80020C & 0xFFFF)($t0)
    /* 3BF10 8004B710 21B80001 */  addu       $s7, $t0, $zero
    /* 3BF14 8004B714 E001E7A6 */  sh         $a3, (0x1F8001E0 & 0xFFFF)($s7)
    /* 3BF18 8004B718 D801E5AE */  sw         $a1, (0x1F8001D8 & 0xFFFF)($s7)
    /* 3BF1C 8004B71C 2180C000 */  addu       $s0, $a2, $zero
    /* 3BF20 8004B720 0C00F28E */  lw         $s2, (0x1F80000C & 0xFFFF)($s7)
    /* 3BF24 8004B724 0A80143C */  lui        $s4, %hi(D_800A38B4)
    /* 3BF28 8004B728 B438948E */  lw         $s4, %lo(D_800A38B4)($s4)
    /* 3BF2C 8004B72C 2000F522 */  addi       $s5, $s7, (0x1F800020 & 0xFFFF) /* handwritten instruction */
    /* 3BF30 8004B730 1001F622 */  addi       $s6, $s7, (0x1F800110 & 0xFFFF) /* handwritten instruction */
    /* 3BF34 8004B734 1002F322 */  addi       $s3, $s7, (0x1F800210 & 0xFFFF) /* handwritten instruction */
    /* 3BF38 8004B738 FF001E3C */  lui        $fp, (0xFFFFFF >> 16)
    /* 3BF3C 8004B73C FFFFDE37 */  ori        $fp, $fp, (0xFFFFFF & 0xFFFF)
    /* 3BF40 8004B740 FFFF8420 */  addi       $a0, $a0, -0x1 /* handwritten instruction */
  .L8004B744:
    /* 3BF44 8004B744 E101E982 */  lb         $t1, (0x1F8001E1 & 0xFFFF)($s7)
    /* 3BF48 8004B748 0000088E */  lw         $t0, 0x0($s0)
    /* 3BF4C 8004B74C 01002931 */  andi       $t1, $t1, 0x1
    /* 3BF50 8004B750 404E0900 */  sll        $t1, $t1, 25
    /* 3BF54 8004B754 25400901 */  or         $t0, $t0, $t1
    /* 3BF58 8004B758 00308848 */  mtc2       $t0, $6 /* handwritten instruction */
    /* 3BF5C 8004B75C 0C00028E */  lw         $v0, 0xC($s0)
    /* 3BF60 8004B760 D401E4AE */  sw         $a0, (0x1F8001D4 & 0xFFFF)($s7)
    /* 3BF64 8004B764 80600200 */  sll        $t4, $v0, 2
    /* 3BF68 8004B768 FC038C31 */  andi       $t4, $t4, 0x3FC
    /* 3BF6C 8004B76C 21409501 */  addu       $t0, $t4, $s5
    /* 3BF70 8004B770 0000088D */  lw         $t0, 0x0($t0)
    /* 3BF74 8004B774 82690200 */  srl        $t5, $v0, 6
    /* 3BF78 8004B778 FC03AD31 */  andi       $t5, $t5, 0x3FC
    /* 3BF7C 8004B77C 2148B501 */  addu       $t1, $t5, $s5
    /* 3BF80 8004B780 0000298D */  lw         $t1, 0x0($t1)
    /* 3BF84 8004B784 82730200 */  srl        $t6, $v0, 14
    /* 3BF88 8004B788 FC03CE31 */  andi       $t6, $t6, 0x3FC
    /* 3BF8C 8004B78C 2150D501 */  addu       $t2, $t6, $s5
    /* 3BF90 8004B790 00004A8D */  lw         $t2, 0x0($t2)
    /* 3BF94 8004B794 00608848 */  mtc2       $t0, $12 /* handwritten instruction */
    /* 3BF98 8004B798 00688948 */  mtc2       $t1, $13 /* handwritten instruction */
    /* 3BF9C 8004B79C 00708A48 */  mtc2       $t2, $14 /* handwritten instruction */
    /* 3BFA0 8004B7A0 24180901 */  and        $v1, $t0, $t1
    /* 3BFA4 8004B7A4 24186A00 */  and        $v1, $v1, $t2
    /* 3BFA8 8004B7A8 00000000 */  nop
    /* 3BFAC 8004B7AC 0600404B */  nclip
    /* 3BFB0 8004B7B0 D9006004 */  bltz       $v1, .L8004BB18
    /* 3BFB4 8004B7B4 00000000 */   nop
    /* 3BFB8 8004B7B8 00806330 */  andi       $v1, $v1, 0x8000
    /* 3BFBC 8004B7BC 00C00248 */  mfc2       $v0, $24 /* handwritten instruction */
    /* 3BFC0 8004B7C0 D5006014 */  bnez       $v1, .L8004BB18
    /* 3BFC4 8004B7C4 00000000 */   nop
    /* 3BFC8 8004B7C8 D3004104 */  bgez       $v0, .L8004BB18
    /* 3BFCC 8004B7CC 22180200 */   neg       $v1, $v0 /* handwritten instruction */
    /* 3BFD0 8004B7D0 03240800 */  sra        $a0, $t0, 16
    /* 3BFD4 8004B7D4 10FF8420 */  addi       $a0, $a0, -0xF0 /* handwritten instruction */
    /* 3BFD8 8004B7D8 032C0900 */  sra        $a1, $t1, 16
    /* 3BFDC 8004B7DC 10FFA520 */  addi       $a1, $a1, -0xF0 /* handwritten instruction */
    /* 3BFE0 8004B7E0 25208500 */  or         $a0, $a0, $a1
    /* 3BFE4 8004B7E4 032C0A00 */  sra        $a1, $t2, 16
    /* 3BFE8 8004B7E8 10FFA520 */  addi       $a1, $a1, -0xF0 /* handwritten instruction */
    /* 3BFEC 8004B7EC 25208500 */  or         $a0, $a0, $a1
    /* 3BFF0 8004B7F0 00808430 */  andi       $a0, $a0, 0x8000
    /* 3BFF4 8004B7F4 C8008010 */  beqz       $a0, .L8004BB18
    /* 3BFF8 8004B7F8 00000000 */   nop
    /* 3BFFC 8004B7FC 42600C00 */  srl        $t4, $t4, 1
    /* 3C000 8004B800 20209601 */  add        $a0, $t4, $s6 /* handwritten instruction */
    /* 3C004 8004B804 00008484 */  lh         $a0, 0x0($a0)
    /* 3C008 8004B808 42680D00 */  srl        $t5, $t5, 1
    /* 3C00C 8004B80C 2028B601 */  add        $a1, $t5, $s6 /* handwritten instruction */
    /* 3C010 8004B810 0000A584 */  lh         $a1, 0x0($a1)
    /* 3C014 8004B814 42700E00 */  srl        $t6, $t6, 1
    /* 3C018 8004B818 2030D601 */  add        $a2, $t6, $s6 /* handwritten instruction */
    /* 3C01C 8004B81C 0000C684 */  lh         $a2, 0x0($a2)
    /* 3C020 8004B820 2A108500 */  slt        $v0, $a0, $a1
    /* 3C024 8004B824 03004010 */  beqz       $v0, .L8004B834
    /* 3C028 8004B828 2A108600 */   slt       $v0, $a0, $a2
    /* 3C02C 8004B82C 2120A000 */  addu       $a0, $a1, $zero
    /* 3C030 8004B830 2A108600 */  slt        $v0, $a0, $a2
  .L8004B834:
    /* 3C034 8004B834 02004010 */  beqz       $v0, .L8004B840
    /* 3C038 8004B838 00000000 */   nop
    /* 3C03C 8004B83C 2120C000 */  addu       $a0, $a2, $zero
  .L8004B840:
    /* 3C040 8004B840 21108000 */  addu       $v0, $a0, $zero
    /* 3C044 8004B844 21384000 */  addu       $a3, $v0, $zero
    /* 3C048 8004B848 C601F886 */  lh         $t8, (0x1F8001C6 & 0xFFFF)($s7)
    /* 3C04C 8004B84C 1800E68E */  lw         $a2, (0x1F800018 & 0xFFFF)($s7)
    /* 3C050 8004B850 1000E58E */  lw         $a1, (0x1F800010 & 0xFFFF)($s7)
    /* 3C054 8004B854 06100203 */  srlv       $v0, $v0, $t8
    /* 3C058 8004B858 C2C20200 */  srl        $t8, $v0, 11
    /* 3C05C 8004B85C FF074230 */  andi       $v0, $v0, 0x7FF
    /* 3C060 8004B860 00F04220 */  addi       $v0, $v0, -0x1000 /* handwritten instruction */
    /* 3C064 8004B864 07100203 */  srav       $v0, $v0, $t8
    /* 3C068 8004B868 FF0F4230 */  andi       $v0, $v0, 0xFFF
    /* 3C06C 8004B86C 2A08A200 */  slt        $at, $a1, $v0
    /* 3C070 8004B870 A9002014 */  bnez       $at, .L8004BB18
    /* 3C074 8004B874 00000000 */   nop
    /* 3C078 8004B878 2A084600 */  slt        $at, $v0, $a2
    /* 3C07C 8004B87C A6002014 */  bnez       $at, .L8004BB18
    /* 3C080 8004B880 00000000 */   nop
    /* 3C084 8004B884 100000CA */  lwc2       $0, 0x10($s0)
    /* 3C088 8004B888 140001CA */  lwc2       $1, 0x14($s0)
    /* 3C08C 8004B88C 04000486 */  lh         $a0, 0x4($s0)
    /* 3C090 8004B890 00000586 */  lh         $a1, 0x0($s0)
    /* 3C094 8004B894 CC01E4A6 */  sh         $a0, (0x1F8001CC & 0xFFFF)($s7)
    /* 3C098 8004B898 CE01E5A6 */  sh         $a1, (0x1F8001CE & 0xFFFF)($s7)
    /* 3C09C 8004B89C 1E04C84A */  ncs
    /* 3C0A0 8004B8A0 00026428 */  slti       $a0, $v1, 0x200
    /* 3C0A4 8004B8A4 00036328 */  slti       $v1, $v1, 0x300
    /* 3C0A8 8004B8A8 20188300 */  add        $v1, $a0, $v1 /* handwritten instruction */
    /* 3C0AC 8004B8AC C001E58E */  lw         $a1, (0x1F8001C0 & 0xFFFF)($s7)
    /* 3C0B0 8004B8B0 00000000 */  nop
    /* 3C0B4 8004B8B4 0300A530 */  andi       $a1, $a1, 0x3
    /* 3C0B8 8004B8B8 1100A010 */  beqz       $a1, .L8004B900
    /* 3C0BC 8004B8BC 21200000 */   addu      $a0, $zero, $zero
    /* 3C0C0 8004B8C0 FEFFA620 */  addi       $a2, $a1, -0x2 /* handwritten instruction */
    /* 3C0C4 8004B8C4 0F00C010 */  beqz       $a2, .L8004B904
    /* 3C0C8 8004B8C8 00000000 */   nop
    /* 3C0CC 8004B8CC E001E482 */  lb         $a0, (0x1F8001E0 & 0xFFFF)($s7)
    /* 3C0D0 8004B8D0 00000000 */  nop
    /* 3C0D4 8004B8D4 FEFF8420 */  addi       $a0, $a0, -0x2 /* handwritten instruction */
    /* 3C0D8 8004B8D8 22200400 */  neg        $a0, $a0 /* handwritten instruction */
    /* 3C0DC 8004B8DC FFFFA620 */  addi       $a2, $a1, -0x1 /* handwritten instruction */
    /* 3C0E0 8004B8E0 0700C010 */  beqz       $a2, .L8004B900
    /* 3C0E4 8004B8E4 00000000 */   nop
    /* 3C0E8 8004B8E8 E001E482 */  lb         $a0, (0x1F8001E0 & 0xFFFF)($s7)
    /* 3C0EC 8004B8EC 00000000 */  nop
    /* 3C0F0 8004B8F0 2A288300 */  slt        $a1, $a0, $v1
    /* 3C0F4 8004B8F4 22280500 */  neg        $a1, $a1 /* handwritten instruction */
    /* 3C0F8 8004B8F8 22206400 */  sub        $a0, $v1, $a0 /* handwritten instruction */
    /* 3C0FC 8004B8FC 24208500 */  and        $a0, $a0, $a1
  .L8004B900:
    /* 3C100 8004B900 21188000 */  addu       $v1, $a0, $zero
  .L8004B904:
    /* 3C104 8004B904 06000496 */  lhu        $a0, 0x6($s0)
    /* 3C108 8004B908 08000596 */  lhu        $a1, 0x8($s0)
    /* 3C10C 8004B90C 0A000696 */  lhu        $a2, 0xA($s0)
    /* 3C110 8004B910 0A006010 */  beqz       $v1, .L8004B93C
    /* 3C114 8004B914 FF001824 */   addiu     $t8, $zero, 0xFF
    /* 3C118 8004B918 06C07800 */  srlv       $t8, $t8, $v1
    /* 3C11C 8004B91C 00CA1800 */  sll        $t9, $t8, 8
    /* 3C120 8004B920 25C01903 */  or         $t8, $t8, $t9
    /* 3C124 8004B924 06206400 */  srlv       $a0, $a0, $v1
    /* 3C128 8004B928 24209800 */  and        $a0, $a0, $t8
    /* 3C12C 8004B92C 06286500 */  srlv       $a1, $a1, $v1
    /* 3C130 8004B930 2428B800 */  and        $a1, $a1, $t8
    /* 3C134 8004B934 06306600 */  srlv       $a2, $a2, $v1
    /* 3C138 8004B938 2430D800 */  and        $a2, $a2, $t8
  .L8004B93C:
    /* 3C13C 8004B93C E201E4A6 */  sh         $a0, (0x1F8001E2 & 0xFFFF)($s7)
    /* 3C140 8004B940 E401E5A6 */  sh         $a1, (0x1F8001E4 & 0xFFFF)($s7)
    /* 3C144 8004B944 E601E6A6 */  sh         $a2, (0x1F8001E6 & 0xFFFF)($s7)
    /* 3C148 8004B948 02001892 */  lbu        $t8, 0x2($s0)
    /* 3C14C 8004B94C E001E782 */  lb         $a3, (0x1F8001E0 & 0xFFFF)($s7)
    /* 3C150 8004B950 42C01800 */  srl        $t8, $t8, 1
    /* 3C154 8004B954 03001833 */  andi       $t8, $t8, 0x3
    /* 3C158 8004B958 04000013 */  beqz       $t8, .L8004B96C
    /* 3C15C 8004B95C FFFF1823 */   addi      $t8, $t8, -0x1 /* handwritten instruction */
    /* 3C160 8004B960 2238F800 */  sub        $a3, $a3, $t8 /* handwritten instruction */
    /* 3C164 8004B964 2000E01C */  bgtz       $a3, .L8004B9E8
    /* 3C168 8004B968 00000000 */   nop
  .L8004B96C:
    /* 3C16C 8004B96C CC01EC86 */  lh         $t4, (0x1F8001CC & 0xFFFF)($s7)
    /* 3C170 8004B970 CE01ED86 */  lh         $t5, (0x1F8001CE & 0xFFFF)($s7)
    /* 3C174 8004B974 080088AE */  sw         $t0, 0x8($s4)
    /* 3C178 8004B978 100089AE */  sw         $t1, 0x10($s4)
    /* 3C17C 8004B97C 18008AAE */  sw         $t2, 0x18($s4)
    /* 3C180 8004B980 00640C00 */  sll        $t4, $t4, 16
    /* 3C184 8004B984 006C0D00 */  sll        $t5, $t5, 16
    /* 3C188 8004B988 E201E896 */  lhu        $t0, (0x1F8001E2 & 0xFFFF)($s7)
    /* 3C18C 8004B98C E401E996 */  lhu        $t1, (0x1F8001E4 & 0xFFFF)($s7)
    /* 3C190 8004B990 E601EA96 */  lhu        $t2, (0x1F8001E6 & 0xFFFF)($s7)
    /* 3C194 8004B994 25400C01 */  or         $t0, $t0, $t4
    /* 3C198 8004B998 25482D01 */  or         $t1, $t1, $t5
    /* 3C19C 8004B99C 0C0088AE */  sw         $t0, 0xC($s4)
    /* 3C1A0 8004B9A0 140089AE */  sw         $t1, 0x14($s4)
    /* 3C1A4 8004B9A4 1C008AA6 */  sh         $t2, 0x1C($s4)
    /* 3C1A8 8004B9A8 040096EA */  swc2       $22, 0x4($s4)
    /* 3C1AC 8004B9AC 80100200 */  sll        $v0, $v0, 2
    /* 3C1B0 8004B9B0 21105200 */  addu       $v0, $v0, $s2
    /* 3C1B4 8004B9B4 0000498C */  lw         $t1, 0x0($v0)
    /* 3C1B8 8004B9B8 24409E02 */  and        $t0, $s4, $fp
    /* 3C1BC 8004B9BC 000048AC */  sw         $t0, 0x0($v0)
    /* 3C1C0 8004B9C0 00070A3C */  lui        $t2, (0x7000000 >> 16)
    /* 3C1C4 8004B9C4 25482A01 */  or         $t1, $t1, $t2
    /* 3C1C8 8004B9C8 000089AE */  sw         $t1, 0x0($s4)
    /* 3C1CC 8004B9CC D401E48E */  lw         $a0, (0x1F8001D4 & 0xFFFF)($s7)
    /* 3C1D0 8004B9D0 18001022 */  addi       $s0, $s0, 0x18 /* handwritten instruction */
    /* 3C1D4 8004B9D4 20009422 */  addi       $s4, $s4, 0x20 /* handwritten instruction */
    /* 3C1D8 8004B9D8 5AFF8014 */  bnez       $a0, .L8004B744
    /* 3C1DC 8004B9DC FFFF8420 */   addi      $a0, $a0, -0x1 /* handwritten instruction */
    /* 3C1E0 8004B9E0 CA2E0108 */  j          .L8004BB28
    /* 3C1E4 8004B9E4 00000000 */   nop
  .L8004B9E8:
    /* 3C1E8 8004B9E8 C001E48E */  lw         $a0, (0x1F8001C0 & 0xFFFF)($s7)
    /* 3C1EC 8004B9EC 00000000 */  nop
    /* 3C1F0 8004B9F0 04008430 */  andi       $a0, $a0, 0x4
    /* 3C1F4 8004B9F4 14008010 */  beqz       $a0, .L8004BA48
    /* 3C1F8 8004B9F8 00000000 */   nop
    /* 3C1FC 8004B9FC 2120E000 */  addu       $a0, $a3, $zero
    /* 3C200 8004BA00 0000058E */  lw         $a1, 0x0($s0)
    /* 3C204 8004BA04 00000000 */  nop
    /* 3C208 8004BA08 FF00013C */  lui        $at, (0xFFFFFF >> 16)
    /* 3C20C 8004BA0C FFFF2134 */  ori        $at, $at, (0xFFFFFF & 0xFFFF)
    /* 3C210 8004BA10 2528A100 */  or         $a1, $a1, $at
    /* 3C214 8004BA14 09008010 */  beqz       $a0, .L8004BA3C
    /* 3C218 8004BA18 00000000 */   nop
    /* 3C21C 8004BA1C 00FF0124 */  addiu      $at, $zero, -0x100
    /* 3C220 8004BA20 2428A100 */  and        $a1, $a1, $at
    /* 3C224 8004BA24 FFFF8420 */  addi       $a0, $a0, -0x1 /* handwritten instruction */
    /* 3C228 8004BA28 04008010 */  beqz       $a0, .L8004BA3C
    /* 3C22C 8004BA2C 00000000 */   nop
    /* 3C230 8004BA30 FFFF013C */  lui        $at, (0xFFFF4000 >> 16)
    /* 3C234 8004BA34 00402134 */  ori        $at, $at, (0xFFFF4000 & 0xFFFF)
    /* 3C238 8004BA38 2428A100 */  and        $a1, $a1, $at
  .L8004BA3C:
    /* 3C23C 8004BA3C 00B08548 */  mtc2       $a1, $22 /* handwritten instruction */
    /* 3C240 8004BA40 00000000 */  nop
    /* 3C244 8004BA44 00000000 */  nop
  .L8004BA48:
    /* 3C248 8004BA48 80100200 */  sll        $v0, $v0, 2
    /* 3C24C 8004BA4C 21105200 */  addu       $v0, $v0, $s2
    /* 3C250 8004BA50 0000518C */  lw         $s1, 0x0($v0)
    /* 3C254 8004BA54 DC01E2AE */  sw         $v0, (0x1F8001DC & 0xFFFF)($s7)
    /* 3C258 8004BA58 0007083C */  lui        $t0, (0x7000000 >> 16)
    /* 3C25C 8004BA5C 25882802 */  or         $s1, $s1, $t0
    /* 3C260 8004BA60 D801E58E */  lw         $a1, (0x1F8001D8 & 0xFFFF)($s7)
    /* 3C264 8004BA64 D001F6EA */  swc2       $22, (0x1F8001D0 & 0xFFFF)($s7)
    /* 3C268 8004BA68 20408C01 */  add        $t0, $t4, $t4 /* handwritten instruction */
    /* 3C26C 8004BA6C 20608801 */  add        $t4, $t4, $t0 /* handwritten instruction */
    /* 3C270 8004BA70 20608501 */  add        $t4, $t4, $a1 /* handwritten instruction */
    /* 3C274 8004BA74 00008885 */  lh         $t0, 0x0($t4)
    /* 3C278 8004BA78 02008985 */  lh         $t1, 0x2($t4)
    /* 3C27C 8004BA7C 04008A85 */  lh         $t2, 0x4($t4)
    /* 3C280 8004BA80 E201EB86 */  lh         $t3, (0x1F8001E2 & 0xFFFF)($s7)
    /* 3C284 8004BA84 080068A6 */  sh         $t0, 0x8($s3)
    /* 3C288 8004BA88 0A0069A6 */  sh         $t1, 0xA($s3)
    /* 3C28C 8004BA8C 0C006AA6 */  sh         $t2, 0xC($s3)
    /* 3C290 8004BA90 0E006BA6 */  sh         $t3, 0xE($s3)
    /* 3C294 8004BA94 2040AD01 */  add        $t0, $t5, $t5 /* handwritten instruction */
    /* 3C298 8004BA98 2068A801 */  add        $t5, $t5, $t0 /* handwritten instruction */
    /* 3C29C 8004BA9C 2068A501 */  add        $t5, $t5, $a1 /* handwritten instruction */
    /* 3C2A0 8004BAA0 0000A885 */  lh         $t0, 0x0($t5)
    /* 3C2A4 8004BAA4 0200A985 */  lh         $t1, 0x2($t5)
    /* 3C2A8 8004BAA8 0400AA85 */  lh         $t2, 0x4($t5)
    /* 3C2AC 8004BAAC E401EB86 */  lh         $t3, (0x1F8001E4 & 0xFFFF)($s7)
    /* 3C2B0 8004BAB0 100068A6 */  sh         $t0, 0x10($s3)
    /* 3C2B4 8004BAB4 120069A6 */  sh         $t1, 0x12($s3)
    /* 3C2B8 8004BAB8 14006AA6 */  sh         $t2, 0x14($s3)
    /* 3C2BC 8004BABC 16006BA6 */  sh         $t3, 0x16($s3)
    /* 3C2C0 8004BAC0 2040CE01 */  add        $t0, $t6, $t6 /* handwritten instruction */
    /* 3C2C4 8004BAC4 2070C801 */  add        $t6, $t6, $t0 /* handwritten instruction */
    /* 3C2C8 8004BAC8 2070C501 */  add        $t6, $t6, $a1 /* handwritten instruction */
    /* 3C2CC 8004BACC 0000C885 */  lh         $t0, 0x0($t6)
    /* 3C2D0 8004BAD0 0200C985 */  lh         $t1, 0x2($t6)
    /* 3C2D4 8004BAD4 0400CA85 */  lh         $t2, 0x4($t6)
    /* 3C2D8 8004BAD8 E601EB86 */  lh         $t3, (0x1F8001E6 & 0xFFFF)($s7)
    /* 3C2DC 8004BADC 180068A6 */  sh         $t0, 0x18($s3)
    /* 3C2E0 8004BAE0 1A0069A6 */  sh         $t1, 0x1A($s3)
    /* 3C2E4 8004BAE4 1C006AA6 */  sh         $t2, 0x1C($s3)
    /* 3C2E8 8004BAE8 1E006BA6 */  sh         $t3, 0x1E($s3)
    /* 3C2EC 8004BAEC DA2E010C */  jal        func_8004BB68
    /* 3C2F0 8004BAF0 2120E000 */   addu      $a0, $a3, $zero
    /* 3C2F4 8004BAF4 DC01E88E */  lw         $t0, (0x1F8001DC & 0xFFFF)($s7)
    /* 3C2F8 8004BAF8 24483E02 */  and        $t1, $s1, $fp
    /* 3C2FC 8004BAFC 000009AD */  sw         $t1, 0x0($t0)
    /* 3C300 8004BB00 D401E48E */  lw         $a0, (0x1F8001D4 & 0xFFFF)($s7)
    /* 3C304 8004BB04 18001022 */  addi       $s0, $s0, 0x18 /* handwritten instruction */
    /* 3C308 8004BB08 0EFF8014 */  bnez       $a0, .L8004B744
    /* 3C30C 8004BB0C FFFF8420 */   addi      $a0, $a0, -0x1 /* handwritten instruction */
    /* 3C310 8004BB10 CA2E0108 */  j          .L8004BB28
    /* 3C314 8004BB14 00000000 */   nop
  .L8004BB18:
    /* 3C318 8004BB18 D401E48E */  lw         $a0, (0x1F8001D4 & 0xFFFF)($s7)
    /* 3C31C 8004BB1C 18001022 */  addi       $s0, $s0, 0x18 /* handwritten instruction */
    /* 3C320 8004BB20 08FF8014 */  bnez       $a0, .L8004B744
    /* 3C324 8004BB24 FFFF8420 */   addi      $a0, $a0, -0x1 /* handwritten instruction */
  .L8004BB28:
    /* 3C328 8004BB28 0A80013C */  lui        $at, %hi(D_800A38B4)
    /* 3C32C 8004BB2C B43834AC */  sw         $s4, %lo(D_800A38B4)($at)
    /* 3C330 8004BB30 21100002 */  addu       $v0, $s0, $zero
    /* 3C334 8004BB34 2140E002 */  addu       $t0, $s7, $zero
    /* 3C338 8004BB38 E8011F8D */  lw         $ra, (0x1F8001E8 & 0xFFFF)($t0)
    /* 3C33C 8004BB3C EC011E8D */  lw         $fp, (0x1F8001EC & 0xFFFF)($t0)
    /* 3C340 8004BB40 F001178D */  lw         $s7, (0x1F8001F0 & 0xFFFF)($t0)
    /* 3C344 8004BB44 F401168D */  lw         $s6, (0x1F8001F4 & 0xFFFF)($t0)
    /* 3C348 8004BB48 F801158D */  lw         $s5, (0x1F8001F8 & 0xFFFF)($t0)
    /* 3C34C 8004BB4C FC01148D */  lw         $s4, (0x1F8001FC & 0xFFFF)($t0)
    /* 3C350 8004BB50 0002138D */  lw         $s3, (0x1F800200 & 0xFFFF)($t0)
    /* 3C354 8004BB54 0402128D */  lw         $s2, (0x1F800204 & 0xFFFF)($t0)
    /* 3C358 8004BB58 0802118D */  lw         $s1, (0x1F800208 & 0xFFFF)($t0)
    /* 3C35C 8004BB5C 0C02108D */  lw         $s0, (0x1F80020C & 0xFFFF)($t0)
    /* 3C360 8004BB60 0800E003 */  jr         $ra
    /* 3C364 8004BB64 00000000 */   nop
endlabel func_8004A940
