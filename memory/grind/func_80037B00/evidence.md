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
