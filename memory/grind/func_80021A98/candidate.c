/* func_80021A98 — best known form after session 1 (recon).
 * Honest floor: sandbox --disable all = 20 / 158 (unchanged from inherited baseline).
 * This is byte-identical to the pre-session form except the second u16 table read
 * is folded inline (P1, measured byte-neutral, matches the m2c reference shape).
 * The whole distance is two RA permutation clusters — see evidence.md/hypotheses.md.
 * Apply this body over func_80021A98 in src/code6cac.c to resume. */
void func_80021A98(s32 arg0, u8 *arg1, s32 arg2) {
    u8 *s0 = ((u8 *) (&D_80101EC8)) + (arg0 * 1100);
    s32 a3;
    if ((*((s16 *) (s0 + 0x4C))) != 0) {
        a3 = *((s16 *) ((*((s32 *) s0)) + 0x4A));
    } else {
        a3 = *((s16 *) (s0 + 0x4A));
    }
    *((s16 *) (s0 + 0x4C)) = 0;
    *((s32 *) (s0 + 0x50)) = (s32) arg1;
    {
        u16 v1 = *((u16 *) (arg1 + 4));
        *((s16 *) (s0 + 0x5C)) = v1;
        if (arg2 != 0) {
            s32 v0 = D_80102764 + (v1 * 4);
            *((s32 *) (s0 + 0x54)) = v0;
            v0 = D_80102768 + *((u16 *) (v0 + 2));
            *((s32 *) (s0 + 0x58)) = v0;
        } else {
            s32 idx = a3 * 5;
            s32 v0 = (&D_801027B4)[idx] + (v1 * 4);
            *((s32 *) (s0 + 0x54)) = v0;
            v0 = (&D_801027B8)[idx] + *((u16 *) (v0 + 2));
            *((s32 *) (s0 + 0x58)) = v0;
        }
    }
    {
        s32 v0_50 = *((s32 *) (s0 + 0x50));
        u16 old_kind = *((u16 *) (s0 + 0x6A));
        s32 a0_58 = *((s32 *) (s0 + 0x58));
        *((u8 *) (s0 + 0x60)) = (u8) arg2;
        do { } while (0);
        *((u8 *) (s0 + 0x61)) = (u8) a3;
        {
            u8 a1_val = *((u8 *) (v0_50 + 6));
            *((s16 *) (s0 + 0x6C)) = old_kind;
            {
                s32 v1_58 = *((s32 *) (s0 + 0x58));
                s32 li1 = 1;
                *((s16 *) (s0 + 0x42)) = 0;
                *((s16 *) (s0 + 0x7A)) = li1;
                *((s32 *) (s0 + 0x7C)) = 0;
                *((s16 *) (s0 + 0x46)) = 0;
                *((s16 *) (s0 + 0x40)) = a1_val;
                *((s16 *) (s0 + 0x6A)) = *((u8 *) a0_58);
                *((s16 *) (s0 + 0x6E)) = *((u8 *) (v1_58 + 2));
            }
        }
        {
            s32 v0_50b = *((s32 *) (s0 + 0x50));
            s32 kind = *((u16 *) (s0 + 0x6A));
            *((s16 *) (s0 + 0x70)) = (*((u8 *) (v0_50b + 9))) & 3;
            {
                s32 a0_flag = 0;
                if ((((kind == 2) || (kind == 0x1B)) || (kind == 0x28)) || (kind == 0x26)) {
                    a0_flag = 1;
                }
                *((u8 *) (s0 + 0xAD)) = a0_flag;
            }
            func_800324D0(s0);
            {
                s32 kind2 = *((u16 *) (s0 + 0x6A));
                s32 v1k = kind2 & 0xFFFF;
                if (v1k == 9) {
                    *((s16 *) (s0 + 0x152)) = 1;
                    *((s16 *) (s0 + 0x154)) = *((u16 *) (s0 + 0x1CA));
                    goto end;
                }
                if (v1k == 2) {
                    if ((*((s16 *) (s0 + 0x152))) != 0) goto clear_152;
                    if ((*((s16 *) (s0 + 0x6C))) == 0x13) goto clear_152;
                    *((s16 *) (s0 + 0x154)) = *((u16 *) (s0 + 0x1D8));
                    goto clear_152;
                }
                if (((u32) (kind2 - 0x19)) >= 2U) goto not_in_range;
                if ((*((s16 *) (s0 + 0x152))) == 0) goto set_154;
                if (v1k != 0x19) goto set_152;
                if ((*((s16 *) (s0 + 0x6C))) != v1k) goto set_152;
                goto set_154;
                set_154:
                *((s16 *) (s0 + 0x154)) = *((u16 *) (s0 + 0x1D8));
                goto set_152;
                not_in_range:
                if (v1k != 0x11) goto clear_152;
                set_152:
                *((s16 *) (s0 + 0x152)) = 1;
                goto end;
                clear_152:
                *((s16 *) (s0 + 0x152)) = 0;
            }
            end:
            {
                u16 v1f = *((u16 *) (s0 + 0x6A));
                if ((((v1f == 2) || (v1f == 0x1B)) || (v1f == 0x28)) || (v1f == 0x26)) {
                    *((u8 *) (s0 + 0xAF)) = ((*((u8 *) (s0 + 0xB0))) & 0xF) != 5;
                }
            }
        }
    }
}
