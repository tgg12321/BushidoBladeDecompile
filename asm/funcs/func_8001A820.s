glabel func_8001A820
    /* B020 8001A820 80FFBD27 */  addiu      $sp, $sp, -0x80
    /* B024 8001A824 21408000 */  addu       $t0, $a0, $zero
    /* B028 8001A828 64000224 */  addiu      $v0, $zero, 0x64
    /* B02C 8001A82C 7400B7AF */  sw         $s7, 0x74($sp)
    /* B030 8001A830 801F173C */  lui        $s7, (0x1F800010 >> 16)
    /* B034 8001A834 7000B6AF */  sw         $s6, 0x70($sp)
    /* B038 8001A838 0F80163C */  lui        $s6, %hi(D_800F6608)
    /* B03C 8001A83C 0866D626 */  addiu      $s6, $s6, %lo(D_800F6608)
    /* B040 8001A840 7C00BFAF */  sw         $ra, 0x7C($sp)
    /* B044 8001A844 7800BEAF */  sw         $fp, 0x78($sp)
    /* B048 8001A848 6C00B5AF */  sw         $s5, 0x6C($sp)
    /* B04C 8001A84C 6800B4AF */  sw         $s4, 0x68($sp)
    /* B050 8001A850 6400B3AF */  sw         $s3, 0x64($sp)
    /* B054 8001A854 6000B2AF */  sw         $s2, 0x60($sp)
    /* B058 8001A858 5C00B1AF */  sw         $s1, 0x5C($sp)
    /* B05C 8001A85C 5800B0AF */  sw         $s0, 0x58($sp)
    /* B060 8001A860 2000A6AF */  sw         $a2, 0x20($sp)
    /* B064 8001A864 2800A7AF */  sw         $a3, 0x28($sp)
    /* B068 8001A868 0F80013C */  lui        $at, %hi(D_800F6638)
    /* B06C 8001A86C 386622A4 */  sh         $v0, %lo(D_800F6638)($at)
    /* B070 8001A870 0F80013C */  lui        $at, %hi(D_800F663A)
    /* B074 8001A874 3A6620A4 */  sh         $zero, %lo(D_800F663A)($at)
    /* B078 8001A878 0F80013C */  lui        $at, %hi(D_800F663C)
    /* B07C 8001A87C 3C6622A4 */  sh         $v0, %lo(D_800F663C)($at)
    /* B080 8001A880 0F80013C */  lui        $at, %hi(D_800F6640)
    /* B084 8001A884 406622A4 */  sh         $v0, %lo(D_800F6640)($at)
    /* B088 8001A888 0F80013C */  lui        $at, %hi(D_800F6642)
    /* B08C 8001A88C 426620A4 */  sh         $zero, %lo(D_800F6642)($at)
    /* B090 8001A890 0F80013C */  lui        $at, %hi(D_800F6644)
    /* B094 8001A894 446622A4 */  sh         $v0, %lo(D_800F6644)($at)
    /* B098 8001A898 0400A38C */  lw         $v1, 0x4($a1)
    /* B09C 8001A89C 0400028D */  lw         $v0, 0x4($t0)
    /* B0A0 8001A8A0 0000A78C */  lw         $a3, 0x0($a1)
    /* B0A4 8001A8A4 0000068D */  lw         $a2, 0x0($t0)
    /* B0A8 8001A8A8 0800A48C */  lw         $a0, 0x8($a1)
    /* B0AC 8001A8AC 23486200 */  subu       $t1, $v1, $v0
    /* B0B0 8001A8B0 2388E600 */  subu       $s1, $a3, $a2
    /* B0B4 8001A8B4 0800028D */  lw         $v0, 0x8($t0)
    /* B0B8 8001A8B8 0A80033C */  lui        $v1, %hi(D_800A3690)
    /* B0BC 8001A8BC 90366390 */  lbu        $v1, %lo(D_800A3690)($v1)
    /* B0C0 8001A8C0 00000000 */  nop
    /* B0C4 8001A8C4 1B006014 */  bnez       $v1, .L8001A934
    /* B0C8 8001A8C8 23808200 */   subu      $s0, $a0, $v0
    /* B0CC 8001A8CC 2110C700 */  addu       $v0, $a2, $a3
    /* B0D0 8001A8D0 C21F0200 */  srl        $v1, $v0, 31
    /* B0D4 8001A8D4 21104300 */  addu       $v0, $v0, $v1
    /* B0D8 8001A8D8 43100200 */  sra        $v0, $v0, 1
    /* B0DC 8001A8DC 801F013C */  lui        $at, (0x1F800008 >> 16)
    /* B0E0 8001A8E0 080022AC */  sw         $v0, (0x1F800008 & 0xFFFF)($at)
    /* B0E4 8001A8E4 0400028D */  lw         $v0, 0x4($t0)
    /* B0E8 8001A8E8 0400A38C */  lw         $v1, 0x4($a1)
    /* B0EC 8001A8EC 00000000 */  nop
    /* B0F0 8001A8F0 21104300 */  addu       $v0, $v0, $v1
    /* B0F4 8001A8F4 C21F0200 */  srl        $v1, $v0, 31
    /* B0F8 8001A8F8 21104300 */  addu       $v0, $v0, $v1
    /* B0FC 8001A8FC 43100200 */  sra        $v0, $v0, 1
    /* B100 8001A900 801F013C */  lui        $at, (0x1F80000C >> 16)
    /* B104 8001A904 0C0022AC */  sw         $v0, (0x1F80000C & 0xFFFF)($at)
    /* B108 8001A908 0800028D */  lw         $v0, 0x8($t0)
    /* B10C 8001A90C 0800A38C */  lw         $v1, 0x8($a1)
    /* B110 8001A910 00000000 */  nop
    /* B114 8001A914 21104300 */  addu       $v0, $v0, $v1
    /* B118 8001A918 C21F0200 */  srl        $v1, $v0, 31
    /* B11C 8001A91C 21104300 */  addu       $v0, $v0, $v1
    /* B120 8001A920 43100200 */  sra        $v0, $v0, 1
    /* B124 8001A924 801F013C */  lui        $at, (0x1F800010 >> 16)
    /* B128 8001A928 100022AC */  sw         $v0, (0x1F800010 & 0xFFFF)($at)
    /* B12C 8001A92C 586A0008 */  j          .L8001A960
    /* B130 8001A930 00000000 */   nop
  .L8001A934:
    /* B134 8001A934 801F053C */  lui        $a1, (0x1F800008 >> 16)
    /* B138 8001A938 0800A534 */  ori        $a1, $a1, (0x1F800008 & 0xFFFF)
    /* B13C 8001A93C 0000028D */  lw         $v0, 0x0($t0)
    /* B140 8001A940 0400038D */  lw         $v1, 0x4($t0)
    /* B144 8001A944 0800048D */  lw         $a0, 0x8($t0)
    /* B148 8001A948 0000A2AC */  sw         $v0, 0x0($a1)
    /* B14C 8001A94C 0400A3AC */  sw         $v1, 0x4($a1)
    /* B150 8001A950 0800A4AC */  sw         $a0, 0x8($a1)
    /* B154 8001A954 0C00028D */  lw         $v0, 0xC($t0)
    /* B158 8001A958 00000000 */  nop
    /* B15C 8001A95C 0C00A2AC */  sw         $v0, 0xC($a1)
  .L8001A960:
    /* B160 8001A960 0800E28E */  lw         $v0, (0x1F800008 & 0xFFFF)($s7)
    /* B164 8001A964 0000C48E */  lw         $a0, 0x0($s6)
    /* B168 8001A968 00000000 */  nop
    /* B16C 8001A96C 23104400 */  subu       $v0, $v0, $a0
    /* B170 8001A970 02004104 */  bgez       $v0, .L8001A97C
    /* B174 8001A974 00000000 */   nop
    /* B178 8001A978 03004224 */  addiu      $v0, $v0, 0x3
  .L8001A97C:
    /* B17C 8001A97C 83100200 */  sra        $v0, $v0, 2
    /* B180 8001A980 0C00E38E */  lw         $v1, (0x1F80000C & 0xFFFF)($s7)
    /* B184 8001A984 0400C58E */  lw         $a1, 0x4($s6)
    /* B188 8001A988 21108200 */  addu       $v0, $a0, $v0
    /* B18C 8001A98C 0000C2AE */  sw         $v0, 0x0($s6)
    /* B190 8001A990 23106500 */  subu       $v0, $v1, $a1
    /* B194 8001A994 02004104 */  bgez       $v0, .L8001A9A0
    /* B198 8001A998 00000000 */   nop
    /* B19C 8001A99C 03004224 */  addiu      $v0, $v0, 0x3
  .L8001A9A0:
    /* B1A0 8001A9A0 83100200 */  sra        $v0, $v0, 2
    /* B1A4 8001A9A4 1000E38E */  lw         $v1, (0x1F800010 & 0xFFFF)($s7)
    /* B1A8 8001A9A8 0800C48E */  lw         $a0, 0x8($s6)
    /* B1AC 8001A9AC 2110A200 */  addu       $v0, $a1, $v0
    /* B1B0 8001A9B0 0400C2AE */  sw         $v0, 0x4($s6)
    /* B1B4 8001A9B4 23106400 */  subu       $v0, $v1, $a0
    /* B1B8 8001A9B8 02004104 */  bgez       $v0, .L8001A9C4
    /* B1BC 8001A9BC 00000000 */   nop
    /* B1C0 8001A9C0 03004224 */  addiu      $v0, $v0, 0x3
  .L8001A9C4:
    /* B1C4 8001A9C4 83100200 */  sra        $v0, $v0, 2
    /* B1C8 8001A9C8 21108200 */  addu       $v0, $a0, $v0
    /* B1CC 8001A9CC 0800C2AE */  sw         $v0, 0x8($s6)
    /* B1D0 8001A9D0 21282002 */  addu       $a1, $s1, $zero
    /* B1D4 8001A9D4 21302001 */  addu       $a2, $t1, $zero
    /* B1D8 8001A9D8 21200002 */  addu       $a0, $s0, $zero
    /* B1DC 8001A9DC 00402226 */  addiu      $v0, $s1, 0x4000
    /* B1E0 8001A9E0 00800334 */  ori        $v1, $zero, 0x8000
    /* B1E4 8001A9E4 2B106200 */  sltu       $v0, $v1, $v0
    /* B1E8 8001A9E8 05004014 */  bnez       $v0, .L8001AA00
    /* B1EC 8001A9EC 21380000 */   addu      $a3, $zero, $zero
    /* B1F0 8001A9F0 00400226 */  addiu      $v0, $s0, 0x4000
    /* B1F4 8001A9F4 2B106200 */  sltu       $v0, $v1, $v0
    /* B1F8 8001A9F8 14004010 */  beqz       $v0, .L8001AA4C
    /* B1FC 8001A9FC 1800A500 */   mult      $a1, $a1
  .L8001AA00:
    /* B200 8001AA00 C2170500 */  srl        $v0, $a1, 31
  .L8001AA04:
    /* B204 8001AA04 2110A200 */  addu       $v0, $a1, $v0
    /* B208 8001AA08 43280200 */  sra        $a1, $v0, 1
    /* B20C 8001AA0C C2170600 */  srl        $v0, $a2, 31
    /* B210 8001AA10 2110C200 */  addu       $v0, $a2, $v0
    /* B214 8001AA14 43300200 */  sra        $a2, $v0, 1
    /* B218 8001AA18 C2170400 */  srl        $v0, $a0, 31
    /* B21C 8001AA1C 21108200 */  addu       $v0, $a0, $v0
    /* B220 8001AA20 43200200 */  sra        $a0, $v0, 1
    /* B224 8001AA24 0040A224 */  addiu      $v0, $a1, 0x4000
    /* B228 8001AA28 00800334 */  ori        $v1, $zero, 0x8000
    /* B22C 8001AA2C 2B106200 */  sltu       $v0, $v1, $v0
    /* B230 8001AA30 F3FF4014 */  bnez       $v0, .L8001AA00
    /* B234 8001AA34 0100E724 */   addiu     $a3, $a3, 0x1
    /* B238 8001AA38 00408224 */  addiu      $v0, $a0, 0x4000
    /* B23C 8001AA3C 2B106200 */  sltu       $v0, $v1, $v0
    /* B240 8001AA40 F0FF4014 */  bnez       $v0, .L8001AA04
    /* B244 8001AA44 C2170500 */   srl       $v0, $a1, 31
    /* B248 8001AA48 1800A500 */  mult       $a1, $a1
  .L8001AA4C:
    /* B24C 8001AA4C 12100000 */  mflo       $v0
    /* B250 8001AA50 00000000 */  nop
    /* B254 8001AA54 00000000 */  nop
    /* B258 8001AA58 18008400 */  mult       $a0, $a0
    /* B25C 8001AA5C 12400000 */  mflo       $t0
    /* B260 8001AA60 00000000 */  nop
    /* B264 8001AA64 00000000 */  nop
    /* B268 8001AA68 1800C600 */  mult       $a2, $a2
    /* B26C 8001AA6C 21104800 */  addu       $v0, $v0, $t0
    /* B270 8001AA70 12180000 */  mflo       $v1
    /* B274 8001AA74 21204300 */  addu       $a0, $v0, $v1
    /* B278 8001AA78 0004822C */  sltiu      $v0, $a0, 0x400
    /* B27C 8001AA7C 06004010 */  beqz       $v0, .L8001AA98
    /* B280 8001AA80 00000000 */   nop
    /* B284 8001AA84 0980013C */  lui        $at, %hi(D_8008D118)
    /* B288 8001AA88 21082400 */  addu       $at, $at, $a0
    /* B28C 8001AA8C 18D12290 */  lbu        $v0, %lo(D_8008D118)($at)
    /* B290 8001AA90 BD6A0008 */  j          .L8001AAF4
    /* B294 8001AA94 C2200200 */   srl       $a0, $v0, 3
  .L8001AA98:
    /* B298 8001AA98 09008004 */  bltz       $a0, .L8001AAC0
    /* B29C 8001AA9C 21180000 */   addu      $v1, $zero, $zero
    /* B2A0 8001AAA0 21608000 */  addu       $t4, $a0, $zero
    /* B2A4 8001AAA4 00F08C48 */  mtc2       $t4, $30 /* handwritten instruction */
    /* B2A8 8001AAA8 00000000 */  nop
    /* B2AC 8001AAAC 00000000 */  nop
    /* B2B0 8001AAB0 1800A227 */  addiu      $v0, $sp, 0x18
    /* B2B4 8001AAB4 21604000 */  addu       $t4, $v0, $zero
    /* B2B8 8001AAB8 00009FE9 */  swc2       $31, 0x0($t4)
    /* B2BC 8001AABC 1800A38F */  lw         $v1, 0x18($sp)
  .L8001AAC0:
    /* B2C0 8001AAC0 FEFF0224 */  addiu      $v0, $zero, -0x2
    /* B2C4 8001AAC4 24106200 */  and        $v0, $v1, $v0
    /* B2C8 8001AAC8 16000324 */  addiu      $v1, $zero, 0x16
    /* B2CC 8001AACC 23186200 */  subu       $v1, $v1, $v0
    /* B2D0 8001AAD0 06106400 */  srlv       $v0, $a0, $v1
    /* B2D4 8001AAD4 0980013C */  lui        $at, %hi(D_8008D118)
    /* B2D8 8001AAD8 21082200 */  addu       $at, $at, $v0
    /* B2DC 8001AADC 18D12490 */  lbu        $a0, %lo(D_8008D118)($at)
    /* B2E0 8001AAE0 42180300 */  srl        $v1, $v1, 1
    /* B2E4 8001AAE4 13000224 */  addiu      $v0, $zero, 0x13
    /* B2E8 8001AAE8 23104300 */  subu       $v0, $v0, $v1
    /* B2EC 8001AAEC 00240400 */  sll        $a0, $a0, 16
    /* B2F0 8001AAF0 06204400 */  srlv       $a0, $a0, $v0
  .L8001AAF4:
    /* B2F4 8001AAF4 0420E400 */  sllv       $a0, $a0, $a3
    /* B2F8 8001AAF8 00408324 */  addiu      $v1, $a0, 0x4000
    /* B2FC 8001AAFC 0002023C */  lui        $v0, (0x2000000 >> 16)
    /* B300 8001AB00 1B004300 */  divu       $zero, $v0, $v1
    /* B304 8001AB04 02006014 */  bnez       $v1, .L8001AB10
    /* B308 8001AB08 00000000 */   nop
    /* B30C 8001AB0C 0D000700 */  break      7
  .L8001AB10:
    /* B310 8001AB10 12100000 */  mflo       $v0
    /* B314 8001AB14 2000AA8F */  lw         $t2, 0x20($sp)
    /* B318 8001AB18 13000724 */  addiu      $a3, $zero, 0x13
    /* B31C 8001AB1C 6A004395 */  lhu        $v1, 0x6A($t2)
    /* B320 8001AB20 00000000 */  nop
    /* B324 8001AB24 11006710 */  beq        $v1, $a3, .L8001AB6C
    /* B328 8001AB28 00044224 */   addiu     $v0, $v0, 0x400
    /* B32C 8001AB2C 1B000624 */  addiu      $a2, $zero, 0x1B
    /* B330 8001AB30 0E006610 */  beq        $v1, $a2, .L8001AB6C
    /* B334 8001AB34 00000000 */   nop
    /* B338 8001AB38 30000524 */  addiu      $a1, $zero, 0x30
    /* B33C 8001AB3C 0B006510 */  beq        $v1, $a1, .L8001AB6C
    /* B340 8001AB40 00000000 */   nop
    /* B344 8001AB44 2800AA8F */  lw         $t2, 0x28($sp)
    /* B348 8001AB48 00000000 */  nop
    /* B34C 8001AB4C 6A004395 */  lhu        $v1, 0x6A($t2)
    /* B350 8001AB50 00000000 */  nop
    /* B354 8001AB54 05006710 */  beq        $v1, $a3, .L8001AB6C
    /* B358 8001AB58 00000000 */   nop
    /* B35C 8001AB5C 03006610 */  beq        $v1, $a2, .L8001AB6C
    /* B360 8001AB60 00000000 */   nop
    /* B364 8001AB64 03006514 */  bne        $v1, $a1, .L8001AB74
    /* B368 8001AB68 EB51033C */   lui       $v1, (0x51EB851F >> 16)
  .L8001AB6C:
    /* B36C 8001AB6C 00104224 */  addiu      $v0, $v0, 0x1000
    /* B370 8001AB70 EB51033C */  lui        $v1, (0x51EB851F >> 16)
  .L8001AB74:
    /* B374 8001AB74 1F856334 */  ori        $v1, $v1, (0x51EB851F & 0xFFFF)
    /* B378 8001AB78 21108200 */  addu       $v0, $a0, $v0
    /* B37C 8001AB7C C0110200 */  sll        $v0, $v0, 7
    /* B380 8001AB80 19004300 */  multu      $v0, $v1
    /* B384 8001AB84 10500000 */  mfhi       $t2
    /* B388 8001AB88 02002105 */  bgez       $t1, .L8001AB94
    /* B38C 8001AB8C 42290A00 */   srl       $a1, $t2, 5
    /* B390 8001AB90 23480900 */  negu       $t1, $t1
  .L8001AB94:
    /* B394 8001AB94 2000AA8F */  lw         $t2, 0x20($sp)
    /* B398 8001AB98 00000000 */  nop
    /* B39C 8001AB9C 6A004495 */  lhu        $a0, 0x6A($t2)
    /* B3A0 8001ABA0 0F000224 */  addiu      $v0, $zero, 0xF
    /* B3A4 8001ABA4 FFFF8330 */  andi       $v1, $a0, 0xFFFF
    /* B3A8 8001ABA8 0B006210 */  beq        $v1, $v0, .L8001ABD8
    /* B3AC 8001ABAC 2128A900 */   addu      $a1, $a1, $t1
    /* B3B0 8001ABB0 E4FF8224 */  addiu      $v0, $a0, -0x1C
    /* B3B4 8001ABB4 0200422C */  sltiu      $v0, $v0, 0x2
    /* B3B8 8001ABB8 07004014 */  bnez       $v0, .L8001ABD8
    /* B3BC 8001ABBC E2FF8224 */   addiu     $v0, $a0, -0x1E
    /* B3C0 8001ABC0 0200422C */  sltiu      $v0, $v0, 0x2
    /* B3C4 8001ABC4 04004014 */  bnez       $v0, .L8001ABD8
    /* B3C8 8001ABC8 E0FF8224 */   addiu     $v0, $a0, -0x20
    /* B3CC 8001ABCC 0200422C */  sltiu      $v0, $v0, 0x2
    /* B3D0 8001ABD0 03004010 */  beqz       $v0, .L8001ABE0
    /* B3D4 8001ABD4 AA2A023C */   lui       $v0, (0x2AAAAAAB >> 16)
  .L8001ABD8:
    /* B3D8 8001ABD8 B80B0524 */  addiu      $a1, $zero, 0xBB8
    /* B3DC 8001ABDC AA2A023C */  lui        $v0, (0x2AAAAAAB >> 16)
  .L8001ABE0:
    /* B3E0 8001ABE0 1800C48E */  lw         $a0, 0x18($s6)
    /* B3E4 8001ABE4 ABAA4234 */  ori        $v0, $v0, (0x2AAAAAAB & 0xFFFF)
    /* B3E8 8001ABE8 2318A400 */  subu       $v1, $a1, $a0
    /* B3EC 8001ABEC 18006200 */  mult       $v1, $v0
    /* B3F0 8001ABF0 C31F0300 */  sra        $v1, $v1, 31
    /* B3F4 8001ABF4 10500000 */  mfhi       $t2
    /* B3F8 8001ABF8 43100A00 */  sra        $v0, $t2, 1
    /* B3FC 8001ABFC 23104300 */  subu       $v0, $v0, $v1
    /* B400 8001AC00 21208200 */  addu       $a0, $a0, $v0
    /* B404 8001AC04 1800C4AE */  sw         $a0, 0x18($s6)
    /* B408 8001AC08 2000AA8F */  lw         $t2, 0x20($sp)
    /* B40C 8001AC0C 00000000 */  nop
    /* B410 8001AC10 6A004595 */  lhu        $a1, 0x6A($t2)
    /* B414 8001AC14 0F000224 */  addiu      $v0, $zero, 0xF
    /* B418 8001AC18 FFFFA330 */  andi       $v1, $a1, 0xFFFF
    /* B41C 8001AC1C 0D006210 */  beq        $v1, $v0, .L8001AC54
    /* B420 8001AC20 E4FFA224 */   addiu     $v0, $a1, -0x1C
    /* B424 8001AC24 0200422C */  sltiu      $v0, $v0, 0x2
    /* B428 8001AC28 0A004014 */  bnez       $v0, .L8001AC54
    /* B42C 8001AC2C E2FFA224 */   addiu     $v0, $a1, -0x1E
    /* B430 8001AC30 0200422C */  sltiu      $v0, $v0, 0x2
    /* B434 8001AC34 07004014 */  bnez       $v0, .L8001AC54
    /* B438 8001AC38 E0FFA224 */   addiu     $v0, $a1, -0x20
    /* B43C 8001AC3C 0200422C */  sltiu      $v0, $v0, 0x2
    /* B440 8001AC40 04004014 */  bnez       $v0, .L8001AC54
    /* B444 8001AC44 70178228 */   slti      $v0, $a0, 0x1770
    /* B448 8001AC48 02004010 */  beqz       $v0, .L8001AC54
    /* B44C 8001AC4C 70170224 */   addiu     $v0, $zero, 0x1770
    /* B450 8001AC50 1800C2AE */  sw         $v0, 0x18($s6)
  .L8001AC54:
    /* B454 8001AC54 0100033C */  lui        $v1, (0x186A0 >> 16)
    /* B458 8001AC58 1800C28E */  lw         $v0, 0x18($s6)
    /* B45C 8001AC5C A0866334 */  ori        $v1, $v1, (0x186A0 & 0xFFFF)
    /* B460 8001AC60 2A106200 */  slt        $v0, $v1, $v0
    /* B464 8001AC64 02004010 */  beqz       $v0, .L8001AC70
    /* B468 8001AC68 00000000 */   nop
    /* B46C 8001AC6C 1800C3AE */  sw         $v1, 0x18($s6)
  .L8001AC70:
    /* B470 8001AC70 1E00C292 */  lbu        $v0, 0x1E($s6)
    /* B474 8001AC74 00000000 */  nop
    /* B478 8001AC78 04004010 */  beqz       $v0, .L8001AC8C
    /* B47C 8001AC7C 00000000 */   nop
    /* B480 8001AC80 1800C28E */  lw         $v0, 0x18($s6)
    /* B484 8001AC84 266B0008 */  j          .L8001AC98
    /* B488 8001AC88 8D554228 */   slti      $v0, $v0, 0x558D
  .L8001AC8C:
    /* B48C 8001AC8C 1800C28E */  lw         $v0, 0x18($s6)
    /* B490 8001AC90 00000000 */  nop
    /* B494 8001AC94 F1554228 */  slti       $v0, $v0, 0x55F1
  .L8001AC98:
    /* B498 8001AC98 01004238 */  xori       $v0, $v0, 0x1
    /* B49C 8001AC9C 1E00C2A2 */  sb         $v0, 0x1E($s6)
    /* B4A0 8001ACA0 1E00C492 */  lbu        $a0, 0x1E($s6)
    /* B4A4 8001ACA4 79FC000C */  jal        game_SetControllerPorts
    /* B4A8 8001ACA8 00000000 */   nop
    /* B4AC 8001ACAC 2000AA8F */  lw         $t2, 0x20($sp)
    /* B4B0 8001ACB0 00000000 */  nop
    /* B4B4 8001ACB4 6A004395 */  lhu        $v1, 0x6A($t2)
    /* B4B8 8001ACB8 11000224 */  addiu      $v0, $zero, 0x11
    /* B4BC 8001ACBC 04006214 */  bne        $v1, $v0, .L8001ACD0
    /* B4C0 8001ACC0 00000000 */   nop
    /* B4C4 8001ACC4 1200C296 */  lhu        $v0, 0x12($s6)
    /* B4C8 8001ACC8 3B6B0008 */  j          .L8001ACEC
    /* B4CC 8001ACCC 0200E2A6 */   sh        $v0, (0x1F800002 & 0xFFFF)($s7)
  .L8001ACD0:
    /* B4D0 8001ACD0 21202002 */  addu       $a0, $s1, $zero
    /* B4D4 8001ACD4 57FF010C */  jal        func_8007FD5C
    /* B4D8 8001ACD8 21280002 */   addu      $a1, $s0, $zero
    /* B4DC 8001ACDC 00040324 */  addiu      $v1, $zero, 0x400
    /* B4E0 8001ACE0 23186200 */  subu       $v1, $v1, $v0
    /* B4E4 8001ACE4 FF0F6330 */  andi       $v1, $v1, 0xFFF
    /* B4E8 8001ACE8 0200E3A6 */  sh         $v1, (0x1F800002 & 0xFFFF)($s7)
  .L8001ACEC:
    /* B4EC 8001ACEC 08000524 */  addiu      $a1, $zero, 0x8
    /* B4F0 8001ACF0 1800F426 */  addiu      $s4, $s7, %lo(D_1F800018)
    /* B4F4 8001ACF4 3800EA26 */  addiu      $t2, $s7, %lo(D_1F800038)
    /* B4F8 8001ACF8 0A80153C */  lui        $s5, %hi(D_800A30F0)
    /* B4FC 8001ACFC F030B526 */  addiu      $s5, $s5, %lo(D_800A30F0)
    /* B500 8001AD00 4800AAAF */  sw         $t2, 0x48($sp)
    /* B504 8001AD04 30000A24 */  addiu      $t2, $zero, 0x30
    /* B508 8001AD08 5000AAAF */  sw         $t2, 0x50($sp)
    /* B50C 8001AD0C 0200E496 */  lhu        $a0, (0x1F800002 & 0xFFFF)($s7)
    /* B510 8001AD10 0A80133C */  lui        $s3, %hi(D_800A30F4)
    /* B514 8001AD14 F4307326 */  addiu      $s3, $s3, %lo(D_800A30F4)
    /* B518 8001AD18 0400E0A6 */  sh         $zero, (0x1F800004 & 0xFFFF)($s7)
    /* B51C 8001AD1C 1200C286 */  lh         $v0, 0x12($s6)
    /* B520 8001AD20 00240400 */  sll        $a0, $a0, 16
    /* B524 8001AD24 03240400 */  sra        $a0, $a0, 16
    /* B528 8001AD28 3C69000C */  jal        func_8001A4F0
    /* B52C 8001AD2C 23208200 */   subu      $a0, $a0, $v0
    /* B530 8001AD30 08000524 */  addiu      $a1, $zero, 0x8
    /* B534 8001AD34 0400E496 */  lhu        $a0, (0x1F800004 & 0xFFFF)($s7)
    /* B538 8001AD38 1400C686 */  lh         $a2, 0x14($s6)
    /* B53C 8001AD3C 1200C396 */  lhu        $v1, 0x12($s6)
    /* B540 8001AD40 00240400 */  sll        $a0, $a0, 16
    /* B544 8001AD44 03240400 */  sra        $a0, $a0, 16
    /* B548 8001AD48 23208600 */  subu       $a0, $a0, $a2
    /* B54C 8001AD4C 21186200 */  addu       $v1, $v1, $v0
    /* B550 8001AD50 3C69000C */  jal        func_8001A4F0
    /* B554 8001AD54 1200C3A6 */   sh        $v1, 0x12($s6)
    /* B558 8001AD58 1400C396 */  lhu        $v1, 0x14($s6)
    /* B55C 8001AD5C 1000CA86 */  lh         $t2, 0x10($s6)
    /* B560 8001AD60 21186200 */  addu       $v1, $v1, $v0
    /* B564 8001AD64 4000AAAF */  sw         $t2, 0x40($sp)
    /* B568 8001AD68 1400C3A6 */  sh         $v1, 0x14($s6)
  .L8001AD6C:
    /* B56C 8001AD6C 2120C002 */  addu       $a0, $s6, $zero
    /* B570 8001AD70 4000B18F */  lw         $s1, 0x40($sp)
    /* B574 8001AD74 1800E526 */  addiu      $a1, $s7, %lo(D_1F800018)
    /* B578 8001AD78 4E69000C */  jal        func_8001A538
    /* B57C 8001AD7C 1000D1A6 */   sh        $s1, 0x10($s6)
    /* B580 8001AD80 0A80023C */  lui        $v0, %hi(D_800A30F0)
    /* B584 8001AD84 F0304224 */  addiu      $v0, $v0, %lo(D_800A30F0)
    /* B588 8001AD88 0400A212 */  beq        $s5, $v0, .L8001AD9C
    /* B58C 8001AD8C 00000000 */   nop
    /* B590 8001AD90 2800AA8F */  lw         $t2, 0x28($sp)
    /* B594 8001AD94 686B0008 */  j          .L8001ADA0
    /* B598 8001AD98 00000000 */   nop
  .L8001AD9C:
    /* B59C 8001AD9C 2000AA8F */  lw         $t2, 0x20($sp)
  .L8001ADA0:
    /* B5A0 8001ADA0 00000000 */  nop
    /* B5A4 8001ADA4 B800428D */  lw         $v0, 0xB8($t2)
    /* B5A8 8001ADA8 BC00438D */  lw         $v1, 0xBC($t2)
    /* B5AC 8001ADAC C000448D */  lw         $a0, 0xC0($t2)
    /* B5B0 8001ADB0 C400458D */  lw         $a1, 0xC4($t2)
    /* B5B4 8001ADB4 2800E2AE */  sw         $v0, (0x1F800028 & 0xFFFF)($s7)
    /* B5B8 8001ADB8 2C00E3AE */  sw         $v1, (0x1F80002C & 0xFFFF)($s7)
    /* B5BC 8001ADBC 3000E4AE */  sw         $a0, (0x1F800030 & 0xFFFF)($s7)
    /* B5C0 8001ADC0 3400E5AE */  sw         $a1, (0x1F800034 & 0xFFFF)($s7)
    /* B5C4 8001ADC4 2800E426 */  addiu      $a0, $s7, %lo(D_1F800028)
    /* B5C8 8001ADC8 21288002 */  addu       $a1, $s4, $zero
    /* B5CC 8001ADCC 5800E726 */  addiu      $a3, $s7, %lo(D_1F800058)
    /* B5D0 8001ADD0 4800A68F */  lw         $a2, 0x48($sp)
    /* B5D4 8001ADD4 2C00E38E */  lw         $v1, (0x1F80002C & 0xFFFF)($s7)
    /* B5D8 8001ADD8 6000E226 */  addiu      $v0, $s7, %lo(D_1F800060)
    /* B5DC 8001ADDC 1000A2AF */  sw         $v0, 0x10($sp)
    /* B5E0 8001ADE0 38FF6324 */  addiu      $v1, $v1, -0xC8
    /* B5E4 8001ADE4 854D010C */  jal        func_80053614
    /* B5E8 8001ADE8 2C00E3AE */   sw        $v1, (0x1F80002C & 0xFFFF)($s7)
    /* B5EC 8001ADEC 53004010 */  beqz       $v0, .L8001AF3C
    /* B5F0 8001ADF0 00000000 */   nop
    /* B5F4 8001ADF4 5A00E296 */  lhu        $v0, (0x1F80005A & 0xFFFF)($s7)
    /* B5F8 8001ADF8 00000000 */  nop
    /* B5FC 8001ADFC 00140200 */  sll        $v0, $v0, 16
    /* B600 8001AE00 03140200 */  sra        $v0, $v0, 16
    /* B604 8001AE04 E0FC4228 */  slti       $v0, $v0, -0x320
    /* B608 8001AE08 4C004010 */  beqz       $v0, .L8001AF3C
    /* B60C 8001AE0C 21288002 */   addu      $a1, $s4, $zero
    /* B610 8001AE10 5000AA8F */  lw         $t2, 0x50($sp)
    /* B614 8001AE14 4800A68F */  lw         $a2, 0x48($sp)
    /* B618 8001AE18 9F69000C */  jal        func_8001A67C
    /* B61C 8001AE1C 2120CA02 */   addu      $a0, $s6, $t2
    /* B620 8001AE20 0000A292 */  lbu        $v0, 0x0($s5)
    /* B624 8001AE24 00000000 */  nop
    /* B628 8001AE28 06004010 */  beqz       $v0, .L8001AE44
    /* B62C 8001AE2C 10000A24 */   addiu     $t2, $zero, 0x10
    /* B630 8001AE30 0000628E */  lw         $v0, 0x0($s3)
    /* B634 8001AE34 00000000 */  nop
    /* B638 8001AE38 20004224 */  addiu      $v0, $v0, 0x20
    /* B63C 8001AE3C 936B0008 */  j          .L8001AE4C
    /* B640 8001AE40 000062AE */   sw        $v0, 0x0($s3)
  .L8001AE44:
    /* B644 8001AE44 00006AAE */  sw         $t2, 0x0($s3)
    /* B648 8001AE48 FFFF3126 */  addiu      $s1, $s1, -0x1
  .L8001AE4C:
    /* B64C 8001AE4C 0000638E */  lw         $v1, 0x0($s3)
    /* B650 8001AE50 00000000 */  nop
    /* B654 8001AE54 10006228 */  slti       $v0, $v1, 0x10
    /* B658 8001AE58 03004010 */  beqz       $v0, .L8001AE68
    /* B65C 8001AE5C 10000A24 */   addiu     $t2, $zero, 0x10
    /* B660 8001AE60 9E6B0008 */  j          .L8001AE78
    /* B664 8001AE64 00006AAE */   sw        $t2, 0x0($s3)
  .L8001AE68:
    /* B668 8001AE68 01026228 */  slti       $v0, $v1, 0x201
    /* B66C 8001AE6C 02004014 */  bnez       $v0, .L8001AE78
    /* B670 8001AE70 00020224 */   addiu     $v0, $zero, 0x200
    /* B674 8001AE74 000062AE */  sw         $v0, 0x0($s3)
  .L8001AE78:
    /* B678 8001AE78 0000628E */  lw         $v0, 0x0($s3)
    /* B67C 8001AE7C 00000000 */  nop
    /* B680 8001AE80 02004104 */  bgez       $v0, .L8001AE8C
    /* B684 8001AE84 00000000 */   nop
    /* B688 8001AE88 07004224 */  addiu      $v0, $v0, 0x7
  .L8001AE8C:
    /* B68C 8001AE8C C3100200 */  sra        $v0, $v0, 3
    /* B690 8001AE90 21802202 */  addu       $s0, $s1, $v0
    /* B694 8001AE94 21900000 */  addu       $s2, $zero, $zero
    /* B698 8001AE98 23103002 */  subu       $v0, $s1, $s0
  .L8001AE9C:
    /* B69C 8001AE9C FF0F4330 */  andi       $v1, $v0, 0xFFF
    /* B6A0 8001AEA0 00086228 */  slti       $v0, $v1, 0x800
    /* B6A4 8001AEA4 02004014 */  bnez       $v0, .L8001AEB0
    /* B6A8 8001AEA8 2120C002 */   addu      $a0, $s6, $zero
    /* B6AC 8001AEAC 00F06324 */  addiu      $v1, $v1, -0x1000
  .L8001AEB0:
    /* B6B0 8001AEB0 21288002 */  addu       $a1, $s4, $zero
    /* B6B4 8001AEB4 C2170300 */  srl        $v0, $v1, 31
    /* B6B8 8001AEB8 21106200 */  addu       $v0, $v1, $v0
    /* B6BC 8001AEBC 43100200 */  sra        $v0, $v0, 1
    /* B6C0 8001AEC0 21100202 */  addu       $v0, $s0, $v0
    /* B6C4 8001AEC4 4E69000C */  jal        func_8001A538
    /* B6C8 8001AEC8 1000C2A6 */   sh        $v0, 0x10($s6)
    /* B6CC 8001AECC 2800E426 */  addiu      $a0, $s7, %lo(D_1F800028)
    /* B6D0 8001AED0 21288002 */  addu       $a1, $s4, $zero
    /* B6D4 8001AED4 3800E626 */  addiu      $a2, $s7, %lo(D_1F800038)
    /* B6D8 8001AED8 5800E726 */  addiu      $a3, $s7, %lo(D_1F800058)
    /* B6DC 8001AEDC 6000E226 */  addiu      $v0, $s7, %lo(D_1F800060)
    /* B6E0 8001AEE0 854D010C */  jal        func_80053614
    /* B6E4 8001AEE4 1000A2AF */   sw        $v0, 0x10($sp)
    /* B6E8 8001AEE8 0B004010 */  beqz       $v0, .L8001AF18
    /* B6EC 8001AEEC 00000000 */   nop
    /* B6F0 8001AEF0 5A00E296 */  lhu        $v0, (0x1F80005A & 0xFFFF)($s7)
    /* B6F4 8001AEF4 00000000 */  nop
    /* B6F8 8001AEF8 00140200 */  sll        $v0, $v0, 16
    /* B6FC 8001AEFC 03140200 */  sra        $v0, $v0, 16
    /* B700 8001AF00 E0FC4228 */  slti       $v0, $v0, -0x320
    /* B704 8001AF04 04004010 */  beqz       $v0, .L8001AF18
    /* B708 8001AF08 00000000 */   nop
    /* B70C 8001AF0C 1000D186 */  lh         $s1, 0x10($s6)
    /* B710 8001AF10 C86B0008 */  j          .L8001AF20
    /* B714 8001AF14 01005226 */   addiu     $s2, $s2, 0x1
  .L8001AF18:
    /* B718 8001AF18 1000D086 */  lh         $s0, 0x10($s6)
    /* B71C 8001AF1C 01005226 */  addiu      $s2, $s2, 0x1
  .L8001AF20:
    /* B720 8001AF20 0200422A */  slti       $v0, $s2, 0x2
    /* B724 8001AF24 DDFF4014 */  bnez       $v0, .L8001AE9C
    /* B728 8001AF28 23103002 */   subu      $v0, $s1, $s0
    /* B72C 8001AF2C 21880002 */  addu       $s1, $s0, $zero
    /* B730 8001AF30 01000224 */  addiu      $v0, $zero, 0x1
    /* B734 8001AF34 176C0008 */  j          .L8001B05C
    /* B738 8001AF38 0000A2A2 */   sb        $v0, 0x0($s5)
  .L8001AF3C:
    /* B73C 8001AF3C 0000A292 */  lbu        $v0, 0x0($s5)
    /* B740 8001AF40 00000000 */  nop
    /* B744 8001AF44 04004010 */  beqz       $v0, .L8001AF58
    /* B748 8001AF48 10000A24 */   addiu     $t2, $zero, 0x10
    /* B74C 8001AF4C 00006AAE */  sw         $t2, 0x0($s3)
    /* B750 8001AF50 DA6B0008 */  j          .L8001AF68
    /* B754 8001AF54 01003126 */   addiu     $s1, $s1, 0x1
  .L8001AF58:
    /* B758 8001AF58 0000628E */  lw         $v0, 0x0($s3)
    /* B75C 8001AF5C 00000000 */  nop
    /* B760 8001AF60 10004224 */  addiu      $v0, $v0, 0x10
    /* B764 8001AF64 000062AE */  sw         $v0, 0x0($s3)
  .L8001AF68:
    /* B768 8001AF68 0000638E */  lw         $v1, 0x0($s3)
    /* B76C 8001AF6C 00000000 */  nop
    /* B770 8001AF70 10006228 */  slti       $v0, $v1, 0x10
    /* B774 8001AF74 03004010 */  beqz       $v0, .L8001AF84
    /* B778 8001AF78 10000A24 */   addiu     $t2, $zero, 0x10
    /* B77C 8001AF7C E56B0008 */  j          .L8001AF94
    /* B780 8001AF80 00006AAE */   sw        $t2, 0x0($s3)
  .L8001AF84:
    /* B784 8001AF84 01016228 */  slti       $v0, $v1, 0x101
    /* B788 8001AF88 02004014 */  bnez       $v0, .L8001AF94
    /* B78C 8001AF8C 00010224 */   addiu     $v0, $zero, 0x100
    /* B790 8001AF90 000062AE */  sw         $v0, 0x0($s3)
  .L8001AF94:
    /* B794 8001AF94 0000628E */  lw         $v0, 0x0($s3)
    /* B798 8001AF98 00000000 */  nop
    /* B79C 8001AF9C 02004104 */  bgez       $v0, .L8001AFA8
    /* B7A0 8001AFA0 00000000 */   nop
    /* B7A4 8001AFA4 07004224 */  addiu      $v0, $v0, 0x7
  .L8001AFA8:
    /* B7A8 8001AFA8 C3100200 */  sra        $v0, $v0, 3
    /* B7AC 8001AFAC 23802202 */  subu       $s0, $s1, $v0
    /* B7B0 8001AFB0 21900000 */  addu       $s2, $zero, $zero
    /* B7B4 8001AFB4 5000BE8F */  lw         $fp, 0x50($sp)
    /* B7B8 8001AFB8 23103002 */  subu       $v0, $s1, $s0
  .L8001AFBC:
    /* B7BC 8001AFBC FF0F4330 */  andi       $v1, $v0, 0xFFF
    /* B7C0 8001AFC0 00086228 */  slti       $v0, $v1, 0x800
    /* B7C4 8001AFC4 02004014 */  bnez       $v0, .L8001AFD0
    /* B7C8 8001AFC8 2120C002 */   addu      $a0, $s6, $zero
    /* B7CC 8001AFCC 00F06324 */  addiu      $v1, $v1, -0x1000
  .L8001AFD0:
    /* B7D0 8001AFD0 21288002 */  addu       $a1, $s4, $zero
    /* B7D4 8001AFD4 C2170300 */  srl        $v0, $v1, 31
    /* B7D8 8001AFD8 21106200 */  addu       $v0, $v1, $v0
    /* B7DC 8001AFDC 43100200 */  sra        $v0, $v0, 1
    /* B7E0 8001AFE0 21100202 */  addu       $v0, $s0, $v0
    /* B7E4 8001AFE4 4E69000C */  jal        func_8001A538
    /* B7E8 8001AFE8 1000C2A6 */   sh        $v0, 0x10($s6)
    /* B7EC 8001AFEC 2800E426 */  addiu      $a0, $s7, %lo(D_1F800028)
    /* B7F0 8001AFF0 21288002 */  addu       $a1, $s4, $zero
    /* B7F4 8001AFF4 5800E726 */  addiu      $a3, $s7, %lo(D_1F800058)
    /* B7F8 8001AFF8 4800A68F */  lw         $a2, 0x48($sp)
    /* B7FC 8001AFFC 6000E226 */  addiu      $v0, $s7, %lo(D_1F800060)
    /* B800 8001B000 854D010C */  jal        func_80053614
    /* B804 8001B004 1000A2AF */   sw        $v0, 0x10($sp)
    /* B808 8001B008 0E004010 */  beqz       $v0, .L8001B044
    /* B80C 8001B00C 00000000 */   nop
    /* B810 8001B010 5A00E296 */  lhu        $v0, (0x1F80005A & 0xFFFF)($s7)
    /* B814 8001B014 00000000 */  nop
    /* B818 8001B018 00140200 */  sll        $v0, $v0, 16
    /* B81C 8001B01C 03140200 */  sra        $v0, $v0, 16
    /* B820 8001B020 E0FC4228 */  slti       $v0, $v0, -0x320
    /* B824 8001B024 07004010 */  beqz       $v0, .L8001B044
    /* B828 8001B028 2120DE02 */   addu      $a0, $s6, $fp
    /* B82C 8001B02C 4800A68F */  lw         $a2, 0x48($sp)
    /* B830 8001B030 9F69000C */  jal        func_8001A67C
    /* B834 8001B034 21288002 */   addu      $a1, $s4, $zero
    /* B838 8001B038 1000D086 */  lh         $s0, 0x10($s6)
    /* B83C 8001B03C 136C0008 */  j          .L8001B04C
    /* B840 8001B040 01005226 */   addiu     $s2, $s2, 0x1
  .L8001B044:
    /* B844 8001B044 1000D186 */  lh         $s1, 0x10($s6)
    /* B848 8001B048 01005226 */  addiu      $s2, $s2, 0x1
  .L8001B04C:
    /* B84C 8001B04C 0200422A */  slti       $v0, $s2, 0x2
    /* B850 8001B050 DAFF4014 */  bnez       $v0, .L8001AFBC
    /* B854 8001B054 23103002 */   subu      $v0, $s1, $s0
    /* B858 8001B058 0000A0A2 */  sb         $zero, 0x0($s5)
  .L8001B05C:
    /* B85C 8001B05C 0A80023C */  lui        $v0, %hi(D_800A30F0)
    /* B860 8001B060 F0304224 */  addiu      $v0, $v0, %lo(D_800A30F0)
    /* B864 8001B064 0300A212 */  beq        $s5, $v0, .L8001B074
    /* B868 8001B068 00000000 */   nop
    /* B86C 8001B06C 1E6C0008 */  j          .L8001B078
    /* B870 8001B070 3800B1AF */   sw        $s1, 0x38($sp)
  .L8001B074:
    /* B874 8001B074 3000B1AF */  sw         $s1, 0x30($sp)
  .L8001B078:
    /* B878 8001B078 0100B526 */  addiu      $s5, $s5, 0x1
    /* B87C 8001B07C 04007326 */  addiu      $s3, $s3, 0x4
    /* B880 8001B080 0A80023C */  lui        $v0, %hi(D_800A30F0 + 0x2)
    /* B884 8001B084 F2304224 */  addiu      $v0, $v0, %lo(D_800A30F0 + 0x2)
    /* B888 8001B088 5000AA8F */  lw         $t2, 0x50($sp)
    /* B88C 8001B08C 2A10A202 */  slt        $v0, $s5, $v0
    /* B890 8001B090 08004A25 */  addiu      $t2, $t2, 0x8
    /* B894 8001B094 35FF4014 */  bnez       $v0, .L8001AD6C
    /* B898 8001B098 5000AAAF */   sw        $t2, 0x50($sp)
    /* B89C 8001B09C 3800B18F */  lw         $s1, 0x38($sp)
    /* B8A0 8001B0A0 3000AA8F */  lw         $t2, 0x30($sp)
    /* B8A4 8001B0A4 00000000 */  nop
    /* B8A8 8001B0A8 2A105101 */  slt        $v0, $t2, $s1
    /* B8AC 8001B0AC 04004014 */  bnez       $v0, .L8001B0C0
    /* B8B0 8001B0B0 8000222A */   slti      $v0, $s1, 0x80
    /* B8B4 8001B0B4 3000B18F */  lw         $s1, 0x30($sp)
    /* B8B8 8001B0B8 00000000 */  nop
    /* B8BC 8001B0BC 8000222A */  slti       $v0, $s1, 0x80
  .L8001B0C0:
    /* B8C0 8001B0C0 03004010 */  beqz       $v0, .L8001B0D0
    /* B8C4 8001B0C4 C101222A */   slti      $v0, $s1, 0x1C1
    /* B8C8 8001B0C8 80001124 */  addiu      $s1, $zero, 0x80
    /* B8CC 8001B0CC C101222A */  slti       $v0, $s1, 0x1C1
  .L8001B0D0:
    /* B8D0 8001B0D0 02004014 */  bnez       $v0, .L8001B0DC
    /* B8D4 8001B0D4 00000000 */   nop
    /* B8D8 8001B0D8 C0011124 */  addiu      $s1, $zero, 0x1C0
  .L8001B0DC:
    /* B8DC 8001B0DC 4000AA8F */  lw         $t2, 0x40($sp)
    /* B8E0 8001B0E0 08000524 */  addiu      $a1, $zero, 0x8
    /* B8E4 8001B0E4 23202A02 */  subu       $a0, $s1, $t2
    /* B8E8 8001B0E8 4000AA97 */  lhu        $t2, 0x40($sp)
    /* B8EC 8001B0EC 3C69000C */  jal        func_8001A4F0
    /* B8F0 8001B0F0 1000CAA6 */   sh        $t2, 0x10($s6)
    /* B8F4 8001B0F4 1000C396 */  lhu        $v1, 0x10($s6)
    /* B8F8 8001B0F8 00000000 */  nop
    /* B8FC 8001B0FC 21186200 */  addu       $v1, $v1, $v0
    /* B900 8001B100 1000C3A6 */  sh         $v1, 0x10($s6)
    /* B904 8001B104 7C00BF8F */  lw         $ra, 0x7C($sp)
    /* B908 8001B108 7800BE8F */  lw         $fp, 0x78($sp)
    /* B90C 8001B10C 7400B78F */  lw         $s7, 0x74($sp)
    /* B910 8001B110 7000B68F */  lw         $s6, 0x70($sp)
    /* B914 8001B114 6C00B58F */  lw         $s5, 0x6C($sp)
    /* B918 8001B118 6800B48F */  lw         $s4, 0x68($sp)
    /* B91C 8001B11C 6400B38F */  lw         $s3, 0x64($sp)
    /* B920 8001B120 6000B28F */  lw         $s2, 0x60($sp)
    /* B924 8001B124 5C00B18F */  lw         $s1, 0x5C($sp)
    /* B928 8001B128 5800B08F */  lw         $s0, 0x58($sp)
    /* B92C 8001B12C 8000BD27 */  addiu      $sp, $sp, 0x80
    /* B930 8001B130 0800E003 */  jr         $ra
    /* B934 8001B134 00000000 */   nop
endlabel func_8001A820
