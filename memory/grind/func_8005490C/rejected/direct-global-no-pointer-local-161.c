extern s32 D_800A3250[];
extern s16 *func_8003D7B4(s32);
extern void func_8001979C(s32, u32 *);
extern void func_8003D774(s32, s32);
extern void MulMatrix2(MATRIX *, MATRIX *);
extern void math_MatrixToAnglesYXZ(s32 *, s16 *);
extern void math_TransposeMatrixInPlace(u16 *);
extern void func_800198D0(s32, s32, u32 *, u16 *);
extern void func_80040D48(s32, s32, s32 *, s16 *, s16 *, s32);
extern void func_80040304(s32, s32);
s32 func_8005490C(void) {
    VECTOR vec;
    s16 frame[0x42];
    s16 *v;
    s32 p;
    s32 i;
    s32 *player;
    s32 rot_z;

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
            if (*(s32 *)D_800EFAE8.unk34[i] == D_800A3250[0]) {
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
        player = func_8004153C(0);
        if (player != 0) {
            func_8003FFC4(player);
        }
        player = func_8004153C(1);
        if (player != 0) {
            func_8003FFC4(player);
        }
    }
    v = func_8003D7B4(0);
    vec.vx = -v[0];
    vec.vy = v[1];
    vec.vz = -v[2];
    {
        s32 c = Judge[(D_800EFAE8.unk1E + 0x400) & 0xFFF];
        s32 sn = Judge[D_800EFAE8.unk1E & 0xFFF];
        s32 x = vec.vx;
        s32 z = vec.vz;
        rot_z = (z * c - x * sn) >> 12;
        vec.vx = (z * sn + x * c) >> 12;
        vec.vz = rot_z;
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
        ((void (*)(SVECTOR *, MATRIX *))g_anim_func_table[0])(&rot, &m);
        ((void (*)(SVECTOR *, MATRIX *))g_anim_func_table[0])((SVECTOR *)&D_80101DF0.xf.rot, (MATRIX *)&D_80101DF0.work);
        MulMatrix2(&m, (MATRIX *)&D_80101DF0.work);
        math_MatrixToAnglesYXZ((s32 *)&D_80101DF0.work, &D_80101DF0.xf.rot.vx);
        math_TransposeMatrixInPlace((u16 *)&D_80101DF0.work);
        D_80101DF0.xf.mat = D_80101DF0.work;
    } else {
        func_800418D0((s32 *)&D_80101DF0);
    }
    D_800EFAE8.unk24[0] = -D_80101DF0.xf.rot.vx;
    D_800EFAE8.unk24[1] = -D_80101DF0.xf.rot.vy;
    D_800EFAE8.unk24[2] = -D_80101DF0.xf.rot.vz;
    func_800420D0();
    func_8004211C();
    camera_InitBoneData();
    stage_InitCollision();
    func_8004A1FC(D_800F62E0[0]);
    func_8004A1FC(D_800F62E0[1]);
    func_8004A1FC(D_800F62E0[4]);
    for (i = 0; i < 2; i++) {
        if (D_800EFAE8.unk34[i] != 0) {
            s32 ang;
            player = func_8004153C(i);
            func_800198D0(i, D_800EFAE8.unk0, (u32 *)frame, (u16 *)0x1F800000);
            vec.vy = frame[0];
            vec.vy = (vec.vy * *(s16 *)((u8 *)player + 0x12)) >> 12;
            ang = frame[1];
            vec.vx = (Judge[ang & 0xFFF] * frame[2]) >> 12;
            vec.vz = (Judge[(ang + 0x400) & 0xFFF] * frame[2]) >> 12;
            vec.vy = -vec.vy;
            vec.vz = -vec.vz;
            {
                s32 c = Judge[(D_800EFAE8.unk1E + 0x400) & 0xFFF];
                s32 sn = Judge[D_800EFAE8.unk1E & 0xFFF];
                s32 x = vec.vx;
                s32 z = vec.vz;
                rot_z = (z * c - x * sn) >> 12;
                vec.vx = (z * sn + x * c) >> 12;
                vec.vz = rot_z;
            }
            vec.vy += D_800EFAE8.unk10;
            vec.vx += D_800EFAE8.unkC;
            vec.vz += D_800EFAE8.unk14;
            func_80040D48(i, 0, &vec.vx, &D_800EFAE8.unk1C, frame, D_800EFAE8.unk10);
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
