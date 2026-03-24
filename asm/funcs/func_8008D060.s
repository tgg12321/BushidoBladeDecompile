glabel func_8008D060
    /* 7D860 8008D060 B0000A24 */  addiu      $t2, $zero, 0xB0
    /* 7D864 8008D064 08004001 */  jr         $t2
    /* 7D868 8008D068 48000924 */   addiu     $t1, $zero, 0x48
    /* 7D86C 8008D06C 00000000 */  nop
  alabel D_8008D070
    /* 7D870 8008D070 00000000 */  nop
    /* 7D874 8008D074 00000000 */  nop
    /* 7D878 8008D078 00000000 */  nop
    /* 7D87C 8008D07C 00000000 */  nop
    /* 7D880 8008D080 00000000 */  nop
    /* 7D884 8008D084 00000000 */  nop
    /* 7D888 8008D088 00000000 */  nop
    /* 7D88C 8008D08C 00000000 */  nop
  alabel D_8008D090
    /* 7D890 8008D090 B0DC0180 */  lb         $at, -0x2350($zero)
    /* 7D894 8008D094 78E80180 */  lb         $at, -0x1788($zero)
    /* 7D898 8008D098 98380380 */  lb         $v1, 0x3898($zero)
    /* 7D89C 8008D09C 08470380 */  lb         $v1, 0x4708($zero)
    /* 7D8A0 8008D0A0 D4970380 */  lb         $v1, -0x682C($zero)
    /* 7D8A4 8008D0A4 3C990380 */  lb         $v1, -0x66C4($zero)
    /* 7D8A8 8008D0A8 D0B90380 */  lb         $v1, -0x4630($zero)
    /* 7D8AC 8008D0AC B4BC0380 */  lb         $v1, -0x434C($zero)
    /* 7D8B0 8008D0B0 80540380 */  lb         $v1, 0x5480($zero)
    /* 7D8B4 8008D0B4 28580380 */  lb         $v1, 0x5828($zero)
    /* 7D8B8 8008D0B8 10BE0380 */  lb         $v1, -0x41F0($zero)
    /* 7D8BC 8008D0BC A8BE0380 */  lb         $v1, -0x4158($zero)
    /* 7D8C0 8008D0C0 04EA0180 */  lb         $at, -0x15FC($zero)
    /* 7D8C4 8008D0C4 84EA0180 */  lb         $at, -0x157C($zero)
    /* 7D8C8 8008D0C8 30540380 */  lb         $v1, 0x5430($zero)
    /* 7D8CC 8008D0CC C4BF0380 */  lb         $v1, -0x403C($zero)
    /* 7D8D0 8008D0D0 B4EE0180 */  lb         $at, -0x114C($zero)
    /* 7D8D4 8008D0D4 A0EF0180 */  lb         $at, -0x1060($zero)
    /* 7D8D8 8008D0D8 40C00380 */  lb         $v1, -0x3FC0($zero)
    /* 7D8DC 8008D0DC C0C20380 */  lb         $v1, -0x3D40($zero)
    /* 7D8E0 8008D0E0 2CC40380 */  lb         $v1, -0x3BD4($zero)
    /* 7D8E4 8008D0E4 60C50380 */  lb         $v1, -0x3AA0($zero)
    /* 7D8E8 8008D0E8 70B80380 */  lb         $v1, -0x4790($zero)
    /* 7D8EC 8008D0EC E4B80380 */  lb         $v1, -0x471C($zero)
    /* 7D8F0 8008D0F0 58C90380 */  lb         $v1, -0x36A8($zero)
    /* 7D8F4 8008D0F4 A4C90380 */  lb         $v1, -0x365C($zero)
    /* 7D8F8 8008D0F8 C85D0380 */  lb         $v1, 0x5DC8($zero)
    /* 7D8FC 8008D0FC 385E0380 */  lb         $v1, 0x5E38($zero)
    /* 7D900 8008D100 18CE0380 */  lb         $v1, -0x31E8($zero)
    /* 7D904 8008D104 84CF0380 */  lb         $v1, -0x307C($zero)
    /* 7D908 8008D108 14C70380 */  lb         $v1, -0x38EC($zero)
    /* 7D90C 8008D10C B4C80380 */  lb         $v1, -0x374C($zero)
    /* 7D910 8008D110 CCCC0380 */  lb         $v1, -0x3334($zero)
    /* 7D914 8008D114 10CD0380 */  lb         $v1, -0x32F0($zero)
  alabel D_8008D118
    /* 7D918 8008D118 00080B0D */  jal        func_842C2000
    /* 7D91C 8008D11C 10111315 */   bne       $t0, $s3, .L80091560
endlabel func_8008D060
