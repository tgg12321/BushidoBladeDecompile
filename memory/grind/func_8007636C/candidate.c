#include "m2c_macros.h"

typedef struct {
    s32 sp18;
    s32 sp1C;
    s32 sp20;
    s32 sp24;
    s32 sp28;
    s32 sp2C;
    s32 sp30;
    s32 sp34;
    s32 sp38;
    s32 sp3C;
    s8 sp40;
    s8 sp41;
    s8 sp42;
    s8 sp43;
} S_8007636C;

void func_8007636C(s32 arg0, s32 arg1, s16 *arg2, s32 arg3) {
    S_8007636C s;
    s16 temp_v1_2;
    s16 temp_v1_4;
    s16 temp_v1_5;
    s16 var_s2;
    s16 var_s2_2;
    s16 var_s2_3;
    s32 temp_a0_3;
    s32 temp_a0_4;
    s32 temp_s0;
    s32 temp_s4_3;
    s32 temp_s5;
    s32 temp_v0_4;
    s32 temp_v1_3;
    s32 var_s6;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v1;
    s32 var_v1_2;
    s32 var_v1_3;
    s8 temp_v0;
    u16 var_s0;
    u8 temp_v0_3;
    void *temp_a0;
    void *temp_a0_2;
    void *temp_a0_5;
    void *temp_a0_6;
    void *temp_a0_7;
    void *temp_a1;
    void *temp_a1_2;
    void *temp_a1_3;
    void *temp_a1_4;
    void *temp_a1_5;
    void *temp_a2;
    void *temp_a2_2;
    void *temp_a2_3;
    void *temp_a2_4;
    void *temp_a2_5;
    void *temp_s4;
    void *temp_s4_2;
    void *temp_v0_2;
    void *temp_v1;
    void *var_a0;
    void *var_v0;

    var_s6 = 0xA;
    s.sp28 = 0;
    if (arg3 != 0) {
        var_s6 = 0x14;
    }
    temp_s0 = arg3 * 2;
    temp_a0 = temp_s0 + D_800A36A0;
    if (M2C_FIELD(temp_a0, s16 *, 0x14) < 4) {
        temp_v1 = M2C_FIELD(M2C_FIELD(M2C_FIELD(arg0, void **, 0), void **, 0x30), void **, 0x30);
        s.sp40 = 0;
        s.sp30 = arg3 * 0xF0;
        temp_a2 = temp_v1 + 0xC;
        s.sp18 = temp_v1;
        s.sp1C = temp_a2;
        s.sp2C = var_s6;
        s.sp34 = M2C_FIELD(temp_a0, s16 *, 0x3C) * 0x22;
        s.sp20 = M2C_FIELD(arg0, s32 *, 0x10);
        M2C_FIELD(arg0, s32 *, 0x10) = func_8007352C((s32)&s);
        SetDrawMode(M2C_FIELD(arg0, s32 *, 0x18), 1, 0, func_8006E480(s.sp18, 0), 0);
        AddPrim(g_gpu_ot_ptr + (var_s6 * 4), M2C_FIELD(arg0, s32 *, 0x18));
        M2C_FIELD(arg0, s32 *, 0x18) = (s32) (M2C_FIELD(arg0, s32 *, 0x18) + 0xC);
    }
    s.sp40 = 0;
    var_s2 = 0;
    temp_v0 = ((s32) (rsin(((M2C_FIELD(D_800A36A0, u16 *, 0x34) & 0x1F) << 7) + 0x1FF) << 5) >> 0xC) - 0x50;
    temp_a1 = D_800A36A0;
    s.sp43 = temp_v0;
    s.sp42 = temp_v0;
    s.sp41 = temp_v0;
    temp_s4 = M2C_FIELD(M2C_FIELD(arg0, void **, 0), void **, 0x14);
    if ((M2C_FIELD(temp_a1, u8 *, 0x65) + 3) > 0) {
        var_v1 = 0 << 0x10;
        do {
            temp_v1_3 = var_v1 >> 0x10;
            s.sp40 = 0;
            temp_a0_2 = M2C_FIELD(((*((temp_v1_3 * 2) + arg2) * 4) + temp_s4), void **, 4);
            temp_a1_2 = temp_s0 + temp_a1;
            s.sp18 = temp_a0_2;
            temp_a2_2 = temp_a0_2 + 0x24;
            if ((M2C_FIELD(temp_a1_2, s16 *, 0x3C) == temp_v1_3) || (var_v1_2 = var_s2 << 0x10, ((M2C_FIELD(temp_a1_2, s16 *, 0x14) < 4) == 0))) {
                s.sp18 = temp_a0_2 + ((arg3 * 0xC) + 0xC);
                var_v1_2 = var_s2 << 0x10;
            }
            s.sp1C = temp_a2_2;
            s.sp30 = arg3 * 0xF0;
            s.sp34 = (var_v1_2 >> 0x10) * 0x22;
            s.sp2C = var_s6;
            s.sp1C = temp_a2_2 + (M2C_FIELD(s.sp18, u8 *, 2) * 0x10);
            s.sp20 = M2C_FIELD(arg0, s32 *, 0x10);
            temp_v1_2 = var_s2 + 1;
            var_s2 = temp_v1_2;
            M2C_FIELD(arg0, s32 *, 0x10) = func_8007352C((s32)&s);
            var_v1 = var_s2 << 0x10;
        } while (temp_v1_2 < (M2C_FIELD(D_800A36A0, u8 *, 0x65) + 3));
    }
    temp_a0_3 = arg3 * 2;
    temp_a2_3 = D_800A36A0;
    temp_s4_2 = M2C_FIELD(M2C_FIELD(arg0, void **, 0), void **, 0x30);
    var_s2_2 = 0;
    if ((M2C_FIELD((temp_a0_3 + temp_a2_3), s16 *, 0x3C) + 1) > 0) {
        temp_s5 = arg3 * 0xA;
        var_a0 = temp_a0_3 + temp_a2_3;
        do {
            var_v0 = temp_s5 + temp_a2_3;
            if (M2C_FIELD(var_a0, s16 *, 0x3C) == var_s2_2) {
                if (M2C_FIELD(var_a0, s16 *, 0x14) >= 4) {
                    var_v0 = temp_s5 + temp_a2_3;
                    goto block_15;
                }
                var_s0 = M2C_FIELD((temp_s5 + temp_a2_3 + (M2C_FIELD(var_a0, s16 *, 0x5C) * 2)), u16 *, 0x48);
                s.sp40 = 1;
            } else {
block_15:
                var_s0 = M2C_FIELD((var_v0 + (var_s2_2 * 2)), u16 *, 0x7E);
                s.sp40 = 0;
            }
            temp_a1_3 = *(void **)(((s32) (var_s0 << 0x10) >> 0xD) + temp_s4_2);
            temp_a0_5 = temp_a0_3 + D_800A36A0;
            s.sp18 = temp_a1_3;
            temp_a2_4 = temp_a1_3 + 0x24;
            if ((M2C_FIELD(temp_a0_5, s16 *, 0x3C) == var_s2_2) || (var_v0_2 = arg3 * 0x10, ((M2C_FIELD(temp_a0_5, s16 *, 0x14) < 4) == 0))) {
                s.sp18 = temp_a1_3 + ((arg3 * 0xC) + 0xC);
                var_v0_2 = arg3 * 0x10;
            }
            s.sp30 = (var_v0_2 - arg3) * 0x10;
            s.sp1C = temp_a2_4;
            s.sp34 = var_s2_2 * 0x22;
            s.sp2C = var_s6;
            s.sp20 = M2C_FIELD(arg0, s32 *, 0x10);
            M2C_FIELD(arg0, s32 *, 0x10) = func_8007352C((s32)&s);
            temp_v0_2 = M2C_FIELD((((s32) (var_s0 << 0x10) >> 0xD) + temp_s4_2), void **, 4);
            s.sp40 = 0;
            s.sp18 = temp_v0_2;
            s.sp1C = temp_v0_2 + 0xC;
            s.sp20 = M2C_FIELD(arg0, s32 *, 0x10);
            temp_v1_4 = var_s2_2 + 1;
            var_s2_2 = temp_v1_4;
            temp_a0_4 = arg3 * 2;
            M2C_FIELD(arg0, s32 *, 0x10) = func_8007352C(&s);
            var_a0 = temp_a0_4 + D_800A36A0;
        } while (temp_v1_4 < (M2C_FIELD((temp_a0_4 + D_800A36A0), s16 *, 0x3C) + 1));
    }
    temp_a1_4 = D_800A36A0;
    temp_v0_3 = M2C_FIELD(temp_a1_4, u8 *, 0x65);
    temp_s4_3 = M2C_FIELD(((temp_v0_3 * 4) + M2C_FIELD(arg0, void **, 0)), s32 *, 0x20);
    var_s2_3 = 0;
    if ((temp_v0_3 + 3) != 0) {
        var_v0_3 = 0 << 0x10;
        do {
            temp_v0_4 = var_v0_3 >> 0x10;
            s.sp40 = 0;
            temp_a0_6 = *(void **)((temp_v0_4 * 4) + temp_s4_3);
            temp_a1_5 = (arg3 * 2) + temp_a1_4;
            s.sp18 = temp_a0_6;
            temp_a2_5 = temp_a0_6 + 0x24;
            if ((M2C_FIELD(temp_a1_5, s16 *, 0x3C) == temp_v0_4) || (var_v1_3 = var_s2_3 << 0x10, ((M2C_FIELD(temp_a1_5, s16 *, 0x14) < 4) == 0))) {
                s.sp18 = temp_a0_6 + ((arg3 * 0xC) + 0xC);
                var_v1_3 = var_s2_3 << 0x10;
            }
            s.sp1C = temp_a2_5;
            s.sp30 = arg3 * 0xF0;
            s.sp34 = (var_v1_3 >> 0x10) * 0x22;
            s.sp1C = temp_a2_5 + (M2C_FIELD(s.sp18, u8 *, 2) * 8);
            s.sp20 = M2C_FIELD(arg0, s32 *, 0x10);
            temp_v1_5 = var_s2_3 + 1;
            var_s2_3 = temp_v1_5;
            M2C_FIELD(arg0, s32 *, 0x10) = func_8007352C((s32)&s);
            var_v0_3 = var_s2_3 << 0x10;
        } while (temp_v1_5 < (M2C_FIELD(D_800A36A0, u8 *, 0x65) + 3));
    }
    temp_a0_7 = M2C_FIELD(M2C_FIELD(M2C_FIELD(arg0, void **, 0), void **, 0x14), void **, 4);
    s.sp18 = temp_a0_7;
    SetDrawMode(M2C_FIELD(arg0, s32 *, 0x18), 1, 0, func_8006E480(temp_a0_7, 0), 0);
    AddPrim(g_gpu_ot_ptr + (var_s6 * 4), M2C_FIELD(arg0, s32 *, 0x18));
    M2C_FIELD(arg0, s32 *, 0x18) = (s32) (M2C_FIELD(arg0, s32 *, 0x18) + 0xC);
}
