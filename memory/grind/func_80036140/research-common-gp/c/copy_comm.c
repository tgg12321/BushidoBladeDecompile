typedef struct { unsigned char val0, val1, val2, val3; } CdlATV;
extern int CdMix(CdlATV *);
CdlATV g_cd_atv;
CdlATV D_800A36B8;
short D_800A3854;
void copy(void) { g_cd_atv = D_800A36B8; }
