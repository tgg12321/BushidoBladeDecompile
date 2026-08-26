/* 2026-08-24 MIGRATION NOTE: HEAD is now INCLUDE_ASM (asm-until-matched
   wave 1, 2026-08-19); rules retired. Statements below about "HEAD",
   rules carried, or "applied to src" describe the PRE-MIGRATION tree
   (banked at retired-chassis-2026-08/body.c). Paste this body over the
   INCLUDE_ASM line before any sandbox re-measure. */
/* func_80022F34 — candidate (grind s2, structural, 2026-07-23)  floor STILL 11
 *
 * CHANGED from s1's candidate (which was vH, the frame-correct/CSE form). s2
 * proved that THIS form (base = HEAD's named-temp body) has a BYTE-PERFECT body
 * vs target — 2x per-access `lw D_801027BC(idx)` + in-place a0 reload
 * `lw $4,0($4)` in $a0 — and its ENTIRE residual 11 is a single defect: a +8
 * phantom frame slot (reg100 combine strand). Remove that ONE artifact and the
 * floor drops to 1 (the maspsx nop, retirable via maspsx_label_nop_funcs.txt).
 * That makes this the highest-value launch point: 1 combine-artifact from match.
 *
 * This IS the current src/code6cac.c HEAD body (applying it is a no-op from HEAD).
 * NB: HEAD's 10 frame-offset regfix substs currently CORRECT this phantom frame
 * (the INCOMPLETE cheat state). A COMPLETED-C match needs the phantom gone in
 * PURE C (no substs), then retire the 10 substs + allowlist the 1 nop.
 *
 * THE ONE DEFECT (s2, fully characterized):
 *   reg100 = &D_801027BC + idx1*20 (val1's address). combine folds it into
 *   `lw val1,D_801027BC(idx1*20)` (per-access, GOOD) but strands a bare
 *   `(use reg:SI 100)` UPSTREAM of its own def, at the switch-merge code_label
 *   (before the val1 sub-block's NOTE_INSN_BLOCK_BEG). reg100 then gets no hard
 *   reg -> alter_reg reserves an unreferenced slot -> vars=8 (+8 frame).
 *   Only val1 strands (long lifetime, loaded early in a sub-block, USED in the
 *   outer-scope call); val2 (loaded late, consumed in-block) folds clean.
 *
 * WHY NOT STRUCTURALLY FIXABLE (s2, ~24 forms measured):
 *   per-access <-> phantom are COUPLED. val1 loaded early-in-subblock => per-access
 *   + strand (this form). val1 loaded late => cse2 shares the `la` base => vars=0
 *   but NOT per-access (vH/vMIRROR, also 11). val1 before the switch => vars=0 +
 *   per-access but gross reorder => sandbox 28. No structural val1-placement escapes.
 *
 * s3 (structural, 2026-07-23): confirmed base is STILL the best form (floor 11).
 * s3 killed the control-boundary re-association axis (do-while(0) around the
 * switch OR the val1 block — vTAIL/vVALDW — both keep vars=8; strand is
 * CFG/scope-invariant since do-while(0) is folded before combine). Both
 * structural axes (s2 placement + s3 control-boundary) are now dead.
 *
 * NEXT LEVER (non-structural): permuter / cse.c study to remove the reg100 strand
 * (H-A option b) OR force cse2 symbol re-materialization on a frame-correct base
 * (H-D). Residual is NOT sp-offset-only -> permuter scorer is valid here.
 *
 * s4 (permuter, 2026-07-23): H-A permuter axis KILLED. Two chassis, ~35k iters,
 * --stack-diffs (phantom frame visible). base (weighted 174, byte-perfect body):
 * 30640 iters, ZERO sub-174 finds — random codegen mutation cannot remove the
 * reg100 combine strand. vH (weighted 1250, frame-correct CSE'd): best find 224,
 * only re-finds the known base/vSPLIT/vH classes, never target's per-access+no-
 * phantom. Corroborates s1-s3: the coupling is a fork-level cse2+combine
 * interaction, not C-reachable. base remains the best form (this file).
 *
 * s6 (forensics, 2026-07-23): mechanism PINNED and OWNER-ESCALATION filed.
 * The +8 phantom is GCC 2.7.2 combine.c:10836-10846 (distribute_notes:
 * REG_DEAD note homeless + hit CODE_LABEL -> emit `(use reg100)` after the
 * switch-merge code_label 85) + combine.c:52-59 (reg_n_refs left stale). reg100
 * = val1's folded address; greg gives it EMPTY conflicts + NO hard reg ->
 * reload homes it to a stack slot -> vars=8. The per-access FOLD and the strand
 * are the SAME combine event (inseparable for this dataflow). Two new forensic
 * forms KILLED (rejected/bytepointer-address-arith-strand-persists.c,
 * named-address-pointer-strand-moves-to-reg83-vars8.c). scan_hand_coded LOW 1/8.
 * ALL sanctioned axes (structural s2/s3, permuter s4/s5, forensics s6) dead;
 * owner-gated (docs/grind/decisions.md 2026-07-23, names func_80022F34).
 * base remains the best form (this file).
 *
 *
 * s7 (solver, 2026-08-26): floor re-measured at 11 on the post-migration chassis
 * (rules_dropped 0 � the 11 is now fully honest, no longer rule-masked).
 * SOLVER AXIS FORECLOSED: goal_from_tgt.py classify (the object-level path;
 * inverse_compose/mkasm_honest cannot serve an INCLUDE_ASM-routed function)
 * reports FIRST DIVERGENCE: PRE-RA, "next tool: none", on BOTH chassis � the
 * streams differ by a frame size / a cse2 materialisation, never by a register
 * seat or an emission order, so ra_solver and sched_solver have nothing to
 * invert. Do not re-run them.
 * s7 also CORRECTS s6: the switch-merge CODE_LABEL is NOT necessary for the
 * strand (vIF / vIF2 / vTERN / vMIN � no switch, no jump table, no case-merge
 * label � all still vars=8, strand=1), so "eliminate the switch-merge label" is
 * a dead lever, not a frontier. And it replaces the val1-placement/lifetime
 * framing with a measured 13-form invariant: separating EITHER D_801027BC load
 * into its own named s32 temp gives per-access + strand + vars=8; keeping BOTH
 * in the single call expression gives a shared `la` + no strand + vars=0.
 * A SECOND, COMPLEMENTARY 11-chassis is banked alongside this file:
 * memory/grind/func_80022F34/chassis-vORIG.c � target-exact frame, prologue,
 * epilogue, loop and switch (39 leading insns identical, vars=0, sp -32), whose
 * whole residual is one 15-insn block where cse2 shares the symbol base instead
 * of re-materialising it per access. THIS file is retained as the canonical
 * candidate (byte-perfect BODY); vORIG is the better handoff chassis for a
 * cse.c cost-model attack. Read both before choosing.
 * Measured: sandbox --disable all = 11, build_insns 69, target 70.
 *
 * s8 (escalation, 2026-08-26): THIS FILE REMAINS THE BEST FORM (floor 11,
 * byte-perfect body). The residual is now CLOSED AT MECHANISM LEVEL rather
 * than by enumeration. s8 measured 6 more forms (19 total, zero exceptions to
 * s7's separation law: definition order, argument order, which value is named,
 * and naming the scaled offset instead of the loaded value are all inert), then
 * isolated the cause with two semantics-broken diagnostics: vDIAG (no CODE_LABEL
 * anywhere in the function) -> strand=0, sp -32; vDIAG2 (loop label only) ->
 * strand=1, sp -40. The +8 slot comes from combine.c:10829-10846, distribute_notes'
 * fallback that emits `(use (reg N))` after a CODE_LABEL when a REG_DEAD note has
 * no home. The fold that produces target's per-access lui/%lo bytes is the SAME
 * event that deletes the address pseudo's only definition, so no anchor can exist
 * for the note; an anchor would need a second reference, and a second reference is
 * precisely what blocks the fold (vORIG's surviving insn 100 proves it). A loop
 * always supplies the label. Target's form (fold AND vars=0, in a loop) is
 * therefore unreachable in this fork for EVERY spelling. s8 also corrects s7's
 * pass attribution: cse shares the symbol in BOTH classes (identical per-pass
 * symbol-ref census); combine, not cse2, is the discriminator.
 * Both endgame-lock gates re-measured and both FAIL (scan_hand_coded LOW 1/8;
 * no PSX SOTN precedent for removing a compiler-added frame slot). Disposition
 * filed in docs/grind/decisions.md (2026-08-26) under the owner's 2026-07-27
 * standing ruling: REFUSED / OWNER-ACCEPTED INCOMPLETE.
 */
void func_80022F34(void) {
    s32 i;
    u16 *tbl;
    s32 offset;

    i = 0;
    tbl = (u16 *)&D_80102778;
    offset = 0;

loop_22F34:
    {
        u8 *a0 = (u8 *)&D_80101EC8 + offset;

        if (*(s16 *)(a0 + 6) != 0) {
            s32 val = D_800A38DC;

            switch (val) {
                case 0:
                    *(s16 *)(a0 + 8) = (&D_80102782)[i] << 4;
                    break;
                case 1:
                case 2:
                default:
                    *(s16 *)(a0 + 8) = *tbl;
                    break;
                case 3:
                    break;
            }

            {
                s16 idx1 = *(s16 *)(a0 + 0x4A);
                s32 val1 = (&D_801027BC)[idx1 * 5];
                a0 = *(u8 **)a0;
                {
                    s16 idx2 = *(s16 *)(a0 + 0x4A);
                    single_game_SetStatusUpData(i, val1, (&D_801027BC)[idx2 * 5]);
                }
            }
        }

        tbl++;
        i++;
        offset += 0x44C;
    }
    if (i < 2) goto loop_22F34;
}
