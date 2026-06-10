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

## Next-session priorities (session 4 onwards)

The manual structural lever exhaustion this session shifts the gradient.
The genuine untried levers now are diagnostic-and-derive:

1. **`BB2_ALLOC_DEBUG` on tmp/gccdbg/cc1** for case 5/6 specifically — dump
   the .greg/.lreg + the allocno priority table at the lbu emission point.
   The hypothesis (jtbl-entry conservative live-set on $v0) is concrete
   and testable from the dump. Without the dump, every lever is guess-driven.
2. **`PERM_GENERAL`-directed permuter** from `candidate.c` base — explore
   type-mutation + statement-reorder + non-obvious structural variants the
   hand search missed. Set up `permuter/func_8003AB44/` with the canonical
   target (per [[difficult-is-not-impossible]] § Metric gotchas — use a
   clean single-function `target.o`).
3. **Read `reorg.c` `relax_delay_slots`** to find what specifically triggers
   the invert peephole on case 2's beqz, and what C-source structure could
   change that trigger (e.g. delay-slot dependency chain length, branch
   prediction heuristic). Currently the prediction is informational only —
   no instrumented dump has run on the reorg pass for this function.
4. **Permuter from a sub-optimally-scored base** (e.g. the score-15 form
   from rejected[0]) — sometimes a +9 jump produces structural mutations
   the from-floor-6 permuter can't reach. Untested.

Anti-priorities (the rejected_forms cluster):
- DO NOT re-test syntactic variants of case 2's test order (== 0 vs != 0,
  else-if vs flat if, named locals). All collapse to the same RTL.
- DO NOT inline done in case 2 (loses the j-delay-slot pair).
- DO NOT collapse case 4/5/6 with internal dispatch (+4 insns).
- DO NOT reach for `do { } while (0);` until BB2_ALLOC_DEBUG + permuter
  exhausted, per [[do-while-zero-exception]]'s lever-exhaustion prerequisite.

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
- [[do-while-zero-exception]] — relevant FUTURE-LAST-RESORT lever for the
  case 2 polarity; do not invoke until [[difficult-is-not-impossible]]'s
  diagnostic playbook (BB2_ALLOC_DEBUG + reorg dump + permuter) has been
  exhausted, per the rule's prerequisite
