CdlATV g_cd_atv; /* probe: tentative definition */
void cdrom_SetMix(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    g_cd_atv.val0 = (u8)arg0;
    g_cd_atv.val1 = (u8)arg1;
    g_cd_atv.val2 = (u8)arg2;
    g_cd_atv.val3 = (u8)arg3;
    CdMix(&g_cd_atv);
    D_800A3854 = 0;
}
