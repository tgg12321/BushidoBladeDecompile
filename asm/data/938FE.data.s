.include "macro.inc"

.section .data, "wa"
    .align 0
    /* 938FE 800A30FE */ .byte 0x00
    /* 938FF 800A30FF */ .byte 0x00

nonmatching g_fight_group_tint_colors

dlabel g_fight_group_tint_colors
    /* 93900 800A3100 03332400 */ .word 0x00243303
    /* 93904 800A3104 50112D00 */ .word 0x002D1150
    /* 93908 800A3108 26000000 */ .word 0x00000026
enddlabel g_fight_group_tint_colors

nonmatching g_fighter_forward_move_rate_limits

dlabel g_fighter_forward_move_rate_limits
    /* 9390C 800A310C */ .short 0x00A6
    /* 9390E 800A310E */ .short 0x00C8
    /* 93910 800A3110 */ .short 0x00E9
    /* 93912 800A3112 */ .short 0x0000
enddlabel g_fighter_forward_move_rate_limits

nonmatching D_800A3114

dlabel D_800A3114
dlabel g_str_on_bbm_800A3114
    /* 93914 800A3114 */ .asciz "ON.BBM"
    .space 1
enddlabel D_800A3114

nonmatching D_800A311C

dlabel D_800A311C
dlabel g_str_gs_bbm_800A311C
    /* 9391C 800A311C */ .asciz "GS.BBM"
    .space 1
enddlabel D_800A311C

nonmatching D_800A3124

dlabel D_800A3124
dlabel g_str_gn_bbm_800A3124
    /* 93924 800A3124 */ .asciz "GN.BBM"
    .space 1
enddlabel D_800A3124

nonmatching D_800A312C

dlabel D_800A312C
    /* 9392C 800A312C 001000FC */ .word 0xFC001000
    /* 93930 800A3130 00000000 */ .word 0x00000000
enddlabel D_800A312C

nonmatching g_fighter_opponent_response_max_separation

dlabel g_fighter_opponent_response_max_separation
    /* 93934 800A3134 B80B0000 */ .word 0x00000BB8
enddlabel g_fighter_opponent_response_max_separation

nonmatching g_unit_y_vector

dlabel g_unit_y_vector
    /* 93938 800A3138 00000010 */ .word 0x10000000
    /* 9393C 800A313C 00000000 */ .word 0x00000000
enddlabel g_unit_y_vector
