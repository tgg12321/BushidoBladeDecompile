glabel func_8003CF84
    /* 2D784 8003CF84 B8FFBD27 */  addiu      $sp, $sp, -0x48
    /* 2D788 8003CF88 4000BFAF */  sw         $ra, 0x40($sp)
    /* 2D78C 8003CF8C 3C00B1AF */  sw         $s1, 0x3C($sp)
    /* 2D790 8003CF90 76CD000C */  jal        func_800335D8
    /* 2D794 8003CF94 3800B0AF */   sw        $s0, 0x38($sp)
    /* 2D798 8003CF98 0A80053C */  lui        $a1, %hi(D_800A3748)
    /* 2D79C 8003CF9C 4837A580 */  lb         $a1, %lo(D_800A3748)($a1)
    /* 2D7A0 8003CFA0 00000000 */  nop
    /* 2D7A4 8003CFA4 00110500 */  sll        $v0, $a1, 4
    /* 2D7A8 8003CFA8 21104500 */  addu       $v0, $v0, $a1
    /* 2D7AC 8003CFAC 80100200 */  sll        $v0, $v0, 2
    /* 2D7B0 8003CFB0 21104500 */  addu       $v0, $v0, $a1
    /* 2D7B4 8003CFB4 80100200 */  sll        $v0, $v0, 2
    /* 2D7B8 8003CFB8 23104500 */  subu       $v0, $v0, $a1
    /* 2D7BC 8003CFBC 80100200 */  sll        $v0, $v0, 2
    /* 2D7C0 8003CFC0 1080013C */  lui        $at, %hi(D_80101ED2)
    /* 2D7C4 8003CFC4 21082200 */  addu       $at, $at, $v0
    /* 2D7C8 8003CFC8 D21E2284 */  lh         $v0, %lo(D_80101ED2)($at)
    /* 2D7CC 8003CFCC 00000000 */  nop
    /* 2D7D0 8003CFD0 40100200 */  sll        $v0, $v0, 1
    /* 2D7D4 8003CFD4 0980013C */  lui        $at, %hi(D_8008EAC0)
    /* 2D7D8 8003CFD8 21082200 */  addu       $at, $at, $v0
    /* 2D7DC 8003CFDC C0EA2384 */  lh         $v1, %lo(D_8008EAC0)($at)
    /* 2D7E0 8003CFE0 0A80023C */  lui        $v0, %hi(D_800A37B8)
    /* 2D7E4 8003CFE4 B837428C */  lw         $v0, %lo(D_800A37B8)($v0)
    /* 2D7E8 8003CFE8 00000000 */  nop
    /* 2D7EC 8003CFEC 08004314 */  bne        $v0, $v1, .L8003D010
    /* 2D7F0 8003CFF0 21880000 */   addu      $s1, $zero, $zero
    /* 2D7F4 8003CFF4 80200500 */  sll        $a0, $a1, 2
    /* 2D7F8 8003CFF8 21208500 */  addu       $a0, $a0, $a1
    /* 2D7FC 8003CFFC C0200400 */  sll        $a0, $a0, 3
    /* 2D800 8003D000 2D008424 */  addiu      $a0, $a0, 0x2D
    /* 2D804 8003D004 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 2D808 8003D008 9471010C */  jal        func_8005C650
    /* 2D80C 8003D00C 7F000624 */   addiu     $a2, $zero, 0x7F
  .L8003D010:
    /* 2D810 8003D010 0980033C */  lui        $v1, %hi(D_8008EB04)
    /* 2D814 8003D014 04EB6384 */  lh         $v1, %lo(D_8008EB04)($v1)
    /* 2D818 8003D018 0A80023C */  lui        $v0, %hi(D_800A37B8)
    /* 2D81C 8003D01C B837428C */  lw         $v0, %lo(D_800A37B8)($v0)
    /* 2D820 8003D020 00000000 */  nop
    /* 2D824 8003D024 09004314 */  bne        $v0, $v1, .L8003D04C
    /* 2D828 8003D028 7F000524 */   addiu     $a1, $zero, 0x7F
    /* 2D82C 8003D02C 0A80023C */  lui        $v0, %hi(D_800A3748)
    /* 2D830 8003D030 48374280 */  lb         $v0, %lo(D_800A3748)($v0)
    /* 2D834 8003D034 7F000624 */  addiu      $a2, $zero, 0x7F
    /* 2D838 8003D038 80200200 */  sll        $a0, $v0, 2
    /* 2D83C 8003D03C 21208200 */  addu       $a0, $a0, $v0
    /* 2D840 8003D040 C0200400 */  sll        $a0, $a0, 3
    /* 2D844 8003D044 9471010C */  jal        func_8005C650
    /* 2D848 8003D048 31008424 */   addiu     $a0, $a0, 0x31
  .L8003D04C:
    /* 2D84C 8003D04C 0980033C */  lui        $v1, %hi(D_8008EB06)
    /* 2D850 8003D050 06EB6384 */  lh         $v1, %lo(D_8008EB06)($v1)
    /* 2D854 8003D054 0A80023C */  lui        $v0, %hi(D_800A37B8)
    /* 2D858 8003D058 B837428C */  lw         $v0, %lo(D_800A37B8)($v0)
    /* 2D85C 8003D05C 00000000 */  nop
    /* 2D860 8003D060 09004314 */  bne        $v0, $v1, .L8003D088
    /* 2D864 8003D064 7F000524 */   addiu     $a1, $zero, 0x7F
    /* 2D868 8003D068 0A80023C */  lui        $v0, %hi(D_800A3748)
    /* 2D86C 8003D06C 48374280 */  lb         $v0, %lo(D_800A3748)($v0)
    /* 2D870 8003D070 7F000624 */  addiu      $a2, $zero, 0x7F
    /* 2D874 8003D074 80200200 */  sll        $a0, $v0, 2
    /* 2D878 8003D078 21208200 */  addu       $a0, $a0, $v0
    /* 2D87C 8003D07C C0200400 */  sll        $a0, $a0, 3
    /* 2D880 8003D080 9471010C */  jal        func_8005C650
    /* 2D884 8003D084 36008424 */   addiu     $a0, $a0, 0x36
  .L8003D088:
    /* 2D888 8003D088 0980033C */  lui        $v1, %hi(D_8008EB08)
    /* 2D88C 8003D08C 08EB6384 */  lh         $v1, %lo(D_8008EB08)($v1)
    /* 2D890 8003D090 0A80023C */  lui        $v0, %hi(D_800A37B8)
    /* 2D894 8003D094 B837428C */  lw         $v0, %lo(D_800A37B8)($v0)
    /* 2D898 8003D098 00000000 */  nop
    /* 2D89C 8003D09C 0A004314 */  bne        $v0, $v1, .L8003D0C8
    /* 2D8A0 8003D0A0 00000000 */   nop
    /* 2D8A4 8003D0A4 0A80023C */  lui        $v0, %hi(D_800A3748)
    /* 2D8A8 8003D0A8 48374280 */  lb         $v0, %lo(D_800A3748)($v0)
    /* 2D8AC 8003D0AC 00000000 */  nop
    /* 2D8B0 8003D0B0 02004014 */  bnez       $v0, .L8003D0BC
    /* 2D8B4 8003D0B4 2B000424 */   addiu     $a0, $zero, 0x2B
    /* 2D8B8 8003D0B8 53000424 */  addiu      $a0, $zero, 0x53
  .L8003D0BC:
    /* 2D8BC 8003D0BC 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 2D8C0 8003D0C0 9471010C */  jal        func_8005C650
    /* 2D8C4 8003D0C4 7F000624 */   addiu     $a2, $zero, 0x7F
  .L8003D0C8:
    /* 2D8C8 8003D0C8 0980033C */  lui        $v1, %hi(D_8008EB0A)
    /* 2D8CC 8003D0CC 0AEB6384 */  lh         $v1, %lo(D_8008EB0A)($v1)
    /* 2D8D0 8003D0D0 0A80023C */  lui        $v0, %hi(D_800A37B8)
    /* 2D8D4 8003D0D4 B837428C */  lw         $v0, %lo(D_800A37B8)($v0)
    /* 2D8D8 8003D0D8 00000000 */  nop
    /* 2D8DC 8003D0DC 04004314 */  bne        $v0, $v1, .L8003D0F0
    /* 2D8E0 8003D0E0 71000424 */   addiu     $a0, $zero, 0x71
    /* 2D8E4 8003D0E4 7F000524 */  addiu      $a1, $zero, 0x7F
    /* 2D8E8 8003D0E8 9471010C */  jal        func_8005C650
    /* 2D8EC 8003D0EC 7F000624 */   addiu     $a2, $zero, 0x7F
  .L8003D0F0:
    /* 2D8F0 8003D0F0 0980033C */  lui        $v1, %hi(D_8008EB0C)
    /* 2D8F4 8003D0F4 0CEB6384 */  lh         $v1, %lo(D_8008EB0C)($v1)
    /* 2D8F8 8003D0F8 0A80023C */  lui        $v0, %hi(D_800A37B8)
    /* 2D8FC 8003D0FC B837428C */  lw         $v0, %lo(D_800A37B8)($v0)
    /* 2D900 8003D100 00000000 */  nop
    /* 2D904 8003D104 18004314 */  bne        $v0, $v1, .L8003D168
    /* 2D908 8003D108 21200000 */   addu      $a0, $zero, $zero
    /* 2D90C 8003D10C 2000B027 */  addiu      $s0, $sp, 0x20
    /* 2D910 8003D110 4C07868F */  lw         $a2, %gp_rel(D_800A3818)($gp)
    /* 2D914 8003D114 4487000C */  jal        func_80021D10
    /* 2D918 8003D118 21280002 */   addu      $a1, $s0, $zero
    /* 2D91C 8003D11C 21200002 */  addu       $a0, $s0, $zero
    /* 2D920 8003D120 0A80053C */  lui        $a1, %hi(D_800A312C)
    /* 2D924 8003D124 2C31A524 */  addiu      $a1, $a1, %lo(D_800A312C)
    /* 2D928 8003D128 2000A28F */  lw         $v0, 0x20($sp)
    /* 2D92C 8003D12C 0980033C */  lui        $v1, %hi(D_8008EB10)
    /* 2D930 8003D130 10EB638C */  lw         $v1, %lo(D_8008EB10)($v1)
    /* 2D934 8003D134 0980063C */  lui        $a2, %hi(D_8008EB14)
    /* 2D938 8003D138 14EBC68C */  lw         $a2, %lo(D_8008EB14)($a2)
    /* 2D93C 8003D13C 0980073C */  lui        $a3, %hi(D_8008EB18)
    /* 2D940 8003D140 18EBE78C */  lw         $a3, %lo(D_8008EB18)($a3)
    /* 2D944 8003D144 21104300 */  addu       $v0, $v0, $v1
    /* 2D948 8003D148 2000A2AF */  sw         $v0, 0x20($sp)
    /* 2D94C 8003D14C 2400A28F */  lw         $v0, 0x24($sp)
    /* 2D950 8003D150 2800A38F */  lw         $v1, 0x28($sp)
    /* 2D954 8003D154 21104600 */  addu       $v0, $v0, $a2
    /* 2D958 8003D158 21186700 */  addu       $v1, $v1, $a3
    /* 2D95C 8003D15C 2400A2AF */  sw         $v0, 0x24($sp)
    /* 2D960 8003D160 2D86010C */  jal        func_800618B4
    /* 2D964 8003D164 2800A3AF */   sw        $v1, 0x28($sp)
  .L8003D168:
    /* 2D968 8003D168 1F54010C */  jal        func_8005507C
    /* 2D96C 8003D16C 00000000 */   nop
    /* 2D970 8003D170 2354010C */  jal        func_8005508C
    /* 2D974 8003D174 21804000 */   addu      $s0, $v0, $zero
    /* 2D978 8003D178 21200002 */  addu       $a0, $s0, $zero
    /* 2D97C 8003D17C 1984010C */  jal        func_80061064
    /* 2D980 8003D180 21284000 */   addu      $a1, $v0, $zero
    /* 2D984 8003D184 DA53010C */  jal        func_80054F68
    /* 2D988 8003D188 00000000 */   nop
    /* 2D98C 8003D18C 02004014 */  bnez       $v0, .L8003D198
    /* 2D990 8003D190 00000000 */   nop
    /* 2D994 8003D194 01001124 */  addiu      $s1, $zero, 0x1
  .L8003D198:
    /* 2D998 8003D198 07002016 */  bnez       $s1, .L8003D1B8
    /* 2D99C 8003D19C 4000033C */   lui       $v1, (0x400040 >> 16)
    /* 2D9A0 8003D1A0 1080023C */  lui        $v0, %hi(D_80102794)
    /* 2D9A4 8003D1A4 9427428C */  lw         $v0, %lo(D_80102794)($v0)
    /* 2D9A8 8003D1A8 40006334 */  ori        $v1, $v1, (0x400040 & 0xFFFF)
    /* 2D9AC 8003D1AC 24104300 */  and        $v0, $v0, $v1
    /* 2D9B0 8003D1B0 38004010 */  beqz       $v0, .L8003D294
    /* 2D9B4 8003D1B4 00000000 */   nop
  .L8003D1B8:
    /* 2D9B8 8003D1B8 3752010C */  jal        func_800548DC
    /* 2D9BC 8003D1BC 00000000 */   nop
    /* 2D9C0 8003D1C0 0A80033C */  lui        $v1, %hi(D_800A38DC)
    /* 2D9C4 8003D1C4 DC386384 */  lh         $v1, %lo(D_800A38DC)($v1)
    /* 2D9C8 8003D1C8 04000224 */  addiu      $v0, $zero, 0x4
    /* 2D9CC 8003D1CC 03006210 */  beq        $v1, $v0, .L8003D1DC
    /* 2D9D0 8003D1D0 06000224 */   addiu     $v0, $zero, 0x6
    /* 2D9D4 8003D1D4 0B006214 */  bne        $v1, $v0, .L8003D204
    /* 2D9D8 8003D1D8 00000000 */   nop
  .L8003D1DC:
    /* 2D9DC 8003D1DC 0A80033C */  lui        $v1, %hi(D_800A3748)
    /* 2D9E0 8003D1E0 48376380 */  lb         $v1, %lo(D_800A3748)($v1)
    /* 2D9E4 8003D1E4 0A80013C */  lui        $at, %hi(D_800A37D2)
    /* 2D9E8 8003D1E8 21082300 */  addu       $at, $at, $v1
    /* 2D9EC 8003D1EC D2372290 */  lbu        $v0, %lo(D_800A37D2)($at)
    /* 2D9F0 8003D1F0 00000000 */  nop
    /* 2D9F4 8003D1F4 01004224 */  addiu      $v0, $v0, 0x1
    /* 2D9F8 8003D1F8 0A80013C */  lui        $at, %hi(D_800A37D2)
    /* 2D9FC 8003D1FC 21082300 */  addu       $at, $at, $v1
    /* 2DA00 8003D200 D23722A0 */  sb         $v0, %lo(D_800A37D2)($at)
  .L8003D204:
    /* 2DA04 8003D204 1080053C */  lui        $a1, %hi(D_80102770)
    /* 2DA08 8003D208 7027A58C */  lw         $a1, %lo(D_80102770)($a1)
    /* 2DA0C 8003D20C E765000C */  jal        func_8001979C
    /* 2DA10 8003D210 21200000 */   addu      $a0, $zero, $zero
    /* 2DA14 8003D214 1080053C */  lui        $a1, %hi(D_801027C0)
    /* 2DA18 8003D218 C027A58C */  lw         $a1, %lo(D_801027C0)($a1)
    /* 2DA1C 8003D21C E765000C */  jal        func_8001979C
    /* 2DA20 8003D220 01000424 */   addiu     $a0, $zero, 0x1
    /* 2DA24 8003D224 1080053C */  lui        $a1, %hi(D_801027D4)
    /* 2DA28 8003D228 D427A58C */  lw         $a1, %lo(D_801027D4)($a1)
    /* 2DA2C 8003D22C E765000C */  jal        func_8001979C
    /* 2DA30 8003D230 02000424 */   addiu     $a0, $zero, 0x2
    /* 2DA34 8003D234 0A80023C */  lui        $v0, %hi(D_800A38DC)
    /* 2DA38 8003D238 DC384284 */  lh         $v0, %lo(D_800A38DC)($v0)
    /* 2DA3C 8003D23C 00000000 */  nop
    /* 2DA40 8003D240 12004014 */  bnez       $v0, .L8003D28C
    /* 2DA44 8003D244 18000224 */   addiu     $v0, $zero, 0x18
    /* 2DA48 8003D248 0A80033C */  lui        $v1, %hi(D_800A3836)
    /* 2DA4C 8003D24C 36386390 */  lbu        $v1, %lo(D_800A3836)($v1)
    /* 2DA50 8003D250 FF000224 */  addiu      $v0, $zero, 0xFF
    /* 2DA54 8003D254 0D006210 */  beq        $v1, $v0, .L8003D28C
    /* 2DA58 8003D258 18000224 */   addiu     $v0, $zero, 0x18
    /* 2DA5C 8003D25C 8B76000C */  jal        func_8001DA2C
    /* 2DA60 8003D260 00000000 */   nop
    /* 2DA64 8003D264 CAEC000C */  jal        func_8003B328
    /* 2DA68 8003D268 00000000 */   nop
    /* 2DA6C 8003D26C D0EB000C */  jal        func_8003AF40
    /* 2DA70 8003D270 21200000 */   addu      $a0, $zero, $zero
    /* 2DA74 8003D274 FFEB000C */  jal        func_8003AFFC
    /* 2DA78 8003D278 00000000 */   nop
    /* 2DA7C 8003D27C 4DED000C */  jal        func_8003B534
    /* 2DA80 8003D280 04000424 */   addiu     $a0, $zero, 0x4
    /* 2DA84 8003D284 A5F40008 */  j          .L8003D294
    /* 2DA88 8003D288 00000000 */   nop
  .L8003D28C:
    /* 2DA8C 8003D28C 0A80013C */  lui        $at, %hi(D_800A3834)
    /* 2DA90 8003D290 343822A4 */  sh         $v0, %lo(D_800A3834)($at)
  .L8003D294:
    /* 2DA94 8003D294 0A80023C */  lui        $v0, %hi(D_800A37B8)
    /* 2DA98 8003D298 B837428C */  lw         $v0, %lo(D_800A37B8)($v0)
    /* 2DA9C 8003D29C 00000000 */  nop
    /* 2DAA0 8003D2A0 01004224 */  addiu      $v0, $v0, 0x1
    /* 2DAA4 8003D2A4 0A80013C */  lui        $at, %hi(D_800A37B8)
    /* 2DAA8 8003D2A8 B83722AC */  sw         $v0, %lo(D_800A37B8)($at)
    /* 2DAAC 8003D2AC 4000BF8F */  lw         $ra, 0x40($sp)
    /* 2DAB0 8003D2B0 3C00B18F */  lw         $s1, 0x3C($sp)
    /* 2DAB4 8003D2B4 3800B08F */  lw         $s0, 0x38($sp)
    /* 2DAB8 8003D2B8 4800BD27 */  addiu      $sp, $sp, 0x48
    /* 2DABC 8003D2BC 0800E003 */  jr         $ra
    /* 2DAC0 8003D2C0 00000000 */   nop
endlabel func_8003CF84
