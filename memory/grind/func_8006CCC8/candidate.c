/* func_8006CCC8 — src/text1b.c — session s5 (permuter modality)
 * Sandbox --disable all floor at end of s5: 23 (target_insns=189, build_insns=188).
 * DOWN FROM 39 (s4 floor) via one structural fix this session, found by a
 * DIRECTED decomp-permuter campaign on a hand-built full-TU workspace
 * (tmp/grind/func_8006CCC8/s5/perm_ws — see below for why a hand-built
 * workspace was required; the s4-blocked `import.py` path is still broken).
 *
 * This candidate IS applied to src/text1b.c as of end of s5 (2026-09-16) — it
 * is the resident WORKING-TREE body (never committed; asm-until-matched keeps
 * main as INCLUDE_ASM until sandbox reaches 0).
 *
 * s5 change from the s4 body (measured this session, both arms):
 *
 * DROPPED the `byte17` named intermediate (`byte17 = *(rec + 0x17);` read once
 * before the i==0/else branch, then reused in both arms) and instead read
 * `*(rec + 0x17)` DIRECTLY at each of the two use sites inside the branch arms.
 * This is the SOTN-sanctioned "duplicate-read into branch arms"
 * ([[split-read-defeats-hoist]]) family — ordinary C, ordinary semantics
 * (the record byte read has no side effects, so reading it once vs. reading
 * it twice — once per arm, each execution taking exactly one of the two arms
 * — is behaviorally identical), no FAKE annotation needed (this family carries
 * no annotation prerequisite, unlike the constant-holder/dead-store carve-outs).
 * Applied symmetrically to BOTH the field28==3 (+0x1A) and field28==4 (+0x1D)
 * inner `for(j...)` record loops, since they are structurally identical
 * (verified: same masked/byte17/i==0 shape in both arms since s4).
 *
 * PROVENANCE: found by permuter iteration `output-870-2` (best find of a
 * ~1900-iteration campaign against the floor-39 chassis; base_score 1480 ->
 * 870 permuter-internal weighted score), then independently APPLIED BY HAND
 * to src/text1b.c and measured with the engine's own sandbox (not trusted
 * from the permuter's own scorer, which uses a different weighted metric —
 * see [[scoring-systems]]) — sandbox confirmed 39 -> 23. The permuter's raw
 * mutation also inserted a stray `;` (empty statement, an artifact of its
 * text-diff mutation engine deleting the `byte17 = ...;` line via replacement
 * rather than clean deletion) — that artifact was NOT carried into the C
 * source; the candidate deletes the `byte17` local and its one statement
 * cleanly, which is the ordinary-C form of the SAME construct.
 *
 * OTHER PERMUTER FINDS THIS SESSION — REJECTED, NOT CHEATS-BY-OMISSION:
 * every other novel find surfaced by ~5.9k combined iterations across two
 * campaigns (floor-39 chassis then re-seeded on the floor-23 chassis) was
 * either (a) a dead-conditional-store / address-of-local RA-steering
 * construct with no semantic purpose (`s32 *new_var; ...; new_var = &masked;
 * ...; *new_var` — Lever-D-shaped, no family covers it, would need a
 * ruling-request, not worth it since it didn't even reach a better score than
 * the accepted split-read form), or (b) an outright INCORRECT mutation that
 * corrupts the outer loop's live `mask` accumulator (`mask = *(rec + 0x17);
 * byte17 = mask;` inside the j-loop — `mask` is read again at the TOP of the
 * NEXT outer-loop iteration via `mask += 2` and the next field28 address
 * computation, so overwriting it inside the inner loop changes runtime
 * behavior, not just bytes for this one input — rejected on correctness
 * grounds regardless of cheat status). Neither was surfaced as a candidate.
 * See memory/grind/func_8006CCC8/rejected/ for both, and the s5 evidence.md
 * entry for full detail.
 *
 * KNOWN REMAINING GAP (floor 23, carried forward from s4's analysis, NOT
 * re-measured this session but still visible in the workspace's objdump
 * diff): the same two residual shapes s4 identified —
 *   (a) THREE `addu` operand-order flips at the `mask + (u8*)D_800A34FC +
 *       0x28` address computations (target mask-register-first, our build
 *       base-pointer-first) — s4 measured this dead as a source-level lever
 *       (GCC's fold-const.c re-canonicalizes the commutative add
 *       independent of source spelling).
 *   (b) A register-allocation difference inside the `for (j...)` record loop
 *       (target keeps the record pointer in $v1 and the mask-shift result in
 *       $a2 — the same register the loop's shift-amount source uses — vs our
 *       build's $a1/$a3 split) — s4 flagged this as a genuine local-alloc.c
 *       register-preference tie, candidate for `tools/ra_solver` /
 *       `inverse_compose.py classify` in a future solver-modality session.
 * These were NOT re-probed this session (the split-read fix already dropped
 * 16 of the 39 floor points); the next session should re-run PASS
 * ATTRIBUTION (`tools/grinder/dump.ps1 func_8006CCC8`) fresh against the
 * floor-23 chassis before assuming s4's diagnosis still applies unchanged.
 *
 * Structure derived from m2c --valid-syntax over asm/funcs/func_8006CCC8.s
 * (s1 provenance) + a full manual read of the target disassembly (s3) + the
 * nested if/else + named-intermediate restructure (s4) + this session's
 * directed-permuter split-read fix (s5). Field layout cross-checked against
 * sibling func_8006CBD4 (same TU, same D_800A34FC / D_800A3524 struct idiom,
 * unchanged since s1). Sibling func_80056CB8 (same TU, floor 38 since its
 * s22, 68 sessions) re-checked this session and remains signature-level
 * disjoint (3-arg pointer-taking s32-return vs its own 1-arg void) — no
 * transplantable lever, consistent with every prior cross-check since s54 of
 * that ledger.
 */
s32 func_8006CCC8(s32 *arg0, s32 *arg1, s16 arg2) {
    s32 i;
    s16 lim;
    s32 shift;
    s32 mask;
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
    mask = 0;
    for (; i < 2; i++, shift += 0x10, mask += 2) {
        t = arg2;
        lim = ((t >> i) & 1) ? 4 : 5;

        if (*arg1 & (0x1000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (*(s16 *)(mask + (u8 *)D_800A34FC + 0x28) <= 0) {
                *(s16 *)(mask + (u8 *)D_800A34FC + 0x28) = lim;
            } else {
                *(s16 *)(mask + (u8 *)D_800A34FC + 0x28) = (s16)(*(s16 *)(mask + (u8 *)D_800A34FC + 0x28) - 1);
            }
        } else if (*arg1 & (0x4000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (*(s16 *)(mask + (u8 *)D_800A34FC + 0x28) >= lim) {
                *(s16 *)(mask + (u8 *)D_800A34FC + 0x28) = 0;
            } else {
                *(s16 *)(mask + (u8 *)D_800A34FC + 0x28) = (s16)(*(s16 *)(mask + (u8 *)D_800A34FC + 0x28) + 1);
            }
        }

        field = *(s16 *)(mask + (u8 *)D_800A34FC + 0x28);
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
