/* func_80041688 (gnd_init_80041688) — s16 closing candidate (current
 * text1a_post chassis, 2026-08-20).
 *
 * STATUS: closes the lbu-order residual (chassis floor 8 -> 6, 82==82
 * insns, FALSE arm byte-exact incl. or a0,v1,a0). The remaining 6 is
 * EXACTLY the frame immediates (sp -0x18 vs -0x38, ra/s0 save+restore
 * offsets): target carries a 0x20-byte allocated-but-untouched LEADING
 * local region (zero sp accesses in 0x10..0x2F; saves at 0x30/0x34).
 * pre_pad below is the sanctioned phantom-frame-slot spelling but the
 * engine strips it until an owner-granted allowlist row exists:
 *   engine/volatile_cheats.py _SANCTIONED_UNWRITTEN_PADS
 *   "func_80041688": frozenset({("pre_pad", 8)}),
 * (identical in form to the b734618d sibling rows). With the row granted
 * this form should measure sandbox 0.
 *
 * TWO GATES before this can land:
 *  1. RULING on the staged loop1 guard (see FAKE comment below): s4/s5/s7
 *     banked the same spelling as variable-reuse-with-dead-store
 *     (rejected/loop1-boolean-stage-b-reuse.c); s16's dump-proven analysis
 *     reads it as staged-value-reused-variable (value consumed by the
 *     branch; the rule's own mechanism citation is this exact
 *     reg_n_sets==1 gate). ruling-request filed s16.
 *  2. The pad allowlist row (owner-only; engine/ is denylisted for
 *     sessions and for add-scope-allow).
 *
 * INTEGRATION WARNING: do NOT place this form in src while regfix.txt
 * lines 477-479 (the 3 lbu-operand subst rules) still apply — the staged
 * form already emits target's [b,r,g] order, and the substs would then
 * corrupt the stream. Retire the rules in the same step.
 */
void func_80041688(s32 arg0, s32 arg1) {
    volatile u32 pre_pad[8]; /* !FAKE: target frame 0x38 keeps 0x20 leading
        bytes allocated-but-untouched (word-diff s16: only sp/ra/s0 offset
        immediates differ); phantom-frame-slot family, needs owner
        allowlist row. No (void) shim per family form constraint. */
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
       lever-exhaustion: memory/grind/func_80041688/ s1-s16 */
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
