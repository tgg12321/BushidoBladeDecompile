void func_80022F34(void) {
    s32 i;
    u16 *tbl;
    s32 offset;

    i = 0;
    tbl = D_80102778.unk_0;
    offset = 0;

loop_22F34:
    {
        PracticeMenuRec *rec = (PracticeMenuRec *)((u8 *)g_practice_menu_table + offset);

        if (rec->unk_06 != 0) {
            s32 mode = D_800A38DC;

            switch (mode) {
                case 0:
                    rec->unk_08 = D_80102778.unk_A[i] << 4;
                    break;
                case 1:
                case 2:
                default:
                    rec->unk_08 = *tbl;
                    break;
                case 3:
                    break;
            }

            {
                s16 idx1 = rec->unk_4A;
                s32 val1 = D_801027B0[idx1][3];
                {
                    s16 idx2 = rec->unk_00->unk_4A;
                    func_80055138(i, val1, D_801027B0[idx2][3]);
                }
            }
        }

        tbl++;
        i++;
        offset += 0x44C;
    }
    if (i < 2) goto loop_22F34;
}
