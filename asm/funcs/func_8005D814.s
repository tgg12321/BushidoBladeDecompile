glabel func_8005D814
    /* 4E014 8005D814 60FFBD27 */  addiu      $sp, $sp, -0xA0
    /* 4E018 8005D818 9000B6AF */  sw         $s6, 0x90($sp)
    /* 4E01C 8005D81C 21B08000 */  addu       $s6, $a0, $zero
    /* 4E020 8005D820 FFFFA524 */  addiu      $a1, $a1, -0x1
    /* 4E024 8005D824 7800B0AF */  sw         $s0, 0x78($sp)
    /* 4E028 8005D828 21800000 */  addu       $s0, $zero, $zero
    /* 4E02C 8005D82C 7C00B1AF */  sw         $s1, 0x7C($sp)
    /* 4E030 8005D830 0A80113C */  lui        $s1, %hi(D_8009B3C8)
    /* 4E034 8005D834 C8B33126 */  addiu      $s1, $s1, %lo(D_8009B3C8)
    /* 4E038 8005D838 9800BEAF */  sw         $fp, 0x98($sp)
    /* 4E03C 8005D83C 5800A6AF */  sw         $a2, 0x58($sp)
    /* 4E040 8005D840 5800BE8F */  lw         $fp, 0x58($sp)
    /* 4E044 8005D844 0A80023C */  lui        $v0, %hi(D_8009B398)
    /* 4E048 8005D848 98B34224 */  addiu      $v0, $v0, %lo(D_8009B398)
    /* 4E04C 8005D84C 9C00BFAF */  sw         $ra, 0x9C($sp)
    /* 4E050 8005D850 9400B7AF */  sw         $s7, 0x94($sp)
    /* 4E054 8005D854 8C00B5AF */  sw         $s5, 0x8C($sp)
    /* 4E058 8005D858 8800B4AF */  sw         $s4, 0x88($sp)
    /* 4E05C 8005D85C 8400B3AF */  sw         $s3, 0x84($sp)
    /* 4E060 8005D860 8000B2AF */  sw         $s2, 0x80($sp)
    /* 4E064 8005D864 5000A5AF */  sw         $a1, 0x50($sp)
    /* 4E068 8005D868 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 4E06C 8005D86C 3400A0AF */  sw         $zero, 0x34($sp)
    /* 4E070 8005D870 3000A0AF */  sw         $zero, 0x30($sp)
    /* 4E074 8005D874 6000A7AF */  sw         $a3, 0x60($sp)
    /* 4E078 8005D878 2C00A7AF */  sw         $a3, 0x2C($sp)
    /* 4E07C 8005D87C 2800A0AF */  sw         $zero, 0x28($sp)
    /* 4E080 8005D880 1800A2AF */  sw         $v0, 0x18($sp)
    /* 4E084 8005D884 2160C003 */  addu       $t4, $fp, $zero
    /* 4E088 8005D888 A0009325 */  addiu      $s3, $t4, 0xA0
    /* 4E08C 8005D88C F8028D25 */  addiu      $t5, $t4, 0x2F8
    /* 4E090 8005D890 04038C25 */  addiu      $t4, $t4, 0x304
    /* 4E094 8005D894 6800ADAF */  sw         $t5, 0x68($sp)
    /* 4E098 8005D898 7000ACAF */  sw         $t4, 0x70($sp)
    /* 4E09C 8005D89C 1800A427 */  addiu      $a0, $sp, 0x18
  .L8005D8A0:
    /* 4E0A0 8005D8A0 00141000 */  sll        $v0, $s0, 16
    /* 4E0A4 8005D8A4 43130200 */  sra        $v0, $v0, 13
    /* 4E0A8 8005D8A8 21105100 */  addu       $v0, $v0, $s1
    /* 4E0AC 8005D8AC 1C00A2AF */  sw         $v0, 0x1C($sp)
    /* 4E0B0 8005D8B0 4BCD010C */  jal        func_8007352C
    /* 4E0B4 8005D8B4 2000B3AF */   sw        $s3, 0x20($sp)
    /* 4E0B8 8005D8B8 21984000 */  addu       $s3, $v0, $zero
    /* 4E0BC 8005D8BC 01000226 */  addiu      $v0, $s0, 0x1
    /* 4E0C0 8005D8C0 21804000 */  addu       $s0, $v0, $zero
    /* 4E0C4 8005D8C4 00140200 */  sll        $v0, $v0, 16
    /* 4E0C8 8005D8C8 03140200 */  sra        $v0, $v0, 16
    /* 4E0CC 8005D8CC 03004228 */  slti       $v0, $v0, 0x3
    /* 4E0D0 8005D8D0 F3FF4014 */  bnez       $v0, .L8005D8A0
    /* 4E0D4 8005D8D4 1800A427 */   addiu     $a0, $sp, 0x18
    /* 4E0D8 8005D8D8 0A80023C */  lui        $v0, %hi(D_8009B3A4)
    /* 4E0DC 8005D8DC A4B34224 */  addiu      $v0, $v0, %lo(D_8009B3A4)
    /* 4E0E0 8005D8E0 1800A2AF */  sw         $v0, 0x18($sp)
    /* 4E0E4 8005D8E4 21800000 */  addu       $s0, $zero, $zero
    /* 4E0E8 8005D8E8 0A80123C */  lui        $s2, %hi(D_8009B3E0)
    /* 4E0EC 8005D8EC E0B35226 */  addiu      $s2, $s2, %lo(D_8009B3E0)
    /* 4E0F0 8005D8F0 01001124 */  addiu      $s1, $zero, 0x1
    /* 4E0F4 8005D8F4 001C1000 */  sll        $v1, $s0, 16
  .L8005D8F8:
    /* 4E0F8 8005D8F8 031C0300 */  sra        $v1, $v1, 16
    /* 4E0FC 8005D8FC C0100300 */  sll        $v0, $v1, 3
    /* 4E100 8005D900 21205200 */  addu       $a0, $v0, $s2
    /* 4E104 8005D904 07006010 */  beqz       $v1, .L8005D924
    /* 4E108 8005D908 1C00A4AF */   sw        $a0, 0x1C($sp)
    /* 4E10C 8005D90C 5000AD8F */  lw         $t5, 0x50($sp)
    /* 4E110 8005D910 00000000 */  nop
    /* 4E114 8005D914 0200B115 */  bne        $t5, $s1, .L8005D920
    /* 4E118 8005D918 3C000224 */   addiu     $v0, $zero, 0x3C
    /* 4E11C 8005D91C 2D000224 */  addiu      $v0, $zero, 0x2D
  .L8005D920:
    /* 4E120 8005D920 060082A0 */  sb         $v0, 0x6($a0)
  .L8005D924:
    /* 4E124 8005D924 2000B3AF */  sw         $s3, 0x20($sp)
    /* 4E128 8005D928 4BCD010C */  jal        func_8007352C
    /* 4E12C 8005D92C 1800A427 */   addiu     $a0, $sp, 0x18
    /* 4E130 8005D930 21984000 */  addu       $s3, $v0, $zero
    /* 4E134 8005D934 01000226 */  addiu      $v0, $s0, 0x1
    /* 4E138 8005D938 21804000 */  addu       $s0, $v0, $zero
    /* 4E13C 8005D93C 00140200 */  sll        $v0, $v0, 16
    /* 4E140 8005D940 03140200 */  sra        $v0, $v0, 16
    /* 4E144 8005D944 02004228 */  slti       $v0, $v0, 0x2
    /* 4E148 8005D948 EBFF4014 */  bnez       $v0, .L8005D8F8
    /* 4E14C 8005D94C 001C1000 */   sll       $v1, $s0, 16
    /* 4E150 8005D950 21A00000 */  addu       $s4, $zero, $zero
    /* 4E154 8005D954 01001724 */  addiu      $s7, $zero, 0x1
    /* 4E158 8005D958 1800B227 */  addiu      $s2, $sp, 0x18
    /* 4E15C 8005D95C 6666113C */  lui        $s1, (0x66666667 >> 16)
    /* 4E160 8005D960 67663136 */  ori        $s1, $s1, (0x66666667 & 0xFFFF)
    /* 4E164 8005D964 6000AC8F */  lw         $t4, 0x60($sp)
    /* 4E168 8005D968 0A80023C */  lui        $v0, %hi(D_8009B398)
    /* 4E16C 8005D96C 98B34224 */  addiu      $v0, $v0, %lo(D_8009B398)
    /* 4E170 8005D970 1800A2AF */  sw         $v0, 0x18($sp)
    /* 4E174 8005D974 16000224 */  addiu      $v0, $zero, 0x16
    /* 4E178 8005D978 4000A0A3 */  sb         $zero, 0x40($sp)
    /* 4E17C 8005D97C 3400A2AF */  sw         $v0, 0x34($sp)
    /* 4E180 8005D980 2800A0AF */  sw         $zero, 0x28($sp)
    /* 4E184 8005D984 2C00ACAF */  sw         $t4, 0x2C($sp)
    /* 4E188 8005D988 21800000 */  addu       $s0, $zero, $zero
  .L8005D98C:
    /* 4E18C 8005D98C 00141400 */  sll        $v0, $s4, 16
    /* 4E190 8005D990 03AC0200 */  sra        $s5, $v0, 16
  .L8005D994:
    /* 4E194 8005D994 4E00B712 */  beq        $s5, $s7, .L8005DAD0
    /* 4E198 8005D998 0200A22A */   slti      $v0, $s5, 0x2
    /* 4E19C 8005D99C 05004010 */  beqz       $v0, .L8005D9B4
    /* 4E1A0 8005D9A0 00000000 */   nop
    /* 4E1A4 8005D9A4 0800A012 */  beqz       $s5, .L8005D9C8
    /* 4E1A8 8005D9A8 00141000 */   sll       $v0, $s0, 16
    /* 4E1AC 8005D9AC 38770108 */  j          .L8005DCE0
    /* 4E1B0 8005D9B0 2000B3AF */   sw        $s3, 0x20($sp)
  .L8005D9B4:
    /* 4E1B4 8005D9B4 02000224 */  addiu      $v0, $zero, 0x2
    /* 4E1B8 8005D9B8 8800A212 */  beq        $s5, $v0, .L8005DBDC
    /* 4E1BC 8005D9BC 00141000 */   sll       $v0, $s0, 16
    /* 4E1C0 8005D9C0 38770108 */  j          .L8005DCE0
    /* 4E1C4 8005D9C4 2000B3AF */   sw        $s3, 0x20($sp)
  .L8005D9C8:
    /* 4E1C8 8005D9C8 03140200 */  sra        $v0, $v0, 16
    /* 4E1CC 8005D9CC 40180200 */  sll        $v1, $v0, 1
    /* 4E1D0 8005D9D0 0000C496 */  lhu        $a0, 0x0($s6)
    /* 4E1D4 8005D9D4 21287200 */  addu       $a1, $v1, $s2
    /* 4E1D8 8005D9D8 0E004010 */  beqz       $v0, .L8005DA14
    /* 4E1DC 8005D9DC 3000A4A4 */   sh        $a0, 0x30($a1)
    /* 4E1E0 8005D9E0 00140400 */  sll        $v0, $a0, 16
    /* 4E1E4 8005D9E4 03240200 */  sra        $a0, $v0, 16
    /* 4E1E8 8005D9E8 18009100 */  mult       $a0, $s1
    /* 4E1EC 8005D9EC C3170200 */  sra        $v0, $v0, 31
    /* 4E1F0 8005D9F0 10680000 */  mfhi       $t5
    /* 4E1F4 8005D9F4 83180D00 */  sra        $v1, $t5, 2
    /* 4E1F8 8005D9F8 23186200 */  subu       $v1, $v1, $v0
    /* 4E1FC 8005D9FC 80100300 */  sll        $v0, $v1, 2
    /* 4E200 8005DA00 21104300 */  addu       $v0, $v0, $v1
    /* 4E204 8005DA04 40100200 */  sll        $v0, $v0, 1
    /* 4E208 8005DA08 23208200 */  subu       $a0, $a0, $v0
    /* 4E20C 8005DA0C 9A760108 */  j          .L8005DA68
    /* 4E210 8005DA10 3000A4A4 */   sh        $a0, 0x30($a1)
  .L8005DA14:
    /* 4E214 8005DA14 4800A297 */  lhu        $v0, 0x48($sp)
    /* 4E218 8005DA18 00000000 */  nop
    /* 4E21C 8005DA1C 00140200 */  sll        $v0, $v0, 16
    /* 4E220 8005DA20 031C0200 */  sra        $v1, $v0, 16
    /* 4E224 8005DA24 18007100 */  mult       $v1, $s1
    /* 4E228 8005DA28 C3170200 */  sra        $v0, $v0, 31
    /* 4E22C 8005DA2C 10600000 */  mfhi       $t4
    /* 4E230 8005DA30 83180C00 */  sra        $v1, $t4, 2
    /* 4E234 8005DA34 23186200 */  subu       $v1, $v1, $v0
    /* 4E238 8005DA38 001C0300 */  sll        $v1, $v1, 16
    /* 4E23C 8005DA3C 032C0300 */  sra        $a1, $v1, 16
    /* 4E240 8005DA40 1800B100 */  mult       $a1, $s1
    /* 4E244 8005DA44 C31F0300 */  sra        $v1, $v1, 31
    /* 4E248 8005DA48 10680000 */  mfhi       $t5
    /* 4E24C 8005DA4C 83200D00 */  sra        $a0, $t5, 2
    /* 4E250 8005DA50 23208300 */  subu       $a0, $a0, $v1
    /* 4E254 8005DA54 80100400 */  sll        $v0, $a0, 2
    /* 4E258 8005DA58 21104400 */  addu       $v0, $v0, $a0
    /* 4E25C 8005DA5C 40100200 */  sll        $v0, $v0, 1
    /* 4E260 8005DA60 2328A200 */  subu       $a1, $a1, $v0
    /* 4E264 8005DA64 4800A5A7 */  sh         $a1, 0x48($sp)
  .L8005DA68:
    /* 4E268 8005DA68 00141000 */  sll        $v0, $s0, 16
    /* 4E26C 8005DA6C 032C0200 */  sra        $a1, $v0, 16
    /* 4E270 8005DA70 40200500 */  sll        $a0, $a1, 1
    /* 4E274 8005DA74 21209200 */  addu       $a0, $a0, $s2
    /* 4E278 8005DA78 30008284 */  lh         $v0, 0x30($a0)
    /* 4E27C 8005DA7C 0A80033C */  lui        $v1, %hi(D_8009B400)
    /* 4E280 8005DA80 00B46324 */  addiu      $v1, $v1, %lo(D_8009B400)
    /* 4E284 8005DA84 C0100200 */  sll        $v0, $v0, 3
    /* 4E288 8005DA88 21104300 */  addu       $v0, $v0, $v1
    /* 4E28C 8005DA8C 1C00A2AF */  sw         $v0, 0x1C($sp)
    /* 4E290 8005DA90 30008284 */  lh         $v0, 0x30($a0)
    /* 4E294 8005DA94 00000000 */  nop
    /* 4E298 8005DA98 06005714 */  bne        $v0, $s7, .L8005DAB4
    /* 4E29C 8005DA9C 00000000 */   nop
    /* 4E2A0 8005DAA0 80100500 */  sll        $v0, $a1, 2
    /* 4E2A4 8005DAA4 21104500 */  addu       $v0, $v0, $a1
    /* 4E2A8 8005DAA8 80100200 */  sll        $v0, $v0, 2
    /* 4E2AC 8005DAAC B0760108 */  j          .L8005DAC0
    /* 4E2B0 8005DAB0 03004224 */   addiu     $v0, $v0, 0x3
  .L8005DAB4:
    /* 4E2B4 8005DAB4 80100500 */  sll        $v0, $a1, 2
    /* 4E2B8 8005DAB8 21104500 */  addu       $v0, $v0, $a1
    /* 4E2BC 8005DABC 80100200 */  sll        $v0, $v0, 2
  .L8005DAC0:
    /* 4E2C0 8005DAC0 3000A2AF */  sw         $v0, 0x30($sp)
    /* 4E2C4 8005DAC4 1C00A38F */  lw         $v1, 0x1C($sp)
    /* 4E2C8 8005DAC8 36770108 */  j          .L8005DCD8
    /* 4E2CC 8005DACC A2010224 */   addiu     $v0, $zero, 0x1A2
  .L8005DAD0:
    /* 4E2D0 8005DAD0 00141000 */  sll        $v0, $s0, 16
    /* 4E2D4 8005DAD4 03140200 */  sra        $v0, $v0, 16
    /* 4E2D8 8005DAD8 40180200 */  sll        $v1, $v0, 1
    /* 4E2DC 8005DADC 0200C492 */  lbu        $a0, 0x2($s6)
    /* 4E2E0 8005DAE0 21287200 */  addu       $a1, $v1, $s2
    /* 4E2E4 8005DAE4 0E004010 */  beqz       $v0, .L8005DB20
    /* 4E2E8 8005DAE8 3000A4A4 */   sh        $a0, 0x30($a1)
    /* 4E2EC 8005DAEC 00140400 */  sll        $v0, $a0, 16
    /* 4E2F0 8005DAF0 03240200 */  sra        $a0, $v0, 16
    /* 4E2F4 8005DAF4 18009100 */  mult       $a0, $s1
    /* 4E2F8 8005DAF8 C3170200 */  sra        $v0, $v0, 31
    /* 4E2FC 8005DAFC 10600000 */  mfhi       $t4
    /* 4E300 8005DB00 83180C00 */  sra        $v1, $t4, 2
    /* 4E304 8005DB04 23186200 */  subu       $v1, $v1, $v0
    /* 4E308 8005DB08 80100300 */  sll        $v0, $v1, 2
    /* 4E30C 8005DB0C 21104300 */  addu       $v0, $v0, $v1
    /* 4E310 8005DB10 40100200 */  sll        $v0, $v0, 1
    /* 4E314 8005DB14 23208200 */  subu       $a0, $a0, $v0
    /* 4E318 8005DB18 DD760108 */  j          .L8005DB74
    /* 4E31C 8005DB1C 3000A4A4 */   sh        $a0, 0x30($a1)
  .L8005DB20:
    /* 4E320 8005DB20 4800A297 */  lhu        $v0, 0x48($sp)
    /* 4E324 8005DB24 00000000 */  nop
    /* 4E328 8005DB28 00140200 */  sll        $v0, $v0, 16
    /* 4E32C 8005DB2C 031C0200 */  sra        $v1, $v0, 16
    /* 4E330 8005DB30 18007100 */  mult       $v1, $s1
    /* 4E334 8005DB34 C3170200 */  sra        $v0, $v0, 31
    /* 4E338 8005DB38 10680000 */  mfhi       $t5
    /* 4E33C 8005DB3C 83180D00 */  sra        $v1, $t5, 2
    /* 4E340 8005DB40 23186200 */  subu       $v1, $v1, $v0
    /* 4E344 8005DB44 001C0300 */  sll        $v1, $v1, 16
    /* 4E348 8005DB48 032C0300 */  sra        $a1, $v1, 16
    /* 4E34C 8005DB4C 1800B100 */  mult       $a1, $s1
    /* 4E350 8005DB50 C31F0300 */  sra        $v1, $v1, 31
    /* 4E354 8005DB54 10600000 */  mfhi       $t4
    /* 4E358 8005DB58 83200C00 */  sra        $a0, $t4, 2
    /* 4E35C 8005DB5C 23208300 */  subu       $a0, $a0, $v1
    /* 4E360 8005DB60 80100400 */  sll        $v0, $a0, 2
    /* 4E364 8005DB64 21104400 */  addu       $v0, $v0, $a0
    /* 4E368 8005DB68 40100200 */  sll        $v0, $v0, 1
    /* 4E36C 8005DB6C 2328A200 */  subu       $a1, $a1, $v0
    /* 4E370 8005DB70 4800A5A7 */  sh         $a1, 0x48($sp)
  .L8005DB74:
    /* 4E374 8005DB74 00141000 */  sll        $v0, $s0, 16
    /* 4E378 8005DB78 032C0200 */  sra        $a1, $v0, 16
    /* 4E37C 8005DB7C 40200500 */  sll        $a0, $a1, 1
    /* 4E380 8005DB80 21209200 */  addu       $a0, $a0, $s2
    /* 4E384 8005DB84 30008284 */  lh         $v0, 0x30($a0)
    /* 4E388 8005DB88 0A80033C */  lui        $v1, %hi(D_8009B400)
    /* 4E38C 8005DB8C 00B46324 */  addiu      $v1, $v1, %lo(D_8009B400)
    /* 4E390 8005DB90 C0100200 */  sll        $v0, $v0, 3
    /* 4E394 8005DB94 21104300 */  addu       $v0, $v0, $v1
    /* 4E398 8005DB98 1C00A2AF */  sw         $v0, 0x1C($sp)
    /* 4E39C 8005DB9C 30008284 */  lh         $v0, 0x30($a0)
    /* 4E3A0 8005DBA0 00000000 */  nop
    /* 4E3A4 8005DBA4 06005714 */  bne        $v0, $s7, .L8005DBC0
    /* 4E3A8 8005DBA8 00000000 */   nop
    /* 4E3AC 8005DBAC 80100500 */  sll        $v0, $a1, 2
    /* 4E3B0 8005DBB0 21104500 */  addu       $v0, $v0, $a1
    /* 4E3B4 8005DBB4 80100200 */  sll        $v0, $v0, 2
    /* 4E3B8 8005DBB8 F3760108 */  j          .L8005DBCC
    /* 4E3BC 8005DBBC 03004224 */   addiu     $v0, $v0, 0x3
  .L8005DBC0:
    /* 4E3C0 8005DBC0 80100500 */  sll        $v0, $a1, 2
    /* 4E3C4 8005DBC4 21104500 */  addu       $v0, $v0, $a1
    /* 4E3C8 8005DBC8 80100200 */  sll        $v0, $v0, 2
  .L8005DBCC:
    /* 4E3CC 8005DBCC 3000A2AF */  sw         $v0, 0x30($sp)
    /* 4E3D0 8005DBD0 1C00A38F */  lw         $v1, 0x1C($sp)
    /* 4E3D4 8005DBD4 36770108 */  j          .L8005DCD8
    /* 4E3D8 8005DBD8 D3010224 */   addiu     $v0, $zero, 0x1D3
  .L8005DBDC:
    /* 4E3DC 8005DBDC 03140200 */  sra        $v0, $v0, 16
    /* 4E3E0 8005DBE0 40180200 */  sll        $v1, $v0, 1
    /* 4E3E4 8005DBE4 0300C492 */  lbu        $a0, 0x3($s6)
    /* 4E3E8 8005DBE8 21287200 */  addu       $a1, $v1, $s2
    /* 4E3EC 8005DBEC 0E004010 */  beqz       $v0, .L8005DC28
    /* 4E3F0 8005DBF0 3000A4A4 */   sh        $a0, 0x30($a1)
    /* 4E3F4 8005DBF4 00140400 */  sll        $v0, $a0, 16
    /* 4E3F8 8005DBF8 03240200 */  sra        $a0, $v0, 16
    /* 4E3FC 8005DBFC 18009100 */  mult       $a0, $s1
    /* 4E400 8005DC00 C3170200 */  sra        $v0, $v0, 31
    /* 4E404 8005DC04 10680000 */  mfhi       $t5
    /* 4E408 8005DC08 83180D00 */  sra        $v1, $t5, 2
    /* 4E40C 8005DC0C 23186200 */  subu       $v1, $v1, $v0
    /* 4E410 8005DC10 80100300 */  sll        $v0, $v1, 2
    /* 4E414 8005DC14 21104300 */  addu       $v0, $v0, $v1
    /* 4E418 8005DC18 40100200 */  sll        $v0, $v0, 1
    /* 4E41C 8005DC1C 23208200 */  subu       $a0, $a0, $v0
    /* 4E420 8005DC20 1F770108 */  j          .L8005DC7C
    /* 4E424 8005DC24 3000A4A4 */   sh        $a0, 0x30($a1)
  .L8005DC28:
    /* 4E428 8005DC28 4800A297 */  lhu        $v0, 0x48($sp)
    /* 4E42C 8005DC2C 00000000 */  nop
    /* 4E430 8005DC30 00140200 */  sll        $v0, $v0, 16
    /* 4E434 8005DC34 031C0200 */  sra        $v1, $v0, 16
    /* 4E438 8005DC38 18007100 */  mult       $v1, $s1
    /* 4E43C 8005DC3C C3170200 */  sra        $v0, $v0, 31
    /* 4E440 8005DC40 10600000 */  mfhi       $t4
    /* 4E444 8005DC44 83180C00 */  sra        $v1, $t4, 2
    /* 4E448 8005DC48 23186200 */  subu       $v1, $v1, $v0
    /* 4E44C 8005DC4C 001C0300 */  sll        $v1, $v1, 16
    /* 4E450 8005DC50 032C0300 */  sra        $a1, $v1, 16
    /* 4E454 8005DC54 1800B100 */  mult       $a1, $s1
    /* 4E458 8005DC58 C31F0300 */  sra        $v1, $v1, 31
    /* 4E45C 8005DC5C 10680000 */  mfhi       $t5
    /* 4E460 8005DC60 83200D00 */  sra        $a0, $t5, 2
    /* 4E464 8005DC64 23208300 */  subu       $a0, $a0, $v1
    /* 4E468 8005DC68 80100400 */  sll        $v0, $a0, 2
    /* 4E46C 8005DC6C 21104400 */  addu       $v0, $v0, $a0
    /* 4E470 8005DC70 40100200 */  sll        $v0, $v0, 1
    /* 4E474 8005DC74 2328A200 */  subu       $a1, $a1, $v0
    /* 4E478 8005DC78 4800A5A7 */  sh         $a1, 0x48($sp)
  .L8005DC7C:
    /* 4E47C 8005DC7C 00141000 */  sll        $v0, $s0, 16
    /* 4E480 8005DC80 032C0200 */  sra        $a1, $v0, 16
    /* 4E484 8005DC84 40200500 */  sll        $a0, $a1, 1
    /* 4E488 8005DC88 21209200 */  addu       $a0, $a0, $s2
    /* 4E48C 8005DC8C 30008284 */  lh         $v0, 0x30($a0)
    /* 4E490 8005DC90 0A80033C */  lui        $v1, %hi(D_8009B400)
    /* 4E494 8005DC94 00B46324 */  addiu      $v1, $v1, %lo(D_8009B400)
    /* 4E498 8005DC98 C0100200 */  sll        $v0, $v0, 3
    /* 4E49C 8005DC9C 21104300 */  addu       $v0, $v0, $v1
    /* 4E4A0 8005DCA0 1C00A2AF */  sw         $v0, 0x1C($sp)
    /* 4E4A4 8005DCA4 30008284 */  lh         $v0, 0x30($a0)
    /* 4E4A8 8005DCA8 00000000 */  nop
    /* 4E4AC 8005DCAC 05005714 */  bne        $v0, $s7, .L8005DCC4
    /* 4E4B0 8005DCB0 80100500 */   sll       $v0, $a1, 2
    /* 4E4B4 8005DCB4 21104500 */  addu       $v0, $v0, $a1
    /* 4E4B8 8005DCB8 80100200 */  sll        $v0, $v0, 2
    /* 4E4BC 8005DCBC 33770108 */  j          .L8005DCCC
    /* 4E4C0 8005DCC0 03004224 */   addiu     $v0, $v0, 0x3
  .L8005DCC4:
    /* 4E4C4 8005DCC4 21104500 */  addu       $v0, $v0, $a1
    /* 4E4C8 8005DCC8 80100200 */  sll        $v0, $v0, 2
  .L8005DCCC:
    /* 4E4CC 8005DCCC 3000A2AF */  sw         $v0, 0x30($sp)
    /* 4E4D0 8005DCD0 1C00A38F */  lw         $v1, 0x1C($sp)
    /* 4E4D4 8005DCD4 09020224 */  addiu      $v0, $zero, 0x209
  .L8005DCD8:
    /* 4E4D8 8005DCD8 000062A4 */  sh         $v0, 0x0($v1)
    /* 4E4DC 8005DCDC 2000B3AF */  sw         $s3, 0x20($sp)
  .L8005DCE0:
    /* 4E4E0 8005DCE0 4BCD010C */  jal        func_8007352C
    /* 4E4E4 8005DCE4 1800A427 */   addiu     $a0, $sp, 0x18
    /* 4E4E8 8005DCE8 21984000 */  addu       $s3, $v0, $zero
    /* 4E4EC 8005DCEC 01000226 */  addiu      $v0, $s0, 0x1
    /* 4E4F0 8005DCF0 21804000 */  addu       $s0, $v0, $zero
    /* 4E4F4 8005DCF4 00140200 */  sll        $v0, $v0, 16
    /* 4E4F8 8005DCF8 03140200 */  sra        $v0, $v0, 16
    /* 4E4FC 8005DCFC 02004228 */  slti       $v0, $v0, 0x2
    /* 4E500 8005DD00 24FF4014 */  bnez       $v0, .L8005D994
    /* 4E504 8005DD04 01008226 */   addiu     $v0, $s4, 0x1
    /* 4E508 8005DD08 21A04000 */  addu       $s4, $v0, $zero
    /* 4E50C 8005DD0C 00140200 */  sll        $v0, $v0, 16
    /* 4E510 8005DD10 03140200 */  sra        $v0, $v0, 16
    /* 4E514 8005DD14 03004228 */  slti       $v0, $v0, 0x3
    /* 4E518 8005DD18 1CFF4014 */  bnez       $v0, .L8005D98C
    /* 4E51C 8005DD1C 21800000 */   addu      $s0, $zero, $zero
    /* 4E520 8005DD20 6666023C */  lui        $v0, (0x66666667 >> 16)
    /* 4E524 8005DD24 5000AC8F */  lw         $t4, 0x50($sp)
    /* 4E528 8005DD28 67664234 */  ori        $v0, $v0, (0x66666667 & 0xFFFF)
    /* 4E52C 8005DD2C 004C0C00 */  sll        $t1, $t4, 16
    /* 4E530 8005DD30 03440900 */  sra        $t0, $t1, 16
    /* 4E534 8005DD34 18000201 */  mult       $t0, $v0
    /* 4E538 8005DD38 C34F0900 */  sra        $t1, $t1, 31
    /* 4E53C 8005DD3C 10180000 */  mfhi       $v1
    /* 4E540 8005DD40 83300300 */  sra        $a2, $v1, 2
    /* 4E544 8005DD44 2330C900 */  subu       $a2, $a2, $t1
    /* 4E548 8005DD48 00240600 */  sll        $a0, $a2, 16
    /* 4E54C 8005DD4C 033C0400 */  sra        $a3, $a0, 16
    /* 4E550 8005DD50 1800E200 */  mult       $a3, $v0
    /* 4E554 8005DD54 EB510A3C */  lui        $t2, (0x51EB851F >> 16)
    /* 4E558 8005DD58 1F854A35 */  ori        $t2, $t2, (0x51EB851F & 0xFFFF)
    /* 4E55C 8005DD5C 21A00000 */  addu       $s4, $zero, $zero
    /* 4E560 8005DD60 1800B127 */  addiu      $s1, $sp, 0x18
    /* 4E564 8005DD64 0A80023C */  lui        $v0, %hi(D_8009B398)
    /* 4E568 8005DD68 98B34224 */  addiu      $v0, $v0, %lo(D_8009B398)
    /* 4E56C 8005DD6C 10180000 */  mfhi       $v1
    /* 4E570 8005DD70 1800A2AF */  sw         $v0, 0x18($sp)
    /* 4E574 8005DD74 5000A297 */  lhu        $v0, 0x50($sp)
    /* 4E578 8005DD78 18000A01 */  mult       $t0, $t2
    /* 4E57C 8005DD7C C3270400 */  sra        $a0, $a0, 31
    /* 4E580 8005DD80 4C00A2A7 */  sh         $v0, 0x4C($sp)
    /* 4E584 8005DD84 4A00A2A7 */  sh         $v0, 0x4A($sp)
    /* 4E588 8005DD88 4800A2A7 */  sh         $v0, 0x48($sp)
    /* 4E58C 8005DD8C 83180300 */  sra        $v1, $v1, 2
    /* 4E590 8005DD90 23186400 */  subu       $v1, $v1, $a0
    /* 4E594 8005DD94 80100300 */  sll        $v0, $v1, 2
    /* 4E598 8005DD98 21104300 */  addu       $v0, $v0, $v1
    /* 4E59C 8005DD9C 40100200 */  sll        $v0, $v0, 1
    /* 4E5A0 8005DDA0 2338E200 */  subu       $a3, $a3, $v0
    /* 4E5A4 8005DDA4 00240700 */  sll        $a0, $a3, 16
    /* 4E5A8 8005DDA8 10580000 */  mfhi       $t3
    /* 4E5AC 8005DDAC 032C0400 */  sra        $a1, $a0, 16
    /* 4E5B0 8005DDB0 29000224 */  addiu      $v0, $zero, 0x29
    /* 4E5B4 8005DDB4 1800AA00 */  mult       $a1, $t2
    /* 4E5B8 8005DDB8 3400A2AF */  sw         $v0, 0x34($sp)
    /* 4E5BC 8005DDBC 80100600 */  sll        $v0, $a2, 2
    /* 4E5C0 8005DDC0 21104600 */  addu       $v0, $v0, $a2
    /* 4E5C4 8005DDC4 40100200 */  sll        $v0, $v0, 1
    /* 4E5C8 8005DDC8 23400201 */  subu       $t0, $t0, $v0
    /* 4E5CC 8005DDCC C3270400 */  sra        $a0, $a0, 31
    /* 4E5D0 8005DDD0 4C00A8A7 */  sh         $t0, 0x4C($sp)
    /* 4E5D4 8005DDD4 4A00A7A7 */  sh         $a3, 0x4A($sp)
    /* 4E5D8 8005DDD8 43110B00 */  sra        $v0, $t3, 5
    /* 4E5DC 8005DDDC 23104900 */  subu       $v0, $v0, $t1
    /* 4E5E0 8005DDE0 4800A2A7 */  sh         $v0, 0x48($sp)
    /* 4E5E4 8005DDE4 10180000 */  mfhi       $v1
    /* 4E5E8 8005DDE8 43190300 */  sra        $v1, $v1, 5
    /* 4E5EC 8005DDEC 23186400 */  subu       $v1, $v1, $a0
    /* 4E5F0 8005DDF0 40100300 */  sll        $v0, $v1, 1
    /* 4E5F4 8005DDF4 21104300 */  addu       $v0, $v0, $v1
    /* 4E5F8 8005DDF8 C0100200 */  sll        $v0, $v0, 3
    /* 4E5FC 8005DDFC 21104300 */  addu       $v0, $v0, $v1
    /* 4E600 8005DE00 80100200 */  sll        $v0, $v0, 2
    /* 4E604 8005DE04 2328A200 */  subu       $a1, $a1, $v0
    /* 4E608 8005DE08 4A00A5A7 */  sh         $a1, 0x4A($sp)
    /* 4E60C 8005DE0C 21100002 */  addu       $v0, $s0, $zero
  .L8005DE10:
    /* 4E610 8005DE10 0C004014 */  bnez       $v0, .L8005DE44
    /* 4E614 8005DE14 00141400 */   sll       $v0, $s4, 16
    /* 4E618 8005DE18 031C0200 */  sra        $v1, $v0, 16
    /* 4E61C 8005DE1C 40100300 */  sll        $v0, $v1, 1
    /* 4E620 8005DE20 21105100 */  addu       $v0, $v0, $s1
    /* 4E624 8005DE24 30004284 */  lh         $v0, 0x30($v0)
    /* 4E628 8005DE28 00000000 */  nop
    /* 4E62C 8005DE2C 05004014 */  bnez       $v0, .L8005DE44
    /* 4E630 8005DE30 00141400 */   sll       $v0, $s4, 16
    /* 4E634 8005DE34 02000224 */  addiu      $v0, $zero, 0x2
    /* 4E638 8005DE38 21006214 */  bne        $v1, $v0, .L8005DEC0
    /* 4E63C 8005DE3C 01008226 */   addiu     $v0, $s4, 0x1
    /* 4E640 8005DE40 00141400 */  sll        $v0, $s4, 16
  .L8005DE44:
    /* 4E644 8005DE44 032C0200 */  sra        $a1, $v0, 16
    /* 4E648 8005DE48 40200500 */  sll        $a0, $a1, 1
    /* 4E64C 8005DE4C 21209100 */  addu       $a0, $a0, $s1
    /* 4E650 8005DE50 30008384 */  lh         $v1, 0x30($a0)
    /* 4E654 8005DE54 0A80023C */  lui        $v0, %hi(D_8009B400)
    /* 4E658 8005DE58 00B44224 */  addiu      $v0, $v0, %lo(D_8009B400)
    /* 4E65C 8005DE5C C0180300 */  sll        $v1, $v1, 3
    /* 4E660 8005DE60 21186200 */  addu       $v1, $v1, $v0
    /* 4E664 8005DE64 F3010224 */  addiu      $v0, $zero, 0x1F3
    /* 4E668 8005DE68 1C00A3AF */  sw         $v1, 0x1C($sp)
    /* 4E66C 8005DE6C 000062A4 */  sh         $v0, 0x0($v1)
    /* 4E670 8005DE70 30008384 */  lh         $v1, 0x30($a0)
    /* 4E674 8005DE74 01000224 */  addiu      $v0, $zero, 0x1
    /* 4E678 8005DE78 07006214 */  bne        $v1, $v0, .L8005DE98
    /* 4E67C 8005DE7C 01001024 */   addiu     $s0, $zero, 0x1
    /* 4E680 8005DE80 80100500 */  sll        $v0, $a1, 2
    /* 4E684 8005DE84 21104500 */  addu       $v0, $v0, $a1
    /* 4E688 8005DE88 80100200 */  sll        $v0, $v0, 2
    /* 4E68C 8005DE8C 21104500 */  addu       $v0, $v0, $a1
    /* 4E690 8005DE90 AA770108 */  j          .L8005DEA8
    /* 4E694 8005DE94 03004224 */   addiu     $v0, $v0, 0x3
  .L8005DE98:
    /* 4E698 8005DE98 80100500 */  sll        $v0, $a1, 2
    /* 4E69C 8005DE9C 21104500 */  addu       $v0, $v0, $a1
    /* 4E6A0 8005DEA0 80100200 */  sll        $v0, $v0, 2
    /* 4E6A4 8005DEA4 21104500 */  addu       $v0, $v0, $a1
  .L8005DEA8:
    /* 4E6A8 8005DEA8 3000A2AF */  sw         $v0, 0x30($sp)
    /* 4E6AC 8005DEAC 2000B3AF */  sw         $s3, 0x20($sp)
    /* 4E6B0 8005DEB0 4BCD010C */  jal        func_8007352C
    /* 4E6B4 8005DEB4 1800A427 */   addiu     $a0, $sp, 0x18
    /* 4E6B8 8005DEB8 21984000 */  addu       $s3, $v0, $zero
    /* 4E6BC 8005DEBC 01008226 */  addiu      $v0, $s4, 0x1
  .L8005DEC0:
    /* 4E6C0 8005DEC0 21A04000 */  addu       $s4, $v0, $zero
    /* 4E6C4 8005DEC4 00140200 */  sll        $v0, $v0, 16
    /* 4E6C8 8005DEC8 03140200 */  sra        $v0, $v0, 16
    /* 4E6CC 8005DECC 03004228 */  slti       $v0, $v0, 0x3
    /* 4E6D0 8005DED0 CFFF4014 */  bnez       $v0, .L8005DE10
    /* 4E6D4 8005DED4 21100002 */   addu      $v0, $s0, $zero
    /* 4E6D8 8005DED8 21A00000 */  addu       $s4, $zero, $zero
    /* 4E6DC 8005DEDC 10001524 */  addiu      $s5, $zero, 0x10
    /* 4E6E0 8005DEE0 0A80173C */  lui        $s7, %hi(D_8009B3B0)
    /* 4E6E4 8005DEE4 B0B3F726 */  addiu      $s7, $s7, %lo(D_8009B3B0)
    /* 4E6E8 8005DEE8 0A80163C */  lui        $s6, %hi(D_8009B3F0)
    /* 4E6EC 8005DEEC F0B3D626 */  addiu      $s6, $s6, %lo(D_8009B3F0)
    /* 4E6F0 8005DEF0 0E00D127 */  addiu      $s1, $fp, 0xE
    /* 4E6F4 8005DEF4 6000AC8F */  lw         $t4, 0x60($sp)
    /* 4E6F8 8005DEF8 FF000224 */  addiu      $v0, $zero, 0xFF
    /* 4E6FC 8005DEFC 4100A2A3 */  sb         $v0, 0x41($sp)
    /* 4E700 8005DF00 10000224 */  addiu      $v0, $zero, 0x10
    /* 4E704 8005DF04 4300A2A3 */  sb         $v0, 0x43($sp)
    /* 4E708 8005DF08 4200A2A3 */  sb         $v0, 0x42($sp)
    /* 4E70C 8005DF0C 01000224 */  addiu      $v0, $zero, 0x1
    /* 4E710 8005DF10 4000A2A3 */  sb         $v0, 0x40($sp)
    /* 4E714 8005DF14 3000A0AF */  sw         $zero, 0x30($sp)
    /* 4E718 8005DF18 2800A0AF */  sw         $zero, 0x28($sp)
    /* 4E71C 8005DF1C 2C00ACAF */  sw         $t4, 0x2C($sp)
  .L8005DF20:
    /* 4E720 8005DF20 B4EA010C */  jal        initTile
    /* 4E724 8005DF24 2120C003 */   addu      $a0, $fp, $zero
    /* 4E728 8005DF28 FF000224 */  addiu      $v0, $zero, 0xFF
    /* 4E72C 8005DF2C 00841400 */  sll        $s0, $s4, 16
    /* 4E730 8005DF30 83831000 */  sra        $s0, $s0, 14
    /* 4E734 8005DF34 F6FF22A2 */  sb         $v0, -0xA($s1)
    /* 4E738 8005DF38 F7FF35A2 */  sb         $s5, -0x9($s1)
    /* 4E73C 8005DF3C F8FF35A2 */  sb         $s5, -0x8($s1)
    /* 4E740 8005DF40 0A80013C */  lui        $at, %hi(D_8009B450)
    /* 4E744 8005DF44 21083000 */  addu       $at, $at, $s0
    /* 4E748 8005DF48 50B42294 */  lhu        $v0, %lo(D_8009B450)($at)
    /* 4E74C 8005DF4C 2120C003 */  addu       $a0, $fp, $zero
    /* 4E750 8005DF50 FAFF22A6 */  sh         $v0, -0x6($s1)
    /* 4E754 8005DF54 0A80013C */  lui        $at, %hi(D_8009B452)
    /* 4E758 8005DF58 21083000 */  addu       $at, $at, $s0
    /* 4E75C 8005DF5C 52B42294 */  lhu        $v0, %lo(D_8009B452)($at)
    /* 4E760 8005DF60 21280000 */  addu       $a1, $zero, $zero
    /* 4E764 8005DF64 FCFF22A6 */  sh         $v0, -0x4($s1)
    /* 4E768 8005DF68 0A80013C */  lui        $at, %hi(D_8009B450)
    /* 4E76C 8005DF6C 21083000 */  addu       $at, $at, $s0
    /* 4E770 8005DF70 50B42394 */  lhu        $v1, %lo(D_8009B450)($at)
    /* 4E774 8005DF74 01000224 */  addiu      $v0, $zero, 0x1
    /* 4E778 8005DF78 000022A6 */  sh         $v0, 0x0($s1)
    /* 4E77C 8005DF7C 38020224 */  addiu      $v0, $zero, 0x238
    /* 4E780 8005DF80 23104300 */  subu       $v0, $v0, $v1
    /* 4E784 8005DF84 5AEA010C */  jal        gpu_SetSemiTransp
    /* 4E788 8005DF88 FEFF22A6 */   sh        $v0, -0x2($s1)
    /* 4E78C 8005DF8C 2128C003 */  addu       $a1, $fp, $zero
    /* 4E790 8005DF90 10003126 */  addiu      $s1, $s1, 0x10
    /* 4E794 8005DF94 1000DE27 */  addiu      $fp, $fp, 0x10
    /* 4E798 8005DF98 6000AD8F */  lw         $t5, 0x60($sp)
    /* 4E79C 8005DF9C 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 4E7A0 8005DFA0 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 4E7A4 8005DFA4 80900D00 */  sll        $s2, $t5, 2
    /* 4E7A8 8005DFA8 2DEA010C */  jal        ot_Link
    /* 4E7AC 8005DFAC 21209200 */   addu      $a0, $a0, $s2
    /* 4E7B0 8005DFB0 0A80013C */  lui        $at, %hi(D_8009B452)
    /* 4E7B4 8005DFB4 21083000 */  addu       $at, $at, $s0
    /* 4E7B8 8005DFB8 52B42284 */  lh         $v0, %lo(D_8009B452)($at)
    /* 4E7BC 8005DFBC 1800A427 */  addiu      $a0, $sp, 0x18
    /* 4E7C0 8005DFC0 1800B7AF */  sw         $s7, 0x18($sp)
    /* 4E7C4 8005DFC4 1C00B6AF */  sw         $s6, 0x1C($sp)
    /* 4E7C8 8005DFC8 2000B3AF */  sw         $s3, 0x20($sp)
    /* 4E7CC 8005DFCC 4BCD010C */  jal        func_8007352C
    /* 4E7D0 8005DFD0 3400A2AF */   sw        $v0, 0x34($sp)
    /* 4E7D4 8005DFD4 1800A427 */  addiu      $a0, $sp, 0x18
    /* 4E7D8 8005DFD8 0A800C3C */  lui        $t4, %hi(D_8009B3BC)
    /* 4E7DC 8005DFDC BCB38C25 */  addiu      $t4, $t4, %lo(D_8009B3BC)
    /* 4E7E0 8005DFE0 0A800D3C */  lui        $t5, %hi(D_8009B3F8)
    /* 4E7E4 8005DFE4 F8B3AD25 */  addiu      $t5, $t5, %lo(D_8009B3F8)
    /* 4E7E8 8005DFE8 1800ACAF */  sw         $t4, 0x18($sp)
    /* 4E7EC 8005DFEC 1C00ADAF */  sw         $t5, 0x1C($sp)
    /* 4E7F0 8005DFF0 4BCD010C */  jal        func_8007352C
    /* 4E7F4 8005DFF4 2000A2AF */   sw        $v0, 0x20($sp)
    /* 4E7F8 8005DFF8 21984000 */  addu       $s3, $v0, $zero
    /* 4E7FC 8005DFFC 01008226 */  addiu      $v0, $s4, 0x1
    /* 4E800 8005E000 21A04000 */  addu       $s4, $v0, $zero
    /* 4E804 8005E004 00140200 */  sll        $v0, $v0, 16
    /* 4E808 8005E008 03140200 */  sra        $v0, $v0, 16
    /* 4E80C 8005E00C 02004228 */  slti       $v0, $v0, 0x2
    /* 4E810 8005E010 C3FF4014 */  bnez       $v0, .L8005DF20
    /* 4E814 8005E014 00000000 */   nop
    /* 4E818 8005E018 0A80043C */  lui        $a0, %hi(D_8009B398)
    /* 4E81C 8005E01C 98B38424 */  addiu      $a0, $a0, %lo(D_8009B398)
    /* 4E820 8005E020 20B9010C */  jal        func_8006E480
    /* 4E824 8005E024 21280000 */   addu      $a1, $zero, $zero
    /* 4E828 8005E028 01000524 */  addiu      $a1, $zero, 0x1
    /* 4E82C 8005E02C 21300000 */  addu       $a2, $zero, $zero
    /* 4E830 8005E030 6800A48F */  lw         $a0, 0x68($sp)
    /* 4E834 8005E034 21384000 */  addu       $a3, $v0, $zero
    /* 4E838 8005E038 92F0010C */  jal        initTexPage
    /* 4E83C 8005E03C 1000A0AF */   sw        $zero, 0x10($sp)
    /* 4E840 8005E040 0A80043C */  lui        $a0, %hi(D_800A374C)
    /* 4E844 8005E044 4C37848C */  lw         $a0, %lo(D_800A374C)($a0)
    /* 4E848 8005E048 6800A58F */  lw         $a1, 0x68($sp)
    /* 4E84C 8005E04C 2DEA010C */  jal        ot_Link
    /* 4E850 8005E050 21209200 */   addu      $a0, $a0, $s2
    /* 4E854 8005E054 7000AC8F */  lw         $t4, 0x70($sp)
    /* 4E858 8005E058 5800AD8F */  lw         $t5, 0x58($sp)
    /* 4E85C 8005E05C 00000000 */  nop
    /* 4E860 8005E060 23108D01 */  subu       $v0, $t4, $t5
    /* 4E864 8005E064 9C00BF8F */  lw         $ra, 0x9C($sp)
    /* 4E868 8005E068 9800BE8F */  lw         $fp, 0x98($sp)
    /* 4E86C 8005E06C 9400B78F */  lw         $s7, 0x94($sp)
    /* 4E870 8005E070 9000B68F */  lw         $s6, 0x90($sp)
    /* 4E874 8005E074 8C00B58F */  lw         $s5, 0x8C($sp)
    /* 4E878 8005E078 8800B48F */  lw         $s4, 0x88($sp)
    /* 4E87C 8005E07C 8400B38F */  lw         $s3, 0x84($sp)
    /* 4E880 8005E080 8000B28F */  lw         $s2, 0x80($sp)
    /* 4E884 8005E084 7C00B18F */  lw         $s1, 0x7C($sp)
    /* 4E888 8005E088 7800B08F */  lw         $s0, 0x78($sp)
    /* 4E88C 8005E08C A000BD27 */  addiu      $sp, $sp, 0xA0
    /* 4E890 8005E090 0800E003 */  jr         $ra
    /* 4E894 8005E094 00000000 */   nop
endlabel func_8005D814
