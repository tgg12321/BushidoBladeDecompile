/* 103 game functions, among them rcnt_StartCnt1, rcnt_GetCnt1, math_GrayscaleRgb555 and
 * math_Grayscale3. .text 0x800460E4 (ROM 0x368E4). Start boundary: GP (a per-file gp merge, Q67);
 * compiled -G8 (Q89). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "bb2.h"
#include "include_asm.h"
#include "gte.h"

/* ---- merged from text1a_c2.c (owner ruling Q67: one original file) ---- */
extern s32 *func_800457A0(s32);
extern s32 func_80044670(s32, s32, s32);
extern void func_800477DC(s32);
extern s32 func_80047EC8(void);
extern void func_800481E8(s32, s32);
extern void func_800466C0(s32, s32);


#define ALIGN4(x) (((u32)(x) >> 2) << 2)
#define PTR_OFF(base, off) ((s32)((u8 *)(base) + (off)))

/* Q65: this file's statics (.sbss, allocated per file in link order by PSYLINK), in address order. */
static s32 D_800A33B0;
static s32 D_800A33B4;
static s32 D_800A33B8;  /* not named by any code or data: size from the gap */
static s32 D_800A33BC;
static s32 D_800A33C0;
static s32 D_800A33C4;  /* not named by any code or data: size from the gap */
static s16 D_800A33C8[2];
static s32 D_800A33CC;  /* not named by any code or data: size from the gap */
static s16 * D_800A33D0;
static s32 D_800A33D4;
static s32 D_800A33D8;
static s32 D_800A33DC;  /* not named by any code or data: size from the gap */
static s32 D_800A33E0;
static s32 D_800A33E4;
static s16 D_800A33E8[2];
static s32 D_800A33EC;

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
        if (g_stage_id == stage_id) {
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
                    func_8003EDC0((u16 *)PTR_OFF(s0, off), 7);
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

    g_stage_id = (s16)stage_id;
    /* FAKE: s7 (the early-return flag above) reused as slot id 7 for func_80045600 /
     * func_80045694; the literal or a separate local scores 37. */
    s7 = 7;
    s0 = func_800455AC(7);

    if (arg1 != 0) {
        func_80044F30(stage_id, arg1);
    } else {
        func_80044F30(stage_id, (s32)s0);
    }

    if (arg1 != 0) {
        /* FAKE: s3 (the slot buffer's word count) reused for arg1's word count; a separate
         * local or the inline read scores 2. */
        s3 = *(s32 *)arg1;
        func_80045824(arg1, (s32)s0, ((s32 *)arg1)[s3]);
    }

    {
        s32 off1_raw = s0[1];
        s3 = s0[0];
        s6 = (s32 *)((u8 *)s0 + ALIGN4(off1_raw));
        {
            s32 *a0_ptr = (s32 *)((s3 << 2) + (s32)s0);
            s4 = (s32 *)((u8 *)s0 + ALIGN4(a0_ptr[-1]));
            sp10 = (s32 *)((u8 *)s0 + ALIGN4(s0[2]));
            sp18 = (s32 *)((u8 *)s0 + ALIGN4(s0[3]));
            sp20 = (s32 *)((u8 *)s0 + ALIGN4(s0[4]));

            {
                s32 off = ALIGN4(a0_ptr[0]);
                s2 = (s32 *)((u8 *)s0 + off);
            }

            if (arg1 != 0) {
                fp_ptr = (s32 *)((u8 *)arg1 + ALIGN4(((s32 *)arg1)[s3]));
            } else {
                fp_ptr = s2;
                {
                    s32 off3 = ALIGN4(a0_ptr[1]);
                    func_80045230(PTR_OFF(s0, off3));
                }
            }
        }
    }

    g_stage_variant = 0;
    /* FAKE: live default init of s1 routed through a delta-rebase detour that
       combine folds back to s1 = s4 with zero emitted bytes -- flow.c
       reg_n_refs (+2 on s1's pseudo) lifts its global.c allocno_compare
       priority above the s2 pointer so allocation order matches target. */
    s1 = (s32 *)((s32)s4 - (s32)s0);
    s1 = (s32 *)((s32)s1 + (s32)s0);
    switch (stage_id) {
    case 3: {
        /* FAKE: fresh once-written/once-read pointer intermediate naming the
         * address of the stage header's last word -- expand-time
         * MEM_IN_STRUCT_P (expr.c:4567-4577) -> sched.c anti_dependence
         * exemption -> sched1 load/store order. */
        s32 *hp = (s32 *)((s3 << 2) + (s32)s0) - 1;
        s1 = s2;
        s6 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 2]));
        s4 = (s32 *)((u8 *)s0 + ALIGN4(*hp));
        g_stage_variant = 1;
        break;
    }
    case 4:
    case 7:
    case 18:
        s1 = s2;
        func_80044010((s32 *)PTR_OFF(s0, ALIGN4(s0[5])), 8);
        s1 = (s32 *)func_80044670(PTR_OFF(s0, ALIGN4(s0[6])), 8, (s32)s1);
        break;
    case 11:
        func_800477DC((s32)s1);
        s1 = (s32 *)((s32)s1 + func_80047EC8());
        break;
    case 13:
        s1 = s2;
        s6 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 2]));
        s4 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 1]));
        func_80044010((s32 *)PTR_OFF(s0, ALIGN4(s0[5])), 8);
        g_stage_variant = 1;
        break;
    case 34:
        s1 = s2;
        g_stage_variant = 1;
        s4 = (s32 *)((u8 *)s0 + ALIGN4(s0[5]));
        break;
    }

    func_80044010(s6, 7);
    func_800481E8((s32)fp_ptr, 0);
    func_8003EDC0((u16 *)s4, 7);
    func_80054410((s32)sp10);
    D_800A33B0 = (s32)sp18;
    D_800A33B4 = (s32)sp20;
    DrawSync(0);
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
    func_8003EDC0((u16 *)s1, 7);
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
    D_800A33B0 = (s32)s4p;
    D_800A33B4 = (s32)s1p;
    func_8003EDC0((u16 *)s0p, 7);
}
void func_8004668C(void) {
    func_800453E0(7);
    g_stage_id = -1;
    g_stage_variant = 0;
}

/* ---- merged from text1a_b.c (owner ruling Q67: one original file) ---- */
extern void func_80044100();
extern void func_80047ED0(s32);
void func_800466C0(s32 a0, s32 a1) {
    s32 rounded;
    func_80044100(7, a1);
    func_8005441C(a1);
    rounded = (a1 / 4) * 4;
    D_800A33B0 += rounded;
    D_800A33B4 += rounded;
    switch (g_stage_id) {
    case 4:
    case 7:
    case 18:
        func_80044C70(a1);
        break;
    case 13:
        func_80044100(8);
        break;
    case 11:
        func_80047ED0(a1);
        break;
    }
}

/* 0x800153F0: the 22-halfword record func_8004A09C unpacks (it walks it as u16). func_80049F4C copies
   it whole by assignment: the copy's run-time alignment test in the target bytes is the halfword
   type's alignment. */
typedef struct {
    u16 v[22];
} Unk800153F0Record;

/* ---- merged from sound.c (owner ruling Q67: one original file) ---- */
/* Forward declarations for called functions */
extern void func_80048F58(s32, s32);
extern void func_80048FFC(s32);
extern s32 *func_8004153C();
extern s32 func_800477E8(void);
extern void func_80047A90(void);
extern void func_80048B8C(s32);
extern void func_800460E4(s32, s32);
extern void func_8004668C(void);

extern void math_RotMatrixYXZ(SVECTOR *, MATRIX *);
extern s16 Judge[];
extern Unk80101DF0Record *D_800A3708;
extern void gte_MulMatrix0ClearTrans(MATRIX *, MATRIX *, MATRIX *);
extern s16 D_800A324A;

/* Externs for globals */
extern MATRIX D_800EEDB0;

extern s32 D_800EF800[];
extern u8 g_stage_data;
extern s16 D_800F6654;
extern u8 g_cam_bone_data;


extern u8 *g_gpu_ot_ptr;
extern void func_80049E4C(void);
extern void func_80049F4C(void);
extern s16 g_color_mode;
extern s16 D_800F665A;
extern void func_80044F80(s32, s32 *);
extern s16 D_800A3248;

void func_800468DC(s32 a0, s32 a1);

/* --- Functions 0x80046780 - 0x80047EC8 --- */

s32 func_80046780(void) {
    return D_800A33B0;
}
s32 func_8004678C(void) {
    return D_800A33B4;
}

s32 stage_GetId(void) {
    return g_stage_id;
}

s32 stage_GetVariant(void) {
    return g_stage_variant;
}

s32 *func_800467B8(s32 a0) {
    s32 arg = a0;
    s32 chan = 8;
    s32 *s2;

    if (func_800486FC()) {
        arg = arg + 0x1B;
    }
    {
        s32 *a1 = func_8004574C(8);
        if (a1 && D_800A3248 == arg) {
            s2 = (s32 *)a1[1];
        } else {
            s32 *s1;
            s32 *s0;
            s2 = func_800455AC(chan);
            func_80044F80(arg, s2);
            {
                s32 off1 = (u32)s2[2] >> 2 << 2;
                s32 off0 = (u32)s2[1] >> 2 << 2;
                s1 = (s32 *)((u8 *)s2 + off1);
                s0 = (s32 *)((u8 *)s2 + off0);
            }
            func_80045230((s32)s1);
            func_80044010(s0, 9);
            func_80045600(chan, (s32)s1);
            D_800A3248 = arg;
        }
    }
    func_80045694(chan, (s32)func_800468DC);
    return s2;
}

void func_800468B0(s32 a0) {
    func_80045510(8, a0);
    func_80045230(0);
}

void func_800468DC(s32 a0, s32 a1) {
    func_80048B8C(a1);
    func_80044100(9, a1);
}

void func_80046914(void) {
    func_800453E0(8);
}

void snd_AllocSe(void) {
    func_800455AC(9);
}

void snd_SeNullCallback(void) {
}

void func_8004695C(s32 a0) {
    func_80045230(a0);
    func_80045600(9, a0);
    func_80045694(9, (s32)snd_SeNullCallback);
}

void func_800469A0(s32 a0) {
    func_80045510(9, a0);
}

void func_80046A80(s32, s32);

s32 *func_800469C4(s32 a0) {
    s32 *v0;
    s32 offset;

    v0 = func_800455AC(0xA);
    func_80044FA0(a0, (s32)v0);
    offset = (u32)v0[v0[0] + 1] >> 2 << 2;
    {
        s32 *s0 = (s32 *)((u8 *)v0 + offset);
        func_80045230((s32)s0);
        func_80045600(0xA, (s32)s0);
    }
    D_800A324A = (s16)a0;
    func_80045694(0xA, (s32)func_80046A80);
    return v0;
}

void func_80046A60(void) {
    func_800453E0(0xA);
}

void func_80046A80(s32 a0, s32 a1) {
    func_80054FDC(a1);
}

void snd_StopAll(void) {
    player_Destroy(0);
    player_Destroy(1);
    func_8004668C();
    func_80046020();
    func_80049E1C();
    func_80046914();
}

void rcnt_StartCnt1(void) {
    SetRCnt(0xF2000001, -1, 0x2000);
    StartRCnt(0xF2000001);
}

void rcnt_GetCnt1(void) {
    GetRCnt(0xF2000001);
}

void game_Init(void) {
    /* FAKE: constant-holder locals — set in source order ahead of the fence
       below so 1 seats in $v0 and 2 in $v1 before the store tail, with $v0
       freed for reuse by 0x23 mid-tail */
    s16 one;
    s16 two;

    func_800451A0();
    func_800451D0();
    func_80042E90();
    func_80044498();
    func_80049E4C();
    func_80049F4C();
    func_8003D91C();
    func_800404D8();
    func_8003F7F4();
    one = 1;
    two = 2;
    /* FAKE: sched.c mid-block loop-note fence — keeps the two pre-fence
       constant sets from being folded into the store tail (measured: without
       it CSE/sched collapse 1/2/0x23 into a single serialized $v0) */
    do { } while (0);
    D_800F6654 = one;
    D_800F665A = one;
    g_color_mode = 0;
    D_800F6650 = 0;
    D_800F6656 = 0;
    D_800F6658 = two;
    g_game_mirror_mode = 0;
    D_800A3790 = 0x23;
    D_800A33BC = 0;
}
void func_80046BF4(s32 *a0, u16 *a1, s32 a2) {
    s32 result[3];
    u16 new_var2;
    s32 trans[3];
    SVECTOR rot;
    MATRIX matrix_buf;

    D_800A3820 = (s32)&D_80102C00;
    {
        u16 cnt = D_800A38D6;
        s32 old_ptr = (s32)g_gpu_ot_ptr;
        new_var2 = cnt + 1;
        D_800A3808 = old_ptr;
        D_800A38D6 = new_var2;
        D_800A378C = (u32 *)(old_ptr + 0x10);
    }

    if (a0 != 0) {
        D_80101DF0.xf.rot.vx = -(s16)a1[0];
        D_80101DF0.xf.rot.vy = -(s16)a1[1];
        D_80101DF0.xf.rot.vz = -(s16)a1[2];

        trans[1] = (trans[0] = 0);
        trans[2] = -a2;

        rot.vx = -(s16)a1[0];
        rot.vy = -(s16)a1[1];
        rot.vz = -(s16)a1[2];

        g_anim_func_table[0](&rot, &matrix_buf);

        ApplyMatrixLV(&matrix_buf, trans, result);

        {
            s32 *rp = result;
            s32 *ap = a0;
            D_80101DF0.work.t[0] = *rp++ + *ap++;
            D_80101DF0.work.t[1] = *rp++ + *ap++;
            D_80101DF0.work.t[2] = *rp++ + *ap++;
        }

        func_800418D0((s32 *)&D_80101DF0);
        camera_InitBoneData();
        stage_InitCollision();

        D_800A33C0 = a2;
    }

    func_8004A1FC(&D_800F62E0[0]);
    func_8004A1FC(&D_800F62E0[1]);
    func_8004A1FC(&D_800F62E0[4]);
    func_800420D0();
    stage_ClearLighting();
    stage_ApplyLighting();
}
void func_80046DA8(s32 a0) {
    if (a0 & 1) {
        func_80046EA0(D_800A33C0);
    }
    func_8004211C();
    func_800444BC();
}

s32 game_GetDummyFlag(void) {
    return 0;
}

void *game_GetPlayerData(s32 a0) {
    void *v0 = func_8004153C(a0);
    if (v0) {
        return (u8 *)v0 + 0x1994;
    }
    return NULL;
}

void *game_GetPlayerBase(void) {
    void *v0 = func_8004153C();
    if (v0) {
        return (u8 *)v0 + 0x2C;
    }
    return NULL;
}

void func_80046E44(void) {
    D_800F6654 = 0;
}

void func_80046E54(s32 a0) {
    if (a0) {
        D_800F6654 = 1;
    } else {
        D_800F6654 = 0;
    }
}

s32 func_80046E7C(void) {
    return D_800F6654;
}

void func_80046E8C(void) {
    D_800A3790 = 0x23;
}

void func_80046EA0(s32 a0) {
    func_8003E6D8(a0);
    {
        s32 v0 = stage_GetId();
        func_8003DA8C(v0, a0);
    }
}

void game_StageCleanup(s32 a0, s32 a1) {
    func_800460E4(a0, a1);
    func_800421C8(a0);
    func_8003E0E0();
}

void *stage_GetDataPtr(void) {
    return &g_stage_data;
}

void func_80046F24(void) {
    s32 num = (s32)D_800F62E0[0].lmat.m[0][0] << 12;
    s32 div = D_800F62E0[0].lmat.m[0][1];
    s32 v0 = num / div;
    s32 v1 = ((s32)D_800F62E0[0].lmat.m[0][2] << 12) / div;
    D_800EEDB0.m[0][2] = 0;
    D_800EEDB0.m[1][0] = 0;
    v1 = -(s16)v1;
    D_800EEDB0.m[1][1] = 0;
    D_800EEDB0.m[1][2] = 0;
    D_800EEDB0.m[2][0] = 0;
    D_800EEDB0.m[0][0] = 0x1000;
    D_800EEDB0.m[2][2] = 0x1000;
    v0 = -(s16)v0;
    D_800EEDB0.m[0][1] = v0;
    D_800EEDB0.m[2][1] = v1;
}
void func_8004700C(MATRIX *a0, MATRIX *a1, s32 a2) {
    gte_MulMatrix0ClearTrans(&D_800EEDB0, a0, a1);
    a1->t[0] = a0->t[0] + (((a0->t[1] - a2) * D_800EEDB0.m[0][1]) >> 12);
    a1->t[1] = a2;
    a1->t[2] = a0->t[2] + (((a0->t[1] - a2) * D_800EEDB0.m[2][1]) >> 12);
}
void func_800470B0(s32 arg0, MATRIX *arg1, MATRIX *arg2, s32 arg3) {
    MATRIX sp10;

    sp10.m[0][0] = 0x1000;
    sp10.m[0][1] = (s16) -((s32)(D_800F62E0[arg0].lmat.m[0][0] << 12) / D_800F62E0[arg0].lmat.m[0][1]);
    sp10.m[0][2] = 0;
    sp10.m[1][0] = 0;
    sp10.m[1][1] = 0;
    sp10.m[1][2] = 0;
    sp10.m[2][0] = 0;
    sp10.m[2][1] = (s16) -((s32)(D_800F62E0[arg0].lmat.m[0][2] << 12) / D_800F62E0[arg0].lmat.m[0][1]);
    sp10.m[2][2] = 0x1000;
    gte_MulMatrix0ClearTrans(&sp10, arg1, arg2);
    arg2->t[0] = arg1->t[0] + (((arg1->t[1] - arg3) * sp10.m[0][1]) >> 12);
    arg2->t[1] = arg3;
    arg2->t[2] = arg1->t[2] + (((arg1->t[1] - arg3) * sp10.m[2][1]) >> 12);
}
typedef struct {
    s32 w[8];
} Block32;
typedef struct { s16 lo; s16 hi; } CamHalves;
extern s16 D_800EEDD6;
extern s16 D_800EEDD8;
void camera_InitBoneData(void) {
    /* FAKE: sched fence — without it sched1 hoists the lhu of D_800EEDD6
       above the block copy (renaming the copy's regs). D_800EEDD6/D_800EEDD8
       physically live INSIDE g_cam_bone_data (+6/+8), so the dependency is
       real, but the split extern symbols hide it from GCC's alias analysis;
       no distinct-symbol spelling can express it. */
    do { *(MATRIX *)&g_cam_bone_data = D_80101DF0.xf.mat; } while (0);
    {
        s16 h0 = D_800EEDD6;
        s16 h1 = D_800EEDD8;
        D_800EEDD6 = h0 >> 1;
        D_800EEDD8 = h1 >> 1;
        /* struct-view of the two halfwords at D_800EEDD8 (target relocation
           D_800EEDD8+0x2 proves the object spans 4 bytes); the direct
           *((&D_800EEDD8)+1) spelling makes cse common the +2 address into a
           register (la) where target keeps both accesses symbolic */
        ((CamHalves *)&D_800EEDD8)->hi = ((CamHalves *)&D_800EEDD8)->hi >> 1;
    }
}

void *camera_GetBoneData(void) {
    return &g_cam_bone_data;
}

void camera_InitRotation(Unk80101DF0Record *node) {
    node->unk4 = 8;
    node->unk8 = 0;
    node->unk2 = 0;
    node->unk0 = 0;
    node->unk1 = 0;
    node->unkC = 0;
    node->unkA = 4;
    node->xf.rot.vx = 0;
    node->xf.rot.vy = 0;
    node->xf.rot.vz = 0;
    g_anim_func_table[node->unk8](&node->xf.rot, &node->work);
    node->work.t[2] = 0;
    node->work.t[1] = 0;
    node->work.t[0] = 0;
    node->xf.mat = node->work;
}

s16 *camera_CalcAngles(void) {
    SVECTOR rot;
    VECTOR dir;
    MATRIX mtx;
    s16 yaw;

    math_RotMatrixYXZ(&D_800A3708->xf.rot, &mtx);
    rot.vx = 0;
    rot.vy = 0;
    rot.vz = 0x1000;
    ApplyMatrix(&mtx, &rot, &dir);
    yaw = ratan2(dir.vx, dir.vz);
    dir.vz = ((s32)Judge[(yaw + 0x400) & 0xFFF] * dir.vz + (s32)Judge[yaw & 0xFFF] * dir.vx) >> 12;
    D_800A33C8[0] = -ratan2(dir.vy, dir.vz);
    D_800A33C8[1] = yaw;
    return D_800A33C8;
}

void game_EffInit(void) {
    func_8004473C();
}

void func_8004748C(void) {
    func_80044800();
}

void game_AnimInit(void) {
    func_80048F58(0, 0);
}

void func_800474D0(void) {
    func_80048FFC(0);
}

void game_EffInit2(void) {
    func_8004473C();
}

void func_80047510(void) {
    func_80044800();
}

void func_80047530(void) {
    func_800477E8();
}

void func_80047550(void) {
    func_80047A90();
}

void camera_InitBone2(void) {
    camera_InitRotation(&g_cam_bone_data2);
    g_cam_bone_data2.unk8 = 4;
}
void func_800475A4(void) {
    SVECTOR rot;
    VECTOR result;
    MATRIX buf1;
    MATRIX buf2;
    s16 angle;
    s32 computed;
    Unk80101DF0Record *base;

    if (stage_GetVariant() != 0) {
        return;
    }

    rot.vx = 0;
    rot.vy = 0;
    rot.vz = 0x6590;
    ApplyMatrix(&D_80101DF0.xf.mat, &rot, &result);

    angle = ratan2(result.vx, result.vz);

    computed = ((s32)Judge[(angle + 0x400) & 0xFFF] * result.vz + (s32)Judge[angle & 0xFFF] * result.vx) >> 12;
    result.vz = computed;

    /* FAKE: second C handle to g_cam_bone_data2 (pointer-alias-fake-exception); the direct
     * spelling scores 6. */
    base = &g_cam_bone_data2;
    {
        /* FAKE: s16 temporary for the negated pitch: storing -ratan2() straight
         * into the field sinks its negu 8 slots to just before the sh and
         * lifts `addiu s2,sp,0x28` 5 slots. */
        s16 neg = -ratan2(result.vy, computed);
        base->xf.rot.vx = neg;
    }
    base->xf.rot.vy = angle;
    base->xf.mat.t[0] = D_80101DF0.xf.mat.t[0];
    base->xf.mat.t[1] = D_80101DF0.xf.mat.t[1];
    base->xf.mat.t[2] = D_80101DF0.xf.mat.t[2] + 0x6590;
    g_anim_func_table[4](&base->xf.rot, &buf1);
    g_anim_func_table[0](&D_80101DF0.xf.rot, &buf2);
    MulMatrix0(&buf2, &buf1, &base->xf.mat);

    {
        s32 *temp = (s32 *)D_800A3820;
        D_800A3820 = (s32)(temp + 1);
        *temp = (s32)base;
    }
}

void game_AnimStart(void) {
    func_80048F58(1, 0);
}

void func_8004775C(void) {
    func_80048FFC(0);
}

void game_EffStart(void) {
    func_8004473C();
}

void func_8004779C(void) {
    func_80044800();
}

void game_Stub1(void) {
}

void game_Stub2(void) {
}

void game_Stub3(void) {
}

void game_Stub4(void) {
}

void func_800477DC(s32 a0) {
    D_800A33D0 = (s16 *)a0;
}
extern s32 D_800EF558[];
extern s32 D_800EF59C[];
s32 func_800477E8(void) {
    s16 *s0;
    s32 s3val;
    s32 s2val;
    u16 s1val;
    u16 t1val;
    s32 a3;
    s32 a0;
    s32 a2;
    s32 a1;
    s32 t0;
    s32 v1;
    s32 t2;
    s32 v0;
    s32 *ptr;
    s32 *p;
    s32 w;
    s32 val;

    s0 = D_800A33D0;
    s3val = GetTPage(0, 0, 0x2C0, 0x1C0);
    s2val = GetTPage(0, 0, 0x2C0, 0x180);
    s1val = GetClut(0x10, 0x1E0);
    t1val = GetClut(0x10, 0x1E0);
    a3 = 0;
    t2 = 0x2C00;
    a0 = 0;
    do {
        t0 = 0x1200;
        a2 = 0x13;
        /* FAKE: single-level do-while(0) wrap. Its loop note adds one unit of
         * loop_depth reference weight to everything in this body, which seats
         * t1val in $t1 and the 0x2C00 constant in $t2 as target has them
         * (without it: t1val 3 refs/76 insns loses to t2's 5/150). */
        do {
        a1 = 0;
        v1 = 1;
inner:
        if (a3 >= 5) {
            *s0 = s3val;
            s0 += 1;
            *s0 = t2;
            s0 += 1;
            *s0 = s1val;
            s0 += 1;
            v0 = -0xC1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x100;
            *s0 = v0;
            s0 += 1;
            v0 = -0x3FC1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x4000;
        } else {
            *s0 = s2val;
            s0 += 1;
            *s0 = t2;
            s0 += 1;
            *s0 = t1val;
            s0 += 1;
            v0 = -0x40C1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x4100;
            *s0 = v0;
            s0 += 1;
            v0 = -0x7FC1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x8000;
        }
        *s0 = v0;
        s0 += 1;
        if (a3 & 1) {
            v0 = a2 | t0;
            *s0 = v0;
            s0 += 1;
            v0 = v1 | a1;
        } else {
            v0 = v1 | a1;
            *s0 = v0;
            s0 += 1;
            v0 = a2 | t0;
        }
        *s0 = v0;
        s0 += 1;
        *s0 = 0;
        s0 += 1;
        *s0 = 0;
        s0 += 1;
        v0 = -0x1000;
        *s0 = v0;
        s0 += 1;
        t0 += 0x100;
        a2 += 1;
        a1 += 0x100;
        a0 += 1;
        v1 += 1;
        if (a0 < 0x10) goto inner;
        a3 += 1;
        } while (0);
        a0 = 0;
    } while (a3 < 8);

    {
        Unk80101DF0Record *node = &D_800EF070;
        node->unk0 = 0xE;
        node->unkA = 4;
        node->work.t[0] = -0x2EE0;
        node->unk1 = 0;
        node->work.t[1] = 0;
        node->work.t[2] = -0xFA0;
        node->xf.rot.vx = 0;
        node->xf.rot.vy = 0;
        node->xf.rot.vz = 0;
        node->unk8 = 0;
        node->unkC = 0;
        node->unk6 = 0;
        func_800417D0((s32 *)node);
    }

    a3 = 0;
    w = 0;
    do {
        val = w;
        p = &D_800EF59C[a3 * 0x11];
        for (a0 = 0x10; a0 >= 0; a0--) {
            p[a0] = val;
        }
        w += 0x7D0;
        a3 += 1;
    } while (a3 < 9);

    a0 = 0;
    ptr = &D_800EF558[0];
loop3:
    *ptr = (a0 << 7) & 0xFFF;
    a0++;
    ptr++;
    if (0x11 > a0) goto loop3;

    return (s32)s0 - (s32)D_800A33D0;
}

void func_80047A90(void) {
    s32 i;
    s32 a3;
    s32 v1;
    s32 a0;
    s16 *jb;
    s32 *p558;
    s32 *p59C;
    s32 *pt2;
    s32 *pt1;
    s32 *pa1;
    s32 *pa2;
    s32 *pt3;
    s32 *temp;

    i = 0;
    jb = Judge;
    p558 = D_800EF558;
    p59C = D_800EF59C;
  loop1:
    i++;
    *p59C = ((s32)jb[*p558 & 0xFFF] * 0x271) >> 10;
    p59C++;
    *p558 += 0x12;
    p558++;
    if (i < 0x11)
        goto loop1;

    i = 1;
    pt2 = D_800EF59C;
    pt1 = D_800EF59C + 0x11;
  outer_loop:
    pa1 = pt1;
    do {
        /* FAKE: loop-note ref weighting lifts a3's allocno priority above
         * the shared counter i, seating a3 in $a3 and i in $t0 */
        a3 = 0;
    } while (0);
    do {
        /* FAKE: loop-note ref weighting lifts pa2 above the shared counter
         * i, seating pa2 in $a2 (shared with the loop-1 Judge base) */
        pa2 = pt2;
    } while (0);
    do {
        /* FAKE: loop-note ref weighting keeps pt1 ahead of pt2 in
         * allocation order (pt1->$t1, pt2->$t2) after the pa2 wrap's
         * weighted pt2 use lifted pt2 */
        pt3 = pt1 + 0x11;
    } while (0);
  inner_loop:
    a0 = 0x7D0 - (*pa1 - *pa2);
    if (a0 < 0) {
        v1 = (a0 + 0xF) >> 4;
    } else {
        v1 = a0 / 10;
    }
    *pa1 += v1;
    if (i == 8) {
        *(s32 *)((s8 *)D_800EF800 + a3) = v1;
        pa1++;
        a3 += 4;
        pa2++;
    } else {
        /* FAKE: loop tail duplicated into both arms (cross-jump re-merges,
         * byte-neutral); reg_n_refs lift lands pa2->$a2, a3->$a3 */
        pa1++;
        a3 += 4;
        pa2++;
    }
    if ((s32)pa1 < (s32)pt3)
        goto inner_loop;
    pt2 += 0x11;
    i++;
    pt1 += 0x11;
    if (i < 9)
        goto outer_loop;

    temp = (s32 *)D_800A3820;
    D_800A3820 = (s32)(temp + 1);
    *temp = (s32)&D_800EF070;
}


extern SVECTOR D_800EF0D8[17];
extern SVECTOR D_800EF168[17];
void func_80047BE0(void) {
    s32 sxy0, sxy1, sxy2, pflag, flag;
    s32 sz0, sz1, sz2;
    s16 *s7val;
    s32 i, j;
    s32 *src;
    SVECTOR *base;
    SVECTOR *v;
    s32 *dst32;
    s16 *dst16;
    s16 z;

    s7val = D_800A33D0;
    i = 0;
    src = D_800EF59C;
    while (i < 9) {
        if (i & 1) {
            base = D_800EF168;
            dst32 = (s32 *)0x1F800068;
            dst16 = (s16 *)0x1F800134;
        } else {
            base = D_800EF0D8;
            dst32 = (s32 *)0x1F800020;
            dst16 = (s16 *)0x1F800110;
        }
        v = base;
        j = 0;
        z = -0x2EE0;
        while (j < 17) {
            v->vx = src[j] - 0xFA0;
            v->vy = 0;
            v->vz = z;
            v++;
            z += 0x7D0;
            j++;
        }
        v = base;
        j = 0;
        while (j < 6) {
            RotTransPers3(&v[0], &v[1], &v[2], &sxy0, &sxy1, &sxy2, &pflag, &flag);
            v += 3;
            ReadSZfifo3(&sz0, &sz1, &sz2);
            *dst32++ = sxy0;
            *dst32++ = sxy1;
            *dst32++ = sxy2;
            *dst16++ = sz0;
            *dst16++ = sz1;
            *dst16++ = sz2;
            j++;
        }
        if (i != 0) {
            s7val = func_8004BCC0(0x10, (s16 *)base, s7val, 0);
        }
        i++;
        src += 17;
    }
}
extern s32 D_800EF7BC[];

s32 func_80047D94(s32 a0) {
    s32 a1 = (a0 + 0x7D00) / 3200;
    s32 a0_div = a0 / 3200;
    s32 remainder = a0 - a0_div * 3200;
    s32 odd = remainder & 1;
    D_800A33D4 = a1;
    D_800A33D8 = odd;
    if ((u32)a1 >= 18) {
        return (s32)0xFFFE7960;
    }
    {
        s32 val1 = D_800EF7BC[a1] * odd;
        s32 val2 = D_800EF7BC[a1 + 1] * (0x1000 - odd);
        return ((val1 + val2) >> 12) - 0x3F48;
    }
}
s32 func_80047E5C(void) {
    s32 v1 = D_800A33D4;
    if ((u32)v1 >= 18) {
        return 0;
    }
    {
        s32 v0 = D_800EF800[v1];
        s32 a0 = D_800A33D8;
        s32 val1 = v0 * a0;
        s32 v3 = D_800EF800[v1 + 1];
        s32 val2 = v3 * (0x1000 - a0);
        return (val1 + val2) >> 12;
    }
}

s32 func_80047EC8(void) {
    return 0xD00;
}

/* ---- merged from text1b.c (owner ruling Q67: one original file) ---- */
extern s32 func_8005C2A8(s32 *, s16, s32);

/* --- Functions from text1b segment (0x80047ED0 - 0x80079A30) --- */

void func_80047ED0(s32 a0) {
    D_800A33D0 = (s16 *)((u8 *)D_800A33D0 + a0);
}

void func_80047EE8(s32 arg0, s32 arg1)
{
    /* FAKE: unwritten leading frame pad (phantom-frame-slot volatile pad family)
     * -- GCC 2.7.2 function.c assign_stack_local reserves the array slot at
     * RTL-expand from the source DECL and never reclaims frame_offset after DCE, so a
     * declared-but-untouched local aggregate reproduces target's allocated-but-unwritten
     * 32-byte vars region (.frame $sp,72 - args 0x00-0x17, vars 0x18-0x37, regs
     * 0x38-0x47; ZERO sw/lw in 0x18-0x37).
     * SOTN-master precedent: volatile u32 pad; // !FAKE: at src/st/sel/2C048.c:564
     * (docs/reference/sotn-construct-index.md:101); volatile u32 pad[4]; // FAKE at
     * src/st/sel/stream.c:80 (sotn-construct-index.md:103). */
    volatile u32 pre_pad[8];
    u32 *p;
    s32 saved;
    s16 new_var;
    s32 count;
    u32 v_off;
    unsigned int new_var2;
    p = (u32 *) arg0;
    saved = (s32) p;
    arg0 = 0; /* FAKE: dead store to a PARAM (dead-store-fake-exception family)
               * -- defeats cse2's canonical-register substitution over the
               * {arg0, p, saved} equivalence class so the second pointer binds
               * addu $s0,$s2,$v0 rather than $a0. */
    p = (u32 *) ((s32) p + (((s32) (arg1 << 16)) >> 14));
    v_off = *p;
    p = (u32 *) (saved + ((v_off >> 2) << 2));
    count = *(p++);
    if (count != 0)
    {
        count--;
        do
        {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            s32 first;
            word = *p;
            p = (u32 *) (((s32) p) + 4);
            a1v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            a2v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            a3v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            new_var = a1v;
            new_var2 = word >> 2;
            first = saved + (new_var2 << 2);
            v0v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            func_800482C8(first, new_var, a2v, a3v, v0v);
        }
        while ((count--) != 0);
    }
}
void func_80047FBC(s32 arg0, s32 arg1, s16 arg2, s16 arg3)
{
    volatile u32 pre_pad[8]; // !FAKE: phantom-frame-slot volatile filler (.claude/rules/no-new-park-categories.md): target reserves 32 locals bytes at sp+0x18..sp+0x37 that no instruction touches; mechanism: GCC 2.7.2 get_frame_size reserves declared locals
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = arg0;
    p = (u32 *)arg0;
    arg0 = 0; /* FAKE: defeats cse2 canonical-reg substitution
                 that folds {reg 72 arg0, reg 78 p, reg 79 base_addr}
                 equivalence class at insn 36 */
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(base_addr + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = base_addr + (((u32)word >> 2) << 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            func_800482C8(new_var,
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg2,
                          (s32)v0v + sx_arg3);
        } while ((count--) != 0);
    }
}
void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    volatile u32 pre_pad[8]; // !FAKE: phantom-frame-slot volatile filler -- the target reserves 32 locals bytes at sp+0x18..sp+0x37 that no instruction in asm/funcs/func_800480C0.s reads or writes; GCC 2.7.2 get_frame_size/expand_decl reserves declared locals (config/mips/mips.c:4443-4475), while any referenced producer costs >=1 store (flow.c:1740-1741 never deletes the last store to a frame object)
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = arg0;
    p = (u32 *)arg0;
    arg0 = 0; // !FAKE: dead store to a PARAMETER (dead-store family; same construct as sibling func_80047FBC) -- defeats GCC 2.7.2 cse.c canonical-register substitution, which otherwise folds the {arg0, p, base_addr} equivalence class and emits one base copy instead of two
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(base_addr + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        s32 sx_arg4;
        s32 sx_arg5;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        sx_arg4 = arg4;
        sx_arg5 = arg5;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = base_addr + (((u32)word >> 2) << 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            func_800482C8(new_var,
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg4,
                          (s32)v0v + sx_arg5);
        } while ((count--) != 0);
    }
}
void func_800481E8(s32 arg0, s32 arg1)
{
    /* Base-copy staging + function-scope precompute (with the `arg0 = 0;`
     * FAKE below) make GCC stage arg0 through $s0 first ($s0=$a0; $s2=$s0),
     * as in the target. */
    /* FAKE: unwritten leading frame pad (phantom-frame-slot volatile pad family;
     * engine/volatile_cheats.py _SANCTIONED_UNWRITTEN_PADS row)
     * -- GCC 2.7.2 function.c assign_stack_local reserves the array slot at
     * RTL-expand from the source DECL and never reclaims frame_offset after DCE, so a
     * declared-but-untouched local aggregate reproduces target's allocated-but-unwritten
     * 32-byte vars region (.frame $sp,72 - args 0x00-0x17 incl the 5th-arg slot
     * sw $v0,0x10($sp); vars 0x18-0x37 with ZERO sw/lw; regs 0x38-0x47). Identical
     * shape and size to the FAKE pads of the two siblings in this file (func_80047EE8 /
     * func_80047FBC).
     * SOTN-master precedent: volatile u32 pad; // !FAKE: at src/st/sel/2C048.c:564
     * (docs/reference/sotn-construct-index.md:101); volatile u32 pad[4]; // FAKE at
     * src/st/sel/stream.c:80 (sotn-construct-index.md:103). */
    volatile u32 pre_pad[8];
    u32 *p;
    u32 *base;
    s32 count;
    s32 a0_for_call;
    p = (u32 *)arg0;
    base = p;
    arg0 = 0; /* FAKE: dead store to a param -- breaks the $a0==base association:
               * without it GCC keeps arg0 live in $a0 and emits `addu $s0,$a0,$v0`;
               * with it the second pointer binds to base in $s2, as in the target. */
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)((s32)base + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        count--;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            u16 v0v;
            unsigned int new_var2;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = *((u16 *)p);
            new_var2 = word >> 2;
            a0_for_call = (s32)base + (new_var2 << 2);  /* compute early (target sched) */
            p = (u32 *)(((s32)p) + 2);  /* always advance (target delay slot) */
            if ((s32)a3v < 0x280) {
                v0v += 1;
            }
            func_800482C8(a0_for_call,
                          (s32)a1v,
                          (s32)a2v,
                          (s32)a3v,
                          (s32)(s16)v0v);
        } while ((count--) != 0);
    }
}
void func_800482C8(u8 *arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4) {
    u16 arg4_lo = *(u16 *)&arg4;
    s16 rect[4];
    s16 buf[512];
    u8 *p_alt;
    s32 flags;
    u32 dim;
    u32 dim2;
    s32 head = arg0[0];
    arg0 += 4;
    if (head != 0x10) return;
    flags = *(s32 *)arg0 & 8;
    arg0 += 4;
    if (flags != 0) {
        p_alt = arg0;
        arg0 = p_alt + (((u32)*(u32 *)p_alt >> 2) << 2);
        p_alt += 4;
    }
    arg0 += 8;
    rect[0] = arg1;
    rect[1] = arg2;
    dim = *(u32 *)arg0;
    rect[3] = dim >> 16;
    rect[2] = dim;
    LoadImage(rect, (s32 *)(arg0 + 4));
    if (flags == 0) return;
    p_alt += 4;
    rect[0] = arg3;
    rect[1] = arg4_lo;
    dim2 = *(u32 *)p_alt;
    p_alt += 4;
    rect[3] = dim2 >> 16;
    rect[2] = dim2;
    if (func_800486FC() != 0) {
        math_GrayscaleRgb555((s32)p_alt, rect[2], (s32)buf);
        LoadImage(rect, (s32 *)buf);
        DrawSync(0);
        return;
    }
    LoadImage(rect, (s32 *)p_alt);
}


void func_800483DC(s32 arg0, s32 arg1, s16 arg2, s16 arg3)
{
    s32 base;
    s32 count;
    s32 off;
    arg1 = (((s32)(arg1 << 16)) >> 14) + arg0;
    base = arg0;
    off = *(s32 *)arg1;
    arg0 += off;
    count = *(s32 *)arg0;
    arg0 += 4;
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        do {
            s32 entry;
            u32 dx_u;
            u32 dy_u;
            s32 dx;
            s32 dy;
            entry = base + *(s32 *)arg0;
            arg0 += 8;
            dx_u = *(u16 *)arg0;
            arg0 += 2;
            dy_u = *(u16 *)arg0;
            arg0 += 2;
            dx = ((s32)(dx_u << 16)) >> 16;
            dy = ((s32)(dy_u << 16)) >> 16;
            func_800484A0(entry, dx + sx_arg2, dy + sx_arg3);
        } while ((count--) != 0);
    }
}
void func_800484A0(u8 *arg0, s16 arg1, s16 arg2) {
    s16 rect[4];
    s16 buf[512];
    u32 dim;
    s32 flags;
    if (arg0[0] != 0x10) return;
    arg0 += 4;
    flags = *(s32 *)arg0;
    arg0 += 4;
    if ((flags & 8) == 0) return;
    arg0 += 8;
    rect[0] = arg1;
    rect[1] = arg2;
    dim = *(u32 *)arg0;
    arg0 += 4;
    rect[3] = dim >> 16;
    rect[2] = dim;
    if (func_800486FC() != 0) {
        math_GrayscaleRgb555((s32)arg0, rect[2], (s32)buf);
        LoadImage(rect, (s32)buf);
        return;
    }
    LoadImage(rect, (s32)arg0);
}
extern void func_800485EC();
s32 func_80048530(s32 arg0, s32 arg1, u32 arg2, s32 arg3) {
    s32 base, count, entry, a, b, c, d, off;
    off = ((s32 *)arg0)[arg1];
    base = arg0;
    /* FAKE: operand order chosen to match target (off + base, not base + off);
     * mechanism: RTL expansion's commutative-operand canonicalization
     * (expand_binop) keeps two equal-precedence pseudos in source order, and
     * no later pass (combine/sched) reorders the addu operands — so only the
     * off-first source spelling emits target's `addu $v1,$v0,$v1` (natural
     * base+off is 1 insn off; cc1psx also emits base-first from it); the
     * single-order carve-out in .claude/rules/or-tree-shape-shift.md. */
    arg0 = off + base;
    count = *(s32 *)arg0;
    arg0 += 4;
    if (arg2 >= (u32)count) return -1;
    arg0 += arg2 * 0xC;
    entry = *(s32 *)arg0;
    arg0 += 4;
    a = (s32)*(u16 *)arg0;
    arg0 += 2;
    b = (s32)*(u16 *)arg0;
    arg0 += 2;
    c = (s32)*(u16 *)arg0;
    arg0 += 2;
    d = (s32)*(u16 *)arg0;
    entry += base;
    func_800485EC(entry, arg3, (s16)a, (s16)b, (s16)c, (s16)d);
    return count;
}
typedef struct {
    /* 0x00 */ s16 mode;
    /* 0x02 */ s16 x;
    /* 0x04 */ s16 y;
    /* 0x06 */ s16 w;
    /* 0x08 */ s16 h;
    /* 0x0A */ s16 cx;
    /* 0x0C */ s16 cy;
    /* 0x0E */ s16 cw;
    /* 0x10 */ s16 ch;
    /* 0x12 */ u16 tpage;
    /* 0x14 */ u16 clut;
    /* 0x18 */ u32 *pixdata;
    /* 0x1C */ u32 *clutdata;
} TimHdr485;
void func_800485EC(tim, spr, x, y, cx, cy)
u32 *tim;
TimHdr485 *spr;
s16 x, y;
u16 cx, cy;
{
    u32 flag;
    u32 *p;

    if (*(u8 *)tim++ == 0x10) {
        flag = *tim++;
        spr->mode = flag & 7;
        if (flag & 8) {
            u32 bnum;

            p = tim;
            bnum = *p;
            spr->cx = cx;
            spr->cy = cy;
            tim = p + (bnum >> 2);
            p += 2;
            spr->ch = ((u16 *)p)[1];
            spr->cw = *p++;
            spr->clutdata = p;
            spr->clut = GetClut(spr->cx, spr->cy);
        } else {
            spr->clut = 0;
        }
        p = tim + 2;
        /* !FAKE: cancellation pair (F6 family, exact `i++; i--;` shape).
         * What: net-zero adjacent same-variable inc/dec of tim, byte-free
         * (survives cse1/cse2, then flow.c dead-store elimination deletes both).
         * Mechanism: cse.c fold_rtx PLUS-association (cse.c:5589-5666, applied
         * uncosted to addresses via find_best_addr, cse.c:2663) rewrites the
         * pixel-block reads onto tim whenever p's recorded equivalent
         * (plus tim 8) is valid; the pair bumps reg_tick(tim) so exp_equiv_p
         * invalidates that equivalence and the reads keep p as base, matching
         * target's addiu v1,s1,8 + lhu 2(v1)/lw 0(v1)/addiu v1,v1,4;
         * every join-local p==tim+K chain folds otherwise. */
        tim++;
        tim--;
        spr->x = x;
        spr->y = y;
        spr->h = ((u16 *)p)[1];
        spr->w = *p++;
        spr->pixdata = p;
        spr->tpage = GetTPage(spr->mode, 0, spr->x & 0xFFC0, spr->y & 0xFF00);
    }
}
s32 func_800486FC(void) {
    if (file_GetFlag0()) {
        g_color_mode = 1;
    } else {
        g_color_mode = 0;
    }
    return g_color_mode;
}
void func_80048744(s32 a0) {
    if (a0) {
        g_color_mode = 1;
    } else {
        g_color_mode = 0;
    }
}
void math_GrayscaleRgb555(u16 *arg0, s32 arg1, u16 *arg2) {
    s32 temp_a3;
    s32 temp_v1;
    s32 var_t0;
    u16 *var_t1;
    s32 temp_a1;
    var_t1 = arg0;
    var_t0 = arg1 - 1;
    if (var_t0 != -1) {
        do {
            temp_a1 = *var_t1;
            var_t1 += 1;
            var_t0 -= 1;
            temp_v1 = temp_a1 << 0x10;
            {
                s32 b;
                s32 g;
                s32 bt;
                temp_a3 = temp_a1 & 0x1F;
                temp_a3 = temp_a3 * 0x547;
                b = (temp_v1 >> 0x1A) & 0x1F;
                g = (temp_v1 >> 0xA) & 0xF800;
                bt = b * 0x2B8;
                temp_a3 = ((temp_a3 + g + bt) >> 0xC) & 0x1F;
            }
            *arg2 = (u16) ((((temp_a1 & (~0x7FFF)) + (temp_a3 << 0xA)) + (temp_a3 << 5)) + temp_a3);
            arg2 += 1;
        } while (var_t0 != (-1));
    }
}
s32 math_Grayscale3(s32 arg0, s32 arg1, s32 arg2) {
    arg0 = arg0 * 0x547;
    arg1 = arg1 << 11;
    arg2 = arg2 * 0x2B8;
    return (arg0 + arg1 + arg2) >> 12;
}

void func_80048864(s32 mode, s32 sx, s32 sy, s32 w, s32 mr, s32 mg, s32 mb, s32 dx, s32 dy) {
    u16 buf[256];
    u16 out[256];
    s16 rect[4];
    u16 *src;
    u16 *dst;
    s32 i;
    u16 p;
    s32 r, g, b, a;

    DrawSync(0);
    rect[0] = sx;
    rect[1] = sy;
    rect[2] = w;
    rect[3] = 1;
    StoreImage(rect, buf);
    DrawSync(0);
    src = buf;
    dst = out;
    for (i = 0; i < w; i++) {
        p = *src;
        if (p == 0) {
            *dst++ = *src++;
            continue;
        }
        r = (p & 0x1F) << 3;
        g = ((p >> 5) & 0x1F) << 3;
        b = ((p >> 10) & 0x1F) << 3;
        a = p & 0x8000;
        src++;
        switch (mode) {
        case 0:
            r = (r * mr) >> 15;
            g = (g * mg) >> 15;
            b = (b * mb) >> 15;
            break;
        case 1:
            r = r * 0x547;
            g = g << 11;
            b = b * 0x2B8;
            r = (r + g + b) >> 15;
            r = (r * mr) >> 12;
            g = (r * mg) >> 12;
            b = (r * mb) >> 12;
            break;
        }
        r &= 0x1F;
        g &= 0x1F;
        b &= 0x1F;
        *dst++ = a | r | (g << 5) | (b << 10);
    }
    rect[0] = dx;
    rect[1] = dy;
    LoadImage(rect, out);
    DrawSync(0);
}
void func_80048A7C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    func_80048864(0, arg0, arg1, arg2, arg3, arg4, arg5, arg0, arg1);
}
extern s32 *func_800467B8(s32); /* matches the definition above */
extern void func_800468B0(s32);
extern u8 D_80099BCC;
s32 func_80048AD0(s32 arg0) {
    s32 temp_v0;
    s32 sound;
    s32 idx;
    u8 *base;
    s32 delta;
    u8 *p;
    u8 *q;

    temp_v0 = (s32)func_8004153C(arg0);
    if (temp_v0 == 0) return 0;
    idx = *(s16 *)(temp_v0 + 8);
    D_800A33E0 = arg0;
    sound = (&D_80099BCC)[idx];
    if (sound == 0xFF) return 0;
    base = (u8 *)func_800467B8(sound);
    p = base + ((*(u32 *)(base + 8) >> 2) << 2);
    delta = (s32)(p - base);
    D_800A33E4 = (s32)p;
    q = p + 0xA;
    /* FAKE: the record counter reuses `sound` rather than a fresh local.
       snd_LoadBgm's argument copy gives `sound` a hard-reg $a0 preference;
       global.c expand_preferences propagates it to the counter, which stops
       prune_preferences making the counter yield $a0 to `delta`. With a
       separate counter the pair allocates $a2/$a0 instead of target's
       $a0/$a2. */
    for (sound = 0; sound < 0x11; sound++) {
        *(s16 *)(q - 8 + sound * 0x68) = sound;
        *(s16 *)(q - 6 + sound * 0x68) = 9;
        p[sound * 0x68] = 0xF;
        *(s8 *)(q - 9 + sound * 0x68) = 0;
        *(s16 *)(q + sound * 0x68) = (s16)arg0;
    }
    func_800468B0(delta + 0x6E8);
    return 1;
}
void func_80048B8C(s32 a0) {
    D_800A33E4 += a0;
}
extern void math_RotMatrixZYX(SVECTOR *, MATRIX *);
extern s32 ClearOTagR(s32, s32);
extern s32 g_gpu_ot256_ptr;
extern u8 g_gpu_ot256_db[];
extern s16 D_80099C14[];

void func_80048BA4(s32 arg0, s32 arg1, s32 arg2) {
    MATRIX mtx;
    SVECTOR rot;
    s32 index;
    s32 scale;
    s32 old;
    s16 *indices;
    s32 *ot;
    MATRIX **player;
    u8 *prim;

    player = game_GetPlayerData(D_800A33E0);
    if (player == 0) {
        return;
    }
    if (arg1 >= 6) {
        arg1 = -1;
    }

    rot.vx = 0x1770;
    rot.vy = 0;
    rot.vz = 0;
    scale = 0x1770;
    rot.vz = ((s32)Judge[arg0 & 0xFFF] * scale) >> 12;
    rot.vx = ((s32)Judge[(arg0 + 0x400) & 0xFFF] * scale) >> 12;
    prim = (u8 *)D_800A33E4;
    ApplyMatrix(player[0], &rot, (VECTOR *)D_800FF558.t);
    D_800FF558.t[0] += player[0]->t[0];
    D_800FF558.t[1] += player[1]->t[1];
    D_800FF558.t[2] += player[2]->t[2];

    rot.vx = 0;
    rot.vy = 0xC00 - arg0;
    rot.vz = 0;
    indices = D_80099C14;
    math_RotMatrixZYX(&rot, &mtx);
    gte_MulMatrix0ClearTrans(player[0], &mtx, &mtx);
    D_800FF558.m[0][0] = mtx.m[0][0];
    D_800FF558.m[0][1] = mtx.m[1][0];
    D_800FF558.m[0][2] = mtx.m[2][0];
    D_800FF558.m[1][0] = mtx.m[0][1];
    D_800FF558.m[1][1] = mtx.m[1][1];
    D_800FF558.m[1][2] = mtx.m[2][1];
    D_800FF558.m[2][0] = mtx.m[0][2];
    D_800FF558.m[2][1] = mtx.m[1][2];
    D_800FF558.m[2][2] = mtx.m[2][2];

    goto test_index;
copy_index:
        *(MATRIX *)(prim + 0x18) = *player[index];
        ot = (s32 *)D_800A3820;
        D_800A3820 = (s32)(ot + 1);
        *ot = (s32)prim;
        prim += 0x68;
test_index:
    index = *indices;
    indices++;
    if (index >= 0) {
        goto copy_index;
    }
    if (arg1 >= 0) {
        *(MATRIX *)(prim + 0x18) = *player[18];
        ot = (s32 *)D_800A3820;
        *(s16 *)(prim + 2) = arg1 + 0xF;
        D_800A3820 = (s32)(ot + 1);
        *ot = (s32)prim;
        prim += 0x68;
    }
    if (arg2 != 0) {
        *(MATRIX *)(prim + 0x18) = *player[19];
        ot = (s32 *)D_800A3820;
        *(s16 *)(prim + 2) = 0x15;
        D_800A3820 = (s32)(ot + 1);
        *ot = (s32)prim;
    }

    g_gpu_ot256_ptr = (s32)(g_gpu_ot256_db + ((D_800A36AC & 1) << 10));
    ClearOTagR(g_gpu_ot256_ptr, 0x100);
    old = D_800A378C[0];
    D_800A378C[0] = (g_gpu_ot256_ptr + 0x3FC) & 0xFFFFFF;
    *(s32 *)g_gpu_ot256_ptr = old;
}

extern void func_80052C10(void);
void func_80048F58(s32 a0, s32 a1) {
    s32 i;
    u16 *src;
    s16 *dst;
    MoveChannel *base;
    if (a1 > 0) {
        func_80052C10();
    }
    base = &D_800EF848[a1];
    base->phase = 0;
    src = D_80099C34[a0];
    dst = base->ctl;
    i = 0;
    do {
        *dst = *src;
        src++;
        i++;
        dst++;
    } while (i < 7);
}

void func_80048FFC(s32 arg0) {
    RECT rect;
    s16 *ctl;
    s32 x;
    s32 y;
    s32 xf;
    s32 yf;
    s32 dx;
    s32 dy;
    s32 dxf;
    s32 dyf;
    s32 w;
    s32 period;
    s32 i;
    MoveChannel *ch;
    DR_MOVE *p;
    s32 phase;

    ch = &D_800EF848[arg0];
    phase = ch->phase;
    ctl = ch->ctl;
    p = ch->move[D_800A36AC & 1];
    x = ctl[0] & ~0x3F;
    y = ctl[1] & ~0xFF;
    xf = ctl[0] % 64;
    yf = ctl[1] % 256;
    w = ctl[2];
    period = ctl[3];
    dx = ctl[4] & ~0x3F;
    dy = ctl[5] & ~0xFF;
    dxf = ctl[4] % 64;
    dyf = ctl[5] % 256;
    for (i = 0; i < 3; i++) {
        s32 nx = dx + dxf;
        s32 ny = dy + dyf;
        s32 sy = y + yf;
        s32 h1 = period - phase;
        /* FAKE: strip two's height taken as an s16 at the top of the level; rect.h = phase
         * directly, an s32 copy or a (s16) cast drops the t1 copy. */
        s16 h2 = phase;
        rect.x = x + xf;
        rect.y = sy;
        rect.w = w;
        rect.h = h1;
        SetDrawMove(p, &rect, nx, ny + phase);
        /* FAKE: SDK addPrim (setaddr/getaddr P_TAG views) on OT entry 0xFFF. */
        /* SOTN: include/psxsdk/libgpu.h:88 @db41b28eee52969244a52cc269c8163d1ed8826a (PS1 use: src/main/psxsdk/libgpu/sys.c:288) */
        ((OTag *)p)->addr = ((OTag *)&D_800A378C[0xFFF])->addr;
        ((OTag *)&D_800A378C[0xFFF])->addr = (u32)p;
        p++;
        rect.y = sy + h1;
        rect.h = h2;
        SetDrawMove(p, &rect, nx, ny);
        ((OTag *)p)->addr = ((OTag *)&D_800A378C[0xFFF])->addr;
        ((OTag *)&D_800A378C[0xFFF])->addr = (u32)p;
        p++;
        xf >>= 1;
        yf >>= 1;
        dxf >>= 1;
        dyf >>= 1;
        w >>= 1;
        period >>= 1;
        phase >>= 1;
    }
    ch->phase += ctl[6];
    if (ch->phase >= ctl[3]) {
        ch->phase %= ctl[3];
    }
}
extern s16 D_800EF9F2;
extern s16 D_800EF9F4;
void func_8004939C(void) {
    s16 val = -1;
    s32 i = 0x39;
    s16 *p = &D_800EF9F2;
    do {
        *p = val;
        i--;
        p--;
    } while (i >= 0);
    D_800EF9F4 = -2;
    D_800A33E8[1] = -1;
    D_800A33E8[0] = -1;
    D_800A33EC = -1;
}

extern u8 D_80099CC8[];
extern u8 D_80099CC9[];
extern s16 D_800EF980[];
void func_800493E4(s32 arg0) {
    u8 temp_v1;
    s32 idx;

    D_800EF980[arg0] = 1;
    if (D_800A33EC == -1) {
        if (arg0 >= 0x33) {
            D_800A33EC = 1;
        } else {
            D_800A33EC = 0;
        }
    } else {
        if (D_800A33EC == 0 && !(arg0 < 0x33)) {
            func_80052C10();
        }
        if (D_800A33EC == 1 && arg0 < 0x33) {
            func_80052C10();
        }
    }
    idx = arg0 * 2;
    temp_v1 = D_80099CC8[idx];
    if (temp_v1 != 0xFF) {
        D_800EF980[temp_v1] = 1;
        /* FAKE: loop notes keep the D_80099CC9 lbu below the first sh (target has
           the unfilled load-delay nop) and keep the shared 1 cached in $v1 */
        do { } while (0);
        D_800EF980[D_80099CC9[idx]] = 1;
    }
}

void func_800494D4(s32 idx, s32 val) {
    s32 cond;
    if (D_800A33EC == 0) {
        cond = val < 16;
    } else {
        cond = val < 8;
    }
    if (!cond) {
        func_80052C10();
    }
    if (((u32)idx) >= 2U) {
        func_80052C10();
    }
    D_800A33E8[idx] = (s16)val;
}
s32 func_8004954C(s32 arg0, s32 arg1, s32 arg2)
{
    s32 sum = 0;
    s32 i;
    for (i = 0; i < arg1; i++) {
        /* FAKE: do-while(0) loop-note ref weighting flips the sum/i allocno
           priority so sum seats in $v1 and i in $a3 (matches target). */
        do { sum += arg0; arg0 -= 1; } while (0);
    }
    return sum + (arg2 - arg1);
}
extern s16 D_80099C50[];
extern s32 D_800A324C;
extern s32 func_8004954C(s32, s32, s32);
void func_80049584(s32 arg0) {
    s16 *dst;
    s16 *src;
    s16 *p;
    /* FAKE: `i` carries both the two loop counters and the computed total,
       mechanism: global.c allocno allocation — only a pseudo that crosses a
       CALL is eligible for a call-saved hard reg, so sharing one variable is
       what puts the loop counter in $s0 (target); with a separate `total` the
       counter takes a call-clobbered reg and 12 insns diverge. */
    s32 i;
    s32 unchanged;
    s32 rank;
    s32 step;
    s32 lo;
    s32 hi;

    unchanged = 1;
    i = 0;
    dst = D_80099C50;
    src = D_800EF980;
    do {
        s16 v = *src;
        if ((v >= 0) != ((*dst) >= 0)) {
            unchanged = 0;
        }
        *dst = v;
        dst++;
        i++;
        src++;
    } while (i < 0x3A);
    rank = 0;
    i = 0;
    p = D_800EF980;
    do {
        if ((*p) >= 0) {
            *p = (s16) rank;
            rank++;
        }
        i++;
        p++;
    } while (i < 0x3A);
    step = 8;
    if (D_800A33EC == 0) {
        step = 0x10;
    }
    hi = D_800A33E8[0];
    if (hi == -1) {
        lo = D_800A33E8[1];
        if (lo == hi) {
            i = 0x24;
            if (D_800A33EC == 0) {
                i = 0x88;
            }
            goto end;
        }
        hi = lo;
    } else {
        lo = hi;
        if ((D_800A33E8[1] != (-1)) && (hi != D_800A33E8[1])) {
            if (hi < D_800A33E8[1]) {
                hi = D_800A33E8[1];
            } else {
                lo = D_800A33E8[1];
            }
        }
    }
    i = func_8004954C(step, lo, hi);
end:
    if (D_800A324C != i) {
        D_800A324C = i;
        unchanged = 0;
    }
    if (unchanged == 0) {
        func_80046020();
        func_80045B68(D_800A33EC, i, D_800EF980, arg0);
        func_8003E120();
    }
}
void func_80049710(void) {
}

extern u8 *D_800A38B4;

/* Appends one or two 0x68-byte draw objects at D_800A38B4 for animation entry
 * arg0 and links each into the ordering table at D_800A3820.  The first object
 * (type 0) gets its rotation and position either from rot_in/pos (flags == 1:
 * g_anim_func_table[0] turns rot_in into the object's matrix at +0x18) or from
 * part (flags & 1) of vehicle flags >> 1: the part's offset (+0x4C) is scaled
 * by the vehicle's +0x12, the vehicle matrix (+0x44) times the part's matrix
 * (+0x38) becomes the object's matrix, the scaled offset run through the
 * parent's matrix (part +0xC, matrix +0x18, translation +0x2C) gives its
 * position, and the matrix is copied back into the part.  The second object
 * (type 3, parent = the first) follows unless flags is still 1.  Objects and
 * parts are walked by byte offset, as func_80049A2C below does. */
void func_80049718(s32 arg0, s32 flags, s32 *pos, s16 *rot_in) {
    SVECTOR ofs;
    s32 val58;
    u8 *vehicle;
    u8 *obj;
    u8 *part;
    /* FAKE: named intermediate (no-new-park-categories entry 6).  Set before
     * the call, sched1 moves the andi past func_8004153C but ahead of the copy
     * of its result (`andi v1,s3,1; move s1,v0`, as in the target); written
     * inside the part expression or after the call it follows the copy and
     * takes $v0. */
    s32 side;
    if (D_800EF980[arg0] < 0) {
        func_80052C10();
    }
    obj = D_800A38B4;
    /* FAKE: dead store (dead-store-fake-exception).  The 0 is never read: the
     * flags == 1 path skips the second object.  Flow cannot tell, so the store
     * stays as the target's `move s5,zero`; without it that instruction is
     * missing. */
    val58 = 0;
    obj[0] = 0;
    obj[1] = 0;
    *(s16 *)(obj + 2) = D_800EF980[arg0] * 2;
    *(s16 *)(obj + 4) = 6;
    *(s16 *)(obj + 8) = 0;
    *(s32 *)(obj + 0xC) = 0;
    *(s16 *)(obj + 0xA) = 4;
    if (flags != 0) {
        if (flags == 1) {
            *(s16 *)(obj + 0x10) = rot_in[0];
            *(s16 *)(obj + 0x12) = rot_in[1];
            *(s16 *)(obj + 0x14) = rot_in[2];
            g_anim_func_table[0]((SVECTOR *)(obj + 0x10), (MATRIX *)(obj + 0x18));
            *(s32 *)(obj + 0x2C) = pos[0];
            *(s32 *)(obj + 0x30) = pos[1];
            *(s32 *)(obj + 0x34) = pos[2];
        } else {
            /* FAKE: flags is rewritten in place - compound-assigned, read (>> 1, & 1),
             * compound-assigned again, read (!= 1) - as SOTN reuses a parameter
             * (Q51).  Copied into a local instead, global.c's allocno order flips:
             * rot_in's pseudo (priority 3333) outranks the table-address pseudo
             * (3000) for $s0, against 2962 / 3333 in place. */
            /* SOTN: src/st/lib/e_shop.c:4621 @aa53500 */
            flags &= 0x7FFF;
            side = flags & 1;
            vehicle = (u8 *)func_8004153C(flags >> 1);
            part = vehicle + (side * 0x68 + 0x7E4);
            *(s32 *)(part + 0x4C) = (*(s32 *)(part + 0x4C) * *(s16 *)(vehicle + 0x12)) >> 12;
            *(s32 *)(part + 0x50) = (*(s32 *)(part + 0x50) * *(s16 *)(vehicle + 0x12)) >> 12;
            *(s32 *)(part + 0x54) = (*(s32 *)(part + 0x54) * *(s16 *)(vehicle + 0x12)) >> 12;
            MulMatrix0((MATRIX *)(vehicle + 0x44), (MATRIX *)(part + 0x38), (MATRIX *)(obj + 0x18));
            ofs.vx = *(s32 *)(part + 0x4C);
            ofs.vy = *(s32 *)(part + 0x50);
            ofs.vz = *(s32 *)(part + 0x54);
            ApplyMatrix((MATRIX *)(*(u8 **)(part + 0xC) + 0x18), &ofs, (VECTOR *)(obj + 0x2C));
            *(s32 *)(obj + 0x2C) = *(s32 *)(obj + 0x2C) + *(s32 *)(*(u8 **)(part + 0xC) + 0x2C);
            *(s32 *)(obj + 0x30) = *(s32 *)(obj + 0x30) + *(s32 *)(*(u8 **)(part + 0xC) + 0x30);
            *(s32 *)(obj + 0x34) = *(s32 *)(obj + 0x34) + *(s32 *)(*(u8 **)(part + 0xC) + 0x34);
            /* SOTN: src/st/lib/e_shop.c:4625 @aa53500 */
            flags |= 0x8000;
            *(MATRIX *)(part + 0x18) = *(MATRIX *)(obj + 0x18);
            val58 = *(s16 *)(vehicle + 0x1A84);
        }
        {
            u8 *ot = (u8 *)D_800A3820;
            D_800A3820 = (s32)(ot + 4);
            *(u8 **)ot = obj;
        }
        obj += 0x68;
        if (flags != 1) {
            /* FAKE: named intermediate (no-new-park-categories entry 6).  The
             * table is read before the object's fields are written, as in the
             * target (lh first); storing D_800EF980[arg0] * 2 + 1 directly at
             * the +2 store reads it last, and moving that store first
             * reorders the stores. */
            s32 frame = D_800EF980[arg0];
            u8 *ot;
            obj[0] = 3;
            obj[1] = 0;
            *(s32 *)(obj + 0x58) = val58;
            ot = (u8 *)D_800A3820;
            *(s32 *)(obj + 0xC) = (s32)(obj - 0x68);
            *(s16 *)(obj + 6) = 1;
            *(s16 *)(obj + 8) = 0;
            *(s16 *)(obj + 0xA) = 0;
            *(s16 *)(obj + 4) = 6;
            *(s16 *)(obj + 2) = frame * 2 + 1;
            D_800A3820 = (s32)(ot + 4);
            *(u8 **)ot = obj;
            obj += 0x68;
        }
        D_800A38B4 = obj;
    }
}
extern s16 D_80099D3C[];
/* func_80049A2C: the only non-ordinary construct is the first declaration,
 * `volatile u32 pre_pad[2];`, a labelled FAKE (phantom-frame-slot volatile pad family; row
 * ("pre_pad", 2) in engine/volatile_cheats.py _SANCTIONED_UNWRITTEN_PADS). It
 * gives target's frame signature, .frame $sp,48 # vars= 8, regs= 5/0.
 * Target's +8 vars region is reachable from ordinary C only via a
 * combine-orphaned pseudo (reload1.c:2404 alter_reg), and the only
 * fold-capable symbol (D_80099D3C) cannot host it: the fold that creates the
 * orphan shortens the arg1 index chain, flips sched1's hoist, and costs a
 * SIXTH callee-saved register (target saves five). D_800EF980/D_80099CC8 are
 * single-index (CSE merges every respelling), and the function is loopless,
 * so no back-edge carrier exists; hence the FAKE pad. SOTN-master PSX precedent:
 * docs/reference/sotn-construct-index.md L620/L626/L627
 * (volatile char pad[8] //! FAKE; volatile u32 pad; volatile u32 pad[4]).
 */
void func_80049A2C(s32 arg0, s32 arg1, s32 arg2) {
    volatile u32 pre_pad[2]; // !FAKE: phantom-frame-slot volatile filler (.claude/rules/no-new-park-categories.md): target reserves 8 locals bytes at sp+0x10..sp+0x17 that no instruction touches; mechanism: GCC 2.7.2 get_frame_size reserves declared locals
    u8 *new_var6;
    u8 *new_var5;
    s16 *new_var7;
    u8 temp_v1;
    u8 *new_var8;
    s16 *p_anim;
    s16 new_var2;
    s16 *src;
    u8 *obj;
    u8 *vehicle;
    s16 a1_val;
    u8 *ot;

    new_var6 = D_80099CC8;
    {
        u8 *p = new_var6 + (arg0 * 2);
        temp_v1 = p[arg2];
    }
    if (temp_v1 == 0xFF) {
        return;
    }
    new_var8 = (u8 *) D_800EF980;
    p_anim = (s16 *) (new_var8 + (temp_v1 * 2));
    if ((*p_anim) < 0) {
        func_80052C10();
    }
    vehicle = (u8 *) func_8004153C(arg1 >> 1);
    obj = D_800A38B4;
    obj[0] = 0;
    obj[1] = 0;
    a1_val = (*p_anim) * 2;
    *((s16 *) (obj + 4)) = 6;
    *((s16 *) (obj + 8)) = 0;
    *((s16 *) (obj + 0xA)) = 4;
    *((s16 *) (obj + 2)) = a1_val;
    src = &D_80099D3C[(arg1 & 1) * 6];
    *((s32 *) (obj + 0x4C)) = ((s32) ((*src) * (*((s16 *) (vehicle + 0x12))))) >> 12;
    src++;
    *((s32 *) (obj + 0x50)) = ((s32) ((*src) * (*((s16 *) (vehicle + 0x12))))) >> 12;
    src++;
    *((s32 *) (obj + 0x54)) = ((s32) ((*src) * (*((s16 *) (vehicle + 0x12))))) >> 12;
    src++;
    *((u16 *) (obj + 0x10)) = (u16) (*src);
    src++;
    *((u16 *) (obj + 0x12)) = (u16) (*src);
    new_var2 = src[1];
    *((s32 *) (obj + 0xC)) = (s32) (vehicle + 0x50C);
    *((s16 *) (obj + 6)) = 0;
    *((u16 *) (obj + 0x14)) = (u16) new_var2;
    func_800417D0((s32 *) obj);
    ot = (u8 *)D_800A3820;
    D_800A3820 = (s32)(ot + 4);
    *((u8 **) ot) = obj;
    obj += 0x68;
    new_var5 = obj + 0xA;
    a1_val = (*p_anim) * 2;
    obj[0] = 3;
    *((s32 *) (obj + 0xC)) = (s32) (obj - 0x68);
    obj[1] = 0;
    new_var7 = (s16 *) (obj + 6);
    *((s16 *) (obj + 8)) = 0;
    *new_var7 = 1;
    *((s16 *) new_var5) = 0;
    *((s16 *) (obj + 4)) = 6;
    *((s16 *) (obj + 2)) = (s16) (a1_val + 1);
    *((s32 *) (obj + 0x58)) = (s32) (*((s16 *) (vehicle + 0x1A84)));
    ot = (u8 *)D_800A3820;
    D_800A3820 = (s32)(ot + 4);
    *((u8 **) ot) = obj;
    D_800A38B4 = obj + 0x68;
}
s32 func_80049C24(s32 arg0, s32 arg1) {
    s32 count;
    s32 temp_v0;
    s32 temp_a2;
    s32 var_s7;
    s32 v0;
    s32 var_fp;
    s32 var_s5;
    s32 var_s6;
    s32 var_s4;
    s32 var_s0;
    s32 var_s2;
    s32 var_s1;
    s32 var_s3;
    s32 v1;
    s32 hdr;
    s32 a0_arg;

    count = *(s32 *)arg0;
    var_s3 = arg1;
    temp_v0 = *(s32 *)(arg0 + (count * 4) + 4);
    temp_a2 = *(s32 *)(arg0 + 8);
    var_s7 = arg0;
    var_s7 += temp_v0;
    v0 = *(s32 *)(arg0 + 4);
    var_fp = arg0 + v0;
    var_s5 = temp_a2 - v0;

    if (count >= 2) {
        var_s6 = arg0 + temp_a2;
        var_s4 = *(s32 *)(arg0 + 0xC) - temp_a2;
    } else {
        var_s6 = 0;
        var_s4 = 0;
    }

    var_s0 = D_800A33E8[0];
    var_s2 = D_800A33E8[1];
    var_s1 = var_s3 + 0xC;

    if (var_s0 == -1) {
        if (var_s2 == var_s0) {
            var_s0 = var_s2;
        } else {
            var_s2 = 0;
        }
    } else if (var_s2 == -1) {
        var_s0 = 0;
    } else if (var_s0 == var_s2) {
        var_s0 = 0;
        var_s2 = 0;
    } else if (var_s0 < var_s2) {
        var_s0 = 0;
        var_s2 = 1;
    } else if (var_s2 < var_s0) {
        var_s0 = 1;
        var_s2 = 0;
    } else {
        func_80052C10();
    }

    hdr = ~var_s0;
    v1 = var_s3;
    var_s3 += 4;
    hdr = (u32)hdr >> 31;
    if (var_s2 >= 0) {
        hdr += 1;
    }
    *(s32 *)v1 = hdr;

    if (var_s0 >= 0) {
        *(s32 *)var_s3 = 2;
        var_s3 += 4;
        if (var_s0 == 0) {
            func_800520B8(var_fp, var_s1, var_s5);
            a0_arg = var_s1 + var_s5;
        } else {
            func_800520B8(var_s6, var_s1, var_s4);
            a0_arg = var_s1 + var_s4;
        }
        func_80045230(a0_arg);
        var_s1 += func_8005C2A8(var_s1, 2, var_s7);
    }

    if (var_s2 >= 0) {
        *(s32 *)var_s3 = 5;
        if (var_s2 == 0) {
            func_800520B8(var_fp, var_s1, var_s5);
            a0_arg = var_s1 + var_s5;
        } else {
            func_800520B8(var_s6, var_s1, var_s4);
            a0_arg = var_s1 + var_s4;
        }
        func_80045230(a0_arg);
        var_s1 += func_8005C2A8(var_s1, 5, var_s7);
    }
    return var_s1;
}
extern s16 D_80099CC2;
void func_80049E1C(void) {
    s16 val = -1;
    s32 i = 0x39;
    s16 *p = &D_80099CC2;
    do {
        *p = val;
        i--;
        p--;
    } while (i >= 0);
    D_800A324C = -1;
}

extern void *D_800A370C;
void func_80049E4C(void) {
    Unk80101DF0Record *p1 = &D_80101DF0;
    Unk80101DF0Record *p2 = &D_800FF638;
    p1->unk0 = 0x64;
    D_80101DF0.unk1 = 0;
    D_80101DF0.xf.rot.vx = 0;
    D_80101DF0.xf.rot.vy = 0;
    D_80101DF0.xf.rot.vz = 0;
    D_80101DF0.work.t[0] = 0;
    D_80101DF0.work.t[1] = 0;
    D_80101DF0.work.t[2] = 0;
    D_80101DF0.unkC = 0;
    D_80101DF0.unk8 = 5;
    func_800418D0((s32 *)p1);
    p2->unk0 = 0x65;
    D_800FF638.unk1 = 0;
    D_800FF638.xf.rot.vx = 0;
    D_800FF638.xf.rot.vy = 0;
    D_800FF638.xf.rot.vz = 0;
    D_800FF638.work.t[0] = 0;
    D_800FF638.work.t[1] = 0;
    D_800FF638.work.t[2] = 0;
    D_800FF638.unkC = 0;
    D_800FF638.unk8 = 2;
    func_800418D0((s32 *)p2);
    D_800A3708 = p1;
    D_800A370C = p2;
}
/* 0x800153F0: the 22-halfword record func_8004A09C unpacks (it walks it as u16). func_80049F4C copies
   it whole by assignment: the copy's run-time alignment test in the target bytes is the halfword
   type's alignment. */
extern const Unk800153F0Record D_800153F0;
extern void func_8004A09C(Unk800F62E0Rec *, u16 *);

void func_80049F4C(void) {
    Unk800153F0Record sp10;
    s32 i;
    sp10 = D_800153F0;
    for (i = 0; i < 8; i++) {
        func_8004A09C(&D_800F62E0[i], sp10.v);
    }
    SetColorMatrix(&D_800F62E0[0].cmat);
    SetBackColor(D_800F62E0[0].back[0], D_800F62E0[0].back[1], D_800F62E0[0].back[2]);
}
void func_8004A09C(Unk800F62E0Rec *arg0, u16 *arg1) {
    arg0->cmat.m[0][0] = *arg1++;
    arg0->cmat.m[1][0] = *arg1++;
    arg0->cmat.m[2][0] = *arg1++;
    arg0->cmat.m[0][1] = *arg1++;
    arg0->cmat.m[1][1] = *arg1++;
    arg0->cmat.m[2][1] = *arg1++;
    arg0->cmat.m[0][2] = *arg1++;
    arg0->cmat.m[1][2] = *arg1++;
    {
        s16 v48 = *arg1++;
        arg0->lmat.m[0][0] = 0;
        arg0->lmat.m[0][1] = 0;
        arg0->lmat.m[0][2] = 0;
        arg0->lmat.m[1][0] = 0;
        arg0->lmat.m[1][1] = 0;
        arg0->lmat.m[1][2] = 0;
        arg0->lmat.m[2][0] = 0;
        arg0->lmat.m[2][1] = 0;
        arg0->lmat.m[2][2] = 0;
        arg0->cmat.m[2][2] = v48;
    }
    arg0->light[0].pitch = *arg1++;
    arg0->light[0].yaw = *arg1++;
    arg0->light[0].on = *arg1++;
    arg0->light[1].pitch = *arg1++;
    arg0->light[1].yaw = *arg1++;
    arg0->light[1].on = *arg1++;
    arg0->light[2].pitch = *arg1++;
    arg0->light[2].yaw = *arg1++;
    arg0->light[2].on = *arg1++;
    func_8004A1FC(arg0);
    arg0->back[0] = *arg1++;
    arg0->back[1] = *arg1++;
    arg0->back[2] = *arg1;
    arg0->unk5C = *(arg1 + 1);
}
extern s32 rcos();
extern s32 rsin();
void func_8004A1FC(arg0) Unk800F62E0Rec *arg0; {
    s16 i;
    Unk800F62E0Light *p;
    s16 c0;
    s32 t;

    i = 0;
    do {
        p = &arg0->light[i];
        if (p->on != 0) {
            c0 = rcos(p->pitch);
            t = ((rsin(p->yaw) * c0) >> 12) * arg0->unk5C;
            arg0->lmat.m[i][0] = -t >> 12;
            t = rsin(p->pitch) * arg0->unk5C;
            arg0->lmat.m[i][1] = t >> 12;
            t = ((rcos(p->yaw) * c0) >> 12) * arg0->unk5C;
            arg0->lmat.m[i][2] = -t >> 12;
        } else {
            arg0->lmat.m[i][0] = 0;
            arg0->lmat.m[i][1] = 0;
            arg0->lmat.m[i][2] = 0;
        }
        i++;
    } while (i < 3);
}

/* Q65: this file's initialized small data (.sdata), in address order; values from the original EXE. */
s16 D_800A3248 = -1;
s16 D_800A324A = -1;
s32 D_800A324C = -1;
/* Q65: tentative definitions (COMMON) of the small data this file reaches gp-relative. */
s32 g_gpu_ot256_ptr;
