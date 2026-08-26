> **2026-08-24 MIGRATION NOTE:** HEAD is now `INCLUDE_ASM` (migrated in
> a7892ba2 (2026-08-24 sweep 2)); rules retired, in-source cheat-asm removed. "HEAD"
> claims below describe the pre-migration tree (`retired-chassis-2026-08/body.c`).

# Evidence bank — func_80037B00

- WIP rejected_form: pin-free + no-dummy (23; whole-function register-rotation cascade + loop scheduling shift + absent 8-byte frame)

- WIP rejected_form: pin-free + sp_dummy[2] (FAILED reviewer 2026-06-22 — dead-vars-local-array; saved under rejected/. GCC DCEs the dummy in pin-free form so the frame doesn't materialize regardless — the construct is both forbidden AND ineffective here)

- WIP rejected_form: do-while restructure pin-free (sandbox=17, build_insns=33 — worse than the goto form: GCC dropped one inner-loop instruction; the do-while loop-exit collapses one slt/bnez pair)

- == imported from memory/wip notes.md ==
# func_80037B00 — BLOCKED (multi-register rotation + phantom +8 frame)

## TL;DR
String-compare loop over D_80102810 entries (matches arg0 against each 0x15-byte
entry; 0x28 stride). HEAD "matches" via NINE `register asm()` pins
(t1,t3,t2,a3,a1,a2,t0,v1,v0) + `s32 sp_dummy[2]` frame-padding cheat —
forbidden, inert under the sandbox. INCOMPLETE.

## Why blocked (pin-free floor 23 honest insns >> HEAD's cheated 15)
candidate.c (no pins, no dummy) measures sandbox=15 weighted-masked / 23 honest
insn diff. Two coupled problems:

1. **5-way register rotation**: GCC picks t0/v1/a1/t1/a2 where target uses
   t1/a1/a2/t0/v1 — a coupled rename across 5 vars, not a single swap. The
   masked-0 score earlier observed with sp_dummy was misleading (registers
   mask out before the weighted score) — `retire` rebuilds and fails SHA1.
2. **8-byte phantom frame**: target emits `addiu sp,sp,-8` / `addiu sp,sp,8`
   wrapping a body that never touches sp. Pin-free C gives GCC no reason to
   reserve frame, so no prologue/epilogue. `s32 sp_dummy[2]` would force the
   frame ONLY when paired with the register pins (the pin reload pressure
   keeps the unused local alive). In pin-free form it DCEs away — AND it's a
   forbidden dead-vars-local-array per the cheat policy (rejected/, 2026-06-22).
3. **Inner-loop scheduling**: subtle shift in `slt`/`addiu` ordering near
   .L80037B30 — a side-effect of the different RA, will likely follow once
   the rotation is forced.

## Cluster note
func_80037AA4, file_LoadSectors, func_80037B00 ALL carry an unexplained +8
stack frame the DCE'd dummy can't supply. A single cc1psx vs cc1-mips-2.7.2
calibration on the cluster could explain whether the toolchain we're using
emits this frame differently from the original PsyQ cc1psx.

## Highest-yield untried lever
**decomp-permuter from candidate.c** directed at the 5-way register rotation.
Same shape as the marionation_Exec coupled-rotation work — may plateau, but
this is the right tool for "C is correct, registers wrong." Seed: candidate.c.

## What was tried (this/prior session)
- Pin-free goto body (m2c shape): score 15 weighted, 23 honest. (candidate.c)
- Pin-free + `s32 sp_dummy[2]`: reviewer FAIL (dead-vars-local-array).
  Importantly: GCC DCEs sp_dummy in pin-free form so the frame doesn't even
  materialize — the construct is both forbidden AND ineffective here.
- Do-while restructure: sandbox=17, build_insns=33 (worse; lost one inner
  insn).

## Floor
HEAD: 15 (cheated, 9 pins + sp_dummy, SHA1-matches).
candidate.c: 23 honest insn diff, 15 weighted-masked. NOT lowered this session.


- [s1] canonical func_80037B00 → verdict=C, distance=15 (pure-C target)

- [s1] sandbox --disable all on HEAD (pins+dummy stripped) → score=15, target_insns=36, build_insns=34, cheat_asm_stripped=27

- [s1] Target frame=8, zero body accesses to sp+0..7, zero jals → outgoing-args=0, so the 8 bytes are unambiguously a vars slot (contrast file_LoadSectors s1 which resolved its frame slack to outgoing-args=24)

- [s1] Sibling file_LoadSectors evidence bank: GCC 2.7.2 provably NEVER DCEs local-array stores (empirical test 2026-06-16)

- [s1] 2026-07-01 dead-vars-local-array carve-out gate FAILS here: requires target dead stores in the reserved slot; func_80037B00 target has zero stores in sp+0..7

- [s1] Owner standing 2026-07-27 auto-ruling: Gate 1 (STRONG scan_hand_coded S1/S2/S6 for canonical-asm) FAILS — this is a plain strncmp-style dispatch loop; Gate 2 (SOTN precedent for a sanctioned coercion) FAILS — no dead-store precedent applies. HOWEVER an unexhausted untried lever exists (directed permuter from candidate.c per WIP), so not yet a both-gates-fail-with-nothing-left endgame lock.

- [s1] Register axis: 5-way coupled rotation (t0,v1,a1,t1,a2) → (t1,a1,a2,t0,v1); not a single swap, no shared-exit CSE, no jals — call-return-* / exit-path-* / compare-operand-order patterns don't apply directly

- [s1] Rejected forms bank contains: dead-vars-local-array.c (do-not-repropose)

- [s2] s2: baseline pin-free candidate.c score=15 weighted, target_insns=36, build_insns=34, cheat_asm_stripped=8 (HEAD's pins+dummy dropped)

- [s2] s2: register-alloc-pure-c Levers A (block-local split) and B (narrow types) both exhausted for this function — no structural rewrite of the local decl set moved the score

- [s2] s2: shared-end-label makes score WORSE (not better) for this function — added live scalar amplifies the 5-way rotation tiebreak rather than resolving it

- [s2] s2: load-order restructuring is contraindicated — GCC folds paired byte-loads and drops instructions target retains

- [s2] s2: 3 KILLED + 2 INERT structural variants, all measured; structural modality now exhausted for the register-rotation axis

- [s2] s2: no rejected form was banked-cheat family; all failed on their measured score, not on policy

- [s3] s3: baseline reaffirmed on pin+dummy HEAD form → sandbox --disable all score=15, target_insns=36, build_insns=34, cheat_asm_stripped=27 (HEAD's 9 pins + sp_dummy dropped)

- [s3] s3: statement re-association variant (var_v0 = D_800A38C8; if (<=0) return 0; var_t1 = 0; ... ; drop block_end label) MEASURED → sandbox=15, build_insns=34, cheat_asm_stripped=8 (pin-free form). Identical floor — GCC hoists the t1-init into blez's delay slot regardless of source position; the inlined return 0 folds to the same j/addu tail. Saved rejected/stmt-reassoc-direct-return.c.

- [s3] s3: structural axis is NOW EXHAUSTIVELY EXHAUSTED across all Lever-A/B/C/D shapes plus statement re-association — 6 measured killed variants + 2 measured inert variants (var_v0 split, decl reorder from s2). The 5-way register rotation + 8-byte phantom frame does not respond to any structural mutation of the pure-C body that stays within cheat policy.

- [s3] s3 baseline reaffirmed on pin+dummy HEAD form: sandbox --disable all score=15, target_insns=36, build_insns=34, cheat_asm_stripped=27 (HEAD's 9 pins + sp_dummy dropped).

- [s3] s3 statement-reassoc variant identical floor: sandbox=15, build_insns=34, cheat_asm_stripped=8 (pin-free, no dummy). Rejected form banked.

- [s3] Structural axis is now exhaustively exhausted: 7 measured killed variants across all catalog levers (register-alloc-pure-c A/B/C/D, shared-end-label, do-while restructure, eager-both-byte-loads, statement re-association) + 2 measured inert variants (var_v0 split, decl reorder). No structural mutation of the pure-C body within cheat policy moves the 5-way register-rotation + 8-byte phantom-frame floor.

- [s3] Owner standing 2026-07-27 both-gates status re-confirmed: Gate 1 (STRONG scan_hand_coded S1/S2/S6 for canonical-asm) FAILS — this is a plain strncmp-style dispatch loop, no S1/S2/S6 signals; Gate 2 (SOTN in-hand precedent for a sanctioned coercion family covering 5-way coupled register rotation) FAILS — no dead-store / pointer-alias / named-local / duplicated-statement / MMIO carve-out applies to the register-rotation shape here.

- [s3] One sanctioned lever remains untried: directed decomp-permuter from candidate.c (pin-free, floor 15) — this is the ledger's live frontier and per s1 is the mandated next step BEFORE any owner-gated escalation. Standing 2026-07-27 ruling requires exhaustion of grindable levers; permuter is grindable, so owner-gated is NOT valid this session.

- [s4] s4 baseline reaffirmed: pin-free candidate.c form applied to src/code6cac_c.c func_80037B00 → sandbox --disable all: score=15, target_insns=36, build_insns=34, scorable=true, cheat_asm_stripped=8, rules_dropped=0. Identical to s2/s3 baselines.

- [s4] src/code6cac_c.c reverted to HEAD (pinned form with sp_dummy) at session end; candidate.c in memory/grind/ unchanged (already correct pin-free form).

- [s4] decomp-permuter import.py produces base.c with two structural bugs (duplicate extern decls at lines 627/645) and one preserved-macro artifact (`_permuter_ignore_line __asm__(...)` at line 751 inside func_80037A20, which the maspsx pipeline treats as an undefined identifier). None of these are catalog concerns; all are import-tool artifacts that need manual patching before compile.sh can run.

- [s4] target.o built by import.py uses `mips-linux-gnu-as -march=vr4300 -mabi=32` (elf32-tradbigmips, WRONG). Correct assembly is `mipsel-linux-gnu-as -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0` with the mar_perm_workspace r3k prelude (drops `.set gp=64` from tools/decomp-permuter/prelude.inc). Rebuilt target.o now elf32-tradlittlemips with 43 objdump lines.

- [s4] maspsx crashes reproducibly with `too many values to unpack (expected 3)` (tools/maspsx/maspsx.py:224-226 catches any Exception from `maspsx_processor.process_lines()`). No traceback surfaces because it's caught and only the message is printed. Root cause is somewhere inside maspsx/__init__.py:process_lines() but not localized this session — the SAME pipeline stages succeed as a standalone script.

- [s4] Artifacts preserved: tmp/grind/func_80037B00/s4/pre.s (776 lines, post-prologue_fix input to maspsx), tmp/grind/func_80037B00/s4/post.s (733 lines, verified maspsx output including `.end func_80037C34`), tmp/grind/func_80037B00/s4/trace.sh (the working standalone pipeline), tmp/grind/func_80037B00/s4/session_notes.md (this session's writeup).

- [s4] Standing 2026-07-27 both-gates status UNCHANGED from s3: Gate 1 (STRONG scan_hand_coded S1/S2/S6 for canonical-asm) FAILS — plain strncmp-style dispatch loop, no hand-coded signals; Gate 2 (SOTN in-hand precedent for a sanctioned coercion covering 5-way coupled register rotation) FAILS — no dead-store / pointer-alias / named-local / duplicated-statement carve-out applies. Directed permuter remains the last untried grindable lever; its exhaustion is still required before the endgame ruling can be applied.

- [s4] This session did NOT run any permuter iterations — the outcome is a workspace-setup blocker, not a permuter-plateau finding. Permuter modality remains open for s5.

- [s5] [s5] Baseline reaffirmed on pin-free candidate.c in src/code6cac_c.c: sandbox --disable all -> score=15, target_insns=36, build_insns=34, scorable=true, cheat_asm_stripped=8, rules_dropped=0. Identical to s2/s3/s4 floor.

- [s5] [s5] Built permuter workspace at tmp/grind/func_80037B00/s5/perm_ws/ via the mar_perm_workspace.sh pattern (full-TU cpp preprocess of src/code6cac_c.c into base.c + region-extract compile.sh + r3k-prelude target.o). Bypasses s4's blocked import.py / bash pipeline issues entirely. Base.o and target.o both elf32-tradlittlemips at offset 0; diff confirms exactly the 5-way register rotation + missing 8-byte frame the ledger predicted.

- [s5] [s5] Permuter campaign pid 414, label s5_pinfree_v1, 4 workers, --stack-diffs --stop-on-zero, base_score=290 (permuter-weighted metric; not comparable to sandbox 15). Launched 21:10:00 UTC, harvested/stopped ~21:30 UTC after ~20 min wall, ~29,000 iterations. 66 novel-scoring finds; ZERO score-0 closing forms.

- [s5] [s5] Low-score tail is EXCLUSIVELY the named-holder-local cheat family: output-70-1 (`short new_var; ... new_var=0; return new_var`), output-75-1 (variant), output-90-1 (`int new_var; new_var=0; new_var = var_v0 <= new_var; if (new_var) goto block_end`), output-95-1 and output-95-2 (dead-reassign existing locals as zero-holders — `var_t3 = 0; if (var_v0 <= var_t3)` etc.), output-175-1 (named-local + `if (1) { }` wrapper). Every one violates no-new-park-categories 'cheats by any spelling' + the explicit Judge constraint in the session brief ('may NOT be closed via nested do-while(0) + named-holder locals'). Banked to memory/grind/func_80037B00/rejected/permuter-*.c.

- [s5] [s5] Best legitimate non-cheat score achieved: 195 (six variants) — still ~20 permuter-weighted diffs above target. Base was 290; the 33% relative reduction is not a byte-match and comes from associative statement reordering that doesn't touch the register-rotation axis.

- [s5] [s5] Gate 1 status (canonical-asm STRONG scan_hand_coded): FAILS. func_80037B00 is a plain strncmp-style dispatch loop over D_80102810 entries; no S1/S2/S6 signals; every PS1 decomp community example writes this shape in C.

- [s5] [s5] Gate 2 status (SOTN in-hand precedent for a sanctioned coercion family covering 5-way coupled register rotation + 8-byte phantom frame): FAILS. Confirmed by evidence.md lines 70, 100, 116 across s1/s3/s4 — no dead-store / pointer-alias / named-local / duplicated-statement carve-out binds the shape. The permuter's finds prove the shape's only score-lowering direction from a pin-free honest base is the banned family.

- [s5] [s5] Structural axis + permuter axis both now measured exhausted. No grindable lever remains. This is the 2026-07-27 standing-ruling terminal endgame.

- [s5] [s5] docs/grind/decisions.md updated with the OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE entry naming func_80037B00 in its title. Driver parks the function; queue advances; no owner action pending.

- [s5] [s5] src/code6cac_c.c reverted to HEAD (pinned form with sp_dummy) at session end. memory/grind/func_80037B00/candidate.c unchanged (pin-free form, floor 15). Four banked permuter-cheat samples added to rejected/.

- [s6] SYNTHESIS. Chassis re-measured with the s0..s5 candidate.c pasted over the INCLUDE_ASM line: `sandbox --disable all` -> score=15, target_insns=36, build_insns=34, cheat_asm_stripped=3, rules_dropped=0. Ledger floor of 15 confirmed on the post-migration chassis.

- [s6] The residual was TYPED for the first time (tmp/grind/func_80037B00/s6/multiset.py — register-blanked multiset + ordered diff of the sandbox .o against asm/funcs/func_80037B00.s). On the s0..s5 form the register-blanked instruction MULTISET differs from target by EXACTLY TWO instructions: `addiu $sp,$sp,-0x8` and `addiu $sp,$sp,0x8`. Nothing else. Every other instruction, including all four `nop`s, is present in both streams with identical multiplicity.

- [s6] KILLED (s1 hypothesis #3, "inner-loop scheduling shift near .L80037B30"): there is NO scheduling residual. Deleting target's two `sp` instructions makes the ORDERED streams line up position-for-position, all 34 slots, modulo register names. The s0..s5 belief that the `slt`/`addiu` ordering was shifted was an artefact of comparing streams of different lengths. sched1/sched2/reorg are NOT implicated in this function at all.

- [s6] GCC 2.7.2 global.c allocation model reproduced EXACTLY for this function from the .greg/.flow dumps. Priority = floor_log2(n_refs)*n_refs*size / live_length. Computed on the s0..s5 form: 81=2.00, 77=0.714, 78=0.615, 80=0.600, 76=0.417, 73=0.286, 79=0.182, 75=0.167, 74=0.083, 72=0.071 -> predicted rank order [81 77 78 80 76 73 79 75 74 72], which is the greg's `;; 10 regs to allocate:` line VERBATIM. All ten allocnos mutually conflict, so assignment is deterministic: rank order maps onto the ascending free-hard-register list [2,3,5,6,7,8,9,10,11] ($4 pre-taken by the incoming arg). The register a value gets is therefore a pure function of its allocno PRIORITY RANK — refs and live_length are the only two C-visible dials, with birth order (= lower pseudo number) as the tiebreak.

- [s6] The frame axis is INDEPENDENTLY INSTRUMENTABLE and was never instrumented in s0..s5. cc1 prints `get_frame_size()` itself: `.frame $sp,N,$31 # vars= N`. Probe script banked at tmp/grind/func_80037B00/s6/frame_probe.sh (cpp | cc1 with the project's -mel flags, awk the `.ent func_80037B00` .. `.frame` line). s0..s5 form measures `vars= 0`; target measures 8.

- [s6] BREAKTHROUGH — the 8-byte phantom frame is REACHABLE FROM ORDINARY LIVE C, with no dead declaration, no local array, and no coercion construct. Writing the OUTER loop as a top-tested `while (var_t1 < var_t3)` with NO explicit pre-guard makes GCC rotate the loop and materialise a duplicated entry guard in basic block 0; combine folds that guard's comparison into a bare `blez`, the compare pseudo survives as an allocno with no hard register left (pseudo 86 in the v_for dump: present in `;; 10 regs to allocate:` but ABSENT from `;; Register dispositions:`), reload's alter_reg gives it a stack slot, and `vars` becomes 8 at ZERO instruction cost. This is producer #1 ("folded loop-guard compare") of .claude/rules/phantom-slot-frame-lever.md, and it dissolves the s0..s5 conclusion that the frame required the forbidden dead-vars-local-array family.

- [s6] FLOOR LOWERED 15 -> 11 (first movement since s2). Form: s0..s5 goto body with the outer `goto loop_outer` loop rewritten as a top-tested `while (var_t1 < var_t3)` and the explicit `if (var_v0 <= 0) goto block_end;` guard DELETED. `sandbox --disable all` -> score=11, target_insns=36, build_insns=35, cheat_asm_stripped=3, rules_dropped=0; `vars= 8`. Banked as memory/grind/func_80037B00/candidate.c and tmp/grind/func_80037B00/s6/v_hybrid2.c.

- [s6] In that form the register-blanked multiset delta collapses from two instructions to ONE: target's `addu $t3,$v0,$zero` preheader copy. The ordered diff shows our stream is target's stream with that one copy removed and two emission-position differences.

- [s6] The "5-way coupled register rotation" that five sessions treated as the core wall is now MOSTLY DISSOLVED. In the new form $v0, $v1, $a1, $a2, $a3, $t2, $t3 all land on target's registers (the 3-cycle {v1,a1,a2} is gone). Exactly ONE 2-swap remains: our outer counter takes $t0 and our inner end-pointer takes $t1; target has counter=$t1, end=$t0.

- [s6] KILLED: re-adding an explicit `if (var_v0 <= 0) return 0;` guard in front of the top-tested `while` collapses the frame back to `vars= 0`. GCC's jump threading deletes the now-redundant rotation guard, the block-0 compare pseudo disappears, and there is no orphan to pay for a slot. The guard duplication MUST be GCC's own, not the programmer's. Banked rejected/explicit-guard-kills-rotation-orphan.c.

- [s6] KILLED: writing the INNER loop as `do { ... break ... } while (p < end)` (natural C, no gotos) makes GCC rotate and peel it as well — the first `lbu` is duplicated above the loop and an extra `beqz`/`bnez` pair appears; build_insns 38, score 22. The inner loop must keep the s0..s5 goto spelling. Banked rejected/for-loop-rotates-inner-loop-too.c.

- [s6] KILLED: splitting the count into `var_v0 = D_800A38C8; var_t3 = var_v0;` to try to recover target's `addu $t3,$v0,$zero` preheader copy is inert — copy propagation folds it. Identical measurement to the merged form (score 11, build_insns 35, vars= 8). Banked rejected/split-count-copy-folded-by-copyprop.c.

- [s6] Endgame-lock gate status is now MOOT and the s5 escalation is SUPERSEDED: the floor is moving again (15 -> 11) on a sanctioned, ordinary-C axis that no prior session had instrumented. func_80037B00 is grindable, not exhausted. The 2026-07-28 decisions.md entry describing it as REFUSED / OWNER-ACCEPTED INCOMPLETE rests on the s1..s5 premise that the frame required a forbidden construct, which s6 measured to be false.

- [s6] Artifacts: tmp/grind/func_80037B00/s6/frame_probe.sh (frame gradient), tmp/grind/func_80037B00/s6/multiset.py (residual typer: register-blanked multiset + ordered diff), tmp/grind/func_80037B00/s6/v_for.c, v_hybrid.c, v_hybrid2.c (the new floor), v_hybrid3.c, v_h4.c, tmp/grind/func_80037B00/s6/func.s + raw.s (cc1 stream), tmp/grind/func_80037B00/dumps/*.greg/.flow (allocation model inputs).

- [s6] Chassis re-measured at s6 start with the s0..s5 candidate.c pasted over the INCLUDE_ASM line: sandbox --disable all -> score=15, target_insns=36, build_insns=34, cheat_asm_stripped=3, rules_dropped=0. The ledger floor of 15 holds on the post-migration chassis.

- [s6] New floor: score=11, target_insns=36, build_insns=35, cheat_asm_stripped=3, rules_dropped=0, cc1 .frame vars=8. Form banked at memory/grind/func_80037B00/candidate.c and tmp/grind/func_80037B00/s6/v_hybrid2.c.

- [s6] On the s0..s5 form the register-blanked instruction multiset differs from target by exactly the two sp-adjust instructions and nothing else; the ordered streams then align across all 34 slots. There is no scheduling residual in this function.

- [s6] cc1's own `.frame $sp,N,$31 # vars= N` is a direct, cheap gradient on the frame and separates 'wrong frame' from 'wrong codegen' in a way the sandbox score cannot. Probe banked at tmp/grind/func_80037B00/s6/frame_probe.sh. Five prior sessions never used it.

- [s6] GCC 2.7.2 global.c allocation reproduced exactly: pri = floor_log2(n_refs)*n_refs*size/live_length, ties to lower pseudo number; all ten allocnos mutually conflict so rank N takes the Nth free hard register from the ascending list [2,3,5,6,7,8,9,10,11] ($4 pre-taken by the incoming arg). Predicted rank order equals the greg's ';; 10 regs to allocate:' line verbatim.

- [s6] In the new form $v0, $v1, $a1, $a2, $a3, $t2 and $t3 all land on target's registers — the 3-cycle {v1,a1,a2} that dominated the s0..s5 framing is gone. Exactly one 2-swap remains: our outer counter takes $t0 and our inner end-pointer takes $t1, where target has counter=$t1 and end=$t0.

- [s6] Remaining residual on the new form, fully characterised: (a) target's `addu $t3,$v0,$zero` preheader copy is absent (we load D_800A38C8 straight into the loop-bound register); (b) `lui/addiu` for D_80102810 is emitted at stream positions 1-2 instead of target's 6-7, i.e. above the lw/blez instead of inside the guarded preheader; (c) the $t0/$t1 2-swap.

- [s6] The construct that produced the frame is ordinary live C — a top-tested while loop over named locals, with the programmer's redundant pre-guard deleted. No dead declaration, no local array, no volatile, no named holder, no wrapper: nothing in the forbidden-family catalog and nothing requiring a FAKE annotation. The frame comes from GCC's own loop rotation, not from anything written in the source.

- [s6] The 2026-07-28 decisions.md entry (OWNER-ESCALATION — RESOLVED BY STANDING RULING: REFUSED / OWNER-ACCEPTED INCOMPLETE) rests on the premise that the frame required the forbidden dead-vars-local-array family and that the structural axis was exhausted. Both are measured false. A superseding entry was filed this session at docs/grind/decisions.md (## 2026-08-26 — func_80037B00 — SUPERSEDED). It requests no owner action and lowers no standard; it exists so the earlier entry is not read as current. func_80037B00 is grindable and should stay active.

- [s6] s5's permuter plateau was measured from the s0..s5 base form (score 15, vars=0), which we now know was two instructions away from target in the multiset. Any future permuter campaign must be re-based on the s6 form; the s5 negative result does not transfer.

- [s7] Chassis re-measured at s7 start with the s6 candidate.c pasted over the INCLUDE_ASM line: `sandbox --disable all` -> score=11, target_insns=36, build_insns=35, cheat_asm_stripped=3, rules_dropped=0. The s6 floor of 11 holds.

- [s7] FLOOR LOWERED 11 -> 9, and the INSTRUCTION MULTISET NOW MATCHES TARGET EXACTLY (build_insns 36 == target_insns 36). Form: the s6 body with the `var_t3` count local DELETED and the outer loop written `while (var_t1 < D_800A38C8)` — the bound is read by the loop test itself. Banked as memory/grind/func_80037B00/candidate.c and tmp/grind/func_80037B00/s7/vA_bound_in_test.c (== FLOOR9.c). `.frame $sp,8,$31 # vars= 8` retained.

- [s7] MECHANISM for the recovered `addu $t3,$v0,$zero`: with the load inside the loop's exit test, GCC's rotation guard in block 0 keeps its own copy of the load and loop.c hoists the in-loop load into the preheader; cse then unifies the two into a register copy. Two pseudos reach RA where the s0..s6 forms had one. This is why the s6 C-level duplicate was folded by copy propagation but this spelling is not: the second pseudo is created by a PASS, not by the source.

- [s7] `tools/ra_solver/goal_from_tgt.py classify code6cac_c func_80037B00` on the new form: **FIRST DIVERGENCE: RA** (was PRE-RA / rtl_shape on every earlier form), substitutions `$t0 -> $t1 x4`, `$t1 -> $t0 x2`, `goal_from_tgt.py goal` reports 5 renamed pairs, 0 skipped. The residual is now fully inside the validated allocation model — no PRE-RA multiset gap remains.

- [s7] TOOL DEFECT (bank it): `tools/ra_solver/extract.py func_80037B00 code6cac_c` FAILS with "FATAL: func_80037B00 not in .ent list (7 functions)". Root cause: the INSTRUMENTED cc1 (`tools/gcc-2.7.2/cc1`, the BB2_*_DEBUG build) SEGFAULTS (RC=139) partway through src/code6cac_c.c, right after func_800379D8 — i.e. on func_80037A20 — so the .s/.greg/.flow/.lreg it emits cover only the first 7 functions. The build compiler `tools/gcc-2.7.2/build/cc1` compiles the same TU fine. Consequence: `simulate.py` / `inverse.py` / `sweep.py` are UNAVAILABLE for any function in code6cac_c after func_800379D8 until the instrumented cc1 is fixed or a trimmed TU path is added. Repro script: tmp/grind/func_80037B00/s7/icc1.sh. Workaround used this session: the plain `-da` dumps from `pwsh tools/grinder/dump.ps1` (.greg/.lreg/.flow) carry every input the priority model needs.

- [s7] global.c priority model RE-VALIDATED on the s7 form (not inherited — recomputed): pri = floor_log2(n_refs)*n_refs*size/live_length. From the .lreg banner: 79=6/4=3.00, 76=10/11=2.727, 77=8/10=2.400, 75=9/24=1.125, 73=8/22=1.0909, 87=2/2=1.000, 78=4/9=0.888, 74=6/16=0.750, 86=3/4=0.750, 81=3/20=0.150, 72=3/24=0.125. Predicted rank order [79 76 77 75 73 87 78 74 86 81 72] equals the greg's `;; 11 regs to allocate:` line VERBATIM.

- [s7] ALLOCATION IS A RANK->SEAT TABLE for this function. Eight allocnos (73,74,75,76,77,78,79,81) mutually conflict and each conflicts with hard reg 2, so in rank order they take exactly [$3,$5,$6,$7,$8,$9,$10,$11]. Observed: 79->$3, 76->$5, 77->$6, 75->$7, 73->$8, 78->$9, 74->$10, 81->$11 (87 is ST_REGS and takes no GR — it is the frame orphan; 86 conflicts with almost nothing and takes $2; 72 takes $4 by copy preference). TARGET's seating requires the rank order [79 76 77 75 78 73 74 81] — i.e. EXACTLY ONE ADJACENT SWAP of 73 (counter) and 78 (end pointer).

- [s7] REFS ARE WEIGHTED x2 FOR IN-LOOP REFERENCES, x1 outside (derived from the .lreg banner, cross-checked on 5 allocnos): 76 = 5 raw in-loop refs -> 10; 77 = 4 -> 8; 79 = 3 -> 6; 78 = 2 -> 4; 86 = 3 raw all outside the loop -> 3; 75 = 4 in-loop + the `la` outside -> 9. So one extra RAW in-loop reference moves an allocno by +2 refs, one outside-loop reference by +1.

- [s7] THE SWAP'S SOLUTION SET IS A SINGLE CELL under the current 36-insn stream. Required: pri(78) in (pri(73)=1.0909, pri(75)=1.125). With refs=4 -> 8/L: L=8 gives 1.000 (too low), L=7 gives 1.143 (overshoots 75 and steals $a3's seat). With refs=6 -> 12/L: L=11 ties 73 exactly and loses the tie (ties go to the lower pseudo number, and 73 < 78), L=10 gives 1.200 (overshoots). With refs=5 -> 10/L: L=9 gives 1.111, which is the ONLY value in the window. refs=5 means exactly one EXTRA reference to the end pointer OUTSIDE the loop, at zero instruction cost, with its live range unchanged at 9.

- [s7] The two remaining residuals are COUPLED through pri(75). If residual (a) is ever solved — the `la` moved into the preheader — reg 75 stops being live across block 0 and its priority rises from 1.125 toward ~1.227, which WIDENS the window to (1.0909, ~1.227) and makes refs=6/live_length=10 viable as well (pri 1.200). Solving (a) is therefore worth more than the two emission slots it directly buys: it is the only known way to open a second cell in the swap's solution set.

- [s7] Residual (a) mechanism is CONFIRMED but every measured spelling of it costs +2 insns: only loop.c writes into a preheader, so the `la` must be created inside the loop as loop-invariant / giv-base code; GCC 2.7.2 then reduces only the mult giv and re-adds the invariant base per iteration (`addu $v1,$t2,$t4`) plus a preheader `move t2,zero`, and separately peels the `var_t2 = 0` flag init. Two spellings measured, both score 19 / build_insns 38, byte-identical to each other.

- [s7] KILLED: inlining the end pointer into the inner-loop test (no `var_t0` local) demotes it from a global allocno to a block-5 local-alloc quantity in `$v0`; score regresses 9 -> 14. The end pointer must stay an outer-loop-scope named local.

- [s7] KILLED / FORECLOSED: re-weighting the COUNTER instead of the end pointer. Lowering pri(73) below 0.888 while keeping it above pri(74)=0.75 needs live_length in (27,32); the counter already spans 22 of ~26 RTL insns and nothing uses it after the loop, so the range cannot be lengthened without adding instructions. Dropping its refs 8 -> 7 lands it at 0.636, below the flag allocno, which mis-seats $t2.

- [s7] Instruments banked and reusable: tmp/grind/func_80037B00/s7/dis.sh (objdump of func_80037B00 out of the honest sandbox object — the fastest register/position gradient there is), s7/probe.sh + s7/apply.py (splice a variant body into src/code6cac_c.c and print the cc1 `.frame` line), s7/rainfo.sh (per-allocno refs/live-length + `regs to allocate` rank order + dispositions, straight out of the dump.ps1 -da dumps), s7/icc1.sh (instrumented-cc1 segfault repro).

- [s7] Chassis at s7 start with the s6 candidate applied: sandbox --disable all -> score=11, target_insns=36, build_insns=35, cheat_asm_stripped=3, rules_dropped=0. The s6 floor of 11 holds.

- [s7] NEW FLOOR 9: score=9, target_insns=36, build_insns=36, cheat_asm_stripped=3, rules_dropped=0, cc1 `.frame $sp,8,$31 # vars= 8`. Form banked at memory/grind/func_80037B00/candidate.c and tmp/grind/func_80037B00/s7/vA_bound_in_test.c (== FLOOR9.c).

- [s7] The instruction MULTISET now matches target exactly - `goal_from_tgt.py classify` reports FIRST DIVERGENCE: RA (every prior form reported PRE-RA / rtl_shape), with substitutions `$t0 -> $t1 x4`, `$t1 -> $t0 x2`; `goal_from_tgt.py goal` reports 5 renamed pairs, 0 skipped.

- [s7] TOOL DEFECT: `tools/ra_solver/extract.py func_80037B00 code6cac_c` fails with 'FATAL: func_80037B00 not in .ent list (7 functions)'. Root cause measured: the INSTRUMENTED cc1 (tools/gcc-2.7.2/cc1, the BB2_*_DEBUG build) SEGFAULTS (RC=139) partway through src/code6cac_c.c, right after func_800379D8 - i.e. on func_80037A20 - so its .s/.greg/.flow/.lreg cover only the first 7 functions. The build compiler tools/gcc-2.7.2/build/cc1 compiles the same TU fine. simulate.py / inverse.py / sweep.py are therefore UNAVAILABLE for every code6cac_c function after func_800379D8 until this is fixed. Repro: tmp/grind/func_80037B00/s7/icc1.sh. Workaround used: the plain -da dumps from `pwsh tools/grinder/dump.ps1` carry every input the priority model needs.

- [s7] global.c priority model RE-VALIDATED on the s7 form (recomputed, not inherited): 79=6/4=3.00, 76=10/11=2.727, 77=8/10=2.400, 75=9/24=1.125, 73=8/22=1.0909, 87=2/2=1.000, 78=4/9=0.888, 74=6/16=0.750, 86=3/4=0.750, 81=3/20=0.150, 72=3/24=0.125 -> rank order identical to the greg's allocation list.

- [s7] Allocation here is a pure RANK->SEAT table: eight allocnos (73,74,75,76,77,78,79,81) mutually conflict and each conflicts with hard reg 2, so in rank order they take [$3,$5,$6,$7,$8,$9,$10,$11]. Observed 79->$3, 76->$5, 77->$6, 75->$7, 73(counter)->$8, 78(end ptr)->$9, 74->$10, 81->$11. TARGET needs rank order [79 76 77 75 78 73 74 81] - exactly one adjacent swap of 73 and 78.

- [s7] REFS ARE WEIGHTED x2 FOR IN-LOOP REFERENCES, x1 outside (derived from the .lreg banner and cross-checked on five allocnos: 76 = 5 raw -> 10, 77 = 4 -> 8, 79 = 3 -> 6, 78 = 2 -> 4, 86 = 3 raw all outside the loop -> 3, 75 = 4 in-loop plus the outside `la` -> 9). One extra raw in-loop reference is +2 refs; one outside-loop reference is +1. CAVEAT worth re-measuring: allocno 73's block-0 init appears to be counted at weight 2 (measured 8, hand-count 7), most likely because the counter's init sits inside the loop notes (preheader) while the `la` sits before them - so an added 'outside-loop' reference may land at weight 2 rather than 1.

- [s7] THE SWAP'S SOLUTION SET IS A SINGLE CELL under the current 36-insn stream. Required: pri(78) in (pri(73)=1.0909, pri(75)=1.125). refs=4 -> 8/L: L=8 gives 1.000 (too low), L=7 gives 1.143 (overshoots 75 and steals $a3's seat). refs=6 -> 12/L: L=11 ties 73 exactly and loses the tie (ties go to the lower pseudo number and 73 < 78), L=10 gives 1.200 (overshoots). refs=5 -> 10/L: L=9 gives 1.111 - the ONLY value inside the window. So the swap needs exactly one extra reference to the end pointer, outside the loop, at zero instruction cost, with its live range unchanged at 9.

- [s7] The end pointer's live range CANNOT be shortened from 9 to 7: it spans its own set plus the seven-insn inner loop body, and shortening it means deleting inner-loop instructions the target also has.

- [s7] THE TWO RESIDUALS ARE COUPLED THROUGH pri(75). If the `la` ever moves into the preheader, reg 75 stops being live across block 0 and its priority rises from 1.125 toward ~1.227, WIDENING the swap's window to (1.0909, ~1.227) and making refs=6 / live_length=10 (pri 1.200) viable as a second cell. Solving the emission-position residual is therefore worth more than the two slots it directly buys.

- [s7] Pseudo numbers follow C DECLARATION ORDER exactly in this function (73..79 = var_t1, var_t2, var_a3, var_a1, var_a2, var_t0, var_v1), so declaration order is a real, ordinary-C dial on the priority TIE-BREAK - useful only if a spelling produces an exact priority tie.

- [s7] Instruments banked and reusable next session: s7/dis.sh (objdump of func_80037B00 straight out of the honest sandbox object - the fastest register-and-position gradient available), s7/probe.sh + s7/apply.py (splice a variant body into src/code6cac_c.c and print the cc1 `.frame` line), s7/rainfo.sh (per-allocno refs / live-length + `regs to allocate` rank order + dispositions out of the dump.ps1 -da dumps), s7/icc1.sh (instrumented-cc1 segfault repro).

- [s7] src/code6cac_c.c was reverted to HEAD (INCLUDE_ASM) at session end; the floor-9 form lives in memory/grind/func_80037B00/candidate.c.

- [s8] FORENSICS. Chassis re-measured at s8 start with the s7 candidate.c spliced over the INCLUDE_ASM line: `sandbox --disable all` -> score=9, target_insns=36, build_insns=36, cheat_asm_stripped=3, rules_dropped=0. The s7 floor of 9 holds; the driver's "measurement unavailable" chassis line is superseded by this measurement.

- [s8] THE s7 REF-WEIGHTING CAVEAT IS RESOLVED, AND IT RESOLVES IN FAVOUR OF WEIGHT 1. Read from the compiler source, not inferred: `tools/gcc-2.7.2/flow.c:434` initialises `depth = 1` and lines 439-443 increment/decrement it on NOTE_INSN_LOOP_BEG / NOTE_INSN_LOOP_END while recording `basic_block_loop_depth[i]`; `propagate_block` (flow.c:1385) seeds `loop_depth` from that array and the four ref sites (flow.c:2081, 2329, 2515, 2725) do `reg_n_refs[regno] += loop_depth`. So a reference OUTSIDE every loop note weighs exactly 1 and one inside the single outer loop weighs exactly 2. There is no third case in this function.

- [s8] THE s7 HAND-COUNT DISCREPANCY THAT PRODUCED THE CAVEAT IS EXPLAINED: `reg_n_refs` counts OCCURRENCES, not insns, so `(set (reg 73) (plus (reg 73) (const_int 1)))` contributes TWO refs, not one. Verified occurrence-by-occurrence against `tmp/grind/func_80037B00/s8/flow.rtl` (the .flow RTL for the s7 candidate): allocno 73 = init at flow.rtl:100 (weight 1, before NOTE_INSN_LOOP_BEG at :132) + duplicated-guard compare at :115 (weight 1) + `plus` set and use at :223/:224 (weight 2 each) + loop test at :274 (weight 2) = 8, exactly the .lreg banner's `used 8 times`. Allocno 78 = set at :151 (2) + slt use at :208 (2) = 4. Allocno 73's block-0 init is NOT counted at weight 2; s7's suspicion was wrong.

- [s8] FRONTIER #1 (s7's "one extra outside-loop reference to var_t0, refs 4 -> 5, live_length 9, pri 1.111") IS FORECLOSED ON THE candidate.c CHASSIS - for a structural reason, not an arithmetic one. The weight-1 region of this function is exactly: block 0 (`var_t1 = 0`, the `la`, the count `lw`), the insns loop.c's `duplicate_loop_exit_test` copies in front of NOTE_INSN_LOOP_BEG (the compare, the branch, and its `(set (reg 81) (reg 86))` transfer copy = target's `addu $t3,$v0,$zero`), and the two return blocks. The end pointer var_t0 is defined inside the outer loop body and is dead in every one of those regions, so there is NO insn outside the loop notes into which a reference to it can be folded at zero instruction cost. Adding an out-of-loop insn to carry the reference costs +1 and breaks the exact 36-insn multiset. The cell (refs 5, live 9) is reachable in ARITHMETIC but has no spelling site.

- [s8] NEW CHASSIS FAMILY FOUND - "explicit if + do-while", banked as memory/grind/func_80037B00/alt_base_vF2.c (== tmp/grind/func_80037B00/s8/vF2_init_before_if.c). Shape: `var_t1 = 0; if (D_800A38C8 > 0) { var_a3 = (s8*)&D_80102810; do { <s0..s5 goto body> } while (var_t1 < D_800A38C8); } return 0;`. Measured: score=10, build_insns=34, target_insns=36, cc1 `.frame $sp,0,$31 # vars= 0`. Every instruction target has except the two `addiu $sp` frame adjusts, and the ONLY register error is a counter/flag adjacent swap.

- [s8] alt_base_vF2 SOLVES RESIDUAL (a), THE `la` EMISSION POSITION, AT ZERO COST - the thing s7 measured at +2 via every giv spelling. Mechanism: with the guard written as a source-level `if`, the guard branch precedes the `var_a3 = &D_80102810;` statement in RTL order, so `lui a3 / addiu a3` land inside the guarded preheader exactly where target has them. No giv, no strength reduction, no loop.c hoist needed. `rejected/giv-index-costs-offset-biv-add.c` and `rejected/giv-pure-coalesced-into-walking-ptr.c` were solving the wrong problem: the la does not have to be hoisted, it has to be written after the guard.

- [s8] alt_base_vF2 ALSO KEEPS target's `addu $t3,$v0,$zero` preheader copy, by the same pass-created mechanism s7 found: the do-while's bottom test RE-READS the global `D_800A38C8` (no cached local), loop.c hoists that load and cse unifies it with the guard's load into a register copy. Caching the count in a local instead kills the copy - banked rejected/if-dowhile-count-local-no-preheader-copy.c (score 11, build_insns 34, `lw t3,0(gp)` and no copy).

- [s8] alt_base_vF2 ALSO FILLS THE `blez` DELAY SLOT with the counter init, matching target's `addu $t1,$zero,$zero`, because `var_t1 = 0;` is placed BEFORE the `if` and reorg fills from the PRECEDING insn. With the init inside the if-body the slot takes a nop and the form measures 35 insns (tmp/grind/func_80037B00/s8/vF_if_dowhile_reload.c, score 10).

- [s8] KILLED: explicit `if` + TOP-TESTED `while` (both spellings). `if (D_800A38C8 > 0) { ... while (var_t1 < D_800A38C8) { ... } }` measures score=10 / build_insns=37 with the init inside the if, and score=14 / build_insns=37 with the init before the if. cc1 emits TWO `blez $2,.L40` branches with the `la` between them: the explicit guard does NOT suppress loop.c's `duplicate_loop_exit_test`, which copies the while's exit test in front of NOTE_INSN_LOOP_BEG regardless. Banked rejected/if-while-init-before-if-double-guard.c.

- [s8] IMPORTANT POSITIVE FROM THAT KILL: the with-init-inside variant measures cc1 `.frame $sp,8,$31 # vars= 8` AND has the `la` inside the guarded region. So the 8-byte phantom frame and the correct `la` position are SIMULTANEOUSLY REACHABLE - they are not in tension. On this chassis they are produced by the SAME construct (the duplicated exit test is both the frame's orphan producer and the redundant branch), so the whole remaining question on that branch is whether the redundant entry branch can be removed while keeping the orphan.

- [s8] RA ARITHMETIC FOR THE alt_base_vF2 CHASSIS (recomputed from its own .lreg/.greg, not inherited). Ten allocnos; the priority model reproduces the greg's `;; 10 regs to allocate: 79 76 77 75 78 74 73 81 85 72` verbatim: 79 = 6 refs/4 insns = 3.00, 76 = 10/11 = 2.727, 77 = 8/10 = 2.400, 75 = 9/21 = 1.2857, 78 = 4/9 = 0.888, 74 = 6/16 = 0.750, 73 = 7/23 = 0.6087, 81 = 3/5 = 0.600, 85 = 3/20 = 0.150, 72 = 3/24 = 0.125. Allocno 81 does not conflict with the mutually-conflicting eight and takes $2, so the seat table is 79->$3, 76->$5, 77->$6, 75->$7, 78->$8, 74->$9, 73->$10, 85->$11. Target needs 73 and 74 swapped (counter -> $t1 = $9, flag -> $t2 = $10); the end pointer 78 is ALREADY on target's $t0 = $8.

- [s8] THAT SWAP IS FORECLOSED UNDER THE CURRENT STREAM. Required: pri(73) strictly inside (pri(74) = 0.750, pri(78) = 0.888). With refs 7 (floor_log2 = 2) that needs live_length in {16, 17, 18}; the counter is live from its init to the bottom test and measures 23 in a ~26-insn function, and shortening it means moving the init after the guard, which costs the delay-slot fill. With refs 8 (floor_log2 = 3) it needs live_length in (27, 32), longer than the entire function. refs 6 needs live_length in (13.5, 16), shorter still. Lowering pri(74) below 0.6087 instead needs live_length(74) >= 20 against a measured 16, and the flag's three raw references are exactly target's three `$t2` instructions so its ref count cannot move. No cell.

- [s8] Also note the la's side effect on the coupling s7 identified: moving the `la` into the preheader shortens allocno 75's (var_a3) live range from 24 to 21 and raises its priority from 1.125 to 1.2857 - the widened window s7 predicted. On the alt_base_vF2 chassis that window is not needed, because 78 already seats correctly there.

- [s8] Artifacts: tmp/grind/func_80037B00/s8/flow.rtl (the .flow RTL slice for the floor-9 form, with the NOTE_INSN_LOOP_BEG at line 132 / NOTE_INSN_LOOP_END at line 286 that define the weight-1 region), s8/apply.py (encoding-safe splice: handles the INCLUDE_ASM line AND a previously spliced body; the s7 apply.py crashes on both counts), s8/vE_if_dowhile.c, s8/vF_if_dowhile_reload.c, s8/vF2_init_before_if.c (the new alternative base), s8/vG_if_while.c, s8/vG2_init_before_if_while.c.

- [s8] TOOL NOTE: tmp/grind/func_80037B00/s7/probe.sh prints nothing on a successful run - its awk pattern for the `.ent` line does not match the tab cc1 emits. The frame is still written to tmp/grind/func_80037B00/s7/raw.s; read the `.frame` line there (or from the `.s` under tmp/grind/func_80037B00/dumps/) instead of trusting the empty output. s7/dis.sh also needs its `cd` line removed and must be run through WSL (`bash tools/wsl.sh 'bash .../dis.sh'`), and it reads tmp/sandbox/func_80037B00/code6cac_c.o, which is only refreshed by an actual `sandbox` run - a stale object silently misreports the variant you just spliced.

- [s8] src/code6cac_c.c was reverted to HEAD (INCLUDE_ASM) at session end. The floor-9 form remains memory/grind/func_80037B00/candidate.c; the new alternative base is memory/grind/func_80037B00/alt_base_vF2.c.

- [s8] FLOOR LOWERED 9 -> 5, AND THE INSTRUCTION STREAM IS NOW EXACT POSITION-FOR-POSITION. Form banked at memory/grind/func_80037B00/candidate.c (== tmp/grind/func_80037B00/s8/FLOOR5.c == s8/vI_count_local_in_if.c): `var_t1 = 0; if (var_t1 < D_800A38C8) { var_t3 = D_800A38C8; var_a3 = (s8*)&D_80102810; while (var_t1 < var_t3) { <s0..s5 goto body, with var_a3 += 0x28 at block_74> } } return 0;`. Measured: score=5, target_insns=36, build_insns=36, cheat_asm_stripped=3, rules_dropped=0, cc1 `.frame $sp,8,$31 # vars= 8`. objdump of the honest sandbox object matches asm/funcs/func_80037B00.s opcode-for-opcode and slot-for-slot; the ONLY difference in the whole function is that our outer counter takes $t0 and our inner end pointer takes $t1 where target has counter=$t1 and end=$t0.

- [s8] THE UNLOCK WAS SPELLING THE ENTRY GUARD AS THE LOOP'S OWN TEST. `if (var_t1 < D_800A38C8)` with var_t1 already 0, wrapped around a top-tested `while`, collapses to ONE `blez` while still letting loop.c's duplicate_loop_exit_test run (so the phantom-frame orphan survives and vars stays 8). Writing the same guard as `if (D_800A38C8 > 0)` emits TWO `blez` branches (+3 insns, score 10 / 37) because the source guard and the duplicated exit test are not the same jump for jump2 to merge. That single spelling change is what made the frame, the la position and the single guard co-exist.

- [s8] MEASURED LADDER FOR THE FINAL TWO EMISSION SLOTS. tmp/grind/func_80037B00/s8/vH_guard_is_loop_test.c (guard-as-loop-test, bound RE-READ in the while test, no var_t3 local) measures score=7 / build_insns=36 with the whole stream correct EXCEPT that `move t3,v0` lands AFTER `lui/addiu a3`: on that spelling the copy is loop.c's duplicate_loop_exit_test TRANSFER copy, inserted in front of NOTE_INSN_LOOP_BEG and therefore after the la. Reading the bound into `var_t3` as a source statement placed BEFORE the `var_a3` assignment (candidate.c) makes cse produce the same copy from the guard's already-loaded value, in source order, i.e. BEFORE the la — target's order. Score 7 -> 5.

- [s8] So all three of s7's residual axes are now CLOSED: (a) the `lui/addiu` emission position, (b) the preheader copy, and the frame — plus the `blez` delay-slot fill. What remains is exactly the one adjacent RA swap, and its window is now much wider than s7's.

- [s8] RA MODEL ON THE FLOOR-5 FORM (recomputed from its own .lreg/.greg): 79 = 6 refs/4 insns = 3.00, 76 = 10/11 = 2.727, 77 = 8/10 = 2.400, 75 = 9/20 = 1.350, 73 (counter) = 8/23 = 1.0435, 78 (end ptr) = 4/9 = 0.888, 74 (flag) = 6/16 = 0.750, 81 = 3/21 = 0.143, 72 = 3/24 = 0.125. The greg prints `;; 11 regs to allocate: 79 76 77 75 73 83 78 74 82 81 72` — the predicted order verbatim (83 is the frame orphan, ST_REGS, takes no GR; 82 takes $2). Dispositions: 73 in 8, 78 in 9, 74 in 10, 75 in 7, 76 in 5, 77 in 6, 79 in 3, 81 in 11, 72 in 4.

- [s8] THE SWAP'S SOLUTION SET IS NOW A RANGE, NOT A SINGLE CELL. Required: pri(78) strictly inside (pri(73) = 1.0435, pri(75) = 1.350) — s7's window was (1.0909, 1.125). Reachable cells, using pri = floor_log2(refs)*refs/live_length: refs 4 -> live_length 6 or 7; refs 5 -> live_length 8 or 9 (9 is the CURRENT live length, giving 1.111); refs 6 -> live_length 9, 10 or 11 (9 gives 1.333, 10 gives 1.200, 11 gives 1.0909). refs 6 means ONE extra IN-LOOP raw reference to the end pointer with the live range allowed to grow by up to 2 insns — a far weaker requirement than s7's "one out-of-loop reference at zero cost, live range frozen at 9". Alternatively lower pri(73): with refs 8 that needs live_length in (27, 32) against a measured 23; with refs 7, live_length in (15.8, 18.7).

- [s8] Note the coupling s7 predicted has materialised: allocno 75 (var_a3) now measures live_length 20 (was 24) because the `la` moved into the preheader, so pri(75) rose 1.125 -> 1.350 and that is precisely what widened the window. s7's analysis of the coupling was correct; only its proposed spelling for (a) was wrong.

- [s8] Artifacts added: tmp/grind/func_80037B00/s8/vH_guard_is_loop_test.c (score 7 / 36), s8/vI_count_local_in_if.c == s8/FLOOR5.c (score 5 / 36, the new floor).

- [s8] NEW FLOOR 5 (was 9). memory/grind/func_80037B00/candidate.c: `var_t1 = 0; if (var_t1 < D_800A38C8) { var_t3 = D_800A38C8; var_a3 = (s8*)&D_80102810; while (var_t1 < var_t3) { <s0..s5 goto body, var_a3 += 0x28 at block_74> } } return 0;`. sandbox --disable all -> score=5, target_insns=36, build_insns=36, cheat_asm_stripped=3, rules_dropped=0, cc1 `.frame $sp,8,$31 # vars= 8`. Verified against the measured file: the candidate body is byte-identical (whitespace-normalised) to tmp/grind/func_80037B00/s8/FLOOR5.c.

- [s8] The 36-instruction stream is EXACT POSITION-FOR-POSITION against asm/funcs/func_80037B00.s — same opcodes, same operands, same order, including `addiu sp,sp,-8` / `addiu sp,sp,8`, `addu $t3,$v0,$zero`, the `lui/addiu D_80102810` pair inside the guarded preheader, and the counter init in the blez delay slot. The ONLY difference anywhere in the function is that our outer counter takes $t0 and our inner end pointer takes $t1, where target has counter=$t1 and end=$t0.

- [s8] THE UNLOCK: spell the entry guard AS the loop's own test. `if (var_t1 < D_800A38C8)` with var_t1 already 0, wrapped around a top-tested `while`, collapses to ONE `blez` while loop.c's duplicate_loop_exit_test still runs and still strands the compare pseudo that reload's alter_reg pays off as the 8-byte frame. Writing the same guard as `if (D_800A38C8 > 0)` emits TWO `blez` branches (+3 insns, score 10 / 37).

- [s8] THE SECOND STEP: read the bound into `var_t3` as a source statement placed BEFORE the `var_a3` assignment. cse turns it into a register copy off the guard's already-loaded value, and source order puts that copy AHEAD of the la — target's order. Re-reading the bound in the while test instead gives the copy as duplicate_loop_exit_test's transfer copy, inserted in front of NOTE_INSN_LOOP_BEG and therefore AFTER the la (tmp/grind/func_80037B00/s8/vH_guard_is_loop_test.c, score 7 / 36).

- [s8] Every construct in the floor-5 form is ordinary live C: named locals that are all read, a real entry guard, a real loop bound, a real walking pointer. No dead store, no holder, no array, no volatile, no wrapper, no annotation-requiring construct of any kind.

- [s8] GCC 2.7.2 ref weighting is now read off the compiler source, not inferred: flow.c:434 initialises depth=1; flow.c:439-443 adjusts it on NOTE_INSN_LOOP_BEG/LOOP_END into basic_block_loop_depth[]; flow.c:1385 seeds propagate_block's loop_depth from it; flow.c:2081/2329/2515/2725 do reg_n_refs[regno] += loop_depth. Out-of-loop = 1, inside the outer loop = 2.

- [s8] reg_n_refs counts OCCURRENCES, not insns: `(set (reg 73) (plus (reg 73) (const_int 1)))` contributes 2. This corrects the s7 'block-0 init measured at weight 2' anomaly and is load-bearing for every future priority computation on this function.

- [s8] RA model recomputed on the floor-5 form from its own .lreg/.greg: 79 = 6 refs/4 insns = 3.00, 76 = 10/11 = 2.727, 77 = 8/10 = 2.400, 75 = 9/20 = 1.350, 73 (counter) = 8/23 = 1.0435, 78 (end ptr) = 4/9 = 0.888, 74 (flag) = 6/16 = 0.750, 81 = 3/21 = 0.143, 72 = 3/24 = 0.125. The greg prints `;; 11 regs to allocate: 79 76 77 75 73 83 78 74 82 81 72` — the predicted order verbatim (83 is the frame orphan, ST_REGS, takes no GR; 82 takes $2). Dispositions: 73 in 8, 78 in 9, 74 in 10, 75 in 7, 76 in 5, 77 in 6, 79 in 3, 81 in 11, 72 in 4.

- [s8] THE SWAP'S SOLUTION SET IS NOW A RANGE, NOT A SINGLE CELL. Required: pri(78) strictly inside (pri(73) = 1.0435, pri(75) = 1.350); s7's window was (1.0909, 1.125). Reachable cells: refs 4 -> live_length 6 or 7; refs 5 -> live_length 8 or 9 (9 is the current live length, 1.111); refs 6 -> live_length 9, 10 or 11 (1.333 / 1.200 / 1.0909). Symmetrically, lowering pri(73) below 0.888 while keeping it above pri(74) = 0.750 needs refs 8 with live_length in (27, 32) against a measured 23, or refs 7 with live_length in (15.8, 18.7).

- [s8] The coupling s7 predicted has materialised and is what widened the window: allocno 75 (var_a3) now measures live_length 20 (was 24) because the la moved into the preheader, so pri(75) rose 1.125 -> 1.350.

- [s8] ALTERNATIVE BASE banked at memory/grind/func_80037B00/alt_base_vF2.c (explicit `if (D_800A38C8 > 0)` + do-while, score 10 / 34 insns, vars=0): superseded by the floor-5 form but kept because it is the cleanest demonstration that the la position is a guard-placement question, and because its RA table (end pointer already on target's $t0, counter/flag transposed instead) is a distinct starting point if the floor-5 swap proves unreachable.

- [s8] Caching the bound BEFORE the `if` instead of inside it kills the preheader copy entirely — cse then has a single pseudo for guard and loop test (rejected/if-dowhile-count-local-no-preheader-copy.c, score 11 / 34, `lw t3,0(gp)` and no copy).

- [s8] Placing `var_t1 = 0;` inside the guarded body instead of before the `if` costs the delay-slot fill: cc1 emits a nop after the blez (tmp/grind/func_80037B00/s8/vF_if_dowhile_reload.c, 35 insns). reorg fills a branch delay slot from the PRECEDING insn first.

- [s8] TOOL NOTE for the next session: tmp/grind/func_80037B00/s7/probe.sh prints nothing even on success (its awk pattern misses the tab in cc1's `.ent` line) — read the `.frame` line out of tmp/grind/func_80037B00/s7/raw.s instead. s7/apply.py crashes both on the INCLUDE_ASM HEAD state and on cp1252 bytes elsewhere in code6cac_c.c; use the fixed tmp/grind/func_80037B00/s8/apply.py. s7/dis.sh needs its `cd` line stripped, must run under WSL (`bash tools/wsl.sh 'bash .../dis.sh'`), and reads tmp/sandbox/func_80037B00/code6cac_c.o, which only refreshes on an actual `sandbox` run — a stale object silently misreports the variant you just spliced.

- [s8] src/code6cac_c.c was reverted to HEAD (INCLUDE_ASM) at session end; no tracked build file is modified.

## s9 (forensics, 2026-08-26) — pass-attributed facts about the last two registers

1. **Chassis re-verified.** candidate.c measures score 5 / 36 vs 36 / 0 rules on
   HEAD this session. The RA inputs reproduce exactly: allocno 73 (counter
   var_t1) = 8 refs / 23 live, allocno 78 (end pointer var_t0) = 4 refs / 9
   live; greg order `79 76 77 75 73 83 78 74 82 81 72`; dispositions 73→$8,
   78→$9 where target wants 73→$9 ($t1) and 78→$8 ($t0).

2. **global.c's priority is integer-truncated.** `global.c:642-649` casts
   `(double)(floor_log2(refs)*refs)/live * 10000 * size` to int, and
   `global.c:655` breaks ties toward the LOWER allocno. Baseline integer
   priorities: 79=30000, 76=27272, 77=24000, 75=13500, 73=10434, 78=8888,
   74=7500, 81=1428, 72=1250. Full reachable-cell enumeration:
   `tmp/grind/func_80037B00/s9/prisolve.py`.

3. **An extra flow-visible insn that does NOT mention the end pointer can never
   help.** Any insn added inside the inner loop lengthens BOTH live ranges, and
   `8/(9+N) > 24/(23+N)` has no positive solution. The perturbation must add a
   reference to allocno 78.

4. **flow.c deletes cse-dead insns BEFORE counting them.** Traced insn-by-insn
   through .rtl/.jump/.cse/.loop/.cse2/.flow for the split-definition spelling
   (`var_t0 = var_a3; var_t0 += 0x15;`): cse1 rewrites the second insn's operand
   from reg 78 to reg 75, the now-dead copy survives loop and cse2 intact, and
   the FLOW pass turns it into `NOTE_INSN_DELETED`. Since propagate_block's
   dead-insn deletion and its `reg_n_refs` accumulation are the same scan, the
   occurrence is never credited. Corollary and permanent correction to the s8
   model note: "reg_n_refs is computed before combine" only buys an occurrence
   if the insn is LIVE at flow time. The only insn-deleting pass after flow is
   combine (toplev.c:3004), and the only combine absorption available on MIPS
   for a pointer is folding `(plus reg const)` into a `mem` address — target's
   36-instruction stream contains no memory access based on the end pointer.

5. **cse1 does NOT reach across the inner loop's blocks.** Reusing the end
   pointer to step to the next table entry (`var_a3 = var_t0 + 0x13;` at
   block_74) is NOT folded back to `var_a3 + 0x28`: allocno 78 measures 6 refs /
   14 live, the exact +2 the weight-2 model predicts. So a second in-loop
   reference IS obtainable in ordinary C; the binding constraint is distance,
   not cse. To land inside the window the extra reference must keep
   live_length(78) <= 11, i.e. it must sit within ~2 insns of the `slt` that is
   currently the end pointer's last use.

6. **Declaration order = pseudo order** on this chassis (measured, not inferred):
   declaring `var_t0` before `var_t1` renumbers the end pointer to allocno 73
   and the counter to 74 with every priority and every seat unchanged. This
   makes the end pointer win an exact tie, converting the target window from
   `pri(end) > pri(counter)` to `>=` at zero cost. Banked as
   memory/grind/func_80037B00/decl_order_swap_vC.c — apply it only in
   combination with a priority-moving change.

- [s9] HEAD chassis re-measured this session: memory/grind/func_80037B00/candidate.c gives sandbox --disable all score 5, target_insns 36, build_insns 36, rules_dropped 0; the s8 RA table reproduces exactly (73 = 8 refs/23 live -> $8, 78 = 4 refs/9 live -> $9; greg order 79 76 77 75 73 83 78 74 82 81 72).

- [s9] global.c:642-649 truncates the priority to int: the baseline integers are 79=30000, 76=27272, 77=24000, 75=13500, 73=10434, 78=8888, 74=7500, 81=1428, 72=1250; global.c:655 breaks ties toward the lower allocno.

- [s9] Reachable cells that transpose the pair and leave every other seat intact (tmp/grind/func_80037B00/s9/prisolve.py): end pointer at (refs 4, live 6), (4,7), (5,8), (5,9), (6,9), (6,10), (6,11); counter at (7,16..18) or (8,28..32).

- [s9] PASS ATTRIBUTION: a same-block re-mention of the end pointer is folded by cse1 (visible as (plus 78 21) -> (plus 75 21) in x.cse), and the resulting dead insn survives loop and cse2 but is turned into NOTE_INSN_DELETED by the FLOW pass - so it is never counted. 'reg_n_refs is computed before combine' only buys an occurrence whose insn is LIVE at flow time.

- [s9] The only insn-deleting pass after flow is combine (toplev.c:3004); the only combine absorption available for a pointer on MIPS is folding (plus reg const) into a mem address, and target's 36-instruction stream has no memory access based on the end pointer - so a zero-cost extra reference cannot come from a combine-absorbed insn on THIS stream.

- [s9] A cross-block reference is NOT folded by cse1: vG raises allocno 78 to 6 refs / 14 live. The refs axis is therefore open; the binding constraint is that live_length(78) must stay <= 11, i.e. the extra reference must sit within about two insns of the slt that is currently its last use.

- [s9] Declaration order = pseudo order (measured with vC): declaring var_t0 first renumbers the end pointer to allocno 73 and the counter to 74 with identical priorities and identical seats - a free precondition for the tie-break route.

- [s9] The counter side is the weaker axis: its window needs either refs 7 with live 16-18 (the only removable weight-1 occurrence is the guard's compare, and removing it breaks the single-blez merge the floor-5 form depends on - measured in s8 as the double-blez family) or refs 8 with live 28-32, i.e. the counter live in the return blocks, which costs an instruction.

## s10 (rederive, 2026-08-26) — the residual is now an ARITHMETIC INVARIANT, not a spelling search

- [s10] CHASSIS RE-VERIFIED ON HEAD. `memory/grind/func_80037B00/candidate.c` spliced into
  src/code6cac_c.c measures `sandbox func_80037B00 --disable all` -> score 5, target_insns 36,
  build_insns 36, rules_dropped 0, cheat_asm_stripped 3. The s8/s9 RA table reproduces verbatim:
  73 (counter var_t1) = 8 refs / 23 live -> $8, 78 (end pointer var_t0) = 4 refs / 9 live -> $9,
  greg order `79 76 77 75 73 83 78 74 82 81 72`. Target wants 78->$8 and 73->$9.

- [s10] REDERIVE — SIBLING TRANSPLANT (new to this ledger; nine sessions never looked at it).
  `func_80037AA4` (src/code6cac_c.c:285, MATCHED) and `func_80037A20` (src/code6cac_c.c:268,
  MATCHED) walk the SAME table (`D_80102810`, stride 0x28) with the SAME counter/bound global
  (`D_800A38C8`). func_80037AA4's accepted C is
  `var_a1 = 0; var_a2 = D_800A38C8; if (var_a1 < var_a2) { var_v1 = (s8 *)&D_80102810;
  do { ... var_a1 += 1; var_v1 += 0x28; } while (var_a1 < var_a2); }` — i.e. the counter-vs-bound
  entry guard wrapped around the table walk is the ORIGINAL AUTHOR'S HOUSE IDIOM for this table,
  and candidate.c's chassis is a faithful reconstruction of it, not an artefact of our search.
  This answers the "is the floor-5 chassis the right shape?" question affirmatively.
  NOTE FOR ANY FUTURE ESCALATION: func_80037AA4 closed its OWN final RA residual with a
  Judge-sanctioned (2026-07-28) `/* FAKE */` constant-holder local (`s32 sh = 0xD;`) whose only
  job was to lift an allocno's priority; reload's `update_equiv_regs` then deleted the `li` at
  zero byte cost. That is a same-file, same-table, same-pass precedent — but it is NOT applicable
  here: see the arithmetic below, no constant-holder can move this function's numbers.

- [s10] REDERIVE — KENGO: no transplant exists. `kengo_matches.csv` rates func_80037B00 against
  `ki_every_little_thing` (src/ishito/is_ki_control.c) as `size-only-ambiguous` (36 vs 36 insns,
  similarity scores 0.00 / 0.00 / 0.00 / 0.42), i.e. size coincidence only, consistent with
  [[slog-kengo-dead-end]]. The same row shape holds for both siblings. Do not re-run Kengo here.

- [s10] THE SEAT RULE, RE-READ FROM SOURCE. `global.c:640-655` (`allocno_compare`) sorts by
  `(int)((double)(floor_log2(refs)*refs)/live_length * 10000 * allocno_size)`, ties broken toward
  the LOWER allocno number; `allocno_size` is 1 for every SImode allocno here, so it is inert.
  Seats then fall out in rank order ($3,$5,$6,$7,$8,$9,$10,$11) for every allocno without a
  hard-register copy preference (only 72 -> $4 and 82/84/87 -> $2 have one, from copies of a0/v0).
  So the swap is EXACTLY: make pri(78) >= pri(73) (with 78 the lower allocno) while keeping
  pri(75) = 13500 above it and pri(74) = 7500 below it.

- [s10] **pri(78) IS INVARIANT AT 8888 ON ANY CHASSIS THAT EMITS TARGET'S 36 INSTRUCTIONS.**
  Two independent legs, both now measured rather than argued:
  (a) refs(78) = 4 is fixed. `$t0` appears in exactly TWO instructions of target's stream — its
      definition `addiu $t0,$a3,0x15` and the loop test `slt $v0,$a1,$t0`. One occurrence each,
      both inside the outer loop (weight 2 per flow.c:434 / flow.c:2081) = 4. A third occurrence
      must come from an insn that either (i) materialises bytes (the stream is already exact at
      36, so any new insn regresses), (ii) replaces an existing insn's operands (no other target
      insn could name $t0 and keep its encoding), or (iii) is deleted before it is counted — and
      s9 proved flow.c's propagate_block deletes cse-dead insns in the SAME scan that accumulates
      reg_n_refs, so (iii) is never credited.
  (b) live_length(78) = 9 is fixed. Read off `tmp/grind/func_80037B00/s9/flow.rtl`: 78's span is
      insn 46 (def) .. insn 82 (the `slt` set), and the pre-combine chain between them is exactly
      the inner loop — 46 def, 50 `(set (reg:QI 85) (mem))`, 51 `(set (reg 79) (zero_extend 85))`,
      54 branch_zero, 63/64 the second QI load + zero_extend, 67 branch_equality, 76 `a1 += 1`,
      79 `a2 += 1`, 82 `slt`. Every one of those ten insns is REQUIRED by the 36-insn stream
      (2 lbu + 2 branches + 2 increments + slt + def), so the span cannot shrink.
  Measured attempts to shrink it, all inert or worse:
  * `rejected/u8-pointer-retyping-ra-inert.c` — var_a1/var_a2/var_a3/var_t0 retyped `u8 *`, casts
    dropped: RA table byte-identical to baseline (78 still 4/9). The QI-load + zero_extend split
    is NOT a product of the `(u8)` cast.
  * `rejected/u8-scalar-retyping-ra-inert.c` — var_v1/var_v0 retyped `u8`: same, 78 still 4/9.
  * `rejected/a2-inc-in-taken-arm-lengthens-both-ranges.c` — `var_a2 += 1` moved inside the
    loop-back arm (semantically identical; a2 is dead on the exit edge): cc1 puts the increment
    back in the delay slot and the emitted stream is unchanged, but the extra arm block costs
    BOTH ranges +1 (73 -> 8/24, 78 -> 4/10 = pri 8000). Strictly worse.

- [s10] **CONSEQUENCE: the only free variable left is live_length(73), and it must reach 27.**
  With pri(78) pinned at 8888 and refs(73) pinned at 8 (init 1 + guard 1 + `t1 += 1` 2x2 +
  while-test 1x2), pri(73) = 24/live*10000 must land in (7500, 8888] — 7500 is pri(74), the flag,
  which must stay below. live(73) = 27 gives exactly 8888 (a TIE, won by the end pointer once
  `memory/grind/func_80037B00/decl_order_swap_vC.c` makes it the lower allocno); live 28..31 wins
  outright; 32 loses the flag. Current live(73) = 23, so the requirement is **+4 flow-visible
  insns that emit no bytes**, placed where 73 is live and 78 is not.
  Every other cell is arithmetically unreachable: refs(73) = 7 needs live 16..18 (a SHORTER range,
  and the only removable weight-1 occurrence is the guard compare, whose removal costs the
  single-blez merge — the s8 double-blez family); refs 6 needs live 14..15; refs 9 needs live
  31..35; refs 8 is what the stream dictates.

- [s10] **REGION MAP for "counts toward 73 but not 78" (measured, three probes).**
  * PREHEADER — counts. `rejected/preheader-foldable-chain-plus1-only.c`
    (`var_a3 = (s8 *)&D_80102810 + 0x14; var_a3 -= 0x14;`): cc1 stream UNCHANGED at 26 cc1 insns
    (combine folds the pair), 73 goes 23 -> 24 live, 78 stays 4/9. This is the first measured
    zero-byte-cost +1 on the counter axis in this function's history.
  * BLOCK_5C — counts. `tmp/grind/func_80037B00/s10/vG1_stride_at_5c.c` (half the 0x28 stride
    applied at block_5c, half at block_74): 73 -> 24 live, 74 -> 17, 78 unchanged. But combine
    cannot fold across the block boundary, so it materialises a 27th cc1 insn. Byte-dead.
  * BLOCK_74 — DOES NOT COUNT. `rejected/stride-split-block74-no-counter-live-change.c`
    (`var_a3 += 0x14; var_a3 += 0x14;` both at block_74): 75 goes 9 -> 13 refs (so both insns ARE
    live at flow time and ARE counted for 75), yet 73's live_length stays 23. flow.c:1685 only
    increments `reg_live_length` for registers in `regs_sometimes_live`; 73 is live across the
    whole of block_74 without changing state there, so the extra insn is invisible to it. This is
    a permanent correction to the naive "live_length = insn span" model used in s7-s9.

- [s10] **THE +4 IS NOT REACHABLE EVEN BY STACKING THE +1 MECHANISM.**
  `rejected/preheader-const-chain4-cse1-collapses.c` builds the maximal version of the lever — a
  fresh pointer temp carrying a four-link constant chain in the preheader
  (`tmp = &D_80102810 + 0x10; tmp += 0x10; tmp += 0x10; tmp += 0x10; var_a3 = tmp - 0x40;`) plus
  the decl-order swap — and it buys only +1, not +4: the counter measures 8 refs / 24 live (pri
  10000, still above the end pointer's 8888) and a 5-insn residue pseudo (3 refs / 5 live) appears.
  cse1 constant-folds the chain before flow ever sees it and flow deletes the dead links, exactly
  as s9's fact 4 predicts. So the +1 of the two-link `+0x14 / -0x14` form is not a unit that can be
  repeated; it survives only because its two links are not constant-foldable into one another's
  operand in a way cse1 recognises, and a longer chain IS.

- [s10] NET: the floor stays 5 and the SHAPE of the remaining question has changed. It is no longer
  "find a spelling"; it is "produce four flow-visible, byte-free insns in the preheader or
  block_5c". Every construct that does that is, by inspection, semantically purposeless (it exists
  only to lengthen a live range) — a T1/T2/T3 failure, and at best the F1 combine-foldable
  chain-extender family, which the measurement above shows caps out at +1 here.

- [s10] src/code6cac_c.c reverted to HEAD (INCLUDE_ASM) at session end; no tracked build file is
  modified. Artifacts: tmp/grind/func_80037B00/s10/{probe.sh,apply.py,flowdbg.sh,findcc1.sh,
  vBASE.c,vA_a2inc_in_arm.c,vC_end_from_a1.c,vD_u8_ptrs.c,vE_u8_scalars.c,vF_split_stride_probe.c,
  vG1_stride_at_5c.c,vG2_preheader_chain.c,vH_model_proof_chain4.c,d_*/}.

- [s10] TOOL NOTE: `tools/gcc-2.7.2/cc1` IS the instrumented build (it carries the `BB2_FLOW_DEBUG`
  hook at flow.c:1685 that prints `FLOWDBG reg=N insn=U bnum=B` for one regno), but
  `engine/buildconfig.py` builds with `tools/gcc-2.7.2/build/cc1`, which is NOT instrumented. The
  two binaries do not agree on pseudo numbering for this TU (the instrumented one printed only 6
  FLOWDBG lines for reg 78 across the whole file, none of them func_80037B00's), so the hook cannot
  audit this function's live ranges without first rebuilding the instrumented cc1 from current
  source. Use the `.lreg` banner from `tmp/grind/func_80037B00/s10/probe.sh` instead — it is
  calibrated against the s8/s9 tables and reproduces them exactly.

- [s10] HEAD chassis re-verified this session: memory/grind/func_80037B00/candidate.c spliced into src/code6cac_c.c measures sandbox --disable all score 5, target_insns 36, build_insns 36, rules_dropped 0, cheat_asm_stripped 3; the s8/s9 RA table reproduces verbatim (73 = 8 refs/23 live -> $8, 78 = 4 refs/9 live -> $9, greg order 79 76 77 75 73 83 78 74 82 81 72).

- [s10] REDERIVE, SIBLING: func_80037AA4 (src/code6cac_c.c:285) and func_80037A20 (src/code6cac_c.c:268) are both MATCHED, both walk D_80102810 at stride 0x28, and both use D_800A38C8 as the entry count. func_80037AA4's accepted C is the counter-vs-bound entry guard wrapped around the table walk - the same chassis candidate.c uses. The floor-5 shape is the original author's house idiom for this table, not an artefact of our search.

- [s10] REDERIVE, SIBLING CAVEAT: func_80037AA4 closed its own final RA residual with a Judge-sanctioned (2026-07-28) /* FAKE */ constant-holder local (s32 sh = 0xD) that reload's update_equiv_regs deletes at zero byte cost. Same file, same table, same pass - but the precedent does NOT transfer to func_80037B00, because no constant-holder can move either of this function's two pinned quantities (refs(78) and live(78)).

- [s10] REDERIVE, KENGO: kengo_matches.csv rates func_80037B00 against ki_every_little_thing (src/ishito/is_ki_control.c) as size-only-ambiguous - 36 vs 36 instructions with similarity scores 0.00/0.00/0.00/0.42. Same for both siblings. No Kengo transplant exists; consistent with the slog-kengo-dead-end memory. Do not re-run this axis.

- [s10] global.c:640-655 (allocno_compare) re-read from source: priority = (int)((double)(floor_log2(refs)*refs)/live_length * 10000 * allocno_size), ties broken toward the LOWER allocno. allocno_size is 1 for every SImode allocno here, so it is an inert factor. Seats fall out in rank order ($3,$5,$6,$7,$8,$9,$10,$11) for every allocno without a hard-register copy preference; only 72 -> $4 and 82/84/87 -> $2 have one (copies of a0/v0), so there is no preference lever on 73 or 78.

- [s10] pri(78) = 8888 is INVARIANT on any chassis emitting target's 36 instructions. refs(78)=4 because $t0 occurs in exactly two target instructions; live(78)=9 because 78's pre-combine span (s9/flow.rtl insns 46..82) is exactly the inner loop's ten mandated insns (2 QI loads + 2 zero_extends + 2 branches + 2 increments + slt + def).

- [s10] Therefore the ONLY free variable left is live_length(73). With refs(73) pinned at 8 by the stream (init 1 + guard 1 + `t1 += 1` 2x2 + while-test 1x2), closing the function requires live(73) >= 27: 27 gives exactly 8888 (a tie, won by the end pointer once decl_order_swap_vC.c makes it the lower allocno), 28..31 wins outright, 32 drops below the flag allocno 74 at 7500. live(73) is 23 today, so the ask is +4 flow-visible insns that emit no bytes.

- [s10] Every other cell for the counter is arithmetically unreachable: refs 7 needs live 16..18 (a SHORTER range, and the only removable weight-1 occurrence is the guard compare, whose removal costs the single-blez merge the floor-5 form depends on); refs 6 needs live 14..15; refs 9 needs live 31..35.

- [s10] REGION MAP (measured): preheader insns count toward the counter and not the end pointer, at zero byte cost when combine folds them (+1 measured); block_5c insns count but cost a byte (combine cannot fold across the block edge); block_74 insns do NOT count for the counter at all (flow.c:1685 only increments reg_live_length for registers in regs_sometimes_live, and 73 never changes state inside block_74) even though they DO count for allocno 75 (9 -> 13 refs); inner-loop-arm insns count for BOTH ranges and are net negative.

- [s10] The +1 preheader mechanism does not stack: a four-link constant chain on a fresh pointer temp is constant-folded by cse1 before flow and its dead links are deleted by flow, buying +1 and leaving a 5-insn residue pseudo (counter 8 refs/24 live, pri 10000). Even the F1 combine-foldable chain-extender family caps out at +1 on this function.

- [s10] TOOL: tools/gcc-2.7.2/cc1 IS the instrumented build carrying the BB2_FLOW_DEBUG hook at flow.c:1685 (prints `FLOWDBG reg=N insn=U bnum=B`), but engine/buildconfig.py builds with tools/gcc-2.7.2/build/cc1, which is NOT instrumented, and the two disagree on pseudo numbering for this TU (only 6 FLOWDBG lines for reg 78 across the whole file, none of them func_80037B00's). The hook cannot audit this function's live ranges until the instrumented cc1 is rebuilt from current source; use the .lreg banner from tmp/grind/func_80037B00/s10/probe.sh, which is calibrated against the s8/s9 tables.

- [s10] src/code6cac_c.c was reverted to HEAD (INCLUDE_ASM) at session end; no tracked build file is modified. candidate.c is unchanged (still the floor-5 form) because nothing this session lowered the floor.

## s11 (rederive, 2026-08-26) — THE GUARD IS THE PIVOT; s7-s10's "pinned quantities" model is wrong

Chassis re-measured at session start: `sandbox func_80037B00 --disable all` with candidate.c applied
= **score 5, target_insns 36, build_insns 36, rules_dropped 0**. Floor unchanged.

### The three-line summary
1. The phantom 8-byte frame does NOT come from the `while` outer loop (s8's attribution). It comes
   from the SOURCE GUARD `if (var_t1 < D_800A38C8)`: expand emits `(set (reg 83) (lt (reg 73) (reg X)))`
   + `(if_then_else (eq (reg 83) 0))`, combine folds the pair into a bare `blez` and DELETES reg 83,
   which then reaches local-alloc carrying flow's stale 2 refs / 2 insns in class ST_REGS, takes no
   hard register, and gets a 4-byte slot from reload's `alter_reg` (rounded to 8). Proof:
   `alt_base_v6_dowhile.c` — candidate.c with `while` rewritten as `do { } while` — is CODEGEN-IDENTICAL
   (26 cc1 insns, both sp adjusts, identical allocno order and identical seats).
2. refs(allocno 73) = 8 is NOT pinned by the target stream. Exactly one of the eight references is the
   guard naming the counter. Every counter-free guard measures refs(73) = 7.
3. refs 7 crosses a floor_log2 boundary (3 -> 2), collapsing pri(73) from 24/L73 to 14/L73, and that
   **SOLVES the $t0/$t1 transposition** that s7, s8, s9 and s10 all reported as the whole residual:
   the end pointer (allocno 78) is now allocated FIRST and takes $t0, exactly as target.

### The measured matrix (all via tmp/grind/func_80037B00/s11/probe.sh; cc1 = tools/gcc-2.7.2/cc1,
### codegen-identical to build/cc1 on this TU, both used and cross-checked this session)
| variant | guard | outer loop | refs(73) | L73 | L74 | frame | cc1 insns | alloc order fragment |
|---|---|---|---|---|---|---|---|---|
| candidate.c | `var_t1 < D_800A38C8` | while | 8 | 23 | 16 | YES | 26 | 75 **73** 83 **78** 74 |
| v6 | `var_t1 < D_800A38C8` | do-while | 8 | 23 | 16 | YES | 26 | 75 **73** 83 **78** 74 |
| v7 (func_80037AA4 idiom, bound read before guard) | `var_t1 < var_t3` | do-while | 8 | 22 | 16 | YES | 25 | 75 **73** 82 **78** 74 |
| v1 | `D_800A38C8 > 0` | while | 8 | 25 | 16 | no | 27 (two blez) | 75 **73** **78** 74 |
| v3 | `D_800A38C8 > 0` | do-while | 7 | 22 | 16 | no | 24 | 75 **78** 74 82 **73** |
| v5 (= v3, counter init last in preheader) | `D_800A38C8 > 0` | do-while | 7 | 20 | 16 | no | 24 | 75 82 **78** 74 **73** |
| v8 (= v5 + `var_a3 += 0x28` before the flag test) | `D_800A38C8 > 0` | do-while | 7 | 20 | 17 | no | 25 | 75 82 **78** 74 **73** |
| v10 | `var_t3 > 0` (bound read first) | do-while | 7 | 22 | 16 | no | 23 | 75 **78** 74 **73** |
| v13 / v14 | `>= 1` / `< 1 return 0` | do-while | 7 | 20 | 16 | no | 24 | 75 82 **78** 74 **73** |
| v11 / v15 / v16 / v17 / v18 | counter-free variants | do-while | 7 | 20 | 15-17 | no | 24-25 | **78** before **73** |
| v19 | `var_t1 < D_800A38C8` + foldable `+0x14/+0x14` at block_74 | do-while | 8 | 23 | 16 | YES | 26 | unchanged |

`v5` scores 8 on the sandbox (build_insns 35 vs target 36) — worse than candidate's 5, because the
frame pair is gone; it is nonetheless the first form in this function's history whose END POINTER
lands in $t0.

### THE RESIDUAL, RESTATED (this replaces the s9/s10 statement of it)
The guard is doing two incompatible jobs.
- **Name the counter** -> the compare pseudo orphans -> the 8-byte phantom frame -> the 26-insn /
  36-instruction stream is exact -> but refs(73) = 8 -> pri(73) = 24/L73 -> to sit below
  pri(78) = 8/9 and above pri(74) = 12/16 needs **L73 in [28,31]** (L73 is 22-23; this is s10's wall).
- **Do not name the counter** -> refs(73) = 7 -> pri(73) = 14/L73 -> **78 is seated before 73 and takes
  $t0, the transposition is solved** -> but no orphan, no frame, cc1 emits 24 insns instead of 26.

Four counter-free guard spellings were measured for an orphan and all four failed: `D_800A38C8 > 0`,
`var_t3 > 0` (bound read into a local first), `D_800A38C8 >= 1`, `D_800A38C8 < 1` as an early return.
GCC canonicalises a bound-vs-constant comparison at expand time into MIPS `branch_zero`/`bgtz` and
never materialises a compare pseudo; `< 1` is folded to `<= 0` before expand, so the hoped-for
`slti + bne -> blez` combine never runs. The only natural zero-valued REGISTER in this function is the
counter itself.

### The secondary (and much smaller) defect in the refs-7 family
With 78 seated correctly, the next pair out of order is counter (73) vs match-flag (74): target wants
$t1 then $t2, so 73 must precede 74. Both have floor_log2 2, so the condition is exactly
**7 * L74 >= 6 * L73** (an exact tie is won by 73 via global.c:655's lower-allocno rule).
Measured: v5 (20,16) = 112 vs 120; **v8 (20,17) = 119 vs 120 — one unit short**; v15 (20,15) worse.
v8 buys its extra L74 by moving `var_a3 += 0x28` ahead of the flag test, which costs +1 emitted
instruction (the bottom `bnez` loses its delay-slot filler), so v8 is not a usable end state as such.
L73 = 20 is rigid: the only three insns where 73 is live and 74 is not are the counter's definition
and the bottom `slt`/`bnez`, all mandated by the stream. Reachable cells: (20,18), (19,17), (21,18).

### Method notes for the next session
- `tmp/grind/func_80037B00/s11/probe.sh <variant.c>` — applies a variant and prints the .lreg register
  table, the `;; N regs to allocate` order, the greg seat map, the cc1 asm and its insn count. ~20 s.
- `tmp/grind/func_80037B00/s11/flowdbg.sh <variant.c> <regno>` — same but under the instrumented
  `tools/gcc-2.7.2/cc1` with `BB2_FLOW_DEBUG=<regno>`, printing one `FLOWDBG reg= insn= bnum=` line per
  `reg_live_length` increment (flow.c:1685). Caveat: life_analysis runs more than once and the pseudo
  numbering differs between runs, so the raw trace needs correlating with `x.flow` before it is
  quotable — the .lreg totals are the reliable reading.
- The priority formula that reproduces every measured order in the table above:
  `pri = floor_log2(refs) * refs / live_length`, ties broken by lower allocno number.

- [s11] Chassis re-measured at session start: candidate.c applied to src/code6cac_c.c gives `sandbox func_80037B00 --disable all` = score 5, target_insns 36, build_insns 36, rules_dropped 0, cheat_asm_stripped 3. Floor unchanged at 5.

- [s11] The phantom 8-byte frame is produced by the ENTRY GUARD, not by the `while` outer loop: alt_base_v6_dowhile.c (candidate.c with `do { } while` instead of `while`) is codegen-identical - same 26 insns, same sp adjusts, same allocno order, same seats. This retires s8's loop.c/duplicate_loop_exit_test attribution.

- [s11] The orphan that pays the frame is `(set (reg:SI 83) (lt:SI (reg/v:SI 73) (reg X)))` at insn 16 of x.flow, gone from x.combine, surviving into .lreg as `Register 83 used 2 times across 2 insns in block 0; ST_REGS or none` and taking no hard register.

- [s11] refs(allocno 73) is 8 with a counter-naming guard and 7 with any counter-free guard - measured across eight spellings. It is NOT pinned by the target stream, contrary to s10's central claim.

- [s11] refs(73)=7 crosses the floor_log2 3->2 boundary and drops pri(73) from 24/L73 to 14/L73, which seats allocno 78 (the inner-loop end pointer) BEFORE the counter. Every refs-7 variant measured puts `78 in 8`, i.e. the end pointer in $t0 exactly as target - the transposition that s7, s8, s9 and s10 each reported as the whole residual is solved.

- [s11] A `while` outer loop forces refs(73)=8 regardless of the source guard, because loop.c's duplicated exit test names the counter (v1: while + `D_800A38C8 > 0` = refs 8, 27 insns, two blez, no merge).

- [s11] Four counter-free guard spellings (`> 0`, `var_t3 > 0`, `>= 1`, `< 1` early-return) all lose the orphan and emit 23-24 cc1 insns instead of 26: GCC canonicalises bound-vs-constant guards into MIPS branch_zero/bgtz at expand time and folds `< 1` to `<= 0` before expand.

- [s11] The refs-7 family's remaining defect is counter-vs-flag order, governed exactly by 7*L74 >= 6*L73 with ties won by allocno 73. Best measured is v8 at (L73,L74) = (20,17) -> 119 vs 120, one unit short, and v8 costs +1 emitted instruction.

- [s11] func_80037AA4's idiom transplanted verbatim (bound read into a local before the guard, v7) measures 25 cc1 insns with the frame present and refs 8 / L73 22 - one instruction short of target because the lw loads straight into the bound register and the `addu $t3,$v0,$zero` preheader copy never appears.

- [s11] v5 (the cleanest refs-7 form) scores 8 on the sandbox with build_insns 35 vs target 36 - worse than candidate's 5, so candidate.c is unchanged as the best form; v5 is banked as rejected/dowhile-refs7-guard-loses-phantom-frame.c because of the frame, not because of the register order.

- [s11] Reusable instruments left in place: tmp/grind/func_80037B00/s11/probe.sh (variant -> .lreg table + allocation order + seat map + cc1 asm and insn count, ~20 s) and tmp/grind/func_80037B00/s11/flowdbg.sh (same under the instrumented tools/gcc-2.7.2/cc1 with BB2_FLOW_DEBUG=<regno>, one line per flow.c:1685 live_length increment).

- [s11] The priority model that reproduces every measured allocation order in this function: pri = floor_log2(refs) * refs / live_length, ties broken by lower allocno number (global.c:655).

## s12 (structural) - facts banked
1. Chassis: candidate.c = score 5, 36/36 insns (re-measured this session). v5 (refs-7) = score 8,
   35 insns; its only defects are the two missing sp adjusts and the counter/flag seats.
2. THE UNIFIED TARGET: both chassis reduce to `live_length(allocno 73, the counter) >= 28`.
   refs-8 (frame present) needs L73 in [28,32] with the added insns OUTSIDE the flag's live range;
   refs-7 (no frame) needs L73 >= 7*(L73-L74), which at the best measured gap (4, form v5) is again
   L73 >= 28, with the added insns INSIDE the flag's live range. Current values: 23 (refs-8) / 20
   (refs-7).
3. The phantom 8-byte frame is paid by the guard's compare pseudo ONLY when the compared register is
   live past the guard. Measured at RTL level: combine re-sites the counter's `(set 73 0)` into the
   compare's insn slot and rewrites the branch to `(le (reg B) 0)`, stranding the compare pseudo with
   flow's stale 2 refs, class `ST_REGS or none`, no hard reg, `alter_reg` slot, `vars= 8`. With a
   fresh local (wC) or the match flag borrowed as the zero (wA) combine deletes both insns and
   `vars= 0`. The counter is the only register in this function's logic that qualifies.
4. Extending the counter's liveness by giving it a use (return arm / epilogue) is self-defeating:
   refs 8 -> 9 moves the required window to L73 in [31,36] while the extension supplies at most 28.
5. The s10 foldable-pair +1 is non-stackable across DISTINCT variables too (cse1 folds `+1/-1` on a
   scalar before flow). At the loop top the pair does give +1 to L73 without touching L74, but its
   two in-loop occurrences add weighted refs to whatever variable carries it, which re-seats that
   variable at the head of the allocation order.
6. func_80037AA4 (same file, same table, MATCHED) carries the same phantom 8-byte frame and uses the
   counter-naming guard - candidate.c's chassis is the author's idiom. AA4's own RA residual was
   closed with a Judge-sanctioned `/* FAKE */` constant-holder (2026-07-28); that specific lever does
   not transfer here (it moves none of L73 / refs(78) / L78).
7. Artifacts: tmp/grind/func_80037B00/s12/probe.sh (RA table + greg order + `.frame` + cc1 asm for
   one variant), d_v5_init_inside_last/, d_wA_flag_reuse_guard/, d_wC_invented_zero_guard_PROBE/,
   d_wD_init_first_bound_guard/, d_wE_counter_live_to_end_PROBE/, d_wF_looptop_foldable_pair_PROBE/,
   d_wG_preheader_two_pairs_PROBE/.


- [s12] Chassis re-measured this session: candidate.c `sandbox --disable all` = score 5, target_insns 36, build_insns 36, rules_dropped 0.

- [s12] v5 (s11's refs-7 form) = score 8, build_insns 35. Its cc1 asm is target's stream position-for-position apart from the two missing sp adjusts, with end pointer -> $8 ($t0) exactly as target wants; counter -> $10 and flag -> $9 where target has $9 and $10.

- [s12] UNIFIED TARGET: both chassis reduce to live_length(allocno 73, the loop counter) >= 28. Current values 23 (refs-8, frame present) and 20 (refs-7, correct end-pointer seat). refs-8 needs the added insns outside the flag's live range (preheader or loop top ahead of `var_t2 = 0`); refs-7 needs them inside it.

- [s12] The frame is paid only by a guard whose compared register is LIVE PAST THE GUARD: combine then re-sites the counter's `(set 73 0)` into the compare's insn slot and rewrites the branch to `(le (reg B) 0)`, stranding the compare pseudo with flow's stale 2 refs in class `ST_REGS or none` for alter_reg. A fresh local or the re-initialised match flag makes combine delete both insns and the frame disappears (`vars= 0`).

- [s12] Any use-based extension of the counter's liveness is self-defeating: refs 8 -> 9 moves the required window to L73 in [31,36] while the return-arm + epilogue extension supplies at most 28 (measured: `return (var_t1 != 0)` gives refs 10 / L73 24 / pri 12500).

- [s12] The s10 foldable-pair +1 does not stack across distinct variables (cse1 folds a scalar `+1 / -1` pair before flow) and, at the loop top, costs the carrying variable +8 weighted refs, which re-seats it at the head of the allocation order (refs(75) 9 -> 17, 75 -> $3).

- [s12] func_80037AA4 (same file, same table, MATCHED) carries the identical phantom 8-byte frame and the counter-naming guard, so candidate.c's chassis is the original author's idiom, not a search artefact.

- [s12] Ledger updated: memory/grind/func_80037B00/hypotheses.md (5 new entries) and evidence.md (s12 section, 7 facts). Six forms banked under memory/grind/func_80037B00/rejected/ (flag-reuse-as-guard-zero-refs74-8-no-frame.c, invented-zero-local-guard-no-orphan-no-frame.c, refs7-init-before-guard-gap7.c, counter-live-to-end-costs-refs9-10.c, looptop-foldable-pair-plus1-refs75-blowup.c, two-preheader-pairs-distinct-vars-still-plus1.c). src/code6cac_c.c restored to its committed INCLUDE_ASM state; candidate.c (score 5) unchanged as the best form.

## s13 (2026-08-26) - structural - FUNCTION MATCHED, score 5 -> 0

- Chassis re-measured at session start: candidate.c (s8 form) = `sandbox --disable all` score 5,
  target_insns 36, build_insns 36; .lreg `73 used 8 times across 23 insns`,
  `78 used 4 times across 9 insns`; .greg `;; 11 regs to allocate: 79 76 77 75 73 83 78 74 82 81 72`,
  seats `73 in 8  78 in 9`; `.frame $sp,8,$31 # vars= 8`. Identical to the ledger's record, so
  every s8..s12 conclusion was chassis-valid.
- THE CLOSE: the inner byte-compare loop was re-spelled from a `goto loop_inner` back-edge to
  `while (1) { ... break; ... }`. Nothing else in the function changed.
- flow.c weights a register reference by LOOP NESTING DEPTH (`REG_N_REFS += loop_depth`), and
  loop_depth only advances across NOTE_INSN_LOOP_BEG/END, which a goto back-edge never emits.
  With the goto spelling the whole outer body sat at depth 2, so the end pointer's def + use were
  charged 2 + 2 = 4. With loop notes the use sits at depth 3: refs(78) = 5, live_length(78)
  unchanged at 9.
- global.c `pri = floor_log2(refs)*refs/live_length`: pri(78) 2*4/9 = 0.888 -> 2*5/9 = 1.111,
  which now sits between pri(73) = 3*8/23 = 1.0435 and pri(75) = 1.350. Allocation order becomes
  `79 76 77 75 78 73 83 74 82 81 72`, seats `78 in 8` ($t0), `73 in 9` ($t1), `74 in 10` ($t2) -
  target's register assignment exactly.
- Everything s12 measured as pinned STAYED pinned: refs(73)=8, L73=23, refs(74)=6, L74=16,
  refs(75)=9, L75=20, the phantom 8-byte frame (`vars= 8`), and the instruction count.
- s12's unified rule "both chassis reduce to live_length(73) >= 28" was arithmetically correct but
  rested on refs(78) = 4 being fixed. refs(78) had only ever been MEASURED, never VARIED. The
  fourth input to the priority formula - the loop depth of each reference, i.e. the source
  spelling of the enclosing loops - was the free variable nobody had touched.
- VERIFICATION: `sandbox func_80037B00 --disable all` -> score 0, 36 vs 36, rules_dropped 0,
  zero regfix/asmfix rules for this function. `verify-oracle` -> `"ok": true`,
  `"build_matches": true`, build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == the locked
  oracle. The body is pure C with no inline asm, no volatile, no FAKE construct and no
  sanctioned-family exception claimed (self_vet.md).
- The `while (1)` + `break` spelling is also the more conventional reading of the bounded strncmp
  the function performs, so this is a fidelity improvement as well as a match.
- Superseded frontier entries: s12 frontier #1 (a combine-absorbed loop-top insn to lift L73 to
  28) and #2 (gap <= 3 on the refs-7 chassis) are both MOOT - neither was needed, and the refs-7
  chassis is abandoned. s12 frontier #3 (file a fidelity/routing packet on the one-register
  transposition) must NOT be filed: the transposition is gone.
- Artifacts: tmp/grind/func_80037B00/s13/probe.sh, base.c, v1_inner_while1.c, d_base/,
  d_v1_inner_while1/ (x.s, x.lreg, x.greg, x.flow, x.combine and the full -da series).
