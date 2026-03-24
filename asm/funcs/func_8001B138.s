glabel func_8001B138
    /* B938 8001B138 0A80023C */  lui        $v0, %hi(D_800A38BA)
    /* B93C 8001B13C BA384284 */  lh         $v0, %lo(D_800A38BA)($v0)
    /* B940 8001B140 1080013C */  lui        $at, %hi(D_800FF5C8)
    /* B944 8001B144 C8F520AC */  sw         $zero, %lo(D_800FF5C8)($at)
    /* B948 8001B148 1080013C */  lui        $at, %hi(D_800FF5CC)
    /* B94C 8001B14C CCF520AC */  sw         $zero, %lo(D_800FF5CC)($at)
    /* B950 8001B150 1080013C */  lui        $at, %hi(D_800FF5D0)
    /* B954 8001B154 D0F520AC */  sw         $zero, %lo(D_800FF5D0)($at)
    /* B958 8001B158 1080013C */  lui        $at, %hi(D_800FF5D8)
    /* B95C 8001B15C D8F520A4 */  sh         $zero, %lo(D_800FF5D8)($at)
    /* B960 8001B160 1080013C */  lui        $at, %hi(D_800FF5DA)
    /* B964 8001B164 DAF520A4 */  sh         $zero, %lo(D_800FF5DA)($at)
    /* B968 8001B168 1080013C */  lui        $at, %hi(D_800FF5DC)
    /* B96C 8001B16C DCF520A4 */  sh         $zero, %lo(D_800FF5DC)($at)
    /* B970 8001B170 1080013C */  lui        $at, %hi(D_800FF5E0)
    /* B974 8001B174 E0F520AC */  sw         $zero, %lo(D_800FF5E0)($at)
    /* B978 8001B178 40004010 */  beqz       $v0, .L8001B27C
    /* B97C 8001B17C 01000224 */   addiu     $v0, $zero, 0x1
    /* B980 8001B180 0A80033C */  lui        $v1, %hi(D_800A3834)
    /* B984 8001B184 34386384 */  lh         $v1, %lo(D_800A3834)($v1)
    /* B988 8001B188 00000000 */  nop
    /* B98C 8001B18C 3B006214 */  bne        $v1, $v0, .L8001B27C
    /* B990 8001B190 00000000 */   nop
    /* B994 8001B194 0000828C */  lw         $v0, 0x0($a0)
    /* B998 8001B198 00000000 */  nop
    /* B99C 8001B19C 01004230 */  andi       $v0, $v0, 0x1
    /* B9A0 8001B1A0 2D004010 */  beqz       $v0, .L8001B258
    /* B9A4 8001B1A4 01000224 */   addiu     $v0, $zero, 0x1
    /* B9A8 8001B1A8 0A80013C */  lui        $at, %hi(D_800A37E0)
    /* B9AC 8001B1AC E03722A0 */  sb         $v0, %lo(D_800A37E0)($at)
    /* B9B0 8001B1B0 0000828C */  lw         $v0, 0x0($a0)
    /* B9B4 8001B1B4 00000000 */  nop
    /* B9B8 8001B1B8 08004230 */  andi       $v0, $v0, 0x8
    /* B9BC 8001B1BC 07004010 */  beqz       $v0, .L8001B1DC
    /* B9C0 8001B1C0 00000000 */   nop
    /* B9C4 8001B1C4 0A80023C */  lui        $v0, %hi(D_800A3710)
    /* B9C8 8001B1C8 10374294 */  lhu        $v0, %lo(D_800A3710)($v0)
    /* B9CC 8001B1CC 00000000 */  nop
    /* B9D0 8001B1D0 CC044224 */  addiu      $v0, $v0, 0x4CC
    /* B9D4 8001B1D4 0A80013C */  lui        $at, %hi(D_800A3710)
    /* B9D8 8001B1D8 103722A4 */  sh         $v0, %lo(D_800A3710)($at)
  .L8001B1DC:
    /* B9DC 8001B1DC 0000828C */  lw         $v0, 0x0($a0)
    /* B9E0 8001B1E0 00000000 */  nop
    /* B9E4 8001B1E4 02004230 */  andi       $v0, $v0, 0x2
    /* B9E8 8001B1E8 07004010 */  beqz       $v0, .L8001B208
    /* B9EC 8001B1EC 00000000 */   nop
    /* B9F0 8001B1F0 0A80023C */  lui        $v0, %hi(D_800A3710)
    /* B9F4 8001B1F4 10374294 */  lhu        $v0, %lo(D_800A3710)($v0)
    /* B9F8 8001B1F8 00000000 */  nop
    /* B9FC 8001B1FC 34FB4224 */  addiu      $v0, $v0, -0x4CC
    /* BA00 8001B200 0A80013C */  lui        $at, %hi(D_800A3710)
    /* BA04 8001B204 103722A4 */  sh         $v0, %lo(D_800A3710)($at)
  .L8001B208:
    /* BA08 8001B208 0A80023C */  lui        $v0, %hi(D_800A3710)
    /* BA0C 8001B20C 10374284 */  lh         $v0, %lo(D_800A3710)($v0)
    /* BA10 8001B210 00000000 */  nop
    /* BA14 8001B214 00E44228 */  slti       $v0, $v0, -0x1C00
    /* BA18 8001B218 03004010 */  beqz       $v0, .L8001B228
    /* BA1C 8001B21C 00E40224 */   addiu     $v0, $zero, -0x1C00
    /* BA20 8001B220 0A80013C */  lui        $at, %hi(D_800A3710)
    /* BA24 8001B224 103722A4 */  sh         $v0, %lo(D_800A3710)($at)
  .L8001B228:
    /* BA28 8001B228 0A80023C */  lui        $v0, %hi(D_800A3710)
    /* BA2C 8001B22C 10374284 */  lh         $v0, %lo(D_800A3710)($v0)
    /* BA30 8001B230 00000000 */  nop
    /* BA34 8001B234 01744228 */  slti       $v0, $v0, 0x7401
    /* BA38 8001B238 03004014 */  bnez       $v0, .L8001B248
    /* BA3C 8001B23C 00740224 */   addiu     $v0, $zero, 0x7400
    /* BA40 8001B240 0A80013C */  lui        $at, %hi(D_800A3710)
    /* BA44 8001B244 103722A4 */  sh         $v0, %lo(D_800A3710)($at)
  .L8001B248:
    /* BA48 8001B248 0000828C */  lw         $v0, 0x0($a0)
    /* BA4C 8001B24C F4FF0324 */  addiu      $v1, $zero, -0xC
    /* BA50 8001B250 24104300 */  and        $v0, $v0, $v1
    /* BA54 8001B254 000082AC */  sw         $v0, 0x0($a0)
  .L8001B258:
    /* BA58 8001B258 0A80023C */  lui        $v0, %hi(D_800A3710)
    /* BA5C 8001B25C 10374284 */  lh         $v0, %lo(D_800A3710)($v0)
    /* BA60 8001B260 00000000 */  nop
    /* BA64 8001B264 02004104 */  bgez       $v0, .L8001B270
    /* BA68 8001B268 00000000 */   nop
    /* BA6C 8001B26C 0F004224 */  addiu      $v0, $v0, 0xF
  .L8001B270:
    /* BA70 8001B270 03110200 */  sra        $v0, $v0, 4
    /* BA74 8001B274 1080013C */  lui        $at, %hi(D_800FF5E0)
    /* BA78 8001B278 E0F522AC */  sw         $v0, %lo(D_800FF5E0)($at)
  .L8001B27C:
    /* BA7C 8001B27C FEFF033C */  lui        $v1, (0xFFFEFFFE >> 16)
    /* BA80 8001B280 0000828C */  lw         $v0, 0x0($a0)
    /* BA84 8001B284 FEFF6334 */  ori        $v1, $v1, (0xFFFEFFFE & 0xFFFF)
    /* BA88 8001B288 24104300 */  and        $v0, $v0, $v1
    /* BA8C 8001B28C 0800E003 */  jr         $ra
    /* BA90 8001B290 000082AC */   sw        $v0, 0x0($a0)
endlabel func_8001B138
