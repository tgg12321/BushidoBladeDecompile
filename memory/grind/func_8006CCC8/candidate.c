/* func_8006CCC8 — src/text1b.c — session s6 (rederive modality)
 * Sandbox --disable all floor at end of s6: 18 (target_insns=189, build_insns=188).
 * DOWN FROM 23 (s5 floor) via two structural fixes this session, both pure
 * C, no FAKE constructs, no new sanctioned-family claims.
 *
 * This candidate IS applied to src/text1b.c as of end of s6 (2026-09-16) — it
 * is the resident WORKING-TREE body (never committed; asm-until-matched keeps
 * main as INCLUDE_ASM until sandbox reaches 0).
 *
 * s6 changes from the s5 body (measured this session):
 *
 * FIX 1 (23 -> 20): rewrote the repeated `*(s16 *)(mask + (u8 *)D_800A34FC +
 * 0x28)` byte-pointer-arithmetic address expression (at all 7 use sites) as
 * `((s16 *)((u8 *)D_800A34FC + 0x28))[i]` — typed s16-array indexing by the
 * outer loop counter `i` directly, instead of a separately-tracked `mask`
 * local incremented by 2 each iteration. `mask` was semantically ALWAYS
 * `i * sizeof(s16)`; expressing it as array indexing is the more direct,
 * equally-truthful spelling of the same value and removes a redundant local.
 * Measured effect: this flips the emitted `addu` operand order at all three
 * `D_800A34FC`-relative address computations from base-pointer-first
 * (`addu $4,$2,$19` — our s5 shape) to index-first (`addu $4,$19,$2`),
 * matching target's own `addu $a0,$s3,$v0` (mask/index register first, base
 * second) at asm/funcs/func_8006CCC8.s:76 (and the two other occurrences).
 * This is the EXACT residual s4's evidence had identified and measured DEAD
 * as a source-level lever under the old byte-pointer-arithmetic spelling
 * (GCC's commutative-add canonicalization ignored `mask + base` vs
 * `base + mask` operand order under that spelling) — restated as typed array
 * indexing, the canonicalization path differs (array subscripting lowers
 * through a MULT-then-PLUS shape rather than a direct pointer-int PLUS) and
 * the operand order DOES follow the source spelling this way. Confirmed via
 * `pwsh tools/grinder/dump.ps1 func_8006CCC8` + hand-read of the regenerated
 * `tmp/grind/func_8006CCC8/dumps/text1b.s` at the three address-computation
 * sites.
 *
 * FIX 2 (20 -> 18): reordered the outer `for` loop's update-clause from
 * `i++, shift += 0x10` to `shift += 0x10, i++` (pure reordering of two
 * independent, side-effect-only compound assignments — both still execute
 * every iteration, C's comma operator sequences left-to-right exactly as
 * written, no semantic change). Target's own tail sequence
 * (asm/funcs/func_8006CCC8.s:189-193) computes `shift`'s register (`$s2`)
 * update BEFORE `i`'s register (`$s1`) update, with the (former, now-removed)
 * `mask`-tracking register's update pushed into the loop-back-edge branch's
 * DELAY SLOT — i.e., target's source almost certainly wrote the `mask`-like
 * update LAST in its own for-loop update-clause. Since FIX 1 eliminated the
 * explicit `mask` variable (folded into array indexing), there is no longer
 * a THIRD update-clause term to place last; matching target's `shift`-before-
 * `i` relative order for the two REMAINING terms was the closest available
 * spelling and measured a clean 2-point drop with the delay-slot fill
 * landing correctly (confirmed via the WSL `engine.score.normalized_insns`
 * masked-opcode diff, `tmp/grind/func_8006CCC8/s6/diff_probe.py`).
 *
 * REJECTED THIS SESSION (see memory/grind/func_8006CCC8/rejected/):
 *   - byte17-hoist-on-floor20-chassis.c — re-testing the s3-style hoisted
 *     `byte17` local (single read of *(rec+0x17) before the i==0/else
 *     branch, matching target's own unconditional-both-loads asm shape) on
 *     the floor-20 chassis: REGRESSED 20 -> 36. The s5 split-read-into-arms
 *     form (this candidate's actual shape) remains strictly better on every
 *     chassis measured across this whole ledger.
 *   - flat-target-literal-compare-order-dispatch.c — rewriting the field28
 *     dispatch as a flat if/else-if chain in target's literal runtime
 *     compare order (==3, then <4, then ==4, matching the asm's beq/slti/beq
 *     read order) REGRESSED 20 -> 54. The nested form (`if (field != 4) {
 *     if (field != 3) { if (field < 4) {...} } else {...} } else {...}`,
 *     this candidate's actual shape, from s4) does not preserve the literal
 *     compare order but DOES match target's physical block LAYOUT, which is
 *     what actually matters for GCC 2.7.2's nested-if/else emission. Also
 *     side-probed the mirror nested form (outer test `field != 3` instead of
 *     `field != 4`): scored an IDENTICAL 18 on this chassis (neutral, not
 *     separately saved) — the two nested-outer-test choices are
 *     interchangeable here; kept `field != 4` for s4-lineage continuity.
 *
 * KNOWN REMAINING GAP (floor 18, from this session's
 * tmp/grind/func_8006CCC8/s6/diff_probe.py masked-opcode diff):
 *   (a) A ~8-insn field28-dispatch-chain mismatch (diff indices ~86-95):
 *       target's asm inserts extra insns around the field==4 compare / the
 *       field>4 "skip" tail (`li v0,0x40; j @; addiu s5,s5,4`) that our
 *       build's nested-if physical layout does not reproduce in that exact
 *       shape, even though the overall block ordering (default nearest,
 *       then +0x1A case, then +0x1D case) matches per s4's original
 *       full-disassembly read. NOT re-diagnosed at the RTL/pass level this
 *       session — next session should re-run PASS ATTRIBUTION
 *       (`pwsh tools/grinder/dump.ps1 func_8006CCC8`, already fresh as of
 *       this session's dumps in tmp/grind/func_8006CCC8/dumps/) and read
 *       `.jump`/`.jump2` for this region specifically (cross-jump block
 *       merging is the most likely candidate mechanism given the "extra"
 *       vs "missing" insn shape in the diff).
 *   (b) The j-loop split-read-vs-hoisted-read residual (diff indices
 *       ~125-132, ~155-162): target keeps the s3-style single hoisted
 *       `*(rec+0x17)` read before the i==0/else branch; this candidate uses
 *       the s5 split-read-into-arms form, which is WORSE in raw insn-shape
 *       match to target but scores BETTER overall (confirmed both directions
 *       this session — see the byte17-hoist rejected form above). This is a
 *       genuine register-allocation trade-off, not a simple "match target's
 *       shape" fix — the two forms were measured head-to-head twice now
 *       (s5's original 39-vs-23 test, and this session's 20-vs-36 retest)
 *       and split-read wins both times on this chassis lineage. Candidate
 *       for `tools/ra_solver` / `inverse_compose.py classify` in a future
 *       solver-modality session, as s4/s5 already flagged.
 *
 * Structure derived from m2c --valid-syntax over asm/funcs/func_8006CCC8.s
 * (s1) + full manual disassembly read (s3) + nested if/else + named-
 * intermediate restructure (s4) + directed-permuter split-read fix (s5) +
 * this session's array-index addressing rewrite + update-clause reorder
 * (s6). Field layout cross-checked against sibling func_8006CBD4 (same TU,
 * same D_800A34FC/D_800A3524 struct idiom, unchanged since s1). Sibling
 * func_80056CB8 (same TU, floor 38 through its own s71) reached COMPLETED-C
 * at its s72 (2026-09-16) — re-checked this session (s6, forced rederive
 * from the sibling-progress notice): its matched body
 * (`void func_80056CB8(s32 arg0)`, src/text1b.c:1813) is a hit-detection
 * routine over `pt0`/`pt1`/`hit0`/`hit1` stack arrays with NO reference to
 * `D_800A34FC` or `D_800A3524` and a completely disjoint 1-arg signature —
 * remains signature- and struct-idiom-level disjoint, consistent with every
 * prior cross-check since its own s54. No transplantable construct; the two
 * fixes this session were independently re-derived from func_8006CCC8's own
 * residual diagnosis, not sourced from the sibling.
 */
s32 func_8006CCC8(s32 *arg0, s32 *arg1, s16 arg2) {
    s32 i;
    s16 lim;
    s32 shift;
    s32 nib;
    s32 fade;
    s32 j;
    u8 *rec;
    s32 ret;
    s32 t;
    s32 masked;
    s16 field;

    ret = 0;
    if ((*(s32 *)((u8 *)D_800A34FC + 0x28) == 0x50005) && (*arg1 & 0x400040)) {
        func_8005C650(1, 0x7F, 0x7F);
        ret = 1;
    }

    i = 0;
    nib = 0xF;
    fade = 0;
    shift = 0;
    for (; i < 2; shift += 0x10, i++) {
        t = arg2;
        lim = ((t >> i) & 1) ? 4 : 5;

        if (*arg1 & (0x1000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (((s16 *)((u8 *)D_800A34FC + 0x28))[i] <= 0) {
                ((s16 *)((u8 *)D_800A34FC + 0x28))[i] = lim;
            } else {
                ((s16 *)((u8 *)D_800A34FC + 0x28))[i] = (s16)(((s16 *)((u8 *)D_800A34FC + 0x28))[i] - 1);
            }
        } else if (*arg1 & (0x4000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (((s16 *)((u8 *)D_800A34FC + 0x28))[i] >= lim) {
                ((s16 *)((u8 *)D_800A34FC + 0x28))[i] = 0;
            } else {
                ((s16 *)((u8 *)D_800A34FC + 0x28))[i] = (s16)(((s16 *)((u8 *)D_800A34FC + 0x28))[i] + 1);
            }
        }

        field = ((s16 *)((u8 *)D_800A34FC + 0x28))[i];
        if (field != 4) {
            if (field != 3) {
                if (field < 4) {
                    if (field >= 0 && (*arg1 & (0xF0 << shift))) {
                        func_8005C650(0, 0x7F, 0x7F);
                        t = i;
                        func_8006CBD4(t, *arg1);
                    }
                }
            } else {
                if (*arg1 & (0x40 << shift)) {
                    func_8005C650(1, 0x7F, 0x7F);
                    for (j = 0; j < 3; j++) {
                        rec = (u8 *)D_800A3524 + j;
                        masked = *(rec + 0x1A) & (nib << fade);
                        if (i == 0) {
                            *(rec + 0x17) = (u8)((*(rec + 0x17) & 0xF0) + masked);
                        } else {
                            *(rec + 0x17) = (u8)((*(rec + 0x17) & 0xF) + masked);
                        }
                    }
                }
            }
        } else {
            if (*arg1 & (0x40 << shift)) {
                func_8005C650(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    masked = *(rec + 0x1D) & (nib << fade);
                    if (i == 0) {
                        *(rec + 0x17) = (u8)((*(rec + 0x17) & 0xF0) + masked);
                    } else {
                        *(rec + 0x17) = (u8)((*(rec + 0x17) & 0xF) + masked);
                    }
                }
            }
        }
        fade += 4;
    }
    return ret;
}
