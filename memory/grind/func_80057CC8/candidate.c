/* s46 (2026-08-31, escalation modality) — CANDIDATE, sandbox distance 0.
 *
 * MEASURED THIS SESSION on the live chassis with these exact edits in src/text1b.c:
 *   `sandbox func_80057CC8 --disable all` -> score 0, target_insns 111, build_insns 111,
 *   rules_dropped 0, zero cheat-asm, zero regfix/asmfix rules
 *   `verify-oracle` -> ok: true, build_matches: true (full-build SHA1 == oracle)
 *   artifacts: tmp/grind/func_80057CC8/s46/{sandbox_score0.json,verify_oracle.json}
 *
 * WHY THIS FORM IS NOW ADMISSIBLE (the whole reason s46 differs from s1..s45).
 * The form is the s40 banked one (rejected/s40-no-base-local-per-use-site-reads-
 * score0-RULING-PENDING.c), which every prior session was forbidden to submit: it
 * writes the vertex-table base expression *(s16 **)(arg0 + 4) at each of its five use
 * sites instead of binding it to one pointer local, and that is the construct the
 * 2026-07-20 owner refusal closed and state.json banned_constructs entry 5 records.
 * The owner's 2026-08-30 escalation-batch **ruling 6b** (docs/grind/decisions.md:14846)
 * REVERSED that: "func_80057CC8 — GRANTED for re-adjudication under F3. The
 * two-materialization construct (compound address expression written per call site)
 * is the F3 compound-address-duplication family sanctioned 2026-08-18; the 2026-07-20
 * refusal predates that grant. Returns to ACTIVE; the banked score-0 forms integrate
 * through a fresh layer-2 review under F3's prerequisites (value real + consumed at
 * each site, annotation)."  F3 itself is .claude/rules/no-new-park-categories.md:377.
 * Both F3 prerequisites are met and stated in the in-source annotation: the duplicated
 * expression's value is loaded and consumed at every one of the five sites (two for
 * the cx/cy centre read, two for the prev-neighbour ratan2 argument pair, one for the
 * next-neighbour address), and the duplication site carries the FAKE annotation naming
 * mechanism (cse1) and lever-exhaustion (hypotheses.md).
 *
 * DELTA vs the banked s40 file: the `s16 new_var;` staging local in the final store was
 * measured byte-NEUTRAL this session (score 0 with and without) and is therefore DELETED
 * — a dead local with an intent-announcing name is exactly what cheat-checklist T6
 * catches, and it bought nothing.  The `base`/`half` split-init in the ang_next<ang_prev
 * arm was tested for the same collapse and is NOT neutral (collapsing it measures score 6,
 * banked as rejected/s46-collapse-base-half-splitinit-score6.c), so it stays; it is the
 * user-sanctioned split-init accumulation family ([[split-init-accumulation-sanctioned]]).
 *
 * Prior floor: 16 (s38-s45, flat across solver/forensics/rederive/structural/escalation).
 * That 16 was the floor of the ban-COMPLIANT search space only; s44/s45 proved in closed
 * form that distance 0 REQUIRES the second source-level materialization, which is exactly
 * what ruling 6b now permits.  Nothing about the ban-compliant foreclosure is retracted.
 *
 * Self-vet: memory/grind/func_80057CC8/self_vet.md
 */
/* Per-vertex neighbour-angle midpoint: computes the outward bisector direction at
 * vertex arg1 of the polygon whose vertex table hangs off arg0[4], and writes the
 * offset point into *arg2 / *arg3.
 *
 * FAKE: the vertex-table base expression *(s16 **)(arg0 + 4) is written out at each
 * of its five use sites rather than bound to one pointer local (F3
 * compound-address duplication across call arg-lists, .claude/rules/no-new-park-categories.md:377,
 * owner ruling 2026-08-18; re-adjudication granted for this function by owner ruling
 * 6b of the 2026-08-30 escalation batch, docs/grind/decisions.md:14846).
 * mechanism: cse1 (cse.c:1948 hash_arg_in_memory / cse.c:7241-7246
 * `if (! CONST_CALL_P (insn)) invalidate_memory (&everything);`) folds the five
 * front-end loads down to the target's two, the intervening ratan2 CALL_INSN being
 * the only thing that stops the fold; a single cached local instead asserts the
 * call cannot write ((s16 **)arg0)[1], which C does not guarantee and which folds
 * to one load (s40 probe pA/pB/pC/pD, tmp/grind/func_80057CC8/s40/probe.c).
 * lever-exhaustion: memory/grind/func_80057CC8/hypotheses.md (46 sessions, 133
 * rejected forms, three ban-compliant regimes foreclosed in closed form at honest
 * floor 16; evidence.md s40-s45).
 */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 *p;
    s32 pi;
    u16 cy;

    prev_idx = arg1 - 1;
    cx = *(u16 *)((s32)(*(s16 **)(arg0 + 4)) + arg1 * 4 + 0);
    cy = *(u16 *)((s32)(*(s16 **)(arg0 + 4)) + arg1 * 4 + 2);

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        next_idx = tmp;
        if ((s16) tmp >= (s32)arg0[3]) {
            next_idx = 0;
        }
    }

    pi = (s16) prev_idx;
    ang_prev = ratan2((*(s16 **)(arg0 + 4))[pi * 2] - (s16) cx,
                      (*(s16 **)(arg0 + 4))[pi * 2 + 1] - (s16) cy) & 0xFFF;
    p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
    ang_next = ratan2(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) {
        /* FAKE: `base` and `half` are fresh once-written/once-read named
         * intermediates for the antipode of ang_prev and half the angular gap
         * (named-intermediate family, .claude/rules/no-new-park-categories.md:204
         * + the 2026-08-17 clarification at :208-229; both values are real and
         * appear in the target's own bytes, build_insns == target_insns == 111).
         * mechanism: local-alloc.c block_alloc -- they become BLOCK-LOCAL allocnos
         * (pseudos 82 and 83, "in block 5", tmp/grind/func_80057CC8/dumps/text1b.lreg
         * at the func_80057CC8 heading) that local-alloc seats before global.c runs;
         * collapsing them into one expression instead yields a single combine-folded
         * tree whose scratch is allocated globally and measures score 6.
         * lever-exhaustion: memory/grind/func_80057CC8/hypotheses.md (46 sessions);
         * both collapse spellings banked in
         * rejected/s46-collapse-base-half-splitinit-score6.c. */
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(*(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
