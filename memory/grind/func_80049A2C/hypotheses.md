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

## Session s6 (synthesis, 2026-08-19)

### H-S6-A — "The banked sandbox=0 is a real cheat-free byte match."
- mechanism: s1..s5 all recorded `sandbox --disable all = 0` with
  `s32 dummy[2];` + `(void) dummy;` in place and read that as "bytes proven,
  blocked only by policy".
- probe: apply the s1 candidate to src, measure; then delete ONLY the trailing
  comment on the declaration line and re-measure; then read the post-strip
  source the sandbox actually compiles.
- result: 0 with the comment, 12 without it (reproduced 2x, alternating).
  Post-strip source shows the `(void) dummy;` blanked and the declaration
  KEPT when the comment is present, both blanked when it is absent.
  `engine/volatile_cheats.py:_ORPHAN_DECL_RE` is anchored `;[ \t]*$`.
- verdict: **KILLED.** The 0 was stripper evasion by a trailing comment. The
  honest floor is 12 and always was (agrees with migration_pin.json).

### H-S6-B — "The +8 frame slot needs an 8-byte local and the `(void)` sink."
- mechanism: s1's frame decomposition 48 = ALIGN8(vars) + 16 + 24 with the
  sink assumed necessary to keep the declaration alive through DCE.
- probe: 10 real (unstripped) builds of `build/src/text1b.o`, objdump the
  prologue: char[1..4], s16[1], s16[4], s32[1], s32[2], s32[3],
  struct{s16;s16;}, with and without the sink.
- result: sink NEVER required — zero-reference BLKmode locals reach 48 on
  their own. Size is not the criterion, ALIGNMENT is: char[4] -> 48 but
  s32[1] -> 40 (both 4 bytes). Scalar-integer-mode-able aggregates
  (char[1]/s16[1]/s32[1]) become pseudos and reserve nothing.
- verdict: **KILLED (hypothesis corrected).** Law: BLKmode local <=> +8 slot.
  Frame equation: 48 = ALIGN8(vars in 2..8) + 16 args + 20 gp-saves.

### H-S6-C — "A semantically LIVE aggregate can hold the slot." (new axis)
- mechanism: if the aggregate carries a real value, it is not a dead
  declaration and clears cheat-checklist T1/T2/T6; the frame slot would then
  be an honest consequence of ordinary C.
- probe: replace scalar `s16 a1_val` with `s16 a1_val_a[1]` (written twice,
  read twice — every use real); real build + sandbox.
- result: real frame 40, sandbox 12, build_insns 126. GCC 2.7.2 promotes the
  HImode-able one-element array to a pseudo: no frame bytes, no stack
  traffic. Forcing BLKmode (>=2 elements / misalignment) necessarily emits
  `sw`/`lw` at 0x00..0x14($sp); target has ZERO such accesses and is exactly
  126 insns with no slack.
- verdict: **KILLED.** No live aggregate can produce the slot.

### H-S6-D — "s4's struct-aggregate H8 (sandbox 0) is a distinct closing form."
- mechanism: `struct { s32 a; s32 b; } dummy;` scored 0 in s4 and was banked
  as "same defect, not a distinct form" — but the 0 itself was never audited.
- probe: `struct { s16 a; s16 b; } dummy;` with ZERO references; real build +
  sandbox + engine detector source read.
- result: real frame 48, sandbox 0 — because NO detector covers a
  zero-reference struct-typed local. Third hole of the same class:
  `(void) dummy[0];` (indexed read) also yields 48/0.
- verdict: **KILLED.** Three independent spelling holes, all checklist-T4
  automatic FAILs; none is a candidate.

## FRONTIER RESET after s6 (strongest first)

1. **F1 — Detector-complete re-audit of every "0" in this ledger, then decide
   the disposition on the corrected floor of 12.** Mechanism: every score in
   s1..s5 was taken through a stripper that silently removes some spellings
   and misses others, so both the 0s (holes) and some of the 12s (over-strip)
   are artifacts; only unstripped real builds are codegen evidence. Next
   probe: for each banked form in rejected/, record BOTH the real frame
   (`make build/src/text1b.o` + objdump) and the sandbox score, and mark which
   rows are artifacts — the s6 matrix
   (tmp/grind/func_80049A2C/s6/matrix.md) already does this for 10 rows and
   is the template.

2. **F2 — The only remaining un-probed route to frame 0x30 without a dead
   local: raise `current_function_outgoing_args_size` from 16 to 24 with
   vars = 0 (0 + 24 + 20 -> ALIGN8 -> 48).** Mechanism: MIPS
   compute_frame_size takes args_size from the widest call in the function;
   REG_PARM_STACK_SPACE pins the floor at 16 and all three calls here take
   <= 1 argument. Next probe: enumerate the constructs that bump
   outgoing_args_size WITHOUT emitting an argument store (library calls
   emitted by the back end for 64-bit shifts / division / soft-float,
   `alloca`, `__builtin_apply`), and check whether any of them can be reached
   from a semantic rewrite of this body that keeps build_insns at 126.
   Prior expectation is negative (a 5th integer argument stores at
   0x10($sp), which target lacks) — but this is the last decomposition of
   the frame equation that has never been measured, and s6 proved the
   equation itself had been mis-derived once already.

3. **F3 — Report the three stripper spelling holes to the operator.**
   Mechanism: `_ORPHAN_DECL_RE`'s `;[ \t]*$` anchor (trailing comment),
   the absent struct/union arm in both the orphan-decl and unused-array
   detectors, and `_VOID_DISCARD_NO_ADDR_RE` not covering indexed discards
   let a dead frame-coercion local score 0 in the cheat-invisible sandbox.
   Grind sessions may not touch `engine/`, so this is an operator item, not a
   probe — but until it is closed, ANY function's sandbox 0 that depends on a
   dead local is suspect, and this function is the worked example.

## [s6] The sandbox --disable all = 0 recorded by sessions s1-s5 is a real cheat-free byte match.
- mechanism: engine/volatile_cheats.py's orphaned-declaration closure (_ORPHAN_DECL_RE) is anchored ';[ \t]*$', so the trailing comment on `s32 dummy[2];    /* LOAD-BEARING ... */` hides the declaration from the stripper while find_void_discard_unused_locals still strips `(void) dummy;`. The dead 8-byte local survives into the 'cheat-invisible' build and produces target's addiu $sp,-0x30.
- probe: Applied the s1 candidate to src and measured (0); deleted ONLY the trailing comment, byte-identical C otherwise, and re-measured (12); reproduced twice alternating base/no-comment in one batch; read the post-strip source the sandbox actually compiles (tmp/sandbox/func_80049A2C/src/text1b.c) and confirmed the declaration survives with the comment and is blanked without it.
- result: 0 with the comment, 12 without it, reproducible; post-strip source confirms the mechanism. Honest floor is 12, agreeing with migration_pin.json's independently recorded floor of 12.
- verdict: KILLED

## [s6] Target's +8 locals area requires an 8-byte local plus a `(void) dummy;` sink to survive DCE (session s1's frame reading).
- mechanism: GCC 2.7.2 expand_decl: an aggregate whose (size, alignment) admits a scalar integer machine mode becomes a pseudo; otherwise it is BLKmode and goes through assign_stack_local, making get_frame_size() nonzero. compute_frame_size then yields 48 = ALIGN8(vars 2..8) + 16 outgoing-args + 20 gp-saves.
- probe: 10 real unstripped builds (`make build/src/text1b.o` + objdump of the prologue, which the cheat-stripper cannot touch): char[1], char[2], char[3], char[4], live s16[1], s16[4], s32[1], s32[2], s32[3], struct{s16;s16;}, with and without the sink. Matrix at tmp/grind/func_80049A2C/s6/matrix.md.
- result: Sink never required - zero-reference BLKmode locals reach frame 48 alone. Alignment decides, not size: char[4] -> 48 but s32[1] (also 4 bytes) -> 40. char[1]/s16[1]/s32[1] are promoted to pseudos and reserve nothing; char[2..4]/s32[2]/struct{s16;s16;} are BLKmode and reserve 8.
- verdict: KILLED

## [s6] A semantically LIVE aggregate can hold the +8 slot, giving a form where every local has a real role (clears cheat-checklist T1/T2/T6).
- mechanism: If the aggregate carries a real value the declaration is not dead and the frame slot would be an ordinary consequence of C. That requires the aggregate to be BLKmode AND its live uses to cost zero instructions.
- probe: Replaced scalar `s16 a1_val` with `s16 a1_val_a[1]` (written twice, read twice, every use real); measured real frame and sandbox score; cross-checked target's sp-relative accesses by fresh grep of asm/funcs/func_80049A2C.s.
- result: Real frame 40, sandbox 12, build_insns 126 - GCC promotes the HImode-able one-element array to a pseudo, reserving no frame bytes. Pushing it to BLKmode necessarily emits sw/lw at 0x00..0x14($sp); target contains ZERO such accesses (only the five s0-s3/ra saves at 0x18..0x28) and is exactly 126 instructions with no slack.
- verdict: KILLED

## [s6] Session s4's struct-aggregate H8 (`struct { s32 a; s32 b; } dummy;`, sandbox 0) is a distinct closing form worth re-examining.
- mechanism: Neither find_unused_local_arrays (array declarators only) nor find_orphaned_local_decls (requires >=1 reference inside a stripped span; its type alternation has no `struct`) covers a zero-reference struct-typed local, so it is never stripped and scores 0 while still reserving the frame slot.
- probe: Built `struct { s16 a; s16 b; } dummy;` with ZERO references and `s32 dummy[2];` + `(void) dummy[0];`; measured real frame and sandbox for both; read the detector source in engine/volatile_cheats.py.
- result: Both give real frame 48 and sandbox 0 - two further spelling holes of the same class as the trailing-comment hole. Session s4's H8 zero is therefore an artifact. Under cheat-checklist T4 (passes only because the detectors miss this spelling) all three are automatic FAILs, not candidates.
- verdict: KILLED

## [s7 FRONTIER RESET] The residual is one phantom reload spill slot, and the mechanism is now named.

s6 closed the ledger on the belief that target's +8 could only come from a dead
BLKmode local (the forbidden `unused-local-array frame coercion` family), which
made this function look like an endgame lock. That belief is measured false.
Frame bytes in GCC 2.7.2 / MIPS have TWO sources, and only the first was ever
explored here:

  (1) get_frame_size() - locals and temps allocated at expand time. s6's law
      correctly describes THIS source: a BLKmode aggregate reserves 8 bytes, and
      any live one emits stack traffic that target does not have.
  (2) reload1.c:2404 alter_reg - a stack slot for EVERY pseudo with
      reg_renumber < 0 and reg_n_refs > 0, sized MAX(inherent, max_ref_width)
      and rounded up to BIGGEST_ALIGNMENT (8) by assign_stack_local's align == -1
      path. When the pseudo's insns were all absorbed by combine, nothing ever
      references the slot: vars=8 with zero sp traffic - exactly target's shape.

Source (2) is not a cheat family at all. It fires from completely ordinary C:
70 functions in this repo's own oracle-matching source have it, 26 of them
loopless, including func_800493E4 - same file, same TU, same compilation
context, and no dead local anywhere in its body.

### H-S7-A - "the +8 is a cc1psx-vs-fork divergence" (the standing cross-ledger claim)
- probe: the same preprocessed TU through build/cc1 and through cc1psx.exe.
- result: both emit `vars= 0, regs= 5/0, args= 16` - frame 40. cc1psx reserves
  nothing either.
- verdict: **KILLED.** The +8 is a property of the C, not of the compiler build.

### H-S7-B - "s6's BLKmode law is the whole truth about this frame"
- probe: whole-tree `.frame` census plus BB2_FRAME_DEBUG attribution.
- verdict: **KILLED.** The phantom-spill source was never considered.

### H-S7-C - "moving a definition across one of this function's two labels orphans a REG_DEAD and buys the slot"
- probe: five ordinary-C reshapings, real cpp + instrumented cc1.
- verdict: **KILLED for those five spellings** (all vars=0). The axis is not
  exhausted - these were the five most obvious shapes, not a partition.

## FRONTIER after s7 (strongest first)

1. **F1 - Find the ordinary-C spelling of this body that leaves one
   combine-absorbed pseudo behind.** Mechanism: combine.c:10836 emits
   `(use (reg:SI N))` after a CODE_LABEL when a REG_DEAD note for a folded-away
   pseudo cannot be placed; the USE costs zero instructions, keeps
   reg_n_refs > 0, regclass reports the pseudo as "ST_REGS or none", global_alloc
   leaves reg_renumber = -1, and alter_reg reserves the 8 bytes. Next probe:
   diff the combine dump of func_800493E4 (which HAS the USE - see
   tmp/grind/func_80049A2C/s7/da/, pseudo 98, `(insn 150 (use (reg:SI 98)))`
   after code_label 83) against func_80049A2C's combine dump to find what stops
   the same absorption here, then reshape the corresponding statement. The two
   candidate carriers in this body are the address-forming pointers `p_anim`
   (= D_800EF980 + temp_v1*2) and `obj`, both of which live across a label;
   func_800493E4's carrier is exactly an address-forming pointer for an indexed
   HImode store.

2. **F2 - Enumerate the 26 loopless phantom-slot precedents' C bodies and
   extract the shared trigger shape.** Mechanism: they are oracle-proven
   ordinary C that produces the exact frame signature this function needs, so
   the trigger vocabulary is already sitting in the repo. Next probe: reproduce
   the census with tmp/grind/func_80049A2C/s7/sweep.sh plus the parser recorded
   in the s7 outcome, then read the eight smallest ones (func_80042F10,
   func_80086014, func_80086130, snd_CalcFade, snd_GetFadeCurve, disp_CalcFov,
   get_cs, get_ce) and classify what combine absorbed in each.

3. **F3 - args=24 (vars=0) remains formally open but is almost certainly dead.**
   Mechanism: current_function_outgoing_args_size is written only in calls.c
   (1400 / 2400 / 2750) from a call's own argument size, and any 5th argument
   word is STORED at 0x10($sp); target has zero non-save sp traffic. The only
   way to bump it without a store is to expand a call and then delete it, which
   is dead code. Next probe: none recommended before F1 and F2 are spent.

## [s7] The +8 frame slot target reserves is a cc1psx-vs-fork compiler divergence (the standing cross-ledger claim recorded in memory/grind/func_80022F34).
- mechanism: cc1psx (GCC 2.7.2.SN.1) was believed to reserve a locals slot that decompals/mips-gcc-2.7.2 does not, making the residual unreachable from C.
- probe: Preprocessed the real src/text1b.c TU with the project cpp flags and fed the identical .i to tools/gcc-2.7.2/build/cc1 (project flags) and to tools/cc1psx_wrapper.sh (-O2 -G0 -funsigned-char -mcpu=3000 -mips1 -w, via dosemu2); compared the emitted .frame directives.
- result: Both compilers emit `.frame $sp,40,$31 # vars= 0, regs= 5/0, args= 16, extra= 0`. cc1psx reserves nothing either. Side finding: our fork hoists `sw $17,20($sp)` away from the other four saves exactly as target does, while cc1psx emits all five saves contiguously - for this function the fork is the closer compiler.
- verdict: KILLED

## [s7] s6's law - the +8 slot is reachable ONLY through a wholly dead, memory-resident (BLKmode) local, i.e. the forbidden unused-local-array frame-coercion family.
- mechanism: s6 enumerated only expand-time frame allocation (get_frame_size()). GCC 2.7.2/MIPS has a second source: reload1.c:2404 alter_reg reserves a stack slot for every pseudo with reg_renumber < 0 and reg_n_refs > 0, sized MAX(inherent, reg_max_ref_width) and rounded up to BIGGEST_ALIGNMENT (8) by assign_stack_local's align == -1 path. If the pseudo's insns were all absorbed by combine, no instruction ever references the slot.
- probe: Compiled every src/*.c to .s with the project cc1 flags, parsed all ~1200 .frame directives, and kept every function with vars > 0 whose body has no ($sp) reference other than callee-save stores/loads; re-ran the instrumented cc1 with BB2_FRAME_DEBUG=1 to attribute each allocation.
- result: 70 such functions, 26 of them loopless. Every attributed one is ctx=spill_new_p<N> mode=4 size=8 align=-1 alignment=8 - a phantom reload spill slot, never a local. Clean ordinary-C examples: memset (src/display.c:926, a six-line for-loop, frame 8/vars 8, zero stack traffic) and func_800493E4 in this function's OWN FILE src/text1b.c (loopless, frame 0x20, vars=8, two saves, zero stack traffic, no dead local in its body). The tree SHA1-matches the oracle, so all 70 are target-proven constructs.
- verdict: KILLED

## [s7] The GCC pass and decision that produces the phantom slot can be named exactly.
- mechanism: combine.c:10836-10841 (distribute_notes): when a REG_DEAD note for a folded-away pseudo cannot be placed on any surviving insn and the placement scan reaches a CODE_LABEL, combine emits `(use (reg:SI N))` after that label to carry the note. The USE emits no code but keeps reg_n_refs[N] > 0; regclass then reports the pseudo as 'ST_REGS or none'; global_alloc lists it with an empty conflict set and does not allocate it; reload1.c alter_reg hands it the 8-byte-rounded slot nothing references.
- probe: cc1 -da minimal repro of memset (pseudo 79, the loop-invariant -1 combine folded into the branch) plus the real text1b -da dumps for func_800493E4 (pseudo 98, an address-forming pointer whose HImode store combine absorbed; it survives only as `(insn 150 (use (reg:SI 98)))` after code_label 83). Cross-read reload1.c alter_reg, mips.c compute_frame_size, and combine.c distribute_notes.
- result: Mechanism traced end to end and reproduced in two independent functions, one loopless and in this function's own TU. lreg/greg signatures for the orphan pseudo: 'Register N used 2 times ...; ST_REGS or none', 'N conflicts:' (empty), absent from Register dispositions, FRAMEDBG ctx=spill_new_pN.
- verdict: CONFIRMED

## [s7] Moving a definition across one of this function's two labels orphans a REG_DEAD note and buys the phantom slot.
- mechanism: The slot needs a pseudo whose defining insn combine absorbs into a use in another block with the death note stranded at a label; this body has two labels (the temp_v1 == 0xFF early return and the InitFadePanel branch).
- probe: Five ordinary-C reshapings built through the real cpp + instrumented cc1 on the full TU, read back through BB2_FRAME_DEBUG and the .frame directive: hoist the D_800EF980 base above the early return; hoist the D_80099D3C rotation-table pointer above the fade branch; pre-read *p_anim into a named s16 before the branch; form obj = D_800A38B4 before the branch; invert the guard to `if (temp_v1 != 0xFF) { ... }`.
- result: All five report the single ctx=round_frame frame_offset=0 record - zero frame allocations, frame 0x28. The rotation-table hoist additionally raised the callee-save count from 5 to 6 and STILL produced frame 40, independently confirming the gp_reg_rounded law. Banked as memory/grind/func_80049A2C/rejected/phantom-slot-*.c.
- verdict: KILLED

## [s7] The frame equation admits decompositions other than vars=8/args=16.
- mechanism: mips.c:4444 compute_frame_size: total = ALIGN8(vars) + ALIGN8(outgoing_args) + extra + ALIGN8(gp_reg_size) + ALIGN8(fp_reg_size); gp_sp_offset = args + extra + vars + gp_reg_size - 4.
- probe: Read compute_frame_size end to end and solved it against target's measured layout (five saves, highest save 0x28, total 0x30), plus the measured 6-save variant.
- result: gp_reg_size is rounded to 8 BEFORE entering the total, so 5 saves (20 B) and 6 saves (24 B) both contribute 24 - a 6th callee-save can never move this frame (measured). Target's 0x30 forces vars + args = 24 exactly: either vars=8/args=16 or vars=0/args=24, and the two are byte-indistinguishable. args=24 requires a call whose 5th argument word is stored at 0x10($sp), and target has zero non-save sp traffic, so the live decomposition is vars=8.
- verdict: CONFIRMED
