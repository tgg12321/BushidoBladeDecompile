# func_8003AB44 — WIP notes (2026-06-10)

## TL;DR

8-way switch (`D_800A38AC` cases 0..7) in code6cac_c_ab.c. HEAD floor: **6**.
This session removed 3 score-inert cheat-asm scheduling barriers
(`__asm__ __volatile__("" ::: "memory")`) in cases 0 and 1, but did NOT lower
the floor. Candidate is a SOURCE CLEANUP, not floor progress.

## Resume instructions

1. Apply `candidate.c` to `src/code6cac_c_ab.c` (replace the existing
   `s32 func_8003AB44(void) { ... }` body).
2. Run `pwsh tools/eng.ps1 sandbox func_8003AB44 --disable all` → expect
   `score: 6, cheat_asm_stripped: 6` (HEAD has `cheat_asm_stripped: 9`).
3. Iterate from there on the remaining 6-distance gap (see "the gap" below).

## The gap (sandbox stripped, target_insns == build_insns == 93)

### Case 2 — branch polarity flip (4 diffs at idx 42-49)

**Target** wants:
```
beqz $v0, .L_end       # if func == 0, go to end (return 0)
addu $v0, $zero, $zero # delay: set return value
j    .L_done           # else jump to done block
addiu $a0, $zero, 3    # delay: setup for done's next jal
```

**Our build** naturally emits:
```
bnez $v0, .L_done      # if func != 0, go to done block
addiu $a0, $zero, 3    # delay: setup for done's jal
j    .L_end            # else go to end (return 0)
addu $v0, $zero, $zero # delay: set return value
```

These are semantically identical but with INVERTED branch sense and
correspondingly swapped delay-slot fills. GCC's reorg pass picks
`addiu $a0,3` as the BRANCH's delay slot fill (longer INSN_PRIORITY chain
to the done block's jal). Target picked `move $v0,$zero`.

Lever attempts that did NOT shift this polarity:
- Source-level test inversion (`==0 return; goto done` vs `!=0 goto done; return`)
- Wrapping in `{ s32 r; r = func(...); if (r == 0) ... }` (probably untested fully)
- Inlining the done block in case 2 (DID flip polarity but lost 1 insn — score 15 vs 6 base)

### Case 5/6 — register choice ($v1 in build, $v0 in target)

The `D_800A38AC++` translates to `lbu $rN, %gp_rel; addiu $rN, $rN, 1;
sb $rN, %gp_rel`. Our GCC picks $v1, target uses $v0. Regfix
`$2 <-> $3 @ 72-75` rewrites.

Likely cause: case 4 falls through to case 5/6. After `jal gpu_SetDispMask`
(void return), GCC conservatively treats $v0 as recently-returned, and the
case 5/6 jtbl entry label (referenced from the jump table) makes GCC
conservative about live-in registers at that point.

Lever attempts (no effect):
- Split case 4 from case 5/6 with duplicated increment (cross-jump merge
  consolidated them back, still picks $v1)

Not tried this session:
- Re-declare `gpu_SetDispMask` to return s32 (currently void in src)
- Move case 5/6's body BEFORE case 4 in source (might break fall-through
  semantics)

## Reusable observations

- The 3 `__asm__ __volatile__("" ::: "memory")` barriers in cases 0 and 1
  of HEAD's source are SCORE-INERT. Sandbox stripped them in both HEAD and
  the candidate (score 6 in both). Removing them is safe and reduces
  cheat-asm debt.
- This is a single-call-result test-and-dispatch within a switch case.
  The polarity is a delay-slot-fill priority issue, not a control-flow
  issue.

## Session 2 (2026-06-10) addendum

Confirmed floor 6 from the WIP candidate base (clean apply). Disassembled both
build objects and pinned the exact diff locations:

| Byte offset | OURS (stripped) | TARGET | Diff |
|---|---|---|---|
| 0xb8 | `bnez v0, e0`     | `beqz v0, 164`  | branch sense + target |
| 0xbc | `li   a0, 3`      | `move v0, zero` | delay slot fill |
| 0xc0 | `j    164`        | `j    e0`       | jump target |
| 0xc4 | `move v0, zero`   | `li   a0, 3`    | delay slot fill |
| 0x130 | `lbu  v1, gprel` | `lbu  v0, gprel` | register choice |
| 0x138 | `addiu v1,v1,1`  | `addiu v0,v0,1` | register choice |
| 0x13c | `sb   v1, gprel` | `sb   v0, gprel` | register choice |

7 distinct insn-byte diffs; masked Levenshtein = 6 (one diff masks into
adjacent context). Build_insns = target_insns = 93 ✓.

`scan_hand_coded --single func_8003AB44 --json`: tier **LOW**, score 0, all
five hand-coded signals (S1/S2/S6/S7/S8) report negative. So pure-C is the
target per [[canonical-gate-distance-not-evidence]]; do NOT route to
canonical-asm.

New variants tested (all score 6, rejected) — see meta.json rejected_forms:
1. Case 2 named local for call return — folded to same RTL.
2. `return r` instead of `return 0` — GCC const-prop folded `r == 0` back to literal 0.
3. Case 5/6 named local for increment value — GCC saw through the alias.

## Next-session priorities (session 3 onwards)

Per session-2's pinned diff locations, the gap is entirely RTL-level
(reorg.c delay-slot priority + global RA tiebreaker). Untried levers, in
expected payoff order:

1. **Read sibling matched switch-dispatch functions in code6cac_c_ab.c**
   (func_8003ACB8 — the immediate next function — is a do-while loop, not a
   switch; look further). Find a function with similar 8-way switch + jtbl +
   fall-through that DID match in pure C; diff its `.greg` and `.dbr` dumps
   against ours.
2. **`BB2_ALLOC_DEBUG` on tmp/gccdbg/cc1** to see why case 5/6's lbu pseudo
   gets $v1 not $v0. The hypothesis (jtbl-entry conservative live-set
   keeping $v0 alive) is testable — the dump shows allocno priority and
   conflict-set membership for each pseudo. Without the dump we're guessing.
3. **`PERM_GENERAL`-directed permuter** from the candidate base. Random
   mutations from the session-1 permuter run were not done; only structural
   variants by hand. Directed-permuter type-mutation + statement-reorder
   passes may find a non-obvious form.
4. **Shorten the `done` block's downstream chain** so reorg.c's delay-slot
   priority gives `move v0,0` (the return path) a higher chain than
   `addiu a0,3` (feeding done's jal). Concretely: see if `done:` can be
   inlined in case 3 only (eliminating case 2's `goto done`) — though that
   loses cross-jump merge benefit. Or inline `func_8008C464(3,0,0)`
   differently so its setup doesn't dominate.

## Session 3 (2026-06-10) addendum

Re-confirmed candidate floor 6 from a clean apply of `candidate.c` to
`src/code6cac_c_ab.c`. Re-disassembled `tmp/sandbox/func_8003AB44/code6cac_c_ab.o`
and matched the exact same 7-slot diff cluster session 2 pinned (4 polarity
diffs at byte offsets 0xb8-0xc4, 3 register diffs at 0x130/138/13c).

4 more structural variants tested (all rejected — see meta.json
rejected_forms). Summary:

| Variant | Score | Notes |
|---|---|---|
| case 2 else-if cascade | 6 | syntactic — same RTL |
| case 2 fully inline done (no goto) | 15 | cross-jump merge drops `j done; addiu $a0,3` pair |
| case 4/5/6 merged with internal `if (D_800A38AC == 4)` check | 10 | +4 insns from the dispatch (97 vs 93) |
| case 4 → case 5/6 goto-bump | 6 | goto-to-following-label folded; same RTL |

Sibling matched switches in the same file (`suDispMentalBar`,
`func_8003AE5C`) use switch-with-break (no per-case early-return), so their
codegen doesn't transfer — they don't exercise the reorg.c invert-jump
peephole or the case-5/6 RA conservativism this function hits.

The manual structural lever search is now near-exhausted across 3 sessions
(11 forms tested + the original candidate). The remaining gap is genuinely
RTL-level:

- **Case 2 polarity (4 diffs):** reorg.c's `relax_delay_slots` invert-jump
  peephole. Target wants `beqz $v0, end / addu v0,0,0 / j done / addiu $a0,3`;
  GCC's natural emission inverts to `bnez $v0, done / addiu $a0,3 / j end /
  addu v0,0,0` because INSN_PRIORITY of the `addiu $a0,3` chain (long: feeds
  done's jal which chains to the next state-store) outweighs `addu v0,0,0`
  (short: only feeds jr ra). Inverting this without LICM regressions or
  cross-jump merge collapse is the open problem.
- **Case 5/6 register choice (3 diffs):** at the `.L8003AC74` jtbl entry
  (case 5/6 fall-through from case 4 after `gpu_SetDispMask(0)`), GCC picks
  $v1 for the `lbu/addiu/sb` chain. Target picks $v0 and reuses it for the
  `j end / addu v0,0,0` return-value setup. The natural ascending preference
  ($v0=2 < $v1=3) says target is following the preference; ours is the
  anomaly. Cause hypothesized to be jtbl-entry conservative live-set keeping
  $v0 considered live-in across the merged case-5/6 entry — but the actual
  pseudo conflict graph hasn't been dumped via BB2_ALLOC_DEBUG yet.

## Next-session priorities (session 5 onwards)

Session 4 executed the BB2_ALLOC_DEBUG diagnostic. The result is decisive
and reshapes the priority list.

### Session 4 finding (2026-06-10) — ALLOCDBG isolated to single allocno

Used `tmp/gccdbg/cc1` (instrumented build SHA1 `a17fc6bbcb...` May 30;
canonical `tools/gcc-2.7.2/build/cc1` SHA1 `045c9543d3...` May 18
UNTOUCHED) on a single-function isolated preprocessed input (built via
`tools/decomp-permuter/strip_other_fns.py tmp/8003ab44/iso.i
func_8003AB44`, then `cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000
-mips1 -mno-abicalls -fno-builtin -w`).

The function has ONE non-trivial allocno (everything else uses scratch
regs directly):

```
ALLOCDBG ord=0 pseudo=94 hardreg=3 nrefs=3 livelen=6 pri=5000
```

- pseudo=94 = the case 5/6 lbu/addiu/sb increment value
- hardreg=3 = $v1 (the wrong register; target uses $v0)
- ord=0 = allocated FIRST
- nrefs=3, livelen=6, pri=5000 (= floor_log2(3)*3/6*10000)

Because pseudo 94 is allocated FIRST (ord=0) and is the ONLY allocno
yet picked $v1 over $v0 (where $v0 is at index 2, lower than $v1 at 3,
preferred per natural ascending order), **$v0 must be excluded from the
available register set when this allocno is processed**. Nothing else
claims $v0, so the exclusion comes from `basic_block_live_at_start
[.L8003AC74]` (the case 5/6 jtbl-entry block) containing $v0 in its
live-in set.

This confirms session 3's hypothesis explicitly: flow.c's conservative
live-set computation at the multi-entry jtbl block keeps $v0 marked
live-in.

Target asm at offsets 2B474-2B488 (the case 5/6 cluster + j-delay slot):
```
.L8003AC74:
   lbu   $v0, %gp_rel(D_800A38AC)($gp)
   nop                                       # load-delay
   addiu $v0, $v0, 0x1
   sb    $v0, %gp_rel(D_800A38AC)($gp)
   j     .L8003ACA8                          # to shared epilogue
   addu  $v0, $zero, $zero                   # DELAY SLOT — return value 0
```

Target REUSES $v0 across the increment cluster AND the return-value
setup (j-delay slot). The reuse is natural — pseudo dies at the sb,
then `move v0, 0` writes $v0 fresh. Same byte count as ours; just
different register.

### What this means for next-session levers

The lever class to explore: **defeat the flow.c $v0 live-in at
.L8003AC74**. Predecessors:
1. Case 5 via jtbl jr — should be clean ($v0 dead after jr)
2. Case 6 via jtbl jr — should be clean
3. Case 4 fall-through after `jal gpu_SetDispMask(0)` — call clobbers
   $v0 semantically, BUT flow.c's call-clobber tracking at the
   fall-through join may be insufficient (the conservative side
   prevails when uncertain)

Concrete next-session hypotheses (un-tried):

1. **Explicit $v0-kill on case 4 path before fall-through.** Try a
   structural construct that makes flow.c definitively see $v0 die
   before the jtbl-entry. Candidates:
   - Read-then-discard in case 4: `(void)D_800A38AC;` (likely DCE'd —
     CHECK).
   - Move the case 5/6 body into case 4 inline, with a tail-call style
     `goto` at the end that prevents cross-jump merge from the jtbl
     entries.
2. **Defeat the multi-entry merge by giving each case 5/6 entry a
   distinct successor.** Currently both case 5 and case 6 jtbl entries
   land at .L8003AC74. If case 5 and case 6 each had their own (byte-
   identical) increment block, jump2 cross_jump would merge them back
   — same wall as the rejected `case 4/5/6 split` variants.
3. **Restructure case 4 to OWN the increment**, leaving case 5/6 as
   the separate path. If case 4's path doesn't fall through to .L8003AC74
   anymore, the flow.c live-in only includes the jtbl-jr predecessors,
   which should be clean. BUT this duplicates the increment AND
   cross-jump merge will likely re-collapse.
4. **PERM_GENERAL permuter from candidate.c** — un-tried; may surface a
   structural mutation the hand search hasn't found.

Notable un-helpful avenues now ruled out by ALLOCDBG:
- ~~Block-local var split on the increment value~~ — Lever A from
  [[register-alloc-pure-c]]; pseudo 94 already has tight livelen 6, no
  room to shrink. The split would just create another allocno of
  similar profile.
- ~~Narrow integer type~~ — Lever B; the value is already u8 (byte).
- ~~Loop-local precompute~~ — Lever C; no loop in the structure.
- ~~Volatile-bump on D_800A38AC~~ — Lever family forbidden per
  [[inline-asm-policy]] expanded catalog; would also bump nrefs into
  cheat territory.

### Case 2 polarity (the other 4-diff gap) — unchanged

Session 4 did not investigate the case 2 polarity gap further. The
reorg.c instrumented dump (BB2_REORG_DEBUG hooks present in
tmp/gccdbg/cc1) remains un-run on this function. Session 5+ could
run it for case 2 specifically (per session 3 priority 3).

## Session 5 (2026-06-10) addendum

Applied candidate.c, confirmed clean floor 6 (build_insns=target_insns=93,
rules_dropped=5, cheat_asm_stripped=6 from sibling functions; the function
itself has 0 cheat-asm in candidate form).

Tested **1 new structural variant** beyond session-4's documented
rejected_forms — source-order flip (case 5/6 cases physically BEFORE case 4
in source, with duplicate `D_800A38AC++; return 0;` in case 4 instead of
fall-through):

| Variant | Score | build_insns | Notes |
|---|---|---|---|
| candidate (canonical, case 4→5/6 fall-through) | 6 | 93 | Reference floor |
| case 5/6 BEFORE case 4 in source + duplicate increment | 6 | 93 | NEW; rejected |

cc1's jump2 cross-jump-merge consolidates the duplicate `D_800A38AC++; return
0;` blocks regardless of source order; the merge target's predecessor
structure remains case-4-fall-through + 2 jtbl entries. flow.c's
basic_block_live_at_start computation at the jtbl-merge block is insensitive
to syntactic case order — pseudo 94 still allocates to $v1.

This confirms session 3-4's diagnosis: the case 5/6 register issue is a deep
property of the **predecessor live-out / multi-entry jtbl join**, not a
function of cc1's RTL block emission order. The manual structural lever
space across 5 sessions now spans **12 rejected variants** (plus the
candidate-form source cleanup). The remaining un-tested lever class is
**directed-permuter from candidate.c base** — the only systematic way to
explore C variants outside the hand-derivable space.

## Session 6 (2026-06-10) addendum

Applied candidate.c, reconfirmed clean floor 6 (build_insns=target_insns=93,
rules_dropped=5, cheat_asm_stripped=6 from sibling functions; the function
itself has 0 cheat-asm in candidate form). Executed PRIORITY 1 from session 5:
**directed permuter from candidate.c base.**

### Setup

The `permuter/8003ab44/` workspace existed from an earlier run today
(timestamps Jun 10 11:18); base.c was structurally equivalent to candidate.c
but carried two `/* fall through */` C-comments that cc1 (which consumes
preprocessed-style input — no comment-stripping pass in cc1 alone) rejected as
parse errors. Removed the comments (no semantic change — fall-through is the
absence of a `break;`, the comment is documentation only) and verified the
base compiles cleanly through the full pipeline.

target.o has func_8003AB44 at offset 0, length 0x174 (= 372 bytes = 93 insns),
matching target's exact byte count.

### Run

```bash
python3 tools/decomp-permuter/permuter.py permuter/8003ab44 -j8 \
        --best-only --stop-on-zero
```

8 parallel workers, ~5 minutes wallclock, **33,307 iterations**. Cleared the
pre-existing `output-*` directories from earlier runs first.

### Result — permuter mutation space exhausted, no legitimate sub-base form

| Metric | Value |
|---|---|
| Base score (no mutations) | 540 |
| Min score over 33k iters | 320 |
| Max score | 12,130 |
| Score-0 hits | **0** |
| Iterations | 33,307 |

Best mutation-bearing form (saved at `permuter/8003ab44/output-320-1/source.c`,
also copied to `memory/wip/func_8003AB44/rejected/session6-permuter-best-320.c`
since `permuter/` is gitignored):

```c
case 1: ...
    if (D_800A38A0 == 0) {
        gpu_SetDispMask(1);
        do { D_800A38AC = 2; } while (0);   /* CHEAT — do-while-zero out of sanctioned scope */
        return 0;
    }
case 2: ...
    if (func_8008C464(3, 1, 0) == 0) {
        return 0;
    }
    D_800A37D8 = 0;                         /* CHEAT — dead-store coercion */
    goto done;
```

### Cheat-reviewer verdict on the 320 candidate: FAIL (test #5, both mutations)

**Mutation 1 — `do { D_800A38AC = 2; } while (0);`** wrap fails the
[[do-while-zero-exception]] gate. That exception is sanctioned ONLY for the
LABEL_OUTSIDE_LOOP_P / reorg.c invert-jump peephole interaction. Func_8003AB44's
gap is (a) case 2 polarity (reorg.c eager-fill of `addiu $a0,3` vs target's
`move v0,0` in the `bnez` delay slot) and (b) case 5/6 register choice ($v1 vs
$v0 — flow.c live-set at the jtbl join). Neither is the LABEL_OUTSIDE_LOOP_P
mechanism — using do-while-zero here bends a different GCC pass and reopens
the slippery-slope the rule's narrow scoping closes.

**Mutation 2 — `D_800A37D8 = 0;` before `goto done;`** in case 2 is a
dead-store coercion in the family of forbidden Lever D /
dead-conditional-store / goto-end-prologue-delay-slot / param-local-alias-
prologue-pair-flip. The store emits a real `sw $zero, %gp_rel(D_800A37D8)` in
the output (NOT DCE'd; would be a +1-insn regression in isolation) yet has no
semantic justification — D_800A37D8 was reset to 0 in case 1 of a prior call
(state machine reaches state 2 only after state 1 ran). A real programmer
would not write the duplicate reset. Same intent / different spelling as the
forbidden families.

### Why permuter score 320 is NOT a real improvement

The permuter's weighted score metric (regs × 5, reorderings × 60, ins/del ×
100) ≠ the sandbox masked-Levenshtein metric ([[scoring-systems]]). The 320
reflects mutation-induced noise reduction in the alignment-sensitive scoring
layer; the sandbox masked floor stays at 6, and the 5 baseline regfix rules
are NOT retireable from any 320-class form (the case 2 polarity flip + case
5/6 register diff cluster remain). I did NOT measure each 320 variant's
sandbox masked-Lev score against build/ because (a) the form is rejected on
policy grounds regardless of measurement, and (b) sandbox measurement would
require restoring src/ with the cheat-form, defeating the discipline that
prevents inadvertent commits.

### Genuine evidence-backed exhaustion is now achieved

- **Manual lever space**: 12+ rejected forms across 5 sessions (1-5).
- **Permuter mutation space**: 33,307 iters from candidate.c base, floor 320,
  no legitimate sub-540 form (session 6).
- **ALLOCDBG diagnostic**: case 5/6 issue isolated to flow.c $v0 live-in at
  .L8003AC74 (session 4).
- **scan_hand_coded**: tier LOW (session 2) — pure-C target per
  [[canonical-gate-distance-not-evidence]].
- **No cc1psx divergence**: toolchain is frozen [[no-compiler-divergence]].

## Next-session priorities (session 7 onwards)

Per session 6's exhaustion of the directed-permuter randomization space, the
priority list reshapes — the next-tier diagnostic levers stand:

1. **Instrumented cc1 with `BB2_FLOW_DEBUG`.** `tmp/gccdbg/cc1` already exists
   (May 30 build with ALLOCDBG + SCHEDDBG + PRIODBG hooks from the
   saEft00Add / csmd4 cluster sessions). Add a flow.c hook dumping
   basic_block_live_at_start[N] for each BB after `update_life_info` for any
   function whose name matches `$BB2_FLOW_FUNC`. Run on isolated
   func_8003AB44 .i. Compare live-in[.L8003AC74] across (a) canonical
   compile, (b) the same with case 5/6 syntactically before case 4 (session
   5's case-order flip — though rejected, the dump value differs by source
   order), (c) the same with case 4 split into a non-fall-through form. The
   dump CONFIRMs which predecessor's live-out propagated $v0 — direct
   empirical evidence beyond session 4's ALLOCDBG-via-ord=0 inference.
2. **m2c-reconstruct func_8003AB44** from asm/funcs/func_8003AB44.s; compare
   against candidate.c for ANY semantic difference. m2c's reconstruction
   shape sometimes reveals an original-source feature the hand decompilation
   missed.
3. **Sibling LREG/GREG dumps** from code6cac_c_ab.c's matched switches.
   Session 3's syntactic inspection ruled them out (siblings use
   switch-with-break, not switch-with-early-return); actual greg dumps may
   show a 5/6-case-with-fall-through-from-4 shape that allocated $v0
   naturally, providing a structural template.

## Session 8 (2026-06-10) addendum — MECHANISM REFRAMED

Applied candidate.c, reconfirmed clean floor 6. Did an index-aligned
objdump diff of `build/src/code6cac_c_ab.o` (canonical, with cheats) vs
`tmp/sandbox/func_8003AB44/code6cac_c_ab.o` (cheat-free) and discovered
the actual gap mechanism is NOT what sessions 2-7 documented:

**The real diff is the `$s0` return-value accumulator.** Cheat-free
sandbox build:
  - Prologue: `sw $s0, 16(sp); sw $ra, 20(sp); ... move $s0, $zero`
  - Every case-end j-delay slot: `move $v0, $s0`
  - Fail / case 7: `li $s0, -1` / `li $s0, 1` (instead of `$v0`)
  - Epilogue: `move $v0, $s0; lw $ra, 20; lw $s0, 16; addiu $sp; jr $ra`

Target compilation: NEVER uses `$s0` for the return value. Each case's
j-delay slot is `move $v0, IMMEDIATE` (direct constant). Frame saves
ONLY `$ra` at offset 16, not 20.

The case 5/6 $v0/$v1 swap and case 2 polarity diffs documented in
sessions 2-7 are **secondary artifacts** of the $s0 accumulator's
existence cascading through the RA. The 5 regfix rules patch the
secondaries; the underlying mechanism (which all 5 rules indirectly
serve) is the `expand_return` unified pseudo allocation.

### 2 new rejected forms (this session)

| Variant | Score | build_insns | Notes |
|---|---|---|---|
| switch-with-break + `s32 ret;` + shared `return ret;` | 17 | 94 | `ret` still lands in $s0; +1 insn at merged epilogue |
| trailing post-switch `return 0;` removed entirely | 18 | 87 | cc1 drops the default-arm; -6 insns vs target; UB at C level |

### Revised priorities for session 9

Per session 8's mechanism reframing:

1. **NEW PRIORITY 1**: Dump the FULL allocno table from cheat-free
   compilation (session 4's ALLOCDBG only captured one allocno).
   Identify which pseudo holds the return value, its livelen, refs,
   and why global.c picked $s0. This is the direct empirical evidence
   the prior priority list lacked.
2. **NEW PRIORITY 2**: Try C-source restructures specifically targeting
   the unified-return-pseudo problem:
   - Narrow the function's return type (`s8` instead of `s32`).
   - Move `D_800A37B8++` INTO each case body.
   - Try `return (volatile s32)0;` per-case (vet against cheats lens).
3. **DEMOTED**: BB2_FLOW_DEBUG (formerly priority 1) — flow.c live-set
   isn't the actual bottleneck; the expand_return pseudo allocation is.
4. **KEPT**: Sibling LREG/GREG dumps for calibration of allocno-priority
   thresholds.

## Anti-priorities (carried across sessions)

The rejected_forms cluster:
- DO NOT re-test syntactic variants of case 2's test order (== 0 vs != 0,
  else-if vs flat if, named locals). All collapse to the same RTL.
- DO NOT inline done in case 2 (loses the j-delay-slot pair).
- DO NOT collapse case 4/5/6 with internal dispatch (+4 insns).
- DO NOT reach for `do { } while (0);` until BB2_ALLOC_DEBUG + permuter
  exhausted, per [[do-while-zero-exception]]'s lever-exhaustion prerequisite.
- DO NOT re-test switch-with-break + accumulator forms (session 8: $s0
  still wins, +1 insn merged epilogue, score 17).
- DO NOT remove the trailing `return 0;` (session 8: UB at C, -6 insns,
  score 18).
- DO NOT prioritize BB2_FLOW_DEBUG over allocno-table investigation —
  the gap is the return-pseudo allocation, not flow.c live-set propagation
  at .L8003AC74.

## Session 9 (2026-06-10) addendum — SESSION 8's MECHANISM REFRAMING DISPROVEN

Applied candidate.c, reconfirmed clean floor 6 (build_insns=target_insns=93,
rules_dropped=5, cheat_asm_stripped=6).

**Session 8's $s0-accumulator claim is empirically WRONG.** Re-ran
index-aligned objdump diff (`tmp/ab44_diff.sh`) of canonical
`build/src/code6cac_c_ab.o` vs cheat-free
`tmp/sandbox/func_8003AB44/code6cac_c_ab.o`. Both builds:
- Use a 24-byte frame with ONLY `sw $ra, 16(sp)` save.
- Have NO `sw $s0` save, NO `move $s0, $zero` init, NO `lw $s0` restore.
- Emit DIRECT `move $v0, zero` / `li $v0, IMM` per-case in j-delay slots
  (never `move $v0, $s0`).
- Have the SAME epilogue layout (`lw $ra, 16(sp); addiu $sp; jr $ra; nop`).

Session 8 likely measured FORM #16's `s32 ret;` accumulator variant (which
DID introduce $s0 and regressed +11 to score 17 as session 8 documented)
and mistakenly attributed FORM #16's emission to the candidate baseline.

**The actual gap is what sessions 2-7 documented all along:** case 2
polarity (4 diffs at idx 46-49) + case 5/6 register choice (3 diffs at
idx 76, 78, 79). Masked-Lev distance 6 = the 7-slot raw diff masked to 6
due to one slot's adjacent-context overlap.

### Levers tested this session (2 new rejections)

| Variant | Score | build_insns | Notes |
|---|---|---|---|
| candidate baseline | 6 | 93 | reference floor (re-confirmed) |
| narrow return: `s8 func_8003AB44(void)` | 6 | 93 | ABI promotes returns to SImode |
| case 3 BEFORE case 2 + `done:` after case 2 (FT) | 15 | 93 | jump2 cross-jump-merge unfavorable; case-order direction matters |

Lever (c) `return (volatile s32)0;` per-case NOT tested — volatile cast on
literal rvalue is cheat-by-any-spelling per [[no-new-park-categories]] /
[[inline-asm-policy]] expanded catalog (zero semantic content, exists
purely to influence cc1's RTL emission of the return path).

### Session-10 priorities (REVISED per session 9 correction)

1. **BB2_FLOW_DEBUG re-promoted to PRIORITY 1**: instrument cc1's flow.c
   to dump `basic_block_live_at_start[bb_for(.L8003AC74)]` after
   `update_life_info`. This is the diagnostic sessions 6+7 named PRIORITY
   1; session 8 demoted it based on its (wrong) $s0 reframing. Session 9
   re-promotes it as the strongest remaining untried lever-finding map
   for the case 5/6 pseudo 94 → $v1 issue.
2. **BB2_REORG_DEBUG dump targeting case 2 invert-jump candidate site**.
   The instrumented cc1 already has DBG_FILL / DBG_LA / DBG_BRK / DBG_WLK
   / DBG_MTLR / DBG_BBLAS hooks from saEft00Add sessions. Run on
   isolated func_8003AB44 .i and compare INSN_PRIORITY chains for the
   case 2 branch vs sibling matched switches.
3. **Permuter sweep targeting case 2 polarity specifically** (vs session
   6's full-candidate sweep which mixed both clusters). PERM_GENERAL
   macros restricted to case 2's local mutations may find a structural
   variant that flips polarity without case 5/6 regression.

### Anti-priorities (updated)

- DO NOT trust session 8's $s0-accumulator framing. It's empirically false.
- DO NOT re-test `s8` return type (session 9: no effect).
- DO NOT re-test source-order flips at case-position level — sessions 5
  (case 5/6 before case 4: no effect), 7 (case 7 before case 4/5/6:
  regression), 9 (case 3 before case 2 + done FT: regression) collectively
  cover the meaningful permutations; only specific directions matter and
  most are unfavorable.
- DO NOT reach for `(volatile s32)0;` return-cast — cheat-by-any-spelling.

## Related rules

- [[switch-vs-ifchain-branch-sense]] — sibling pattern (rewrite as real
  switch retired regfix rules); applied here but didn't help because
  we already use a switch
- [[register-alloc-pure-c]] — pure-C levers for register allocation issues
- [[store-before-jal]] — relevant for understanding why GCC picks
  particular delay-slot fills
- [[inline-asm-policy]] — the rule classifying `__asm__("" ::: "memory")`
  as cheat-asm (the 3 barriers I removed are in this category)
- [[cross-jump-store-tail-merge]] — explains why inlining done in case 2
  drops the `j done; addiu $a0,3` delay-slot pair (jump2 cross_jump merges
  the duplicated done-body suffix back into one block)
- [[do-while-zero-exception]] — TESTED session 10, NEGATIVE at two scopes
  (entire case 2 body wrap, and tightest if-bnez wrap). The lever is now
  closed for THIS function's case 2 polarity gap; the polarity is
  INSN_PRIORITY-driven, not LABEL_OUTSIDE_LOOP_P-driven.

## Session 10 (2026-06-10) addendum

Per session 9's anti-priority "DO NOT reach for do-while-zero until
BB2_ALLOC_DEBUG + permuter exhausted", BOTH prerequisites are now
satisfied (session 4 + session 6). Tested 3 new structural variants:

| Variant | Score | Notes |
|---|---|---|
| `do { ...case 2 body... } while (0);` (wide scope) | 6 | No effect — peephole still fires |
| `do { if (call==0) return 0; } while (0);` (tight scope) | 6 | No effect — same |
| `s32 state = D_800A38AC; switch(state); ... case 5/6: D_800A38AC = state + 1;` | 14 | Regression — state spills callee-save, increment still uses local reg |

Closes the do-while-zero hypothesis for this function. The case 2
polarity gap is rooted in INSN_PRIORITY chain depth (per session 9's
mechanism note); the loop-note approach doesn't move it. The
state-keep-alive lever for case 5/6 fails because state's live-range
crossing func_8003A308's call forces callee-save promotion, and case
5/6's increment is a memory RMW that doesn't naturally reuse the
dispatch's register.

### Session-11 priorities (RE-CONFIRMED from session 9, unchanged after session 10)

1. **BB2_FLOW_DEBUG instrumented cc1** — still the highest-value
   diagnostic. Needs flow.c modifications + rebuild. Out of session
   10's budget; remains untried.
2. **BB2_REORG_DEBUG with relax_delay_slots-specific instrumentation
   targeting case 2's invert-jump site** — partially exists from the
   saEft00Add session (DBG_FILL / DBG_LA / DBG_BRK / DBG_WLK / DBG_MTLR
   / DBG_BBLAS hooks); needs targeted dump on func_8003AB44's case 2.
3. **Sibling LREG/GREG dumps** — un-tried (session 3 ruled out by
   syntactic inspection only).
4. **Case-2-specific PERM_GENERAL permuter** — un-tried; would need a
   permuter-base mutating only case 2's local structure.

### Updated anti-priorities (session 10)

- DO NOT re-test do-while-zero scopes at OTHER positions in case 2.
  Two scopes tested negatively; the mechanism is not LABEL_OUTSIDE_LOOP_P.
- DO NOT re-test state-keep-alive levers — the call in case 1 forces
  callee-save spill that regresses far more than the case 5/6 register
  flip could save.

## Session 11 (2026-06-10) addendum

Applied candidate.c, reconfirmed clean floor 6 (build_insns=target_insns=93,
rules_dropped=5, cheat_asm_stripped=6). Tested two structural variants
NOT in sessions 1-10's rejected_forms list; both negative.

| FORM | Lever | Score | build_insns | Verdict |
|------|-------|-------|-------------|---------|
| #23 | Case 5/6 `D_800A38AC = D_800A38AC + 1;` (assignment vs compound `++`) | 6 | 93 | NO-OP — identical RTL gimplification |
| #24 | Lever C precompute: `u8 next_dispatch = D_800A38AC + 1;` at fn entry; case 5/6 uses `D_800A38AC = next_dispatch;` | 18 | 94 | REGRESSION — +1 emit + callee-save spill across dispatch's calls |

### Session 11 mechanism findings

**FORM #23 closes the syntactic-increment-variation hypothesis.** cc1 2.7.2
lowers `PREINCREMENT_EXPR(x)` and `MODIFY_EXPR(x, PLUS_EXPR(x, 1))` to
identical `(set x (plus x 1))` RTL at gimplification, well before global.c's
allocation. The C-level distinction is erased and cannot be a lever.
Useful for future agents: don't re-test `x++` vs `x = x + 1` vs `++x` vs
`x += 1` — all produce identical RTL for unsigned scalars.

**FORM #24 closes the Lever-C whole-function precompute hypothesis for
this function's case 5/6 register gap.** This was a legitimate lever per
[[register-alloc-pure-c]] (Lever C paid off for InitHiraRmd_80047FBC),
and the +1 computation at entry was hypothesized to give the case 5/6
increment value a long live range that exists BEFORE .L8003AC74 (the
jtbl-merge block whose live-in $v0 excludes pseudo 94 per session 4's
ALLOCDBG). Empirically: the value DOES get a long live range — but the
dispatch crosses 6 function calls (func_8003A308 in case 1, gpu_SetDispMask
in case 1, two func_8008C464 calls in cases 2/3/done, func_8003A39C in
fail, func_8003A360 in case 7) which force the live value to a callee-save
register (cost: 2 prologue/epilogue saves) AND the +1 computation itself
adds 1 insn at function entry (cost: +1 emitted insn). Net regression
+12 over the floor-6 baseline.

The Lever-C result is the more useful of the two findings: it
**definitively rules out the whole "extend the increment value's live
range to predate .L8003AC74" lever family** for this function. Any C
construct that makes the case 5/6 increment value live across the
dispatch will pay the same callee-save spill cost — that cost
mechanically exceeds the +1 insn the register flip could save. Future
agents should NOT re-test this lever family in any variant (loop-local,
named-block-local, parameter-shaped, externalized state machine).

### Next-session priorities (session 12 onwards, unchanged from session 10's list)

Per Session 11's negative results narrowing the lever space further, the
priority list shifts ZERO entries — session 10's diagnosis stands. The
remaining genuinely-untried levers are all diagnostic-instrumentation
based:

1. **BB2_FLOW_DEBUG cc1 instrumentation** (still PRIORITY 1). The case
   5/6 issue is now triply confirmed (sessions 4, 10, 11) to be rooted at
   flow.c's basic_block_live_at_start computation at .L8003AC74.
   BB2_FLOW_DEBUG would directly read this live-set and identify which
   predecessor's live-out propagates $v0. The instrumented cc1 already
   exists from prior cluster sessions (`tmp/gccdbg/cc1` with ALLOCDBG +
   SCHEDDBG + PRIODBG hooks); adding flow.c hooks is a ~30-50 line patch
   + rebuild. The canonical `tools/gcc-2.7.2/build/cc1` stays UNTOUCHED.
2. **BB2_REORG_DEBUG targeting case 2's invert-jump site** for the
   polarity gap (sessions 2-9). Partial DBG_* hooks exist; targeting
   `mostly_true_jump` / `relax_delay_slots` at the case 2 candidate site
   has not been done.
3. **Sibling LREG/GREG dumps** from code6cac_c_ab.c's matched switches.
   Session 3 ruled them out by SYNTACTIC inspection; actual RTL dumps
   may surface allocno-priority patterns that transfer.
4. **PERM_GENERAL permuter scoped to case 2's local mutations.** Session
   6's permuter mutated the full candidate; a permuter-base restricted
   to case 2 only may find a structural variant for that polarity gap
   without regression in case 5/6.

### Anti-priorities (added session 11)

- DO NOT re-test compound-vs-assignment syntactic variations of any
  increment in this function. All produce identical RTL.
- DO NOT re-test ANY Lever-C-style whole-function precompute for the
  case 5/6 increment value. The dispatch's call density forces
  callee-save promotion of any cross-dispatch-live value at cost
  greater than any allocation flip could save.
