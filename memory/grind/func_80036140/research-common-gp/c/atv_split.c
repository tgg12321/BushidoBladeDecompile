extern int CdMix(void *);
unsigned char g_cd_atv, g_cd_atv_1, g_cd_atv_2, g_cd_atv_3;
short D_800A3854;
void cdrom_SetMix(int arg0, int arg1, int arg2, int arg3) {
    g_cd_atv = arg0; g_cd_atv_1 = arg1; g_cd_atv_2 = arg2; g_cd_atv_3 = arg3;
    CdMix(&g_cd_atv);
    D_800A3854 = 0;
}
