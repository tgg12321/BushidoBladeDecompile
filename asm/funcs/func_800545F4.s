dlabel func_800545F4
    /* 44DF4 800545F4 804C083C */ .word 0x3C084C80 /* lui $t0, 0x4C80 */
enddlabel func_800545F4
dlabel g_color_immediate_patch_first_ori_template
    /* 44DF8 800545F8 80800835 */ .word 0x35088080 /* ori $t0, $t0, 0x8080 */
enddlabel g_color_immediate_patch_first_ori_template
dlabel g_color_immediate_patch_second_lui_template
    /* 44DFC 800545FC 8048083C */ .word 0x3C084880 /* lui $t0, 0x4880 */
enddlabel g_color_immediate_patch_second_lui_template
dlabel g_color_immediate_patch_second_ori_template
    /* 44E00 80054600 80800835 */ .word 0x35088080 /* ori $t0, $t0, 0x8080 */
enddlabel g_color_immediate_patch_second_ori_template
