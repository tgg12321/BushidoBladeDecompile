void *func_80021424(u8 *rec, s32 id, u8 *out)
{
    s16 t;
    s32 ch;

    *(s16 *)(rec + 0x78) = 0;
    *(s16 *)out = 0;
    if ((u32)(id - 0x7FF5) < 11) {
        return (void *)(D_801027B0[*(s16 *)(rec + 0x4A)][0]
             + D_800A3860[*(s16 *)(rec + 0x4A)]->f66[id - 0x7FF5][*(s16 *)(rec + 0x86)] * 2);
    }
    switch (id) {
    case 0x7FF0:
        *(s16 *)(rec + 0x78) = 1;
        *(s16 *)(rec + 0x86) = *(s16 *)(rec + 0x84);
        return (void *)(D_801027B0[*(s16 *)(rec + 0x4A)][0]
             + D_800A3860[*(s16 *)(rec + 0x4A)]->f4E[*(s16 *)(rec + 0x84)] * 2);
    case 0x7FF1:
        *(s16 *)(rec + 0x86) = (*(s16 *)(rec + 0x86) + 1)
                             % D_800A3860[*(s16 *)(rec + 0x4A)]->f14;
    case 0x7FF2:
    case 0x7FF4:
        if (id == 0x7FF4) {
            *(s16 *)(rec + 0x78) = 1;
        }
        t = *(s16 *)(rec + 0x86);
        if ((t == *(s16 *)(rec + 0x88) && *(s16 *)(rec + 0x8A) == 0)
         || (t == *(s16 *)(rec + 0x8E) && *(s16 *)(rec + 0x90) == 0)) {
            *(s16 *)(rec + 0x86) = (*(s16 *)(rec + 0x86) + 1)
                                 % D_800A3860[*(s16 *)(rec + 0x4A)]->f14;
        } else if (D_800A38DC == 3 && *(s16 *)(rec + 6) != 0) {
            func_800213A0((s16 *)rec);
        }
        return (void *)(D_801027B0[*(s16 *)(rec + 0x4A)][0]
             + D_800A3860[*(s16 *)(rec + 0x4A)]->f4E[*(s16 *)(rec + 0x86)] * 2);
    case 0x7FF3:
        t = (*(s16 *)(rec + 0x86) + 1) % D_800A3860[*(s16 *)(rec + 0x4A)]->f14;
        if ((t == *(s16 *)(rec + 0x88) && *(s16 *)(rec + 0x8A) == 0)
         || (t == *(s16 *)(rec + 0x8E) && *(s16 *)(rec + 0x90) == 0)) {
            t = (t + 1) % D_800A3860[*(s16 *)(rec + 0x4A)]->f14;
        } else if (D_800A38DC == 3 && *(s16 *)(rec + 6) != 0
                   && (t == *(s16 *)(rec + 0x88) || t == *(s16 *)(rec + 0x8E))) {
            t = (t + 1) % D_800A3860[*(s16 *)(rec + 0x4A)]->f14;
        }
        return (void *)(D_801027B0[*(s16 *)(rec + 0x4A)][0]
             + D_800A3860[*(s16 *)(rec + 0x4A)]->f54[*(s16 *)(rec + 0x86)][t] * 2);
    }
    if (id & 0x8000) {
        if (*(s16 *)(rec + 0x4C) != 0) {
            ch = *(s16 *)(*(u8 **)rec + 0x4A);
        } else {
            ch = *(s16 *)(rec + 0x4A);
        }
        return (void *)(D_801027B0[ch][0] + (id & 0x7FFF) * 2);
    }
    *(s16 *)out = 1;
    return (void *)(D_80102760 + id * 2);
}
