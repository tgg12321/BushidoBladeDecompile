glabel func_8002FDB0
    /* 205B0 8002FDB0 04008384 */  lh         $v1, 0x4($a0)
    /* 205B4 8002FDB4 00000000 */  nop
    /* 205B8 8002FDB8 40110300 */  sll        $v0, $v1, 5
    /* 205BC 8002FDBC 21104300 */  addu       $v0, $v0, $v1
    /* 205C0 8002FDC0 C0100200 */  sll        $v0, $v0, 3
    /* 205C4 8002FDC4 801F013C */  lui        $at, (0x1F8000C0 >> 16)
    /* 205C8 8002FDC8 21084100 */  addu       $at, $v0, $at
    /* 205CC 8002FDCC C000238C */  lw         $v1, (0x1F8000C0 & 0xFFFF)($at)
    /* 205D0 8002FDD0 801F013C */  lui        $at, (0x1F8000B4 >> 16)
    /* 205D4 8002FDD4 21084100 */  addu       $at, $v0, $at
    /* 205D8 8002FDD8 B400248C */  lw         $a0, (0x1F8000B4 & 0xFFFF)($at)
    /* 205DC 8002FDDC 00000000 */  nop
    /* 205E0 8002FDE0 23186400 */  subu       $v1, $v1, $a0
    /* 205E4 8002FDE4 801F013C */  lui        $at, (0x1F800360 >> 16)
    /* 205E8 8002FDE8 600323AC */  sw         $v1, (0x1F800360 & 0xFFFF)($at)
    /* 205EC 8002FDEC 801F013C */  lui        $at, (0x1F8000C4 >> 16)
    /* 205F0 8002FDF0 21084100 */  addu       $at, $v0, $at
    /* 205F4 8002FDF4 C400238C */  lw         $v1, (0x1F8000C4 & 0xFFFF)($at)
    /* 205F8 8002FDF8 801F013C */  lui        $at, (0x1F8000B8 >> 16)
    /* 205FC 8002FDFC 21084100 */  addu       $at, $v0, $at
    /* 20600 8002FE00 B800248C */  lw         $a0, (0x1F8000B8 & 0xFFFF)($at)
    /* 20604 8002FE04 00000000 */  nop
    /* 20608 8002FE08 23186400 */  subu       $v1, $v1, $a0
    /* 2060C 8002FE0C 801F013C */  lui        $at, (0x1F800364 >> 16)
    /* 20610 8002FE10 640323AC */  sw         $v1, (0x1F800364 & 0xFFFF)($at)
    /* 20614 8002FE14 801F013C */  lui        $at, (0x1F8000C8 >> 16)
    /* 20618 8002FE18 21084100 */  addu       $at, $v0, $at
    /* 2061C 8002FE1C C800238C */  lw         $v1, (0x1F8000C8 & 0xFFFF)($at)
    /* 20620 8002FE20 801F013C */  lui        $at, (0x1F8000BC >> 16)
    /* 20624 8002FE24 21084100 */  addu       $at, $v0, $at
    /* 20628 8002FE28 BC00248C */  lw         $a0, (0x1F8000BC & 0xFFFF)($at)
    /* 2062C 8002FE2C 00000000 */  nop
    /* 20630 8002FE30 23186400 */  subu       $v1, $v1, $a0
    /* 20634 8002FE34 801F013C */  lui        $at, (0x1F800368 >> 16)
    /* 20638 8002FE38 680323AC */  sw         $v1, (0x1F800368 & 0xFFFF)($at)
    /* 2063C 8002FE3C 801F013C */  lui        $at, (0x1F8000CC >> 16)
    /* 20640 8002FE40 21084100 */  addu       $at, $v0, $at
    /* 20644 8002FE44 CC00238C */  lw         $v1, (0x1F8000CC & 0xFFFF)($at)
    /* 20648 8002FE48 801F013C */  lui        $at, (0x1F8000B4 >> 16)
    /* 2064C 8002FE4C 21084100 */  addu       $at, $v0, $at
    /* 20650 8002FE50 B400248C */  lw         $a0, (0x1F8000B4 & 0xFFFF)($at)
    /* 20654 8002FE54 00000000 */  nop
    /* 20658 8002FE58 23186400 */  subu       $v1, $v1, $a0
    /* 2065C 8002FE5C 801F013C */  lui        $at, (0x1F800370 >> 16)
    /* 20660 8002FE60 700323AC */  sw         $v1, (0x1F800370 & 0xFFFF)($at)
    /* 20664 8002FE64 801F013C */  lui        $at, (0x1F8000D0 >> 16)
    /* 20668 8002FE68 21084100 */  addu       $at, $v0, $at
    /* 2066C 8002FE6C D000238C */  lw         $v1, (0x1F8000D0 & 0xFFFF)($at)
    /* 20670 8002FE70 801F013C */  lui        $at, (0x1F8000B8 >> 16)
    /* 20674 8002FE74 21084100 */  addu       $at, $v0, $at
    /* 20678 8002FE78 B800248C */  lw         $a0, (0x1F8000B8 & 0xFFFF)($at)
    /* 2067C 8002FE7C 00000000 */  nop
    /* 20680 8002FE80 23186400 */  subu       $v1, $v1, $a0
    /* 20684 8002FE84 801F013C */  lui        $at, (0x1F800374 >> 16)
    /* 20688 8002FE88 740323AC */  sw         $v1, (0x1F800374 & 0xFFFF)($at)
    /* 2068C 8002FE8C 801F013C */  lui        $at, (0x1F8000D4 >> 16)
    /* 20690 8002FE90 21084100 */  addu       $at, $v0, $at
    /* 20694 8002FE94 D400238C */  lw         $v1, (0x1F8000D4 & 0xFFFF)($at)
    /* 20698 8002FE98 801F013C */  lui        $at, (0x1F8000BC >> 16)
    /* 2069C 8002FE9C 21084100 */  addu       $at, $v0, $at
    /* 206A0 8002FEA0 BC00228C */  lw         $v0, (0x1F8000BC & 0xFFFF)($at)
    /* 206A4 8002FEA4 801F053C */  lui        $a1, (0x1F800360 >> 16)
    /* 206A8 8002FEA8 6003A534 */  ori        $a1, $a1, (0x1F800360 & 0xFFFF)
    /* 206AC 8002FEAC 23186200 */  subu       $v1, $v1, $v0
    /* 206B0 8002FEB0 801F013C */  lui        $at, (0x1F800378 >> 16)
    /* 206B4 8002FEB4 780323AC */  sw         $v1, (0x1F800378 & 0xFFFF)($at)
    /* 206B8 8002FEB8 2160A000 */  addu       $t4, $a1, $zero
    /* 206BC 8002FEBC 00008D8D */  lw         $t5, 0x0($t4)
    /* 206C0 8002FEC0 04008E8D */  lw         $t6, 0x4($t4)
    /* 206C4 8002FEC4 0000CD48 */  ctc2       $t5, $0 /* handwritten instruction */
    /* 206C8 8002FEC8 08008F8D */  lw         $t7, 0x8($t4)
    /* 206CC 8002FECC 0010CE48 */  ctc2       $t6, $2 /* handwritten instruction */
    /* 206D0 8002FED0 0020CF48 */  ctc2       $t7, $4 /* handwritten instruction */
    /* 206D4 8002FED4 801F053C */  lui        $a1, (0x1F800370 >> 16)
    /* 206D8 8002FED8 7003A534 */  ori        $a1, $a1, (0x1F800370 & 0xFFFF)
    /* 206DC 8002FEDC 2160A000 */  addu       $t4, $a1, $zero
    /* 206E0 8002FEE0 08008BC9 */  lwc2       $11, 0x8($t4)
    /* 206E4 8002FEE4 000089C9 */  lwc2       $9, 0x0($t4)
    /* 206E8 8002FEE8 04008AC9 */  lwc2       $10, 0x4($t4)
    /* 206EC 8002FEEC 00000000 */  nop
    /* 206F0 8002FEF0 00000000 */  nop
    /* 206F4 8002FEF4 0C00704B */  op         0
    /* 206F8 8002FEF8 801F053C */  lui        $a1, (0x1F800380 >> 16)
    /* 206FC 8002FEFC 8003A534 */  ori        $a1, $a1, (0x1F800380 & 0xFFFF)
    /* 20700 8002FF00 2160A000 */  addu       $t4, $a1, $zero
    /* 20704 8002FF04 000099E9 */  swc2       $25, 0x0($t4)
    /* 20708 8002FF08 04009AE9 */  swc2       $26, 0x4($t4) /* handwritten instruction */
    /* 2070C 8002FF0C 08009BE9 */  swc2       $27, 0x8($t4) /* handwritten instruction */
    /* 20710 8002FF10 801F023C */  lui        $v0, (0x1F800384 >> 16)
    /* 20714 8002FF14 8403428C */  lw         $v0, (0x1F800384 & 0xFFFF)($v0)
    /* 20718 8002FF18 0800E003 */  jr         $ra
    /* 2071C 8002FF1C 2A100200 */   slt       $v0, $zero, $v0
endlabel func_8002FDB0
