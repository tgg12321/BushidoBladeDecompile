#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "sound.h"
#include "game.h"
#include "code6cac.h"

extern s16 D_80099478;
extern s16 D_8009947A;
extern s32 D_800A33B0;
extern s32 D_800A33B4;
extern s32 *func_800457A0(s32);
extern s32 *func_800455AC(s32);
extern void func_80044F30(s32, s32);
extern void func_80045824(s32, s32, s32);
extern void func_80045230(s32);
extern void func_80044010(s32, s32);
extern void md_game_check_mode(s32, s32);
extern s32 func_80044670(s32, s32, s32);
extern void snd_SetVolume(s32);
extern s32 snd_GetMaxFade(void);
extern void func_800481E8(s32, s32);
extern void func_80054410(s32);
extern void func_80045600(s32, s32);
extern void func_80045694(s32, s32);
extern void stage_ExecInitFunc(void);
extern void func_8004659C(s32);
extern void func_800466C0(void);
extern void func_80045510(s32, s32);
extern void func_80044098(s32);
extern void func_800453E0(s32);

#define ALIGN4(x) (((u32)(x) >> 2) << 2)
#define PTR_OFF(base, off) ((s32)((u8 *)(base) + (off)))

void func_800460E4(s32 stage_id, s32 arg1) {
    s32 *s0;
    s32 s7;
    s32 *s6, *s4, *s2;
    s32 s3;
    s32 *s1;
    s32 *fp_ptr;
    s32 *sp10, *sp18, *sp20;

    s0 = func_800457A0(7);
    if (s0 != NULL) {
        if (D_80099478 == stage_id) {
            s7 = 1;
            switch (stage_id) {
            case 3:
                break;
            case 4:
            case 7:
            case 18:
                s3 = s0[0];
                {
                    s32 off = ALIGN4(s0[s3 - 1]);
                    md_game_check_mode(PTR_OFF(s0, off), 7);
                }
                break;
            case 34:
                s7 = 0;
                break;
            }
            stage_ExecInitFunc();
            if (s7 != 0) {
                return;
            }
        }
    }

    D_80099478 = (s16)stage_id;
    s7 = 7;
    s0 = func_800455AC(7);

    if (arg1 != 0) {
        func_80044F30(stage_id, arg1);
    } else {
        func_80044F30(stage_id, (s32)s0);
    }

    if (arg1 != 0) {
        s32 *p = (s32 *)arg1;
        s3 = p[0];
        func_80045824(arg1, (s32)s0, p[s3]);
    }

    {
        s32 off1_raw = s0[1];
        s3 = s0[0];
        s6 = (s32 *)((u8 *)s0 + ALIGN4(off1_raw));
        {
            s32 *a0_ptr = (s32 *)((u8 *)s0 + (s3 << 2));
            s4 = (s32 *)((u8 *)s0 + ALIGN4(a0_ptr[-1]));
            sp10 = (s32 *)((u8 *)s0 + ALIGN4(s0[2]));
            sp18 = (s32 *)((u8 *)s0 + ALIGN4(s0[3]));
            sp20 = (s32 *)((u8 *)s0 + ALIGN4(s0[4]));

            {
                s32 off = ALIGN4(a0_ptr[0]);
                s2 = (s32 *)((u8 *)s0 + off);
            }

            if (arg1 != 0) {
                s32 *p2 = (s32 *)((u8 *)arg1 + (s3 << 2));
                s32 off2 = ALIGN4(p2[0]);
                fp_ptr = (s32 *)((u8 *)arg1 + off2);
            } else {
                fp_ptr = s2;
                {
                    s32 off3 = ALIGN4(a0_ptr[1]);
                    func_80045230(PTR_OFF(s0, off3));
                }
            }
        }
    }

    D_8009947A = 0;
    s1 = s4;
    switch (stage_id) {
    case 3:
        s1 = s2;
        {
            s32 *ptr = (s32 *)((u8 *)s0 + (s3 << 2));
            s32 raw_m2 = ptr[-2];
            s32 raw_m1 = *(volatile s32 *)&ptr[-1];
            D_8009947A = 1;
            s6 = (s32 *)((u8 *)s0 + ALIGN4(raw_m2));
            s4 = (s32 *)((u8 *)s0 + ALIGN4(raw_m1));
        }
        break;
    case 4:
    case 7:
    case 18:
        s1 = s2;
        func_80044010(PTR_OFF(s0, ALIGN4(s0[5])), 8);
        s1 = (s32 *)func_80044670(PTR_OFF(s0, ALIGN4(s0[6])), 8, (s32)s1);
        break;
    case 11:
        snd_SetVolume((s32)s1);
        s1 = (s32 *)((s32)s1 + snd_GetMaxFade());
        break;
    case 13:
        s1 = s2;
        {
            s32 *ptr = (s32 *)((u8 *)s0 + (s3 << 2));
            s32 off1 = ALIGN4(ptr[-2]);
            s32 off2 = ALIGN4(ptr[-1]);
            s6 = (s32 *)((u8 *)s0 + off1);
            s4 = (s32 *)((u8 *)s0 + off2);
            func_80044010(PTR_OFF(s0, ALIGN4(s0[5])), 8);
            D_8009947A = 1;
        }
        break;
    case 34:
        s1 = s2;
        D_8009947A = 1;
        s4 = (s32 *)((u8 *)s0 + ALIGN4(s0[5]));
        break;
    }

    func_80044010((s32)s6, 7);
    func_800481E8((s32)fp_ptr, 0);
    md_game_check_mode((s32)s4, 7);
    func_80054410((s32)sp10);
    D_800A33B0 = (s32)sp18;
    D_800A33B4 = (s32)sp20;
    gpu_DrawSync(0);
    func_80045600(s7, (s32)s1);
    func_80045694(s7, (s32)func_800466C0);
    stage_ExecInitFunc();
    if (D_800A38DC != 0) {
        if (stage_id != 0x22) {
            func_8004659C(-1);
        }
    }
}
void func_800464C4(void) {
    s32 *s0;
    s32 *s1;
    s32 *a0;
    s32 v0;

    if (g_stage_variant == 0) {
        return;
    }
    s0 = (s32 *)func_800457A0(7);
    v0 = ((u32)s0[1] >> 2) << 2;
    a0 = (s32 *)((u8 *)s0 + v0);
    switch (g_stage_id) {
    case 0xD:
        v0 = ((u32)s0[6] >> 2) << 2;
        s1 = (s32 *)((u8 *)s0 + v0);
        break;
    case 3:
    case 0x22:
        v0 = ((u32)s0[5] >> 2) << 2;
        s1 = (s32 *)((u8 *)s0 + v0);
        break;
    }
    func_80044010(a0, 7);
    md_game_check_mode(s1, 7);
    func_80045510(7, (s32)((u8 *)s1 - (u8 *)s0));
    g_stage_variant = 0;
}
void func_8004659C(s32 a0) {
    s32 *s0;
    s32 *s2;
    s32 *s3;
    s32 *s4p;
    s32 *s1p;
    s32 *s0p;
    s32 v0;
    if (a0 < 0) {
        func_800464C4();
        return;
    }
    if (g_stage_variant == 0) {
        return;
    }
    v0 = func_800457A0(7);
    s0 = (s32 *)v0;
    v0 += 4;
    v0 = v0 + a0 * 20;
    s2 = (s32 *)((u8 *)s0 + *(s32 *)v0);
    v0 += 4;
    s3 = (s32 *)((u8 *)s0 + *(s32 *)v0);
    v0 += 4;
    s4p = (s32 *)((u8 *)s0 + *(s32 *)v0);
    v0 += 4;
    s1p = (s32 *)((u8 *)s0 + *(s32 *)v0);
    s0p = (s32 *)((u8 *)s0 + *(s32 *)(v0 + 4));
    func_80044098(7);
    func_80044010(s2, 7);
    func_80054410(s3);
    g_snd_bgm_id = (s32)s4p;
    g_snd_se_id = (s32)s1p;
    md_game_check_mode(s0p, 7);
}
extern void func_800453E0(s32);
void func_8004668C(void) {
    func_800453E0(7);
    g_stage_id = -1;
    g_stage_variant = 0;
}
