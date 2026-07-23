/* func_800611A4 — s5 directed-permuter find (output-30-1, perm score 30).
 * Honest sandbox --disable all = 2 — the CLOSEST-EVER form (s4 best was 5,
 * floor is 6). BUT IT IS A CHEAT: `new_var2` is an INVENTED local that stages
 * the arg0[0] load. Fails staged-value-reused-variable prereq #2 (must reuse a
 * var the fn ALREADY uses for a real job, not invent one); no /* FAKE */; and
 * it does NOT close the match (residual 2). cheat-reviewer would FAIL it, same
 * class as s4's new_var2 form. Banked for the MECHANISTIC finding, not as a
 * candidate.
 *
 * MECHANISTIC VALUE (the s5 session's real result):
 *  - This form reproduces target's EXACT three-way register layout —
 *    load-temp -> $v0 (all three), mask 0xFFFFEF -> $v1 built INTERLEAVED
 *    (lui after load 2, ori after load 3, sw). This is the layout s1-s4
 *    declared unreachable cheat-free; the invented-var staging DOES flip it.
 *  - The residual 2 diffs are PURELY load-offset ORDER: this form emits the
 *    loads 0x8,0x4,0x0 ($s0) while target emits 0x0,0x4,0x8. See
 *    tmp/grind/func_800611A4/s5/out30_s5_disasm.txt.
 *  - The RA flip is NOT from the load reordering: the cheat-FREE reverse-order
 *    form (plain `t`, no new_var2) scores 9 (the wall). The staging var IS the
 *    lever.
 *  - The flip is order-AND-position-specific and does NOT reconcile with
 *    target's forward order:
 *      reverse order + stage offset-0 load (last)  = 2   (this form)
 *      forward order + stage offset-0 load (first) = 11
 *      forward order + stage offset-2 load (last)  = 8
 *    Every staged form matching target's forward ORDER fails to flip the RA;
 *    the only form that flips the RA has the wrong (reverse) order. Target's
 *    forward-order + flipped-RA is what the $3 pin forces by fiat and is not
 *    produced by our GCC from any staged C measured. */
extern u8 D_800F116A;
extern s32 D_800F116C;
extern s32 D_800A3464;
extern s32 D_800A3468;
void func_800611A4(s32 *arg0, s32 *arg1) {
    u16 sp[3];
    u16 new_var;
    s32 new_var2;      /* CHEAT: invented staging local */
    s32 t;
    s32 *v1 = (s32 *) (&D_800F116C);
    sp[0] = *((u16 *) (((s32) arg1) + 0));
    sp[1] = *((u16 *) (((s32) arg1) + 2));
    D_800A3468 = (s32) v1;
    new_var = *((u16 *) (((s32) arg1) + 4));
    D_800F117C = (s32) (&sp[0]);
    D_800F1178 = (s32) arg0;
    D_800F1180 = (s32) (&D_800F116A);
    *v1 = 0x21001A;
    sp[2] = new_var;
    func_80060A68();
    t = arg0[2];
    D_800F1148 = t;
    t = arg0[1];
    D_800F1144 = t;
    new_var2 = arg0[0];   /* CHEAT: stages the last (offset-0) load */
    t = new_var2;
    D_800F1140 = t;
    D_800A3464 = 0xFFFFEF;
}
