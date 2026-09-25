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
    /* 0x24 */ s16 unk24[4];
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34[2];
    /* 0x3C */ s32 unk3C[2];
    /* 0x44 */ s16 unk44[2];
    /* 0x48 */ s16 unk48[2];
} Ctrl2;
#define D_800EFAE8 (*(Ctrl2 *)&D_800EFAE8)
#define Unk800EFAE8Ctrl Ctrl2
#define g_anim_func_table ((s32 (**)(s16 *, s16 *))D_800F66A0)
extern s32 D_800F66A0[];
typedef struct { s32 vx, vy, vz, pad; } VECTOR_;
#define VECTOR VECTOR_
/* SCAFFOLD END */
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
    VECTOR vec;
    Unk8005490CPose pose;
    s16 *v;
    s32 p;
    s32 i;
    s32 obj;
    s32 vz;

    if (D_800EFAE8.unk0 < 0) {
        return 0;
    }
    if (D_800EFAE8.unk0 == 0) {
        p = D_800EFAE8.unk2C;
        D_800EFAE8.unk30 = *(s32 *)(p + 0xC) + p;
        D_800EFAE8.unk34[0] = *(s32 *)(p + 0x10) + p;
        D_800EFAE8.unk34[1] = *(s32 *)(p + 0x14) + p;
        func_8003D774(D_800EFAE8.unk30, 0);
        for (i = 0; i < 2; i++) {
            if (*(s32 *)D_800EFAE8.unk34[i] == D_800A3250) {
                D_800EFAE8.unk34[i] = 0;
            }
            if (D_800EFAE8.unk34[i] != 0) {
                func_8001979C(i, (u32 *)D_800EFAE8.unk34[i]);
            }
        }
        D_800EFAE8.unk3C[1] = 0;
        D_800EFAE8.unk3C[0] = 0;
        if (D_800EFAE8.unk4 & 0x80) {
            D_800EFAE8.unk3C[0] = *(s32 *)(D_800EFAE8.unk2C + 0x18) + D_800EFAE8.unk2C;
        }
        if (D_800EFAE8.unk4 & 0x40) {
            D_800EFAE8.unk3C[1] = *(s32 *)(D_800EFAE8.unk2C + 0x1C) + D_800EFAE8.unk2C;
        }
        obj = func_8004153C(0);
        if (obj != 0) {
            func_8003FFC4(obj);
        }
        obj = func_8004153C(1);
        if (obj != 0) {
            func_8003FFC4(obj);
        }
    }
    v = func_8003D7B4(0);
    vec.vx = -v[0];
    vec.vy = v[1];
    vec.vz = -v[2];
    {
        s32 c = (&Judge)[(D_800EFAE8.unk1E + 0x400) & 0xFFF];
        s32 sn = (&Judge)[D_800EFAE8.unk1E & 0xFFF];
        s32 x = vec.vx;
        s32 z = vec.vz;
        vz = (z * c - x * sn) >> 12;
        vec.vx = (z * sn + x * c) >> 12;
        vec.vz = vz;
    }
    D_80101DF0.work.t[0] = vec.vx + D_800EFAE8.unkC;
    D_80101DF0.work.t[1] = vec.vy + D_800EFAE8.unk10;
    D_80101DF0.work.t[2] = vec.vz + D_800EFAE8.unk14;
    D_80101DF0.xf.rot.vx = v[3];
    D_80101DF0.xf.rot.vy = v[4] + 0x800;
    D_80101DF0.xf.rot.vz = v[5];
    if (D_800EFAE8.unk1E != 0) {
        MATRIX m;
        SVECTOR rot;
        rot.vx = 0;
        rot.vz = 0;
        rot.vy = D_800EFAE8.unk1E;
        g_anim_func_table[0](&rot.vx, m.m[0]);
        g_anim_func_table[0](&D_80101DF0.xf.rot.vx, D_80101DF0.work.m[0]);
        MulMatrix2(&m, (MATRIX *)&D_80101DF0.work);
        func_80042FA0((s32 *)&D_80101DF0.work, &D_80101DF0.xf.rot.vx);
        func_80042ED8((u16 *)&D_80101DF0.work);
        D_80101DF0.xf.mat = D_80101DF0.work;
    } else {
        func_800418D0(&D_80101DF0);
    }
    D_800EFAE8.unk24[0] = -D_80101DF0.xf.rot.vx;
    D_800EFAE8.unk24[1] = -D_80101DF0.xf.rot.vy;
    D_800EFAE8.unk24[2] = -D_80101DF0.xf.rot.vz;
    func_800420D0();
    func_8004211C();
    camera_InitBoneData();
    stage_InitCollision();
    func_8004A1FC(&D_800F62E0);
    func_8004A1FC(&D_800F62E0 + 0x60);
    func_8004A1FC(&D_800F62E0 + 0x180);
    for (i = 0; i < 2; i++) {
        if (D_800EFAE8.unk34[i] != 0) {
            s32 ang;
            obj = func_8004153C(i);
            func_800198D0(i, D_800EFAE8.unk0, &pose, (void *)0x1F800000);
            vec.vy = pose.unk0;
            vec.vy = (vec.vy * *(s16 *)(obj + 0x12)) >> 12;
            ang = pose.unk2;
            vec.vx = ((&Judge)[ang & 0xFFF] * pose.unk4) >> 12;
            vec.vz = ((&Judge)[(ang + 0x400) & 0xFFF] * pose.unk4) >> 12;
            vec.vy = -vec.vy;
            vec.vz = -vec.vz;
            {
                s32 c = (&Judge)[(D_800EFAE8.unk1E + 0x400) & 0xFFF];
                s32 sn = (&Judge)[D_800EFAE8.unk1E & 0xFFF];
                s32 x = vec.vx;
                s32 z = vec.vz;
                vz = (z * c - x * sn) >> 12;
                vec.vx = (z * sn + x * c) >> 12;
                vec.vz = vz;
            }
            vec.vy += D_800EFAE8.unk10;
            vec.vx += D_800EFAE8.unkC;
            vec.vz += D_800EFAE8.unk14;
            func_80040D48(i, 0, &vec.vx, &D_800EFAE8.unk1C, &pose.unk0, D_800EFAE8.unk10);
            if (D_800EFAE8.unk44[i] >= 0) {
                func_80049718(D_800EFAE8.unk44[i], (i * 2) | 0x8000, 0, 0);
                func_80049A2C(D_800EFAE8.unk44[i], i * 2, 0);
            }
            if (D_800EFAE8.unk48[i] >= 0) {
                func_80049A2C(D_800EFAE8.unk48[i], (i * 2) | 1, 1);
            }
            if (D_800EFAE8.unk3C[i] != 0) {
                func_80040304(i, (((u32 *)D_800EFAE8.unk3C[i])[D_800EFAE8.unk0 / 8] >> ((D_800EFAE8.unk0 % 8) * 4)) & 0xF);
            }
        }
    }
    func_80046EA0(10000);
    D_800EFAE8.unk0++;
    if (D_800EFAE8.unk0 >= D_800EFAE8.unk2) {
        D_800EFAE8.unk0 = -1;
    }
    return 1;
}
#undef D_800EFAE8
#undef Unk800EFAE8Ctrl
#undef g_anim_func_table
#undef VECTOR
