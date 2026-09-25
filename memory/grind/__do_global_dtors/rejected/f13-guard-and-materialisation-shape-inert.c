/*
 * REJECTED (session 11, modality `structural`) — frontier F13: the GUARD and
 * MATERIALISATION shape of motion_Close is codegen-inert on the session-9 goto
 * chassis, and every arrangement that is NOT inert is strictly worse.
 *
 * WHY THIS WAS RE-MEASURED AT ALL (it is not a re-run of a dead axis).
 * Sessions 2 and 3 swept guard shape — early return, hoisted guard local,
 * &&-chain, block-local declarations, declaration-with-initialiser,
 * count-assigned-first, decrement-before-call — and recorded them as inert or
 * costly. Every one of those measurements was taken on the do-while-LOOP
 * chassis, i.e. inside the LOOP-NOTE regime, where flow.c weights every
 * reference in the function by loop depth and the two allocnos sit ~10% apart
 * (p 10 refs / live_length 8 = 37500 against count 8/7 = 34285). Session 9
 * replaced the chassis with a `goto` loop, which emits no NOTE_INSN_LOOP_BEG/
 * END for the walk, so every reference is counted RAW and the margin collapsed
 * to 1.4% (p 5/8 = 12500 against count 5/7 = 14285). A one-reference or
 * one-insn lever that is worthless at a 10% margin can be decisive at 1.4%, so
 * the s2/s3 verdicts do not transfer by construction. This is the same
 * chassis-transfer error frontier FR3 exists to prevent — and session 11
 * measured a concrete instance of it: `count--` before `f()` cost 19 points in
 * the loop-note regime (s3 sweep3.py) and is completely FREE here (13, cell
 * V7_dec_before_call_d2). The conclusions changed; the floor did not.
 *
 * THE SWEEP. tmp/grind/motion_Close/s11/f13sweep.py — 10 variants x
 * do-while(0) wrap depths 0/1/2 = 30 cells, every cell measured with
 * `sandbox motion_Close --disable all` AND with the instrumented cc1
 * (BB2_ALLOC_DEBUG=1) reading the per-allocno n_refs / live_length / priority /
 * assigned-hardreg table. Log f13sweep.log, per-cell assembly f13_*.s.
 *
 * RESULT — six variants are BYTE-IDENTICAL to the control at every depth, with
 * the identical allocno table (count 5 raw refs / live_length 7 = 14285; p
 * 4+depth / 8 = 10000 / 12500 / 15000) and the identical 20 / 20 / 13 ladder:
 *   V0 control          if (D_800A2668 != 0) { p = ...; count = ...; walk }
 *   V1 early_return     if (D_800A2668 == 0) { return; } ...
 *   V2 guard_local      s32 g = D_800A2668; if (g != 0) { ... }
 *   V4 blocklocal_decls p and count declared inside the guard block
 *   V6 f_outer          the call temp `f` declared at function scope
 *   V7 dec_before_call  count-- placed before f() in the loop body
 * The guard's spelling, the guard value's staging, the declaration SCOPE of the
 * two locals, the scope of the call temp and the loop body's statement order
 * are all free variables of this function in the raw-count regime. None of them
 * touches either allocno's reference count or live length.
 *
 * The three non-inert variants are all worse, and each one is worse for a
 * mechanically explained reason:
 *   V3 countfirst_assign  17 / 16 / 16. Assigning count before p shortens p's
 *      live_length to 7 and lengthens count's to 8, so at wrap depth 1 p wins
 *      $s0 with 5/7 = 14285 against count's 5/8 = 12500 — the target's ROLES at
 *      wrap depth ONE. It still scores 16, not 13, because the two address
 *      materialisations then emit count-pair-then-p-pair while the target emits
 *      p-pair-then-count-pair. This is the first direct measurement of frontier
 *      FR2's cost on the goto chassis: the role flip is available one wrap level
 *      cheaper, and buying it costs exactly the 3 points of materialisation
 *      order it saves in device weight. Trading a policy device for residual
 *      points is not an improvement.
 *   V5 declinit           19 at every depth. Declaring p and count WITH
 *      initialisers at function scope hoists both materialisations above the
 *      D_800A2668 guard (the target materialises them inside it), and the wrap
 *      then lands on the walk instead of on p's initialiser, weighting BOTH
 *      allocnos together (9/13 refs against 7/10) exactly as session 4 measured
 *      for nested wraps — the multiplier cannot separate two allocnos that both
 *      live inside the wrapped region.
 *   V8 guard_inside       20 / 21 / 21. Materialising p above the guard grows
 *      p's live_length to 9-10 and adds two emitted instructions (27 vs 25).
 *
 * CONSEQUENCE. The guard/materialisation surface joins the loop construct
 * (F12, s10), the global declarations (F11, s9), the whole-TU shape (F10, s8)
 * and the pointer/loop idiom (s9) as a measured-dead axis. Nothing on this
 * surface reaches 13 at a wrap depth below 2, and nothing on it reaches a score
 * below 13 — consistent with the s7 residual table, which attributes all 13
 * remaining points to H1 (frame / outgoing-arg area), F5 ($v0-vs-$t0 hard-reg
 * scan order) and F7a/F7b (save order + beqz delay slot), none of which any
 * guard shape can touch.
 *
 * The representative rejected member is the cheapest non-control form, V3 at
 * wrap depth 1 — the only cell in the sweep that reaches the target's register
 * roles with fewer than two wrap levels:
 */

void motion_Close(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 != 0) {
        count = (s32)&D_00000000;      /* count assigned FIRST: p live_length
                                          7 vs count 8, so one wrap level is
                                          enough for p to take $s0 ... */
        do { p = &D_8008D070; } while (0);
        /* ... but the address materialisations now emit count-pair before
           p-pair, which the target emits the other way round: score 16, three
           points worse than the depth-2 p-first form's 13. */
        if (count != 0) {
        again:
            {
                void (*f)(void) = *p;
                p++;
                f();
                count--;
            }
            if (count != 0) {
                goto again;
            }
        }
    }
}
