typedef struct {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ s16 unk20;
    /* 0x22 */ s16 unk22;
    /* 0x24 */ Unk80101DF0Rot unk24;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34[2];
    /* 0x3C */ s32 unk3C[2];
    /* 0x44 */ s16 unk44[2];
    /* 0x48 */ s16 unk48[2];
} Ctrl2;
extern s32 D_800A3250;
extern s32 *func_8003D7B4(s32);
extern void func_8001979C(s32, s32);
extern void func_8003D774(s32, s32);
extern void MulMatrix2(void *, void *);
extern void func_80042FA0(void *, void *);
extern void func_80042ED8(void *);
extern void func_800420D0(void);
extern void func_8004211C(void);
extern void camera_InitBoneData(void);
extern void stage_InitCollision(void);
extern void func_800198D0(s32, s32, void *, void *);
extern void func_80040D48(s32, s32, void *, void *, void *, s32);
extern void func_80040304(s32, s32);
extern void func_80046EA0(s32);
extern s16 Judge;
extern u8 D_800F62E0;
extern s32 D_800F66A0[];
typedef struct { s32 vx, vy, vz; } Vec3L;
typedef struct { s16 vx, vy, vz, pad; } Vec3S;
typedef struct { s16 unk0; s16 unk2; s16 unk4; s16 unk6; s32 unk8[31]; } Pose84;
s32 func_8005490C(void) {
    Ctrl2 *s = (Ctrl2 *)&D_800EFAE8;
    Vec3L vec;
    Pose84 sv;
    s16 *v;
    s32 p;
    s32 i;
    s32 obj;
    s32 ang;
    s32 nz;

    if (s->unk0 < 0) {
        return 0;
    }
    if (s->unk0 == 0) {
        p = D_800EFAE8.unk2C;
        D_800EFAE8.unk30 = *(s32 *)(p + 0xC) + p;
        D_800EFAE8.unk34 = *(s32 *)(p + 0x10) + p;
        D_800EFAE8.unk38 = *(s32 *)(p + 0x14) + p;
        func_8003D774(D_800EFAE8.unk30, 0);
        for (i = 0; i < 2; i++) {
            if (*(s32 *)s->unk34[i] == D_800A3250) {
                s->unk34[i] = 0;
            }
            if (s->unk34[i] != 0) {
                func_8001979C(i, s->unk34[i]);
            }
        }
        s->unk3C[1] = 0;
        s->unk3C[0] = 0;
        if (s->unk4 & 0x80) {
            p = s->unk2C;
            s->unk3C[0] = *(s32 *)(p + 0x18) + p;
        }
        if (s->unk4 & 0x40) {
            p = s->unk2C;
            s->unk3C[1] = *(s32 *)(p + 0x1C) + p;
        }
        {
            s32 veh = func_8004153C(0);
            if (veh != 0) {
                func_8003FFC4(veh);
            }
        }
        {
            s32 veh = func_8004153C(1);
            if (veh != 0) {
                func_8003FFC4(veh);
            }
        }
    }
    v = (s16 *)func_8003D7B4(0);
    vec.vx = -v[0];
    vec.vy = v[1];
    vec.vz = -v[2];
    {
        s32 c = (&Judge)[(s->unk1E + 0x400) & 0xFFF];
        s32 sn = (&Judge)[s->unk1E & 0xFFF];
        s32 x = vec.vx;
        s32 z = vec.vz;
        nz = (z * c - x * sn) >> 12;
        vec.vx = (z * sn + x * c) >> 12;
        vec.vz = nz;
    }
    D_80101DF0.work.t[0] = vec.vx + s->unkC;
    D_80101DF0.work.t[1] = vec.vy + s->unk10;
    D_80101DF0.work.t[2] = vec.vz + s->unk14;
    D_80101DF0.xf.rot.vx = v[3];
    D_80101DF0.xf.rot.vy = v[4] + 0x800;
    D_80101DF0.xf.rot.vz = v[5];
    if (s->unk1E != 0) {
        Unk80101DF0Mat m;
        Vec3S rot;
        rot.vx = 0;
        rot.vz = 0;
        rot.vy = s->unk1E;
        ((void (*)(void *, void *))D_800F66A0[0])(&rot, &m);
        ((void (*)(void *, void *))D_800F66A0[0])(&D_80101DF0.xf.rot, &D_80101DF0.work);
        MulMatrix2(&m, &D_80101DF0.work);
        func_80042FA0(&D_80101DF0.work, &D_80101DF0.xf.rot);
        func_80042ED8(&D_80101DF0.work);
        D_80101DF0.xf.mat = D_80101DF0.work;
    } else {
        func_800418D0(&D_80101DF0);
    }
    s->unk24.vx = -D_80101DF0.xf.rot.vx;
    s->unk24.vy = -D_80101DF0.xf.rot.vy;
    s->unk24.vz = -D_80101DF0.xf.rot.vz;
    func_800420D0();
    func_8004211C();
    camera_InitBoneData();
    stage_InitCollision();
    func_8004A1FC(&D_800F62E0);
    func_8004A1FC(&D_800F62E0 + 0x60);
    func_8004A1FC(&D_800F62E0 + 0x180);
    for (i = 0; i < 2; i++) {
        if (s->unk34[i] != 0) {
            obj = func_8004153C(i);
            func_800198D0(i, s->unk0, &sv, (void *)0x1F800000);
            vec.vy = sv.unk0;
            vec.vy = (vec.vy * *(s16 *)(obj + 0x12)) >> 12;
            ang = sv.unk2;
            vec.vx = ((&Judge)[ang & 0xFFF] * sv.unk4) >> 12;
            vec.vz = ((&Judge)[(ang + 0x400) & 0xFFF] * sv.unk4) >> 12;
            vec.vy = -vec.vy;
            vec.vz = -vec.vz;
            {
                s32 c = (&Judge)[(s->unk1E + 0x400) & 0xFFF];
                s32 sn = (&Judge)[s->unk1E & 0xFFF];
                s32 x = vec.vx;
                s32 z = vec.vz;
                nz = (z * c - x * sn) >> 12;
                vec.vx = (z * sn + x * c) >> 12;
                vec.vz = nz;
            }
            vec.vy += s->unk10;
            vec.vx += s->unkC;
            vec.vz += s->unk14;
            func_80040D48(i, 0, &vec, &s->unk1C, &sv, s->unk10);
            if (s->unk44[i] >= 0) {
                func_80049718(s->unk44[i], (i * 2) | 0x8000, 0, 0);
                func_80049A2C(s->unk44[i], i * 2, 0);
            }
            if (s->unk48[i] >= 0) {
                func_80049A2C(s->unk48[i], (i * 2) | 1, 1);
            }
            if (s->unk3C[i] != 0) {
                func_80040304(i, (((u32 *)s->unk3C[i])[s->unk0 / 8] >> ((s16)(s->unk0 % 8) * 4)) & 0xF);
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
