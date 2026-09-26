typedef struct { unsigned char val0, val1, val2, val3; } CdlATV;
extern CdlATV D_800A36B8;
short D_800A3854;
short D_800A3840;
void func_80035F78(short arg0, int arg1, int arg2, int arg3, int arg4) {
    D_800A36B8.val0 = arg1;
    D_800A36B8.val1 = arg2;
    D_800A36B8.val2 = arg3;
    D_800A3854 = arg0;
    D_800A3840 = 0;
    D_800A36B8.val3 = arg4;
}
