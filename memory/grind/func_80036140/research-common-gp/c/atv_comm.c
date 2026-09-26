typedef struct { unsigned char val0, val1, val2, val3; } CdlATV;
extern int CdMix(CdlATV *);
CdlATV g_cd_atv;
CdlATV D_800A36B8;
short D_800A3854;
void cdrom_SetMix(int arg0, int arg1, int arg2, int arg3) {
    g_cd_atv.val0 = arg0;
    g_cd_atv.val1 = arg1;
    g_cd_atv.val2 = arg2;
    g_cd_atv.val3 = arg3;
    CdMix(&g_cd_atv);
    D_800A3854 = 0;
}
void copy(void) { g_cd_atv = D_800A36B8; }
