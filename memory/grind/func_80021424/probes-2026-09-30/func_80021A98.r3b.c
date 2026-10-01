void func_80021A98(s32 arg0, u8 *arg1, s32 arg2) {
    PracticeMenuRec *s0 = &g_practice_menu_table[arg0];
    s32 a3;
    if ((s0->unk_4C) != 0) {
        a3 = s0->unk_00->unk_4A;
    } else {
        a3 = s0->unk_4A;
    }
    s0->unk_4C = 0;
    s0->unk_50 = arg1;
    {
        u16 v1 = *((u16 *) (arg1 + 4));
        s0->unk_5C = v1;
        if (arg2 != 0) {
            s32 v0 = D_80102764 + (v1 * 4);
            s0->unk_54 = v0;
            v1 = *((u16 *) (v0 + 2));
            s0->unk_58 = D_80102768 + v1;
        } else {
            s32 v0 = D_801027B0[a3][1] + (v1 * 4);
            s0->unk_54 = v0;
            v1 = *((u16 *) (v0 + 2));
            s0->unk_58 = D_801027B0[a3][2] + v1;
        }
    }
    {
        u8 *v0_50 = s0->unk_50;
        u16 old_kind = s0->unk_6A;
        s32 a0_58 = s0->unk_58;
        s0->unk_60 = (u8) arg2;
        /* FAKE: load-bearing match device — removing this empty do-while(0)
         * moves the sandbox score 0 -> 2 (measured 2026-08-08); mechanism:
         * the sanctioned do-while(0) wrap's codegen effect on the seating
         * of the surrounding byte stores (do-while-zero-exception.md,
         * owner ruling 2026-07-06). */
        do { } while (0);
        s0->unk_61 = (u8) a3;
        {
            u8 a1_val = v0_50[6];
            s0->unk_6C = old_kind;
            {
                s32 v1_58 = s0->unk_58;
                s0->unk_42 = 0;
                s0->unk_7A = 1;
                s0->unk_7C = 0;
                s0->unk_46 = 0;
                s0->unk_40 = a1_val;
                /* FAKE: the do-while(0) wrap's weighting seats a0_58 in $a0
                 * and a1_val in $a1 as in target (cluster-2 $4/$5
                 * close-out). */
                do { s0->unk_6A = *((u8 *) a0_58); } while (0);
                s0->unk_6E = *((u8 *) (v1_58 + 2));
            }
        }
        {
            u8 *v0_50b = s0->unk_50;
            s32 kind = s0->unk_6A;
            s0->unk_70 = (v0_50b[9]) & 3;
            {
                s32 a0_flag = 0;
                if ((((kind == 2) || (kind == 0x1B)) || (kind == 0x28)) || (kind == 0x26)) {
                    a0_flag = 1;
                }
                s0->unk_AD = a0_flag;
            }
            func_800324D0((u8 *)s0);
            {
                s32 kind2 = s0->unk_6A;
                s32 v1k = kind2 & 0xFFFF;
                if (v1k == 9) {
                    s0->unk_152 = 1;
                    s0->unk_154 = (u16)s0->unk_1C8.vy;
                    goto end;
                }
                if (v1k == 2) {
                    if ((s0->unk_152) != 0) goto clear_152;
                    if ((s0->unk_6C) == 0x13) goto clear_152;
                    s0->unk_154 = (u16)s0->unk_1D8;
                    goto clear_152;
                }
                if (((u32) (kind2 - 0x19)) >= 2U) goto not_in_range;
                if ((s0->unk_152) == 0) goto set_154;
                if (v1k != 0x19) goto set_152;
                if ((s0->unk_6C) != v1k) goto set_152;
                goto set_154;
                set_154:
                s0->unk_154 = (u16)s0->unk_1D8;
                goto set_152;
                not_in_range:
                if (v1k != 0x11) goto clear_152;
                set_152:
                s0->unk_152 = 1;
                goto end;
                clear_152:
                s0->unk_152 = 0;
            }
            end:
            {
                u16 v1f = s0->unk_6A;
                if ((((v1f == 2) || (v1f == 0x1B)) || (v1f == 0x28)) || (v1f == 0x26)) {
                    s0->unk_AF = ((s0->unk_B0) & 0xF) != 5;
                }
            }
        }
    }
}
