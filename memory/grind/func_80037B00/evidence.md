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
