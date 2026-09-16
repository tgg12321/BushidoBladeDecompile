/* REJECTED (s1) — introducing a persistent `s16 *slot = (s16*)((u8*)D_800A34FC + mask + 0x28);`
 * local and reusing it across all field28 accesses within one loop iteration.
 * Measured: sandbox --disable all score 142 (vs 94 for the inline-dereference form
 * banked as candidate.c), and it grew the frame from target's 0x48 to 0x58 (extra
 * callee-save-equivalent stack slot) because the cached pointer pseudo raised
 * register pressure enough that `ret` (the $fp-resident flag in target) got
 * spilled to the stack instead of register-allocated. Not a cheat question — this
 * was ordinary C that simply didn't reproduce target's register allocation. Killed
 * as an approach for this function; do not re-introduce a cached field28 pointer
 * local. See candidate.c's header comment for the accepted structure (fresh
 * `*(s16 *)((u8 *)D_800A34FC + mask + 0x28)` dereference written out at each of
 * the ~5 syntactic use sites, matching target's fresh-address-per-use pattern).
 */
s32 func_8006CCC8(s32 *arg0, s32 *arg1, s16 arg2) {
    s16 i;
    s16 lim;
    s16 *slot;
    s32 shift;
    s32 mask;
    s32 fade;
    s32 j;
    u8 *rec;
    s32 ret;

    ret = 0;
    if ((*(s32 *)((u8 *)D_800A34FC + 0x28) == 0x50005) && (*arg1 & 0x400040)) {
        func_8005C650(1, 0x7F, 0x7F);
        ret = 1;
    }

    fade = 0;
    for (i = 0, shift = 0, mask = 0; i < 2; i++, shift += 0x10, mask += 2) {
        slot = (s16 *)((u8 *)D_800A34FC + mask + 0x28);
        lim = ((arg2 >> i) & 1) ? 4 : 5;

        if (*arg1 & (0x1000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (*slot <= 0) {
                *slot = lim;
            } else {
                *slot = (s16)(*slot - 1);
            }
        } else if (*arg1 & (0x4000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (*slot >= lim) {
                *slot = 0;
            } else {
                *slot = (s16)(*slot + 1);
            }
        }

        if (*slot == 3) {
            if (*arg1 & (0x40 << shift)) {
                func_8005C650(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    if (i == 0) {
                        *(rec + 0x17) = (u8)((*(rec + 0x17) & 0xF0) + (*(rec + 0x1A) & (0xF << fade)));
                    } else {
                        *(rec + 0x17) = (u8)((*(rec + 0x17) & 0xF) + (*(rec + 0x1A) & (0xF << fade)));
                    }
                }
            }
        } else if (*slot >= 4) {
            if (*slot == 4) {
                if (*arg1 & (0x40 << shift)) {
                    func_8005C650(1, 0x7F, 0x7F);
                    for (j = 0; j < 3; j++) {
                        rec = (u8 *)D_800A3524 + j;
                        if (i == 0) {
                            *(rec + 0x17) = (u8)((*(rec + 0x17) & 0xF0) + (*(rec + 0x1D) & (0xF << fade)));
                        } else {
                            *(rec + 0x17) = (u8)((*(rec + 0x17) & 0xF) + (*(rec + 0x1D) & (0xF << fade)));
                        }
                    }
                }
            }
        } else if (*slot >= 0 && (*arg1 & (0xF0 << shift))) {
            func_8005C650(0, 0x7F, 0x7F);
            func_8006CBD4(i, *arg1);
        }
        fade += 4;
    }
    return ret;
}
