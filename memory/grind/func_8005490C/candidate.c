/* 0x84-byte (0x21-word) record filled by func_800198D0, which copies 0x21
 * words into its third argument; func_80023F08 passes two of these at
 * sp+0x18 and sp+0x9C (0x84 apart). Only the three leading halfwords are
 * read here. */
typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    s32 unk8[31];
} Unk8005490CPose;
extern s32 D_800A3250;
extern s16 *func_8003D7B4(s32);
extern void func_8001979C(s32, u32 *);
extern void func_8003D774(s32, s32);
extern void MulMatrix2(MATRIX *, MATRIX *);
extern void func_80042FA0(s32 *, s16 *);
extern void func_80042ED8(u16 *);
extern void func_800420D0(void);
extern void func_8004211C(void);
extern void camera_InitBoneData(void);
extern void stage_InitCollision(void);
extern void func_800198D0(s32, s32, Unk8005490CPose *, void *);
extern void func_80040D48(s32, s32, s32 *, s16 *, s16 *, s32);
extern void func_80040304(s32, s32);
extern void func_80046EA0(s32);
s32 func_8005490C(void) {
    /* FAKE: second C handle to the global ctrl block (pointer-alias family),
       as in func_80054604 above; mechanism: expand/cse address
       materialisation -- the pointer local seats %hi/%lo(D_800EFAE8) in one
       callee-saved base register ($s3) for the whole body, while the init
       block's D_800EFAE8.field accesses stay absolute %hi/%lo as in the
       target; lever-exhaustion: direct-global form measured 141 vs 0
       (memory/grind/func_8005490C/evidence.md,
       rejected/direct-global-no-pointer-local-141.c). */
    Unk800EFAE8Ctrl *s = &D_800EFAE8;
    VECTOR vec;
    Unk8005490CPose pose;
    s16 *v;
    s32 p;
    s32 i;

    if (s->unk0 < 0) {
        return 0;
    }
    if (s->unk0 == 0) {
        p = D_800EFAE8.unk2C;
        D_800EFAE8.unk30 = *(s32 *)(p + 0xC) + p;
        D_800EFAE8.unk34[0] = *(s32 *)(p + 0x10) + p;
        D_800EFAE8.unk34[1] = *(s32 *)(p + 0x14) + p;
        func_8003D774(D_800EFAE8.unk30, 0);
        for (i = 0; i < 2; i++) {
            if (*(s32 *)s->unk34[i] == D_800A3250) {
                s->unk34[i] = 0;
            }
            if (s->unk34[i] != 0) {
                func_8001979C(i, (u32 *)s->unk34[i]);
            }
        }
        s->unk3C[1] = 0;
        s->unk3C[0] = 0;
        if (s->unk4 & 0x80) {
            s->unk3C[0] = *(s32 *)(s->unk2C + 0x18) + s->unk2C;
        }
        if (s->unk4 & 0x40) {
            s->unk3C[1] = *(s32 *)(s->unk2C + 0x1C) + s->unk2C;
        }
        {
            s32 pl = func_8004153C(0);
            if (pl != 0) {
                func_8003FFC4(pl);
            }
        }
        {
            s32 pl = func_8004153C(1);
            if (pl != 0) {
                func_8003FFC4(pl);
            }
        }
    }
    v = func_8003D7B4(0);
    vec.vx = -v[0];
    vec.vy = v[1];
    vec.vz = -v[2];
    {
        s32 c = (&Judge)[(s->unk1E + 0x400) & 0xFFF];
        s32 sn = (&Judge)[s->unk1E & 0xFFF];
        s32 x = vec.vx;
        s32 z = vec.vz;
        vec.vz = (z * c - x * sn) >> 12;
        vec.vx = (z * sn + x * c) >> 12;
    }
    D_80101DF0.work.t[0] = vec.vx + s->unkC;
    D_80101DF0.work.t[1] = vec.vy + s->unk10;
    D_80101DF0.work.t[2] = vec.vz + s->unk14;
    D_80101DF0.xf.rot.vx = v[3];
    D_80101DF0.xf.rot.vy = v[4] + 0x800;
    D_80101DF0.xf.rot.vz = v[5];
    if (s->unk1E != 0) {
        MATRIX m;
        SVECTOR rot;
        rot.vx = 0;
        rot.vz = 0;
        rot.vy = s->unk1E;
        g_anim_func_table[0](&rot.vx, m.m[0]);
        g_anim_func_table[0](&D_80101DF0.xf.rot.vx, D_80101DF0.work.m[0]);
        MulMatrix2(&m, (MATRIX *)&D_80101DF0.work);
        func_80042FA0((s32 *)&D_80101DF0.work, &D_80101DF0.xf.rot.vx);
        func_80042ED8((u16 *)&D_80101DF0.work);
        D_80101DF0.xf.mat = D_80101DF0.work;
    } else {
        func_800418D0(&D_80101DF0);
    }
    s->unk24[0] = -D_80101DF0.xf.rot.vx;
    s->unk24[1] = -D_80101DF0.xf.rot.vy;
    s->unk24[2] = -D_80101DF0.xf.rot.vz;
    func_800420D0();
    func_8004211C();
    camera_InitBoneData();
    stage_InitCollision();
    func_8004A1FC(&D_800F62E0);
    func_8004A1FC(&D_800F62E0 + 0x60);
    func_8004A1FC(&D_800F62E0 + 0x180);
    for (i = 0; i < 2; i++) {
        if (s->unk34[i] != 0) {
            s32 ang;
            s32 obj = func_8004153C(i);
            func_800198D0(i, s->unk0, &pose, (void *)0x1F800000);
            vec.vy = pose.unk0;
            vec.vy = (vec.vy * *(s16 *)(obj + 0x12)) >> 12;
            ang = pose.unk2;
            vec.vx = ((&Judge)[ang & 0xFFF] * pose.unk4) >> 12;
            vec.vz = ((&Judge)[(ang + 0x400) & 0xFFF] * pose.unk4) >> 12;
            vec.vy = -vec.vy;
            vec.vz = -vec.vz;
            {
                s32 c = (&Judge)[(s->unk1E + 0x400) & 0xFFF];
                s32 sn = (&Judge)[s->unk1E & 0xFFF];
                s32 x = vec.vx;
                s32 z = vec.vz;
                vec.vz = (z * c - x * sn) >> 12;
                vec.vx = (z * sn + x * c) >> 12;
            }
            vec.vy += s->unk10;
            vec.vx += s->unkC;
            vec.vz += s->unk14;
            func_80040D48(i, 0, &vec.vx, &s->unk1C, &pose.unk0, s->unk10);
            if (s->unk44[i] >= 0) {
                func_80049718(s->unk44[i], (i * 2) | 0x8000, 0, 0);
                func_80049A2C(s->unk44[i], i * 2, 0);
            }
            if (s->unk48[i] >= 0) {
                func_80049A2C(s->unk48[i], (i * 2) | 1, 1);
            }
            if (s->unk3C[i] != 0) {
                func_80040304(i, (((u32 *)s->unk3C[i])[s->unk0 / 8] >> ((s->unk0 % 8) * 4)) & 0xF);
            }
        }
    }
    func_80046EA0(10000);
    s->unk0++;
    if (s->unk0 >= s->unk2) {
        s->unk0 = -1;
    }
    return 1;
}
