typedef struct {
    u16 flags;
    u16 id;
    u8 b[4];
} StatusEvt;

extern void *func_80021424(u8 *, u16, u8 *);
extern void func_80032854(s32, s32, s32 *, s16 *);

void func_8001FBE8(void) {
    u8 *rec;
    u8 *sel;
    StatusEvt *ent;
    u8 *data;
    u8 *snd;
    s32 lo;
    s32 hi;
    s32 dz;
    s32 i;
    u16 kind;
    s32 pos[3];

    if (D_800A376E != 0) {
        D_800A376E = 0;
        D_800A38E8 = 0xFF;
        if (D_80101F5E != 0) {
            return;
        }
        if (D_801023AA != 0) {
            return;
        }
        func_80021A98(D_800A38AE, (u8 *)D_800A36D8, D_800A381C);
        if (D_800A36CA & 0x1000) {
            s32 o = (D_800A38AE == 0) ? 0x44C : 0;
            *(s16 *)(&D_80101EC8 + o + 0x4C) = 1;
        }
        func_80021A98(D_800A38AE == 0, (u8 *)D_800A36D8, D_800A381C);
        D_8010238E = 2;
        D_80101F42 = 2;
        return;
    }
    if (D_800A3758 != 0xFF) {
        sel = &D_80101EC8 + D_800A3758 * 0x44C;
        if (D_800A3769 != 0) {
            *(s16 *)(sel + 0x286) = 1;
            *(s16 *)(sel + 0x94) = 0;
        } else {
            *(s16 *)(sel + 0x286) = 0;
            *(s16 *)(sel + 0x94) = 1;
        }
        if (*(s16 *)(sel + 0x96) != 0) {
            *(s16 *)(sel + 0x286) += 2;
        }
        *(s32 *)(sel + 0x74) = *(s32 *)(sel + 0xBC);
        *(s16 *)(*(u8 **)sel + 0x286) = 1;
        *(s16 *)(*(u8 **)sel + 0x94) = 0;
        *(s32 *)(*(u8 **)sel + 0x74) = *(s32 *)(*(u8 **)sel + 0xBC);
        if (*(s16 *)(*(u8 **)sel + 0x96) != 0) {
            *(s16 *)(*(u8 **)sel + 0x286) += 2;
        }
        D_800A3758 = 0xFF;
        return;
    }
    if (D_8010214E != -1) {
        return;
    }
    if (D_8010259A != -1) {
        return;
    }
    for (i = 0; i < 2; i++) {
        rec = &D_80101EC8 + i * 0x44C;
        if (*(s16 *)(rec + 0x7A) == 0) {
            continue;
        }
        ent = (StatusEvt *)func_8001FAE4(*(s32 **)(rec + 0x50));
        if (ent == 0) {
            continue;
        }
        data = ent->b;
        lo = data[0] * 20;
        hi = data[1] * 20;
        dz = *(s32 *)(rec + 0xBC) - *(s32 *)(*(u8 **)rec + 0xBC);
        if (func_8001FB34((s32 *)rec, data[3] & 0x80) == 0) {
            continue;
        }
        if (D_800A387C < lo) {
            continue;
        }
        if (hi < D_800A387C) {
            continue;
        }
        if (dz <= -100 || dz >= 100) {
            continue;
        }
        kind = *(u16 *)(*(u8 **)rec + 0x6A);
        if (kind != 0x15 && kind != 0x2C && kind != 0xE && kind != 0x19) {
            continue;
        }
        D_800A38AE = i;
        D_800A376E = 0;
        D_800A3758 = 0xFF;
        D_800A371C = data[2] * 20;
        D_800A38E8 = data[3] & 0x7F;
        snd = func_80021424(rec, ent->id, rec + 0x5E);
        func_80021A98(i, snd, *(s16 *)(rec + 0x5E));
        *(s16 *)(*(u8 **)rec + 0x4C) = 1;
        *(s16 *)(*(u8 **)rec + 0x5E) = *(s16 *)(rec + 0x5E);
        func_80021A98(i == 0, snd, *(s16 *)(rec + 0x5E));
        *(s16 *)(rec + 0x7A) = 2;
        *(s16 *)(*(u8 **)rec + 0x7A) = 2;
        *(s16 *)(*(u8 **)rec + 0x86) = *(s16 *)(*(u8 **)rec + 0x84);
        *(s16 *)(*(u8 **)rec + 0x272) += 1;
        pos[0] = (*(s32 *)(rec + 0xF4) + *(s32 *)(*(u8 **)rec + 0xF4)) / 2;
        pos[1] = (*(s32 *)(rec + 0xF8) + *(s32 *)(*(u8 **)rec + 0xF8)) / 2;
        pos[2] = (*(s32 *)(rec + 0xFC) + *(s32 *)(*(u8 **)rec + 0xFC)) / 2;
        func_80032854(i, 0x10, pos, 0);
        return;
    }
}
