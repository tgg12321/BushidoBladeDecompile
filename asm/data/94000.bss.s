.include "macro.inc"

/* .bss 0x800A3800..0x801078E0: one label per name the code uses, written by
 * tools/bss_link.py (crt0 zeroes this range; nothing here is in the EXE). */

.bss
    .align 0

/* 800A3800 */
dlabel g_memcard_sw_event_new
    .space 0x4
enddlabel g_memcard_sw_event_new

/* 800A3804 */
dlabel D_800A3804
    .space 0x4
enddlabel D_800A3804

/* 800A3808 */
dlabel D_800A3808
    .space 0x4
enddlabel D_800A3808

/* 800A380C */
dlabel D_800A380C
    .space 0x4
enddlabel D_800A380C

/* 800A3810 */
dlabel g_comb_event_error
    .space 0x4
enddlabel g_comb_event_error

/* 800A3814 */
dlabel D_800A3814
    .space 0x2
enddlabel D_800A3814

/* 800A3816 */
dlabel D_800A3816
    .space 0x1
enddlabel D_800A3816

/* 800A3817 */
dlabel D_800A3817
    .space 0x1
enddlabel D_800A3817

/* 800A3818 */
dlabel D_800A3818
    .space 0x4
enddlabel D_800A3818

/* 800A381C */
dlabel D_800A381C
    .space 0x2
enddlabel D_800A381C

/* 800A381E */
dlabel D_800A381E
    .space 0x2
enddlabel D_800A381E

/* 800A3820 */
dlabel g_draw_queue_cursor
    .space 0x4
enddlabel g_draw_queue_cursor

/* 800A3824 */
dlabel D_800A3824
    .space 0x4
enddlabel D_800A3824

/* 800A3828 */
dlabel D_800A3828
    .space 0x4
enddlabel D_800A3828

/* 800A382C */
dlabel D_800A382C
    .space 0x1
enddlabel D_800A382C

/* 800A382D */
dlabel D_800A382D
    .space 0x1
enddlabel D_800A382D

/* 800A382E */
dlabel D_800A382E
    .space 0x2
enddlabel D_800A382E

/* 800A3830 */
dlabel g_motion_common_buffer
    .space 0x4
enddlabel g_motion_common_buffer

/* 800A3834 */
dlabel D_800A3834
    .space 0x2
enddlabel D_800A3834

/* 800A3836 */
dlabel D_800A3836
    .space 0x2
enddlabel D_800A3836

/* 800A3838 */
dlabel g_memcard_hw_event_ioe
    .space 0x4
enddlabel g_memcard_hw_event_ioe

/* 800A383C */
dlabel g_memcard_hw_event_err
    .space 0x4
enddlabel g_memcard_hw_event_err

/* 800A3840 */
dlabel g_cd_fade_step
    .space 0x4
enddlabel g_cd_fade_step

/* 800A3844 */
dlabel D_800A3844
    .space 0x4
enddlabel D_800A3844

/* 800A3848 */
dlabel g_memcard_hw_event_timeout
    .space 0x4
enddlabel g_memcard_hw_event_timeout

/* 800A384C */
dlabel D_800A384C
    .space 0x4
enddlabel D_800A384C

/* 800A3850 */
dlabel g_memcard_hw_event_new
    .space 0x4
enddlabel g_memcard_hw_event_new

/* 800A3854 */
dlabel g_cd_fade_len
    .space 0x4
enddlabel g_cd_fade_len

/* 800A3858 */
dlabel D_800A3858
    .space 0x4
enddlabel D_800A3858

/* 800A385C */
dlabel D_800A385C
    .space 0x4
enddlabel D_800A385C

/* 800A3860 */
dlabel g_motion_fighter_buffers
    .space 0x4
enddlabel g_motion_fighter_buffers

/* 800A3864 */
dlabel D_800A3864
    .space 0xC
enddlabel D_800A3864

/* 800A3870 */
dlabel g_comb_wait_state
    .space 0x4
enddlabel g_comb_wait_state

/* 800A3874 */
dlabel g_fight_match_cursor
    .space 0x2
enddlabel g_fight_match_cursor

/* 800A3876 */
dlabel D_800A3876
    .space 0x2
enddlabel D_800A3876

/* 800A3878 */
dlabel D_800A3878
    .space 0x4
enddlabel D_800A3878

/* 800A387C */
dlabel D_800A387C
    .space 0x4
enddlabel D_800A387C

/* 800A3880 */
dlabel g_motion_common_loaded
    .space 0x8
enddlabel g_motion_common_loaded

/* 800A3888 */
dlabel g_motion_frame_buffers
    .space 0x4
enddlabel g_motion_frame_buffers

/* 800A388C */
dlabel D_800A388C
    .space 0x4
enddlabel D_800A388C

/* 800A3890 */
dlabel D_800A3890
dlabel g_memcard_op_completion_code
    .space 0x4
enddlabel D_800A3890
enddlabel g_memcard_op_completion_code

/* 800A3894 */
dlabel D_800A3894
    .space 0x4
enddlabel D_800A3894

/* 800A3898 */
dlabel D_800A3898
    .space 0x1
enddlabel D_800A3898

/* 800A3899 */
dlabel D_800A3899
    .space 0x1
enddlabel D_800A3899

/* 800A389A */
dlabel D_800A389A
    .space 0x1
enddlabel D_800A389A

/* 800A389B */
dlabel g_fight_match_count
    .space 0x1
enddlabel g_fight_match_count

/* 800A389C */
dlabel D_800A389C
    .space 0x4
enddlabel D_800A389C

/* 800A38A0 */
dlabel D_800A38A0
    .space 0x4
enddlabel D_800A38A0

/* 800A38A4 */
dlabel D_800A38A4
    .space 0x4
enddlabel D_800A38A4

/* 800A38A8 */
dlabel D_800A38A8
    .space 0x2
enddlabel D_800A38A8

/* 800A38AA */
dlabel D_800A38AA
    .space 0x1
enddlabel D_800A38AA

/* 800A38AB */
dlabel D_800A38AB
    .space 0x1
enddlabel D_800A38AB

/* 800A38AC */
dlabel g_comb_handshake_step
    .space 0x2
enddlabel g_comb_handshake_step

/* 800A38AE */
dlabel D_800A38AE
    .space 0x2
enddlabel D_800A38AE

/* 800A38B0 */
dlabel D_800A38B0
    .space 0x4
enddlabel D_800A38B0

/* 800A38B4 */
dlabel g_prim_buf_cursor
    .space 0x4
enddlabel g_prim_buf_cursor

/* 800A38B8 */
dlabel D_800A38B8
    .space 0x2
enddlabel D_800A38B8

/* 800A38BA */
dlabel D_800A38BA
    .space 0x2
enddlabel D_800A38BA

/* 800A38BC */
dlabel g_rng_state
    .space 0x4
enddlabel g_rng_state

/* 800A38C0 */
dlabel g_motion_frame_set_ids
    .space 0x4
enddlabel g_motion_frame_set_ids

/* 800A38C4 */
dlabel D_800A38C4
    .space 0x4
enddlabel D_800A38C4

/* 800A38C8 */
dlabel g_memcard_file_count
    .space 0x4
enddlabel g_memcard_file_count

/* 800A38CC */
dlabel D_800A38CC
    .space 0x4
enddlabel D_800A38CC

/* 800A38D0 */
dlabel D_800A38D0
    .space 0x4
enddlabel D_800A38D0

/* 800A38D4 */
dlabel D_800A38D4
    .space 0x2
enddlabel D_800A38D4

/* 800A38D6 */
dlabel D_800A38D6
    .space 0x2
enddlabel D_800A38D6

/* 800A38D8 */
dlabel g_gpu_ot256_ptr
    .space 0x4
enddlabel g_gpu_ot256_ptr

/* 800A38DC */
dlabel D_800A38DC
    .space 0x2
enddlabel D_800A38DC

/* 800A38DE */
dlabel D_800A38DE
    .space 0x1
enddlabel D_800A38DE

/* 800A38DF */
dlabel D_800A38DF
    .space 0x1
enddlabel D_800A38DF

/* 800A38E0 */
dlabel D_800A38E0
    .space 0x1
enddlabel D_800A38E0

/* 800A38E1 */
dlabel D_800A38E1
    .space 0x1
enddlabel D_800A38E1

/* 800A38E2 */
dlabel D_800A38E2
    .space 0x2
enddlabel D_800A38E2

/* 800A38E4 */
dlabel D_800A38E4
    .space 0x4
enddlabel D_800A38E4

/* 800A38E8 */
dlabel D_800A38E8
    .space 0x1
enddlabel D_800A38E8

/* 800A38E9 */
dlabel D_800A38E9
    .space 0x3
enddlabel D_800A38E9

/* 800A38EC */
dlabel D_800A38EC
    .space 0x1
enddlabel D_800A38EC

/* 800A38ED */
dlabel D_800A38ED
    .space 0x1
enddlabel D_800A38ED

/* 800A38EE */
dlabel D_800A38EE
    .space 0x2
enddlabel D_800A38EE

/* 800A38F0 */
dlabel D_800A38F0
    .space 0x4
enddlabel D_800A38F0

/* 800A38F4 */
dlabel D_800A38F4
    .space 0x4
enddlabel D_800A38F4

/* 800A38F8 */
dlabel D_800A38F8
    .space 0x4
enddlabel D_800A38F8

/* 800A38FC */
dlabel D_800A38FC
    .space 0x4
enddlabel D_800A38FC

/* 800A3900 */
dlabel D_800A3900
    .space 0x4
enddlabel D_800A3900

/* 800A3904 */
dlabel D_800A3904
    .space 0x2
enddlabel D_800A3904

/* 800A3906 */
dlabel D_800A3906
    .space 0x1
enddlabel D_800A3906

/* 800A3907 */
dlabel D_800A3907
    .space 0x1
enddlabel D_800A3907

/* 800A3908 */
dlabel D_800A3908
    .space 0x4
enddlabel D_800A3908

/* 800A390C */
dlabel D_800A390C
    .space 0x1
enddlabel D_800A390C

/* 800A390D */
dlabel D_800A390D
    .space 0x1
enddlabel D_800A390D

/* 800A390E */
dlabel D_800A390E
    .space 0x1
enddlabel D_800A390E

/* 800A390F */
dlabel D_800A390F
    .space 0x1
enddlabel D_800A390F

/* 800A3910 */
dlabel D_800A3910
    .space 0x2
enddlabel D_800A3910

/* 800A3912 */
dlabel D_800A3912
    .space 0x1
enddlabel D_800A3912

/* 800A3913 */
dlabel D_800A3913
    .space 0x1
enddlabel D_800A3913

/* 800A3914 */
dlabel D_800A3914
    .space 0x1
enddlabel D_800A3914

/* 800A3915 */
dlabel D_800A3915
    .space 0x1
enddlabel D_800A3915

/* 800A3916 */
dlabel D_800A3916
    .space 0x2
enddlabel D_800A3916

/* 800A3918 */
dlabel g_snd_sequence_ticks
    .space 0x6
enddlabel g_snd_sequence_ticks

/* 800A391E */
dlabel D_800A391E
    .space 0x1
enddlabel D_800A391E

/* 800A391F */
dlabel D_800A391F
    .space 0x1
enddlabel D_800A391F

/* 800A3920 */
dlabel D_800A3920
    .space 0x4
enddlabel D_800A3920

/* 800A3924 */
dlabel g_memcard_poll_count
    .space 0x4
enddlabel g_memcard_poll_count

/* 800A3928 */
dlabel g_comb_abort_request
    .space 0x1
enddlabel g_comb_abort_request

/* 800A3929 */
dlabel D_800A3929
    .space 0x7
enddlabel D_800A3929

/* 800A3930 */
dlabel g_font_sprites
    .space 0x400
enddlabel g_font_sprites

/* 800A3D30 */
dlabel g_font_tpage
    .space 0x10
enddlabel g_font_tpage

/* 800A3D40 */
dlabel g_bitstream_delta_accumulators
    .space 0x30
enddlabel g_bitstream_delta_accumulators

/* 800A3D70 */
dlabel D_800A3D70
    .space 0x5D0
enddlabel D_800A3D70

/* 800A4340 */
dlabel D_800A4340
    .space 0x410
enddlabel D_800A4340

/* 800A4750 */
dlabel D_800A4750
    .space 0x1F40
enddlabel D_800A4750

/* 800A6690 */
dlabel D_800A6690
    .space 0x1860
enddlabel D_800A6690

/* 800A7EF0 */
dlabel D_800A7EF0
    .space 0xF0
enddlabel D_800A7EF0

/* 800A7FE0 */
dlabel D_800A7FE0
    .space 0x800
enddlabel D_800A7FE0

/* 800A87E0 */
dlabel D_800A87E0
    .space 0x7D0
enddlabel D_800A87E0

/* 800A8FB0 */
dlabel D_800A8FB0
    .space 0x400
enddlabel D_800A8FB0

/* 800A93B0 */
dlabel D_800A93B0
    .space 0xC
enddlabel D_800A93B0

/* 800A93BC */
dlabel D_800A93BC
    .space 0x474
enddlabel D_800A93BC

/* 800A9830 */
dlabel g_gpu_draw_move_packets
    .space 0xF0
enddlabel g_gpu_draw_move_packets

/* 800A9920 */
dlabel D_800A9920
    .space 0xF0
enddlabel D_800A9920

/* 800A9A10 */
dlabel D_800A9A10
    .space 0x10
enddlabel D_800A9A10

/* 800A9A20 */
dlabel D_800A9A20
    .space 0x4
enddlabel D_800A9A20

/* 800A9A24 */
dlabel g_gpu_store_buf
    .space 0x104
enddlabel g_gpu_store_buf

/* 800A9B28 */
dlabel D_800A9B28
    .space 0x1D0
enddlabel D_800A9B28

/* 800A9CF8 */
dlabel D_800A9CF8
    .space 0x2
enddlabel D_800A9CF8

/* 800A9CFA */
dlabel D_800A9CFA
    .space 0x2
enddlabel D_800A9CFA

/* 800A9CFC */
dlabel D_800A9CFC
    .space 0x2
enddlabel D_800A9CFC

/* 800A9CFE */
dlabel D_800A9CFE
    .space 0x2
enddlabel D_800A9CFE

/* 800A9D00 */
dlabel D_800A9D00
    .space 0x4
enddlabel D_800A9D00

/* 800A9D04 */
dlabel D_800A9D04
    .space 0x4
enddlabel D_800A9D04

/* 800A9D08 */
dlabel D_800A9D08
    .space 0x8
enddlabel D_800A9D08

/* 800A9D10 */
dlabel g_arena
    .space 0x44FF0
enddlabel g_arena

/* 800EED00 */
dlabel D_800EED00
    .space 0x10
enddlabel D_800EED00

/* 800EED10 */
dlabel g_arena_blocks
    .space 0x4
enddlabel g_arena_blocks

/* 800EED14 */
dlabel D_800EED14
    .space 0x4
enddlabel D_800EED14

/* 800EED18 */
dlabel D_800EED18
    .space 0x4
enddlabel D_800EED18

/* 800EED1C */
dlabel D_800EED1C
    .space 0x94
enddlabel D_800EED1C

/* 800EEDB0 */
dlabel g_math_light_plane_projection
    .space 0x2
enddlabel g_math_light_plane_projection

/* 800EEDB2 */
dlabel D_800EEDB2
    .space 0x2
enddlabel D_800EEDB2

/* 800EEDB4 */
dlabel D_800EEDB4
    .space 0x2
enddlabel D_800EEDB4

/* 800EEDB6 */
dlabel D_800EEDB6
    .space 0x2
enddlabel D_800EEDB6

/* 800EEDB8 */
dlabel D_800EEDB8
    .space 0x2
enddlabel D_800EEDB8

/* 800EEDBA */
dlabel D_800EEDBA
    .space 0x2
enddlabel D_800EEDBA

/* 800EEDBC */
dlabel D_800EEDBC
    .space 0x2
enddlabel D_800EEDBC

/* 800EEDBE */
dlabel D_800EEDBE
    .space 0x2
enddlabel D_800EEDBE

/* 800EEDC0 */
dlabel D_800EEDC0
    .space 0x10
enddlabel D_800EEDC0

/* 800EEDD0 */
dlabel D_800EEDD0
    .space 0x6
enddlabel D_800EEDD0

/* 800EEDD6 */
dlabel D_800EEDD6
    .space 0x2
enddlabel D_800EEDD6

/* 800EEDD8 */
dlabel D_800EEDD8
    .space 0x18
enddlabel D_800EEDD8

/* 800EEDF0 */
dlabel D_800EEDF0
    .space 0x8
enddlabel D_800EEDF0

/* 800EEDF8 */
dlabel D_800EEDF8
    .space 0x8
enddlabel D_800EEDF8

/* 800EEE00 */
dlabel D_800EEE00
    .space 0x2
enddlabel D_800EEE00

/* 800EEE02 */
dlabel D_800EEE02
    .space 0x1A
enddlabel D_800EEE02

/* 800EEE1C */
dlabel D_800EEE1C
    .space 0x4
enddlabel D_800EEE1C

/* 800EEE20 */
dlabel D_800EEE20
    .space 0x4
enddlabel D_800EEE20

/* 800EEE24 */
dlabel D_800EEE24
    .space 0x24C
enddlabel D_800EEE24

/* 800EF070 */
dlabel D_800EF070
    .space 0x1
enddlabel D_800EF070

/* 800EF071 */
dlabel D_800EF071
    .space 0x5
enddlabel D_800EF071

/* 800EF076 */
dlabel D_800EF076
    .space 0x2
enddlabel D_800EF076

/* 800EF078 */
dlabel D_800EF078
    .space 0x2
enddlabel D_800EF078

/* 800EF07A */
dlabel D_800EF07A
    .space 0x2
enddlabel D_800EF07A

/* 800EF07C */
dlabel D_800EF07C
    .space 0x4
enddlabel D_800EF07C

/* 800EF080 */
dlabel D_800EF080
    .space 0x2
enddlabel D_800EF080

/* 800EF082 */
dlabel D_800EF082
    .space 0x2
enddlabel D_800EF082

/* 800EF084 */
dlabel D_800EF084
    .space 0x38
enddlabel D_800EF084

/* 800EF0BC */
dlabel D_800EF0BC
    .space 0x4
enddlabel D_800EF0BC

/* 800EF0C0 */
dlabel D_800EF0C0
    .space 0x4
enddlabel D_800EF0C0

/* 800EF0C4 */
dlabel D_800EF0C4
    .space 0x14
enddlabel D_800EF0C4

/* 800EF0D8 */
dlabel D_800EF0D8
    .space 0x90
enddlabel D_800EF0D8

/* 800EF168 */
dlabel D_800EF168
    .space 0x3F0
enddlabel D_800EF168

/* 800EF558 */
dlabel D_800EF558
    .space 0x44
enddlabel D_800EF558

/* 800EF59C */
dlabel D_800EF59C
    .space 0x220
enddlabel D_800EF59C

/* 800EF7BC */
dlabel D_800EF7BC
    .space 0x4
enddlabel D_800EF7BC

/* 800EF7C0 */
dlabel D_800EF7C0
    .space 0x40
enddlabel D_800EF7C0

/* 800EF800 */
dlabel D_800EF800
    .space 0x4
enddlabel D_800EF800

/* 800EF804 */
dlabel D_800EF804
    .space 0x44
enddlabel D_800EF804

/* 800EF848 */
dlabel D_800EF848
    .space 0x138
enddlabel D_800EF848

/* 800EF980 */
dlabel D_800EF980
    .space 0x72
enddlabel D_800EF980

/* 800EF9F2 */
dlabel D_800EF9F2
    .space 0x2
enddlabel D_800EF9F2

/* 800EF9F4 */
dlabel D_800EF9F4
    .space 0x4
enddlabel D_800EF9F4

/* 800EF9F8 */
dlabel D_800EF9F8
    .space 0x8
enddlabel D_800EF9F8

/* 800EFA00 */
dlabel D_800EFA00
    .space 0xE8
enddlabel D_800EFA00

/* 800EFAE8 */
dlabel D_800EFAE8
    .space 0x24
enddlabel D_800EFAE8

/* 800EFB0C */
dlabel D_800EFB0C
    .space 0x8
enddlabel D_800EFB0C

/* 800EFB14 */
dlabel D_800EFB14
    .space 0x4
enddlabel D_800EFB14

/* 800EFB18 */
dlabel D_800EFB18
    .space 0x4
enddlabel D_800EFB18

/* 800EFB1C */
dlabel D_800EFB1C
    .space 0x4
enddlabel D_800EFB1C

/* 800EFB20 */
dlabel D_800EFB20
    .space 0x4
enddlabel D_800EFB20

/* 800EFB24 */
dlabel D_800EFB24
    .space 0x4
enddlabel D_800EFB24

/* 800EFB28 */
dlabel D_800EFB28
    .space 0x10
enddlabel D_800EFB28

/* 800EFB38 */
dlabel g_vab_vb_sbaddr
    .space 0x40
enddlabel g_vab_vb_sbaddr

/* 800EFB78 */
dlabel g_snd_queue
    .space 0x4
enddlabel g_snd_queue

/* 800EFB7C */
dlabel D_800EFB7C
    .space 0x1
enddlabel D_800EFB7C

/* 800EFB7D */
dlabel D_800EFB7D
    .space 0xBB
enddlabel D_800EFB7D

/* 800EFC38 */
dlabel g_vab_rec_ptr
    .space 0x40
enddlabel g_vab_rec_ptr

/* 800EFC78 */
dlabel g_eff_particles
    .space 0x12
enddlabel g_eff_particles

/* 800EFC8A */
dlabel D_800EFC8A
    .space 0xEEE
enddlabel D_800EFC8A

/* 800F0B78 */
dlabel D_800F0B78
    .space 0x4
enddlabel D_800F0B78

/* 800F0B7C */
dlabel D_800F0B7C
    .space 0x1C
enddlabel D_800F0B7C

/* 800F0B98 */
dlabel D_800F0B98
    .space 0xC
enddlabel D_800F0B98

/* 800F0BA4 */
dlabel D_800F0BA4
    .space 0x4
enddlabel D_800F0BA4

/* 800F0BA8 */
dlabel D_800F0BA8
    .space 0x24
enddlabel D_800F0BA8

/* 800F0BCC */
dlabel D_800F0BCC
    .space 0x20
enddlabel D_800F0BCC

/* 800F0BEC */
dlabel D_800F0BEC
    .space 0x18
enddlabel D_800F0BEC

/* 800F0C04 */
dlabel D_800F0C04
    .space 0xC
enddlabel D_800F0C04

/* 800F0C10 */
dlabel D_800F0C10
    .space 0x4
enddlabel D_800F0C10

/* 800F0C14 */
dlabel D_800F0C14
    .space 0x4
enddlabel D_800F0C14

/* 800F0C18 */
dlabel D_800F0C18
    .space 0x88
enddlabel D_800F0C18

/* 800F0CA0 */
dlabel D_800F0CA0
    .space 0xD8
enddlabel D_800F0CA0

/* 800F0D78 */
dlabel D_800F0D78
    .space 0x4
enddlabel D_800F0D78

/* 800F0D7C */
dlabel D_800F0D7C
    .space 0xBC
enddlabel D_800F0D7C

/* 800F0E38 */
dlabel D_800F0E38
    .space 0x4
enddlabel D_800F0E38

/* 800F0E3C */
dlabel D_800F0E3C
    .space 0x4
enddlabel D_800F0E3C

/* 800F0E40 */
dlabel D_800F0E40
    .space 0x88
enddlabel D_800F0E40

/* 800F0EC8 */
dlabel D_800F0EC8
    .space 0x4
enddlabel D_800F0EC8

/* 800F0ECC */
dlabel D_800F0ECC
    .space 0x4
enddlabel D_800F0ECC

/* 800F0ED0 */
dlabel D_800F0ED0
    .space 0xE8
enddlabel D_800F0ED0

/* 800F0FB8 */
dlabel D_800F0FB8
    .space 0x4
enddlabel D_800F0FB8

/* 800F0FBC */
dlabel D_800F0FBC
    .space 0x4
enddlabel D_800F0FBC

/* 800F0FC0 */
dlabel D_800F0FC0
    .space 0x40
enddlabel D_800F0FC0

/* 800F1000 */
dlabel D_800F1000
    .space 0x2
enddlabel D_800F1000

/* 800F1002 */
dlabel D_800F1002
    .space 0x2
enddlabel D_800F1002

/* 800F1004 */
dlabel D_800F1004
    .space 0x9C
enddlabel D_800F1004

/* 800F10A0 */
dlabel D_800F10A0
    .space 0x2
enddlabel D_800F10A0

/* 800F10A2 */
dlabel D_800F10A2
    .space 0x2
enddlabel D_800F10A2

/* 800F10A4 */
dlabel D_800F10A4
    .space 0x2C
enddlabel D_800F10A4

/* 800F10D0 */
dlabel g_eff_command_phase
    .space 0x70
enddlabel g_eff_command_phase

/* 800F1140 */
dlabel D_800F1140
    .space 0x10
enddlabel D_800F1140

/* 800F1150 */
dlabel g_eff_command_status
    .space 0x1
enddlabel g_eff_command_status

/* 800F1151 */
dlabel D_800F1151
    .space 0x1
enddlabel D_800F1151

/* 800F1152 */
dlabel D_800F1152
    .space 0x2
enddlabel D_800F1152

/* 800F1154 */
dlabel D_800F1154
    .space 0x4
enddlabel D_800F1154

/* 800F1158 */
dlabel D_800F1158
    .space 0x1
enddlabel D_800F1158

/* 800F1159 */
dlabel D_800F1159
    .space 0x2
enddlabel D_800F1159

/* 800F115B */
dlabel D_800F115B
    .space 0x1
enddlabel D_800F115B

/* 800F115C */
dlabel D_800F115C
    .space 0x4
enddlabel D_800F115C

/* 800F1160 */
dlabel D_800F1160
    .space 0x4
enddlabel D_800F1160

/* 800F1164 */
dlabel D_800F1164
    .space 0x4
enddlabel D_800F1164

/* 800F1168 */
dlabel D_800F1168
    .space 0x2
enddlabel D_800F1168

/* 800F116A */
dlabel D_800F116A
    .space 0x2
enddlabel D_800F116A

/* 800F116C */
dlabel g_eff_command_block
    .space 0xC
enddlabel g_eff_command_block

/* 800F1178 */
dlabel D_800F1178
    .space 0x4
enddlabel D_800F1178

/* 800F117C */
dlabel D_800F117C
    .space 0x1C
enddlabel D_800F117C

/* 800F1198 */
dlabel g_eff_billboards
    .space 0x4
enddlabel g_eff_billboards

/* 800F119C */
dlabel D_800F119C
    .space 0x4
enddlabel D_800F119C

/* 800F11A0 */
dlabel D_800F11A0
    .space 0x40
enddlabel D_800F11A0

/* 800F11E0 */
dlabel D_800F11E0
    .space 0x258
enddlabel D_800F11E0

/* 800F1438 */
dlabel D_800F1438
    .space 0x60
enddlabel D_800F1438

/* 800F1498 */
dlabel D_800F1498
    .space 0x3A0
enddlabel D_800F1498

/* 800F1838 */
dlabel patch0
    .space 0x4
enddlabel patch0

/* 800F183C */
dlabel patch0_plus_0x4
    .space 0x4
enddlabel patch0_plus_0x4

/* 800F1840 */
dlabel patch0_plus_0x8
    .space 0x4
enddlabel patch0_plus_0x8

/* 800F1844 */
dlabel patch0_plus_0xC
    .space 0x4
enddlabel patch0_plus_0xC

/* 800F1848 */
dlabel D_800F1848
dlabel g_rand_state
    .space 0x8
enddlabel D_800F1848
enddlabel g_rand_state

/* 800F1850 */
dlabel column
    .space 0x8
enddlabel column

/* 800F1858 */
dlabel D_800F1858
    .space 0x4
enddlabel D_800F1858

/* 800F185C */
dlabel D_800F185C
    .space 0x4
enddlabel D_800F185C

/* 800F1860 */
dlabel D_800F1860
    .space 0x4
enddlabel D_800F1860

/* 800F1864 */
dlabel D_800F1864
    .space 0x4
enddlabel D_800F1864

/* 800F1868 */
dlabel D_800F1868
    .space 0x4
enddlabel D_800F1868

/* 800F186C */
dlabel D_800F186C
    .space 0x30
enddlabel D_800F186C

/* 800F189C */
dlabel ctlbuf
    .space 0x104
enddlabel ctlbuf

/* 800F19A0 */
dlabel Result
    .space 0x8
enddlabel Result

/* 800F19A8 */
dlabel Result_plus_0x8
    .space 0x8
enddlabel Result_plus_0x8

/* 800F19B0 */
dlabel Result_plus_0x10
    .space 0x8
enddlabel Result_plus_0x10

/* 800F19B8 */
dlabel Alarm
    .space 0x18
enddlabel Alarm

/* 800F19D0 */
dlabel D_800F19D0
    .space 0x108
enddlabel D_800F19D0

/* 800F1AD8 */
dlabel regs
    .space 0x10
enddlabel regs

/* 800F1AE8 */
dlabel CombWaitCallback
    .space 0x4
enddlabel CombWaitCallback

/* 800F1AEC */
dlabel sen
    .space 0x10
enddlabel sen

/* 800F1AFC */
dlabel D_800F1AFC
    .space 0x14
enddlabel D_800F1AFC

/* 800F1B10 */
dlabel _svm_okon1
    .space 0x2
enddlabel _svm_okon1

/* 800F1B12 */
dlabel _svm_okon2
    .space 0x2
enddlabel _svm_okon2

/* 800F1B14 */
dlabel D_800F1B14
    .space 0x4
enddlabel D_800F1B14

/* 800F1B18 */
dlabel g_motion_decoders
    .space 0x1050
enddlabel g_motion_decoders

/* 800F2B68 */
dlabel D_800F2B68
    .space 0x8
enddlabel D_800F2B68

/* 800F2B70 */
dlabel D_800F2B70
    .space 0x7D0
enddlabel D_800F2B70

/* 800F3340 */
dlabel D_800F3340
    .space 0x98
enddlabel D_800F3340

/* 800F33D8 */
dlabel D_800F33D8
    .space 0x100
enddlabel D_800F33D8

/* 800F34D8 */
dlabel D_800F34D8
    .space 0x1940
enddlabel D_800F34D8

/* 800F4E18 */
dlabel _svm_voice
    .space 0x6
enddlabel _svm_voice

/* 800F4E1E */
dlabel _svm_voice_plus_0x6
    .space 0x17
enddlabel _svm_voice_plus_0x6

/* 800F4E35 */
dlabel _svm_voice_plus_0x1D
    .space 0x1
enddlabel _svm_voice_plus_0x1D

/* 800F4E36 */
dlabel D_800F4E36
    .space 0xC
enddlabel D_800F4E36

/* 800F4E42 */
dlabel D_800F4E42
    .space 0x4E6
enddlabel D_800F4E42

/* 800F5328 */
dlabel D_800F5328
    .space 0x4
enddlabel D_800F5328

/* 800F532C */
dlabel D_800F532C
    .space 0x4
enddlabel D_800F532C

/* 800F5330 */
dlabel D_800F5330
    .space 0x14
enddlabel D_800F5330

/* 800F5344 */
dlabel D_800F5344
    .space 0x3
enddlabel D_800F5344

/* 800F5347 */
dlabel D_800F5347
    .space 0x11
enddlabel D_800F5347

/* 800F5358 */
dlabel D_800F5358
    .space 0x2
enddlabel D_800F5358

/* 800F535A */
dlabel D_800F535A
    .space 0x2
enddlabel D_800F535A

/* 800F535C */
dlabel D_800F535C
    .space 0x4
enddlabel D_800F535C

/* 800F5360 */
dlabel D_800F5360
    .space 0x2
enddlabel D_800F5360

/* 800F5362 */
dlabel D_800F5362
    .space 0x2
enddlabel D_800F5362

/* 800F5364 */
dlabel D_800F5364
    .space 0x4
enddlabel D_800F5364

/* 800F5368 */
dlabel D_800F5368
    .space 0x8
enddlabel D_800F5368

/* 800F5370 */
dlabel g_scratchpad_save
    .space 0x3E0
enddlabel g_scratchpad_save

/* 800F5750 */
dlabel _svm_rattr
    .space 0x18
enddlabel _svm_rattr

/* 800F5768 */
dlabel g_gpu_ot256_db
    .space 0x800
enddlabel g_gpu_ot256_db

/* 800F5F68 */
dlabel D_800F5F68
    .space 0x4
enddlabel D_800F5F68

/* 800F5F6C */
dlabel D_800F5F6C
    .space 0x374
enddlabel D_800F5F6C

/* 800F62E0 */
dlabel D_800F62E0
    .space 0x2
enddlabel D_800F62E0

/* 800F62E2 */
dlabel D_800F62E2
    .space 0x5E
enddlabel D_800F62E2

/* 800F6340 */
dlabel D_800F6340
    .space 0x2
enddlabel D_800F6340

/* 800F6342 */
dlabel D_800F6342
    .space 0x120
enddlabel D_800F6342

/* 800F6462 */
dlabel D_800F6462
    .space 0x17E
enddlabel D_800F6462

/* 800F65E0 */
dlabel _svm_sreg_dirty
    .space 0x18
enddlabel _svm_sreg_dirty

/* 800F65F8 */
dlabel g_fight_match_scores
    .space 0x10
enddlabel g_fight_match_scores

/* 800F6608 */
dlabel D_800F6608
    .space 0x4
enddlabel D_800F6608

/* 800F660C */
dlabel D_800F660C
    .space 0x4
enddlabel D_800F660C

/* 800F6610 */
dlabel D_800F6610
    .space 0x8
enddlabel D_800F6610

/* 800F6618 */
dlabel D_800F6618
    .space 0x2
enddlabel D_800F6618

/* 800F661A */
dlabel D_800F661A
    .space 0x2
enddlabel D_800F661A

/* 800F661C */
dlabel D_800F661C
    .space 0x4
enddlabel D_800F661C

/* 800F6620 */
dlabel D_800F6620
    .space 0x6
enddlabel D_800F6620

/* 800F6626 */
dlabel D_800F6626
    .space 0x1
enddlabel D_800F6626

/* 800F6627 */
dlabel D_800F6627
    .space 0x11
enddlabel D_800F6627

/* 800F6638 */
dlabel D_800F6638
    .space 0x2
enddlabel D_800F6638

/* 800F663A */
dlabel D_800F663A
    .space 0x2
enddlabel D_800F663A

/* 800F663C */
dlabel D_800F663C
    .space 0x4
enddlabel D_800F663C

/* 800F6640 */
dlabel D_800F6640
    .space 0x2
enddlabel D_800F6640

/* 800F6642 */
dlabel D_800F6642
    .space 0x2
enddlabel D_800F6642

/* 800F6644 */
dlabel D_800F6644
    .space 0xC
enddlabel D_800F6644

/* 800F6650 */
dlabel D_800F6650
    .space 0x2
enddlabel D_800F6650

/* 800F6652 */
dlabel D_800F6652
dlabel g_color_mode
    .space 0x2
enddlabel D_800F6652
enddlabel g_color_mode

/* 800F6654 */
dlabel D_800F6654
    .space 0x2
enddlabel D_800F6654

/* 800F6656 */
dlabel D_800F6656
    .space 0x2
enddlabel D_800F6656

/* 800F6658 */
dlabel D_800F6658
    .space 0x2
enddlabel D_800F6658

/* 800F665A */
dlabel D_800F665A
    .space 0x2
enddlabel D_800F665A

/* 800F665C */
dlabel D_800F665C
    .space 0x4
enddlabel D_800F665C

/* 800F6660 */
dlabel _svm_vab_pg
    .space 0x40
enddlabel _svm_vab_pg

/* 800F66A0 */
dlabel D_800F66A0
    .space 0x18
enddlabel D_800F66A0

/* 800F66B8 */
dlabel _svm_vab_vh
    .space 0x40
enddlabel _svm_vab_vh

/* 800F66F8 */
dlabel _svm_damper
    .space 0x8
enddlabel _svm_damper

/* 800F6700 */
dlabel _svm_vab_tn
    .space 0x40
enddlabel _svm_vab_tn

/* 800F6740 */
dlabel g_softbody_slots
    .space 0x1A0
enddlabel g_softbody_slots

/* 800F68E0 */
dlabel g_replay_model_draws
    .space 0x9B8
enddlabel g_replay_model_draws

/* 800F7298 */
dlabel D_800F7298
    .space 0x188
enddlabel D_800F7298

/* 800F7420 */
dlabel _spu_RQ
    .space 0x2
enddlabel _spu_RQ

/* 800F7422 */
dlabel _spu_RQ_plus_0x2
    .space 0x2
enddlabel _spu_RQ_plus_0x2

/* 800F7424 */
dlabel _spu_RQ_plus_0x4
    .space 0x2
enddlabel _spu_RQ_plus_0x4

/* 800F7426 */
dlabel _spu_RQ_plus_0x6
    .space 0x12
enddlabel _spu_RQ_plus_0x6

/* 800F7438 */
dlabel g_gpu_db
    .space 0x8120
enddlabel g_gpu_db

/* 800FF558 */
dlabel D_800FF558
    .space 0x20
enddlabel D_800FF558

/* 800FF578 */
dlabel _svm_stereo_mono
    .space 0x8
enddlabel _svm_stereo_mono

/* 800FF580 */
dlabel g_pad_buf
    .space 0x48
enddlabel g_pad_buf

/* 800FF5C8 */
dlabel g_camera_view_offset
    .space 0x48
enddlabel g_camera_view_offset

/* 800FF610 */
dlabel D_800FF610
    .space 0x20
enddlabel D_800FF610

/* 800FF630 */
dlabel _snd_ev_flag
    .space 0x4
enddlabel _snd_ev_flag

/* 800FF634 */
dlabel kMaxPrograms
    .space 0x4
enddlabel kMaxPrograms

/* 800FF638 */
dlabel D_800FF638
    .space 0x68
enddlabel D_800FF638

/* 800FF6A0 */
dlabel _svm_pg
    .space 0x8
enddlabel _svm_pg

/* 800FF6A8 */
dlabel D_800FF6A8
    .space 0x251C
enddlabel D_800FF6A8

/* 80101BC4 */
dlabel _svm_vh
    .space 0x4
enddlabel _svm_vh

/* 80101BC8 */
dlabel _svm_tn
    .space 0x4
enddlabel _svm_tn

/* 80101BCC */
dlabel _SsVmMaxVoice
    .space 0x4
enddlabel _SsVmMaxVoice

/* 80101BD0 */
dlabel D_80101BD0
    .space 0x20
enddlabel D_80101BD0

/* 80101BF0 */
dlabel g_replay_events
    .space 0x200
enddlabel g_replay_events

/* 80101DF0 */
dlabel D_80101DF0
    .space 0x68
enddlabel D_80101DF0

/* 80101E58 */
dlabel D_80101E58
    .space 0x70
enddlabel D_80101E58

/* 80101EC8 */
dlabel D_80101EC8
    .space 0x38
enddlabel D_80101EC8

/* 80101F00 */
dlabel D_80101F00
    .space 0x12
enddlabel D_80101F00

/* 80101F12 */
dlabel D_80101F12
    .space 0x20
enddlabel D_80101F12

/* 80101F32 */
dlabel D_80101F32
    .space 0x1A
enddlabel D_80101F32

/* 80101F4C */
dlabel D_80101F4C
    .space 0x2
enddlabel D_80101F4C

/* 80101F4E */
dlabel D_80101F4E
    .space 0x10
enddlabel D_80101F4E

/* 80101F5E */
dlabel D_80101F5E
    .space 0x1D
enddlabel D_80101F5E

/* 80101F7B */
dlabel D_80101F7B
    .space 0x5
enddlabel D_80101F7B

/* 80101F80 */
dlabel D_80101F80
    .space 0x4
enddlabel D_80101F80

/* 80101F84 */
dlabel D_80101F84
    .space 0x4
enddlabel D_80101F84

/* 80101F88 */
dlabel D_80101F88
    .space 0x8
enddlabel D_80101F88

/* 80101F90 */
dlabel D_80101F90
    .space 0x10
enddlabel D_80101F90

/* 80101FA0 */
dlabel D_80101FA0
    .space 0x4
enddlabel D_80101FA0

/* 80101FA4 */
dlabel D_80101FA4
    .space 0x4
enddlabel D_80101FA4

/* 80101FA8 */
dlabel D_80101FA8
    .space 0x8
enddlabel D_80101FA8

/* 80101FB0 */
dlabel D_80101FB0
    .space 0x4
enddlabel D_80101FB0

/* 80101FB4 */
dlabel D_80101FB4
    .space 0x4
enddlabel D_80101FB4

/* 80101FB8 */
dlabel D_80101FB8
    .space 0x4
enddlabel D_80101FB8

/* 80101FBC */
dlabel D_80101FBC
    .space 0x4
enddlabel D_80101FBC

/* 80101FC0 */
dlabel D_80101FC0
    .space 0x4
enddlabel D_80101FC0

/* 80101FC4 */
dlabel D_80101FC4
    .space 0x8
enddlabel D_80101FC4

/* 80101FCC */
dlabel D_80101FCC
    .space 0x4
enddlabel D_80101FCC

/* 80101FD0 */
dlabel D_80101FD0
    .space 0x4
enddlabel D_80101FD0

/* 80101FD4 */
dlabel D_80101FD4
    .space 0x8
enddlabel D_80101FD4

/* 80101FDC */
dlabel D_80101FDC
    .space 0x4
enddlabel D_80101FDC

/* 80101FE0 */
dlabel D_80101FE0
    .space 0x4
enddlabel D_80101FE0

/* 80101FE4 */
dlabel D_80101FE4
    .space 0x8
enddlabel D_80101FE4

/* 80101FEC */
dlabel D_80101FEC
    .space 0x4
enddlabel D_80101FEC

/* 80101FF0 */
dlabel D_80101FF0
    .space 0x4
enddlabel D_80101FF0

/* 80101FF4 */
dlabel D_80101FF4
    .space 0x8
enddlabel D_80101FF4

/* 80101FFC */
dlabel D_80101FFC
    .space 0x4
enddlabel D_80101FFC

/* 80102000 */
dlabel D_80102000
    .space 0x4
enddlabel D_80102000

/* 80102004 */
dlabel D_80102004
    .space 0x8
enddlabel D_80102004

/* 8010200C */
dlabel D_8010200C
    .space 0x4
enddlabel D_8010200C

/* 80102010 */
dlabel D_80102010
    .space 0x4
enddlabel D_80102010

/* 80102014 */
dlabel D_80102014
    .space 0x2
enddlabel D_80102014

/* 80102016 */
dlabel D_80102016
    .space 0x2
enddlabel D_80102016

/* 80102018 */
dlabel D_80102018
    .space 0x2
enddlabel D_80102018

/* 8010201A */
dlabel D_8010201A
    .space 0x16
enddlabel D_8010201A

/* 80102030 */
dlabel D_80102030
    .space 0xC
enddlabel D_80102030

/* 8010203C */
dlabel D_8010203C
    .space 0x4
enddlabel D_8010203C

/* 80102040 */
dlabel D_80102040
    .space 0x4
enddlabel D_80102040

/* 80102044 */
dlabel D_80102044
    .space 0x10
enddlabel D_80102044

/* 80102054 */
dlabel D_80102054
    .space 0x4
enddlabel D_80102054

/* 80102058 */
dlabel D_80102058
    .space 0x4
enddlabel D_80102058

/* 8010205C */
dlabel D_8010205C
    .space 0x36
enddlabel D_8010205C

/* 80102092 */
dlabel D_80102092
    .space 0x2E
enddlabel D_80102092

/* 801020C0 */
dlabel D_801020C0
    .space 0x54
enddlabel D_801020C0

/* 80102114 */
dlabel D_80102114
    .space 0x3A
enddlabel D_80102114

/* 8010214E */
dlabel D_8010214E
    .space 0x94
enddlabel D_8010214E

/* 801021E2 */
dlabel D_801021E2
    .space 0x138
enddlabel D_801021E2

/* 8010231A */
dlabel D_8010231A
    .space 0x64
enddlabel D_8010231A

/* 8010237E */
dlabel D_8010237E
    .space 0x2C
enddlabel D_8010237E

/* 801023AA */
dlabel D_801023AA
    .space 0x42
enddlabel D_801023AA

/* 801023EC */
dlabel D_801023EC
    .space 0x8
enddlabel D_801023EC

/* 801023F4 */
dlabel D_801023F4
    .space 0x14
enddlabel D_801023F4

/* 80102408 */
dlabel D_80102408
    .space 0x8
enddlabel D_80102408

/* 80102410 */
dlabel D_80102410
    .space 0x18A
enddlabel D_80102410

/* 8010259A */
dlabel D_8010259A
    .space 0x94
enddlabel D_8010259A

/* 8010262E */
dlabel D_8010262E
    .space 0x132
enddlabel D_8010262E

/* 80102760 */
dlabel g_motion_common_sections
    .space 0x18
enddlabel g_motion_common_sections

/* 80102778 */
dlabel D_80102778
    .space 0x10
enddlabel D_80102778

/* 80102788 */
dlabel g_pad_state
    .space 0x2
enddlabel g_pad_state

/* 8010278A */
dlabel g_pad_state_plus_0x2
    .space 0x2
enddlabel g_pad_state_plus_0x2

/* 8010278C */
dlabel g_pad_state_plus_0x4
    .space 0x2
enddlabel g_pad_state_plus_0x4

/* 8010278E */
dlabel g_pad_state_plus_0x6
    .space 0x6
enddlabel g_pad_state_plus_0x6

/* 80102794 */
dlabel g_pad_state_plus_0xC
    .space 0x4
enddlabel g_pad_state_plus_0xC

/* 80102798 */
dlabel g_pad_state_plus_0x10
    .space 0x4
enddlabel g_pad_state_plus_0x10

/* 8010279C */
dlabel g_pad_state_plus_0x14
    .space 0x4
enddlabel g_pad_state_plus_0x14

/* 801027A0 */
dlabel D_801027A0
    .space 0x10
enddlabel D_801027A0

/* 801027B0 */
dlabel g_motion_fighter_sections
    .space 0xC
enddlabel g_motion_fighter_sections

/* 801027BC */
dlabel D_801027BC
    .space 0x4
enddlabel D_801027BC

/* 801027C0 */
dlabel D_801027C0
    .space 0x14
enddlabel D_801027C0

/* 801027D4 */
dlabel D_801027D4
    .space 0x4
enddlabel D_801027D4

/* 801027D8 */
dlabel D_801027D8
    .space 0xC
enddlabel D_801027D8

/* 801027E4 */
dlabel _snd_openflag
    .space 0x4
enddlabel _snd_openflag

/* 801027E8 */
dlabel D_801027E8
    .space 0x8
enddlabel D_801027E8

/* 801027F0 */
dlabel _svm_cur
    .space 0x1
enddlabel _svm_cur

/* 801027F1 */
dlabel _svm_cur_plus_0x1
    .space 0x1
enddlabel _svm_cur_plus_0x1

/* 801027F2 */
dlabel _svm_cur_plus_0x2
    .space 0x1
enddlabel _svm_cur_plus_0x2

/* 801027F3 */
dlabel _svm_cur_plus_0x3
    .space 0x1
enddlabel _svm_cur_plus_0x3

/* 801027F4 */
dlabel _svm_cur_plus_0x4
    .space 0x1
enddlabel _svm_cur_plus_0x4

/* 801027F5 */
dlabel _svm_cur_plus_0x5
    .space 0x1
enddlabel _svm_cur_plus_0x5

/* 801027F6 */
dlabel _svm_cur_plus_0x6
    .space 0x1
enddlabel _svm_cur_plus_0x6

/* 801027F7 */
dlabel _svm_cur_plus_0x7
    .space 0x3
enddlabel _svm_cur_plus_0x7

/* 801027FA */
dlabel _svm_cur_plus_0xA
    .space 0x1
enddlabel _svm_cur_plus_0xA

/* 801027FB */
dlabel _svm_cur_plus_0xB
    .space 0x1
enddlabel _svm_cur_plus_0xB

/* 801027FC */
dlabel _svm_cur_plus_0xC
    .space 0x1
enddlabel _svm_cur_plus_0xC

/* 801027FD */
dlabel _svm_cur_plus_0xD
    .space 0x1
enddlabel _svm_cur_plus_0xD

/* 801027FE */
dlabel _svm_cur_plus_0xE
    .space 0x1
enddlabel _svm_cur_plus_0xE

/* 801027FF */
dlabel _svm_cur_plus_0xF
    .space 0x1
enddlabel _svm_cur_plus_0xF

/* 80102800 */
dlabel _svm_cur_plus_0x10
    .space 0x1
enddlabel _svm_cur_plus_0x10

/* 80102801 */
dlabel _svm_cur_plus_0x11
    .space 0x1
enddlabel _svm_cur_plus_0x11

/* 80102802 */
dlabel _svm_cur_plus_0x12
    .space 0x1
enddlabel _svm_cur_plus_0x12

/* 80102803 */
dlabel _svm_cur_plus_0x13
    .space 0x1
enddlabel _svm_cur_plus_0x13

/* 80102804 */
dlabel _svm_cur_plus_0x14
    .space 0x2
enddlabel _svm_cur_plus_0x14

/* 80102806 */
dlabel _svm_cur_plus_0x16
    .space 0x2
enddlabel _svm_cur_plus_0x16

/* 80102808 */
dlabel _svm_cur_plus_0x18
    .space 0x2
enddlabel _svm_cur_plus_0x18

/* 8010280A */
dlabel _svm_cur_plus_0x1A
    .space 0x2
enddlabel _svm_cur_plus_0x1A

/* 8010280C */
dlabel _svm_cur_plus_0x1C
    .space 0x2
enddlabel _svm_cur_plus_0x1C

/* 8010280E */
dlabel _svm_cur_plus_0x1E
    .space 0x2
enddlabel _svm_cur_plus_0x1E

/* 80102810 */
dlabel g_memcard_file_list
    .space 0x258
enddlabel g_memcard_file_list

/* 80102A68 */
dlabel _svm_vab_used
    .space 0x10
enddlabel _svm_vab_used

/* 80102A78 */
dlabel _svm_sreg_buf
    .space 0x2
enddlabel _svm_sreg_buf

/* 80102A7A */
dlabel _svm_sreg_buf_plus_0x2
    .space 0x2
enddlabel _svm_sreg_buf_plus_0x2

/* 80102A7C */
dlabel D_80102A7C
    .space 0x2
enddlabel D_80102A7C

/* 80102A7E */
dlabel _svm_sreg_buf_plus_0x6
    .space 0x2
enddlabel _svm_sreg_buf_plus_0x6

/* 80102A80 */
dlabel _svm_sreg_buf_plus_0x8
    .space 0x2
enddlabel _svm_sreg_buf_plus_0x8

/* 80102A82 */
dlabel _svm_sreg_buf_plus_0xA
    .space 0x176
enddlabel _svm_sreg_buf_plus_0xA

/* 80102BF8 */
dlabel D_80102BF8
    .space 0x8
enddlabel D_80102BF8

/* 80102C00 */
dlabel g_draw_queue
    .space 0xA00
enddlabel g_draw_queue

/* 80103600 */
dlabel _svm_auto_kof_mode
    .space 0x4
enddlabel _svm_auto_kof_mode

/* 80103604 */
dlabel D_80103604
    .space 0x4
enddlabel D_80103604

/* 80103608 */
dlabel D_80103608
    .space 0x1C
enddlabel D_80103608

/* 80103624 */
dlabel D_80103624
    .space 0x34
enddlabel D_80103624

/* 80103658 */
dlabel D_80103658
    .space 0x26
enddlabel D_80103658

/* 8010367E */
dlabel D_8010367E
    .space 0x2
enddlabel D_8010367E

/* 80103680 */
dlabel _que
    .space 0x1800
enddlabel _que

/* 80104E80 */
dlabel VBLANK_MINUS
    .space 0x8
enddlabel VBLANK_MINUS

/* 80104E88 */
dlabel D_80104E88
    .space 0xB0
enddlabel D_80104E88

/* 80104F38 */
dlabel D_80104F38
    .space 0x1B18
enddlabel D_80104F38

/* 80106A50 */
dlabel D_80106A50
    .space 0x28
enddlabel D_80106A50

/* 80106A78 */
dlabel g_object_pool
    .space 0x4B0
enddlabel g_object_pool

/* 80106F28 */
dlabel _ss_score
    .space 0x80
enddlabel _ss_score

/* 80106FA8 */
dlabel _SsMarkCallback
    .space 0x800
enddlabel _SsMarkCallback

/* 801077A8 */
dlabel _snd_seq_s_max
    .space 0x2
enddlabel _snd_seq_s_max

/* 801077AA */
dlabel _snd_seq_t_max
    .space 0x6
enddlabel _snd_seq_t_max

/* 801077B0 */
dlabel D_801077B0
    .space 0x18
enddlabel D_801077B0

/* 801077C8 */
dlabel _svm_vab_total
    .space 0x40
enddlabel _svm_vab_total

/* 80107808 */
dlabel _svm_vab_count
    .space 0x8
enddlabel _svm_vab_count

/* 80107810 */
dlabel _svm_vab_start
    .space 0x40
enddlabel _svm_vab_start

/* 80107850 */
dlabel g_snd_sequence_positions
    .space 0x48
enddlabel g_snd_sequence_positions

/* 80107898 */
dlabel _svm_envx_hist
    .space 0x40
enddlabel _svm_envx_hist

/* 801078D8 */
dlabel _svm_okof1
    .space 0x2
enddlabel _svm_okof1

/* 801078DA */
dlabel _svm_okof2
    .space 0x6
enddlabel _svm_okof2

/* 801078E0 */
dlabel D_801078E0
enddlabel D_801078E0
