/* 103 game functions, among them rcnt_StartCnt1, rcnt_GetCnt1,
 * math_GrayscaleRgb555 and math_Grayscale3. .text 0x800460E4 (ROM 0x368E4).
 * Start boundary: GP (a per-file gp merge, Q67); compiled -G8 (Q89). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "bb2.h"
#include "include_asm.h"
#include "gte.h"

/* ---- merged section (owner ruling Q67: one original file) ---- */
extern void func_800477DC(s32);
extern s32 func_80047EC8(void);
extern void func_800481E8(s32, s32);
extern void func_800466C0(s32, s32);

#define ALIGN4(x) (((u32)(x) >> 2) << 2)
#define PTR_OFF(base, off) ((s32)((u8 *)(base) + (off)))

/* Q65: this file's statics (.sbss, allocated per file by PSYLINK), in
 * address order. */
static s32 D_800A33B0;
static s32 D_800A33B4;
static s32 D_800A33B8; /* not named by any code or data: size from the gap */
static s32 D_800A33BC;
static s32 D_800A33C0;
static s32 D_800A33C4; /* not named by any code or data: size from the gap */
static s16 D_800A33C8[2];
static s32 D_800A33CC; /* not named by any code or data: size from the gap */
static s16 *D_800A33D0;
static s32 D_800A33D4;
static s32 D_800A33D8;
static s32 D_800A33DC; /* not named by any code or data: size from the gap */
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

    s0 = (s32 *)func_800457A0(7);
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
                    func_8003EDC0((u16 *)PTR_OFF(s0, off), 7);
                }
                break;
            case 34:
                s7 = 0;
                break;
            }
            func_8003F168();
            if (s7 != 0) {
                return;
            }
        }
    }

    D_80099478 = (s16)stage_id;
    /* FAKE: s7 (the early-return flag above) reused as slot id 7 for
     * func_80045600 / func_80045694; the literal or a separate local scores 37.
     */
    s7 = 7;
    s0 = func_800455AC(7);

    if (arg1 != 0) {
        func_80044F30(stage_id, arg1);
    } else {
        func_80044F30(stage_id, (s32)s0);
    }

    if (arg1 != 0) {
        /* FAKE: s3 (the slot buffer's word count) reused for arg1's word count;
         * a separate local or the inline read scores 2. */
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

    D_8009947A = 0;
    /* FAKE: s1 = s4 routed through a delta-rebase detour that combine folds
       back (zero bytes); the extra refs lift s1's allocation priority above
       the s2 pointer, as in the target; plain s1 = s4: score 32 */
    s1 = (s32 *)((s32)s4 - (s32)s0);
    s1 = (s32 *)((s32)s1 + (s32)s0);
    switch (stage_id) {
    case 3: {
        /* FAKE: once-written pointer naming the stage header's last word; it
         * fixes the sched1 load/store order; s0[s3 - 1] inline: score 9 */
        s32 *hp = (s32 *)((s3 << 2) + (s32)s0) - 1;
        s1 = s2;
        s6 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 2]));
        s4 = (s32 *)((u8 *)s0 + ALIGN4(*hp));
        D_8009947A = 1;
        break;
    }
    case 4:
    case 7:
    case 18:
        s1 = s2;
        func_80044010((s32 *)PTR_OFF(s0, ALIGN4(s0[5])), 8);
        s1 =
            (s32 *)func_80044670((s16 *)PTR_OFF(s0, ALIGN4(s0[6])), 8, (s32)s1);
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
        D_8009947A = 1;
        break;
    case 34:
        s1 = s2;
        D_8009947A = 1;
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
    func_8003F168();
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

    if (D_8009947A == 0) {
        return;
    }
    s0 = (s32 *)func_800457A0(7);
    v0 = ((u32)s0[1] >> 2) << 2;
    a0 = (s32 *)((u8 *)s0 + v0);
    switch (D_80099478) {
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
    D_8009947A = 0;
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
    if (D_8009947A == 0) {
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
    D_80099478 = -1;
    D_8009947A = 0;
}

/* ---- merged section (owner ruling Q67: one original file) ---- */
extern void func_80047ED0(s32);

void func_800466C0(s32 a0, s32 a1) {
    s32 rounded;
    func_80044100(7, a1);
    func_8005441C(a1);
    rounded = (a1 / 4) * 4;
    D_800A33B0 += rounded;
    D_800A33B4 += rounded;
    switch (D_80099478) {
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

/* 0x800153F0: the 22-halfword record func_8004A09C unpacks (it walks it as
   u16). func_80049F4C copies it whole by assignment: the copy's run-time
   alignment test in the target bytes is the halfword type's alignment. */
typedef struct {
    u16 v[22];
} Unk800153F0Record;

/* ---- merged section (owner ruling Q67: one original file) ---- */
extern void func_80048F58(s32, s32);
extern void func_80048FFC(s32);
extern s32 func_800477E8(void);
extern void func_80047A90(void);
extern void func_80048B8C(s32);
extern void func_800460E4(s32, s32);
extern void func_8004668C(void);

extern s16 D_800A324A;

extern MATRIX D_800EEDB0;

extern s32 D_800EF800[];
extern u8 D_8009947C;
extern s16 D_800F6654;
extern MATRIX D_800EEDD0;

extern void func_80049E4C(void);
extern void func_80049F4C(void);
extern s16 g_color_mode;
extern s16 D_800F665A;
extern s16 D_800A3248;

void func_800468DC(s32 a0, s32 a1);

s32 func_80046780(void) { return D_800A33B0; }

s32 func_8004678C(void) { return D_800A33B4; }

s32 stage_GetId(void) { return D_80099478; }

s32 func_800467A8(void) { return D_8009947A; }

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
            func_80044F80(arg, (s32)s2);
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

void func_80046914(void) { func_800453E0(8); }

void snd_AllocSe(void) { func_800455AC(9); }

void func_80046954(void) {}

void func_8004695C(s32 a0) {
    func_80045230(a0);
    func_80045600(9, a0);
    func_80045694(9, (s32)func_80046954);
}

void func_800469A0(s32 a0) { func_80045510(9, a0); }

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

void func_80046A60(void) { func_800453E0(0xA); }

void func_80046A80(s32 a0, s32 a1) { func_80054FDC(a1); }

void func_80046AA0(void) {
    func_800415C4(0);
    func_800415C4(1);
    func_8004668C();
    func_80046020();
    func_80049E1C();
    func_80046914();
}

void rcnt_StartCnt1(void) {
    SetRCnt(0xF2000001, -1, 0x2000);
    StartRCnt(0xF2000001);
}

void rcnt_GetCnt1(void) { GetRCnt(0xF2000001); }

void func_80046B44(void) {
    /* FAKE: constant-holder locals set in source order ahead of the fence
       below so 1 seats in $v0 and 2 in $v1 before the store tail; literals:
       score 6 */
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
    /* FAKE: loop-note fence keeps the two constant sets out of the store
       tail; without it 1/2/0x23 collapse into one serialized $v0 */
    do {
    } while (0);
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

void func_80046BF4(Vec3i32 *a0, SVECTOR *a1, s32 a2) {
    s32 result[3];
    u16 count1;
    s32 trans[3];
    SVECTOR rot;
    MATRIX matrix_buf;

    g_draw_queue_cursor = g_draw_queue;
    {
        /* FAKE: count1 holds the count + 1 ahead of the D_800A3808 store; at
         * the D_800A38D6 store the loads reorder (score 16). */
        count1 = D_800A38D6 + 1;
        D_800A3808 = g_gpu_ot_ptr;
        D_800A38D6 = count1;
        D_800A378C = g_gpu_ot_ptr + 4;
    }

    if (a0 != 0) {
        D_80101DF0.xf.rot.vx = -a1->vx;
        D_80101DF0.xf.rot.vy = -a1->vy;
        D_80101DF0.xf.rot.vz = -a1->vz;

        trans[0] = 0;
        trans[1] = 0;
        trans[2] = -a2;

        rot.vx = -a1->vx;
        rot.vy = -a1->vy;
        rot.vz = -a1->vz;

        g_anim_func_table[0](&rot, &matrix_buf);

        ApplyMatrixLV(&matrix_buf, trans, result);

        {
            D_80101DF0.work.t[0] = result[0] + a0->x;
            D_80101DF0.work.t[1] = result[1] + a0->y;
            D_80101DF0.work.t[2] = result[2] + a0->z;
        }

        func_800418D0(&D_80101DF0);
        func_80047210();
        func_8003F274();

        D_800A33C0 = a2;
    }

    func_8004A1FC(&D_800F62E0[0]);
    func_8004A1FC(&D_800F62E0[1]);
    func_8004A1FC(&D_800F62E0[4]);
    func_800420D0();
    func_8003F568();
    func_8003F5CC();
}

void func_80046DA8(s32 a0) {
    if (a0 & 1) {
        func_80046EA0(D_800A33C0);
    }
    func_8004211C();
    func_800444BC();
}

s32 func_80046DE4(void) { return 0; }

void *func_80046DEC(s32 a0) {
    Unk80045878Obj *v0 = func_8004153C(a0);
    if (v0) {
        return v0->unk_1994;
    }
    return NULL;
}

void *func_80046E18(s32 a0) {
    Unk80045878Obj *v0 = func_8004153C(a0);
    if (v0) {
        return v0->unk_2C;
    }
    return NULL;
}

void func_80046E44(void) { D_800F6654 = 0; }

void func_80046E54(s32 a0) {
    if (a0) {
        D_800F6654 = 1;
    } else {
        D_800F6654 = 0;
    }
}

s32 func_80046E7C(void) { return D_800F6654; }

void func_80046E8C(void) { D_800A3790 = 0x23; }

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

void *func_80046F14(void) { return &D_8009947C; }

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
    sp10.m[0][1] = (s16) - ((s32)(D_800F62E0[arg0].lmat.m[0][0] << 12) /
                            D_800F62E0[arg0].lmat.m[0][1]);
    sp10.m[0][2] = 0;
    sp10.m[1][0] = 0;
    sp10.m[1][1] = 0;
    sp10.m[1][2] = 0;
    sp10.m[2][0] = 0;
    sp10.m[2][1] = (s16) - ((s32)(D_800F62E0[arg0].lmat.m[0][2] << 12) /
                            D_800F62E0[arg0].lmat.m[0][1]);
    sp10.m[2][2] = 0x1000;
    gte_MulMatrix0ClearTrans(&sp10, arg1, arg2);
    arg2->t[0] = arg1->t[0] + (((arg1->t[1] - arg3) * sp10.m[0][1]) >> 12);
    arg2->t[1] = arg3;
    arg2->t[2] = arg1->t[2] + (((arg1->t[1] - arg3) * sp10.m[2][1]) >> 12);
}

typedef struct {
    s32 w[8];
} Block32;

void func_80047210(void) {
    D_800EEDD0 = D_80101DF0.xf.mat;
    D_800EEDD0.m[1][0] >>= 1;
    D_800EEDD0.m[1][1] >>= 1;
    D_800EEDD0.m[1][2] >>= 1;
}

void *func_800472B0(void) { return &D_800EEDD0; }

void func_800472C0(Unk80101DF0Record *node) {
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

    math_RotMatrixYXZ((u16 *)&D_800A3708->xf.rot, &mtx);
    rot.vx = 0;
    rot.vy = 0;
    rot.vz = 0x1000;
    ApplyMatrix(&mtx, &rot, &dir);
    yaw = ratan2(dir.vx, dir.vz);
    dir.vz = ((s32)Judge[(yaw + 0x400) & 0xFFF] * dir.vz +
              (s32)Judge[yaw & 0xFFF] * dir.vx) >>
             12;
    D_800A33C8[0] = -ratan2(dir.vy, dir.vz);
    D_800A33C8[1] = yaw;
    return D_800A33C8;
}

void func_8004746C(void) { func_8004473C(); }

void func_8004748C(void) { func_80044800(); }

void func_800474AC(void) { func_80048F58(0, 0); }

void func_800474D0(void) { func_80048FFC(0); }

void func_800474F0(void) { func_8004473C(); }

void func_80047510(void) { func_80044800(); }

void func_80047530(void) { func_800477E8(); }

void func_80047550(void) { func_80047A90(); }

void func_80047570(void) {
    func_800472C0(&D_800EEDF0);
    D_800EEDF0.unk8 = 4;
}

void func_800475A4(void) {
    SVECTOR rot;
    VECTOR result;
    MATRIX buf1;
    MATRIX buf2;
    s16 angle;
    s32 computed;
    Unk80101DF0Record *base;

    if (func_800467A8() != 0) {
        return;
    }

    rot.vx = 0;
    rot.vy = 0;
    rot.vz = 0x6590;
    ApplyMatrix(&D_80101DF0.xf.mat, &rot, &result);

    angle = ratan2(result.vx, result.vz);

    computed = ((s32)Judge[(angle + 0x400) & 0xFFF] * result.vz +
                (s32)Judge[angle & 0xFFF] * result.vx) >>
               12;
    result.vz = computed;

    /* FAKE: second C handle to D_800EEDF0 (pointer-alias-fake-exception);
     * the direct spelling scores 6. */
    base = &D_800EEDF0;
    {
        /* FAKE: s16 temporary for the negated pitch; storing -ratan2() straight
         * into the field moves its negu and the addiu: score 3 */
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
        void **temp = g_draw_queue_cursor;
        g_draw_queue_cursor = temp + 1;
        *temp = base;
    }
}

void func_80047738(void) { func_80048F58(1, 0); }

void func_8004775C(void) { func_80048FFC(0); }

void func_8004777C(void) { func_8004473C(); }

void func_8004779C(void) { func_80044800(); }

void func_800477BC(void) {}

void func_800477C4(void) {}

void func_800477CC(void) {}

void func_800477D4(void) {}

void func_800477DC(s32 a0) { D_800A33D0 = (s16 *)a0; }

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
        /* FAKE: do-while(0) loop depth weights this body's refs, seating
         * t1val in $t1 and 0x2C00 in $t2 as in the target; without it t1val
         * (3 refs/76 insns) loses to t2 (5/150) */
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
            if (a0 < 0x10)
                goto inner;
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
        func_800417D0(node);
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
    if (0x11 > a0)
        goto loop3;

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
    void **temp;

    i = 0;
    /* FAKE: jb holds the Judge base ahead of the loop; indexing Judge directly
     * re-forms it in the loop (score 5). */
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
        /* FAKE: loop-note ref weighting seats a3 in $a3 and i in $t0;
         * without it: score 10 */
        a3 = 0;
    } while (0);
    do {
        /* FAKE: loop-note ref weighting seats pa2 in $a2 (shared with the
         * loop-1 Judge base); without it: score 13 */
        pa2 = pt2;
    } while (0);
    do {
        /* FAKE: loop-note ref weighting keeps pt1 ($t1) ahead of pt2 ($t2)
         * in allocation order; without it: score 8 */
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
        /* FAKE: loop tail duplicated into both arms (cross-jump re-merges it);
         * seats pa2 in $a2, a3 in $a3; without it: score 16 */
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

    temp = g_draw_queue_cursor;
    g_draw_queue_cursor = temp + 1;
    *temp = &D_800EF070;
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
            RotTransPers3(
                &v[0], &v[1], &v[2], &sxy0, &sxy1, &sxy2, &pflag, &flag);
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

s32 func_80047EC8(void) { return 0xD00; }

/* ---- merged section (owner ruling Q67: one original file) ---- */
extern s32 func_8005C2A8(Unk8005C2A8Pack *, s16, s32);

void func_80047ED0(s32 a0) { D_800A33D0 = (s16 *)((u8 *)D_800A33D0 + a0); }

void func_80047EE8(s32 arg0, s32 arg1) {
    /* FAKE: unwritten volatile pad reproduces the target's untouched 32-byte
     * locals region (sp+0x18..0x37) (phantom-frame pad; without it: score
     * 10). SOTN precedent:
     * `volatile u32 pad; // !FAKE:` (src/st/sel/2C048.c:564), `volatile u32
     * pad[4]; // FAKE` (src/st/sel/stream.c:80). */
    volatile u32 pre_pad[8];
    u32 *p;
    s32 saved;
    s16 new_var;
    s32 count;
    u32 v_off;
    unsigned int new_var2;
    p = (u32 *)arg0;
    saved = (s32)p;
    /* FAKE: dead store to a param; defeats cse2's substitution over
     * {arg0, p, saved} so the second pointer binds addu $s0,$s2,$v0, not $a0
     * (dead-store-fake-exception); without it: score 1 */
    arg0 = 0;
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    v_off = *p;
    p = (u32 *)(saved + ((v_off >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        count--;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            s32 first;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = a1v;
            new_var2 = word >> 2;
            first = saved + (new_var2 << 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            func_800482C8(first, new_var, a2v, a3v, v0v);
        } while ((count--) != 0);
    }
}

void func_80047FBC(s32 arg0, s32 arg1, s16 arg2, s16 arg3) {
    // !FAKE: phantom-frame-slot pad, without it: score 14
    // (no-new-park-categories): target reserves 32 locals bytes at
    // sp+0x18..sp+0x37 that no instruction touches
    volatile u32 pre_pad[8];
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = arg0;
    p = (u32 *)arg0;
    /* FAKE: defeats cse2 folding the {arg0, p, base_addr} equivalence
       class; without it: score 1 */
    arg0 = 0;
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
            func_800482C8(new_var, (s32)a1v + sx_arg2, (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg2, (s32)v0v + sx_arg3);
        } while ((count--) != 0);
    }
}

void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5) {
    // !FAKE: phantom-frame-slot pad (without it: score 20): target
    // reserves 32 locals bytes at sp+0x18..sp+0x37 that no instruction touches
    volatile u32 pre_pad[8];
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = arg0;
    p = (u32 *)arg0;
    // !FAKE: dead store to a parameter, as in func_80047FBC (without it:
    // score 16); defeats cse folding {arg0, p, base_addr}, which emits
    // one base copy instead of two
    arg0 = 0;
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
            func_800482C8(new_var, (s32)a1v + sx_arg2, (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg4, (s32)v0v + sx_arg5);
        } while ((count--) != 0);
    }
}

void func_800481E8(s32 arg0, s32 arg1) {
    /* Base-copy staging with the `arg0 = 0;` FAKE below makes GCC stage arg0
     * through $s0 first ($s0=$a0; $s2=$s0), as in the target. */
    /* FAKE: unwritten volatile pad reproduces the target's untouched 32-byte
     * locals region (sp+0x18..0x37), as the FAKE pads of func_80047EE8 /
     * func_80047FBC (phantom-frame pad; without it: score 10). SOTN
     * precedent: `volatile u32 pad;
     * // !FAKE:` (src/st/sel/2C048.c:564), `volatile u32 pad[4]; // FAKE`
     * (src/st/sel/stream.c:80). */
    volatile u32 pre_pad[8];
    u32 *p;
    u32 *base;
    s32 count;
    s32 a0_for_call;
    p = (u32 *)arg0;
    base = p;
    /* FAKE: dead store to a param -- breaks the $a0==base association:
     * without it GCC keeps arg0 live in $a0 and emits `addu $s0,$a0,$v0`;
     * with it the second pointer binds to base in $s2, as in the target;
     * without it: score 1. */
    arg0 = 0;
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
            /* compute early (target sched) */
            a0_for_call = (s32)base + (new_var2 << 2);
            p = (u32 *)(((s32)p) + 2); /* always advance (target delay slot) */
            if ((s32)a3v < 0x280) {
                v0v += 1;
            }
            func_800482C8(
                a0_for_call, (s32)a1v, (s32)a2v, (s32)a3v, (s32)(s16)v0v);
        } while ((count--) != 0);
    }
}

void func_800482C8(u8 *arg0, s16 arg1, s16 arg2, s16 arg3, u16 arg4) {
    RECT rect;
    u16 buf[512];
    u8 *p_alt;
    s32 flags;
    u32 dim;
    u32 dim2;
    s32 head = arg0[0];
    arg0 += 4;
    if (head != 0x10)
        return;
    flags = *(s32 *)arg0 & 8;
    arg0 += 4;
    if (flags != 0) {
        p_alt = arg0;
        arg0 = p_alt + (((u32) * (u32 *)p_alt >> 2) << 2);
        p_alt += 4;
    }
    arg0 += 8;
    rect.x = arg1;
    rect.y = arg2;
    dim = *(u32 *)arg0;
    rect.h = dim >> 16;
    rect.w = dim;
    LoadImage(&rect, (u32 *)(arg0 + 4));
    if (flags == 0)
        return;
    p_alt += 4;
    rect.x = arg3;
    rect.y = arg4;
    dim2 = *(u32 *)p_alt;
    p_alt += 4;
    rect.h = dim2 >> 16;
    rect.w = dim2;
    if (func_800486FC() != 0) {
        math_GrayscaleRgb555((u16 *)p_alt, rect.w, buf);
        LoadImage(&rect, (u32 *)buf);
        DrawSync(0);
        return;
    }
    LoadImage(&rect, (u32 *)p_alt);
}

void func_800483DC(s32 arg0, s32 arg1, s16 arg2, s16 arg3) {
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
    RECT rect;
    u16 buf[512];
    u32 dim;
    s32 flags;
    if (arg0[0] != 0x10)
        return;
    arg0 += 4;
    flags = *(s32 *)arg0;
    arg0 += 4;
    if ((flags & 8) == 0)
        return;
    arg0 += 8;
    rect.x = arg1;
    rect.y = arg2;
    dim = *(u32 *)arg0;
    arg0 += 4;
    rect.h = dim >> 16;
    rect.w = dim;
    if (func_800486FC() != 0) {
        math_GrayscaleRgb555((u16 *)arg0, rect.w, buf);
        LoadImage(&rect, (u32 *)buf);
        return;
    }
    LoadImage(&rect, (u32 *)arg0);
}

extern void func_800485EC();

s32 func_80048530(s32 arg0, s32 arg1, u32 arg2, s32 arg3) {
    s32 base, count, entry, a, b, c, d, off;
    off = ((s32 *)arg0)[arg1];
    base = arg0;
    /* FAKE: off + base operand order; base + off emits the addu operands
     * swapped (1 insn off) (single-order carve-out, or-tree-shape-shift) */
    arg0 = off + base;
    count = *(s32 *)arg0;
    arg0 += 4;
    if (arg2 >= (u32)count)
        return -1;
    arg0 += arg2 * 0xC;
    entry = *(s32 *)arg0;
    arg0 += 4;
    a = (s32) * (u16 *)arg0;
    arg0 += 2;
    b = (s32) * (u16 *)arg0;
    arg0 += 2;
    c = (s32) * (u16 *)arg0;
    arg0 += 2;
    d = (s32) * (u16 *)arg0;
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

void func_800485EC(tim, spr, x, y, cx, cy) u32 *tim;
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
        /* !FAKE: cancellation pair (F6): bumping tim stops cse rebasing the
         * pixel-block reads onto tim, so they keep p as base as in the
         * target; flow deletes both; without the pair: score 7 */
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
    if (func_800167AC()) {
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
            *arg2 = (u16)((((temp_a1 & (~0x7FFF)) + (temp_a3 << 0xA)) +
                           (temp_a3 << 5)) +
                          temp_a3);
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

void func_80048864(
    s32 mode, s32 sx, s32 sy, s32 w, s32 mr, s32 mg, s32 mb, s32 dx, s32 dy) {
    u16 buf[256];
    u16 out[256];
    RECT rect;
    u16 *src;
    u16 *dst;
    s32 i;
    u16 p;
    s32 r, g, b, a;

    DrawSync(0);
    rect.x = sx;
    rect.y = sy;
    rect.w = w;
    rect.h = 1;
    StoreImage(&rect, (u32 *)buf);
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
    rect.x = dx;
    rect.y = dy;
    LoadImage(&rect, (u32 *)out);
    DrawSync(0);
}

void func_80048A7C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    func_80048864(0, arg0, arg1, arg2, arg3, arg4, arg5, arg0, arg1);
}

extern s32 *func_800467B8(s32); /* matches the definition above */
extern void func_800468B0(s32);
extern u8 D_80099BCC;

s32 func_80048AD0(s32 arg0) {
    Unk80045878Obj *temp_v0;
    s32 sound;
    u8 *base;
    s32 delta;
    u8 *p;
    u8 *q;

    temp_v0 = func_8004153C(arg0);
    if (temp_v0 == 0)
        return 0;
    D_800A33E0 = arg0;
    sound = (&D_80099BCC)[temp_v0->unk_08];
    if (sound == 0xFF)
        return 0;
    base = (u8 *)func_800467B8(sound);
    p = base + ((*(u32 *)(base + 8) >> 2) << 2);
    /* FAKE: delta taken before the loop (subu a2 ahead of it); computed at the
     * call, p's registers shift (score 19) */
    delta = (s32)(p - base);
    D_800A33E4 = (s32)p;
    /* FAKE: q = p + 0xA, the target's second cursor (addiu a1,v1,10; stores at
     * -8 / -6 / -9 / 0 from it); through p the cursor goes (score 11) */
    q = p + 0xA;
    /* FAKE: the record counter reuses `sound`, inheriting its $a0
       preference from func_800467B8's argument; a separate counter swaps
       $a0/$a2 with delta: score 6 */
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

void func_80048B8C(s32 a0) { D_800A33E4 += a0; }

extern u32 *g_gpu_ot256_ptr;
extern u32 g_gpu_ot256_db[][256];
extern s16 D_80099C14[];

void func_80048BA4(s32 arg0, s32 arg1, s32 arg2) {
    MATRIX mtx;
    SVECTOR rot;
    s32 index;
    s32 scale;
    s32 old;
    s16 *indices;
    void **list;
    MATRIX **player;
    Unk80045878Node *node;

    player = func_80046DEC(D_800A33E0);
    if (player == 0) {
        return;
    }
    if (arg1 >= 6) {
        arg1 = -1;
    }

    rot.vx = 0x1770;
    rot.vy = 0;
    rot.vz = 0;
    /* FAKE: constant holder for the 0x1770 radius; the literal scores 20 */
    scale = 0x1770;
    rot.vz = ((s32)Judge[arg0 & 0xFFF] * scale) >> 12;
    rot.vx = ((s32)Judge[(arg0 + 0x400) & 0xFFF] * scale) >> 12;
    node = (Unk80045878Node *)D_800A33E4;
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
    node->node.xf.mat = *player[index];
    list = g_draw_queue_cursor;
    g_draw_queue_cursor = list + 1;
    *list = node;
    node++;
test_index:
    index = *indices;
    indices++;
    if (index >= 0) {
        goto copy_index;
    }
    if (arg1 >= 0) {
        node->node.xf.mat = *player[18];
        list = g_draw_queue_cursor;
        node->node.unk2 = arg1 + 0xF;
        g_draw_queue_cursor = list + 1;
        *list = node;
        node++;
    }
    if (arg2 != 0) {
        node->node.xf.mat = *player[19];
        list = g_draw_queue_cursor;
        node->node.unk2 = 0x15;
        g_draw_queue_cursor = list + 1;
        *list = node;
    }

    g_gpu_ot256_ptr = g_gpu_ot256_db[D_800A36AC & 1];
    ClearOTagR(g_gpu_ot256_ptr, 0x100);
    old = D_800A378C[0];
    D_800A378C[0] = (u32)(g_gpu_ot256_ptr + 0xFF) & 0xFFFFFF;
    *g_gpu_ot256_ptr = old;
}

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
        /* FAKE: strip two's height taken as an s16 at the top of the level;
         * rect.h = phase directly, an s32 copy or a (s16) cast drops the t1
         * copy: score 57 */
        s16 h2 = phase;
        rect.x = x + xf;
        rect.y = sy;
        rect.w = w;
        rect.h = h1;
        SetDrawMove(p, &rect, nx, ny + phase);
        addPrim(&D_800A378C[0xFFF], p);
        p++;
        rect.y = sy + h1;
        rect.h = h2;
        SetDrawMove(p, &rect, nx, ny);
        addPrim(&D_800A378C[0xFFF], p);
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
        /* FAKE: loop notes keep the D_80099CC9 lbu below the first sh (target
           has the unfilled load-delay nop) and keep the shared 1 cached in $v1;
           without it: score 11 */
        do {
        } while (0);
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

s32 func_8004954C(s32 arg0, s32 arg1, s32 arg2) {
    s32 sum = 0;
    s32 i;
    for (i = 0; i < arg1; i++) {
        /* FAKE: do-while(0) loop-note ref weighting flips the sum/i allocno
           priority so sum seats in $v1 and i in $a3 (matches target); without
           it: score 6. */
        do {
            sum += arg0;
            arg0 -= 1;
        } while (0);
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
    /* FAKE: `i` carries both loop counters and the computed total: only a
       pseudo crossing a call gets a call-saved reg, so sharing puts the
       counter in $s0 as in the target; a separate `total`: 12 insns differ */
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
            *p = (s16)rank;
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

void func_80049710(void) {}

/* Appends one or two 0x68-byte draw nodes at g_prim_buf_cursor for animation
 * entry arg0 and queues each on g_draw_queue. The first
 * node (type 0) takes its rotation and position from rot_in / pos (flags == 1)
 * or from node 19 + (flags & 1) of player flags >> 1's model object, whose
 * offset it scales in place by the object's unk_12 and into which it copies the
 * new matrix back; the second (type 3, parent = the first) follows unless flags
 * is still 1. */
void func_80049718(s32 arg0, s32 flags, s32 *pos, s16 *rot_in) {
    SVECTOR ofs;
    s32 val58;
    Unk80045878Obj *player;
    Unk80045878Node *obj;
    Unk80045878Node *part;
    /* FAKE: named intermediate (no-new-park-categories entry 6) set before
     * the call so the andi lands ahead of the copy of its result, as in the
     * target; written in the expression or after the call it takes $v0: score
     * 5 */
    s32 side;
    if (D_800EF980[arg0] < 0) {
        func_80052C10();
    }
    obj = (Unk80045878Node *)g_prim_buf_cursor;
    /* FAKE: dead store: the 0 is never read (flags == 1 skips the second
     * node) but gives the target's `move s5,zero`: score 1
     * (dead-store-fake-exception) */
    val58 = 0;
    obj->node.unk0 = 0;
    obj->node.unk1 = 0;
    obj->node.unk2 = D_800EF980[arg0] * 2;
    obj->node.unk4 = 6;
    obj->node.unk8 = 0;
    obj->node.unkC = 0;
    obj->node.unkA = 4;
    if (flags != 0) {
        if (flags == 1) {
            obj->node.xf.rot.vx = rot_in[0];
            obj->node.xf.rot.vy = rot_in[1];
            obj->node.xf.rot.vz = rot_in[2];
            g_anim_func_table[0](&obj->node.xf.rot, &obj->node.xf.mat);
            obj->node.xf.mat.t[0] = pos[0];
            obj->node.xf.mat.t[1] = pos[1];
            obj->node.xf.mat.t[2] = pos[2];
        } else {
            /* FAKE: flags rewritten in place, as SOTN reuses a parameter
             * (Q51); a local copy flips the $s0 allocation order: score 63 */
            /* SOTN: src/st/lib/e_shop.c:4621 @aa53500 */
            flags &= 0x7FFF;
            side = flags & 1;
            player = func_8004153C(flags >> 1);
            part = &player->unk_2C[19 + side];
            part->node.work.t[0] =
                (part->node.work.t[0] * player->unk_12) >> 12;
            part->node.work.t[1] =
                (part->node.work.t[1] * player->unk_12) >> 12;
            part->node.work.t[2] =
                (part->node.work.t[2] * player->unk_12) >> 12;
            MulMatrix0(&player->unk_2C[0].node.xf.mat, &part->node.work,
                       &obj->node.xf.mat);
            ofs.vx = part->node.work.t[0];
            ofs.vy = part->node.work.t[1];
            ofs.vz = part->node.work.t[2];
            ApplyMatrix(
                &part->node.unkC->xf.mat, &ofs, (VECTOR *)obj->node.xf.mat.t);
            obj->node.xf.mat.t[0] =
                obj->node.xf.mat.t[0] + part->node.unkC->xf.mat.t[0];
            obj->node.xf.mat.t[1] =
                obj->node.xf.mat.t[1] + part->node.unkC->xf.mat.t[1];
            obj->node.xf.mat.t[2] =
                obj->node.xf.mat.t[2] + part->node.unkC->xf.mat.t[2];
            /* SOTN: src/st/lib/e_shop.c:4625 @aa53500 */
            flags |= 0x8000;
            part->node.xf.mat = obj->node.xf.mat;
            val58 = player->unk_1A84;
        }
        {
            void **list = g_draw_queue_cursor;
            g_draw_queue_cursor = list + 1;
            *list = obj;
        }
        obj++;
        if (flags != 1) {
            /* FAKE: named intermediate (no-new-park-categories entry 6) reads
             * the table before the node's fields are written, as in the target;
             * storing it directly reads it last: score 22 */
            s32 frame = D_800EF980[arg0];
            void **list;
            obj->node.unk0 = 3;
            obj->node.unk1 = 0;
            /* FAKE: unk58 stored through a pointer; the member store lets sched
             * sink it below the g_draw_queue_cursor load (score 2). */
            {
                s32 *p58 = &obj->unk58;
                *p58 = val58;
            }
            list = g_draw_queue_cursor;
            obj->node.unkC = &obj[-1].node;
            obj->node.unk6 = 1;
            obj->node.unk8 = 0;
            obj->node.unkA = 0;
            obj->node.unk4 = 6;
            obj->node.unk2 = frame * 2 + 1;
            g_draw_queue_cursor = list + 1;
            *list = obj;
            obj++;
        }
        g_prim_buf_cursor = (u32 *)obj;
    }
}

extern s16 D_80099D3C[];

/* func_80049A2C: `volatile u32 pre_pad[2]` is a labelled FAKE (phantom-frame
 * pad) giving the target's frame signature (.frame $sp,48, vars= 8, regs= 5).
 * The only fold-capable symbol (D_80099D3C) cannot host the vars region
 * without costing a sixth callee-saved register, and the function is
 * loopless, so no back-edge carrier exists; hence the FAKE pad. SOTN
 * precedent: `volatile char pad[8] //! FAKE`. */
void func_80049A2C(s32 arg0, s32 arg1, s32 arg2) {
    // !FAKE: phantom-frame-slot pad, without it: score 12
    // (no-new-park-categories): target reserves 8 locals bytes at
    // sp+0x10..sp+0x17 that no instruction touches
    volatile u32 pre_pad[2];
    u8 *new_var6;
    u8 temp_v1;
    s16 *new_var8;
    s16 *p_anim;
    s16 new_var2;
    s16 *src;
    Unk80045878Node *obj;
    Unk80045878Obj *player;
    s16 a1_val;
    void **list;

    /* FAKE: the table base in its own holder; indexing D_80099CC8 directly
       emits the row shift ahead of the base load (score 2). */
    new_var6 = D_80099CC8;
    { temp_v1 = (new_var6 + (arg0 * 2))[arg2]; }
    if (temp_v1 == 0xFF) {
        return;
    }
    /* FAKE: the table base in its own holder; &D_800EF980[temp_v1] emits the
       shift ahead of the base load and swaps the addu operands (score 4). */
    new_var8 = D_800EF980;
    p_anim = new_var8 + temp_v1;
    if ((*p_anim) < 0) {
        func_80052C10();
    }
    player = func_8004153C(arg1 >> 1);
    obj = (Unk80045878Node *)g_prim_buf_cursor;
    obj->node.unk0 = 0;
    obj->node.unk1 = 0;
    /* FAKE: a1_val reads *p_anim ahead of each node's stores; read at the unk2
       stores the lh moves down to them (score: first 29, second 6, both 35). */
    a1_val = (*p_anim) * 2;
    obj->node.unk4 = 6;
    obj->node.unk8 = 0;
    obj->node.unkA = 4;
    obj->node.unk2 = a1_val;
    src = &D_80099D3C[(arg1 & 1) * 6];
    obj->node.work.t[0] = ((*src) * player->unk_12) >> 12;
    src++;
    obj->node.work.t[1] = ((*src) * player->unk_12) >> 12;
    src++;
    obj->node.work.t[2] = ((*src) * player->unk_12) >> 12;
    src++;
    obj->node.xf.rot.vx = *src;
    src++;
    obj->node.xf.rot.vy = *src;
    /* FAKE: rot.vz's value read into a local ahead of the parent store; read at
       its own store it scores 8. */
    new_var2 = src[1];
    obj->node.unkC = &player->unk_2C[12].node;
    obj->node.unk6 = 0;
    obj->node.xf.rot.vz = new_var2;
    func_800417D0(&obj->node);
    list = g_draw_queue_cursor;
    g_draw_queue_cursor = list + 1;
    *list = obj;
    obj++;
    a1_val = (*p_anim) * 2;
    obj->node.unk0 = 3;
    obj->node.unkC = &obj[-1].node;
    obj->node.unk1 = 0;
    obj->node.unk8 = 0;
    obj->node.unk6 = 1;
    obj->node.unkA = 0;
    obj->node.unk4 = 6;
    obj->node.unk2 = a1_val + 1;
    /* FAKE: unk58 stored through a pointer; the member store lets sched lift
       the g_draw_queue_cursor reload above the node's stores (score 17). */
    {
        s32 *p58 = &obj->unk58;
        *p58 = player->unk_1A84;
    }
    list = g_draw_queue_cursor;
    g_draw_queue_cursor = list + 1;
    *list = obj;
    g_prim_buf_cursor = (u32 *)(obj + 1);
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
    s32 *tbl = (s32 *)arg0;

    count = tbl[0];
    var_s3 = arg1;
    /* FAKE: temp_v0 reads the word here; read at its use, arg0 is copied to s6
     * and the saved registers shift (score 11). */
    temp_v0 = tbl[count + 1];
    temp_a2 = tbl[2];
    /* FAKE: var_s7 built in two steps; `var_s7 = arg0 + temp_v0` swaps s7 / s8
     * (score 6). */
    var_s7 = arg0;
    var_s7 += temp_v0;
    v0 = tbl[1];
    var_fp = arg0 + v0;
    var_s5 = temp_a2 - v0;

    if (count >= 2) {
        var_s6 = arg0 + temp_a2;
        var_s4 = tbl[3] - temp_a2;
    } else {
        var_s6 = 0;
        var_s4 = 0;
    }

    var_s0 = D_800A33E8[0];
    var_s2 = D_800A33E8[1];
    var_s1 = var_s3 + 0xC;

    if (var_s0 == -1) {
        if (var_s2 == var_s0) {
            /* FAKE: a no-op copy (both are -1); the target's `move s0,s2`;
             * without it the test inverts (score 4). */
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

    /* FAKE: hdr built in two steps (one expression emits the nor after the slot
     * copy, score 2); v1 holds the header slot so var_s3 steps before the store
     * (stored through var_s3, the step follows the store, score 4). */
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
        var_s1 += func_8005C2A8((Unk8005C2A8Pack *)var_s1, 2, var_s7);
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
        var_s1 += func_8005C2A8((Unk8005C2A8Pack *)var_s1, 5, var_s7);
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
    func_800418D0(p1);
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
    func_800418D0(p2);
    D_800A3708 = p1;
    D_800A370C = p2;
}

/* 0x800153F0: the 22-halfword record func_8004A09C unpacks. */
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
    SetBackColor(
        D_800F62E0[0].back[0], D_800F62E0[0].back[1], D_800F62E0[0].back[2]);
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

void func_8004A1FC(arg0) Unk800F62E0Rec *arg0;
{
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

/* Q65: this file's initialized small data (.sdata), in address order. */
s16 D_800A3248 = -1;
s16 D_800A324A = -1;
s32 D_800A324C = -1;
/* Q65: tentative definitions (COMMON) of small data reached gp-relative. */
u32 *g_gpu_ot256_ptr;
