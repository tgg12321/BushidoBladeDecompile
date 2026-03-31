glabel func_8001E878
    /* F078 8001E878 D0FFBD27 */  addiu      $sp, $sp, -0x30
    /* F07C 8001E87C 2C00BFAF */  sw         $ra, 0x2C($sp)
    /* F080 8001E880 AC1C010C */  jal        camera_GetBoneData
    /* F084 8001E884 2800B0AF */   sw        $s0, 0x28($sp)
    /* F088 8001E888 1080043C */  lui        $a0, %hi(D_80102030)
    /* F08C 8001E88C 30208424 */  addiu      $a0, $a0, %lo(D_80102030)
    /* F090 8001E890 4C048524 */  addiu      $a1, $a0, 0x44C
    /* F094 8001E894 98FE9024 */  addiu      $s0, $a0, -0x168
    /* F098 8001E898 21300002 */  addu       $a2, $s0, $zero
    /* F09C 8001E89C 0A80013C */  lui        $at, %hi(D_800A3778)
    /* F0A0 8001E8A0 783722AC */  sw         $v0, %lo(D_800A3778)($at)
    /* F0A4 8001E8A4 086A000C */  jal        func_8001A820
    /* F0A8 8001E8A8 E4028724 */   addiu     $a3, $a0, 0x2E4
    /* F0AC 8001E8AC 0A80023C */  lui        $v0, %hi(D_800A38BA)
    /* F0B0 8001E8B0 BA384284 */  lh         $v0, %lo(D_800A38BA)($v0)
    /* F0B4 8001E8B4 00000000 */  nop
    /* F0B8 8001E8B8 0D004010 */  beqz       $v0, .L8001E8F0
    /* F0BC 8001E8BC 00000000 */   nop
    /* F0C0 8001E8C0 0A80023C */  lui        $v0, %hi(D_800A36F6)
    /* F0C4 8001E8C4 F6364284 */  lh         $v0, %lo(D_800A36F6)($v0)
    /* F0C8 8001E8C8 00000000 */  nop
    /* F0CC 8001E8CC 00210200 */  sll        $a0, $v0, 4
    /* F0D0 8001E8D0 21208200 */  addu       $a0, $a0, $v0
    /* F0D4 8001E8D4 80200400 */  sll        $a0, $a0, 2
    /* F0D8 8001E8D8 21208200 */  addu       $a0, $a0, $v0
    /* F0DC 8001E8DC 80200400 */  sll        $a0, $a0, 2
    /* F0E0 8001E8E0 23208200 */  subu       $a0, $a0, $v0
    /* F0E4 8001E8E4 80200400 */  sll        $a0, $a0, 2
    /* F0E8 8001E8E8 1E6D000C */  jal        func_8001B478
    /* F0EC 8001E8EC 21209000 */   addu      $a0, $a0, $s0
  .L8001E8F0:
    /* F0F0 8001E8F0 0179000C */  jal        func_8001E404
    /* F0F4 8001E8F4 00000000 */   nop
    /* F0F8 8001E8F8 C8E4000C */  jal        func_80039320
    /* F0FC 8001E8FC 00000000 */   nop
    /* F100 8001E900 1B80000C */  jal        func_8002006C
    /* F104 8001E904 00000000 */   nop
    /* F108 8001E908 21200000 */  addu       $a0, $zero, $zero
    /* F10C 8001E90C 886F000C */  jal        func_8001BE20
    /* F110 8001E910 1000A527 */   addiu     $a1, $sp, 0x10
    /* F114 8001E914 21200000 */  addu       $a0, $zero, $zero
    /* F118 8001E918 C28F000C */  jal        func_80023F08
    /* F11C 8001E91C 1000A527 */   addiu     $a1, $sp, 0x10
    /* F120 8001E920 01000424 */  addiu      $a0, $zero, 0x1
    /* F124 8001E924 886F000C */  jal        func_8001BE20
    /* F128 8001E928 1000A527 */   addiu     $a1, $sp, 0x10
    /* F12C 8001E92C 01000424 */  addiu      $a0, $zero, 0x1
    /* F130 8001E930 C28F000C */  jal        func_80023F08
    /* F134 8001E934 1000A527 */   addiu     $a1, $sp, 0x10
    /* F138 8001E938 87B1000C */  jal        func_8002C61C
    /* F13C 8001E93C 00000000 */   nop
    /* F140 8001E940 5FC3000C */  jal        func_80030D7C
    /* F144 8001E944 00000000 */   nop
    /* F148 8001E948 7AC8000C */  jal        func_800321E8
    /* F14C 8001E94C 00000000 */   nop
    /* F150 8001E950 E8E5000C */  jal        func_800397A0
    /* F154 8001E954 00000000 */   nop
    /* F158 8001E958 0A80023C */  lui        $v0, %hi(D_800A38BA)
    /* F15C 8001E95C BA384284 */  lh         $v0, %lo(D_800A38BA)($v0)
    /* F160 8001E960 00000000 */  nop
    /* F164 8001E964 0A004010 */  beqz       $v0, .L8001E990
    /* F168 8001E968 00000000 */   nop
    /* F16C 8001E96C 0A80023C */  lui        $v0, %hi(D_800A36FA)
    /* F170 8001E970 FA364290 */  lbu        $v0, %lo(D_800A36FA)($v0)
    /* F174 8001E974 00000000 */  nop
    /* F178 8001E978 05004014 */  bnez       $v0, .L8001E990
    /* F17C 8001E97C 00000000 */   nop
    /* F180 8001E980 007A000C */  jal        func_8001E800
    /* F184 8001E984 00000000 */   nop
    /* F188 8001E988 707A0008 */  j          .L8001E9C0
    /* F18C 8001E98C 00000000 */   nop
  .L8001E990:
    /* F190 8001E990 1080043C */  lui        $a0, %hi(D_80101FBC)
    /* F194 8001E994 BC1F848C */  lw         $a0, %lo(D_80101FBC)($a0)
    /* F198 8001E998 1080053C */  lui        $a1, %hi(D_80101FC4)
    /* F19C 8001E99C C41FA58C */  lw         $a1, %lo(D_80101FC4)($a1)
    /* F1A0 8001E9A0 A8F9000C */  jal        func_8003E6A0
    /* F1A4 8001E9A4 00000000 */   nop
    /* F1A8 8001E9A8 1080043C */  lui        $a0, %hi(D_80102408)
    /* F1AC 8001E9AC 0824848C */  lw         $a0, %lo(D_80102408)($a0)
    /* F1B0 8001E9B0 1080053C */  lui        $a1, %hi(D_80102410)
    /* F1B4 8001E9B4 1024A58C */  lw         $a1, %lo(D_80102410)($a1)
    /* F1B8 8001E9B8 A8F9000C */  jal        func_8003E6A0
    /* F1BC 8001E9BC 00000000 */   nop
  .L8001E9C0:
    /* F1C0 8001E9C0 0A80043C */  lui        $a0, %hi(D_800A3690)
    /* F1C4 8001E9C4 90368490 */  lbu        $a0, %lo(D_800A3690)($a0)
    /* F1C8 8001E9C8 00000000 */  nop
    /* F1CC 8001E9CC 01008438 */  xori       $a0, $a0, 0x1
    /* F1D0 8001E9D0 6A1B010C */  jal        game_StageInit
    /* F1D4 8001E9D4 2B200400 */   sltu      $a0, $zero, $a0
    /* F1D8 8001E9D8 9873000C */  jal        func_8001CE60
    /* F1DC 8001E9DC 00000000 */   nop
    /* F1E0 8001E9E0 76CD000C */  jal        func_800335D8
    /* F1E4 8001E9E4 00000000 */   nop
    /* F1E8 8001E9E8 3772000C */  jal        func_8001C8DC
    /* F1EC 8001E9EC 00000000 */   nop
    /* F1F0 8001E9F0 2C00BF8F */  lw         $ra, 0x2C($sp)
    /* F1F4 8001E9F4 2800B08F */  lw         $s0, 0x28($sp)
    /* F1F8 8001E9F8 3000BD27 */  addiu      $sp, $sp, 0x30
    /* F1FC 8001E9FC 0800E003 */  jr         $ra
    /* F200 8001EA00 00000000 */   nop
endlabel func_8001E878
