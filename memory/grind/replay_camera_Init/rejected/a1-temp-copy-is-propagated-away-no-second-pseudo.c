/* REJECTED (s6 H23, 2026-07-30) — holding the second parameter in a C temporary
 * cannot create the `move a3,a1` copy, because cse/jump copy-propagate the
 * temporary away before register allocation ever runs.
 *
 * Measured on the cc1 -da allocno dumps, not on the score
 * (tmp/grind/replay_camera_Init/s6/liveness.py, rtl_tw1/ and rtl_tw2/):
 * this form and its sibling (second consumer of `a1` at the D_80101E9E store)
 * both produce exactly THREE allocnos — 72, 73 and the pe62 pointer — the same
 * three the plain candidate.c produces, with 73 allocated hard reg 5 ($a1) and
 * no hard conflict on 5.  The temporary is gone by the time global.c sees the
 * function.
 *
 * NOTE: the forms below are DIAGNOSTIC ONLY and semantically divergent (they add
 * a second consumer of `a1` that target does not have) — they were never
 * candidates, they exist to test whether a second pseudo can be created at all.
 * The answer is no, in any placement tried.
 *
 * See also s5 H18 (expression spelling is inert) — same conclusion from the
 * score side; this is the allocator-level proof of it.
 */
s32 replay_camera_Init(s32 a0, s32 a1) {
    s32 sval;
    s16 *pe62 = &D_80101E62;
    s32 *pe70 = &D_80101E70;
    s32 reloaded;

    if (*pe62 != 0) {
        return 0;
    }

    sval = ((s32)(a0 << 16)) >> 13;
    D_80101E60 = a0;
    {
        extern u8 SpecialCam;
        s32 cam_val;
        s32 ec_val;
        cam_val = *(s32 *)((u8 *)&SpecialCam + sval);
        D_80101E6C = cam_val;
        {s32 t = a1; D_80101E7C = t;}     /* the temporary — propagated away */
        ec_val = *(s32 *)((u8 *)&D_8008EC38 + sval);
        D_80101E70 = ec_val;
    }
    D_80101E68 = a1;                      /* the second consumer — divergent */
    *pe62 = 2;
    reloaded = *pe70;
    D_80101E9E = 0;
    D_80101E78 = (u32)(reloaded + 0x7FF) >> 11;
    return 1;
}
