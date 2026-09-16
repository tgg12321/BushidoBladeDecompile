/* s8 REJECTED (c01/c08): cluster-B group with the *(arg1+8) = *(arg0+0x14)
 * store moved ahead of the two record stores. MEASURED 8 and build_insns 197
 * (parity lost -- cse folds the arg0+0x14 read). c02/c06 (same store placed
 * BETWEEN the record stores) measured 6 / build_insns 198. */
extern s32 D_800A34F8;
void func_8006A564(u8 *arg0, u8 *arg1, s32 arg2) {
    u8 *tile;
    u8 *obj2;
    s32 s4;

    tile = *(u8 **)(arg0 + 0x18);
    SetTile(tile);
    {
        s32 v0;
        if ((D_800A34F8 & 0xF) == arg2) {
            v0 = *(u8 *)(arg1 + 0x29);
            tile[5] = 0;
            tile[4] = v0;
            v0 = *(u8 *)(arg1 + 0x2B);
            s4 = 0;
        } else {
            s4 = 0x20;
            v0 = 0x50;
            tile[4] = v0;
            tile[5] = v0;
        }
        tile[6] = v0;
        *(s16 *)(tile + 8) = (0x5F);
        *(s16 *)(tile + 0xA) = ((*(s32 *)(arg1 + 0x1C)) + 0xF);
        *(s16 *)(tile + 0xC) = ((*(s32 *)(arg1 + 0x18)) + 0x19);
        *(s16 *)(tile + 0xE) = 1;
    }
    SetSemiTrans(tile, *(s32 *)(arg1 + 0x10));
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);
    tile += 0x10;

    SetTile(tile);
    {
        s32 v0;
        if ((D_800A34F8 & 0xF) == arg2) {
            v0 = *(u8 *)(arg1 + 0x29);
            tile[5] = 0;
            tile[4] = v0;
            v0 = *(u8 *)(arg1 + 0x2B);
            tile[6] = v0;
        } else {
            v0 = 0x20;
            tile[4] = v0;
            tile[5] = v0;
            tile[6] = v0;
        }
        *(s16 *)(tile + 8) = (*(s32 *)(arg1 + 0x18));
        *(s16 *)(tile + 0xA) = ((*(s32 *)(arg1 + 0x1C)) + 0xE);
        *(s16 *)(tile + 0xC) = 0x78;
        *(s16 *)(tile + 0xE) = 1;
    }
    SetSemiTrans(tile, 1);
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);
    tile += 0x10;

    SetTile(tile);
    {
        s32 v0;
        if ((D_800A34F8 & 0xF) == arg2) {
            v0 = *(u8 *)(arg1 + 0x29);
            tile[5] = 0;
            v0 = (u32)v0 >> 1;
            tile[4] = v0;
            v0 = *(u8 *)(arg1 + 0x2B);
            v0 = (u32)v0 >> 1;
            tile[6] = v0;
        } else {
            v0 = 0x10;
            tile[4] = v0;
            tile[5] = v0;
            tile[6] = v0;
        }
        *(s16 *)(tile + 8) = ((*(s32 *)(arg1 + 0x18)) + 0x40);
        *(s16 *)(tile + 0xA) = ((*(s32 *)(arg1 + 0x1C)) + 0xD);
        *(s16 *)(tile + 0xC) = 0x38;
        *(s16 *)(tile + 0xE) = 1;
    }
    SetSemiTrans(tile, 1);
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);

    obj2 = *(u8 **)(arg0 + 4);
    tile = tile + 0x10;
    *(u8 **)(arg0 + 0x18) = tile;
    tile = *(u8 **)(obj2 + 0x1C);
    {
        s32 v0;
        *(s32 *)(arg1 + 0) = *(s32 *)(tile + 0x28);

        if ((D_800A34F8 & 0xF) == arg2) {
            *(u8 *)(arg1 + 0x2A) = 0;
            *(u8 *)(arg1 + 0x29) = (u32)(*(u8 *)(arg1 + 0x29)) >> 1;
            *(u8 *)(arg1 + 0x2B) = (u32)(*(u8 *)(arg1 + 0x2B)) >> 1;
        } else {
            *(u8 *)(arg1 + 0x2B) = 0x28;
            *(u8 *)(arg1 + 0x2A) = 0x28;
            *(u8 *)(arg1 + 0x29) = 0x28;
        }

        *(s32 *)(arg1 + 0x18) = 0;
        v0 = *(s32 *)(arg1 + 0);
        v0 += 0xC;
        *(s32 *)(arg1 + 0x1C) += 0xF;
        *(s32 *)(arg1 + 4) = v0;

        *(s32 *)(arg1 + 8) = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg0 + 0x14) = func_8007352C((s32)arg1);
    }
    {
        s32 v0;
        v0 = *(s32 *)(tile + 0x2C);
        *(s32 *)(arg1 + 8) = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg1 + 0) = v0;
        *(s32 *)(arg1 + 4) = v0 + 0xC;
        *(s32 *)(arg0 + 0x14) = func_8007352C((s32)arg1);
    }

    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0,
                func_8006E480(*(s32 *)(arg1 + 0), s4), 0);
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), *(u8 **)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) = *(s32 *)(arg0 + 0x1C) + 0xC;
}