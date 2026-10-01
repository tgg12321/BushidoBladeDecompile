void *func_80021424(PracticeMenuRec *rec, s32 id, s16 *out)
{
    s16 t;
    s32 ch;

    rec->unk_78 = 0;
    *out = 0;
    if ((u32)(id - 0x7FF5) < 11) {
        return (void *)(D_801027B0[rec->unk_4A][0]
             + D_800A3860[rec->unk_4A]->f66[id - 0x7FF5][rec->unk_86] * 2);
    }
    switch (id) {
    case 0x7FF0:
        rec->unk_78 = 1;
        rec->unk_86 = rec->unk_84;
        return (void *)(D_801027B0[rec->unk_4A][0]
             + D_800A3860[rec->unk_4A]->f4E[rec->unk_84] * 2);
    case 0x7FF1:
        rec->unk_86 = (rec->unk_86 + 1)
                    % D_800A3860[rec->unk_4A]->f14;
    case 0x7FF2:
    case 0x7FF4:
        if (id == 0x7FF4) {
            rec->unk_78 = 1;
        }
        t = rec->unk_86;
        if ((t == rec->unk_88 && rec->unk_8A == 0)
         || (t == rec->unk_8E && rec->unk_90 == 0)) {
            rec->unk_86 = (rec->unk_86 + 1)
                        % D_800A3860[rec->unk_4A]->f14;
        } else if (D_800A38DC == 3 && rec->unk_06 != 0) {
            func_800213A0(rec);
        }
        return (void *)(D_801027B0[rec->unk_4A][0]
             + D_800A3860[rec->unk_4A]->f4E[rec->unk_86] * 2);
    case 0x7FF3:
        t = (rec->unk_86 + 1) % D_800A3860[rec->unk_4A]->f14;
        if ((t == rec->unk_88 && rec->unk_8A == 0)
         || (t == rec->unk_8E && rec->unk_90 == 0)) {
            t = (t + 1) % D_800A3860[rec->unk_4A]->f14;
        } else if (D_800A38DC == 3 && rec->unk_06 != 0
                   && (t == rec->unk_88 || t == rec->unk_8E)) {
            t = (t + 1) % D_800A3860[rec->unk_4A]->f14;
        }
        return (void *)(D_801027B0[rec->unk_4A][0]
             + D_800A3860[rec->unk_4A]->f54[rec->unk_86][t] * 2);
    }
    if (id & 0x8000) {
        if (rec->unk_4C != 0) {
            ch = rec->unk_00->unk_4A;
        } else {
            ch = rec->unk_4A;
        }
        return (void *)(D_801027B0[ch][0] + (id & 0x7FFF) * 2);
    }
    *out = 1;
    return (void *)(D_80102760 + id * 2);
}
