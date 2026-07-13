/* func_80030BA8 — candidate, session 2. sandbox --disable all == 0.
 *
 * Replaces the two audit-flagged constructs from HEAD:
 *   - `s32 new_var;` (function-scope, assigned 0xF423F only inside the inner
 *     block)  ->  `s32 range_sq = 0xF423F;` declared+initialized in the block
 *     that uses it.  Measured byte-identical (score 0); the function-scope
 *     lifetime was never load-bearing, only the single-pseudo identity was.
 *   - `s32 neg1 = -1;`  ->  `s32 empty_slot = -1;` (rename only; still
 *     function-scope, which IS load-bearing — see rejected/ for the two
 *     measured negatives).
 */
s32 func_80030BA8(u8 *arg0) {
    s32 i = 0;
    s32 empty_slot = -1;
    u8 *p = (u8 *)&D_80106A7A;
    s32 old_val;

    loop:;
    {
        u16 val = *(u16 *)p;
        s32 sval;
        if ((unsigned)(val - 0x12) < 12u) {
            goto next;
        }
        sval = (s16)val;
        if (sval == empty_slot) {
            goto next;
        }
        if (*(s32 *)(p + 0x4E) != 0) {
            goto next;
        }
        {
            s32 bc = *(s32 *)(arg0 + 0xBC);
            s32 pos2e = *(s32 *)(p + 0x2E);
            if (bc - 0x64 >= pos2e) {
                goto next;
            }
            if (pos2e >= bc + 0x64) {
                goto next;
            }
        }
        {
            s32 dx = *(s32 *)(arg0 + 0xF4) - *(s32 *)(p + 0x2A);
            s32 dz = *(s32 *)(arg0 + 0xFC) - *(s32 *)(p + 0x32);
            s32 range_sq = 0xF423F;
            i++;
            if (dx * dx + dz * dz > range_sq) {
                goto loop_test;
            }
        }
        if (func_80030B10(arg0, sval) == 0) {
            return -1;
        }
        old_val = (s32)(*(s16 *)p);
        *(s16 *)p = (s16)empty_slot;
        if (old_val == 0xE) {
            s32 a0val = D_800A36F2 ^ 0xE;
            func_80032854(a0val != 0, 0x2F, arg0 + 0xF4, 0);
        } else {
            func_80032854(*(s16 *)(arg0 + 4), 0x11, arg0 + 0xF4, 0);
        }
        return old_val;
    }
    next:
    i++;
    loop_test:
    if (i < 12) {
        p += 0x64;
        goto loop;
    }
    return -1;
}
