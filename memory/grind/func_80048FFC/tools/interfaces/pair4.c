void func_8003DBE4(s32 arg0, s32 arg1, DR_MOVE (*arg2)[2], s32 arg3, s32 arg4) {
    s32 limit;
    s32 step;
    u32 *colors;
    s32 i;

    /* FAKE: aggregate word view keeps the original full-word tag operations;
     * direct packet/pair and union views miss (interface ledger). */
    /* SOTN: src/dra/4DA70.c:10 @db41b28eee52969244a52cc269c8163d1ed8826a */
    colors = (u32 *)arg2;

    if (arg4 != 0) {
        limit = arg1;
    } else {
        limit = arg1 - 1;
    }

    if (D_800F6656 & 1) {
        step = D_80090600;
    } else {
        s32 base_val;
        if (func_8003F268() == 0) {
            base_val = 0x6590;
        } else {
            base_val = 0x55F0;
        base_val++; base_val--; /* !FAKE: F6 bounded base-value reference probe. */
        }
        step = base_val - arg0;
    }

    if (step < 0) {
        return;
    }

    if ((u32)arg3 < (u32)step) {
        step = (s32)((u32)arg3 / (u32)arg1);
    } else {
        step = step / arg1;
    }

    i = 0;
    colors = (u32 *)((u32)colors + (D_800A36AC & 1) * 24);

    if (i < limit) {
        u32 rgb_mask = 0xFFFFFF;

        do {
            s32 idx = func_80052C28((u32)arg0 >> 2, 2);
            if (idx < 0x1000) {
                /* Encode the indexed DMA-tag address for this inherited word API. */
                u32 *pal = (u32 *)((u32)idx * 4 + (u32)D_800A378C);
                s32 tmp;
                *colors = (*colors & 0xFF000000) | (*pal & rgb_mask);
                tmp = (*pal & 0xFF000000) | ((u32)colors & rgb_mask);
                *pal = tmp;
                /* Advance the encoded DMA address. At the end it may identify
                 * the following packet without accessing that packet's storage. */
                colors = (u32 *)((u32)colors + 0x30);
                tmp = limit - 1; /* reuse tmp (multi-set) so limit-1 isn't a loop.c movable -> recomputed inline, not hoisted (see rule defeat-licm-hoist-var-reuse) */
                if (i == tmp) {
                    D_800905F8 = idx;
                }
            }
            arg0 += step;
            i++;
        } while (i < limit);
    }

    if (arg4 != 0) {
        func_8003DDF8((u32)colors);
    } else {
        u32 *pal = D_800A378C;
        *colors = (*colors & (s32)0xFF000000) | (pal[0xFFB] & 0xFFFFFF);
        pal[0xFFB] = (pal[0xFFB] & (s32)0xFF000000) | ((u32)colors & 0xFFFFFF);
    }
}