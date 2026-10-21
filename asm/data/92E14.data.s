.include "macro.inc"

.section .data, "wa"
    .align 0

nonmatching D_800A2614

dlabel D_800A2614
dlabel g_irq_vsync_callbacks
    /* 92E14 800A2614 00000000 */ .word 0x00000000
    /* 92E18 800A2618 00000000 */ .word 0x00000000
    /* 92E1C 800A261C 00000000 */ .word 0x00000000
    /* 92E20 800A2620 00000000 */ .word 0x00000000
    /* 92E24 800A2624 00000000 */ .word 0x00000000
    /* 92E28 800A2628 00000000 */ .word 0x00000000
    /* 92E2C 800A262C 00000000 */ .word 0x00000000
    /* 92E30 800A2630 00000000 */ .word 0x00000000
enddlabel D_800A2614

nonmatching Vcount

dlabel Vcount
    /* 92E34 800A2634 00000000 */ .word 0x00000000
enddlabel Vcount

nonmatching D_800A2638

dlabel D_800A2638
    /* 92E38 800A2638 1411801F */ .word 0x1F801114
enddlabel D_800A2638

nonmatching D_800A263C

dlabel D_800A263C
    /* 92E3C 800A263C F410801F */ .word 0x1F8010F4
enddlabel D_800A263C

nonmatching D_800A2640

dlabel D_800A2640
    /* 92E40 800A2640 00000000 */ .word 0x00000000
    /* 92E44 800A2644 00000000 */ .word 0x00000000
    /* 92E48 800A2648 00000000 */ .word 0x00000000
    /* 92E4C 800A264C 00000000 */ .word 0x00000000
    /* 92E50 800A2650 00000000 */ .word 0x00000000
    /* 92E54 800A2654 00000000 */ .word 0x00000000
    /* 92E58 800A2658 00000000 */ .word 0x00000000
    /* 92E5C 800A265C 00000000 */ .word 0x00000000
enddlabel D_800A2640

nonmatching D_800A2660

dlabel D_800A2660
    /* 92E60 800A2660 8010801F */ .word 0x1F801080
enddlabel D_800A2660

nonmatching video_mode

dlabel video_mode
    /* 92E64 800A2664 00000000 */ .word 0x00000000
enddlabel video_mode

nonmatching D_800A2668

dlabel D_800A2668
    /* 92E68 800A2668 00000000 */ .word 0x00000000
enddlabel D_800A2668

nonmatching __heapbase

dlabel __heapbase
    /* 92E6C 800A266C 00000000 */ .word 0x00000000
enddlabel __heapbase

nonmatching __heapsize

dlabel __heapsize
    /* 92E70 800A2670 00000000 */ .word 0x00000000
enddlabel __heapsize

nonmatching D_800A2674

dlabel D_800A2674
    /* 92E74 800A2674 F8640180 */ .word func_800164F8
    /* 92E78 800A2678 786B0700 */ .word 0x00076B78
    /* 92E7C 800A267C 70D00880 */ .word D_8008D070
    /* 92E80 800A2680 5C600100 */ .word 0x0001605C
    /* 92E84 800A2684 30390A80 */ .word g_font_sprites
    /* 92E88 800A2688 B03F0600 */ .word 0x00063FB0
enddlabel D_800A2674

nonmatching _stacksize

dlabel _stacksize
    /* 92E8C 800A268C 00800000 */ .word 0x00008000
enddlabel _stacksize

nonmatching _ramsize

dlabel _ramsize
    /* 92E90 800A2690 00008000 */ .word 0x00800000
enddlabel _ramsize

nonmatching D_800A2694

dlabel D_800A2694
    /* 92E94 800A2694 50731017 */ .word 0x17107350
    /* 92E98 800A2698 E4974040 */ .word 0x404097E4
enddlabel D_800A2694

nonmatching default_voice

dlabel default_voice
    /* 92E9C 800A269C 00000000 */ .word 0x00000000
    /* 92EA0 800A26A0 00100030 */ .word 0x30001000
    /* 92EA4 800A26A4 BF000000 */ .word 0x000000BF
    /* 92EA8 800A26A8 00000000 */ .word 0x00000000
enddlabel default_voice

nonmatching default_state

dlabel default_state
    /* 92EAC 800A26AC FF3FFF3F */ .word 0x3FFF3FFF
    /* 92EB0 800A26B0 00000000 */ .word 0x00000000
    /* 92EB4 800A26B4 00000000 */ .word 0x00000000
    /* 92EB8 800A26B8 00000000 */ .word 0x00000000
    /* 92EBC 800A26BC 00000000 */ .word 0x00000000
    /* 92EC0 800A26C0 00000000 */ .word 0x00000000
    /* 92EC4 800A26C4 00000000 */ .word 0x00000000
    /* 92EC8 800A26C8 00000000 */ .word 0x00000000
enddlabel default_state

nonmatching _snd_seq_tick_env

dlabel _snd_seq_tick_env
    /* 92ECC 800A26CC 3C000000 */ .word 0x0000003C
enddlabel _snd_seq_tick_env

nonmatching _snd_seq_tick_env_plus_0x4

dlabel _snd_seq_tick_env_plus_0x4
    /* 92ED0 800A26D0 01000000 */ .word 0x00000001
enddlabel _snd_seq_tick_env_plus_0x4

nonmatching _snd_seq_tick_env_plus_0x8

dlabel _snd_seq_tick_env_plus_0x8
    /* 92ED4 800A26D4 6C3F0880 */ .word SsSeqCalledTbyT
enddlabel _snd_seq_tick_env_plus_0x8

nonmatching _snd_seq_tick_env_plus_0xC

dlabel _snd_seq_tick_env_plus_0xC
    /* 92ED8 800A26D8 00000000 */ .word 0x00000000
enddlabel _snd_seq_tick_env_plus_0xC

nonmatching _snd_seq_tick_env_plus_0x10

dlabel _snd_seq_tick_env_plus_0x10
    /* 92EDC 800A26DC */ .byte 0x00
enddlabel _snd_seq_tick_env_plus_0x10

nonmatching _snd_seq_tick_env_plus_0x11

dlabel _snd_seq_tick_env_plus_0x11
    /* 92EDD 800A26DD */ .byte 0x00
enddlabel _snd_seq_tick_env_plus_0x11

nonmatching _snd_seq_tick_env_plus_0x12

dlabel _snd_seq_tick_env_plus_0x12
    /* 92EDE 800A26DE */ .byte 0x7F
    /* 92EDF 800A26DF */ .byte 0x00
enddlabel _snd_seq_tick_env_plus_0x12

nonmatching _snd_seq_tick_env_plus_0x14

dlabel _snd_seq_tick_env_plus_0x14
    /* 92EE0 800A26E0 00000000 */ .word 0x00000000
enddlabel _snd_seq_tick_env_plus_0x14

nonmatching D_800A26E4

dlabel D_800A26E4
    /* 92EE4 800A26E4 */ .short 0x1000
    /* 92EE6 800A26E6 */ .short 0x100E
    /* 92EE8 800A26E8 */ .short 0x101D
    /* 92EEA 800A26EA */ .short 0x102C
    /* 92EEC 800A26EC */ .short 0x103B
    /* 92EEE 800A26EE */ .short 0x104A
    /* 92EF0 800A26F0 */ .short 0x1059
    /* 92EF2 800A26F2 */ .short 0x1068
    /* 92EF4 800A26F4 */ .short 0x1078
    /* 92EF6 800A26F6 */ .short 0x1087
    /* 92EF8 800A26F8 */ .short 0x1096
    /* 92EFA 800A26FA */ .short 0x10A5
    /* 92EFC 800A26FC */ .short 0x10B5
    /* 92EFE 800A26FE */ .short 0x10C4
    /* 92F00 800A2700 */ .short 0x10D4
    /* 92F02 800A2702 */ .short 0x10E3
    /* 92F04 800A2704 */ .short 0x10F3
    /* 92F06 800A2706 */ .short 0x1103
    /* 92F08 800A2708 */ .short 0x1113
    /* 92F0A 800A270A */ .short 0x1122
    /* 92F0C 800A270C */ .short 0x1132
    /* 92F0E 800A270E */ .short 0x1142
    /* 92F10 800A2710 */ .short 0x1152
    /* 92F12 800A2712 */ .short 0x1162
    /* 92F14 800A2714 */ .short 0x1172
    /* 92F16 800A2716 */ .short 0x1182
    /* 92F18 800A2718 */ .short 0x1193
    /* 92F1A 800A271A */ .short 0x11A3
    /* 92F1C 800A271C */ .short 0x11B3
    /* 92F1E 800A271E */ .short 0x11C4
    /* 92F20 800A2720 */ .short 0x11D4
    /* 92F22 800A2722 */ .short 0x11E5
    /* 92F24 800A2724 */ .short 0x11F5
    /* 92F26 800A2726 */ .short 0x1206
    /* 92F28 800A2728 */ .short 0x1216
    /* 92F2A 800A272A */ .short 0x1227
    /* 92F2C 800A272C */ .short 0x1238
    /* 92F2E 800A272E */ .short 0x1249
    /* 92F30 800A2730 */ .short 0x125A
    /* 92F32 800A2732 */ .short 0x126B
    /* 92F34 800A2734 */ .short 0x127C
    /* 92F36 800A2736 */ .short 0x128D
    /* 92F38 800A2738 */ .short 0x129E
    /* 92F3A 800A273A */ .short 0x12AF
    /* 92F3C 800A273C */ .short 0x12C1
    /* 92F3E 800A273E */ .short 0x12D2
    /* 92F40 800A2740 */ .short 0x12E3
    /* 92F42 800A2742 */ .short 0x12F5
    /* 92F44 800A2744 */ .short 0x1306
    /* 92F46 800A2746 */ .short 0x1318
    /* 92F48 800A2748 */ .short 0x132A
    /* 92F4A 800A274A */ .short 0x133C
    /* 92F4C 800A274C */ .short 0x134D
    /* 92F4E 800A274E */ .short 0x135F
    /* 92F50 800A2750 */ .short 0x1371
    /* 92F52 800A2752 */ .short 0x1383
    /* 92F54 800A2754 */ .short 0x1395
    /* 92F56 800A2756 */ .short 0x13A7
    /* 92F58 800A2758 */ .short 0x13BA
    /* 92F5A 800A275A */ .short 0x13CC
    /* 92F5C 800A275C */ .short 0x13DE
    /* 92F5E 800A275E */ .short 0x13F1
    /* 92F60 800A2760 */ .short 0x1403
    /* 92F62 800A2762 */ .short 0x1416
    /* 92F64 800A2764 */ .short 0x1428
    /* 92F66 800A2766 */ .short 0x143B
    /* 92F68 800A2768 */ .short 0x144E
    /* 92F6A 800A276A */ .short 0x1460
    /* 92F6C 800A276C */ .short 0x1473
    /* 92F6E 800A276E */ .short 0x1486
    /* 92F70 800A2770 */ .short 0x1499
    /* 92F72 800A2772 */ .short 0x14AC
    /* 92F74 800A2774 */ .short 0x14BF
    /* 92F76 800A2776 */ .short 0x14D3
    /* 92F78 800A2778 */ .short 0x14E6
    /* 92F7A 800A277A */ .short 0x14F9
    /* 92F7C 800A277C */ .short 0x150D
    /* 92F7E 800A277E */ .short 0x1520
    /* 92F80 800A2780 */ .short 0x1534
    /* 92F82 800A2782 */ .short 0x1547
    /* 92F84 800A2784 */ .short 0x155B
    /* 92F86 800A2786 */ .short 0x156F
    /* 92F88 800A2788 */ .short 0x1583
    /* 92F8A 800A278A */ .short 0x1597
    /* 92F8C 800A278C */ .short 0x15AB
    /* 92F8E 800A278E */ .short 0x15BF
    /* 92F90 800A2790 */ .short 0x15D3
    /* 92F92 800A2792 */ .short 0x15E7
    /* 92F94 800A2794 */ .short 0x15FB
    /* 92F96 800A2796 */ .short 0x1610
    /* 92F98 800A2798 */ .short 0x1624
    /* 92F9A 800A279A */ .short 0x1638
    /* 92F9C 800A279C */ .short 0x164D
    /* 92F9E 800A279E */ .short 0x1662
    /* 92FA0 800A27A0 */ .short 0x1676
    /* 92FA2 800A27A2 */ .short 0x168B
    /* 92FA4 800A27A4 */ .short 0x16A0
    /* 92FA6 800A27A6 */ .short 0x16B5
    /* 92FA8 800A27A8 */ .short 0x16CA
    /* 92FAA 800A27AA */ .short 0x16DF
    /* 92FAC 800A27AC */ .short 0x16F4
    /* 92FAE 800A27AE */ .short 0x170A
    /* 92FB0 800A27B0 */ .short 0x171F
    /* 92FB2 800A27B2 */ .short 0x1734
    /* 92FB4 800A27B4 */ .short 0x174A
    /* 92FB6 800A27B6 */ .short 0x175F
    /* 92FB8 800A27B8 */ .short 0x1775
    /* 92FBA 800A27BA */ .short 0x178B
    /* 92FBC 800A27BC */ .short 0x17A1
    /* 92FBE 800A27BE */ .short 0x17B6
    /* 92FC0 800A27C0 */ .short 0x17CC
    /* 92FC2 800A27C2 */ .short 0x17E2
    /* 92FC4 800A27C4 */ .short 0x17F9
    /* 92FC6 800A27C6 */ .short 0x180F
    /* 92FC8 800A27C8 */ .short 0x1825
    /* 92FCA 800A27CA */ .short 0x183B
    /* 92FCC 800A27CC */ .short 0x1852
    /* 92FCE 800A27CE */ .short 0x1868
    /* 92FD0 800A27D0 */ .short 0x187F
    /* 92FD2 800A27D2 */ .short 0x1896
    /* 92FD4 800A27D4 */ .short 0x18AC
    /* 92FD6 800A27D6 */ .short 0x18C3
    /* 92FD8 800A27D8 */ .short 0x18DA
    /* 92FDA 800A27DA */ .short 0x18F1
    /* 92FDC 800A27DC */ .short 0x1908
    /* 92FDE 800A27DE */ .short 0x191F
    /* 92FE0 800A27E0 */ .short 0x1937
    /* 92FE2 800A27E2 */ .short 0x194E
    /* 92FE4 800A27E4 */ .short 0x1965
    /* 92FE6 800A27E6 */ .short 0x197D
    /* 92FE8 800A27E8 */ .short 0x1995
    /* 92FEA 800A27EA */ .short 0x19AC
    /* 92FEC 800A27EC */ .short 0x19C4
    /* 92FEE 800A27EE */ .short 0x19DC
    /* 92FF0 800A27F0 */ .short 0x19F4
    /* 92FF2 800A27F2 */ .short 0x1A0C
    /* 92FF4 800A27F4 */ .short 0x1A24
    /* 92FF6 800A27F6 */ .short 0x1A3C
    /* 92FF8 800A27F8 */ .short 0x1A55
    /* 92FFA 800A27FA */ .short 0x1A6D
    /* 92FFC 800A27FC */ .short 0x1A85
    /* 92FFE 800A27FE */ .short 0x1A9E
    /* 93000 800A2800 */ .short 0x1AB7
    /* 93002 800A2802 */ .short 0x1ACF
    /* 93004 800A2804 */ .short 0x1AE8
    /* 93006 800A2806 */ .short 0x1B01
    /* 93008 800A2808 */ .short 0x1B1A
    /* 9300A 800A280A */ .short 0x1B33
    /* 9300C 800A280C */ .short 0x1B4C
    /* 9300E 800A280E */ .short 0x1B66
    /* 93010 800A2810 */ .short 0x1B7F
    /* 93012 800A2812 */ .short 0x1B98
    /* 93014 800A2814 */ .short 0x1BB2
    /* 93016 800A2816 */ .short 0x1BCC
    /* 93018 800A2818 */ .short 0x1BE5
    /* 9301A 800A281A */ .short 0x1BFF
    /* 9301C 800A281C */ .short 0x1C19
    /* 9301E 800A281E */ .short 0x1C33
    /* 93020 800A2820 */ .short 0x1C4D
    /* 93022 800A2822 */ .short 0x1C67
    /* 93024 800A2824 */ .short 0x1C82
    /* 93026 800A2826 */ .short 0x1C9C
    /* 93028 800A2828 */ .short 0x1CB7
    /* 9302A 800A282A */ .short 0x1CD1
    /* 9302C 800A282C */ .short 0x1CEC
    /* 9302E 800A282E */ .short 0x1D07
    /* 93030 800A2830 */ .short 0x1D22
    /* 93032 800A2832 */ .short 0x1D3D
    /* 93034 800A2834 */ .short 0x1D58
    /* 93036 800A2836 */ .short 0x1D73
    /* 93038 800A2838 */ .short 0x1D8E
    /* 9303A 800A283A */ .short 0x1DA9
    /* 9303C 800A283C */ .short 0x1DC5
    /* 9303E 800A283E */ .short 0x1DE0
    /* 93040 800A2840 */ .short 0x1DFC
    /* 93042 800A2842 */ .short 0x1E18
    /* 93044 800A2844 */ .short 0x1E34
    /* 93046 800A2846 */ .short 0x1E50
    /* 93048 800A2848 */ .short 0x1E6C
    /* 9304A 800A284A */ .short 0x1E88
    /* 9304C 800A284C */ .short 0x1EA4
    /* 9304E 800A284E */ .short 0x1EC1
    /* 93050 800A2850 */ .short 0x1EDD
    /* 93052 800A2852 */ .short 0x1EFA
    /* 93054 800A2854 */ .short 0x1F16
    /* 93056 800A2856 */ .short 0x1F33
    /* 93058 800A2858 */ .short 0x1F50
    /* 9305A 800A285A */ .short 0x1F6D
    /* 9305C 800A285C */ .short 0x1F8A
    /* 9305E 800A285E */ .short 0x1FA7
    /* 93060 800A2860 */ .short 0x1FC5
    /* 93062 800A2862 */ .short 0x1FE2
    /* 93064 800A2864 */ .short 0x2000
    /* 93066 800A2866 */ .short 0x0000
    /* 93068 800A2868 */ .short 0x7350
    /* 9306A 800A286A */ .short 0x1711
    /* 9306C 800A286C */ .short 0x4D7C
    /* 9306E 800A286E */ .short 0x3040
enddlabel D_800A26E4

nonmatching _spu_EVdma

dlabel _spu_EVdma
    /* 93070 800A2870 00000000 */ .word 0x00000000
enddlabel _spu_EVdma

nonmatching _spu_keystat

dlabel _spu_keystat
    /* 93074 800A2874 00000000 */ .word 0x00000000
enddlabel _spu_keystat

nonmatching _spu_trans_mode

dlabel _spu_trans_mode
    /* 93078 800A2878 00000000 */ .word 0x00000000
enddlabel _spu_trans_mode

nonmatching _spu_rev_flag

dlabel _spu_rev_flag
    /* 9307C 800A287C 00000000 */ .word 0x00000000
enddlabel _spu_rev_flag

nonmatching _spu_rev_reserve_wa

dlabel _spu_rev_reserve_wa
    /* 93080 800A2880 00000000 */ .word 0x00000000
enddlabel _spu_rev_reserve_wa

nonmatching _spu_rev_offsetaddr

dlabel _spu_rev_offsetaddr
    /* 93084 800A2884 00000000 */ .word 0x00000000
enddlabel _spu_rev_offsetaddr

nonmatching _spu_rev_attr

dlabel _spu_rev_attr
    /* 93088 800A2888 00000000 */ .word 0x00000000
enddlabel _spu_rev_attr

nonmatching _spu_rev_attr_plus_0x4

dlabel _spu_rev_attr_plus_0x4
    /* 9308C 800A288C 00000000 */ .word 0x00000000
enddlabel _spu_rev_attr_plus_0x4

nonmatching _spu_rev_attr_plus_0x8

dlabel _spu_rev_attr_plus_0x8
    /* 93090 800A2890 */ .short 0x0000
enddlabel _spu_rev_attr_plus_0x8

nonmatching _spu_rev_attr_plus_0xA

dlabel _spu_rev_attr_plus_0xA
    /* 93092 800A2892 */ .short 0x0000
enddlabel _spu_rev_attr_plus_0xA

nonmatching _spu_rev_attr_plus_0xC

dlabel _spu_rev_attr_plus_0xC
    /* 93094 800A2894 00000000 */ .word 0x00000000
enddlabel _spu_rev_attr_plus_0xC

nonmatching _spu_rev_attr_plus_0x10

dlabel _spu_rev_attr_plus_0x10
    /* 93098 800A2898 00000000 */ .word 0x00000000
enddlabel _spu_rev_attr_plus_0x10

nonmatching _spu_RQvoice

dlabel _spu_RQvoice
    /* 9309C 800A289C 00000000 */ .word 0x00000000
enddlabel _spu_RQvoice

nonmatching _spu_RQmask

dlabel _spu_RQmask
    /* 930A0 800A28A0 00000000 */ .word 0x00000000
enddlabel _spu_RQmask

nonmatching _spu_voice_centerNote

dlabel _spu_voice_centerNote
    /* 930A4 800A28A4 00C000C0 */ .word 0xC000C000
    /* 930A8 800A28A8 00C000C0 */ .word 0xC000C000
    /* 930AC 800A28AC 00C000C0 */ .word 0xC000C000
    /* 930B0 800A28B0 00C000C0 */ .word 0xC000C000
    /* 930B4 800A28B4 00C000C0 */ .word 0xC000C000
    /* 930B8 800A28B8 00C000C0 */ .word 0xC000C000
    /* 930BC 800A28BC 00C000C0 */ .word 0xC000C000
    /* 930C0 800A28C0 00C000C0 */ .word 0xC000C000
    /* 930C4 800A28C4 00C000C0 */ .word 0xC000C000
    /* 930C8 800A28C8 00C000C0 */ .word 0xC000C000
    /* 930CC 800A28CC 00C000C0 */ .word 0xC000C000
    /* 930D0 800A28D0 */ .short 0xC000
enddlabel _spu_voice_centerNote

nonmatching _spu_voice_centerNote_plus_0x2E

dlabel _spu_voice_centerNote_plus_0x2E
    /* 930D2 800A28D2 */ .short 0xC000
enddlabel _spu_voice_centerNote_plus_0x2E

nonmatching _spu_zerobuf

dlabel _spu_zerobuf
    /* 930D4 800A28D4 00000000 */ .word 0x00000000
    /* 930D8 800A28D8 00000000 */ .word 0x00000000
    /* 930DC 800A28DC 00000000 */ .word 0x00000000
    /* 930E0 800A28E0 00000000 */ .word 0x00000000
    /* 930E4 800A28E4 00000000 */ .word 0x00000000
    /* 930E8 800A28E8 00000000 */ .word 0x00000000
    /* 930EC 800A28EC 00000000 */ .word 0x00000000
    /* 930F0 800A28F0 00000000 */ .word 0x00000000
    /* 930F4 800A28F4 00000000 */ .word 0x00000000
    /* 930F8 800A28F8 00000000 */ .word 0x00000000
    /* 930FC 800A28FC 00000000 */ .word 0x00000000
    /* 93100 800A2900 00000000 */ .word 0x00000000
    /* 93104 800A2904 00000000 */ .word 0x00000000
    /* 93108 800A2908 00000000 */ .word 0x00000000
    /* 9310C 800A290C 00000000 */ .word 0x00000000
    /* 93110 800A2910 00000000 */ .word 0x00000000
    /* 93114 800A2914 00000000 */ .word 0x00000000
    /* 93118 800A2918 00000000 */ .word 0x00000000
    /* 9311C 800A291C 00000000 */ .word 0x00000000
    /* 93120 800A2920 00000000 */ .word 0x00000000
    /* 93124 800A2924 00000000 */ .word 0x00000000
    /* 93128 800A2928 00000000 */ .word 0x00000000
    /* 9312C 800A292C 00000000 */ .word 0x00000000
    /* 93130 800A2930 00000000 */ .word 0x00000000
    /* 93134 800A2934 00000000 */ .word 0x00000000
    /* 93138 800A2938 00000000 */ .word 0x00000000
    /* 9313C 800A293C 00000000 */ .word 0x00000000
    /* 93140 800A2940 00000000 */ .word 0x00000000
    /* 93144 800A2944 00000000 */ .word 0x00000000
    /* 93148 800A2948 00000000 */ .word 0x00000000
    /* 9314C 800A294C 00000000 */ .word 0x00000000
    /* 93150 800A2950 00000000 */ .word 0x00000000
    /* 93154 800A2954 00000000 */ .word 0x00000000
    /* 93158 800A2958 00000000 */ .word 0x00000000
    /* 9315C 800A295C 00000000 */ .word 0x00000000
    /* 93160 800A2960 00000000 */ .word 0x00000000
    /* 93164 800A2964 00000000 */ .word 0x00000000
    /* 93168 800A2968 00000000 */ .word 0x00000000
    /* 9316C 800A296C 00000000 */ .word 0x00000000
    /* 93170 800A2970 00000000 */ .word 0x00000000
    /* 93174 800A2974 00000000 */ .word 0x00000000
    /* 93178 800A2978 00000000 */ .word 0x00000000
    /* 9317C 800A297C 00000000 */ .word 0x00000000
    /* 93180 800A2980 00000000 */ .word 0x00000000
    /* 93184 800A2984 00000000 */ .word 0x00000000
    /* 93188 800A2988 00000000 */ .word 0x00000000
    /* 9318C 800A298C 00000000 */ .word 0x00000000
    /* 93190 800A2990 00000000 */ .word 0x00000000
    /* 93194 800A2994 00000000 */ .word 0x00000000
    /* 93198 800A2998 00000000 */ .word 0x00000000
    /* 9319C 800A299C 00000000 */ .word 0x00000000
    /* 931A0 800A29A0 00000000 */ .word 0x00000000
    /* 931A4 800A29A4 00000000 */ .word 0x00000000
    /* 931A8 800A29A8 00000000 */ .word 0x00000000
    /* 931AC 800A29AC 00000000 */ .word 0x00000000
    /* 931B0 800A29B0 00000000 */ .word 0x00000000
    /* 931B4 800A29B4 00000000 */ .word 0x00000000
    /* 931B8 800A29B8 00000000 */ .word 0x00000000
    /* 931BC 800A29BC 00000000 */ .word 0x00000000
    /* 931C0 800A29C0 00000000 */ .word 0x00000000
    /* 931C4 800A29C4 00000000 */ .word 0x00000000
    /* 931C8 800A29C8 00000000 */ .word 0x00000000
    /* 931CC 800A29CC 00000000 */ .word 0x00000000
    /* 931D0 800A29D0 00000000 */ .word 0x00000000
    /* 931D4 800A29D4 00000000 */ .word 0x00000000
    /* 931D8 800A29D8 00000000 */ .word 0x00000000
    /* 931DC 800A29DC 00000000 */ .word 0x00000000
    /* 931E0 800A29E0 00000000 */ .word 0x00000000
    /* 931E4 800A29E4 00000000 */ .word 0x00000000
    /* 931E8 800A29E8 00000000 */ .word 0x00000000
    /* 931EC 800A29EC 00000000 */ .word 0x00000000
    /* 931F0 800A29F0 00000000 */ .word 0x00000000
    /* 931F4 800A29F4 00000000 */ .word 0x00000000
    /* 931F8 800A29F8 00000000 */ .word 0x00000000
    /* 931FC 800A29FC 00000000 */ .word 0x00000000
    /* 93200 800A2A00 00000000 */ .word 0x00000000
    /* 93204 800A2A04 00000000 */ .word 0x00000000
    /* 93208 800A2A08 00000000 */ .word 0x00000000
    /* 9320C 800A2A0C 00000000 */ .word 0x00000000
    /* 93210 800A2A10 00000000 */ .word 0x00000000
    /* 93214 800A2A14 00000000 */ .word 0x00000000
    /* 93218 800A2A18 00000000 */ .word 0x00000000
    /* 9321C 800A2A1C 00000000 */ .word 0x00000000
    /* 93220 800A2A20 00000000 */ .word 0x00000000
    /* 93224 800A2A24 00000000 */ .word 0x00000000
    /* 93228 800A2A28 00000000 */ .word 0x00000000
    /* 9322C 800A2A2C 00000000 */ .word 0x00000000
    /* 93230 800A2A30 00000000 */ .word 0x00000000
    /* 93234 800A2A34 00000000 */ .word 0x00000000
    /* 93238 800A2A38 00000000 */ .word 0x00000000
    /* 9323C 800A2A3C 00000000 */ .word 0x00000000
    /* 93240 800A2A40 00000000 */ .word 0x00000000
    /* 93244 800A2A44 00000000 */ .word 0x00000000
    /* 93248 800A2A48 00000000 */ .word 0x00000000
    /* 9324C 800A2A4C 00000000 */ .word 0x00000000
    /* 93250 800A2A50 00000000 */ .word 0x00000000
    /* 93254 800A2A54 00000000 */ .word 0x00000000
    /* 93258 800A2A58 00000000 */ .word 0x00000000
    /* 9325C 800A2A5C 00000000 */ .word 0x00000000
    /* 93260 800A2A60 00000000 */ .word 0x00000000
    /* 93264 800A2A64 00000000 */ .word 0x00000000
    /* 93268 800A2A68 00000000 */ .word 0x00000000
    /* 9326C 800A2A6C 00000000 */ .word 0x00000000
    /* 93270 800A2A70 00000000 */ .word 0x00000000
    /* 93274 800A2A74 00000000 */ .word 0x00000000
    /* 93278 800A2A78 00000000 */ .word 0x00000000
    /* 9327C 800A2A7C 00000000 */ .word 0x00000000
    /* 93280 800A2A80 00000000 */ .word 0x00000000
    /* 93284 800A2A84 00000000 */ .word 0x00000000
    /* 93288 800A2A88 00000000 */ .word 0x00000000
    /* 9328C 800A2A8C 00000000 */ .word 0x00000000
    /* 93290 800A2A90 00000000 */ .word 0x00000000
    /* 93294 800A2A94 00000000 */ .word 0x00000000
    /* 93298 800A2A98 00000000 */ .word 0x00000000
    /* 9329C 800A2A9C 00000000 */ .word 0x00000000
    /* 932A0 800A2AA0 00000000 */ .word 0x00000000
    /* 932A4 800A2AA4 00000000 */ .word 0x00000000
    /* 932A8 800A2AA8 00000000 */ .word 0x00000000
    /* 932AC 800A2AAC 00000000 */ .word 0x00000000
    /* 932B0 800A2AB0 00000000 */ .word 0x00000000
    /* 932B4 800A2AB4 00000000 */ .word 0x00000000
    /* 932B8 800A2AB8 00000000 */ .word 0x00000000
    /* 932BC 800A2ABC 00000000 */ .word 0x00000000
    /* 932C0 800A2AC0 00000000 */ .word 0x00000000
    /* 932C4 800A2AC4 00000000 */ .word 0x00000000
    /* 932C8 800A2AC8 00000000 */ .word 0x00000000
    /* 932CC 800A2ACC 00000000 */ .word 0x00000000
    /* 932D0 800A2AD0 00000000 */ .word 0x00000000
    /* 932D4 800A2AD4 00000000 */ .word 0x00000000
    /* 932D8 800A2AD8 00000000 */ .word 0x00000000
    /* 932DC 800A2ADC 00000000 */ .word 0x00000000
    /* 932E0 800A2AE0 00000000 */ .word 0x00000000
    /* 932E4 800A2AE4 00000000 */ .word 0x00000000
    /* 932E8 800A2AE8 00000000 */ .word 0x00000000
    /* 932EC 800A2AEC 00000000 */ .word 0x00000000
    /* 932F0 800A2AF0 00000000 */ .word 0x00000000
    /* 932F4 800A2AF4 00000000 */ .word 0x00000000
    /* 932F8 800A2AF8 00000000 */ .word 0x00000000
    /* 932FC 800A2AFC 00000000 */ .word 0x00000000
    /* 93300 800A2B00 00000000 */ .word 0x00000000
    /* 93304 800A2B04 00000000 */ .word 0x00000000
    /* 93308 800A2B08 00000000 */ .word 0x00000000
    /* 9330C 800A2B0C 00000000 */ .word 0x00000000
    /* 93310 800A2B10 00000000 */ .word 0x00000000
    /* 93314 800A2B14 00000000 */ .word 0x00000000
    /* 93318 800A2B18 00000000 */ .word 0x00000000
    /* 9331C 800A2B1C 00000000 */ .word 0x00000000
    /* 93320 800A2B20 00000000 */ .word 0x00000000
    /* 93324 800A2B24 00000000 */ .word 0x00000000
    /* 93328 800A2B28 00000000 */ .word 0x00000000
    /* 9332C 800A2B2C 00000000 */ .word 0x00000000
    /* 93330 800A2B30 00000000 */ .word 0x00000000
    /* 93334 800A2B34 00000000 */ .word 0x00000000
    /* 93338 800A2B38 00000000 */ .word 0x00000000
    /* 9333C 800A2B3C 00000000 */ .word 0x00000000
    /* 93340 800A2B40 00000000 */ .word 0x00000000
    /* 93344 800A2B44 00000000 */ .word 0x00000000
    /* 93348 800A2B48 00000000 */ .word 0x00000000
    /* 9334C 800A2B4C 00000000 */ .word 0x00000000
    /* 93350 800A2B50 00000000 */ .word 0x00000000
    /* 93354 800A2B54 00000000 */ .word 0x00000000
    /* 93358 800A2B58 00000000 */ .word 0x00000000
    /* 9335C 800A2B5C 00000000 */ .word 0x00000000
    /* 93360 800A2B60 00000000 */ .word 0x00000000
    /* 93364 800A2B64 00000000 */ .word 0x00000000
    /* 93368 800A2B68 00000000 */ .word 0x00000000
    /* 9336C 800A2B6C 00000000 */ .word 0x00000000
    /* 93370 800A2B70 00000000 */ .word 0x00000000
    /* 93374 800A2B74 00000000 */ .word 0x00000000
    /* 93378 800A2B78 00000000 */ .word 0x00000000
    /* 9337C 800A2B7C 00000000 */ .word 0x00000000
    /* 93380 800A2B80 00000000 */ .word 0x00000000
    /* 93384 800A2B84 00000000 */ .word 0x00000000
    /* 93388 800A2B88 00000000 */ .word 0x00000000
    /* 9338C 800A2B8C 00000000 */ .word 0x00000000
    /* 93390 800A2B90 00000000 */ .word 0x00000000
    /* 93394 800A2B94 00000000 */ .word 0x00000000
    /* 93398 800A2B98 00000000 */ .word 0x00000000
    /* 9339C 800A2B9C 00000000 */ .word 0x00000000
    /* 933A0 800A2BA0 00000000 */ .word 0x00000000
    /* 933A4 800A2BA4 00000000 */ .word 0x00000000
    /* 933A8 800A2BA8 00000000 */ .word 0x00000000
    /* 933AC 800A2BAC 00000000 */ .word 0x00000000
    /* 933B0 800A2BB0 00000000 */ .word 0x00000000
    /* 933B4 800A2BB4 00000000 */ .word 0x00000000
    /* 933B8 800A2BB8 00000000 */ .word 0x00000000
    /* 933BC 800A2BBC 00000000 */ .word 0x00000000
    /* 933C0 800A2BC0 00000000 */ .word 0x00000000
    /* 933C4 800A2BC4 00000000 */ .word 0x00000000
    /* 933C8 800A2BC8 00000000 */ .word 0x00000000
    /* 933CC 800A2BCC 00000000 */ .word 0x00000000
    /* 933D0 800A2BD0 00000000 */ .word 0x00000000
    /* 933D4 800A2BD4 00000000 */ .word 0x00000000
    /* 933D8 800A2BD8 00000000 */ .word 0x00000000
    /* 933DC 800A2BDC 00000000 */ .word 0x00000000
    /* 933E0 800A2BE0 00000000 */ .word 0x00000000
    /* 933E4 800A2BE4 00000000 */ .word 0x00000000
    /* 933E8 800A2BE8 00000000 */ .word 0x00000000
    /* 933EC 800A2BEC 00000000 */ .word 0x00000000
    /* 933F0 800A2BF0 00000000 */ .word 0x00000000
    /* 933F4 800A2BF4 00000000 */ .word 0x00000000
    /* 933F8 800A2BF8 00000000 */ .word 0x00000000
    /* 933FC 800A2BFC 00000000 */ .word 0x00000000
    /* 93400 800A2C00 00000000 */ .word 0x00000000
    /* 93404 800A2C04 00000000 */ .word 0x00000000
    /* 93408 800A2C08 00000000 */ .word 0x00000000
    /* 9340C 800A2C0C 00000000 */ .word 0x00000000
    /* 93410 800A2C10 00000000 */ .word 0x00000000
    /* 93414 800A2C14 00000000 */ .word 0x00000000
    /* 93418 800A2C18 00000000 */ .word 0x00000000
    /* 9341C 800A2C1C 00000000 */ .word 0x00000000
    /* 93420 800A2C20 00000000 */ .word 0x00000000
    /* 93424 800A2C24 00000000 */ .word 0x00000000
    /* 93428 800A2C28 00000000 */ .word 0x00000000
    /* 9342C 800A2C2C 00000000 */ .word 0x00000000
    /* 93430 800A2C30 00000000 */ .word 0x00000000
    /* 93434 800A2C34 00000000 */ .word 0x00000000
    /* 93438 800A2C38 00000000 */ .word 0x00000000
    /* 9343C 800A2C3C 00000000 */ .word 0x00000000
    /* 93440 800A2C40 00000000 */ .word 0x00000000
    /* 93444 800A2C44 00000000 */ .word 0x00000000
    /* 93448 800A2C48 00000000 */ .word 0x00000000
    /* 9344C 800A2C4C 00000000 */ .word 0x00000000
    /* 93450 800A2C50 00000000 */ .word 0x00000000
    /* 93454 800A2C54 00000000 */ .word 0x00000000
    /* 93458 800A2C58 00000000 */ .word 0x00000000
    /* 9345C 800A2C5C 00000000 */ .word 0x00000000
    /* 93460 800A2C60 00000000 */ .word 0x00000000
    /* 93464 800A2C64 00000000 */ .word 0x00000000
    /* 93468 800A2C68 00000000 */ .word 0x00000000
    /* 9346C 800A2C6C 00000000 */ .word 0x00000000
    /* 93470 800A2C70 00000000 */ .word 0x00000000
    /* 93474 800A2C74 00000000 */ .word 0x00000000
    /* 93478 800A2C78 00000000 */ .word 0x00000000
    /* 9347C 800A2C7C 00000000 */ .word 0x00000000
    /* 93480 800A2C80 00000000 */ .word 0x00000000
    /* 93484 800A2C84 00000000 */ .word 0x00000000
    /* 93488 800A2C88 00000000 */ .word 0x00000000
    /* 9348C 800A2C8C 00000000 */ .word 0x00000000
    /* 93490 800A2C90 00000000 */ .word 0x00000000
    /* 93494 800A2C94 00000000 */ .word 0x00000000
    /* 93498 800A2C98 00000000 */ .word 0x00000000
    /* 9349C 800A2C9C 00000000 */ .word 0x00000000
    /* 934A0 800A2CA0 00000000 */ .word 0x00000000
    /* 934A4 800A2CA4 00000000 */ .word 0x00000000
    /* 934A8 800A2CA8 00000000 */ .word 0x00000000
    /* 934AC 800A2CAC 00000000 */ .word 0x00000000
    /* 934B0 800A2CB0 00000000 */ .word 0x00000000
    /* 934B4 800A2CB4 00000000 */ .word 0x00000000
    /* 934B8 800A2CB8 00000000 */ .word 0x00000000
    /* 934BC 800A2CBC 00000000 */ .word 0x00000000
    /* 934C0 800A2CC0 00000000 */ .word 0x00000000
    /* 934C4 800A2CC4 00000000 */ .word 0x00000000
    /* 934C8 800A2CC8 00000000 */ .word 0x00000000
    /* 934CC 800A2CCC 00000000 */ .word 0x00000000
    /* 934D0 800A2CD0 00000000 */ .word 0x00000000
enddlabel _spu_zerobuf

nonmatching _spu_env

dlabel _spu_env
    /* 934D4 800A2CD4 00000000 */ .word 0x00000000
enddlabel _spu_env

nonmatching _spu_isCalled

dlabel _spu_isCalled
    /* 934D8 800A2CD8 00000000 */ .word 0x00000000
enddlabel _spu_isCalled

nonmatching _spu_RXX

dlabel _spu_RXX
    /* 934DC 800A2CDC 001C801F */ .word 0x1F801C00
enddlabel _spu_RXX

nonmatching dma_spu_madr

dlabel dma_spu_madr
    /* 934E0 800A2CE0 C010801F */ .word 0x1F8010C0
enddlabel dma_spu_madr

nonmatching dma_spu_bcr

dlabel dma_spu_bcr
    /* 934E4 800A2CE4 C410801F */ .word 0x1F8010C4
enddlabel dma_spu_bcr

nonmatching dma_spu_chcr

dlabel dma_spu_chcr
    /* 934E8 800A2CE8 C810801F */ .word 0x1F8010C8
enddlabel dma_spu_chcr

nonmatching dma_dpcr

dlabel dma_dpcr
    /* 934EC 800A2CEC F010801F */ .word 0x1F8010F0
enddlabel dma_dpcr

nonmatching spu_delay

dlabel spu_delay
    /* 934F0 800A2CF0 1410801F */ .word 0x1F801014
enddlabel spu_delay

nonmatching _spu_tsa

dlabel _spu_tsa
    /* 934F4 800A2CF4 */ .short 0x0000
    /* 934F6 800A2CF6 */ .short 0x0000
enddlabel _spu_tsa

nonmatching _spu_transMode

dlabel _spu_transMode
    /* 934F8 800A2CF8 00000000 */ .word 0x00000000
enddlabel _spu_transMode

nonmatching _spu_addrMode

dlabel _spu_addrMode
    /* 934FC 800A2CFC 00000000 */ .word 0x00000000
enddlabel _spu_addrMode

nonmatching _spu_mem_mode

dlabel _spu_mem_mode
    /* 93500 800A2D00 02000000 */ .word 0x00000002
enddlabel _spu_mem_mode

nonmatching _spu_mem_mode_plus

dlabel _spu_mem_mode_plus
    /* 93504 800A2D04 03000000 */ .word 0x00000003
enddlabel _spu_mem_mode_plus

nonmatching _spu_mem_mode_unit

dlabel _spu_mem_mode_unit
    /* 93508 800A2D08 08000000 */ .word 0x00000008
enddlabel _spu_mem_mode_unit

nonmatching _spu_mem_mode_unitM

dlabel _spu_mem_mode_unitM
    /* 9350C 800A2D0C 07000000 */ .word 0x00000007
enddlabel _spu_mem_mode_unitM

nonmatching _spu_inTransfer

dlabel _spu_inTransfer
    /* 93510 800A2D10 01000000 */ .word 0x00000001
enddlabel _spu_inTransfer

nonmatching _spu_transferCallback

dlabel _spu_transferCallback
    /* 93514 800A2D14 00000000 */ .word 0x00000000
enddlabel _spu_transferCallback

nonmatching _spu_IRQCallback

dlabel _spu_IRQCallback
    /* 93518 800A2D18 00000000 */ .word 0x00000000
enddlabel _spu_IRQCallback

nonmatching D_800A2D1C

dlabel D_800A2D1C
    /* 9351C 800A2D1C 07070707 */ .word 0x07070707
    /* 93520 800A2D20 07070707 */ .word 0x07070707
    /* 93524 800A2D24 07070707 */ .word 0x07070707
    /* 93528 800A2D28 07070707 */ .word 0x07070707
enddlabel D_800A2D1C

nonmatching D_800A2D2C

dlabel D_800A2D2C
    /* 9352C 800A2D2C 00000000 */ .word 0x00000000
enddlabel D_800A2D2C

nonmatching spu_madr

dlabel spu_madr
    /* 93530 800A2D30 00000000 */ .word 0x00000000
enddlabel spu_madr

nonmatching spu_bcr

dlabel spu_bcr
    /* 93534 800A2D34 00000000 */ .word 0x00000000
enddlabel spu_bcr

nonmatching _spu_AllocBlockNum

dlabel _spu_AllocBlockNum
    /* 93538 800A2D38 00000000 */ .word 0x00000000
enddlabel _spu_AllocBlockNum

nonmatching _spu_AllocLastNum

dlabel _spu_AllocLastNum
    /* 9353C 800A2D3C 00000000 */ .word 0x00000000
enddlabel _spu_AllocLastNum

nonmatching _spu_memList

dlabel _spu_memList
    /* 93540 800A2D40 00000000 */ .word 0x00000000
enddlabel _spu_memList

nonmatching _spu_rev_startaddr

dlabel _spu_rev_startaddr
    /* 93544 800A2D44 FEFF0000 */ .word 0x0000FFFE
    /* 93548 800A2D48 28FB0000 */ .word 0x0000FB28
    /* 9354C 800A2D4C 18FC0000 */ .word 0x0000FC18
    /* 93550 800A2D50 F8F60000 */ .word 0x0000F6F8
    /* 93554 800A2D54 04F20000 */ .word 0x0000F204
    /* 93558 800A2D58 44EA0000 */ .word 0x0000EA44
    /* 9355C 800A2D5C 28E10000 */ .word 0x0000E128
    /* 93560 800A2D60 F8CF0000 */ .word 0x0000CFF8
    /* 93564 800A2D64 F8CF0000 */ .word 0x0000CFF8
    /* 93568 800A2D68 80F80000 */ .word 0x0000F880
    /* 9356C 800A2D6C 02000000 */ .word 0x00000002
    /* 93570 800A2D70 D8040000 */ .word 0x000004D8
    /* 93574 800A2D74 D8030000 */ .word 0x000003D8
    /* 93578 800A2D78 08090000 */ .word 0x00000908
    /* 9357C 800A2D7C FC0D0000 */ .word 0x00000DFC
    /* 93580 800A2D80 BC150000 */ .word 0x000015BC
    /* 93584 800A2D84 D81E0000 */ .word 0x00001ED8
    /* 93588 800A2D88 08300000 */ .word 0x00003008
    /* 9358C 800A2D8C 08300000 */ .word 0x00003008
    /* 93590 800A2D90 80070000 */ .word 0x00000780
enddlabel _spu_rev_startaddr

nonmatching _spu_rev_param

dlabel _spu_rev_param
    /* 93594 800A2D94 00000000 */ .word 0x00000000
    /* 93598 800A2D98 00000000 */ .word 0x00000000
    /* 9359C 800A2D9C 00000000 */ .word 0x00000000
    /* 935A0 800A2DA0 00000000 */ .word 0x00000000
    /* 935A4 800A2DA4 00000000 */ .word 0x00000000
    /* 935A8 800A2DA8 00000000 */ .word 0x00000000
    /* 935AC 800A2DAC 00000000 */ .word 0x00000000
    /* 935B0 800A2DB0 00000000 */ .word 0x00000000
    /* 935B4 800A2DB4 00000000 */ .word 0x00000000
    /* 935B8 800A2DB8 00000000 */ .word 0x00000000
    /* 935BC 800A2DBC 00000000 */ .word 0x00000000
    /* 935C0 800A2DC0 00000000 */ .word 0x00000000
    /* 935C4 800A2DC4 00000000 */ .word 0x00000000
    /* 935C8 800A2DC8 00000000 */ .word 0x00000000
    /* 935CC 800A2DCC 00000000 */ .word 0x00000000
    /* 935D0 800A2DD0 00000000 */ .word 0x00000000
    /* 935D4 800A2DD4 00000000 */ .word 0x00000000
    /* 935D8 800A2DD8 00000000 */ .word 0x00000000
    /* 935DC 800A2DDC 7D005B00 */ .word 0x005B007D
    /* 935E0 800A2DE0 806DB854 */ .word 0x54B86D80
    /* 935E4 800A2DE4 D0BE0000 */ .word 0x0000BED0
    /* 935E8 800A2DE8 000080BA */ .word 0xBA800000
    /* 935EC 800A2DEC 00580053 */ .word 0x53005800
    /* 935F0 800A2DF0 D6043303 */ .word 0x033304D6
    /* 935F4 800A2DF4 F0032702 */ .word 0x022703F0
    /* 935F8 800A2DF8 7403EF01 */ .word 0x01EF0374
    /* 935FC 800A2DFC 3403B501 */ .word 0x01B50334
    /* 93600 800A2E00 00000000 */ .word 0x00000000
    /* 93604 800A2E04 00000000 */ .word 0x00000000
    /* 93608 800A2E08 00000000 */ .word 0x00000000
    /* 9360C 800A2E0C 00000000 */ .word 0x00000000
    /* 93610 800A2E10 B4013601 */ .word 0x013601B4
    /* 93614 800A2E14 B8005C00 */ .word 0x005C00B8
    /* 93618 800A2E18 00800080 */ .word 0x80008000
    /* 9361C 800A2E1C 00000000 */ .word 0x00000000
    /* 93620 800A2E20 33002500 */ .word 0x00250033
    /* 93624 800A2E24 F070A84F */ .word 0x4FA870F0
    /* 93628 800A2E28 E0BC1044 */ .word 0x4410BCE0
    /* 9362C 800A2E2C F0C0009C */ .word 0x9C00C0F0
    /* 93630 800A2E30 8052C04E */ .word 0x4EC05280
    /* 93634 800A2E34 E4031B03 */ .word 0x031B03E4
    /* 93638 800A2E38 A403AF02 */ .word 0x02AF03A4
    /* 9363C 800A2E3C 72036602 */ .word 0x02660372
    /* 93640 800A2E40 1C035D02 */ .word 0x025D031C
    /* 93644 800A2E44 5C028E01 */ .word 0x018E025C
    /* 93648 800A2E48 2F023501 */ .word 0x0135022F
    /* 9364C 800A2E4C D201B700 */ .word 0x00B701D2
    /* 93650 800A2E50 8F01B500 */ .word 0x00B5018F
    /* 93654 800A2E54 B4008000 */ .word 0x008000B4
    /* 93658 800A2E58 4C002600 */ .word 0x0026004C
    /* 9365C 800A2E5C 00800080 */ .word 0x80008000
    /* 93660 800A2E60 00000000 */ .word 0x00000000
    /* 93664 800A2E64 B1007F00 */ .word 0x007F00B1
    /* 93668 800A2E68 F070A84F */ .word 0x4FA870F0
    /* 9366C 800A2E6C E0BC1045 */ .word 0x4510BCE0
    /* 93670 800A2E70 F0BEC0B4 */ .word 0xB4C0BEF0
    /* 93674 800A2E74 8052C04E */ .word 0x4EC05280
    /* 93678 800A2E78 04096B07 */ .word 0x076B0904
    /* 9367C 800A2E7C 24085F06 */ .word 0x065F0824
    /* 93680 800A2E80 A2071606 */ .word 0x061607A2
    /* 93684 800A2E84 6C07ED05 */ .word 0x05ED076C
    /* 93688 800A2E88 EC052E04 */ .word 0x042E05EC
    /* 9368C 800A2E8C 0F050503 */ .word 0x0305050F
    /* 93690 800A2E90 6204B702 */ .word 0x02B70462
    /* 93694 800A2E94 2F046502 */ .word 0x0265042F
    /* 93698 800A2E98 6402B201 */ .word 0x01B20264
    /* 9369C 800A2E9C 00018000 */ .word 0x00800100
    /* 936A0 800A2EA0 00800080 */ .word 0x80008000
    /* 936A4 800A2EA4 00000000 */ .word 0x00000000
    /* 936A8 800A2EA8 E300A900 */ .word 0x00A900E3
    /* 936AC 800A2EAC 606FA84F */ .word 0x4FA86F60
    /* 936B0 800A2EB0 E0BC1045 */ .word 0x4510BCE0
    /* 936B4 800A2EB4 F0BE80A6 */ .word 0xA680BEF0
    /* 936B8 800A2EB8 8056C052 */ .word 0x52C05680
    /* 936BC 800A2EBC FB0D580B */ .word 0x0B580DFB
    /* 936C0 800A2EC0 090D3C0A */ .word 0x0A3C0D09
    /* 936C4 800A2EC4 D90B7309 */ .word 0x09730BD9
    /* 936C8 800A2EC8 590BDA08 */ .word 0x08DA0B59
    /* 936CC 800A2ECC D908E905 */ .word 0x05E908D9
    /* 936D0 800A2ED0 EC07B004 */ .word 0x04B007EC
    /* 936D4 800A2ED4 EF06D203 */ .word 0x03D206EF
    /* 936D8 800A2ED8 EA051D03 */ .word 0x031D05EA
    /* 936DC 800A2EDC 1C033802 */ .word 0x0238031C
    /* 936E0 800A2EE0 5401AA00 */ .word 0x00AA0154
    /* 936E4 800A2EE4 00800080 */ .word 0x80008000
    /* 936E8 800A2EE8 00000000 */ .word 0x00000000
    /* 936EC 800A2EEC A5013901 */ .word 0x013901A5
    /* 936F0 800A2EF0 00600050 */ .word 0x50006000
    /* 936F4 800A2EF4 004C00B8 */ .word 0xB8004C00
    /* 936F8 800A2EF8 00BC00C0 */ .word 0xC000BC00
    /* 936FC 800A2EFC 0060005C */ .word 0x5C006000
    /* 93700 800A2F00 BA15BB11 */ .word 0x11BB15BA
    /* 93704 800A2F04 C214BD10 */ .word 0x10BD14C2
    /* 93708 800A2F08 BC11C10D */ .word 0x0DC111BC
    /* 9370C 800A2F0C C011C30D */ .word 0x0DC311C0
    /* 93710 800A2F10 C00DC109 */ .word 0x09C10DC0
    /* 93714 800A2F14 C40BC107 */ .word 0x07C10BC4
    /* 93718 800A2F18 000ACD06 */ .word 0x06CD0A00
    /* 9371C 800A2F1C C209C105 */ .word 0x05C109C2
    /* 93720 800A2F20 C0051A04 */ .word 0x041A05C0
    /* 93724 800A2F24 74023A01 */ .word 0x013A0274
    /* 93728 800A2F28 00800080 */ .word 0x80008000
    /* 9372C 800A2F2C 00000000 */ .word 0x00000000
    /* 93730 800A2F30 3D033102 */ .word 0x0231033D
    /* 93734 800A2F34 007E0050 */ .word 0x50007E00
    /* 93738 800A2F38 00B400B0 */ .word 0xB000B400
    /* 9373C 800A2F3C 004C00B0 */ .word 0xB0004C00
    /* 93740 800A2F40 00600054 */ .word 0x54006000
    /* 93744 800A2F44 D61E311A */ .word 0x1A311ED6
    /* 93748 800A2F48 141D3B18 */ .word 0x183B1D14
    /* 9374C 800A2F4C C21BB216 */ .word 0x16B21BC2
    /* 93750 800A2F50 321AEF15 */ .word 0x15EF1A32
    /* 93754 800A2F54 EE155510 */ .word 0x105515EE
    /* 93758 800A2F58 34132D0F */ .word 0x0F2D1334
    /* 9375C 800A2F5C F6115D0C */ .word 0x0C5D11F6
    /* 93760 800A2F60 5610E10A */ .word 0x0AE11056
    /* 93764 800A2F64 E00AA207 */ .word 0x07A20AE0
    /* 93768 800A2F68 64043202 */ .word 0x02320464
    /* 9376C 800A2F6C 00800080 */ .word 0x80008000
    /* 93770 800A2F70 00000000 */ .word 0x00000000
    /* 93774 800A2F74 01000100 */ .word 0x00010001
    /* 93778 800A2F78 FF7FFF7F */ .word 0x7FFF7FFF
    /* 9377C 800A2F7C 00000000 */ .word 0x00000000
    /* 93780 800A2F80 00000081 */ .word 0x81000000
    /* 93784 800A2F84 00000000 */ .word 0x00000000
    /* 93788 800A2F88 FF1FFF0F */ .word 0x0FFF1FFF
    /* 9378C 800A2F8C 05100500 */ .word 0x00051005
    /* 93790 800A2F90 00000000 */ .word 0x00000000
    /* 93794 800A2F94 05100500 */ .word 0x00051005
    /* 93798 800A2F98 00000000 */ .word 0x00000000
    /* 9379C 800A2F9C 00000000 */ .word 0x00000000
    /* 937A0 800A2FA0 00000000 */ .word 0x00000000
    /* 937A4 800A2FA4 00000000 */ .word 0x00000000
    /* 937A8 800A2FA8 04100210 */ .word 0x10021004
    /* 937AC 800A2FAC 04000200 */ .word 0x00020004
    /* 937B0 800A2FB0 00800080 */ .word 0x80008000
    /* 937B4 800A2FB4 00000000 */ .word 0x00000000
    /* 937B8 800A2FB8 01000100 */ .word 0x00010001
    /* 937BC 800A2FBC FF7FFF7F */ .word 0x7FFF7FFF
    /* 937C0 800A2FC0 00000000 */ .word 0x00000000
    /* 937C4 800A2FC4 00000000 */ .word 0x00000000
    /* 937C8 800A2FC8 00000000 */ .word 0x00000000
    /* 937CC 800A2FCC FF1FFF0F */ .word 0x0FFF1FFF
    /* 937D0 800A2FD0 05100500 */ .word 0x00051005
    /* 937D4 800A2FD4 00000000 */ .word 0x00000000
    /* 937D8 800A2FD8 05100500 */ .word 0x00051005
    /* 937DC 800A2FDC 00000000 */ .word 0x00000000
    /* 937E0 800A2FE0 00000000 */ .word 0x00000000
    /* 937E4 800A2FE4 00000000 */ .word 0x00000000
    /* 937E8 800A2FE8 00000000 */ .word 0x00000000
    /* 937EC 800A2FEC 04100210 */ .word 0x10021004
    /* 937F0 800A2FF0 04000200 */ .word 0x00020004
    /* 937F4 800A2FF4 00800080 */ .word 0x80008000
    /* 937F8 800A2FF8 00000000 */ .word 0x00000000
    /* 937FC 800A2FFC 17001300 */ .word 0x00130017
    /* 93800 800A3000 F070A84F */ .word 0x4FA870F0
    /* 93804 800A3004 E0BC1045 */ .word 0x4510BCE0
    /* 93808 800A3008 F0BE0085 */ .word 0x8500BEF0
    /* 9380C 800A300C 805FC054 */ .word 0x54C05F80
    /* 93810 800A3010 7103AF02 */ .word 0x02AF0371
    /* 93814 800A3014 E502DF01 */ .word 0x01DF02E5
    /* 93818 800A3018 B002D701 */ .word 0x01D702B0
    /* 9381C 800A301C 58036A02 */ .word 0x026A0358
    /* 93820 800A3020 D6011E01 */ .word 0x011E01D6
    /* 93824 800A3024 2D01B100 */ .word 0x00B1012D
    /* 93828 800A3028 1F015900 */ .word 0x0059011F
    /* 9382C 800A302C A001E300 */ .word 0x00E301A0
    /* 93830 800A3030 58004000 */ .word 0x00400058
    /* 93834 800A3034 28001400 */ .word 0x00140028
    /* 93838 800A3038 00800080 */ .word 0x80008000
    /* 9383C 800A303C 50730515 */ .word 0x15057350
    /* 93840 800A3040 9C874000 */ .word 0x0040879C
enddlabel _spu_rev_param
