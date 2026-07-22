/* AddTbpOfst_80047EE8 (text1b.c) — s1 candidate, sandbox --disable all = 10.
 *
 * Instruction stream is 53/53 and IDENTICAL to target except the 10
 * frame-offset insns (2x addiu sp: -0x28 vs target -0x48; 8x save/restore
 * offsets shifted by the 32-byte unused vars region). Same terminal species
 * as sibling InitHiraRmd_80047FBC (owner-escalation filed 2026-07-20,
 * awaiting ruling).
 *
 * Levers that produced this (s1, 2026-07-21):
 *  1. SINGLE-WALKER: one pointer variable `p` carries arg0-copy -> add ->
 *     reload -> loop walker (was split cached/p in the old candidate;
 *     merging them: 16 -> 14).
 *  2. LIVE precompute of the call's first arg into `first` inside the loop
 *     (sibling InitHiraRmd's committed lever 1). This flips whole-function
 *     RA so GCC stages arg0 through $s0 (sw s0; move s0,a0; sw s2;
 *     move s2,s0; addu s0,s0,a1) exactly as target: 14 -> 11, 53/53 insns.
 *     `first` is live code (consumed by the call) — not a FAKE construct.
 *  3. FAKE-annotated arg0 = 0 dead store (dead-store-fake-exception family,
 *     sanctioned 2026-07-01) breaks cse2's {arg0,p,saved} canonical-reg
 *     class so the second pointer binds addu s0,s2,v0 (not a0): 11 -> 10.
 *     Same lever the sibling's s6 Judge-PASSed. PREREQUISITE DISCHARGED
 *     s2 (2026-07-21): 6 pure spellings of the saved/p init chain measured
 *     dead ON THIS BODY (const decl-init, decl-order, split-init reversal,
 *     u32 retype, two-statement rebind = 11; mask offset spelling = 14
 *     byte-diverging; FAKE removed = 11). See rejected/pure-*.c +
 *     tmp/grind/AddTbpOfst_80047EE8/s2/spelling_sweep.md. The FAKE
 *     construct's lever-exhaustion requirement is met on this function.
 *
 * Remaining gap: the 32-byte unused frame (target vars=32 phantom; this form
 * vars=0, cc1 .frame). s3 (2026-07-21, structural) DIRECTLY measured the
 * phantom-frame grid DEAD on this body: 9 variants via the cc1 .frame
 * instrument, NO stream-preserving shape moves vars off 0. Decisive control
 * v08 = the exact tslLineG5Init phantom trigger (s16 pair `(a&~b)&1` guarding a
 * real store) reserves vars=0 here — the mechanism does not fire. Only a
 * written s32 rec[6] reaches vars=24, at +6 diverging stores the target lacks
 * (forbidden dead-array). See tmp/grind/AddTbpOfst_80047EE8/s3/frame_grid.md.
 * Structural axis exhausted; same endgame-lock species as sibling
 * InitHiraRmd_80047FBC (owner-escalation filed 2026-07-20, awaiting ruling).
 *
 * s4 (2026-07-21, permuter): whole-function campaign on THIS floor-10 chassis
 * (--stack-diffs, base_score 266, 9451 iters, fresh seed) lowered the weighted
 * score to 202 ONLY via a volatile dead-frame local (`volatile long`/`unsigned
 * long long pad`) — a forbidden volatile/dead-vars frame cheat reserving ~8
 * bytes; never reached 0, next novel WORSE (207). No legit pure-C frame-growth
 * form. Permuter modality independently corroborates s3's structural DEAD.
 * See rejected/permuter-volatile-pad-frame-coercion.c.
 */
void AddTbpOfst_80047EE8(s32 arg0, s32 arg1)
{
    u32 *p;
    s32 saved;
    s16 new_var;
    s32 count;
    u32 v_off;
    unsigned int new_var2;
    p = (u32 *) arg0;
    saved = (s32) p;
    arg0 = 0; /* FAKE */
    p = (u32 *) ((s32) p + (((s32) (arg1 << 16)) >> 14));
    v_off = *p;
    p = (u32 *) (saved + ((v_off >> 2) << 2));
    count = *(p++);
    if (count != 0)
    {
        count--;
        do
        {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            s32 first;
            word = *p;
            p = (u32 *) (((s32) p) + 4);
            a1v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            a2v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            a3v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            new_var = a1v;
            new_var2 = word >> 2;
            first = saved + (new_var2 << 2);
            v0v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            efc_buki_draw_zanzou(first, new_var, a2v, a3v, v0v);
        }
        while ((count--) != 0);
    }
}
