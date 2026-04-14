#include "common.h"

extern s32 D_800A31E8;
extern s32 D_800A31EC;
extern s32 D_800A3890;
extern s32 D_800A3924;
extern s32 D_800A37F4;

extern void func_8007A2F8(s32);
extern void func_8007A308(s32);
extern void func_8007A318(s32);
extern s32  func_80037804(void);
extern void func_800379D8(void);
extern s32  func_80037964(void);
extern void func_8003791C(void);

s32 func_80037D14(s32 arg0, s32 arg1) {
    s32 p = (arg0 << 4) + arg1;
    s32 v1;
    s32 v0;

    switch (D_800A31EC) {
    case 0:
        func_8007A2F8(p);
        D_800A31EC = 1;
        D_800A3924 = 0;
        D_800A3890 = 0;
        break;
    case 1: {
        v1 = func_80037804();
        if (v1 == 0) break;
        if (v1 == 2) goto c1_e20;
        if (v1 < 3) {
            if (v1 == 1) goto c1_dc4;
            v0 = -3; goto c1_e24;
        }
        if (v1 == 3) goto c1_dec;
        if (v1 == 4) goto c1_df4;
        v0 = -3; goto c1_e24;
    c1_dc4:
        v0 = D_800A31E8;
        D_800A37F4 = v1;
        if (v0 == 0) goto c1_de0;
        D_800A31EC = 4;
        break;
    c1_de0:
        D_800A31EC = 2;
        break;
    c1_dec:
        v0 = -1;
        goto c1_e24;
    c1_df4:
        D_800A37F4 = 2;
        func_800379D8();
        func_8007A318(p);
        func_80037964();
        D_800A31EC = 2;
        D_800A31E8 = 0;
        break;
    c1_e20:
        v0 = -3;
    c1_e24:
        D_800A37F4 = v0;
        D_800A31EC = 4;
        D_800A31E8 = 0;
        break;
    }
    case 2:
        func_8003791C();
        func_8007A308(p);
        D_800A31EC = 3;
        D_800A3924 = 0;
        break;
    case 3: {
        v1 = func_80037804();
        if (v1 == 0) break;
        D_800A31EC = 4;
        if (v1 == 2) goto c3_ecc;
        if (v1 < 3) {
            if (v1 == 1) goto c3_eb8;
            v0 = -3; goto c3_ed0;
        }
        if (v1 == 3) goto c3_ec4;
        if (v1 == 4) goto c3_ea8;
        v0 = -3; goto c3_ed0;
    c3_eb8:
        D_800A31E8 = v1;
        break;
    c3_ec4:
        v0 = -1;
        goto c3_ed0;
    c3_ea8:
        v0 = -2;
        goto c3_ed0;
    c3_ecc:
        v0 = -3;
    c3_ed0:
        D_800A37F4 = v0;
        D_800A31E8 = 0;
        break;
    }
    case 4:
        D_800A3890 = D_800A37F4;
        D_800A31EC = 0;
        break;
    }
    return D_800A3890;
}
