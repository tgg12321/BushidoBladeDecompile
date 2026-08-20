# Evidence bank — func_800481E8

## s1 (recon, 2026-07-28) — floor 14 → 10, inline-move cheat dissolved

- canonical: verdict C, 56 target insns. sandbox --disable all: 14 at session
  start (12 rules dropped, cheat-asm stripped), **10 after s1 edits** (in src/).
- Frame equation (target): 72 = ALIGN8(vars=32) + args 24 (5th call arg at
  0x10) + regs 16 (s0/s1/s2/ra at 56/60/64/68). Our build: 40 with vars=0 —
  IDENTICAL otherwise. All 10 residual diffs are frame-offset instructions
  (2× addiu sp, 4× sw, 4× lw). Loop body, prologue staging, epilogue order,
  register allocation: all byte-match already.
- The prior session's "forbidden dead-frame floor / canonical-asm endpoint"
  conclusion is DISSOLVED: it predates [[phantom-frame-slots-gcc272]]
  (2026-07-13), which proves vars>0 with zero dead decls is reachable
  (witness tslLineG5Init, COMPLETED-C, vars=8 untouched). The 32 bytes need a
  live phantom-slot spelling, not a pad array and not canonical-asm.
- Sibling frame witnesses are NOT clean: AddTbpOfst_80047EE8 (parked) gets
  vars=32 via forbidden `s32 unused_slack[8]`; InitHiraRmd_80047FBC via
  declared `s32 buf[8]` + (void)buf. No legitimate vars=32 witness in cluster.
- InitHiraRmd_80047FBC's prologue technique (its in-file comment, levers 1+2)
  transfers verbatim: base-copy staging + function-scope `a0_for_call`
  precompute + FAKE `arg0 = 0;` produce target's `$s0=$a0; $s2=$s0` chain and
  `addu $s0,$s2,$v0` second-pointer binding. Measured both directions
  (with/without the dead store — see rejected/).
- Fresh find_duplicates.py run (s1): NO lead for func_800481E8 (stale 0.625
  lead to func_800483DC gone; that shape lacks the conditional increment).
- cc1 `.frame` probe harness: tmp/grind/func_800481E8/s1/frame_probe.sh
  (arg = .c path; prints per-func vars=; direct frame gradient per
  [[phantom-frame-slots-gcc272]]).

- WIP rejected_form: {'form': 's32 pad[8] / register pins + frame-size regfix', 'score': 0, 'reason': 'FORBIDDEN dead-frame coercion per dead-vars-local-array; engine refuses completion.'}

- == imported from memory/wip notes.md ==
# func_800481E8 (text1b.c) — BLOCKED: forbidden dead-frame floor (canonical-asm)

## TL;DR
Target frame is **72 bytes** (`addiu sp,sp,-72`, saves s0/s1/s2/ra at 56/60/64/
68); GCC produces a **40-byte** frame. The ~32 extra bytes have **no recoverable
semantic local** — the function uses only registers + one stacked 5th call arg to
efc_buki_draw_zanzou. The 12 regfix rules patch the frame size (40->72) + all 8
register save/restore offsets (24->56, 28->60, 32->64, 36->68 and the lw mirror)
+ a prologue reorder + an INLINE_MOVE_ALIASING `addu $16,$18,$5 -> addu $16,$16,$5`.
HEAD also carries a `register asm("$16")` pin + `__asm__("move %0,%1")`
INLINE_MOVE_ALIASING in the body. Honest distance 14.

## Why it's not pure-C (FORBIDDEN class)
Per [[dead-vars-local-array]] (FORBIDDEN 2026-05-31): matching the 72-byte frame
requires either `s32 pad[8];` (forbidden dead local array — the detector refuses
completion) or the register pins / frame-size regfix (cheats). There is NO local
with clear semantic meaning that accounts for the 32 dead bytes. The rule's
explicit guidance for this exact situation: park / request canonical-asm
authorization; do NOT add a pad array.

Sibling precedent: `AddTbpOfst_80047EE8` (same file) was PARKED 2026-06-07 for
the identical "dead-vars-local-array frame floor" (commit 0244c4dd).

## Endpoint
Canonical-asm authorization (or a genuine identification of what the 32 frame
bytes semantically held in the original — none found; the call-loop uses no stack
buffer). Blocked. This is a user/orchestrator decision, not worker pure-C work.


- [s1] canonical: verdict C, distance 14 at start, 56 target insns

- [s1] Target frame equation: 72 = ALIGN8(vars=32) + args 24 + regs 16 (s0/s1/s2/ra @ 56/60/64/68); our build 40 with vars=0

- [s1] After s1 edits (applied in src/text1b.c): sandbox --disable all = 10, build_insns 56 == target 56; instruction-text diff shows exactly 2x addiu sp + 4x sw + 4x lw offset diffs and nothing else

- [s1] Prior WIP conclusion 'forbidden dead-frame floor / canonical-asm endpoint' is dissolved by phantom-frame-slots-gcc272 (post-dates it): vars>0 with zero dead decls is reachable pure-C (witness tslLineG5Init, COMPLETED-C, vars=8 untouched)

- [s1] No legitimate vars=32 witness in the sibling cluster: AddTbpOfst_80047EE8 (parked) and InitHiraRmd_80047FBC both get vars=32 via forbidden unused/dead arrays

- [s1] Fresh find_duplicates.py run: no near-duplicate lead for func_800481E8 (stale 0.625 lead to func_800483DC did not survive re-scan)

- [s1] Reusable frame gradient harness banked: tmp/grind/func_800481E8/s1/frame_probe.sh <file.c> prints per-function cc1 .frame vars=

## s2 (structural, 2026-07-28) — floor unchanged at 10; the phantom-slot axis is MEASURED DEAD for this function

- [s2] Baseline re-verified with candidate.c applied to src/text1b.c: sandbox
  --disable all = 10, build 56/56 insns (rules_dropped 12, cheat_asm_stripped 323).
- [s2] **Frontier item 1 SUPERSEDED, not run as written.** The phantom slot is NOT
  assign_stack_temp during expansion — reproduced the tslLineG5Init trigger
  standalone (vars=8) and the lreg dump shows the orphan pseudo verbatim
  ("Register 77 used 2 times across 2 insns in block 0; dies in 0 places;
  ST_REGS or none") = the reload alter_reg stale-ref orphan already nailed by
  func_80037540 H14 (gdb-confirmed there). No expansion-site map is needed;
  the map's question is answered by the orphan's entry condition instead.
- [s2] **ENTRY CONDITION, BISECTED** (tmp/grind/func_800481E8/s2/minrepro.py +
  bisect2.py; 14 cases). The orphan fires iff combine DELETES an emitted 2-insn
  HImode→SImode sign-extension chain (ashift16/ashiftrt16 with an intermediate
  pseudo) AFTER flow counted refs. Triggers measured (all vars=8, orphan in
  lreg): T0 faithful tslLineG5Init; **T3 = same trigger with s16 locals loaded
  from u16 MEM via cast — OUR load shape, so lh loads are NOT required**;
  B2 wide bitwise + raw `&1` second use; **B3 = ONE variable, `if (v1 < 640)` +
  `if (v1 & 1)` — minimal known trigger, and the compare is literally our
  a3v < 0x280 shape**; B5 trigger coexisting with a LIVE emitted sll/sra of the
  same value; B7 HImode ADD low-bit consumer (bitwise NOT required); B8 `== 1`
  consumer. Non-triggers (vars=0): single consumer (T1); u16/zero-extension
  everywhere (T2, B6 — zero-ext is 1-insn andi or free, no intermediate pseudo
  exists to orphan); constant-&-only (T4); two extended-compare uses (T5);
  extended second use (B1); one ext use + raw use (B4 — clean lh conversion).
- [s2] **THE KILL: every deletion route is VISIBLE in the emitted bytes.**
  Either (a) the sll/sra chain is NOT emitted (high-bits-dead consumer), or
  (b) the lhu converts to lh, or (c) consumer insns (andi/branch/nor) appear
  that the target lacks. func_800481E8's target contains all 4 lhu loads, all
  4 sll/sra pairs LIVE (consumers: 3 call args, slti 0x280, sw of the 5th
  arg — every one needs correct high bits), and NO low-bit-only use of any
  halfword value. Therefore no honest spelling of THIS function's semantics
  can host a combine-deleted extension → no orphan → vars pinned at 0.
- [s2] Hand grid over the candidate chassis (probe.py, 11 variants): every
  s32-holder / redundant-(s16)-re-ext / staged-HI-view / zero-ext-recast
  spelling of the stream values gives vars=0; the holder spellings additionally
  convert lhu→lh (byte-break). Banked rejected/s32-holder-reext-no-orphan-lh-conversion.c.
  V5 (tree-fold) and V11 ((s16)arg1*4 prologue) are byte-identical controls at
  vars=0; V8 (`*p & ~3u` round) emits li/and — byte-break, dead.
- [s2] **BYTE-IDENTICAL ALTERNATIVE SPELLINGS BANKED** (equivalent-spelling
  class, cc1-output-identical to candidate): V6 `s32 a1w = *(u16*)p;` with call
  arg `(s32)(s16)a1w`; V7 `u32 a1u = *(u16*)p;` with arg `(s32)(s16)(u16)a1u`.
  Useful chassis variety for any future ruling-driven work.
- [s2] **QUANTITATIVE SWEEP — 52,043-iteration instrumented permuter campaign**
  (workspace tmp/grind/func_800481E8/s2/perm; compile.sh logs cc1 vars= for
  EVERY candidate and captures every vars>0 source; perm_pad_var_decl zeroed;
  honest --stack-diffs scorer, base 266). vars distribution over 42,374 logged
  compiles: 33,415×0, 8,718×8, 219×16, 12×24, **6×32**. All 8,644 captured
  vars>0 sources classified (classify_hits.py): 7,127 volatile-coercion, 1,457
  address-escape, **60 "clean" — ALL at vars=8 ONLY, all ≥33 non-sp body diffs,
  all semantically broken permuter mutations** (char locals, constant-folded
  strides, same-address double reads). ALL 6 vars=32 forms are volatile cheats
  (4-5 volatile decls each). ZERO clean forms reach vars∈[25,32]; ZERO clean
  forms near byte-identity. Only honest-score find: `volatile int new_var;`
  dead-pad (score 202) = the known cheat family, banked
  rejected/volatile-dead-pad-permuter-find.c. Campaign harvested + STOPPED
  (9 procs killed, 0 left).
- [s2] **FIVE-WAY FRAME TAXONOMY CLOSED for func_800481E8** (mirrors
  func_80037540 H21): frame 0x48 needs vars∈[25,32]; routes: (i) written
  aggregate → adds stores the 56-insn byte-matched target lacks; (ii) volatile
  → cheat, score-inert; (iii) address-escape → cheat + extra insns; (iv) stale-
  ref phantom → measured dead THIS session (no deletable extension exists);
  (v) unwritten tail → forbidden family (no dead stores in target, so the
  2026-07-01 carve-out does not apply). The frame axis has no sanctioned route
  left; this matches the pending file_LoadSectors owner question (same family,
  parked owner-gated 2026-07-27) and the endgame-lock-disposition standing
  policy surface.
- [s2] PROJECT-WIDE REUSABLE FINDING: B3 is the smallest known phantom trigger
  — a target containing a signed halfword compare PLUS a low-bit test of the
  same halfword value can host phantom frame slots in pure C. Relevant to the
  in-binary census (28 functions with untouched frame slack, func_80037540 s5).
- [s2] Instrument caveat: /tmp is unreliable in fresh WSL sessions here
  ("Failed to start the systemd user session" runs) — compile hooks must use
  repo-local temp files (mkperm.sh was patched accordingly).

- [s2] Baseline re-verified with candidate.c applied to src/text1b.c: sandbox --disable all = 10, 56/56 insns, rules_dropped 12, cheat_asm_stripped 323

- [s2] Five-way frame taxonomy CLOSED for func_800481E8: (i) written aggregate adds stores the byte-matched target lacks; (ii) volatile = cheat (score-inert, sandbox strips it); (iii) address-escape = cheat + extra insns; (iv) stale-ref phantom measured dead this session; (v) unwritten tail forbidden (target has no dead stores, 2026-07-01 carve-out inapplicable)

- [s2] Campaign telemetry: base 266 (honest stack-diffs scorer), 52,043 iters, 1 find (volatile dead-pad, 202, 6.7min), harvested + STOPPED, 9 procs killed, 0 orphaned

- [s2] vars distribution over 42,374 logged compiles: 33,415x0 / 8,718x8 / 219x16 / 12x24 / 6x32; every vars=32 form is the volatile-coercion family

- [s2] Byte-identical alternative spellings banked for the stream reads (V6: s32 raw holder + (s32)(s16) at call; V7: u32 holder + (s32)(s16)(u16)): equivalent-spelling class for future ruling-driven work

- [s2] Project-wide reusable: B3 is the smallest known phantom trigger (signed halfword compare + low-bit test of same value) — applicable to the 28-function untouched-frame-slack census

- [s2] Instrument caveat: /tmp unreliable in fresh WSL sessions (systemd user session failure); compile hooks must use repo-local temps

- [s2] Same owner-question family as file_LoadSectors (parked owner-gated 2026-07-27) and the endgame-lock-disposition standing policy (2026-07-20)

- [s3] s3 orient: candidate.c at floor 10 (pure C, 0 pins, 0 rules, 56/56 insns, residual = frame delta only); HEAD carries the byte-correct cheat form (12 rules + $16 pin + INLINE_MOVE_ALIASING) at honest floor 14.

- [s3] s3 gate-1 (canonical-asm): FAILS. Target is verdict C. 56/56 insns match except 10 frame-offset instructions (2x addiu sp + 4x sw + 4x lw). No S1/S2/S6 STRONG scan_hand_coded signals — no redundant-mask-before-discarding-shift, no hand-scheduled cop2 pipeline, no swc2 with hardcoded reg.

- [s3] s3 gate-2 (coercion/spelling family): FAILS. 5-way frame taxonomy CLOSED by s2. No SOTN-master precedent for unwritten-tail phantom-frame family. Same family question terminated on file_LoadSectors option a (2026-07-28, docs/grind/decisions.md line ~1992): 'An unwritten phantom-frame carrier has no SOTN precedent distinguishing it from frame coercion.' Partition arguments explicitly ruled insufficient.

- [s3] s3 filed entry: docs/grind/decisions.md '2026-07-28 — func_800481E8 (src/text1b.c) — OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE' (both-gates-fail terminal disposition; keeps HEAD's byte-correct cheat form; banks candidate.c as best clean form at floor 10).

- [s3] Reusable project-wide byproduct (from s2, not this function's work): B3 minimal phantom trigger characterized — signed halfword compare + low-bit test of the same halfword value; applicable to the 28-function untouched-frame-slack census.

## s4 (permuter, 2026-08-20) -- CHASSIS RE-VERIFIED on the post-migration tree; the permuter modality is MEASURED DEAD

- [s4] CHASSIS CHANGE, IDENTIFIED AND RE-MEASURED. HEAD no longer carries the
  byte-correct cheat form the s3 escalation entry described. The 2026-08-19
  asm-until-matched migration replaced it with `INCLUDE_ASM("asm/funcs",
  func_800481E8);` at src/text1b.c:68; regfix.txt keeps only a COMMENT line
  (regfix.txt:2442) and zero rules; asmfix.txt has zero. Additionally a naming
  wave renamed the callee: candidate.c's `efc_buki_draw_zanzou` is now
  `func_800482C8` (the jal target in asm/funcs/func_800481E8.s:48). candidate.c
  has been updated in place for both facts.
- [s4] BANKED FLOOR RE-MEASURED, UNCHANGED. Applying the (renamed) candidate.c
  over the INCLUDE_ASM line: `sandbox func_800481E8 --disable all` = **10**,
  target_insns 56 / build_insns 56, rules_dropped 0, cheat_asm_stripped 278.
  The migration is codegen-neutral for this function -- every s1/s2/s3 spelling
  conclusion remains valid on the new chassis, and the residual is still exactly
  the 10 frame-offset instructions (2x addiu sp, 4x sw, 4x lw at the s0/s1/s2/ra
  save slots: 24/28/32/36 ours vs 56/60/64/68 target; frame 40 vs 72).
  NOTE the callee is UNPROTOTYPED at the call site (func_800482C8 is defined
  BELOW func_800481E8 in src/text1b.c and there is no forward declaration), so
  args are int-promoted -- the same ABI under which the floor was originally
  measured. Do not "fix" this by adding a prototype without re-measuring.
- [s4] FRESH-SEED PERMUTER CAMPAIGN ON A STRUCTURALLY DIFFERENT CHASSIS -- the
  distinguishing work of this session vs s2. s2's 52k campaign seeded from the
  candidate chassis (s16 locals for the stream halfwords). s4 seeded from the
  banked byte-identical **V6 spelling** (s32 raw holders `a1w/a2w/a3w/v0w` loaded
  from `*(u16*)p`, narrowed with `(s32)(s16)` at the call site and in the
  `< 0x280` compare). Workspace tmp/grind/func_800481E8/s4/perm, built by
  tmp/grind/func_800481E8/s4/mkperm.sh. The seed was VALIDATED byte-identical to
  candidate before launch: 56 insns, exactly the same 10 frame-offset diffs and
  nothing else. Honest `--stack-diffs` scorer, base score 266 (identical to s2's
  base -- independent confirmation that V6 and the candidate are the same bytes).
- [s4] CAMPAIGN RESULT (telemetry in metrics/events.jsonl, label `v6-fresh-seed`):
  **54,167 iterations / 1,402 s / 8 jobs / 3 novel finds / best_new_score 202 /
  harvested + STOPPED (9 procs killed, 0 orphaned; pgrep clean)**. ALL THREE finds
  are the SAME construct -- a function-scope `volatile <T> new_var;` dead pad whose
  only effect is to grow the frame -- differing only in the volatile width:
  output-212-1 `volatile unsigned short`, output-207-1 `volatile short`,
  output-202-1 `volatile unsigned char`. That is the volatile-coercion forbidden
  family (frame coercion by volatile-typed scalar), score-inert under the sandbox
  and a layer-1 FAIL on sight. Banked
  rejected/v6-chassis-volatile-pad-permuter-find.c.
- [s4] **THE KILL (permuter modality).** Two independent seeds, two structurally
  different chassis spellings, ~106k combined iterations, and the search converges
  on the identical forbidden construct BOTH times -- so the basin is a property of
  THE FUNCTION, not of the seed spelling. Stronger still: even the cheat does not
  close the gap. Best honest score across all 3 finds is 202 against a base of 266;
  no find reaches or approaches 0. The permuter has no mutation that adds frame
  bytes except by declaring a local, and every local-declaring mutation lands in a
  forbidden family (volatile-coercion, address-escape, dead array). This is exactly
  what s2's structural proof predicted, now confirmed from a second basin.
  The permuter modality is spent for func_800481E8.
- [s4] Fresh-seed discipline observed per .claude/rules/permuter-directives.md
  Campaign discipline: 4 in-turn blocking `wait` windows (30 s / 105 s / 547 s
  timeout / 89 s / 546 s timeout), ~23 min wall on a single fresh seed, novel finds
  all in one dead family, then harvest --stop. No campaign outlived the session.

## s4-rerun (permuter, 2026-08-20) — third seed: DIRECTED HYBRID, dry campaign

Context: the first s4 session was DISCARDED by the driver validator (it returned
`owner-gated` while its mandated modality was `permuter`; a dead axis in a
non-escalation modality is a `progress` outcome). Its src/ edits were reverted by
the driver, but its ledger writes survived on disk and are the block above. This
re-run re-measured the chassis and spent one further, structurally distinct
permuter lever before banking.

- [s4b] CHASSIS RE-MEASURED on HEAD (which carries `INCLUDE_ASM("asm/funcs",
  func_800481E8);` at src/text1b.c:68 after the 2026-08-19 asm-until-matched
  migration): applying memory/grind/func_800481E8/candidate.c over that line gives
  `sandbox func_800481E8 --disable all` = **10**, target_insns 56, build_insns 56,
  rules_dropped 0, cheat_asm_stripped 278. Floor is UNCHANGED at 10; residual is
  still exactly the 10 frame-offset instructions (frame 40 vs 72, vars 0 vs 32).
- [s4b] THIRD SEED (new lever, not previously tried): a **directed hybrid** base,
  i.e. one workspace whose PERM_* directives span BOTH previously-campaigned
  chassis at once instead of one fixed spelling per campaign —
  `PERM_GENERAL` over the loop-local declaration set (s16/u16 locals ⇄ s32 raw
  holders, i.e. the s2 candidate chassis ⇄ the V6 chassis), `PERM_GENERAL` over
  each of the three halfword stream reads (`(s16)(*(u16*)p)` ⇄ `*(u16*)p`), and
  `PERM_LINESWAP` over the entire 13-statement loop body (word load, four pointer
  advances, four stream reads, the `>>2<<2` address derivation, the early
  `a0_for_call` precompute). Workspace tmp/grind/func_800481E8/s4/perm3
  (base.c/settings.toml/compile.sh/target.o). `perm_pad_var_decl` and
  `perm_inline` weights pinned to 0.0. Honest `--stack-diffs` scorer.
  Base score **266** — identical to both prior seeds, independently re-confirming
  that all three chassis emit the same 56 bytes and differ only in the frame.
- [s4b] CAMPAIGN RESULT (telemetry label `directed-hybrid-permgeneral`, in
  metrics/events.jsonl): **46,124 iterations / 1,678 s (28 min) / 8 jobs /
  0 finds / best_new_score null / harvested + STOPPED (9 procs killed)**.
  Three in-turn blocking `wait` windows (544 s / 545 s / 544 s), all returning
  `reason: timeout, novel: []`. Not one output-* directory was produced — the
  directed seed did not even reproduce the volatile-pad cheat that both random
  seeds converged on, because the directive-driven mutation space is confined to
  the spellings and orderings I enumerated and NONE of them changes the frame.
- [s4b] Cumulative permuter evidence for this function now stands at **THREE
  structurally distinct seeds and ~152k iterations** (s2 instrumented 52,043;
  s4 V6 random 54,167; s4b directed hybrid 46,124) with **zero clean forms below
  the base score of 266** and every non-zero find in a forbidden family. The
  directed run adds the sharper statement: mutations restricted to legal
  re-spellings and re-orderings of THIS function's own computations produce no
  frame change at all, which is the permuter-side confirmation of s2's structural
  result (the vars=32 delta cannot come from the body; it can only come from a
  declaration, and every declaration that produces it is a forbidden construct).
- [s4b] Struct/array chassis measured and rejected as seeds (not campaign-worthy):
  a 12-byte `Ent` struct walk (`e->ofs/x/y/z/w`, `e++`) compiles to **47 insns**
  (83 diff lines) and an `h = (u16*)((s32)p+4); h[0..3]` index form compiles to
  **49 insns** (71 diff lines) — both collapse the target's four separate
  `addiu p,p,2` advances, so the walking-pointer spelling in candidate.c is
  load-bearing for the 56-instruction body, not a stylistic choice. Variants
  banked in tmp/grind/func_800481E8/s4/variants/, harness
  tmp/grind/func_800481E8/s4/try.sh.
- [s4b] src/text1b.c was returned to its HEAD state (`INCLUDE_ASM`) before the
  session ended; no build-surface file was modified.

- [s4] Chassis re-measured THIS session on HEAD: candidate.c applied over src/text1b.c:68 gives sandbox --disable all = 10, 56/56 insns, rules_dropped 0, cheat_asm_stripped 278. Floor unchanged at 10; residual is exactly the 10 frame-offset instructions (frame 40 vs 72, vars 0 vs 32).

- [s4] Third permuter seed built and campaigned: tmp/grind/func_800481E8/s4/perm3, a DIRECTED HYBRID base spanning both prior chassis via PERM_GENERAL (loop-local declaration set s16/u16 <-> s32 raw holders; each halfword read (s16)(*(u16*)p) <-> *(u16*)p) and PERM_LINESWAP over the whole loop body. Base score 266 -- bit-for-bit the same base score as the s2 and V6 seeds, independently re-confirming all three chassis emit the same 56 bytes.

- [s4] Campaign telemetry (metrics/events.jsonl, label directed-hybrid-permgeneral): 46,124 iterations, 1,678 s, 8 jobs, finds_total 0, finds_new 0, best_new_score null, stopped=true, procs_killed 9. Three in-turn blocking wait windows, each reason=timeout with novel=[].

- [s4] The directed seed produced ZERO finds where both random seeds produced volatile-pad cheats -- direct evidence that the permuter's frame-moving finds come exclusively from its declaration-inventing mutations (volatile-typed scalar / address-escape / pad decl), never from re-spelling or re-ordering the function's own computations.

- [s4] Cumulative permuter search on this function: 3 structurally distinct seeds, ~152,334 iterations, 3 finds total, ALL of the identical forbidden volatile dead-pad construct, best honest score 202 against a base of 266 -- the cheat does not even close the gap.

- [s4] Seed-viability measurements: struct-walk chassis = 47 insns (83 diff lines); array-index chassis = 49 insns (71 diff lines) vs target 56. Both collapse the four separate pointer advances the target emits, so the walking-pointer spelling is required for the body match.

- [s4] No build-surface file was modified: src/text1b.c was returned to its HEAD INCLUDE_ASM state before the session ended, and no permuter process outlived the session (campaign harvested with --stop, 9 procs killed, post-harvest process scan clean).

- [s4] Process note for the ledger: the FIRST s4 session was discarded by the driver validator for returning owner-gated while its mandated modality was permuter (a standing-ruling terminal disposition requires driver-declared escalation modality). Its src/ edits were reverted; its ledger writes survived on disk and are preserved in evidence.md/hypotheses.md above this entry. This re-run re-measured them and banks one further distinct lever as progress.

- [s5] **BYTES PROVEN.** `memory/grind/func_800481E8/candidate.c` with ONE addition -- a first-declaration `volatile u32 pre_pad[8];` -- applied over `INCLUDE_ASM("asm/funcs", func_800481E8);` at src/text1b.c:176 produces a FULL DRIVER BUILD whose SHA1 is 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (tmp/grind/func_800481E8/s5/build_pad.log). The built object's function disassembles at `addiu sp,sp,-72` with all 58 lines matching asm/funcs/func_800481E8.s (tmp/grind/func_800481E8/s5/built_func.txt). No other change to the s1 chassis; first attempt, no search.

- [s5] **The s2 frame taxonomy mislabelled this function's residual.** The 32 untouched bytes are NOT a trailing/tail pad. Frame map decoded from asm/funcs/func_800481E8.s: `.frame $sp,72`; outgoing-args area 0x00-0x17 (the 5th-argument slot at 0x10 IS written -- `sw $v0,0x10($sp)` at 0x80048288); vars 0x18-0x37 with ZERO sw/lw/sh/sb/lh/lb touching it; saved regs 0x38-0x47 (`sw s0,0x38 / s1,0x3C / s2,0x40 / ra,0x44`). The untouched region sits BETWEEN args and regs -- i.e. it is the VARS region, exactly where a FIRST-DECLARED local lands. That is the leading-pad shape, byte-for-byte the same args24/vars32/regs16 = 72 layout as the two granted in-file siblings func_80047EE8 and func_80047FBC, not the tail-pad shape the owner's 2026-08-20 grant entry declined to extend to.

- [s5] Consequence for the standing disposition: the 2026-07-28 / 2026-08-20 "REFUSED / OWNER-ACCEPTED INCOMPLETE" entries rested on gate 2 failing for lack of precedent for an "unwritten-TAIL phantom-frame family". With the shape correctly identified as the LEADING/first-decl form, gate 2 PASSES on precedent the owner has already granted twice in this same file and sanctioned generally on 2026-08-18 (.claude/rules/no-new-park-categories.md:390), with SOTN-master exhibits at docs/reference/sotn-construct-index.md:101 (`src/st/sel/2C048.c:564 volatile u32 pad; // !FAKE:`) and :103 (`src/st/sel/stream.c:80 volatile u32 pad[4]; // FAKE`). Those terminal entries are superseded by measurement.

- [s5] `sandbox func_800481E8 --disable all` after the change still prints score 10 / 56 insns / rules_dropped 0 / cheat_asm_stripped 277 (tmp/grind/func_800481E8/s5/sandbox_pad.log) -- the engine strips the pad because func_800481E8 has no row in `engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS`. Exactly the sibling situation before the 2026-08-20 grant. The remaining work is the one-line row `"func_800481E8": frozenset({("pre_pad", 8)})`; engine/ is outside a grind session's scope AND on the add-scope-allow denylist, so this is an INTEGRATION HANDOFF, not a codegen question.

- [s5] Cross-knowledge sweep is what unlocked this: the two siblings were completed HOURS earlier (commits 3f995fe9 / b734618d / 33e87423, 2026-08-20) and their evidence (memory/grind/func_80047EE8/evidence.md:154 -- all four cluster members reserve EXACTLY 32 phantom bytes) was already in the brief's cross-knowledge hits. The ledger's own tail-pad label was the only thing keeping this function parked.

- [s5] src/text1b.c was returned to its HEAD `INCLUDE_ASM("asm/funcs", func_800481E8);` state before the session ended; `git status` shows only memory/grind/func_800481E8/ and metrics/events.jsonl modified.

- [s5] Full driver build with candidate.c + `volatile u32 pre_pad[8];` applied over INCLUDE_ASM at src/text1b.c:176 produced SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (tmp/grind/func_800481E8/s5/build_pad.log). The function is byte-matched by this C.

- [s5] Built object disassembly: `addiu sp,sp,-72` frame, 58 lines identical to asm/funcs/func_800481E8.s (tmp/grind/func_800481E8/s5/built_func.txt).

- [s5] Frame map decoded from target asm: .frame $sp,72 = args 0x00-0x17 (the 5th-arg slot at 0x10 IS written by `sw $v0,0x10($sp)` at 0x80048288) + vars 0x18-0x37 (ZERO accesses of any width) + saved regs 0x38-0x47 (s0/s1/s2/ra). The untouched span is the VARS region between args and regs - the first-declared-local position - not a trailing pad.

- [s5] That layout (args 24 / vars 32 / regs 16 = 72) is byte-for-byte the layout of the two in-file siblings func_80047EE8 and func_80047FBC, whose ('pre_pad', 8) rows the owner granted on 2026-08-20 (engine/volatile_cheats.py:757-758).

- [s5] sandbox func_800481E8 --disable all after the change: score 10, target_insns 56, build_insns 56, rules_dropped 0, cheat_asm_stripped 277 - the engine strips the pad because func_800481E8 has no _SANCTIONED_UNWRITTEN_PADS row. engine/ is outside session scope AND on the add-scope-allow denylist (.claude/rules/integration-handoff-self-serve.md), so this is an INTEGRATION HANDOFF, not an endgame lock.

- [s5] Family conformance: ARRAY form, volatile-qualified, first-declaration position, no `(void)pad;` or `&pad` shim, FAKE-annotated with what + named GCC-pass mechanism + lever-exhaustion - every constraint the 2026-08-18 family names. Six-test vet written to memory/grind/func_800481E8/self_vet.md.

- [s5] The only other FAKE construct in the body is `arg0 = 0;`, the dead-param-assign spelling listed verbatim at .claude/rules/dead-store-fake-exception.md:29 and Judge-PASSed on the in-file sibling func_80047EE8. Zero pins, zero __asm__, zero alias renames, zero regfix/asmfix rules.

- [s5] Provenance is NOT search: s2/s4's ~152k permuter iterations converged instead on the forbidden function-scope `volatile <scalar>` coercion (banked in rejected/). This construct came from the cross-knowledge sweep of the siblings solved hours earlier plus a re-derivation of the frame map.

- [s5] HEAD is unchanged: src/text1b.c was returned to `INCLUDE_ASM("asm/funcs", func_800481E8);` before session end; git status shows only memory/grind/func_800481E8/, docs/grind/decisions.md and metrics/events.jsonl modified.

- [s5] Escalation filed this session at the end of docs/grind/decisions.md, naming func_800481E8, with the exact operator steps (add the row, apply candidate.c, re-sandbox, full build, layer-2 cheat-reviewer, queue done).
