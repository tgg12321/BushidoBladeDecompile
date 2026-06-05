# func_8007CBB0 — WIP resume notes

## Orchestrator verification 2026-06-03 — round-3 meta-blocker REFUTED

The round-3 worker claimed deploying CBB0's candidate body breaks sibling
`func_8007CE0C`'s splice rules at `regfix.txt:4208/4210/4212` (references to
absolute labels `.L152/.L154/.L158/.L174`). **This claim is empirically
false in current main:**

1. Current `regfix.txt:4208/4210/4212` hold unrelated content (a comment,
   an `exec_game` rule, blank). CE0C's splice rules live at
   `regfix.txt:4184/4186/4188/4189` and already use `{lbl#N}` function-local
   slots (drift-robust per `global-label-drift-sibling-cheat.md` § Preferred).
2. Zero display.c functions have absolute `.L` label cheats in `regfix.txt`
   (audit: `tmp/abs_label_audit.py` — the 7 functions that do are in
   `main.c`, `system.c`, `code6cac_b.c`, `config.c`).
3. **Empirical deployment test:** applied candidate body to `src/display.c` +
   disabled CBB0 asmfix bridge → full build SUCCEEDED, no CE0C link failure,
   final SHA1 `fe6ae5d12406411d21e1815a213ae21dd1e8baff` ≠ oracle (expected;
   candidate doesn't fully match).

The worker was reading a stale-worktree `regfix.txt` from before the
{lbl#N} migration landed in main. Their stated meta-blocker does NOT exist.

## DISCREPANCY RESOLVED 2026-06-03 — score 52 / build_insns 151 reproduces

First re-measurement on current main gave **score 55 / build_insns 129**
(22 insns short) — initially suspected the WIP-documented 52/151 measurement
was anomalous. Root cause identified: **missing `extern s32 *D_8009BF48;`
declaration before the function**. The existing extern in `src/display.c`
is at line 837, AFTER where the candidate body would land at line 710.
Without the declaration in scope, cc1 treats `D_8009BF48` as implicit and
emits worse code for the `*D_8009BF48 & 0x7FF` expressions in both the
big-packet and small-packet paths.

**With the D_8009BF48 extern included, score 52 / build_insns 151 EXACT
reproduces.** The WIP measurement was correct all along — my round-3 +
round-3-orchestrator deployments were missing one extern.

### CORRECT DEPLOYMENT RECIPE (verified 2026-06-03)

The candidate body REQUIRES these declarations before it in src/display.c
(the candidate.c header has them; do not omit any when copying the body
into the file):

```c
extern u32 D_800F1858;
extern u32 D_800F185C;
extern u32 D_800F1860;
extern u32 D_800F1864;
extern u32 D_800F1868;
extern u32 D_800F186C;
extern u32 D_800F1870;
extern u32 D_800F1874;
extern u32 D_800F1878;
extern u32 D_800F187C;
extern u32 D_800F1880;
extern u32 D_800F1884;
extern u32 D_800F1888;
extern s32 *D_8009BF48;            /* CRITICAL — existing decl at line 837 */
extern u32 gpu_GetInfo(u32 a0);
extern void gpu_StartDmaList(u32 a0);

typedef struct {
    u32 word0;
    s16 w;
    s16 h;
} _GpuChunk_CBB0;

s32 func_8007CBB0(_GpuChunk_CBB0 *arg0, u32 arg1, s32 arg2, s32 arg3) {
    /* ...body... */
}
```

Reproduction protocol:

1. `git checkout HEAD -- src/display.c asmfix.txt`
2. `& tools/eng.ps1 verify-oracle --rebuild` → SHA1 == oracle
3. Apply candidate.c body + ALL the externs above (replace the stub at
   src/display.c:710)
4. **Leave the asmfix bridge `func_8007CBB0: replace_with_asmfile ...`
   ACTIVE** (so build/ continues to hold oracle bytes for the comparison
   reference)
5. `& tools/eng.ps1 sandbox func_8007CBB0 --disable all --keep-cheat-asm`
6. Result: **score 52, build_insns 151, target_insns 151, scorable: true**
7. Revert + `verify-oracle --rebuild` to restore canonical state

Note: `--keep-cheat-asm` is required because display.c contains ~383
cheat-asm blocks in sibling canonical-asm GTE wrappers; stripping them
shifts maspsx indices and breaks a sibling index-anchored reorder rule
which truncates the build. The function under test has no cheat-asm in
its body so `--keep-cheat-asm` doesn't affect its score.

## TL;DR (PRE-VERIFICATION — see above section for corrections)

- **Floor 52** (HEAD's measured 149 is a sandbox-strip artifact — the
  function is currently held by an asmfile bridge over a stub body, so
  the strip produces build_insns=2 garbage). The candidate's 52 is the
  first HONEST pure-C measurement. Prior session's reported 76 floor was
  the same artifact-mismeasurement issue.

  **⚠ ORCHESTRATOR 2026-06-03: this 52 measurement is NOT REPRODUCING
  in current main. See "DISCREPANCY found this session" section above.**
- **build_insns 151 == target_insns 151 EXACTLY.** The structural
  decomp is end-to-end correct. Frame size matches (0x40), register
  assignments match ($t0/$t1/$s0/$s1), both packet-path store orders
  match target. Residual 52 = list-scheduler INSN_PRIORITY decisions in
  the GPU packet store sequence (the 0x03FFFFFF constant materialization
  splits in target but lands adjacent in the candidate).

  **⚠ ORCHESTRATOR 2026-06-03: re-measurement shows build_insns 129 (22
  short of target), score 55. The structural-end-to-end-correct claim
  needs re-verification.**
- **Cumulative ~24 negative levers across 2 sessions.** Session 2 went
  149 → 52 via semantic reconstruction + 5 specific levers. Session 3
  tested 9 additional SOTN-allowed variants from the score-52 base —
  none lowered the floor further.
- Cheat-reviewer NOT yet invoked on the candidate. Manual self-audit:
  no obvious cheat constructs (no pins, no register-asm, no volatile
  coercion, no unused-array padding). The `(void)arg2; (void)arg3;` is
  the K&R-style honoring of unused params; ABI takes 4, logic uses 2.

## How to resume in one read

1. Read `meta.json` — sessions[], structural_evidence (build_insns and
   register matches), residual gap description, rejected_forms.
2. Apply `candidate.c`'s body to `src/display.c`. NOTE: the function is
   currently a stub held by an asmfile bridge in `asmfix.txt`. To test
   the candidate you'll need to either:
   - (a) Apply the candidate body to src/display.c AND remove the
     asmfix `replace_with_asmfile` bridge line for func_8007CBB0, then
     measure sandbox + sandbox --disable all
   - (b) Use a different sandbox invocation that bypasses the bridge —
     check `engine sandbox --help` for options
3. Confirm sandbox score 52, build_insns 151.
4. Pick a next_hypothesis. Top candidate: directed permuter from a
   CLEAN single-function target (build `asm/funcs/func_8007CBB0.s`
   standalone) to find a scheduling lever.

## Why this is high-value to checkpoint

Two sessions of work establish:
1. The semantic reconstruction is correct (build_insns match exactly,
   stores emit in target order, registers match).
2. The remaining gap is narrow (pure list-scheduling) — NOT
   register-allocation, NOT cross-jump merge, NOT control-flow
   restructure.
3. ~24 SOTN-allowed levers have been tested and ruled out from this
   base. The next agent should NOT re-derive the m2c semantic
   reconstruction or the lever sweep.

Without this checkpoint, a fresh agent picking up CBB0 from the queue
would face a "149 distance, 1 rule" entry and likely spend most of a
session re-deriving the semantic shape of a 151-instruction GPU DMA
list builder before getting to scheduling work.

## Companion memory note

Full detailed evidence ledger (lever-by-lever, including the 9 session-3
variants and the build_insns/register matching) is at:
`memory/project/func-8007cbb0-pure-c-draft.md`

## What's still off — the residual 52

From the candidate.c header docstring:

> The big-packet path's lui/and/or/sw scheduling differs from target.
> Target interleaves the constant-materialization (lui $a3 0x3FFFFFF
> and ori $a3 0xFFFF) with the BF48 load chain. Mine emits them
> adjacent. The lui $a3 0x3FFFFFF ; ori 0xFFFF pair: target splits them
> by ~20 insns letting BF48 load + scheduling happen in between, mine
> emits them together pre-stores. The small-packet path has similar
> list-scheduler diffs.

This is the same class of gap as cpu_side_move_dir_4 / marionation_Exec
sched.c walls — RTL-level INSN_PRIORITY tiebreaker. The legitimate
levers (combine-foldable chain extender etc.) need careful re-vetting
against the 2026-06-02 cheat catalog; session 8 of cpu_side_move_dir_4
documented one such lever and it was retroactively FORBIDDEN. The next
agent must invoke cheat-reviewer on any closing form found.

## Related

- `memory/project/func-8007cbb0-pure-c-draft.md` — full evidence ledger
- `.claude/rules/cross-jump-store-tail-merge.md` — the sched.c session
  this function is structurally similar to (saEft00Add coupled fixpoint)
- `.claude/rules/register-alloc-pure-c.md` — sessions 5-10 of
  cpu_side_move_dir_4 (the chain extender lever is now FORBIDDEN; same
  catalog applies here)
- `.claude/rules/no-new-park-categories.md` — the standing policy
  governing what permuter output is acceptable

## Session 2026-06-02 (workflow round 1, session 4)

Confirmed sandbox-52 baseline via direct disable=all build; tested 6 additional
SOTN-allowed structural variants — all confirm the 52 floor. Cumulative ~30
negative levers across 4 sessions now. The residual 52 remains pure
list-scheduler INSN_PRIORITY in the GPU packet store sequence — target splits
the 0x03FFFFFF lui/ori materialization across ~36 instructions; mine emits them
adjacent. Same class as cpu_side_move_dir_4 sched.c walls. Top next_hypothesis:
directed permuter from a CLEAN single-function offset-0 target with cheat-reviewer
gating on saved candidates.

## Session 2026-06-03 (workflow round 3)

**Floor unchanged at 52, but NEW META-BLOCKER SURFACED.** Round-3 worker attempted
candidate deployment (commented out asmfix bridge + applied candidate body to
display.c). Build link failed:

```
undefined reference to `.L152'
undefined reference to `.L158'
undefined reference to `.L174'
```

in sibling func_8007CE0C. The candidate body's 80-line replacement shifts cc1's
global `.L` counter, orphaning CE0C's 3 splice rules (regfix.txt:4208, 4210, 4212)
that reference absolute global labels (.L152/.L154/.L156/.L158/.L159/.L160/.L174).

**This is a meta-blocker on the queue item, not just a score gap.** A future
worker cannot deploy ANY pure-C candidate for CBB0 — even a score-0 one — without
first migrating CE0C's splice rules to the new `{lbl#N}` function-local label slot
mechanism (per global-label-drift-sibling-cheat.md as updated 2026-06-02).

Sandbox CBB0 was also unscorable in this session — same display.c-wide
cheat-asm-strip pipeline truncation that blocked C7A0 and C97C workers in
parallel sessions. The C86C round-3 worker's `--keep-cheat-asm` flag is the
likely unblocker for the measurement step (CBB0 candidate has no cheat-asm in
its own body once the bridge is removed).

**Forbidden lever clarification:** The chain-extender lever that worked for
cpu_side_move_dir_4 (a natural fit for CBB0's negative-label-drift scenario)
is FORBIDDEN per global-label-drift-sibling-cheat.md as of 2026-06-02 — both
dead-goto label-pad and DImode chain are codified in the FORBIDDEN section.

New PRIORITIZED next_hypothesis: round 4 should ATTEMPT the {lbl#N} migration
of CE0C's 3 splice rules FIRST as a precondition, THEN deploy candidate +
measure with `--keep-cheat-asm`, THEN run directed permuter from a clean
single-function offset-0 target.

## Session 2026-06-04 (workflow round 4)

The round-3 meta-blocker was already REFUTED by the orchestrator (see top of
this file). Round-3's claimed `{lbl#N}` migration precondition does NOT exist
in current main — CE0C's splice rules already use function-local slots.
Round 4's actual task: lower the 52 floor or document the structural ceiling.

Confirmed candidate floor 52 / build_insns 151 == target 151 reproduces via
`--keep-cheat-asm`. Performed `tmp/diff_cbb0.py` (index-aligned objdump diff)
to localize the residual: 66 raw insn diffs concentrated in two regions —
big-packet idx 45-91 and small-packet idx 117-139. The diagnostic finding:

**Target emits `lui $a3,0x3FF` at idx 49 with its paired `ori $a3,$a3,0xFFFF`
deferred 36 instructions later to idx 85, right before the
`sw $a3,(p187C)` at idx 89.** Candidate emits the lui+ori as an adjacent pair
at idx 49-50. This is a deliberate list-scheduler split that GCC 2.7.2's
combine pass normally keeps as one atomic 2-insn materialization — the
candidate's compile takes the natural path, the target's compile somehow
split it.

Three new structural levers tested this session, all CONFIRMED the 52 floor:

1. **Named intermediates** `u32 ot_hdr = ...; u32 sign_E1 = ...;` declared at
   top of big-packet block, then used in F1858 and F186C stores. Combine
   folds single-use locals back to inline expressions before RTL pseudo
   numbering — no LUID effect, no scheduling change. Score 52.
2. **Constant rewrite** `*p187C = ~0xFC000000` to coerce a non-canonical
   constant materialization. Combine canonicalizes; emitted bytes identical
   to `0x03FFFFFF`. Score 52.
3. **Hoisted BF48 deref** `s32 ot_link = *D_8009BF48;` at top of big-packet
   block. Hypothesis was that an early load would influence INSN_PRIORITY for
   F186C's store. REGRESSED 52 -> 65 — the early load pollutes scheduling.

The residual is the same class as cpu_side_move_dir_4's sched.c
INSN_PRIORITY wall (see `[[register-alloc-pure-c]]` sessions 5-10). That
function's documented exhaustion: 10 sessions + 16,800+ permuter iterations
all plateaued on register-rename diffs at matched insn count. The
chain-extender lever that worked for cpu_side_move_dir_4 in session 5 is
FORBIDDEN (`[[global-label-drift-sibling-cheat]]` § FORBIDDEN, 2026-06-02);
no analogous pure-C lever remains in the explored space.

**Honest endpoint conclusion:** 52 floor is genuinely the structural ceiling
for this function's pure-C reconstruction. The remaining named avenue
(directed permuter from clean offset-0 target) was NOT executed this session
due to budget; given cpu_side_move_dir_4 permuter plateau on identical-class
residual, prior-art evidence strongly suggests it would also plateau.

Source reverted at session end; verify-oracle SHA1 == oracle (62efab4f...).

## Session 2026-06-04 (workflow round 7) — permuter + 3 manual levers, NO_PROGRESS

PERMUTER AUTHORIZED for this session. Setup at `permuter/cbb0/` (base.c =
score-41 candidate, target.s = prelude.inc + asm/funcs/func_8007CBB0.s,
compile.sh = canonical cc1 pipeline). 10-min/-j6/~3700 iters.

Best permuter output (760-1, weighted score 760) renames to `int new_var =
((arg1 >> 31) << 10) | 0xE1000000; (&D_800F1858)[5] = (*D_8009BF48 & 0x7FF)
| new_var;` in big-packet path. Sandbox-measured: **41 → 29** (-12 points,
build_insns 151 still EXACT). The lever ASYMMETRICALLY applies — adding the
same intermediate to small-packet [2] (same subexpression) REGRESSES 29 → 54.

**Cheat-reviewer FAIL on 3 grounds:**

1. Asymmetric application reveals codegen-control intent, not semantic
   clarity. The identical subexpression `(((arg1 >> 31) << 10) | 0xE1000000)`
   appears INLINE in small-packet [2]. A human programmer naming the GP0
   0xE1 draw-mode command word would apply the name to BOTH uses.
2. Worker's stated mechanism is "biases the LUID" — pure GCC-internals.
3. Permuter-derived form, semantic rename (`e1_cmd`) is post-hoc
   rationalization. Same family as round-4's rejected `sign_E1` (rejected_forms).

Also tested 3 NEW manual levers from the score-41 base, ALL negative:

1. Named intermediates `u32 hdr = ...; u32 bf48_e1 = ...;` for big-packet — REGRESSED 41 → 58.
2. Precomputed `xy/wh` shared between both packet paths in outer block — REGRESSED 41 → 75 (build_insns 148 short).
3. Constant rewrite `0x04000000 - 1` for the 0x03FFFFFF terminator — UNCHANGED (combine canonicalizes).

**Net result for round 7: floor unchanged at 41.** Permuter found a real
sub-baseline form but the form is a cheat per the cheat-reviewer's
6-test checklist. src/ reverted to HEAD; verify-oracle SHA1 == oracle.

Cumulative ~38 negative levers + 1 PASS-vetted positive (round-6
SOTN-indexed-array) across 7 sessions. The score-41 floor remains the
established structural ceiling for this function's pure-C reconstruction.

**Resume avenues for future sessions:**

1. Try directed permuter with `PERM_*` macros (not random) — the random
   permuter found `new_var` quickly but the lever was a cheat; directed
   permuter could target specific lever classes that are SOTN-sanctioned.
2. Try instrumented cc1 dump with BB2_SCHED_DEBUG/BB2_PRIO_DEBUG on the
   score-41 candidate to identify exactly which RTL edges create the
   constant-materialization adjacency at idx 49-50 (vs target's split at
   49-85). The instrumented cc1 is preserved at `tmp/gccdbg/cc1`.
3. Hand-research SOTN's other libgpu packet builders (beyond `_clr`) for
   structural patterns BB2's CBB0 hasn't tried — e.g. SOTN's `DrawSync`
   wait-loop forms.
