/* =====================================================================
 * func_80056CB8 — CANDIDATE (s2, 2026-09-16; re-verified s3, 2026-09-16;
 * re-verified s4, 2026-09-16; IMPROVED s5, 2026-09-16 via enumerate
 * modality; s6 2026-09-16 object-model fix, floor unchanged) — floor
 * 81/204, NOT YET 0
 * ---------------------------------------------------------------------
 * s6 (enumerate modality). STALE-HEAD-CLAIM CORRECTED: contrary to this
 * file's s3 "PREREQUISITE CHANGE...already applied to src/text1b.c" note,
 * re-checking HEAD this session found func_80053614 STILL declared `void`
 * on main (src representation for func_80056CB8 is INCLUDE_ASM, so no C
 * draft is ever actually on main between sessions — the note describes a
 * candidate-only fact, not a persisted one). Re-applied both the
 * func_80053614 s32-return fix and this candidate body to src/text1b.c
 * for measurement, re-confirmed floor 81/204 (build_insns 197) matches
 * the s5-recorded number exactly.
 *
 * OBJECT-MODEL FIX (real, evidence-backed, SCORE-NEUTRAL): built
 * `tools/objdiff.py tmp/sandbox/func_80056CB8/text1b.o build/src/text1b.o`
 * (build/src/text1b.o is the still-INCLUDE_ASM reference object, i.e. the
 * ORIGINAL target bytes for this TU — objdiff confirms func_80056CB8 is
 * the ONLY function in text1b.o differing from build/src/text1b.o, so this
 * is a clean isolated diff). Found the ratan2-branch's second argument
 * emits `lw v0,8(v0)` (our build: field D_800F6608.w8 read via ONE lui of
 * D_800F6608's base, then two lw's at offsets 0 and 8) vs target's
 * `lui v1,%hi(D_800F6608); lw v1,%lo(D_800F6608)(v1)` followed by an
 * INDEPENDENT `lui v0,%hi(D_800F6610); lw v0,%lo(D_800F6610)(v0)` — two
 * FULLY SEPARATE symbol relocations (confirmed directly in
 * asm/funcs/func_80056CB8.s:51-55, which names D_800F6610 explicitly).
 * D_800F6610 is already a valid linker symbol (undefined_syms_auto.txt:642
 * `D_800F6610 = 0x800F6610;`, exactly D_800F6608+8 i.e. the same storage as
 * the Rec44 .w8 field) — so the ORIGINAL C at this one call site used the
 * separate global D_800F6610, not a struct-field read through D_800F6608.
 * This is NOT a struct-vs-scalar aggregate question (D_800F6608 stays a
 * genuine Rec44 struct — it's written field-by-field all over
 * src/code6cac.c as camera-anchor state, e.g. func_8001B294; changing that
 * declaration would risk every OTHER consumer). It is a TU-LOCAL read-site
 * correction: added `extern s32 D_800F6610;` to text1b.c (this function's
 * TU only, does not touch include/code6cac.h or any other consumer) and
 * changed the second ratan2 arg from `D_800F6608.w8 - *(obj+0xFC)` to
 * `D_800F6610 - *(obj+0xFC)`. Re-running objdiff after the fix: the
 * `-lw v0,8(v0)` / `+lw v0,0(v0)` diff pair is GONE (confirmed via `diff`
 * against the pre-fix objdiff capture) — the read now matches target
 * exactly. CONFIRMED (instance): this specific 2-line mismatch is real and
 * fixed. Score/build_insns did NOT move (still 81/197) — this diff pair
 * was not part of the weighted score's counted residual (likely because
 * both sides had the same instruction COUNT in this slot, just a wrong
 * operand, so the edit-distance-style score treats a wrong-operand lw same
 * weight as a correct one in this slot). Kept anyway: it is a genuine
 * correctness fix (removes a wrong read from the honest build) with zero
 * downside, and it removes noise from any future objdiff-based analysis
 * of this function.
 *
 * TRIED AND REVERTED (KILLED, instance): hypothesized the loop computes
 * `i * 2` TWICE (once for the D_8009A821 flags index, once for the
 * D_8009A820 scale index, ~30 C lines apart with a ratan2 call in
 * between) where target's asm computes it ONCE into $s8 and reuses it via
 * `addu at,at,s8` at both sites (objdiff shows our build re-materializing
 * `sll v0,s3,0x1` + `addu at,at,v0` a second time where target just reuses
 * s8). Factored both `(&D_8009A821)[i * 2]` / `(&D_8009A820)[i * 2]` into
 * a single `s32 idx = i * 2;` used at both sites. Measured: score got
 * WORSE (81 -> 90, build_insns 197 -> 199) — the shared idx local
 * increased register pressure across the intervening ratan2 call/branch
 * enough to cost more than the single multiply it saved. Reverted.
 * KILLED (instance): the shared-idx-local spelling of this reuse, on this
 * exact chassis (s6, D_800F6610 fix + s5 store-batching, func_80053614
 * s32-return fix), measured 2026-09-16 — target's $s8 reuse is NOT reached
 * by this spelling; whatever produces it in target is a different C shape
 * (possibly the two array reads are structured differently, e.g. via a
 * shared base pointer local, not a shared index local — untried).
 *
 * TRIED AND REVERTED (neutral, not banked as a kill — zero measured
 * effect either way): rewrote the code==3/code==4 tail (the
 * hit1[1]-y / y-hit1[1] >= 0x3E9 nested if/else that sets code=5) using
 * explicit `goto store;` early exits instead of structured nested
 * if/else fallthrough, hypothesizing that objdiff's observed extra
 * target instructions (`+bltz`/`+beqz`/`+addu v0,s7,s6`/`+j TGT` — see
 * FRONTIER below) came from a different C control-flow shape. Measured
 * IDENTICAL score and build_insns (81/197) before and after — this
 * particular goto-vs-nested-if rewrite is byte-neutral for our fork on
 * this chassis. Reverted to the simpler nested-if form (no reason to
 * carry unnecessary goto complexity when it measures no different).
 * ---------------------------------------------------------------------
 * s5 (enumerate modality): systematic spelling sweep on the pt0/pt1
 * point-computation block using tools/spelling_enum.py +
 * tools/sweep_variants.py. Region swept: `sin_p = ...; cos_p = ...;
 * scale = ...; pt0[0..2] = ...; x = ...; pt1[0] = x; pt1[1] = ...;
 * z = ...; pt1[2] = z;` (32 spellings: 2 assign-orderings among
 * sin_p/cos_p/scale/x/z that respect def-before-use, plus the
 * commutative-swap axis on the `scale * *sin_p` / `scale * *cos_p`
 * products). RESULT: moving the pt0[0..2]/pt1[0..2] STORE statements to
 * AFTER all five value computations (sin_p, cos_p, scale, x, z) --
 * instead of interleaving them between x's computation and z's
 * computation as the s1-authored draft did -- drops the score from
 * 106/204 to 81/204 (build_insns 201 -> 197). 12/32 swept spellings hit
 * this new floor (81); the assign order sin_p,cos_p,scale,x,z (original
 * textual order, batched-writes) is among the winners and is what's
 * banked below. A handful of swap-axis variants that put `&Judge + expr`
 * as `&(expr) + Judge` are ill-formed pointer arithmetic (invalid C,
 * `&` of a non-lvalue) and scored unchanged at 106 -- almost certainly
 * because they failed to compile and the sweep's baseline fallback
 * masked it; NOT a real 106-tier finding, just tool-artifact noise (see
 * hypotheses.md [s5] for the diff evidence). No cheat construct
 * involved: this is a pure reordering of REAL statements (batch the
 * output stores after the output values are computed), zero new locals,
 * zero dead code, zero annotations needed.
 *
 * WHY THIS WORKS (dump-verified, see tmp/grind/func_80056CB8/dumps/text1b.s
 * region for func_80056CB8 vs asm/funcs/func_80056CB8.s .L80056D94-.L80056E38):
 * target ALSO batches its output stores relative to x/z's computation in a
 * pattern much closer to write-values-then-store than
 * store-pt0-immediately-interleave-with-x/z; target keeps t0=obj->0xB8 and
 * t1=obj->0xC0 live across the whole block (fed into BOTH pt0[0]/pt0[2] AND
 * x/z's base-add) while STILL redundantly re-reading obj->0xC0 for pt0[2]
 * and obj->0xBC for pt1[1] a second time (the classic
 * split-read-defeats-hoist / duplicate-read pattern, already
 * SOTN-sanctioned, .claude/rules/no-new-park-categories.md). Our C source
 * already writes these as separate literal `*(s32 *)(obj + 0xC0)`
 * expressions (not through a shared named variable), so GCC does NOT
 * CSE-merge them in the built object -- confirmed by grepping the s5 dump
 * for the `lw` sequence around the pt0[2]/pt1[1] stores, which shows two
 * distinct loads exactly like target. The remaining 81-point residual is
 * therefore NOT primarily about missing redundant reads (those already
 * match); it's back to register allocation (build_insns 197 vs target
 * 204 -- a 7-insn shortfall, plus whatever weighted reg/reorder diffs make
 * up the rest of 81).
 * ---------------------------------------------------------------------
 * s4 (permuter modality): body UNCHANGED from s2/s3 at the time (before
 * this session's structural improvement). Re-confirmed floor 106 before
 * permuting. Ran a first-ever directed permuter campaign (44,294
 * iterations, 24 min, base permuter score 5235) on that chassis -- best
 * find 4103/5235 (21.6% reduction), plateaued, and the best form itself
 * is a type-broken mutation, not a valid lever. KILLED (instance):
 * permuter-as-primary-lever does not close (or meaningfully narrow) this
 * residual on the pre-s5 chassis. See hypotheses.md [s4] for the full
 * workspace/campaign writeup.
 * ---------------------------------------------------------------------
 * s3 (structural modality): re-applied the s2 body + the func_80053614
 * prerequisite fix, re-confirmed score 106 (baseline WITHOUT the fix is
 * 147 — the fix is load-bearing, not optional). Tried two DECLARATION-
 * order probes (flags-before-obj; r1-declared-next-to-flags) — both
 * measured ZERO effect on the obj/flags/r1 register assignment or score.
 * ---------------------------------------------------------------------
 * PREREQUISITE CHANGE IN THE SAME TU (already applied to src/text1b.c,
 * verified byte-neutral — sandbox func_80053614 --disable all still
 * scores 0/32 after the change):
 *   func_80053614's return type was `void`; changed to `s32` with
 *   `return func_80052D00(arg2, arg3);` as its last statement (was a
 *   bare call). func_80053614's own asm (asm/funcs/func_80053614.s)
 *   ends `jal func_80052D00` immediately followed by the epilogue with
 *   no further $v0 traffic, so $v0 falls through to the caller
 *   regardless of the C-level return type — this is why the change is
 *   byte-neutral for func_80053614 itself. func_80056CB8's two calls to
 *   it DO consume the return value ($v0 feeds `move $22,$2` /
 *   `sll $2,$2,1` in the target asm), so the void declaration must be
 *   fixed for this candidate to type-check and to carry the value.
 *
 * REMAINING RESIDUAL (81) — STILL REGISTER ALLOCATION / SCHEDULING.
 * Frame layout still matches exactly (168/0xA8 bytes total). The
 * instruction-sequence match is now even closer to target in the pt0/pt1
 * block (see WHY THIS WORKS above). build_insns is 197 vs target 204 (a
 * 7-insn shortfall).
 *
 * s6 EVIDENCE (via `python3 tools/objdiff.py tmp/sandbox/func_80056CB8/text1b.o
 * build/src/text1b.o`, full capture in
 * tmp/grind/func_80056CB8/s6/objdiff_full2.txt): after the D_800F6610 fix
 * above, the remaining diff is ALL register-name substitutions (a global
 * rotation: ours uses s3/s6/s0/s1/... where target uses a different but
 * internally-consistent assignment — classic register-rotation-cluster
 * shape, see [[no-new-park-categories]] — NOT itself a new lever) PLUS one
 * concrete shape difference worth chasing next: in the code==4 tail
 * (the y/hit1[1] >= 0x3E9 nested-if that sets code=5), our build emits
 * ONE `bgez` branch for the outer `if (y - hit1[1] >= 0)` while target
 * emits `bltz` + `beqz` as TWO separate branches, and target additionally
 * has FOUR extra `addu v0,s7,s6` (recomputing the `arg0+i` store address)
 * instructions — one duplicated into the delay slot of EACH of the 4
 * exit branches reaching the shared `sb $s0,0x444($v0)` store at
 * .L80056F98/.L80056F94 (see asm/funcs/func_80056CB8.s:160-206), plus a
 * `j TGT` we don't have. This is reorg.c duplicating the shared store's
 * leading address-computation into multiple predecessor delay slots
 * (profitable-duplication-into-delay-slot, a real GCC reorg.c mechanism,
 * NOT under direct C control) — a `goto`-based rewrite of this exact tail
 * measured BYTE-IDENTICAL to the nested-if form on our chassis (see TRIED
 * AND REVERTED above), so the branch-topology difference is NOT caused by
 * missing gotos in our C; it's something upstream (register pressure /
 * instruction scheduling earlier in the function) that changes whether
 * reorg finds these delay slots profitable to fill. NEXT SESSION: this
 * looks like a scheduler-tie class (5 exit branches vs our fewer/merged
 * branches for the same logic) — read tmp/grind/func_80056CB8/dumps/text1b.sched
 * around the code==4 tail's RTL to see whether reorg's duplication
 * decision is gated by a specific insn count/priority threshold that a
 * local restructuring elsewhere in the function (NOT this tail) could
 * shift. Do NOT re-try the goto-vs-nested-if axis on this exact tail —
 * measured neutral, already killed.
 *
 * The register-rotation portion of the residual: re-dump .greg/.lreg for
 * the pseudo->hardreg map on THIS (s6) chassis (pseudo numbers renumber
 * every structural edit, so s3's recorded map is stale) and diff
 * instruction-by-instruction against asm/funcs/func_80056CB8.s from
 * .L80056D94 through the func_80053614 call to find what's still
 * missing/extra, rather than reusing any earlier session's pseudo map.
 *
 * PROVENANCE: this file's C body is inherited verbatim (plus the [S7]
 * work-size fix, the func_80053614 signature fix, and this session's
 * pt0/pt1 store-batching reorder) from
 * memory/grind/func_80056CB8/authored-draft-2026-08-18.c, itself derived
 * from m2c + hand analysis in
 * memory/grind/func_80056CB8/authored-notes-2026-08-18.md. See that
 * file's SYMBOL / TYPE MAPPING and SUSPICIOUS SPOTS sections for the
 * full derivation of every field offset and the *0x7D fallback multiplier.
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
        s32 ang;
        s32 scale;
        s16 *sin_p;
        s16 *cos_p;
        s32 x;
        s32 z;
        s32 r1;
        s32 r2;
        s32 code;

        obj = arg0;
        flags = (&D_8009A821)[i * 2] << 8;
        if ((flags & 0x1000) != 0) {
            obj = *(s32 *)arg0;
        }

        if (*(u16 *)(arg0 + 0x6A) == 0x13 || *(u16 *)(arg0 + 0x6A) == 6) {
            ang = flags + *(s16 *)(obj + 0x1CA);
        } else {
            ang = flags + ratan2(D_800F6608.w0 - *(s32 *)(obj + 0xF4),
                                  D_800F6610 - *(s32 *)(obj + 0xFC));
        }

        sin_p = &Judge + (ang & 0xFFF);
        cos_p = &Judge + ((ang + 0x400) & 0xFFF);
        scale = (&D_8009A820)[i * 2] << 8;
        x = *(s32 *)(obj + 0xB8) + ((scale * *sin_p) >> 12);
        z = *(s32 *)(obj + 0xC0) + ((scale * *cos_p) >> 12);
        pt0[0] = *(s32 *)(obj + 0xB8);
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x320;
        pt0[2] = *(s32 *)(obj + 0xC0);
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) - 0x320;
        pt1[2] = z;

        r1 = func_80053614(pt0, pt1, (s32)hit0, (s32)work, 0x1F8002B8);
        if (r1 != 0) {
            x += (*sin_p * 0x7D) >> 8;
            z += (*cos_p * 0x7D) >> 8;
        }

        pt0[0] = x;
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x834;
        pt0[2] = z;
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) + 0x1004;
        pt1[2] = z;

        r2 = func_80053614(pt0, pt1, (s32)hit1, (s32)work, 0x1F8002B8);

        code = (r1 | (r2 << 1)) + 1;
        if (code == 3) {
            if (hit1[1] - *(s32 *)(obj + 0xBC) < 5) {
                code = 0;
            }
        } else if (code == 4) {
            s32 dx = hit0[0] - *(s32 *)(obj + 0xB8);
            s32 dz = hit0[2] - *(s32 *)(obj + 0xC0);
            if (0x3D0900 < dx * dx + dz * dz) {
                s32 y = *(s32 *)(obj + 0xBC);
                if (y - hit1[1] >= 0) {
                    if (y - hit1[1] >= 0x3E9) {
                        code = 5;
                    }
                } else {
                    if (hit1[1] - y >= 0x3E9) {
                        code = 5;
                    }
                }
            }
        }
        *(s8 *)(arg0 + 0x444 + i) = (s8)code;
    }
}
