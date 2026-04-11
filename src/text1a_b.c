#include "common.h"

extern void func_80044100(s32, ...);
extern void func_80044C70(s32);
extern s32 D_800A33B0;
extern s32 D_800A33B4;
extern u16 D_80099478;
extern void func_8005441C(s32);
extern void func_80047ED0(s32);
void func_800466C0(s32 a0, s32 a1) {
    s32 rounded;
    s16 val;
    func_80044100(7, a1);
    func_8005441C(a1);
    rounded = (a1 / 4) * 4;
    D_800A33B0 += rounded;
    val = (s16)(D_80099478 - 4);
    D_800A33B4 += rounded;
    switch (val) {
    case 0:
    case 3:
    case 14:
        func_80044C70(a1);
        break;
    case 9:
        ((void (*)(s32))func_80044100)(8);
        break;
    case 7:
        func_80047ED0(a1);
        break;
    }
}
