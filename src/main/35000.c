/* 44 game functions. .text 0x80044800 (ROM 0x35000). Start boundary: GP (a per-file gp split,
 * Q65). */
#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bb2.h"
#include "gte.h"

/* Declarations from the file this TU was split from (text1a_c.c). */
extern void func_80049E1C(void);
extern void func_80052C10(void);
extern s16 D_800963EE;
extern void func_80041430(s32, s32);
extern s32 func_8004019C(s32 *, s32);
extern s16 Judge[];
extern void math_RotMatrixZYX(SVECTOR *, MATRIX *);
extern s32 rcos(s32);
extern s32 rsin(s32);
void func_800433E4();
void func_80044010(s32 *p, s16 slot);
void func_80044100(s32 a0, s32 a1);
extern void func_800520B8(s32, s32, s32);
s32 func_8004428C(s32 *base, s16 *offsets);
s32 func_80044378(s32 src_base, s32 *dest_arr, s16 *frame_offsets);
extern void func_800417D0(s32 *);

void func_80044800(void) {
    SVECTOR sv;
    MATRIX mat;
    Unk800A9CF8Entry *rec;
    Unk800A9CF8Entry *ent;
    s32 i;
    s32 frame;
    s32 angle;
    s16 *scan;
    s16 cos_val;
    s16 sin_val;
    s32 cz;
    s32 sz;
    s32 cx;
    s32 sx;
    s32 last;
    s32 fade;
    s32 *list;

    rec = (Unk800A9CF8Entry *)D_800A9CF8.unkC;
    for (i = 0; i < D_800A9CF8.unk6; rec++, i++) {
        frame = rec->unk58;
        if (frame < 0) continue;
        if (frame < D_800A9CF8.unk2) {
            sv.vx = 0;
            angle = rec->unk5C;
            sv.vz = 0;
            scan = (s16 *)(D_800A9CF8.unk8 + frame * 12);
            sv.vy = angle;
            math_RotMatrixZYX(&sv, &mat);
            sv.vx = *scan++;
            sv.vy = *scan++;
            sv.vz = *scan++;
            rec->node.xf.rot.vx = *scan++;
            rec->node.xf.rot.vy = *scan++;
            rec->node.xf.rot.vz = *scan++;
            rec->node.unk6 = 0;
            func_800417D0((s32 *)rec);
            MulMatrix2(&mat, &rec->node.xf.mat);
            cos_val = Judge[(angle + 0x400) & 0xFFF];
            cz = cos_val * sv.vz;
            sin_val = Judge[angle & 0xFFF];
            sz = sin_val * sv.vz;
            cx = cos_val * sv.vx;
            sx = sin_val * sv.vx;
            rec->node.xf.mat.t[0] += (sz + cx) >> 12;
            rec->node.xf.mat.t[1] += sv.vy;
            rec->node.xf.mat.t[2] += (cz - sx) >> 12;
            frame++;
            if (frame >= D_800A9CF8.unk2) {
                rec->unk60 = 0x1000;
            }
            rec->unk58 = frame;
            if (D_800A9CF8.unk4 == 0x12) {
                /* FAKE: rec is reused (restored from ent below) for the paired
                 * game_GetCharData entry, an Unk800A6690Rec reached only through its
                 * node member (both records start with the node); a separate
                 * Unk80101DF0Record * local scores 70. */
                ent = rec;
                rec = (Unk800A9CF8Entry *)((Unk800A6690Rec *)D_800A9CF8.unk10 + i);
                last = D_800A9CF8.unk2 - 1;
                scan = (s16 *)(D_800A9CF8.unk8 + (frame + last) * 12);
                sv.vx = *scan++;
                sv.vy = *scan++;
                sv.vz = *scan++;
                rec->node.xf.rot.vx = *scan++;
                rec->node.xf.rot.vy = *scan++;
                rec->node.xf.rot.vz = *scan++;
                rec->node.unk6 = 0;
                func_800417D0((s32 *)rec);
                rec->node.xf.mat.t[0] += sv.vx;
                rec->node.xf.mat.t[1] += sv.vy;
                rec->node.xf.mat.t[2] += sv.vz;
                rec = ent;
            }
        } else {
            fade = rec->unk60 - 0x40;
            if (fade < 0) {
                fade = 0;
            }
            if (fade == 0 && D_800A9CF8.unk4 != 4) {
                rec->unk58 = -2;
            }
            rec->unk60 = fade;
        }
        list = (s32 *)D_800A3820;
        D_800A3820 = (s32)(list + 1);
        *list = (s32)rec;
    }
}

void func_80044B30(s32 a0, s32 a1) {
    Unk800A9CF8Entry *p;

    if (a0 >= D_800A9CF8.unk6) return;

    p = (Unk800A9CF8Entry *)D_800A9CF8.unkC + a0;
    if (p->unk58 != -1) return;

    switch (D_800A9CF8.unk4) {
    case 4:
        if (a0 == 0) {
            a1 = 0x800;
        }
        if (a0 == 1) {
            a1 = 0;
        }
        break;
    case 0x12:
        a1 = 0;
        break;
    }
    p->unk5C = a1;
    p->unk58 = 0;
    /* FAKE: p is reused for the paired game_GetCharData entry, an
     * Unk800A6690Rec reached only through its node member (both records start
     * with the node); a separate Unk80101DF0Record * local scores 4. */
    p = (Unk800A9CF8Entry *)((Unk800A6690Rec *)D_800A9CF8.unk10 + a0);
    p->node.unk2 = 1;
    p->node.unk4 = D_800A9CF8.unk0;
    p->node.xf.rot.vy = a1;
    p->node.unk0 = 0;
    p->node.unkC = 0;
    p->node.xf.rot.vx = 0;
    p->node.xf.rot.vz = 0;
    p->node.unk6 = 1;
    g_anim_func_table[p->node.unk8](&p->node.xf.rot, &p->node.xf.mat);
    if (D_800A9CF8.unk4 == 0x12) {
        p->node.work.t[0] = p->node.xf.mat.t[0];
        p->node.work.t[1] = p->node.xf.mat.t[1];
        p->node.work.t[2] = p->node.xf.mat.t[2];
    }
}
extern void func_80044100(s32, s32);
void func_80044C70(s32 a0) {
    func_80044100((s32)D_800A9CF8.unk0, a0);
    D_800A9CF8.unkC += a0;
    D_800A9CF8.unk8 += a0;
}
extern s32 rsin(s32);
extern s32 rcos(s32);
void func_80044CCC(s16 *a0, s16 *a1, s32 a2, s32 a3) {
    s32 sp18[3];
    s32 sp28[3];
    s32 angle;
    s32 radius;

    sp18[1] = a0[0];
    angle = a0[1];
    radius = a0[2];
    sp18[0] = (rsin(angle) * radius) >> 12;
    sp18[2] = (rcos(angle) * radius) >> 12;
    sp18[1] = -sp18[1];
    sp18[2] = -sp18[2];

    sp28[1] = a1[0];
    angle = a1[1];
    radius = a1[2];
    sp28[0] = (rsin(angle) * radius) >> 12;
    sp28[2] = (rcos(angle) * radius) >> 12;
    sp28[1] = -sp28[1];
    sp28[2] = -sp28[2];

    LoadAverage12(sp18, sp28, 0x1000 - a2, a2, a3);
}
void func_80044DE4(s16 *a0, s16 *a1, s32 a2, s32 a3) {
    s32 sp18[3];
    s32 sp28[3];
    sp18[0] = a0[0];
    a0++;
    sp18[1] = -a0[0];
    sp18[2] = -a0[1];
    sp28[0] = a1[0];
    a1++;
    sp28[1] = -a1[0];
    sp28[2] = -a1[1];
    LoadAverage12(sp18, sp28, 0x1000 - a2, a2, a3);
}
s32 func_80044E64(void) {
    return 0x25;
}
s32 func_80044E6C(void) {
    return 0x26;
}
extern void game_FrameLoop(void);
extern void cdrom_StartReadAt(s32, s32, s32, s32);

typedef struct { s16 start_sector; s16 length_sectors; } NdataInfEntry;
extern NdataInfEntry D_800963EC[];

void func_80044E74(s32 a0, s32 a1) {
    game_FrameLoop();
    cdrom_StartReadAt(0, a1, D_800963EC[a0].start_sector, D_800963EC[a0].length_sectors);
    game_FrameLoop();
}
void func_80044ED8(s32 a0, s32 a1) {
    if (a0 >= 0x1F) {
        a0 -= 0x1B;
    }
    if (!func_800450F4(a0, a1)) {
        func_80044E74(a0, a1);
    }
}


extern void func_80044E74(s32, s32);
void func_80044F30(s32 a0, s32 a1) {
    func_80044E74(a0 + 0x27, a1);
}
void func_80044F50(s32 a0, s32 a1, s32 a2) {
    if (!a0) {
        func_80044E74(a1 + 0x83, a2);
    } else {
        func_80044E74(a1 + 0x10C, a2);
    }
}
void func_80044F80(s32 a0, s32 a1) {
    func_80044E74(a0 + 0x4D, a1);
}
extern s32 D_800A3240;
extern char D_8001528C[];
s32 func_80044FA0(s32 a0, s32 a1) {
    s32 v0;
    s32 s0;

    s0 = a1 - (s32)func_80045814();
    if (s0 < 0) {
        goto set_from_table;
    }
    v0 = func_80045808();
    if (s0 >= v0) {
        goto set_from_table;
    }
    if (D_800A3240 != 0) {
        s0 = (s32)*(s16 *)((u8 *)&D_800963EE + a0 * 4) << 11;
    } else {
        s0 = 0;
    }
    v0 = func_800457DC();
    if (v0 < s0) {
        printf(D_8001528C, a0, s0 - v0);
        while (1) {
            func_800164F8();
        }
    }
    goto do_return;
set_from_table:
    s0 = (s32)*(s16 *)((u8 *)&D_800963EE + a0 * 4) << 11;
do_return:
    func_80044E74(a0, a1);
    return s0;
}
extern s16 D_800963EE;
extern s32 func_800457DC(void);
s32 func_80045080(s32 a0) {
    s32 val = (s32)*(s16 *)((u8 *)&D_800963EE + a0 * 4) << 11;
    return func_800457DC() - val;
}
/* Q65: this file's statics (.sbss, allocated per file in link order by PSYLINK), in address order. */
static s32 D_800A3398;
static s32 D_800A339C;  /* not named by any code or data: size from the gap */
static s32 D_800A33A0;
static s32 D_800A33A4;
static s32 D_800A33A8;
static s32 D_800A33AC;

void seq_Start(s32 a0, s32 a1) {
    func_80044E74(a0 + 0x25, a1);
    D_800A3398 = a1;
    D_800A3244 = 1;
}
extern s16 D_800973EC[];
s32 func_800450F4(s32 a0, s32 a1) {
    s32 *a2_ptr;
    s32 v1;
    s32 *v0_ptr;
    if (D_800A3244 == 0) {
        return 0;
    }
    a0 -= 0x1E;
    if ((u32)a0 >= 7) {
        return 0;
    }
    a0 = D_800973EC[a0];
    if (a0 == -1) {
        return 0;
    }
    a2_ptr = (s32 *)D_800A3398;
    if (*a2_ptr != 5) {
        D_800A3244 = 0;
        return 0;
    }
    v0_ptr = (s32 *)(a0 * 4 + (s32)a2_ptr + 4);
    v1 = *v0_ptr;
    {
        s32 a0_new = (s32)a2_ptr + (((u32)v1 >> 2) << 2);
        s32 a2_val = v0_ptr[1] - v1;
        func_800520B8(a0_new, a1, a2_val);
    }
    return 1;
}
void seq_Reset(void) {
    D_800A3244 = 0;
}
s32 seq_GetState(void) {
    return D_800A3244;
}
void func_800451A0(void) {
    cdrom_StartReadAt(1, (s32)D_800963EC, 0, 2);
}
void func_800451D0(void) {
    s32 i;
    for (i = 9; i >= 0; i--) {
        D_800EED10[i].id = -1;
    }
    D_800A33A0 = (s32)&D_800A9D10;
    D_800A33A4 = 0x45000;
    D_800A33AC = 0;
    D_800A33A8 = 0;
    func_80049E1C();
}
void func_80045230(s32 a0) {
    s32 v1;
    if (!a0) {
        a0 = D_800A33A0;
    }
    a0 -= (s32)&D_800A9D10;
    v1 = a0;
    if (a0 < D_800A33A8) {
        v1 = D_800A33A8;
    }
    D_800A33A8 = v1;
    if (a0 > 0x44FFF) {
        func_80052C10();
    }
}
void func_80045294(s32 a0, s32 a1) {
    s32 sum;
    s32 i;
    s32 src;
    s32 dst;
    s32 count;

    sum = 0;
    src = D_800EED10[a0].unk4;
    count = D_800A33AC;
    dst = src + a1;
    for (i = a0; i < count; i++) {
        sum += D_800EED10[i].amt;
    }
    if (sum != 0) {
        DrawSync(0);
        func_800520B8(src, dst, sum);
        for (i = a0; i < D_800A33AC; i++) {
            D_800EED10[i].unk4 += a1;
            if (D_800EED10[i].fn != 0) {
                D_800EED10[i].fn(D_800EED10[i].id, a1);
            }
        }
    }
    D_800A33A0 += a1;
    D_800A33A4 -= a1;
}
/* Remove the table entry whose id == a0: undo its contribution via
   func_80045294(index + 1, -amt), shift the following records down one slot,
   clear the freed tail slot, decrement the count. */
void func_800453E0(s32 a0) {
    s32 i;
    s32 j;
    s32 last;

    for (i = 0; i < D_800A33AC; i++) {
        if (D_800EED10[i].id == a0) {
            func_80045294(i + 1, -D_800EED10[i].amt);
            if (i < D_800A33AC - 1) {
                for (j = i + 1; j < D_800A33AC; j++) {
                    D_800EED10[j - 1] = D_800EED10[j];
                }
            }
            last = D_800A33AC - 1;
            D_800EED10[last].id = -1;
            D_800EED10[last].fn = 0;
            D_800A33AC = last;
            return;
        }
    }
}
void func_80045510(s32 a0, s32 a1) {
    s32 i;
    s32 diff;

    for (i = 0; i < D_800A33AC; i++) {
        if (D_800EED10[i].id == a0) {
            diff = a1 - D_800EED10[i].amt;
            if (diff == 0) return;
            func_80045294(i + 1, diff);
            D_800EED10[i].amt = a1;
            return;
        }
    }
}
s32 *func_800455AC(s32 a0) {
    Unk800EED10Entry *e;
    s32 idx;
    s32 ret;

    func_800453E0(a0);
    idx = D_800A33AC;
    e = &D_800EED10[idx];
    D_800A33AC = idx + 1;
    ret = D_800A33A0;
    e->unk4 = ret;
    e->id = a0;
    e->fn = 0;
    return (s32 *)ret;
}
void func_80045600(s32 a0, s32 a1) {
    s32 i = 0;
    s32 count = D_800A33AC;
    Unk800EED10Entry *e;
    if (i >= count) goto not_found;
    {
        Unk800EED10Entry *p = D_800EED10;
        do {
            /* FAKE: the cursor `p` and the tested entry `e` are two names for one
               walk; mechanism: the target keeps both live (`move a3,a2` at the
               loop head, `addiu a2,a3,16` in the back-edge slot, `sw ...,8(a3)`
               after the loop).  One pointer `e` walked by `e++` scores 7. */
            e = p;
            if (e->id == a0) goto found;
            i++;
            p = e + 1;
        } while (i < count);
    }
found:
    if (i < D_800A33AC) {
        s32 cur = D_800A33A0;
        s32 rest = D_800A33A4;
        /* FAKE: the parameter a0 (the id, dead here) reused for the entry's new
           amt; the target computes it into $a0 (`subu a0,a1,v0`).  A fresh local
           scores 17. */
        a0 = a1 - cur;
        cur += a0;
        rest -= a0;
        e->amt = a0;
        D_800A33A0 = cur;
        D_800A33A4 = rest;
        return;
    }
not_found:
    func_80052C10();
}
void func_80045694(s32 a0, s32 a1) {
    s32 i;
    s32 count = D_800A33AC;
    for (i = 0; i < count; i++) {
        if (D_800EED10[i].id == a0) {
            D_800EED10[i].fn = (void (*)(s16, s32))a1;
            return;
        }
    }
}
void func_800456F0(s32 a0) {
    s32 i;
    s32 count = D_800A33AC;
    for (i = 0; i < count; i++) {
        if (D_800EED10[i].id == a0) {
            D_800EED10[i].fn = 0;
            return;
        }
    }
}
s32 *func_8004574C(s32 arg0) {
    s32 i;
    s32 count = D_800A33AC;
    for (i = 0; i < count; i++) {
        if (D_800EED10[i].id == arg0) {
            return (s32 *)&D_800EED10[i];
        }
    }
    return NULL;
}
s32 func_800457A0(s32 a0) {
    s32 *v0 = ((s32 *(*)())func_8004574C)();
    if (v0) {
        return v0[1];
    }
    return 0;
}
void func_800457D4(void) {
}
s32 func_800457DC(void) {
    return D_800A33A4;
}
s32 func_800457E8(void) {
    return 0x45000 - D_800A33A8;
}
s32 func_800457FC(void) {
    return D_800A33A0;
}
s32 func_80045808(void) {
    return 0x45000;
}
void *func_80045814(void) {
    return &D_800A9D10;
}
extern void func_800520B8(s32, s32, s32);
void func_80045824(s32 a0, s32 a1, s32 a2) {
    func_80045230(a1 + a2);
    func_800520B8(a0, a1, a2);
}
extern void func_800400F8(s32);
extern void func_80044ED8(s32, s32);
extern s32 *func_800455AC(s32);
extern void func_80045AA4(s32, s32);

s16 *func_80045878(s32 a0, s32 a1, s32 a2) {
    s32 s3;
    s16 *v0;
    s16 *s1;
    s32 s0;
    v0 = (s16 *) func_8004574C(a0);
    if (v0 != 0) {
        s1 = (s16 *) ((s32 *) v0)[1];
    } else {
        s1 = (s16 *) func_800455AC(a0);
        func_80045600(a0, 0x1A88 + ((s32) s1));
        func_80045230(0);
        func_80045694(a0, (s32) (&func_80045AA4));
        s1[4] = -1;
        s1[3] = 0;
    }
    s3 = a0 + 3;
    if (func_8004574C(s3) != 0) {
        func_800400F8((s32) s1);
    }
    if (((func_8004574C(s3) != 0) && (s1[4] == a1)) && (s1[3] != (-2))) {
        s1[3] = 0;
    } else {
        *((s32 *) (((s32) s1) + 0x20)) = a2;
        s0 = (s32) func_800455AC(s3);
        *((s32 *) (((s32) s1) + 0x1C)) = s0;
        if (a2 != 0) {
            func_80044ED8(a1, a2);
        } else {
            func_80044ED8(a1, s0);
            s0 = s0 + ((((u32) ((s32 *) s0)[*((s32 *) s0)]) >> 2) << 2);
            func_80045230(s0);
        }
        func_80045600(s3, s0);
        func_80045694(s3, (s32) (&func_80045AA4));
        s1[3] = 1;
        *((s32 *) (((s32) s1) + 0x24)) = 0;
        *((s32 *) s1) = 0;
    }
    s1[2] = a0;
    s1[4] = a1;
    s1[10] = a0;
    s1[8] = a0;
    s1[11] = a0 + 3;
    *((s32 *) (((s32) s1) + 0x18)) = 0x8000;
    return s1;
}
void func_80045A28(s32 a0, s32 a1) {
    func_80045510(a0 + 3, a1);
    func_80045230(0);
}
extern void func_8005B644(void);
extern void func_800456F0(s32);
void func_80045A50(s32 a0) {
    s32 a0p3 = a0 + 3;
    func_8005B644();
    func_800456F0(a0p3);
    func_800456F0(a0);
    func_800453E0(a0p3);
    func_800453E0(a0);
}
extern void func_80044100(s32, s32);
extern void snd_VabFakeOpen(s32, s32);
void func_80045AA4(s32 a0, s32 a1) {
    s32 *ptr;
    s32 idx;
    if (a0 < 3) {
        func_80041430(a0, a1);
        return;
    }
    idx = a0 - 3;
    func_80044100(idx, a1);
    func_80044100(a0, a1);
    ptr = (s32 *)func_800457A0(idx);
    if (ptr == 0) return;
    ptr[7] = ptr[7] + a1;
    func_8004019C((s32)ptr, a1);
    if ((ptr[0] >> 1) & 1) {
        s32 val = *(s16 *)((u8 *)ptr + 4);
        idx = 3 * val + 1;
        snd_VabFakeOpen(a1, idx);
    }
}
extern s16 D_800993FC[];
extern void func_800480C0(s32, s32, s16, s16, s16, s16);
extern s32 func_80044378(s32, s32 *, s16 *);
extern s32 func_8004428C(s32 *, s16 *);
extern void func_80044010(s32 *, s16);
extern s32 func_80049C24(s32, s32);
extern void func_80044F50(s32, s32, s32);
extern s32 *func_800455AC(s32);
extern void func_80046048(s32, s32);

void func_80045B68(s32 arg0, s32 arg1, s16 *arg2, s32 arg3) {
    s16 sp18[120];
    s16 sp108[32];
    s32 *p;
    s32 *hdr;
    s32 last;
    s32 prev;
    s32 dl;
    s32 sec;
    s32 n;
    s32 i;
    s32 y;
    s16 *q;
    s16 *r;
    u16 t;
    s32 *dst;

    p = func_800455AC(6);
    if (arg3 != 0) {
        hdr = (s32 *)arg3;
    } else {
        hdr = p;
    }
    func_80044F50(arg0, arg1, (s32)hdr);

    last = (s32)hdr + (((u32)hdr[1] >> 2) << 2);
    n = hdr[0];
    sec = (s32)hdr + (((u32)hdr[n] >> 2) << 2);
    if (n >= 3) {
        prev = (s32)hdr + (((u32)hdr[n - 1] >> 2) << 2);
    } else {
        prev = 0;
    }

    for (i = 0; i < 32; i++) {
        sp108[i] = -1;
    }

    q = arg2;
    t = *q++;
    i = 0;
    while ((s16)t != -2) {
        if ((s16)t >= 0) {
            sp108[D_800993FC[i]] = 1;
        }
        i++;
        t = *q++;
    }

    n = 0;
    for (i = 0; i < 32; i++) {
        if (sp108[i] >= 0) {
            switch (n) {
            case 0:
                func_800480C0(sec, i, 0, 0, -0x180, 0xF0);
                break;
            case 1:
                func_800480C0(sec, i, 0, 0x40, -0x180, 0xF1);
                break;
            case 2:
                func_800480C0(sec, i, 0x80, 0, -0x160, 0xF0);
                break;
            case 3:
                func_800480C0(sec, i, 0x80, 0x40, -0x160, 0xF1);
                break;
            }
            sp108[i] = n;
            n++;
        }
    }

    DrawSync(0);

    q = sp18;
    if (arg0 == 0) {
        r = arg2;
    } else {
        r = arg2 + 0x33;
    }
    while (*r != -2) {
        if (*r >= 0) {
            *q++ = 1;
            *q++ = 1;
        } else {
            *q++ = -1;
            *q++ = -1;
        }
        r++;
    }
    *q = -2;

    if (arg3 != 0) {
        dst = (s32 *)((s32)p + 4);
        dl = func_80044378(last, dst, sp18);
        func_80044010(dst, 6);
    } else {
        dl = func_8004428C((s32 *)last, sp18);
        func_80044010((s32 *)last, 6);
    }
    func_80045230(dl);

    q = sp18;
    y = 0;
    i = 0;
    while (*q != -2) {
        if (*q >= 0) {
            switch (sp108[D_800993FC[i]]) {
            case 0:
                func_800433E4(6, y * 2, 0, 0, -0x180, 0xE8);
                break;
            case 1:
                func_800433E4(6, y * 2, 0, 0x40, -0x180, 0xE9);
                break;
            case 2:
                func_800433E4(6, y * 2, 0x80, 0, -0x160, 0xE8);
                break;
            case 3:
                func_800433E4(6, y * 2, 0x80, 0x40, -0x160, 0xE9);
                break;
            }
            y++;
        }
        q += 2;
        i++;
    }

    if (prev != 0) {
        *p = dl - (s32)p;
        dl = func_80049C24(prev, dl);
        func_80045230(dl);
    } else {
        *p = 0;
    }
    func_80045600(6, dl);
    func_80045694(6, (s32)&func_80046048);
}
extern void func_8005B6AC(void);
void func_80046020(void) {
    func_800453E0(6);
    func_8005B6AC();
}
extern void snd_VabFakeOpen(s32, s32);
void func_80046048(s32 a0, s32 a1) {
    s32 *s0;
    s32 count;
    func_80044100(6, a1);
    s0 = (s32 *)func_800457A0(6);
    if (s0 == NULL) {
        return;
    }
    {
        s32 off = *s0;
        if (off == 0) {
            return;
        }
        s0 = (s32 *)((u8 *)s0 + off);
    }
    count = *s0;
    count = count - 1;
    if (count == -1) {
        return;
    }
    s0++;
    do {
        snd_VabFakeOpen(a1, *s0++);
        count--;
    } while (count != -1);
}

/* Q65: this file's initialized small data (.sdata), in address order; values from the original EXE. */
s32 D_800A3240 = 1;
s32 D_800A3244 = 0;
