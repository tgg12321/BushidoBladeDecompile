/* REJECTED (grind s10, 2026-09-05) - the ONLY difference from the MATCHED
 * candidate.c is the loop spelling: `do { ... } while (i < 2);` instead of the
 * bottom-tested `goto loop_22F34;`. Everything else (the corrected
 * `extern s32 D_801027BC[][5];` / `extern u8 D_80102782[2];` declarations, the
 * maspsx_label_nop_funcs.txt entry, the local names) is identical.
 * MEASURED: sandbox --disable all = 14, full build SHA1
 * 051da27f25e3f2b6266f1bd3a6971b9e19c8cab5 != oracle. With a compile-time-known
 * trip count of 2, loop.c rotates/peels the do-while form; the goto form keeps
 * the single bottom test the target has (slti/bnez at 0x80023024).
 * So: the goto loop is load-bearing, and this is NOT an "idiomatic cleanup"
 * that a later session should re-apply.
 */
void func_80022F34(void) {
    s32 i;
    u16 *tbl;
    s32 offset;

    i = 0;
    tbl = (u16 *)&D_80102778;
    offset = 0;

    do {
        u8 *rec = (u8 *)&D_80101EC8 + offset;

        if (*(s16 *)(rec + 6) != 0) {
            s32 mode = D_800A38DC;

            switch (mode) {
                case 0:
                    *(s16 *)(rec + 8) = D_80102782[i] << 4;
                    break;
                case 1:
                case 2:
                default:
                    *(s16 *)(rec + 8) = *tbl;
                    break;
                case 3:
                    break;
            }

            {
                s16 idx1 = *(s16 *)(rec + 0x4A);
                s32 val1 = D_801027BC[idx1][0];
                rec = *(u8 **)rec;
                {
                    s16 idx2 = *(s16 *)(rec + 0x4A);
                    func_80055138(i, val1, D_801027BC[idx2][0]);
                }
            }
        }

        tbl++;
        i++;
        offset += 0x44C;
    } while (i < 2);
}
