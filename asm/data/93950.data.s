.include "macro.inc"

.section .data, "wa"
    .align 0

nonmatching g_str_network

dlabel g_str_network
    /* 93950 800A3150 */ .asciz "NETWORK"
enddlabel g_str_network

nonmatching g_str_team

dlabel g_str_team
    /* 93958 800A3158 */ .asciz "TEAM"
    .space 3
enddlabel g_str_team

nonmatching g_str_vs

dlabel g_str_vs
    /* 93960 800A3160 */ .asciz "VS"
    .space 1
enddlabel g_str_vs

nonmatching g_str_vscpu

dlabel g_str_vscpu
    /* 93964 800A3164 */ .asciz "VSCPU"
    .space 2
enddlabel g_str_vscpu

nonmatching g_str_story

dlabel g_str_story
    /* 9396C 800A316C */ .asciz "STORY"
    .space 2
enddlabel g_str_story
