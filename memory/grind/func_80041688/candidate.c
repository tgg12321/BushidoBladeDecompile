/* func_80041688 (gnd_init_80041688) — s17 closing candidate (text1a_post
 * chassis, 2026-08-20).
 *
 * STATUS: honest floor 6, measured s17 (sandbox --disable all = 6,
 * 82==82, staged-guard form in place, zero cheat constructs stripped).
 * The 6 is EXACTLY the frame immediates (sp -0x18 vs -0x38, ra/s0
 * save+restore offsets): target carries a 0x20-byte allocated-but-
 * untouched LEADING local region (zero sp accesses in 0x10..0x2F; saves
 * at 0x30/0x34 — independently verified by the Judge, decisions.md:9176).
 *
 * GATE 1 RESOLVED: the staged loop1 guard (`b = *(s16 *)(p+2) >= 0;
 * if (b)`) was GRANTED by the Judge 2026-08-20 12:41 (decisions.md:9172)
 * under [[staged-value-reused-variable]] — all five bounds hold; the
 * s4/s5/s7 rejected-bank entry (loop1-boolean-stage-b-reuse.c) is
 * SUPERSEDED (its dead-store premise ignored the same-iteration if(b)
 * read).
 *
 * GATE 2 NOW RIPE: the Judge's ripeness prerequisite for the pad row
 * (decisions.md:9174 — "honest producers measured inert FIRST") was
 * discharged in s17: the full [[phantom-slot-frame-lever]] recipe was
 * run on the honest staged-guard baseline — .frame gradient (vars=0
 * honest vs 32 target), orphan detector (0 unallocated pseudos, 0 bare
 * (use (reg)) in greg/combine dumps), and all three producer classes
 * measured (P1 folded guard-compare x3 spellings: all vars=0, one
 * regresses score 6->7; P2 orphan-USE: single-narrow-consumer folds
 * clean, target's own bytes contain exactly ONE lh so no second HImode
 * use exists; P3 named locals: no multi-read field exists, loop2-field
 * naming vars=0). Evidence: memory/grind/func_80041688/evidence.md [s17]
 * + tmp/grind/func_80041688/s17/. The remaining step is the owner-only
 * allowlist row:
 *   engine/volatile_cheats.py _SANCTIONED_UNWRITTEN_PADS
 *   "func_80041688": frozenset({("pre_pad", 8)}),
 * (identical in form to the b734618d sibling rows). With the row granted
 * this form should measure sandbox 0.
 *
 * INTEGRATION WARNING (Judge-corrected line numbers, decisions.md:9176):
 * do NOT place this form in src while regfix.txt lines 212-214 (the 3
 * lbu-operand subst rules) still apply — the staged form already emits
 * target's [b,r,g] order and the substs would corrupt the stream. Retire
 * the rules in the same integration step.
 */
void func_80041688(s32 arg0, s32 arg1) {
    volatile u32 pre_pad[8]; /* !FAKE: target frame 0x38 keeps 0x20 leading
        bytes allocated-but-untouched (word-diff s16: only sp/ra/s0 offset
        immediates differ; Judge-verified decisions.md:9176);
        phantom-frame-slot family (no-new-park-categories.md 2026-08-18);
        honest producers measured inert s17 (all 3 classes + both
        instruments, evidence.md [s17]); needs owner allowlist row. No
        (void) shim per family form constraint. */
    s32 *player;
    s32 i;
    u8 *p;
    u8 *q;
    s32 b, r, g, v;
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
    /* FAKE: guard staged through the existing local b (value consumed by
       the branch below; b's real color-byte assignment follows later and
       this staged value is dead before it), mechanism: sched.c
       adjust_priority -> birthing_insn_p reg_n_sets==1 launch-boost gate
       (second live set of b turns off the FALSE-arm lbu's LAUNCH_PRIORITY
       so it is picked last = emitted first, matching target [b,r,g]),
       lever-exhaustion: memory/grind/func_80041688/ s1-s17. GRANTED by
       Judge 2026-08-20 (decisions.md:9172) under
       staged-value-reused-variable. */
    b = *(s16 *)(p + 2) >= 0;
    if (b) {
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
        func_80041398((v << 16) | (v << 8) | v);
    } else {
        r = *((u8 *)player + 0x18);
        g = *((u8 *)player + 0x19);
        b = *((u8 *)player + 0x1A);
        func_80041398(b | ((r << 16) | (g << 8)));
    }
}
