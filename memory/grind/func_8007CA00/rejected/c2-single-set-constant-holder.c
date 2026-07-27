/* REJECTED (s4, measured) — the s3 frontier-1 hypothesis: single-set
 * constant-holder `{ s32 c2 = 0x400; t = c2; }` in case2 feeding shared t,
 * betting on birthing_insn_p promotion (reg_n_sets(c2)==1) + greg copy
 * coalescing. MEASURED DEAD: 22 diff lines (ccopy tail) / 24 (direct tail)
 * vs the plain multi-set form's identical 22/24 — the holder changes NOTHING
 * (the copy does not coalesce into an anchored li; t's allocation still
 * fails the same way). Would also have needed the named-local-fake-exception
 * protocol. Superseded by the do-while(0) fence (sanctioned, measured 0).
 * Do NOT re-propose constant-holder locals for this function. */
extern u8 D_8009BE74;
extern u8 D_8009BE77;
s32 func_8007CA00(s16 *arg0) {
    s32 v1, a, t;
    switch (D_8009BE74) {
    case 1:
        if (D_8009BE77 != 0) {
            t = 0x400;
            v1 = arg0[2];
            a = arg0[0];
        sub:
            t = t - v1;
            return t - a;
        }
        t = arg0[0];
        goto ret;
    case 2:
        if (0 != D_8009BE77) {
            v1 = ((s16)(*((u16 *)(arg0 + 2)))) / 2;
            a = arg0[0];
            { s32 c2 = 0x400; t = c2; }
            goto sub;
        }
        t = ((s32)((s16)(*((u16 *)arg0)))) / 2;
        goto ret;
    default:
        t = arg0[0];
    ret:
        return t;
    }
}
