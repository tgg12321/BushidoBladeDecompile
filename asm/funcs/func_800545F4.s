glabel func_800545F4
    /* 44DF4 800545F4 804C083C */  lui        $t0, (0x4C808080 >> 16)
  alabel g_color_immediate_patch_first_ori_template
    /* 44DF8 800545F8 80800835 */  ori        $t0, $t0, (0x4C808080 & 0xFFFF)
  alabel g_color_immediate_patch_second_lui_template
    /* 44DFC 800545FC 8048083C */  lui        $t0, (0x48808080 >> 16)
  alabel g_color_immediate_patch_second_ori_template
    /* 44E00 80054600 80800835 */  ori        $t0, $t0, (0x48808080 & 0xFFFF)
endlabel func_800545F4
