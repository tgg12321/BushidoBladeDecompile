/* REJECTED s7 forensics probe (frontier from s6 hypothesis 3).
 *
 * Mutation: stage the raw s16 halfword through `b` before the sign test:
 *   b = *(s16 *)(p + 2);
 *   if (b >= 0) { if (arg1) p[1] |= 1; else p[1] &= ~1; }
 *
 * Rationale attempted: s6 CONFIRMED the mechanism is flow.c fusion of
 * source-level `b` into pseudo 78, triggering a sched1 hazard-tag priority
 * differential on the FALSE-arm color-lbu triple. The raw-halfword spelling
 * was proposed as a narrower "semantically-purposeful" alternative to the
 * loop1-boolean-stage-b-reuse cheat (which stores the boolean result of
 * >= 0 into b). The raw halfword IS the value the branch tests.
 *
 * FORENSIC RESULT (s7): the standalone-cc1 diff vs baseline confirms the
 * FALSE-arm lbu order DOES flip to target's [b,r,g]. Mechanism reproduced:
 *   baseline .s FALSE (127-129): lbu $4,24 / lbu $2,25 / lbu $3,26   [r,g,b]
 *   s7raw    .s FALSE (125-127): lbu $3,26 / lbu $4,24 / lbu $2,25   [b,r,g] ✓
 *
 * BUT the total sandbox score REGRESSED 2 -> 11 (build_insns 82 -> 81) due
 * to a loop1 register-allocation cascade:
 *   .greg pseudo 75 (loop1 i): baseline $a0(4) -> s7raw $a2(6)
 *   .greg pseudo 76 (loop1 p): baseline $v1(3) -> s7raw $a0(4)
 *   .greg pseudo 78 (b):       $v1(3) in both
 *
 * Mechanism: the raw-halfword form gives `b` a live range of two insns
 * (lh -> bltz) INSIDE loop1's body. That live range CONFLICTS with p's
 * live range (p is used for the base of the lh AND for the p[1] stores
 * downstream in the body), so greg cannot coalesce b and p into $v1 —
 * b keeps $v1 and p is displaced to $a0, cascading i to $a2. The
 * displacement also changes loop reorg (loop label placement flips —
 * .L21 above vs .L5 below the addu p+=104), adding delay-slot changes.
 *
 * Why cheat10 (`b = (X >= 0)`) doesn't cascade the same way: the boolean
 * spelling emits as a setcc/slt whose def-use is back-to-back (b defined,
 * then immediately consumed by `if (b)`), with an intra-iter live range
 * near-zero. That short live range doesn't force a hardreg conflict
 * against p, so greg keeps pseudo 76(p) in $v1 and pseudo 78(b) also
 * in $v1 (non-overlapping in each iter). The boolean's flow.c fusion
 * across iters still widens 78's conflict set enough to flip sched1,
 * without paying the loop1 RA displacement cost.
 *
 * POLICY REJECTION: even if it had been byte-neutral, this construct
 * is a dead store to `b` for RA/scheduling coercion — same intent as
 * loop1-boolean-stage-b-reuse.c, different spelling. The value stored
 * to `b` in loop1 is IMMEDIATELY dead after the `if (b >= 0)` test
 * (b is unread until the FALSE-arm reassignment overwrites it). A human
 * programmer writing this function would not stage the halfword through
 * a color-byte accumulator variable; the natural spelling is the direct
 * `if (*(s16 *)(p+2) >= 0)` (which is what target reads back to). Per
 * [[no-new-park-categories]] cheats-by-any-spelling, the raw-halfword
 * variant is the same forbidden pattern as the boolean variant with a
 * different value stored.
 *
 * IMPLICATIONS for the frontier:
 *   1. Mechanism (pseudo-78 fusion -> sched1 priority flip) is CONFIRMED
 *      to be reachable via loop1-scope defs of source-level `b`.
 *   2. Any LIVE, semantically-purposeful spelling that stores TO `b` in
 *      loop1 must give `b` a non-zero intra-iter live range (that's what
 *      makes the store "live" rather than dead). But intra-iter live
 *      range for `b` in loop1 body creates a conflict with `p`, forcing
 *      p out of $v1 and regressing loop1 emission.
 *   3. Conclusion: there is NO in-loop1 spelling of the axis that is
 *      BOTH byte-neutral AND non-cheat. Either it's a zero-live-range
 *      store (cheat10 shape, forbidden as dead store) or it's a
 *      non-zero-live-range store (this s7 shape, regresses via RA
 *      cascade).
 *   4. The un-KILLED remaining surface is s6 frontier #2: use a
 *      DIFFERENT variable (not `b`) in loop1 whose live range overlaps
 *      p's but whose greg-conflict-graph contribution to pseudo 78's
 *      neighbor set matches the cheat10 widening (74/79/80). This is
 *      indirect and speculative; not measured this session.
 */
void gnd_init_80041688(s32 arg0, s32 arg1) {
    s32 *player;
    s32 i;
    u8 *p;
    u8 *q;
    s32 b, r, g, v;
    volatile s32 sp10[8];
    extern s32 func_800486FC(void);

    player = (s32 *)g_player_ptrs[arg0];
    if (player == NULL) return;

    p = (u8 *)player + 0x94;
    if (arg1) {
        p[1] |= 1;
    } else {
        p[1] &= ~1;
    }

    i = 1;
loop1:
    p += 0x68;
    b = *(s16 *)(p + 2);       /* CHEAT: dead store to steer RA + sched1 */
    if (b >= 0) {
        if (arg1) p[1] |= 1;
        else      p[1] &= ~1;
    }
    i++;
    if (i < 18) goto loop1;

    q = (u8 *)player + 0x10D5;
loop2:
    if (*(s32 *)(q + 0x57) == 0) goto after2;
    if (arg1) *q |= 1;
    else      *q &= ~1;
    q += 0x68;
    goto loop2;
after2:

    if (func_800486FC()) {
        r = *((u8 *)player + 0x18);
        g = *((u8 *)player + 0x19);
        b = *((u8 *)player + 0x1A);
        v = func_8004881C(b, g, r);
        gnd_load_tex((v << 16) | (v << 8) | v);
    } else {
        r = *((u8 *)player + 0x18);
        g = *((u8 *)player + 0x19);
        b = *((u8 *)player + 0x1A);
        gnd_load_tex(b | ((r << 16) | (g << 8)));
    }
    (void)sp10;
}
