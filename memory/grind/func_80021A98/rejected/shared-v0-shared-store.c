/* REJECTED (session 1, probe P2): single shared v0 across both arms of the
 * if (arg2) table block + ONE shared *(s0+0x58) = v0 store after the if/else
 * (the literal m2c shape). Measured WORSE: sandbox 25, build 157 insns
 * (target 158) — the shared store defeats the cross-jump tail shape that the
 * per-arm-store spelling reproduces exactly. Do not re-propose.
 * Fragment (rest of function unchanged): */
    {
        u16 v1 = *((u16 *) (arg1 + 4));
        s32 v0;
        *((s16 *) (s0 + 0x5C)) = v1;
        if (arg2 != 0) {
            v0 = D_80102764 + (v1 * 4);
            *((s32 *) (s0 + 0x54)) = v0;
            v0 = D_80102768 + *((u16 *) (v0 + 2));
        } else {
            s32 idx = a3 * 5;
            v0 = (&D_801027B4)[idx] + (v1 * 4);
            *((s32 *) (s0 + 0x54)) = v0;
            v0 = (&D_801027B8)[idx] + *((u16 *) (v0 + 2));
        }
        *((s32 *) (s0 + 0x58)) = v0;
    }
