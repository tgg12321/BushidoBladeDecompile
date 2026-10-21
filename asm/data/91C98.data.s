.include "macro.inc"

.section .data, "wa"
    .align 0

nonmatching D_800A14B0

dlabel D_800A14B0
dlabel g_cdrom_delay_reg
    /* 91CB0 800A14B0 1810801F */ .word 0x1F801018
enddlabel D_800A14B0

nonmatching D_800A14B4

dlabel D_800A14B4
    /* 91CB4 800A14B4 F010801F */ .word 0x1F8010F0
enddlabel D_800A14B4

nonmatching D_800A14B8

dlabel D_800A14B8
dlabel g_cd_dma_dest
    /* 91CB8 800A14B8 B010801F */ .word 0x1F8010B0
enddlabel D_800A14B8

nonmatching D_800A14BC

dlabel D_800A14BC
dlabel g_cd_dma_size
    /* 91CBC 800A14BC B410801F */ .word 0x1F8010B4
enddlabel D_800A14BC

nonmatching D_800A14C0

dlabel D_800A14C0
dlabel g_cd_dma_ctrl
    /* 91CC0 800A14C0 B810801F */ .word 0x1F8010B8
    /* 91CC4 800A14C4 50730215 */ .word 0x15027350
    /* 91CC8 800A14C8 9C844000 */ .word 0x0040849C
enddlabel D_800A14C0

nonmatching g_CdReadCallback_func

dlabel g_CdReadCallback_func
    /* 91CCC 800A14CC 00000000 */ .word 0x00000000
enddlabel g_CdReadCallback_func

nonmatching D_800A14D0

dlabel D_800A14D0
    /* 91CD0 800A14D0 00000000 */ .word 0x00000000
enddlabel D_800A14D0

nonmatching D_800A14D4

dlabel D_800A14D4
    /* 91CD4 800A14D4 00000000 */ .word 0x00000000
enddlabel D_800A14D4

nonmatching D_800A14D8

dlabel D_800A14D8
    /* 91CD8 800A14D8 00000000 */ .word 0x00000000
enddlabel D_800A14D8

nonmatching D_800A14DC

dlabel D_800A14DC
    /* 91CDC 800A14DC 00000000 */ .word 0x00000000
enddlabel D_800A14DC

nonmatching D_800A14E0

dlabel D_800A14E0
    /* 91CE0 800A14E0 00000000 */ .word 0x00000000
enddlabel D_800A14E0

nonmatching D_800A14E4

dlabel D_800A14E4
    /* 91CE4 800A14E4 00000000 */ .word 0x00000000
enddlabel D_800A14E4

nonmatching D_800A14E8

dlabel D_800A14E8
    /* 91CE8 800A14E8 00000000 */ .word 0x00000000
enddlabel D_800A14E8

nonmatching D_800A14EC

dlabel D_800A14EC
    /* 91CEC 800A14EC 00000000 */ .word 0x00000000
enddlabel D_800A14EC

nonmatching D_800A14F0

dlabel D_800A14F0
    /* 91CF0 800A14F0 00000000 */ .word 0x00000000
enddlabel D_800A14F0

nonmatching D_800A14F4

dlabel D_800A14F4
    /* 91CF4 800A14F4 00000000 */ .word 0x00000000
enddlabel D_800A14F4

nonmatching D_800A14F8

dlabel D_800A14F8
    /* 91CF8 800A14F8 00000000 */ .word 0x00000000
enddlabel D_800A14F8

nonmatching D_800A14FC

dlabel D_800A14FC
    /* 91CFC 800A14FC 00000000 */ .word 0x00000000
enddlabel D_800A14FC

nonmatching D_800A1500

dlabel D_800A1500
    /* 91D00 800A1500 00000000 */ .word 0x00000000
enddlabel D_800A1500

nonmatching D_800A1504

dlabel D_800A1504
    /* 91D04 800A1504 00000000 */ .word 0x00000000
    /* 91D08 800A1508 50730615 */ .word 0x15067350
    /* 91D0C 800A150C 9C884000 */ .word 0x0040889C
enddlabel D_800A1504

nonmatching g_vsync_gpu_stat_reg

dlabel g_vsync_gpu_stat_reg
    /* 91D10 800A1510 1418801F */ .word 0x1F801814
enddlabel g_vsync_gpu_stat_reg

nonmatching g_vsync_rcnt1_count_reg

dlabel g_vsync_rcnt1_count_reg
    /* 91D14 800A1514 1011801F */ .word 0x1F801110
enddlabel g_vsync_rcnt1_count_reg

nonmatching Hcount

dlabel Hcount
    /* 91D18 800A1518 00000000 */ .word 0x00000000
enddlabel Hcount

nonmatching prevVcount

dlabel prevVcount
    /* 91D1C 800A151C 00000000 */ .word 0x00000000
enddlabel prevVcount

nonmatching D_800A1520

dlabel D_800A1520
    /* 91D20 800A1520 4C696272 */ .word 0x7262694C
    /* 91D24 800A1524 61727920 */ .word 0x20797261
    /* 91D28 800A1528 50726F67 */ .word 0x676F7250
    /* 91D2C 800A152C 72616D73 */ .word 0x736D6172
    /* 91D30 800A1530 20286329 */ .word 0x29632820
    /* 91D34 800A1534 20313939 */ .word 0x39393120
    /* 91D38 800A1538 332D3139 */ .word 0x39312D33
    /* 91D3C 800A153C 39372053 */ .word 0x53203739
    /* 91D40 800A1540 6F6E7920 */ .word 0x20796E6F
    /* 91D44 800A1544 436F6D70 */ .word 0x706D6F43
    /* 91D48 800A1548 75746572 */ .word 0x72657475
    /* 91D4C 800A154C 20456E74 */ .word 0x746E4520
    /* 91D50 800A1550 65727461 */ .word 0x61747265
    /* 91D54 800A1554 696E6D65 */ .word 0x656D6E69
    /* 91D58 800A1558 6E742049 */ .word 0x4920746E
    /* 91D5C 800A155C 6E632E2C */ .word 0x2C2E636E
    /* 91D60 800A1560 20416C6C */ .word 0x6C6C4120
    /* 91D64 800A1564 20526967 */ .word 0x67695220
    /* 91D68 800A1568 68747320 */ .word 0x20737468
    /* 91D6C 800A156C 52657365 */ .word 0x65736552
    /* 91D70 800A1570 72766564 */ .word 0x64657672
    /* 91D74 800A1574 2E000000 */ .word 0x0000002E
enddlabel D_800A1520
