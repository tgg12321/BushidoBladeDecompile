# Hypothesis ledger — func_80049A2C

## Frontier after s1 recon

### H1 (frontier) — replace `dummy[2]` with a live-then-DCE'd scalar (phantom-frame-slots pattern)
- **Mechanism:** per phantom-frame-slots-gcc272, GCC 2.7.2 reserves
  `get_frame_size()` bytes for locals it later register-allocates away — no
  store emitted. A live scalar assignment whose result the RA folds into a
  reg still bumps `vars` by 4 (aligned to 8). Delivers the +8 frame slot
  target has, without any dead-declaration/dead-array cheat construct.
- **Next probe:** try `int fp_slot = temp_v1 & 0xFF;` (or similar live use of an
  existing value) declared alongside the other locals; measure vars= in a cc1
  `-da` .frame dump and sandbox score. If frame goes to 0x30 with score 0,
  candidate-ready.

### H2 (frontier) — widen an existing narrow local to force a stack temp
- **Mechanism:** the current body has s16 locals (`a1_val`, `new_var2`) and
  various pointer aliases. Widening `new_var2` to s32 or introducing an s32
  local that captures an intermediate (`s32 tmp = (*p_anim) * 2;`) may push
  `get_frame_size()` above 0 for the same phantom-frame reason.
- **Next probe:** substitute each narrow local one at a time, measure sandbox
  + cc1 `.frame` vars=; correlate against the +8-frame target.

### H3 (frontier) — restructure the two obj-init blocks to share an intermediate
- **Mechanism:** the code initializes two consecutive 0x68-byte objects with a
  near-identical field pattern. A local capturing the shared computation
  (e.g. `s32 hdr = (a1_val) | ((*p_anim) * 2);` or a shared pointer) may
  land as a stack temp regardless of DCE outcome, replicating the frame
  reservation via ordinary source structure rather than a padding decl.
- **Next probe:** build a variant that hoists the `a1_val = (*p_anim) * 2`
  recomputation into a single local reused across both blocks; measure.

## Killed / eliminated (this session)
- `char new_var4;` — score contribution 0 → **just delete**.
- Empty `if (a1_val) { }` — score contribution 0 → **just delete**.
- `(new_var = new_var3)` inline-assign + `int new_var;` decl — score
  contribution 0 → **just delete** (write `obj + new_var3` directly).

## [s1] char new_var4 declaration is pure noise (no codegen effect)
- mechanism: Unused scalar; GCC DCEs the decl, no frame reservation, no insn emitted.
- probe: Delete `char new_var4;` + `(void) new_var4;` alone (keep dummy, empty-if, inline-assign). Run sandbox --disable all.
- result: score=0 (was 0). Zero delta.
- verdict: KILLED

## [s1] Empty `if (a1_val) { }` is pure noise (no codegen effect)
- mechanism: GCC evaluates a1_val (already live), empty body DCE'd, branch DCE'd.
- probe: Delete the empty-if line alone (keep dummy, new_var4, inline-assign). Run sandbox --disable all.
- result: score=0 (was 0). Zero delta.
- verdict: KILLED

## [s1] Inline-assign `(obj + (new_var = new_var3))` + `int new_var` decl is pure noise
- mechanism: new_var is a dead scalar; the assign folds; expression collapses to `obj + new_var3` which is what the plain-C spelling emits anyway.
- probe: Rewrite as `*((s16 *)(obj + new_var3)) = 0;` and drop `int new_var;` (keep dummy, new_var4, empty-if). Run sandbox --disable all.
- result: score=0 (was 0). Zero delta.
- verdict: KILLED

## [s1] s32 dummy[2] IS load-bearing; removing it alone jumps score to 12
- mechanism: Target frame = 0x30 = ALIGN8(vars) + 16 args + 24 gp-regs → vars = 1..8. dummy[2] provides 8 bytes of get_frame_size() → sp-relative offsets shift +8 across the whole body when removed.
- probe: Delete `s32 dummy[2];` + `(void) dummy;` alone (keep the other three). Run sandbox --disable all.
- result: score=12, build_insns=126, cheat_asm_stripped=396 (was 397). Frame collapse propagates as offset diffs across every sp-relative access.
- verdict: CONFIRMED

## [s1] Combined cleanup (delete all four constructs) yields the same score as removing dummy[2] alone (12), confirming dummy is the sole load-bearing item
- mechanism: Additive score contribution of the three noise constructs is 0; only dummy[2] moves the needle.
- probe: Apply all four deletions together. Run sandbox --disable all.
- result: score=12, build_insns=126.
- verdict: CONFIRMED

## [s2] Dead HImode-bitwise pair (s16 fp_a = *p_anim; s16 fp_b = (s16)temp_v1; if ((fp_a & ~fp_b) & 1) {(void)fp_a;}) inserted after the fade-check triggers the reload/alter_reg stale-ref phantom slot.
- mechanism: phantom-frame-slots-gcc272 s3 mechanism: combine eliminates the redundant HImode->SImode sign-ext on paradoxical subreg; flow's stale reg_n_refs makes alter_reg reserve a frame slot.
- probe: Applied over baseline (dummy[2] removed, sandbox=12). Two variants: (A) mixed lh + lbu-derived pair; (B) two independent lh loads via *p_anim and *((s16*)new_var8).
- result: Both A and B: score 12, build_insns 126 (unchanged from dummy-removed baseline). The if-block is fully DCE'd BEFORE combine, so no sign-ext to elide, no stale ref, no phantom.
- verdict: KILLED

## [s2] Widen a1_val from s16 to s32 forces a phantom-frame slot via changed RTL mode/register-class.
- mechanism: H2 (state.json frontier) — mode widening pushes a spill into locals via the same phantom mechanism.
- probe: s16 a1_val -> s32 a1_val; measure sandbox.
- result: Score 12, build_insns 126. No frame effect; the sh-truncating stores still allow scalar promotion.
- verdict: KILLED

## [s2] Widen new_var2 from s16 to s32 forces a phantom-frame slot.
- mechanism: H2 sibling probe.
- probe: s16 new_var2 -> s32 new_var2; measure sandbox.
- result: Score 13, build_insns 126. STRICTLY WORSE than baseline (new_var2 codegen diverges from target).
- verdict: KILLED

## [s2] Live HImode-bitwise embedded in a1_val's computation (constant-fold-neutral): a1_val = (fp_a*2) | ((fp_a & ~fp_b) & 0) survives dead-code elimination and triggers the phantom.
- mechanism: Same as H1 but with the bitwise expression flowing into a live variable used in a real store.
- probe: Inserted at the a1_val assignment site; measure sandbox.
- result: Score 12, build_insns 126. GCC's constant-fold eliminates the '& 0' clause at the tree level, before combine sees it. No phantom.
- verdict: KILLED

## [s2] H3 — share a1_val across the two obj-init blocks (compute once, drop the recomputation) forces the value into a callee-save/frame slot across the func_800417D0 call.
- mechanism: Extending a1_val's live range across a call demands a stack home or callee-save; per s1 frontier note about shared intermediate.
- probe: Removed the second `a1_val = (*p_anim) * 2;` recomputation; measure sandbox.
- result: Score 34, build_insns 123 (LOST 3 target insns). Target contains a genuine second `lh 0x0($s3); nop; sll $a1,1` sequence at 3A2CC-3A2D8 — the recomputation is real target bytes, not compiler-hallucinated. Cannot share.
- verdict: KILLED

## [s3] H4 — pinpoint via cc1 -da the exact reload/alter_reg trigger conditions in a MATCHING BUILD of func_80049A2C (running with dummy[2] in place); if any pseudo already sits in reg_n_refs>0 && reg_renumber<0 && no-hard-reg state cheat-freely, the phantom is already firing on live code and can be replicated in a form without dummy[2].
- mechanism: Directly instrument the reload pass on the working (dummy[2]-carrying) baseline via project-flag cc1 -da to dump greg/lreg/flow/combine. If a pseudo denied a hard reg surfaces, replicate its entry condition without dummy[2].
- probe: Wrote tmp/grind/func_80049A2C/s3/build_da.sh (mirrors project cc1 flags exactly: -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin) targeting src/text1b.c whole-TU with -da. Sliced func_80049A2C from text1b.i.greg / .lreg / .flow / .combine via slice.py. Ran the counterfactual by editing src to remove dummy[2] and re-dumping. Compared the two greg passes.
- result: Dummy-IN baseline: .frame vars=8, 7 pseudos (89 123 133 141 94 73 81) — ALL allocated hard regs (73→17, 81→19, 89→3, 94→3, 123/133/141→65); only spill is hard-reg $a3 for insn 135 (outgoing-arg passing, not a pseudo frame slot); no reload/alter_reg reservation emitted. Dummy-OUT counterfactual: .frame vars=0, IDENTICAL greg dispositions (same 7 pseudos, same hard-reg assignments, same conflicts, same $a3 spill for insn 135). Delta between the two greg passes: ZERO. The phantom-slot mechanism does not fire anywhere in this function's pipeline; the 8-byte slot is 100% attributable to dummy[2]'s aggregate declaration (mips.c compute_frame_size reserves 8 bytes for s32[2] unconditionally at declaration).
- verdict: KILLED

## [s4] H8 — struct-typed aggregate `struct { s32 a; s32 b; } dummy;` + `(void) dummy;` reaches the +8 frame slot with a semantically-distinct spelling that GCC 2.7.2 might scalarize into scalars (dropping to score=12 like s32 dummy;) OR that a reviewer might treat as a distinct sanctioned family (SOTN struct-shaped dead locals).
- mechanism: GCC 2.7.2 has partial SRA (register-promotion of address-not-taken aggregates via flow_delete_insn). If the two-field struct scalarizes, frame goes to 0 (H8 KILL, matches scalar-dummy). If it doesn't, frame reserves 8 bytes like the array and the reviewer-family question governs.
- probe: Edit src/text1b.c line 1061: replace `s32 dummy[2];` with `struct { s32 a; s32 b; } dummy;` (keep `(void) dummy;` sink and the three s1 noise-drops in place). Run `& tools/wteng.ps1 main sandbox func_80049A2C --disable all`.
- result: sandbox --disable all = 0, target_insns=126, build_insns=126, cheat_asm_stripped=396. GCC 2.7.2 does NOT scalarize the two-field struct — it reserves the same +8 locals frame slot as `s32 dummy[2]` (aggregate decl → mips.c compute_frame_size reserves vars=8 unconditionally). Prong (a) CONFIRMED (same frame). Prong (b) FAILS: struct is unwritten + unread + `(void)`-sinked, identical 'no semantic purpose' defect as `s32 dummy[2]` under [[inline-asm-policy]] expanded cheat catalog + [[no-new-park-categories]] cheats-by-any-spelling 6-test checklist. 2026-07-01 [[dead-vars-local-array]] carve-out is written-never-read arrays with dead stores oracle-present; a fully-dead struct is not within that carve-out and no SOTN precedent found for a fully-dead 8-byte struct pad. Rejected form banked at memory/grind/func_80049A2C/rejected/struct-dummy-h8-same-frame-same-defect.c.
- verdict: KILLED

## [s4] H7-prong — canonical-asm authorization for func_80049A2C is supportable by hand-coded-asm evidence per [[hand-coded-asm-recognition]] STRONG signals (S1/S2/S6).
- mechanism: scan_hand_coded.py --single applies the S1..S8 signal framework to the target asm; STRONG tier requires ≥1 of S1/S2/S6/S7/S8. Certifies whether canonical-asm-refusal is honest before filing OWNER-ESCALATION.
- probe: python3 tools/scan_hand_coded.py --single func_80049A2C
- result: HAND_CODED: tier=LOW score=0/8 (func_80049A2C, 126 insns). No S1 (only 0 multu/mflo pairs), no S2 (no empty-body branches), no S3 (126 insns / 5 spills / 10 distinct regs — well within compiled range), no S4 (max load burst was 3 in 8-insn window), no S5 (no high-similarity siblings jaccard<0.5), no S6 (no BIOS jumptable), no S7 ($sN callee-saves all $sp-saved), no S8 (no redundant mask-before-shift). Canonical-asm-refusal certified: no evidence the original was hand-written. Confirms [[endgame-lock-disposition]] path (b) 'canonical-asm ONLY with hand-coded evidence' is UNAVAILABLE for this function.
- verdict: KILLED

## [s4] S4-mandate permuter — a directed permuter campaign starting from the score=12 dummy-out baseline can find a fundamentally novel closing form (aggregate-free, non-cheat) that reproduces the +8 frame slot via a codegen path the s3 cc1 -da instrumentation missed.
- mechanism: decomp-permuter mutates C source syntactically searching for byte matches; if the aggregate-only mechanism proof is complete, every closing candidate it finds will either (a) re-introduce an aggregate declaration (already-measured cheat family) or (b) plateau at 12. A genuinely novel non-aggregate finder would falsify s3's mechanism proof and open a new axis.
- probe: 1. Set src/text1b.c to the score=12 dummy-out clean baseline (drop dummy[2] + 3 s1-noise). 2. `python3 tools/decomp-permuter/import.py src/text1b.c asm/funcs/func_80049A2C.s` — auto-generated nonmatchings/func_80049A2C/ workspace. 3. `python3 tools/permuter_campaign.py launch --func func_80049A2C --dir nonmatchings/func_80049A2C --label s4-dummy-out -j 4`.
- result: Import succeeded but permuter launch failed rc=1: 'Syntax error in base.c. before: __asm__ at approximately line 1770, column 23 (after PERM expansion)'. Root cause: import.py preserved the `_permuter_ignore_line __asm__(...)` block for canonical-asm sibling func_8004A348 (text1b.c INCLUDE_ASM_USE_MACRO_INC; ~100-line embedded MIPS GTE asm), and the permuter's syntax parser did NOT actually treat the `_permuter_ignore_line` prefix as an ignore directive. Manually stripping that line let permuter parse but the workspace compile.sh (full-TU maspsx pipeline) then died with 'MASPSX: An exception occurred: too many values to unpack (expected 2)' on the fresh TU. A leaner bespoke workspace (tmp/grind/func_80049A2C/s4/build_perm_workspace.sh) hit the same maspsx exception. Setting up a runnable permuter workspace for func_80049A2C requires a project-tooling fix outside the scope of a grind session. Permuter campaign blocker banked as tmp/grind/func_80049A2C/s4/permuter-blocked-summary.txt + tmp/grind/func_80049A2C/s4/permuter-launch-failed.log.
- verdict: KILLED

## [s5] s5-permuter: fresh permuter workspace for func_80049A2C can be built and launched without touching tools/decomp-permuter or tools/maspsx (contract-forbidden).
- mechanism: s4 documented two blockers: (1) permuter's syntax parser does not honor `_permuter_ignore_line` prefix on the canonical-asm sibling func_8004A348 (~100-line embedded GTE __asm__ block in text1b.c via INCLUDE_ASM_USE_MACRO_INC); (2) the import.py-generated workspace compile.sh maspsx pipeline exceptions on the enlarged text1b.c TU. Both blockers require touching tools/decomp-permuter/scripts or tools/maspsx, which the grind-session contract forbids.
- probe: s5: bash tools/wsl.sh 'python3 tools/decomp-permuter/import.py src/text1b.c asm/funcs/func_80049A2C.s' — logged tmp/grind/func_80049A2C/s5/permuter-import-fresh.log. Then bash tools/wsl.sh 'cd nonmatchings/func_80049A2C && bash compile.sh base.c' — logged tmp/grind/func_80049A2C/s5/permuter-compile-fresh.log.
- result: Import warned exactly as s4: 'Syntax error in base.c before: __asm__ at approximately line 1775' — the _permuter_ignore_line prefix is not treated as an ignore directive; import proceeded 'anyway, but expect errors when permuting'. compile.sh died at 'MASPSX: An exception occurred: too many values to unpack (expected 2)' on the enlarged TU. Both blockers reproduced verbatim; neither can be fixed without touching tools/, which the contract forbids.
- verdict: KILLED

## [s5] s5-owner-escalation: filing the OWNER-ESCALATION entry into docs/grind/decisions.md is the only remaining artifact required to make 'owner-gated' disposition valid per the task-brief contract.
- mechanism: Task-brief contract: 'owner-gated' is ONLY valid when a filed OWNER-ESCALATION exists in docs/grind/decisions.md AND every remaining sanctioned axis is measured dead. Every axis measured dead across s1..s4 (phantom-firing H4, phantom-injection H1/H2/H3, scalar-widening, scalar-dummy H6, struct-aggregate H8, scan_hand_coded LOW 0/8). Prior grep(func_80049A2C) on decisions.md returned 4 hits (2 FAIL rulings + 2 headers, 0 OWNER-ESCALATION headers). s5 permuter modality also KILLED above.
- probe: Append an OWNER-ESCALATION entry (in the hirahira_w_frie / saTan0Init / cpu_side_move_dir_4 format) to docs/grind/decisions.md presenting: (a) canonical-asm authorization option and its plainly-stated cost under the owner's 2026-07-20 criterion; (b) INCOMPLETE-owner-accepted option per endgame-lock-disposition; complete s1..s5 lever-exhaustion ledger; plain statement of no-SOTN-precedent for a fully-dead 8-byte pad; reference the .claude/rules/endgame-lock-disposition.md 2026-07-20 standing policy. Verify grep-able. Draft banked at tmp/grind/func_80049A2C/s5/escalation-entry.md.
- result: Entry appended at docs/grind/decisions.md:954 (file grew 951 → 975 lines). Verified: grep '## 2026-07-20 — func_80049A2C.*OWNER-ESCALATION' matches once, at line 954. The escalation cites the 2026-07-19 23:46 + 2026-07-20 00:36 Judge FAILs, presents both options honestly, includes the full s1..s5 exhaustion ledger with pass/probe/result quotations, plainly acknowledges no SOTN precedent for a fully-dead 8-byte pad, references the 2026-07-20 endgame-lock-disposition rule, notes precedent saTan0Init + cpu_side_move_dir_4 both ruled option (b) on 2026-07-20, and does NOT self-resolve. Baseline reconfirmed post-file: sandbox --disable all = 0, target_insns=126, build_insns=126.
- verdict: CONFIRMED
