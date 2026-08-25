# Evidence bank — func_80040D48

## [s1 2026-08-24, recon] Baseline, directive acknowledgment, full diff map, floor 34 → 24

**Owner directive ACKNOWLEDGED and being executed:** RULES-TO-ZERO campaign —
34 regfix rules (largest remaining stack), jtbl-coupled (the compiled switch
emits the jtbl), pure-C COMPLETED-C is the route, INCLUDE_RODATA measured dead
in wave 2 (do not re-attempt). This session works pure-C toward 0; no
INCLUDE_ASM/INCLUDE_RODATA probes were run or are needed.

**Honest floor:** at dispatch (HEAD 250f7ee6 committed body) `sandbox
func_80040D48 --disable all` = **34** (272/272 insns, rules_dropped 34,
cheat_asm_stripped 4 — the body's `register s32 a0_s7 asm("s7")` pin among
them). End of s1 with the candidate body in src: **24** (272/272). Every
divergence is a register-seat / addressing-anchor diff; instruction count and
schedule match throughout.

**canonical:** verdict C (pure-C target, distance 34 ≤ 50). Queue agrees.

**Rule-stack decode (regfix.txt:999-1040, all 34):** (1) `$21<->$22` swaps ×3
ranges = the s5/s6 seat swap; (2) prologue structural substs @14-28 = the
load-into-a0 / copy-in-delay-slot shape (paired with the committed pin); (3)
offset substs @49-84 = the a4p IV-bias cluster (s3+0x7C anchor vs s3+0x68);
(4) reg substs @160-244 = mflo temp + tail-loop pointer seats. Classes (1),
(2) and most of (4) are now closed honestly; see below.

**What closed this session (34 → 24), in order:**

1. **Prologue two-var load shape (34 → 31).** Target: `lw $a0,%lo(D_800A9A10)($at);
   beqz $a0; addu $s4,$a0,$zero` (delay slot) + `addiu $s5,$s4,0x2C` landing
   at insn 28 filling a later load-delay nop. Committed body loaded straight
   into s4 (pin + rules faked the rest). Honest fix: `tmp = (u8 *)D_800A9A10[a0];
   if (tmp == 0) return; s4 = tmp;` — tmp is caller-save (dies at the copy),
   the copy lands in the beqz delay slot, and s4+0x2C computes late exactly as
   target. ALSO removed the `asm("s7")` pin entirely: param a0 naturally lands
   in s7 (it crosses 6 calls; pseudo 72 is allocated last of the call-crossing
   set and takes s7 with no coercion). The final call uses plain `a0`.
   NOTE for vetting: `tmp` is once-written, twice-read? — no: written once,
   read twice (test + copy). It holds a real consumed value (the loaded
   pointer) and a human writes exactly this null-check-then-keep shape; it is
   arguably more natural than the one-liner. Flag for self-vet when
   candidate-ready; if the Judge treats it as a named-intermediate family
   member it may need the 6-prong check (multi-read may disqualify prong 1 —
   in that case respell as `if ((s4 = (u8 *)D_800A9A10[a0]) == 0)`-class forms
   and re-measure; NOT yet measured).

2. **s5/s6 seat swap + mflo temp (31 → 24, with the goto-loop spelling).**
   greg dump (tmp/grind/func_80040D48/dumps/, regenerate via
   `pwsh tools/grinder/dump.ps1 func_80040D48`): allocation order is
   priority-sorted; pseudo 77 (arg5, 4 refs/362 insns, log2(4)*4/362 ≈ .022)
   was allocated immediately BEFORE pseudo 79 (s4+0x2C ptr, 2 refs/109 insns
   ≈ .018), taking s5 and pushing the pointer to s6 — exactly the adjacent
   inversion the 3 swap rules papered over. Target's $s5 holds BOTH the
   s4+0x2C value (def@29, use@166 = `sh $zero,6($s5)`) AND the Copy8-loop
   source pointer (def@203 `lw $s5,0x40($a3)`, 10 loop-weighted refs) — i.e.
   in the ORIGINAL these were ONE C variable. flow.c counts
   `reg_n_refs += loop_depth` (flow.c:2081 etc.), so the merged pseudo's
   priority beats arg5's and it is allocated first → s5; arg5 falls to s6.
   Honest fix: reuse the existing `s5` local as the copy-loop pointer
   (variable-reuse family, and here it is evidence-backed as the original's
   own structure, not a codegen trick — the seat only falls out right if the
   two roles share one variable).

3. **Copy-loop spelling constraint (the rotation trap).** With `for (;;) {...
   break;}` + reused s5, loop.c emits loop notes, the loop rotates (bottom
   `bnez` test) and cse folds the first-iteration load to `lw s5,0x112C(s4)`
   → 273 insns, floor 29. Target keeps top-test + unconditional `j` back-edge
   and loads through the a3p base even on iteration 0. Fix: explicit-label
   spelling (`copyloop: { ... goto copyloop; } copydone:;`) — no loop notes,
   no rotation, floor 24, 272/272. Banked:
   rejected/for-loop-s5-reuse-rotates-copy-loop.c.

**Remaining 24 (floor-24 map, artifact
tmp/grind/func_80040D48/s1/floor24_diff.txt; line numbers = 0-based insn
index; ignore LO/LBL/li lines — normalizer reloc artifacts):**

- **Class A — a4p IV-bias cluster (4 diffs; lines 50/65/75/85).** First init
  loop (case 0): ours anchors the store pointer at `addiu $a0,$s3,124`
  (= s3+0x7C) with sh offsets -4/-2/0; target anchors at s3+0x68 (the natural
  `a4p = s3 + 0x68`) with offsets 16/18/20. The C already spells
  `a4p = s3 + 0x68` + `*(s16*)(a4p+0x10/0x12/0x14)`; loop.c strength-reduction
  re-biases the induction variable by +0x14. Old rules @1012-1015 rewrote
  exactly this. Pass attribution needed from .loop dump (regenerate dumps —
  they were generated pre-candidate this session, so the .loop content is
  stale vs the current body but the a4p loop was untouched by s1 edits).
- **Class B — a2p/a3p seat swap in the Copy8 loop (~14 diffs; lines 200-230).**
  Target: a2p→$a2(6), a3p→$a3(7). Ours: a3p→$6, a2p→$7. Both local-alloc'd
  (no calls in loop). a3p has ~3-4x a2p's weighted refs, so priority order
  gives a3p the first free reg ($6) — target's order implies the original's
  a2p qty outranked or preceded a3p's. Derived-base anchor respelling is DEAD
  for this (cse folds — rejected/derived-base-anchor-ref-cse-folded.c). Next:
  read local-alloc.c qty_compare + .lreg qty dump for the ACTUAL order; probe
  first-use-order / def-order permutations (e.g. define a3p first, or touch
  a2p before a3p inside the loop).
- **Class C — a2p2/-1 seats (6 diffs; lines 234-246).** Final walker loop:
  target a2p2→$6, -1 holder→$4; ours a2p2→$4, -1→$5. Straight-line local
  alloc after the copy loop; plausibly cascades from Class B ($6/$7 roles) —
  re-measure after B closes before spending levers on it.
- **mflo seat (was 2 diffs) closed itself** when s5/s6 flipped (162: mflo $8 ✓).

**Chassis notes:** all measurements this session on HEAD 250f7ee6 + the
candidate body in src/text1a_pre.c. The committed rule-era body's shape
(named intermediates, decl order) is rule-calibrated — the candidate body
supersedes it as the working frontier. Diff tooling: s1/diff.sh + s1/norm.py
(normalizes objdump-vs-splat mnemonics; li/addiu-from-zero and LO/LBL lines
are artifacts, not diffs).

**Sibling reference:** func_80040B44 (same TU, COMPLETED-C 2026-08-24,
ledger in git at 68065f31) — its s2 confirmed the fresh-vs-shared-local
partition drives seats and that suspect holders must be re-tested after every
structural change (probe relativity). Both lessons applied here.

- [s1] Owner directive acknowledged + executed: pure-C route only, no INCLUDE_RODATA/INCLUDE_ASM probes run (wave-2 dead per directive)

- [s1] Dispatch floor 34 (272/272, 34 rules dropped, 4 cheat-asm stripped incl. the body's asm(s7) pin); end-of-s1 floor 24 with candidate body in src, 272/272

- [s1] Full rule-stack decode + floor-24 diff map banked in evidence.md; residual = Class A a4p IV-bias (4, loop.c giv re-bias +0x14), Class B a2p/a3p local-alloc seats (~14), Class C a2p2/-1 seats (6, likely cascades from B)

- [s1] greg/lreg dumps read for pass attribution: seat swap was global.c priority order (pseudo 77 arg5 4refs/362 vs 79 ptr 2refs/109, adjacent); flow.c:2081 reg_n_refs += loop_depth is the weighting that lets the merged-variable structure win

- [s1] cse1-folds-derived-base-anchors banked as a dead mechanism class for ref-bumping (rejected/derived-base-anchor-ref-cse-folded.c)
