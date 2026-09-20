# func_800747D8 — evidence

## Provenance of candidate.c (2026-09-20, operator salvage)

`candidate.c` was recovered from an **uncommitted working-tree edit** to
`src/text1b.c` found at session start (prior session ended without banking it).
It was reverted out of `src/` per [[asm-until-matched]] — `main` carries
`INCLUDE_ASM("asm/funcs", func_800747D8);` — and banked here intact.

**Measured, not claimed** — run by the operator while the body was still live in
`src/text1b.c`, before the revert:

```
engine sandbox func_800747D8 --disable all
{"score": 6, "target_insns": 208, "build_insns": 208, "scorable": true,
 "cheat_asm_stripped": 132, "rules_dropped": 0}
```

- **Honest pure-C distance: 6** (queue's pinned floor was 208 = the bare
  `INCLUDE_ASM` baseline, so the pin is stale-high — re-measure on first session).
- `build_insns == target_insns == 208` — instruction *count* already matches;
  the residual 6 is spelling/allocation, not missing or extra work.
- `cheat_asm_stripped: 132` is **file-wide** (other `text1b.c` functions), not
  this body. Scanned: the banked block contains no `__asm__`, no register pins,
  no scheduling barriers — it is pure C.

## Caveats for the first session

- This candidate was **never cheat-reviewed and never self-vetted** — no
  `self_vet.md` exists. Its constructs have not been cleared by the Judge.
  Treat the spelling as unaudited: re-measure it, then vet it before submitting.
- The struct `S_800747D8` and the `MENU_800747D8` macro are this candidate's
  own invention (declaration puns over `D_800A36A0`), not established project
  types — they are exactly the kind of construct the Judge scrutinizes.
- No `state.json` yet; the pre-existing `pre-include-asm-body.c` (2026-08-26)
  predates this work.

## Session 1 (recon) — re-measured, classed the diff, one confirmed sub-residual attack, two instance kills

**Re-measurement.** Applied `candidate.c` verbatim to `src/text1b.c` in place
of the `INCLUDE_ASM` line. First build attempt FAILED
(`conflicting types for D_800A35DC`: candidate declared `extern u8
D_800A35DC;`, but the file's own already-matched, already-COMPLETED-C
function at line ~8974/9120 (the `func_80077374`-adjacent body) declares
`extern s8 D_800A35DC;` and stores signed values through it). Fixed the
candidate's declaration to `extern s8 D_800A35DC;` to match the established
canonical type for this TU — this is not a codegen lever, it's a plain type
correction to what the rest of the file already committed. With that one-line
fix: `sandbox func_800747D8 --disable all` → **score 6, build_insns 208 ==
target_insns 208** — confirms the evidence.md floor-6 claim exactly.

- **OBJECT MODEL:** both DATA MODEL signals MATCH the candidate's existing
  declarations, verified against use sites in OTHER functions of this same
  TU (not guessed):
  - `D_8009BD20`: candidate declares `extern u8 D_8009BD20[][2];`. Confirmed
    against `asm/funcs/func_80074488.s:150-160` (sibling reader in the same
    file): index = `(row << 1)`, immediately followed by a second read at
    `D_8009BD21` (the `+1` column) — i.e. row-major stride-2 byte pairs,
    exactly the `[][2]` shape. Our target function's own asm
    (`asm/funcs/func_800747D8.s:154-158`) computes `row<<1 + parity` the
    same way. MATCHES, measured (score 6 achieved with this declaration in
    place; no residual hunk in the diff touches this access).
  - `D_800A35D0`: candidate declares `extern s16 D_800A35D0;` (scalar). Every
    reference to this symbol across the whole TU (this function,
    `func_80075F80`, `func_800768DC`, plus the two other same-symbol call
    sites at src/text1b.c:8770/8784 already COMPLETED-C) takes its ADDRESS
    only (`&D_800A35D0`, `lui/addiu %hi/%lo`) and never dereferences it in
    this TU — it's passed as opaque state storage to `func_800692C0`. A
    scalar vs. array declaration is address-identical at every one of these
    call sites (same two-insn `lui+addiu` regardless of element count), so
    MISMATCH-unmeasured is moot here: no measurable codegen difference is
    possible from this TU's perspective. Left as-is.
  - `jtbl_80015A0C`: has no C declaration and is not referenced by name in
    any TU — it is GCC-synthesized per the state switch (field3C, values
    0-4) rather than hand-placed. Not a declaration-fix candidate.

**Diff classification** (`sandbox --disable all --diff`, 30 hunks total, 208
insns both sides):
  - 26 not-scored (masked branch/jump target address artifacts — correctly
    not chased).
  - 2 operand-only (real register/table-offset seats):
    - hunk 9 (insn idx 66): target `lw v0,0(at)` vs ours `lw v0,24(at)` — the
      state-switch (field3C ∈ [0,4]) jump-table load. A 24-byte (6-word)
      table-offset difference; GCC's synthesized jump table for our `switch
      (state)` differs from target's in a way the score does NOT mask (this
      is a raw immediate operand, not a relocation the scorer strips).
      UNEXPLAINED this session — flagged to the frontier, not attacked (see
      below).
    - hunk 17 (insn idx 99): target `lbu v0,100(v0)` vs ours `lbu a0,100(v0)`
      — register choice for the `field64` load feeding the `selection_sound`
      block (see next).
  - 2 source-level (the C says something different; NOT reg-alloc/scheduling
    territory):
    - hunks 18-19 (insn idx 101/103): target keeps the loaded test value in
      `$v0`, branches on it (`beqz v0,...`), and sets the call-arg `$a0`
      SEPARATELY in each arm (delay-slot literal `4` on the taken path,
      explicit `addu a0,zero,zero` on the fallthrough). Our build folds the
      load directly into `$a0` and tests `$a0` itself
      (`bnez a0,...; move a0,zero`) — a genuinely different value-flow
      shape, not just a register rename.

**Attack on hunks 17-19 (the `selection_sound` block, candidate.c line
~8475-8481, baseline spelling `s32 sound = field64; if (sound==0) sound+=4;
else sound-=sound;`).** Tried two natural two-local respellings that make the
test value and the result value visibly distinct C objects (matching the
apparent v0-vs-a0 structural split in target):
  1. `s32 flag = field64; s32 sound; if (flag==0) sound=4; else sound=0;`
     → **score 10, build_insns 205** (3 insns SHORT of target — worse on
     both axes). Banked: `rejected/selection_sound-two-var-split.c`.
  2. `s32 flag = field64; s32 sound=4; if (flag!=0) sound=0;`
     → **score 10, build_insns 205** — identical result to (1). Banked:
     `rejected/selection_sound-default-then-override.c`.

Also tried a third respelling, dropping even the `flag` intermediate and
testing the field directly in the condition
(`if (MENU_800747D8->field64==0) sound=4; else sound=0;`, single `sound`
local, declared uninitialized) →  **score 10, build_insns 205** — identical
to both prior attempts. Banked:
`rejected/selection_sound-single-shot-no-intermediate.c`.

All three KILLED as instances (kill_scope: instance; measured_on: this
chassis, `sandbox --disable all`, no FAKE constructs present in any
spelling). The baseline's `sound -= sound;` self-subtract outperforms all
three natural if/else spellings by 4 score points and 3 instructions — worth
noting as a construct to re-examine once the residual is closer to 0 (a
self-subtract instead of a plain `= 0` is not what a human would write from
a spec; it may be load-bearing for now or may be an artifact of an earlier
session's search — not determined this session). The fact that all 3 natural
forms collapse to the identical wrong shape regardless of local-variable
count/placement is itself evidence: whatever GCC pass produces the 205-insn
fold doesn't key off source-level variable structure here the way the
baseline's self-subtract avoids it — worth reading the `.combine`/`.cse`
dumps before hand-writing a 4th spelling blind.

**Frontier for next session:** (1) hunk 9's jtbl table-offset residual (24
vs 0) is unexplained — read the `.combine`/`.jump2` cc1 dumps
(`pwsh tools/grinder/dump.ps1 func_800747D8`) for the state-switch region to
see whether GCC is building a jump table with a different base/entry-count
than target's 5-entry table, or reordering case labels; (2) hunks 17-19 —
3/3 natural if/else respellings measured identically wrong (score 10,
build_insns 205); next step is dump-driven (`.combine`/`.cse`), not another
blind hand-spelling — figure out WHY `sound -= sound` (self-subtract)
reaches 208 insns while every plain `= 0`/`= 4` assignment reaches only 205,
before trying a 4th form.

- [s1] Re-applying memory/grind/func_800747D8/candidate.c to src/text1b.c (with a one-line D_800A35DC u8->s8 type fix required for the build to succeed) reproduces the banked floor exactly: sandbox --disable all -> score 6, target_insns 208, build_insns 208.

- [s1] sandbox --disable all --diff classes the 30-hunk diff as 26 not-scored (masked branch/jump address artifacts, correctly not chased), 2 operand-only (hunk 9: state-switch jtbl table-offset 0 vs 24; hunk 17: field64 load lands in v0 for target vs a0 for ours), 2 source-level (hunks 18-19: target keeps the test value in v0 and sets the call-arg a0 separately per branch via a delay-slot literal + explicit zero, while our build folds the load directly into a0 and tests a0 itself).

- [s1] D_8009BD20's [][2] declaration is corroborated by asm/funcs/func_80074488.s (row<<1 stride-2 pair indexing, same-file sibling reader) and by the target function's own asm (row<<1 + parity indexing) - the DATA MODEL signal MATCHES, no fix needed.

- [s1] D_800A35D0 is address-only-used everywhere in this TU (this function, func_80075F80, func_800768DC, two already-matched call sites) so its declared size cannot affect codegen here - DATA MODEL signal is moot for this function.

- [s1] Three natural if/else respellings of the selection_sound block (two-local split, default-then-override, single-shot-no-intermediate) all measured IDENTICALLY at score 10 / build_insns 205, three instructions short of target - while the baseline's sound -= sound self-subtract spelling reaches build_insns == target_insns 208 with the smaller score-6 residual. This 3-for-3 identical-wrong-shape result is evidence the fold is not keyed to local-variable count/placement; next session should read .combine/.cse dumps rather than hand-write a 4th blind spelling.

- [s1] The baseline's sound -= sound construct is flagged (not changed) as a candidate for cheat-smell review later: a human would write sound = 0, not a self-subtract. It currently measures as the best-known honest form and was left untouched pending further narrowing.
