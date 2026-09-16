/* =====================================================================
 * func_80056CB8 — CANDIDATE (s11 forensics-modality win) — floor 48/204,
 * NOT YET 0. (Prior: 58/204 s7-s10.)
 * ---------------------------------------------------------------------
 * s11 (forensics modality). STALE-HEAD-CLAIM NOTE (same as every prior
 * session): src representation is INCLUDE_ASM between grind sessions;
 * nothing persists on main. Re-applied the s7-banked body to
 * src/text1b.c, re-confirmed floor 58/204 (build_insns 198) exactly
 * matches the s7-s10 record before any s11 change.
 *
 * PASS-ATTRIBUTION WORK: ran `pwsh tools/grinder/dump.ps1 func_80056CB8`
 * (instrumented cc1, whole-TU dumps under tmp/grind/func_80056CB8/dumps/)
 * and read the func_80056CB8 slice of the .loop dump (lines 13251-14311
 * of tmp/grind/func_80056CB8/dumps/text1b.loop). Findings on the s10
 * frontier's induction-variable hypothesis (item 1, "invert the primary
 * biv to the doubled index j"):
 *   - loop.c's biv/giv analysis (`strength_reduce`) already tracks `i`
 *     (reg 74) as the loop's ONLY verified biv, and BOTH byte-table
 *     index computations `i*2` (insns 42 and 140, "giv reg 93/126 src
 *     reg 74 ... mult 2 add 0") are recognized as givs of that SAME biv
 *     and get COMBINED by `combine_givs` ("giv at 42 combined with giv
 *     at 140") — i.e. GCC already unifies the two `i*2` index
 *     computations into one shared reduction candidate purely from our
 *     existing `(&D_x)[i*2]` spelling; no C-level index-sharing lever
 *     was missing.
 *   - Both the combined giv AND the store-address giv (insn 427, "src
 *     reg 74 ... mult 1 add (reg/v:SI 72)", i.e. `arg0+0x444+i`) are
 *     then REJECTED for strength-reduction: "giv of insn 140 not worth
 *     while, 124 vs 164" / "giv of insn 427 not worth while, 0 vs 164".
 *     The rejection predicate is loop.c:3823
 *     (`v->lifetime * threshold * benefit < insn_count`) — read
 *     tools/gcc-2.7.2/loop.c:3806-3833. `insn_count` here is 164 (this
 *     loop's real-insn count, printed at the top of the dump slice:
 *     "Loop from 22 to 444: 164 real insns."); with lifetime==1 for both
 *     givs, the product of the per-target `threshold` (the same
 *     hoisting-threshold constant the softfloat-adoption memory names —
 *     58 under -msoft-float) and each giv's `benefit` field falls below
 *     164, so `strength_reduce` sets `v->ignore = 1` and leaves the
 *     index/address recomputed via an ordinary `mult`/`add` from the biv
 *     at each use, INSTEAD OF maintaining a separate incrementing
 *     accumulator register for it.
 *   - CONSEQUENCE: this is exactly why our build never produces target's
 *     `$fp` accumulator (`sll $fp,$v1,2` once before the loop, `addiu
 *     $fp,$fp,0x2` once per iteration, read directly by both byte-table
 *     `lbu`s) — GCC's own cost model for THIS loop's insn_count (164)
 *     rejects the strength-reduction that would produce that shape,
 *     regardless of which of `i`/`j` is nominally the biv in the source.
 *     Confirms this residual is a genuine benefit-threshold decision
 *     inside loop.c, not a missing C spelling of the index arithmetic.
 *
 * PROBE 1 (killed, instance): tried making the DOUBLED value the primary
 * loop-control variable instead of `i` (`for (j = start*2; j < start*2+4;
 * j += 2) { s32 i = j >> 1; ...index sites use [j]...; store site uses
 * i; }`, j declared in function scope like `i` was). This is a genuinely
 * new spelling (distinct from s10's loop-carried idx2, which kept `i` as
 * the loop control and added idx2 as a SECOND loop-carried variable of
 * the same biv class). Measured: score REGRESSED 58 -> 86 (build_insns
 * 198 -> 202). Reverted immediately, re-confirmed floor 58 exactly
 * reproduces after revert. Consistent with the dump finding above: `j`
 * becomes the new (and only) biv, `i = j >> 1` is NOT a giv (loop.c only
 * tracks affine mult/add relations to a biv, not right-shifts), so `i`
 * has to be recomputed by an actual `sra` every iteration instead of
 * being a strength-reduced accumulator OR a cheap direct read of the
 * biv — strictly worse than the original shape on every axis. Kills the
 * "the original C's loop was literally indexed by the doubled value"
 * hypothesis outright, per the s10 frontier's own stated fallback: the
 * `$fp` accumulator is NOT reachable by respelling which variable is the
 * loop's primary induction variable; it is a direct consequence of the
 * insn_count-gated strength-reduction threshold above, which is fixed by
 * the toolchain, not by source loop shape, for this loop body's size.
 *
 * THE WIN (structural, SOTN-sanctioned variable-reuse-for-codegen-control
 * family, .claude/rules/no-new-park-categories.md § SOTN-accepted /
 * .claude/rules/defeat-licm-hoist-var-reuse.md — borrowing an EXISTING
 * local for a second unrelated but REAL value, same family as s7's
 * flags/ang/code merge, now extended to r1):
 *
 * Read the full target asm (asm/funcs/func_80056CB8.s) end to end this
 * session (frontier item 2, carried since s7/s8/s9/s10). Target's $s0 —
 * already carrying flags/ang/code per the s7 win — ALSO carries `r1`,
 * the first `func_80053614` call's return value:
 *   `jal func_80053614` / `addu $s0, $v0, $zero` (line 105 of the asm
 *   listing, .L80056E38-.L80056E40) — the call result is moved straight
 *   into $s0, the SAME register flags/ang lived in one instruction
 *   earlier (flags is dead the instant sin_p/cos_p/scale/x/z are
 *   derived from it, well before this call). $s0 (as r1) then survives
 *   THROUGH the `if (r1 != 0) {...}` branch AND through the entire
 *   second `func_80053614` call, because the final disposition value is
 *   computed as `or $s0,$s0,$v0; addiu $s0,$s0,0x1` — i.e. `flags =
 *   (r1 | (r2 << 1)) + 1` is computed by OR-ing r2 directly into the
 *   register that ALREADY holds r1, not by combining two freshly-loaded
 *   values. r2 itself never gets a persistent register at all: it's
 *   consumed straight out of $v0 immediately after the second call
 *   (`sll $v0,$v0,1; or $s0,$s0,$v0`), matching an inlined
 *   non-declared expression rather than a named local.
 *
 * CHANGE: removed the `r1` local declaration; the first
 * `func_80053614(...)` call's result is now assigned directly to
 * `flags` (`flags = func_80053614(...)`) and the `if (r1 != 0)` guard
 * became `if (flags != 0)`. Removed the `r2` local declaration entirely;
 * the second `func_80053614(...)` call is now inlined directly into the
 * final disposition expression: `flags = (flags | (func_80053614(pt0,
 * pt1, (s32)hit1, (s32)work, 0x1F8002B8) << 1)) + 1;` — this mirrors
 * target's own shape (r2 never named, consumed once out of $v0) and
 * measured BYTE-IDENTICAL to keeping a separate `r2` local that's used
 * exactly once (48/198 either way) — kept the no-extra-local form per
 * the pipeline's simplest-known-form tiebreak (Ruling 1(4),
 * ordinary-c-judge-decidable.md) since it's strictly fewer declared
 * locals with identical bytes.
 *
 * MEASURED: sandbox func_80056CB8 --disable all: score 58 -> 48/204
 * (build_insns 198, UNCHANGED insn count — pure register-identity win,
 * same shape as the s7 flags/ang/code merge). Re-verified after the
 * r2-inlining simplification (still 48/198, neutral).
 *
 * REMAINING RESIDUAL (48) — same two classes noted since s6/s7: (a) the
 * register-rotation cluster ($s3 vs $s6/$s7 for the loop counter and its
 * derived store addresses, now root-caused this session to loop.c's
 * strength-reduction benefit threshold — see PASS-ATTRIBUTION above —
 * genuinely NOT a source-spelling lever for THIS loop's insn_count), and
 * (b) the code==4 tail's branch-topology difference (target: `bltz` +
 * `beqz` as two branches; ours: one `bgez`), confirmed by s6/s7 to be
 * gated upstream, not by this tail's own C shape.
 *
 * FRONTIER FOR s12: (1) re-run tools/ra_solver/inverse_compose.py
 * classify on this s11 chassis — the r1/r2 merge is a genuine
 * instruction-count-neutral register-identity change and the loop.c
 * root-cause finding narrows what's left to a named, non-source-facing
 * mechanism; worth checking whether the PRE-RA verdict from s8 changes
 * now that TWO of the three s7-frontier merge candidates are spent. (2)
 * dx/dz/y in the code==4 tail were checked this session by re-reading
 * the tail asm (lines .L80056F08 onward): dx/dz never acquire a
 * persistent register (transient mult/mflo operands only, `$a0`/`$t0`
 * one-shot), and `y` (`lw $a0,0xBC($s1)`) is read directly into a
 * compare with no persistent home either — NO register-reuse
 * opportunity exists for these three, closing frontier item 2 from
 * s7-s10 as fully explored (r1/r2 was the only real hit in that item).
 * (3) the loop.c threshold-rejection root cause suggests the ONLY way
 * left to get the `$fp`-style accumulator is to change the loop's
 * insn_count itself (i.e., find further C restructuring that shrinks or
 * reshapes the loop body enough to cross the strength-reduction
 * benefit threshold) — NOT to respell the induction variable. This is
 * speculative and unconfirmed; a structural-modality session should
 * treat it as a hypothesis to test by measuring insn_count shifts as
 * other levers land, not a lever to chase directly.
 * ---------------------------------------------------------------------
 * s7 (structural modality, content preserved below for provenance).
 * STALE-HEAD-CLAIM NOTE (same as every prior session): src
 * representation is INCLUDE_ASM between grind sessions; nothing
 * persists on main. Re-applied the s6-banked body (D_800F6610 fix + s5
 * store-batching + func_80053614 s32-return fix) to src/text1b.c,
 * re-confirmed floor 81/204 (build_insns 197) exactly matches the s5/s6
 * record before any s7 change.
 *
 * THE WIN (structural, SOTN-sanctioned variable-reuse-for-codegen-control
 * family, .claude/rules/no-new-park-categories.md § SOTN-accepted /
 * .claude/rules/defeat-licm-hoist-var-reuse.md — borrowing an EXISTING
 * local for a second unrelated but REAL value, no new local invented):
 *
 * Read asm/funcs/func_80056CB8.s directly (not re-derived from a stale
 * dump) and found target keeps ONE hardware register ($s0) for THREE
 * successive, non-overlapping-lifetime roles across the loop body:
 *   1. `flags` (the D_8009A821 byte-table value, shifted << 8)
 *   2. `ang` — target computes this as `addu $s0,$s0,$v0` (asm lines
 *      .L80056D60/.L80056D90), i.e. literally `flags += ...`, NOT a
 *      separate register/variable.
 *   3. `code` — after the Judge-table lookups, target reuses the SAME
 *      $s0 for the final disposition code.
 *
 * CHANGE: removed the `ang` and `code` local declarations entirely;
 * every former `ang` use became `flags`; every former `code` use became
 * a re-assignment of `flags`.
 *
 * MEASURED: sandbox func_80056CB8 --disable all: score 81 -> 58/204
 * (build_insns 197 -> 198).
 * ---------------------------------------------------------------------
 * s6 (enumerate modality) content preserved below for provenance —
 * object-model fix (D_800F6610) and the killed shared-idx-local /
 * goto-tail hypotheses are UNCHANGED by this session's edits; see
 * hypotheses.md/evidence.md [s6] entries for the full writeup.
 *
 * OBJECT-MODEL FIX (real, evidence-backed, carried forward unchanged):
 * the ratan2 branch's second argument reads the separate global
 * `D_800F6610` (== D_800F6608+8, same storage as the Rec44 `.w8` field,
 * but a genuinely separate symbol per the target asm's independent
 * lui/lw(D_800F6610) relocation pair) rather than `D_800F6608.w8`. See
 * s6 evidence for the full objdiff-verified derivation.
 * ---------------------------------------------------------------------
 * PREREQUISITE CHANGE IN THE SAME TU (func_80053614 void -> s32 return
 * type, byte-neutral standalone) — unchanged since s2, still required.
 *
 * PROVENANCE: body is the s2-authored draft + s5 store-batching + s6
 * D_800F6610 fix + func_80053614 signature fix + s7 flags/ang/code
 * variable-reuse merge + s11 r1/r2 variable-reuse merge. See earlier
 * session headers (git history of this file, or hypotheses.md) for the
 * full derivation chain back to
 * memory/grind/func_80056CB8/authored-notes-2026-08-18.md.
 * ===================================================================== */

extern s16 Judge;
extern s32 ratan2(s32, s32);
extern u8 D_8009A820;
extern u8 D_8009A821;
extern s32 D_800F6610;

void func_80056CB8(s32 arg0) {
    s32 pt0[4];
    s32 pt1[4];
    s32 hit0[4];
    s32 hit1[4];
    s32 work[4];
    s32 start;
    s32 i;

    start = (*(u16 *)(arg0 + 0x3E8) & 3) * 2;
    for (i = start; i < start + 2; i++) {
        s32 obj;
        s32 flags;
        s32 scale;
        s16 *sin_p;
        s16 *cos_p;
        s32 x;
        s32 z;

        obj = arg0;
        flags = (&D_8009A821)[i * 2] << 8;
        if ((flags & 0x1000) != 0) {
            obj = *(s32 *)arg0;
        }

        if (*(u16 *)(arg0 + 0x6A) == 0x13 || *(u16 *)(arg0 + 0x6A) == 6) {
            flags += *(s16 *)(obj + 0x1CA);
        } else {
            flags += ratan2(D_800F6608.w0 - *(s32 *)(obj + 0xF4),
                             D_800F6610 - *(s32 *)(obj + 0xFC));
        }

        sin_p = &Judge + (flags & 0xFFF);
        cos_p = &Judge + ((flags + 0x400) & 0xFFF);
        scale = (&D_8009A820)[i * 2] << 8;
        x = *(s32 *)(obj + 0xB8) + ((scale * *sin_p) >> 12);
        z = *(s32 *)(obj + 0xC0) + ((scale * *cos_p) >> 12);
        pt0[0] = *(s32 *)(obj + 0xB8);
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x320;
        pt0[2] = *(s32 *)(obj + 0xC0);
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) - 0x320;
        pt1[2] = z;

        flags = func_80053614(pt0, pt1, (s32)hit0, (s32)work, 0x1F8002B8);
        if (flags != 0) {
            x += (*sin_p * 0x7D) >> 8;
            z += (*cos_p * 0x7D) >> 8;
        }

        pt0[0] = x;
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x834;
        pt0[2] = z;
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) + 0x1004;
        pt1[2] = z;

        flags = (flags | (func_80053614(pt0, pt1, (s32)hit1, (s32)work, 0x1F8002B8) << 1)) + 1;
        if (flags == 3) {
            if (hit1[1] - *(s32 *)(obj + 0xBC) < 5) {
                flags = 0;
            }
        } else if (flags == 4) {
            s32 dx = hit0[0] - *(s32 *)(obj + 0xB8);
            s32 dz = hit0[2] - *(s32 *)(obj + 0xC0);
            if (0x3D0900 < dx * dx + dz * dz) {
                s32 y = *(s32 *)(obj + 0xBC);
                if (y - hit1[1] >= 0) {
                    if (y - hit1[1] >= 0x3E9) {
                        flags = 5;
                    }
                } else {
                    if (hit1[1] - y >= 0x3E9) {
                        flags = 5;
                    }
                }
            }
        }
        *(s8 *)(arg0 + 0x444 + i) = (s8)flags;
    }
}
