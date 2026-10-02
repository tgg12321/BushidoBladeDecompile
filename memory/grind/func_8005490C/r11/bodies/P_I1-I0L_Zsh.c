extern s32 D_800A3250[2];
extern s16 *func_8003D7B4(s32);
extern void func_8001979C(s32, u32 *);
extern void func_8003D774(s32, s32);
extern void MulMatrix2(MATRIX *, MATRIX *);
extern void math_MatrixToAnglesYXZ(s32 *, s16 *);
extern void math_TransposeMatrixInPlace(u16 *);
extern void func_800198D0(s32, s32, u32 *, u16 *);
extern void func_80040D48(s32, s32, s32 *, s16 *, s16 *, s32);
extern void func_80040304(s32, s32);
/* Per-frame stage handler on the ctrl block D_800EFAE8.  On the first frame
 * (unk0 == 0) it resolves the loaded data's offset table (unk2C) into the
 * camera stream (unk30), the per-player motion streams (unk34[], dropped
 * when they start with the "NULL" tag D_800A3250) and the per-player nibble
 * tables (unk3C[]).  Every frame it places the camera (rotated about y by
 * unk1E, plus the stage offset unkC..unk14), then decodes and places each
 * player's motion frame.  The block holds those addresses as integers: typed
 * as pointers, the relocation sums in func_80054FDC and func_80054604 swap
 * their addu operands (memory/grind/func_8005490C/evidence.md [s2]). */
s32 func_8005490C(void) {
    /* FAKE: second C handle to the global ctrl block (pointer-alias family),
       as in func_80054604 above; mechanism: expand/cse address
       materialisation -- the pointer local seats %hi/%lo(D_800EFAE8) in one
       callee-saved base register ($s3) for the whole body; lever-exhaustion:
       the direct D_800EFAE8.field form re-materialises the address and
       measures 161 (420 insns) vs 0 (memory/grind/func_8005490C/evidence.md
       [s2], rejected/direct-global-no-pointer-local-161.c). */
    Unk800EFAE8Ctrl *s = &D_800EFAE8;
    VECTOR vec;
    /* The 0x84-byte motion frame func_800198D0 decodes (func_80023F08 keeps
       its pair as MotionFrame).  Here the root offset, heading and distance
       are read as signed halfwords (lh at 0x80054D10, 0x80054D28,
       0x80054D2C) and the frame goes to func_80040D48's s16 * parameter, so
       it is the s16 channel array; MotionFrame's u16 unk_02 / unk_04 (lhu in
       func_80023F08) would load lhu here. */
    s16 frame[0x42];
    s16 *v;
    s32 i;
    s32 *player1;
    s32 rot_z;

    if (s->unk0 < 0) {
        return 0;
    }
    if (s->unk0 == 0) {
        s32 *player0;

        s32 p;
        s32 j;

        p = s->unk2C;
        s->unk30 = *(s32 *)(p + 0xC) + p;
        s->unk34[0] = *(s32 *)(p + 0x10) + p;
        s->unk34[1] = *(s32 *)(p + 0x14) + p;
        func_8003D774(s->unk30, 0);
        for (j = 0; j < 2; j++) {
            if (*(s32 *)s->unk34[j] == D_800A3250[0]) {
                s->unk34[j] = 0;
            }
            if (s->unk34[j] != 0) {
                func_8001979C(j, (u32 *)s->unk34[j]);
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
        player1 = func_8004153C(0);
        if (player1 != 0) {
            func_8003FFC4(player1);
        }
        player0 = func_8004153C(1);
        if (player0 != 0) {
            func_8003FFC4(player0);
        }
    }
    v = func_8003D7B4(0);
    vec.vx = -v[0];
    vec.vy = v[1];
    vec.vz = -v[2];
    {
        s32 c = Judge[(s->unk1E + 0x400) & 0xFFF];
        s32 sn = Judge[s->unk1E & 0xFFF];
        s32 x = vec.vx;
        s32 z = vec.vz;
        rot_z = (z * c - x * sn) >> 12;
        vec.vx = (z * sn + x * c) >> 12;
        vec.vz = rot_z;
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
        ((void (*)(SVECTOR *, MATRIX *))g_anim_func_table[0])(&rot, &m);
        ((void (*)(SVECTOR *, MATRIX *))g_anim_func_table[0])((SVECTOR *)&D_80101DF0.xf.rot, (MATRIX *)&D_80101DF0.work);
        MulMatrix2(&m, (MATRIX *)&D_80101DF0.work);
        math_MatrixToAnglesYXZ((s32 *)&D_80101DF0.work, &D_80101DF0.xf.rot.vx);
        math_TransposeMatrixInPlace((u16 *)&D_80101DF0.work);
        D_80101DF0.xf.mat = D_80101DF0.work;
    } else {
        func_800418D0((s32 *)&D_80101DF0);
    }
    s->unk24[0] = -D_80101DF0.xf.rot.vx;
    s->unk24[1] = -D_80101DF0.xf.rot.vy;
    s->unk24[2] = -D_80101DF0.xf.rot.vz;
    func_800420D0();
    func_8004211C();
    camera_InitBoneData();
    stage_InitCollision();
    func_8004A1FC(D_800F62E0[0]);
    func_8004A1FC(D_800F62E0[1]);
    func_8004A1FC(D_800F62E0[4]);
    for (i = 0; i < 2; i++) {
        if (s->unk34[i] != 0) {
            s32 ang;
            player1 = func_8004153C(i);
            func_800198D0(i, s->unk0, (u32 *)frame, (u16 *)0x1F800000);
            vec.vy = frame[0];
            vec.vy = (vec.vy * *(s16 *)((u8 *)player1 + 0x12)) >> 12;
            ang = frame[1];
            vec.vx = (Judge[ang & 0xFFF] * frame[2]) >> 12;
            vec.vz = (Judge[(ang + 0x400) & 0xFFF] * frame[2]) >> 12;
            vec.vy = -vec.vy;
            vec.vz = -vec.vz;
            {
                s32 c = Judge[(s->unk1E + 0x400) & 0xFFF];
                s32 sn = Judge[s->unk1E & 0xFFF];
                s32 x = vec.vx;
                s32 z = vec.vz;
                rot_z = (z * c - x * sn) >> 12;
                vec.vx = (z * sn + x * c) >> 12;
                vec.vz = rot_z;
            }
            vec.vy += s->unk10;
            vec.vx += s->unkC;
            vec.vz += s->unk14;
            func_80040D48(i, 0, &vec.vx, &s->unk1C, frame, s->unk10);
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
