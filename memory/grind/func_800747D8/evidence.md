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

## Session 2 (structural) — 2026-09-20

**Re-measurement.** candidate.c (s1 form, `sound -= sound` self-subtract)
spliced onto `src/text1b.c` (with the `D_800A35DC` u8->s8 fix already
baked into candidate.c this session) reproduces the s1 floor exactly:
`sandbox --disable all` -> score 6, target_insns 208, build_insns 208.
`--diff` reproduces the identical 30-hunk classification: 2 source-level
(hunks 18-19) + 2 operand-only (hunks 9, 17) + 26 not-scored.

**De-smelled the baseline (CONFIRMED, banked as the new candidate.c).**
Replaced the flagged-as-suspicious `sound -= sound;` self-subtract with a
plain, ordinary-C literal form that reuses the SAME variable across load,
test, and result:

```c
s32 sound = MENU_800747D8->field64;
if (sound == 0) {
    sound = 4;
} else {
    sound = 0;
}
func_8005C650(sound, 0x7F, 0x7F);
```

Measured: score 6, build_insns 208 == target_insns 208 — IDENTICAL to the
self-subtract baseline in both score and full hunk classification (same
hunks 9/17/18/19 in the same classes, byte-for-byte identical diff output).
This is a strict improvement: same honest floor, zero cheat-smell (no
self-subtract, no reader-hostile arithmetic-as-zero idiom). Banked as the
new `candidate.c`. Branch source-order (`sound == 0` first vs `sound != 0`
first) measured identical too — GCC normalizes branch polarity
independent of source order here
(tmp/grind/func_800747D8/s2/diff_swapped.txt).

**Named the mechanism behind the 3-instruction shortfall in all 4
prior/repeated two-value spellings (frontier item 2, now explained).**
Re-measured `rejected/selection_sound-default-then-override.c`
(`s32 flag = field64; s32 sound = 4; if (flag != 0) sound = 0;`) with
`--diff` this session (tmp/grind/func_800747D8/s2/diff_defaultoverride.txt):
score 10, build_insns 205, and the diff shows EXACTLY where the 3 insns
vanish (hunks 17-19) — our build folds the whole flag-test + literal-select
into 3 branchless insns:

```
lbu   a0,0x64(v0)
sltiu a0,a0,1
sll   a0,a0,0x2
```

i.e. `a0 = (flag == 0) * 4`, a fully branchless arithmetic fold of the
`flag != 0 ? 0 : 4` ternary — GCC eliminates the branch entirely. Target
keeps a REAL branch (`beqz v0,...` / delay-slot `li a0,4` / fallthrough
`move a0,zero`), so this is a genuinely different (branchless vs branching)
shape, not just a register-choice difference. The trigger is a FRESH,
single-use-then-dead `flag` pseudo isolated from the result pseudo `sound`
— GCC's combine/constant-propagation recognizes that isolated two-constant
conditional-select shape and folds it branchless. Reusing ONE variable
across load+test+result (this session's new candidate.c spelling) denies
that isolation and keeps the real branch, landing on 208 insns exactly like
the self-subtract baseline. Full mechanism note + all 4 confirmed-identical
prior spellings: `rejected/selection_sound-flag-then-literal-branchless-fold.c`.

**Hunk 9 (jtbl table-offset residual, frontier item 1) — likely explained,
not yet closable from src/text1b.c alone.** `jtbl_80015A0C` (the table this
function's `state` switch, case 0-4, dispatches through) is currently a
HAND-TRANSCRIBED `const u32 jtbl_80015A0C[6]` array in
`src/text1a_b_mid_rodata.c:44-52` — not GCC-synthesized, because
func_800747D8 is still `INCLUDE_ASM`. `bb2.ld` orders
`text1a_b_pre_rodata.o, text1b.o, text1a_b_mid_rodata.o`
(text1a_b_mid_rodata.c:1-8 comment). Sibling func_8006B578's ledger
(session 12, 2026-09-16) established the EXACT same mechanism for
`jtbl_80015988`: once that function's switch became real C, GCC's own
compiler-synthesized ADDR_VEC (emitted into `build/src/text1b.o`'s
own, currently-empty, `.rodata`) landed at 0x80015988 with NO bb2.ld
edit, and the hand-transcribed array for it was deleted from
`text1a_b_mid_rodata.c` in the same change (see that file's own header
comment, which documents doing exactly this for
`jtbl_800159B0`/`jtbl_800159D0`/`jtbl_80015A0C`/`jtbl_80015A24` NOT yet
having happened — only `jtbl_80015988`, owned by `func_8006B578`, has been
migrated). The 24-byte (6-word) offset hunk 9 shows (target `lw v0,0(at)`
vs ours `lw v0,24(at)`) is consistent with our SANDBOX single-function
build's synthesized table landing at a different sub-offset than the
hand-transcribed array it's being scored against, because the two tables
currently coexist (this candidate's real `switch` AND the mid_rodata.c
hand array both claim table space) rather than the hand array being
retired in favor of the compiler-synthesized one. **This is a cross-file
integration matter, not a pure src/text1b.c C-structure lever** — closing
it for real requires deleting the `jtbl_80015A0C` array from
`src/text1a_b_mid_rodata.c` in the SAME change that lands func_800747D8's
C body, exactly as was done for func_8006B578/jtbl_80015988. That edit is
outside this session's mandated surface (func_800747D8 in src/text1b.c
only) and is deferred to the session that submits the final candidate —
NOT flagged as a wall, this is a known, already-precedented mechanical
step. No bb2.ld edit is expected to be needed (same as the sibling case).

- [s2] candidate.c (s1 form) re-measured this session: sandbox --disable all -> score 6, target_insns 208, build_insns 208 - reproduces the s1 floor exactly on the current tree with the D_800A35DC u8->s8 fix baked in.

- [s2] --diff reproduces the identical 30-hunk classification from s1: 2 source-level (hunks 18-19) + 2 operand-only (hunks 9, 17) + 26 not-scored.

- [s2] jtbl_80015A0C (the table func_800747D8's state switch dispatches through) is currently a hand-transcribed const u32[6] array in src/text1a_b_mid_rodata.c:44-52, not GCC-synthesized, because func_800747D8 is still INCLUDE_ASM.

- [s2] src/text1a_b_mid_rodata.c's own header comment documents that this exact migration (hand-transcribed array -> compiler-synthesized ADDR_VEC, zero bb2.ld edits) was already executed for jtbl_80015988 when sibling func_8006B578 landed as C, and that jtbl_800159B0/jtbl_800159D0/jtbl_80015A0C/jtbl_80015A24 have NOT yet been migrated.

- [s2] Sibling func_8006B578's own ledger (s7/s12, 2026-09-16) independently derived and executed the identical mechanism for jtbl_80015988: bb2.ld orders text1a_b_pre_rodata.o, text1b.o, text1a_b_mid_rodata.o; once the switch became real C, GCC's own ADDR_VEC landed at the exact target address with no bb2.ld edit required, after the hand array was deleted.

- [s2] The new candidate.c (same-variable literal-assignment selection_sound spelling) is strictly better than the inherited s1 candidate: identical score/insns, zero self-subtract construct - the s1 ledger's flagged cheat-smell concern about `sound -= sound;` is resolved.

## Session 3 (permuter)

- `import.py`'s full-TU import of `src/text1b.c` is broken for this function (maspsx crash on a pruning artifact); worked around with a hand-built minimal-context permuter workspace (`tmp/grind/func_800747D8/s3/perm_ws/base.c`) that compiles through the real pipeline and reuses the correctly-generated `target.o`. Reusable recipe for any future permuter session on this function or other text1b.c functions hitting the same import.py failure.
- Ran a real permuter campaign (21,884 iterations, 8 jobs, 5 novel finds, base_score 265 unmasked). No find improves on the honest floor of 6; best find (100) is a pure reformat with zero codegen difference. The local C-spelling space around the selection_sound block (hunks 17-19) is now exhausted by both hand-written (s1/s2) and randomized (s3 permuter) search.
- Hand-tested the sanctioned `do { ... } while (0);` wrap (one of the permuter's finds, output-170-1) directly on src/text1b.c: regressed score 6 -> 7 (added a real instruction). Banked as `rejected/selection_sound-do-while-zero-wrap.c`. Confirms this specific sanctioned-family application does not help here.
- Floor unchanged: still 6 (2 source-level: hunks 18-19 selection_sound branch/value-flow shape; 2 operand-only: hunk 9 jtbl table-offset, hunk 17 register seat; 26 not-scored). Frontier unchanged from s2 — hunk 9's jtbl migration (src/text1a_b_mid_rodata.c, out of this session's src/text1b.c-only surface) remains the next actionable step, to be done at final-submission assembly time, mirroring the func_8006B578 precedent.

- [s3] Chassis check confirmed floor 6 at dispatch (candidate.c re-applied, sandbox --disable all): 2 source-level hunks (18-19, selection_sound branch/value-flow shape), 2 operand-only hunks (9: jtbl_80015A0C table-offset, 17: register seat lbu v0 vs a0), 26 not-scored.

- [s3] import.py's default full-TU import of src/text1b.c produces a compile.sh that crashes maspsx with 'too many values to unpack' on a corrupted .comm line -- a pruning-pass artifact from one of the TU's many canonical-asm placeholder stubs, unrelated to func_800747D8's own C.

- [s3] A hand-built minimal-context permuter workspace (self-contained base.c, real target.o from asm/funcs/func_800747D8.s) is a working, reusable substitute for future permuter sessions on this function or other text1b.c functions hitting the same import.py failure.

- [s3] Permuter campaign: 21,884 iterations, 5 novel finds (scores 100/120/130/170/220 vs base 265), 0 finds beat or matched candidate.c's real honest floor; best find is a no-op reformat.

- [s3] Hand-tested the campaign's do-while(0) find directly on src/text1b.c: regressed score 6 -> 7. The local C-spelling space around the selection_sound block is now exhausted by both hand-written (s1/s2) and randomized (s3 permuter) search.

- [s3] src/text1b.c reverted to INCLUDE_ASM("asm/funcs", func_800747D8); before session end per asm-until-matched; candidate.c unchanged in memory/grind/func_800747D8/.

- [s4, enumerate modality] Chassis check confirmed floor 6 at dispatch (candidate.c applied to src/text1b.c, sandbox --disable all --diff): 30 hunks total, 2 source-level (18-19, selection_sound branch/value-flow), 2 operand-only (9: jtbl_80015A0C table offset, 17: register seat lbu v0 vs a0 reading MENU_800747D8->field64), 26 not-scored masked address artifacts. Confirmed hunks 17-19 are the `selection_sound` block (`MENU_800747D8->field64` load + `if (sound==0) sound=4; else sound=0;` diamond + `func_8005C650(sound,...)` call) — same block s1-s3 already worked, NOT the switch(state) case-1 field67 block (which currently scores 0 diff against target already).

- [s4] Ran `tools/spelling_enum.py` (new systematic-sweep tool, first use on this ledger) as a CONTROL on the field67 block (`case 1: if ((ret & 0xFF) != 0) { menu->field67 += 1; ...; work->field66 = D_8009BD20[row][D_800A35DC & 1]; func_8005C650(...); }` — currently byte-matching, zero diff): 4 statement-level assigns (2x `menu = ...`, `work = ...`, `row = work->field67`), 0 named decls, `--no-swaps` -> 6 distinct def-before-use orderings. `tools/sweep_variants.py --func func_800747D8 --file text1b --variants tmp/grind/func_800747D8/s4/enum --json`: baseline 6, ALL 6 orderings scored 24 (build_insns 206, i.e. the compiler CSE-merges the two identical `menu = (S_800747D8*)D_800A36A0;` reloads once they're no longer interleaved with the `menu->field67 += 1;` / `&= 1;` compound-assigns between them). Confirms the interleaved order (assign, compound-modify-anchor, reassign, compound-modify-anchor, assign, assign, anchor, anchor) in the CURRENT candidate.c is uniquely necessary for this already-matching block — hoisting the four plain assigns ahead of the two field67 compound-assigns (the tool's only available transform, since it always sorts decls/assigns before anchors) is strictly worse on all 6 valid orderings. Full sweep JSON: `tmp/grind/func_800747D8/s4/enum/` (6 variant files) + raw sweep stdout captured in session transcript (not persisted as a separate log — scores recorded here are the complete result set).

- [s4] Attempted the same tool on the ACTUAL residual (selection_sound block, hunks 17-19): the region is a decl (`s32 sound = MENU_800747D8->field64;`) followed by an `if (sound == 0) { sound = 4; } else { sound = 0; }` diamond and a trailing call. `spelling_enum.py`'s statement model has NO concept of if/else nesting — it parses the region as a flat statement list and unconditionally emits every decl/assign BEFORE every "anchor" (which is what the un-parsed `if (...) {`/`} else {`/`}` lines become). All 4 generated variants are therefore either (a) semantically WRONG (the `sound = 4;` / `sound = 0;` assigns hoisted out of the if/else arms to run unconditionally in sequence, leaving an empty-armed if/else that does nothing useful — `enum_ss/v0.c`, `v1.c`) or (b) not even valid C (the tool's k>=1 inlining combo drops the `s32 sound = ...;` declaration entirely while `sound = 4;`/`sound = 0;` assign statements survive, referencing an undeclared identifier — `enum_ss/v2.c`, `v3.c`; confirmed by direct compile failure `text1b.c:8475: 'sound' undeclared` when `sweep_variants.py`'s restore-on-finally hit an unrelated transient OSError mid-run and left v2's broken body spliced into src/text1b.c — manually reverted back to candidate.c's known-good text, re-verified floor 6). **This is a tool-capability gap, not a spelling-space result**: `spelling_enum.py` cannot usefully enumerate a region whose control flow is itself part of the differing bytes (an if/else diamond with per-arm assignments) — it is only sound for flat straight-line statement sequences (as the field67 control run above confirms). No new evidence for or against any hunk-17-19 respelling was produced by this tool; the s1-s3 manual-derivation + 21,884-iteration permuter evidence remains the complete search-space record for that block.

- [s4] sandbox --disable all --diff on the current candidate.c (applied to src/text1b.c) reconfirms floor 6: 30 hunks, 2 source-level (18-19), 2 operand-only (9: jtbl_80015A0C table offset; 17: register seat lbu v0 vs a0), 26 not-scored masked address artifacts.

- [s4] Hunks 17-19 are the selection_sound block (MENU_800747D8->field64 load, if(sound==0){sound=4;}else{sound=0;} diamond, func_8005C650(sound,...) call) -- the same block s1-s3 already exhaustively worked (5 rejected forms banked, 21,884-iteration permuter campaign, 6 documented instance kills). This is NOT the switch(state) case-1 field67 block, which currently has zero diff against target.

- [s4] Cross-checked against target asm (asm/funcs/func_800747D8.s, jlabel .L80074984 / .L800749D8) and our own -da dump (tmp/grind/func_800747D8/dumps/text1b.s, .L1302/.L1304) to confirm the field67 block's current C spelling already produces byte-identical bytes to target for that region.

- [s4] src/text1b.c restored to exactly candidate.c's content at session end; sandbox --disable all reconfirms score 6, build_insns 208; git status shows only src/text1b.c + metrics/events.jsonl changed (metrics is pre-existing drift, not from this session's work).

## Session 5 (synthesis) - 2026-09-20

**Chassis re-confirmed.** `candidate.c` spliced onto `src/text1b.c` measures
`sandbox --disable all` -> score 6, build_insns 208 == target_insns 208, the
same figure the driver measured at dispatch. `--diff` reproduces the s1-s4
classification unchanged: 2 source-level (hunks 18-19), 2 operand-only
(hunk 9 jtbl_80015A0C table offset, hunk 17 register seat), 26 not-scored.

**KILL RE-AUDIT (mandated).** Every instance kill in this ledger was
measured on the SAME chassis that is live now (candidate.c, floor 6) and
this ledger has never contained a single `/* FAKE */` construct - neither
candidate.c nor any of the seven banked rejected forms carries one - so
`tools/fake_ablate.py` has nothing to ablate here and the FAKE-carrier
failure mode it guards against cannot apply. The re-audit was therefore
done by DIRECT RE-MEASUREMENT of the two kills that sat closest to target:
s1/H4 (`default-then-override`) re-measured this session as variant
`b2_default4_direct_test.c` (and as `v3_zero_default_override.c` in the
opposite polarity) -> score 10, build_insns 205, reproducing the banked
figure exactly; s1/H5 (`single-shot-no-intermediate`) re-measured as part
of the same class sweep -> identical. Both kills stand, unchanged, on the
current chassis.

**THE SESSION'S MAIN RESULT: the residual is now mechanistically explained
end to end, and the explanation is read out of the compiler source rather
than inferred.** Three regimes exist for the selection_sound block (hunks
17-19), and thirteen measured spellings fall into exactly three buckets:

| regime | spellings measured | score / build_insns |
|---|---|---|
| one variable reused across load+test+result (the banked baseline) | 3 (s1 self-subtract, s2 literal form, s5 pointer-local, s5 function-scope decl) | **6 / 208** |
| single call, result pseudo DISTINCT from the loaded test value | 12 (s1 H3/H4/H5, s2, s5 v3/v4/b1/b2/b11/c1/c2/c3/c4) | 10 / 205 |
| the call DUPLICATED into both arms | 4 (s5 v1/v2/v7 + plain if/else) | 7 / 211 |

- The 205 bucket is GCC 2.7.2's jump.c store-flag transform
  (`tools/gcc-2.7.2/jump.c:1166-1197`, the block commented "That didn't
  work, try a store-flag insn"). Its admitting disjunct is
  `jump.c:1190-1191`: it fires whenever EITHER of the two selected
  constants is `const0_rtx`. The pair selected here is {4, 0}, so one of
  them is always zero regardless of spelling - which is why a `switch`, a
  ternary in the call argument, a QImode (`u8`) result variable, and reuse
  of an already-live local (`state`, `ret`) ALL fold to the identical
  `lbu / sltiu / sll` sequence.
- The baseline escapes that transform for a reason that is now named: its
  result variable's prior value is the LOAD (not a `CONST_INT`, so
  `temp3` fails the first disjunct) and the arm sitting immediately after
  the conditional jump assigns 4 (so `temp2 != const0_rtx`), leaving only
  the `BRANCH_COST >= 3` fallback, which MIPS does not satisfy. The price
  of that escape is exactly hunk 17: the test value and the call argument
  are the same pseudo, so the load is emitted straight into `$a0`.
- The 211 bucket blocks the transform at a DIFFERENT clause,
  `tools/gcc-2.7.2/jump.c:1066` (the insn after the conditional jump must
  be followed immediately by the join label - i.e. the arm must be exactly
  one insn; a full argument setup plus call is five).

**NEW, and the most useful single fact this ledger has gained: duplicating
the call into both arms REPRODUCES TARGET'S REGISTER SEAT.** Measured
objdump of the sandbox object for that form:
`lw v0,0(gp) / lbu v0,100(v0) / bnez v0,L / move a0,zero / li a0,4 /
li a1,127 / j <shared jal> / li a2,127` - the load lands in **$v0**, and
`$a0` is materialized separately in each arm, which is target's exact shape
(asm/funcs/func_800747D8.s:108-118). Hunk 17 disappears from the diff in
that form. The whole residual of the 211 form is 3 SURPLUS instructions:
the two arms' identical `li a1,127 / j <jal> / li a2,127` tails were never
cross-jump-merged with each other. `tools/gcc-2.7.2/jump.c:2005` tries
`find_cross_jump (insn, JUMP_LABEL (insn), 1, ...)` first - a minimum=1
merge against the code physically preceding the jump's own label - and only
falls back to the sibling-jump pairing at `jump.c:2011-2021` when that
returns `newjpos == 0`. Here the first attempt succeeds with a ONE-insn
match against the shared `jal func_8005C650` that the field67 path falls
into, so the deeper 3-insn arm-to-arm merge target shows is never tried.

- [s5] Chassis re-confirmed at dispatch value: candidate.c on src/text1b.c -> sandbox --disable all score 6, build_insns 208 == target_insns 208; --diff reproduces the s1-s4 hunk classification unchanged (2 source-level 18-19, 2 operand-only 9 and 17, 26 not-scored).

- [s5] KILL RE-AUDIT: this ledger contains zero /* FAKE */ constructs in candidate.c and in all seven banked rejected forms, so tools/fake_ablate.py has nothing to ablate; the two closest-to-target instance kills (s1 H4 default-then-override, s1 H5 single-shot-no-intermediate) were instead re-measured directly on the current chassis this session and both reproduce their banked figures exactly (score 10, build_insns 205). Both kills stand.

- [s5] Thirteen spellings of the selection_sound block now partition into exactly three measured regimes: same-variable reuse -> 6/208 (the banked baseline, plus two NEW neutral ties: a pointer local `S_800747D8 *m = MENU_800747D8;` and hoisting `s32 sound;` to the function-scope declaration list, both 6/208); any single-call spelling with the result pseudo distinct from the loaded test value -> 10/205; the call duplicated into both arms -> 7/211.

- [s5] The 10/205 regime is GCC 2.7.2's jump.c store-flag transform (tools/gcc-2.7.2/jump.c:1166-1197), admitted by the disjunct at tools/gcc-2.7.2/jump.c:1190-1191 which fires whenever either selected constant is const0_rtx. The constant pair here is {4, 0}, so one is always zero and no C-level respelling of a two-constant select can miss that disjunct - confirmed by a switch statement, a ternary in the call argument, a u8 (QImode) result variable, and reuse of two different already-live locals (`state`, `ret`) all folding identically.

- [s5] The banked baseline escapes the store-flag transform because its result variable's prior value is the load (not a CONST_INT) AND the arm immediately after the conditional jump assigns 4 (not zero), so neither disjunct at jump.c:1190-1191 applies and only the BRANCH_COST >= 3 fallback remains, which MIPS does not satisfy. That escape is exactly what forces the load into $a0 and keeps hunk 17 open.

- [s5] Duplicating func_8005C650(4/0, 0x7F, 0x7F) into both arms reproduces target's register seat exactly (objdump of the sandbox object: lbu v0,100(v0) with $a0 materialized per-arm, matching asm/funcs/func_800747D8.s:108-118) and removes hunk 17 from the diff; its only residual is 3 surplus instructions from the two arms' identical `li a1,127 / j <jal> / li a2,127` tails failing to cross-jump-merge with each other.

- [s5] That missed merge is explained by tools/gcc-2.7.2/jump.c:2005: for a simplejump GCC tries find_cross_jump against the code before the jump's own label (minimum=1) BEFORE the sibling-jump pairing at jump.c:2011-2021, and the minimum=1 attempt succeeds here on the single shared `jal func_8005C650` the field67 path falls into - so the deeper 3-insn arm-to-arm merge that target shows is never attempted.

- [s5] tools/gcc-2.7.2/jump.c carries a pre-existing BB2 diagnostic knob (jump.c:66-89, env var BB2_XJUMP_DEBUG, codegen-inert when unset) that prints find_cross_jump pairing decisions as "XJDBG: DO_CROSS_JUMP jump=N newjpos=N newlpos=N". It was NOT exercised this session and is the cheapest next probe for the 211 form's missed merge.

- [s5] src/text1b.c restored to INCLUDE_ASM("asm/funcs", func_800747D8); at session end per asm-until-matched; candidate.c unchanged (still the floor-6 same-variable form); git status shows only metrics/events.jsonl (pre-existing drift) outside memory/grind and tmp.

- [s5] Chassis re-confirmed: candidate.c on src/text1b.c -> sandbox --disable all score 6, build_insns 208 == target_insns 208; --diff reproduces the s1-s4 classification unchanged (2 source-level hunks 18-19, 2 operand-only hunks 9 and 17, 26 not-scored).

- [s5] Thirteen spellings of the selection_sound block now partition into exactly three measured regimes: same-variable reuse -> 6/208 (the banked baseline plus two new neutral ties); any single-call spelling whose result pseudo is distinct from the loaded test value -> 10/205; the call duplicated into both arms -> 7/211.

- [s5] The 10/205 regime is GCC 2.7.2's jump.c store-flag transform (tools/gcc-2.7.2/jump.c:1166-1197), admitted by the disjunct at tools/gcc-2.7.2/jump.c:1190-1191 which fires whenever either selected constant is const0_rtx; the constant pair here is {4, 0}, so a switch statement, a ternary in the call argument, a u8 (QImode) result variable and reuse of two different already-live locals all fold identically.

- [s5] The banked floor-6 baseline escapes that transform because its result variable's prior value is the load (not a CONST_INT) AND the arm immediately after the conditional jump assigns 4 (not zero), leaving only the BRANCH_COST >= 3 fallback which MIPS does not satisfy - and that escape is precisely what forces the load into $a0 and keeps hunk 17 open.

- [s5] Duplicating func_8005C650(4/0, 0x7F, 0x7F) into both arms reproduces target's register seat exactly (objdump: lbu v0,100(v0) with $a0 materialized per-arm, matching asm/funcs/func_800747D8.s:108-118) and removes hunk 17 from the diff; its only residual is 3 surplus instructions from the two arms' identical `li a1,127 / j <jal> / li a2,127` tails failing to cross-jump-merge with each other.

- [s5] That missed merge is explained by tools/gcc-2.7.2/jump.c:2005: for a simplejump GCC tries find_cross_jump against the code before the jump's own label (minimum=1) BEFORE the sibling-jump pairing at tools/gcc-2.7.2/jump.c:2011-2021, and the minimum=1 attempt succeeds here on the single shared `jal func_8005C650` the field67 path falls into.

- [s5] tools/gcc-2.7.2/jump.c carries a pre-existing BB2 diagnostic knob (jump.c:66-89, env var BB2_XJUMP_DEBUG, codegen-inert when unset) that prints find_cross_jump pairing decisions as 'XJDBG: DO_CROSS_JUMP jump=N newjpos=N newlpos=N'. It was NOT exercised this session and is the cheapest next probe for the 211 form's missed merge.

- [s5] Hunk 9's jtbl_80015A0C table-offset residual is unchanged and remains a cross-file final-submission step (delete the hand-transcribed array from src/text1a_b_mid_rodata.c in the same change that lands the C body), per the s2 analysis and the executed func_8006B578/jtbl_80015988 precedent.

- [s5] src/text1b.c restored to INCLUDE_ASM("asm/funcs", func_800747D8); at session end per asm-until-matched; candidate.c unchanged (still the floor-6 same-variable form, zero FAKE constructs); git status shows only metrics/events.jsonl (pre-existing drift) outside memory/grind and tmp.

## Session 6 (solver) - 2026-09-20

**Chassis re-confirmed.** `candidate.c` spliced onto `src/text1b.c` measures
`sandbox --disable all` -> score 6, build_insns 208 == target_insns 208, the
figure the driver measured at dispatch. `--diff` reproduces the s1-s5
classification unchanged (2 source-level hunks 18-19, 2 operand-only hunks 9
and 17, 26 not-scored).

**THE SESSION'S HEADLINE: the selection-sound residual is no longer a search
problem - both of the two remaining C-level questions now have a named,
dump-confirmed compiler gate, and one NEW form (variant F) reproduces target's
register seat AND target's branch at target's instruction count.**

### 1. The 205 regime is fully attributed (was "GCC folds it", now named)

The twelve-spelling 10/205 bucket is the product of TWO passes, in order, and
the .rtl-vs-.jump dump pair names both:

- `tools/gcc-2.7.2/jump.c:726-860` - "Simplify `if (...) x = a; else x = b;` by
  converting it to `x = b; if (...) x = a;`". In the pre-jump dump
  (`tmp/grind/func_800747D8/dumps/text1b.rtl`, function at line 69695) the arms
  are insn 282 (`r140 = 0`) and insn 290 (`r140 = 4`); in the post-jump dump
  (`text1b.jump`, function at 66301) insn 290 is GONE and a NEWLY created insn
  622 (`r140 = 4`) sits immediately BEFORE the conditional jump. That is this
  transform's output, observed, not inferred.
- `tools/gcc-2.7.2/jump.c:1178-1181` - the store-flag gate. Once `x = 4`
  precedes the jump, `reg_set_last (temp1, insn)` at
  `tools/gcc-2.7.2/jump.c:1061` returns CONST_INT 4, which satisfies the gate's
  FIRST disjunct. MIPS `BRANCH_COST` is **1** for the R3000
  (`tools/gcc-2.7.2/config/mips/mips.h:2937` returns 2 only for R4000/R6000),
  so both `BRANCH_COST >= 2` and `BRANCH_COST >= 3` fallbacks are dead here and
  `temp3 == CONST_INT` is the ONLY door into the transform. emit_store_flag
  then emits `xor / ltu / neg / and 4` (.jump insns 629-635), which combine
  folds to the observed `sltiu a0,a0,1 / sll a0,a0,2`.

This corrects the s5 attribution in one respect worth recording: s5 credited
the fold to the `jump.c:1190-1191` "either constant is const0_rtx" disjunct.
That disjunct is necessary but NOT sufficient - the load-bearing gate for this
function is `temp3 == CONST_INT` at jump.c:1178-1181, because BRANCH_COST is 1.
This matters because it names the ESCAPE: make reg_set_last unable to return a
CONST_INT.

### 2. reg_set_last's label-stop IS a usable escape (variant F, NEW)

`reg_set_last` (`tools/gcc-2.7.2/rtlanal.c:886-888`) scans backwards and
"Stop[s] when we reach a label", returning 0. Variant F
(`memory/grind/func_800747D8/rejected/selection_sound-default-hoisted-above-label.c`)
puts `sound = 4;` ABOVE the `selection_sound:` CODE_LABEL, leaving only
`if (MENU_800747D8->field64 != 0) sound = 0;` inside the block. Measured
**score 8, build_insns 208**, and for the first time in this ledger:

- hunk 17 MATCHES: `lbu v0,0x64(v0)` - target's register seat, the load out of
  $a0 because $a0 now carries the default 4.
- the branch MATCHES: `beqz v0,<join>` - target's exact branch (the floor-6
  baseline emits `bnez a0,<join>`).

The price, all four diffs visible in `tmp/grind/func_800747D8/s6/diff_F.txt`:
`sound = 4;` is now in a DIFFERENT basic block from the branch, so reorg's
backward `fill_simple_delay_slots` cannot reach it. The branch's delay slot
instead gets `li a1,127` stolen from the target thread; the orphaned `li a0,4`
lands in the switch-dispatch delay slot at c734 (hunk 10, an insn target does
not have); and with $a0 pinned live across the field65 block, that block's
`lbu`/`bne`/`addiu` shift to $a1 (hunks 13/14, 3 operand-only diffs the
baseline does not have).

**So the residual is now a single, precisely stated tension:** target needs
`a0 = 4` to be (a) inside the branch's own basic block, so reorg's backward
slot fill takes it, and (b) invisible to `reg_set_last`, so the store-flag gate
fails. The only two things that stop reg_set_last's backward scan are a
CODE_LABEL (`rtlanal.c:886-888`) and `reg_set_last_unknown`, which
`reg_set_last_1` (`tools/gcc-2.7.2/rtlanal.c:851-858`) sets only for a CLOBBER
or when `SET_DEST (pat) != x` (a SUBREG / STRICT_LOW_PART destination). A
CODE_LABEL also stops reorg's backward fill, so (a) and (b) cannot both be had
via a label. That leaves two live routes, both named in the frontier.

### 3. The duplicated-call form's missed cross-jump is PROVED, not hypothesised

The s5 frontier's named next probe (BB2_XJUMP_DEBUG=1, the codegen-inert knob
at `tools/gcc-2.7.2/jump.c:66-89`) was run this session through the
INSTRUMENTED cc1 (`tools/gcc-2.7.2/cc1`, per [[instrumented-cc1-location]]).
Trace: `tmp/grind/func_800747D8/s6/dumps/xjdbg.txt` (2202 lines for the TU;
func_800747D8's region is lines 1840-1930, identified by insn uids read out of
`tmp/grind/func_800747D8/s6/dumps/text1b.sched2` at 106432+). The decisive
lines, verbatim:

    XJDBG: enter e1=288 e2=424 min=1 (own-label)
    XJDBG:   MATCH i1=286 i2=418 parallel min->0
    XJDBG:   PAT-MISMATCH i1=284 set(reg<-127) vs i2=409 set lose=0
    XJDBG: result e1=288 min=0 last1=286 => WIN
    XJDBG: DO_CROSS_JUMP jump=288 newjpos=286 newlpos=418
    XJDBG: enter e1=288 e2=660 min=1 (own-label)
    XJDBG:   PAT-MISMATCH i1=284 set(reg<-127) vs i2=409 set lose=0
    XJDBG: result e1=288 min=1 last1=0 => no
    XJDBG: enter e1=304 e2=424 min=1 (own-label)
    XJDBG:   MATCH i1=300 i2=418 parallel min->0
    XJDBG: result e1=304 min=0 last1=300 => WIN
    XJDBG: DO_CROSS_JUMP jump=304 newjpos=300 newlpos=418

Reading it against the sched2 RTL: jump_insn 288 and 304 are the two arms'
`goto confirm`; code_label 424 is `confirm`; call_insn 418 is case 2's
`func_8005C650` call, which falls through into `confirm` and is therefore the
insn physically preceding that label. The minimum=1 own-label attempt at
`tools/gcc-2.7.2/jump.c:2005` WINS with a ONE-insn match (call vs call) for
BOTH arms and retargets both jumps to a NEWLY created label, uid 660. From
then on the sibling-jump pairing loop at
`tools/gcc-2.7.2/jump.c:2011-2021` - the one that would have matched the arms
against each other 3 insns deep (`li a1,127 / li a2,127 / jal`) - is
unreachable for those two jumps for two independent reasons: it is guarded by
`INSN_UID (JUMP_LABEL (insn)) < max_uid`, false for uid 660, and `jump_chain`
is never updated for a label created by `do_cross_jump`. The trace contains
ZERO `chain-partner` entries for e1=288 or e1=304 across every
`while (changed)` iteration. By contrast e1=369 (case 1's jump, still pointing
at the original label 424) merges 8 insns deep in the same trace.

### 4. Three more spellings measured and killed

- Duplicated call with INVERTED polarity (`!= 0` / zero-arm first): 7/211,
  the same figures as the `== 0` spelling. Extends the s2 polarity kill from
  the same-variable chassis to the duplicated-call chassis.
- Duplicated call in `else`-free / early-`goto` spelling: 7/211. Written to
  test whether hoisting `goto confirm` into each arm gives the arms a private
  join label; GCC threads both arm jumps through to `confirm` before jump2
  runs, so the layout is unchanged.
- Single call, distinct local, ZERO arm first (the last unmeasured natural
  polarity of that regime): 10/205, joining the twelve.

- [s6] Chassis re-confirmed: candidate.c on src/text1b.c -> sandbox --disable all score 6, build_insns 208 == target_insns 208; --diff reproduces the s1-s5 hunk classification unchanged.
- [s6] MIPS BRANCH_COST is 1 for the R3000 (tools/gcc-2.7.2/config/mips/mips.h:2937 returns 2 only for PROCESSOR_R4000 / PROCESSOR_R6000), so in the store-flag gate at tools/gcc-2.7.2/jump.c:1178-1181 the BRANCH_COST >= 2 and BRANCH_COST >= 3 fallbacks are both dead and `temp3 == CONST_INT` is the ONLY admitting disjunct for this function. This REFINES the s5 attribution, which credited jump.c:1190-1191 (either constant is const0_rtx) - that clause is necessary but not sufficient.
- [s6] The 10/205 regime is produced by TWO passes in sequence, confirmed by reading tmp/grind/func_800747D8/dumps/text1b.rtl (function at line 69695) against text1b.jump (function at 66301): tools/gcc-2.7.2/jump.c:726-860 first normalises `if (c) x=a; else x=b;` into `x=b; if (c) x=a;` (arms insn 282 / insn 290 in .rtl become a NEW insn 622 `r140 = 4` sitting before the conditional jump in .jump), and only then does the store-flag gate see a CONST_INT in reg_set_last and fire (emitting .jump insns 629-635, `xor / ltu / neg / and 4`, which combine folds to sltiu+sll).
- [s6] reg_set_last (tools/gcc-2.7.2/rtlanal.c:886-888) stops its backward scan at a CODE_LABEL and returns 0; reg_set_last_1 (tools/gcc-2.7.2/rtlanal.c:851-858) additionally returns-unknown only for a CLOBBER or when SET_DEST (pat) != x, i.e. a SUBREG / STRICT_LOW_PART destination. Those are the ONLY two ways a C author can make the store-flag gate's temp3 non-CONST_INT while a constant default is in flight.
- [s6] NEW BEST-SHAPE FORM (variant F, rejected/selection_sound-default-hoisted-above-label.c): hoisting `sound = 4;` above the `selection_sound:` CODE_LABEL measures score 8, build_insns 208, and is the FIRST spelling in this ledger to match BOTH of target's selection-block bytes that the floor-6 baseline gets wrong - `lbu v0,0x64(v0)` (hunk 17 register seat) and `beqz v0,<join>` (the branch). Its cost is entirely placement: `sound = 4;` is in a different basic block from the branch, so reorg's backward fill_simple_delay_slots cannot reach it (slot gets `li a1,127` stolen from the target thread), the orphaned `li a0,4` lands in the switch-dispatch delay slot at c734, and $a0 pinned live across the field65 block pushes that block onto $a1 (3 new operand-only diffs). Scored diff: tmp/grind/func_800747D8/s6/diff_F.txt.
- [s6] The residual is now ONE stated tension: target needs `a0 = 4` both (a) inside the branch's own basic block, so reorg's backward slot fill takes it, and (b) invisible to reg_set_last, so the store-flag gate fails. A CODE_LABEL delivers (b) but destroys (a), because reorg's backward fill also stops at a label.
- [s6] The duplicated-call form's 3-insn surplus is PROVED (not hypothesised) to be the jump.c cross-jump ordering, via the BB2_XJUMP_DEBUG trace at tmp/grind/func_800747D8/s6/dumps/xjdbg.txt lines 1855-1870 read against tmp/grind/func_800747D8/s6/dumps/text1b.sched2: the minimum=1 own-label find_cross_jump at tools/gcc-2.7.2/jump.c:2005 WINS with a ONE-insn match (arm call_insn 286 / 300 against case 2's call_insn 418, which falls through into `confirm`) and retargets both arm jumps to a newly created label (uid 660); thereafter the sibling-jump pairing loop at tools/gcc-2.7.2/jump.c:2011-2021 is unreachable for them both because `INSN_UID (JUMP_LABEL (insn)) < max_uid` is false for uid 660 and because jump_chain is never updated for a do_cross_jump-created label. Zero `chain-partner` entries appear for e1=288/304 in the whole trace, while e1=369 (case 1's jump, still on the original label) merges 8 insns deep.
- [s6] Duplicated-call polarity is codegen-inert on the duplicated-call chassis: `!= 0` with the zero arm first measures 7/211, the same as `== 0` with the 4 arm first. This extends the s2 polarity kill (measured on the same-variable chassis) to the dup chassis.
- [s6] The `else`-free / early-`goto` spelling of the duplicated-call form also measures 7/211: GCC threads both arm jumps straight through to `confirm` before jump2's cross-jumping runs, so giving the arms a private join label is not reachable by moving the `goto confirm` into the arms.
- [s6] Single call, distinct local, ZERO arm first measures 10/205 - the last unmeasured natural polarity of that regime now joins the other twelve.
- [s6] SIBLING SWEEP: the two "UNSPENT" siblings the dispatch flagged (`main` in src/ings.c, func_800692C0 in src/text1b.c) carry nothing transplantable for this residual. func_800692C0 is this function's CALLEE, already COMPLETED-C on main, and the candidate already calls it with the prototype main ships; no block is shared. `main`/ings.c shares no block either - it names func_800747D8 only through the shared text1b/ings call graph. Recorded so a later session does not re-sweep them.
- [s6] src/text1b.c restored to INCLUDE_ASM("asm/funcs", func_800747D8); at session end per asm-until-matched; candidate.c unchanged (still the floor-6 same-variable form, zero FAKE constructs); git status shows only metrics/events.jsonl (pre-existing drift) outside memory/grind and tmp.

- [s6] Chassis re-confirmed: candidate.c on src/text1b.c measures sandbox --disable all score 6, build_insns 208 == target_insns 208; --diff reproduces the s1-s5 hunk classification unchanged (2 source-level hunks 18-19, 2 operand-only hunks 9 and 17, 26 not-scored).

- [s6] MIPS BRANCH_COST is 1 for the R3000 (tools/gcc-2.7.2/config/mips/mips.h:2937 returns 2 only for PROCESSOR_R4000 / PROCESSOR_R6000), so in the store-flag gate at tools/gcc-2.7.2/jump.c:1178-1181 the BRANCH_COST >= 2 and BRANCH_COST >= 3 fallbacks are both dead and temp3 == CONST_INT is the ONLY admitting disjunct for this function. This refines the s5 attribution, which credited jump.c:1190-1191 (either constant is const0_rtx) - that clause is necessary but not sufficient.

- [s6] The 10/205 regime is produced by TWO passes in sequence, confirmed by reading tmp/grind/func_800747D8/dumps/text1b.rtl (function at line 69695) against text1b.jump (function at 66301): tools/gcc-2.7.2/jump.c:726-860 first normalises `if (c) x=a; else x=b;` into `x=b; if (c) x=a;` (the .rtl arms insn 282 / insn 290 become a NEW insn 622 `r140 = 4` sitting before the conditional jump in .jump), and only then does the store-flag gate see a CONST_INT in reg_set_last and fire, emitting .jump insns 629-635 (xor / ltu / neg / and 4) which combine folds to sltiu+sll.

- [s6] reg_set_last (tools/gcc-2.7.2/rtlanal.c:867-916) stops its backward scan at a CODE_LABEL and returns 0; reg_set_last_1 (tools/gcc-2.7.2/rtlanal.c:851-858) additionally returns-unknown only for a CLOBBER or when SET_DEST (pat) != x, i.e. a SUBREG / STRICT_LOW_PART destination. Those are the only two ways a C author can make the store-flag gate's temp3 non-CONST_INT while a constant default is in flight.

- [s6] NEW BEST-SHAPE FORM (variant F, rejected/selection_sound-default-hoisted-above-label.c): hoisting `sound = 4;` above the `selection_sound:` CODE_LABEL measures score 8, build_insns 208, and is the FIRST spelling in this ledger to match BOTH selection-block bytes the floor-6 baseline gets wrong - `lbu v0,0x64(v0)` (hunk 17 register seat) and `beqz v0,<join>` (the branch). Its cost is placement only: the assignment is in a different basic block from the branch, so reorg's backward fill_simple_delay_slots cannot reach it (slot gets `li a1,127` stolen from the target thread), the orphaned `li a0,4` lands in the switch-dispatch delay slot at c734, and $a0 pinned live across the field65 block pushes that block onto $a1 (3 new operand-only diffs). Scored diff: tmp/grind/func_800747D8/s6/diff_F.txt.

- [s6] The residual is now ONE precisely stated tension: target needs `a0 = 4` both (a) inside the branch's own basic block, so reorg's backward slot fill takes it, and (b) invisible to reg_set_last, so the store-flag gate fails. A CODE_LABEL delivers (b) but destroys (a), because reorg's backward fill also stops at a label.

- [s6] The duplicated-call form's 3-insn surplus is PROVED to be jump.c cross-jump ordering, via the BB2_XJUMP_DEBUG trace at tmp/grind/func_800747D8/s6/dumps/xjdbg.txt lines 1855-1870 read against tmp/grind/func_800747D8/s6/dumps/text1b.sched2: the minimum=1 own-label find_cross_jump at tools/gcc-2.7.2/jump.c:2005 WINS with a one-insn match (arm call_insn 286 / 300 against case 2's call_insn 418, which falls through into `confirm`) and retargets both arm jumps to a newly created label (uid 660); thereafter the sibling-jump pairing loop at tools/gcc-2.7.2/jump.c:2011-2021 is unreachable for both because INSN_UID (JUMP_LABEL (insn)) < max_uid is false for uid 660 (tools/gcc-2.7.2/jump.c:2012) and because jump_chain is never updated for a do_cross_jump-created label. Zero `chain-partner` entries appear for e1=288/304 in the whole trace, while e1=369 (case 1's jump, still on the original label) merges 8 insns deep.

- [s6] Duplicated-call polarity is codegen-inert on the duplicated-call chassis: `!= 0` with the zero arm first measures 7/211, the same as `== 0` with the 4 arm first. This extends the s2 polarity kill from the same-variable chassis to the dup chassis.

- [s6] The `else`-free / early-`goto` spelling of the duplicated-call form also measures 7/211: GCC threads both arm jumps straight through to `confirm` before jump2's cross-jumping runs, so giving the arms a private join label is not reachable by moving `goto confirm` into the arms.

- [s6] Single call, distinct local, ZERO arm first measures 10/205 - the last unmeasured natural polarity of that regime now joins the other thirteen.

- [s6] SOLVER-MODALITY NOTE for the next session: the residual triaged PRE-RA, not RA or SCHED. The full classed --diff, the -da dump pair and the XJDBG trace all place both remaining scored hunks (17 and 18/19) inside tools/gcc-2.7.2/jump.c decisions taken before local-alloc runs, so ra_solver / sched_solver vectors cannot move them; the levers are source-level control-flow shape and basic-block boundaries. Recorded so a later solver session does not spend a campaign on the wrong layer.

- [s6] Hunk 9's jtbl_80015A0C table-offset residual is unchanged and remains a cross-file final-submission step (delete the hand-transcribed array from src/text1a_b_mid_rodata.c in the same change that lands the C body), per the s2 analysis and the executed func_8006B578/jtbl_80015988 precedent.

- [s6] src/text1b.c restored to INCLUDE_ASM("asm/funcs", func_800747D8); at session end per asm-until-matched; candidate.c unchanged (still the floor-6 same-variable form, zero FAKE constructs); git status shows only metrics/events.jsonl (pre-existing drift) outside memory/grind and tmp.

## Session 7 (forensics) - 2026-09-20

**Chassis re-confirmed.** `candidate.c` spliced onto `src/text1b.c` measures
`sandbox --disable all` -> score 6, build_insns 208 == target_insns 208 (the
figure the driver measured at dispatch). `--diff`
(`tmp/grind/func_800747D8/s7/diff_base.txt`) reproduces the s1-s6
classification unchanged: 30 hunks, 2 source-level (18/19), 2 operand-only
(9 and 17), 26 not-scored.

**THE SESSION'S HEADLINE: the 205 fold has been re-attributed one pass
UPSTREAM, and its entry gate's eleven C-visible preconditions are now
enumerated with file:line predicates - and the duplicated-call family's
3-insn surplus has been traced to its actual cause (jump threading of the
if/else join label) and REPRODUCED closing, three insns deep, in an XJDBG
trace.**

### 1. Re-attribution: the entry gate is jump.c:754-860, not the store-flag gate

s5 credited the 10/205 fold to `tools/gcc-2.7.2/jump.c:1190-1191`; s6 corrected
that to the `temp3 == CONST_INT` disjunct at
`tools/gcc-2.7.2/jump.c:1178-1181` (BRANCH_COST is 1 on the R3000, so the two
BRANCH_COST fallbacks are dead). Both are true of the insn stream the
store-flag gate SEES, but neither is the gate that decides. On the two-arm
chassis the store-flag block at `tools/gcc-2.7.2/jump.c:1040-1200` cannot fire
at all until the normalisation at `tools/gcc-2.7.2/jump.c:754-860` has already
rewritten `if (c) x = a; else x = b;` into `x = b; if (c) x = a;`:

- its own precondition `reallabelprev == temp || (next_active_insn (temp) is a
  simplejump to JUMP_LABEL (insn))` (`tools/gcc-2.7.2/jump.c:1065-1068`) is
  FALSE for the un-normalised two-arm shape, because the fallthrough arm is
  followed by the arm's own `goto join`, whose JUMP_LABEL is the join and not
  the else-label the conditional jump targets; and
- `reg_set_last` (`tools/gcc-2.7.2/rtlanal.c:886-888`) returns 0 anyway, its
  backwards scan reaching the `selection_sound:` CODE_LABEL without finding any
  set of the result pseudo.

Practical consequence: **blocking the normalisation is SUFFICIENT.** There is
no separate store-flag escape to engineer, and the s6 "single stated tension"
(`a0 = 4` must be both inside the branch's basic block for reorg's backward
fill and invisible to `reg_set_last`) is a property of the SINGLE-ARM
`x = 4; if (c) x = 0;` chassis only. On the two-arm chassis the tension does
not exist, because target's `li a0,4` is stolen from the branch's TARGET
THREAD - the arm never has to live in the branch's own block.

### 2. PASS-INPUT ENUMERATION of the normalisation (H19)

Eleven predicates, each with a file:line, are now banked in hypotheses.md H19.
The load-bearing ones for this function:

- **B2** (`tools/gcc-2.7.2/jump.c:775`) and **B6**
  (`tools/gcc-2.7.2/jump.c:784-789`): either arm having two or more insns kills
  the normalisation. This is exactly why every duplicated-call spelling escapes
  the fold (7/211) and every one-insn-arm spelling does not (10/205).
- **B10** (`tools/gcc-2.7.2/jump.c:833`): the condition may not mention the
  result pseudo. This is the floor-6 baseline's escape - and it is also the
  reason the baseline emits `lbu a0,0x64(v0)` where target emits
  `lbu v0,0x64(v0)`, because "the condition mentions the result" forces the
  loaded byte and the result into one pseudo.
- **B4** (`tools/gcc-2.7.2/jump.c:766-768`), **B8**
  (`tools/gcc-2.7.2/jump.c:794-812,831`) and **B9**
  (`tools/gcc-2.7.2/jump.c:832`) are the three predicates NO banked spelling
  has yet tripped. They are the next session's search space.

### 3. The duplicated-call arms DO merge - proved, with the real cause named

s6 proved the symptom (the minimum=1 own-label `find_cross_jump` at
`tools/gcc-2.7.2/jump.c:2005` wins a 1-insn call-vs-call match and locks both
arm jumps out of the sibling-pairing loop). This session names the CAUSE and
reproduces the merge.

`find_cross_jump` (`tools/gcc-2.7.2/jump.c:2403`) walks `i2 = PREV_INSN (e2)`
skipping only NOTEs and CODE_LABELs, so the match depth is decided by whatever
insn PHYSICALLY PRECEDES the jump's label. In the in-place duplicated-call
spelling the if/else join label is immediately followed by `goto confirm;`, so
jump.c threads both arm jumps past the join to `confirm` - and `confirm`'s
physical predecessor is case 2's `func_8005C650` call, giving a 1-insn match.
Remove the `goto confirm;` (by making the selection block the last thing before
`confirm:`) and the arm jump's own label is the join again, whose predecessor
is the SIBLING ARM. Trace, verbatim, from
`tmp/grind/func_800747D8/s7/dumps/xjdbg.txt:1901-1907`:

    XJDBG: enter e1=406 e2=421 min=1 (own-label)
    XJDBG:   MATCH i1=404 i2=418 parallel min->0
    XJDBG:   MATCH i1=402 i2=416 set(reg<-127) min->-1
    XJDBG:   MATCH i1=400 i2=414 set(reg<-127) min->-2
    XJDBG:   PAT-MISMATCH i1=398 set(reg<-4) vs i2=412 set(reg<-0) lose=0
    XJDBG: result e1=406 min=-2 last1=400 => WIN
    XJDBG: DO_CROSS_JUMP jump=406 newjpos=400 newlpos=414

That is `jal` / `a2 = 127` / `a1 = 127` merged, mismatching only at `a0 = 4`
vs `a0 = 0` - the exact three-insn merge the in-place spelling never gets.
s6's suggested next probe (reorder case 2 so its call is not the last insn
before `confirm`) attacks the wrong end of the chain and should not be spent.

### 4. But the relocation costs more than the merge saves, and target's layout forbids it

- duplicated-call, relocated: score 32, build_insns 217
  (`rejected/selection_sound-dup-call-relocated-before-confirm.c`).
- single-call CONTROL, relocated: score 29, build_insns 214
  (`rejected/selection_sound-single-call-relocated-before-confirm.c`) - so the
  relocation ALONE costs +9 insns over the in-place 10/205 spelling, and the
  dup form's +6 net is that +9 minus the 3 insns the merge recovers.
- target's own layout forbids the relocation regardless: the selection block
  sits FIRST, before case 1's label (asm/funcs/func_800747D8.s:107-118 precede
  `jlabel .L80074984` at :120), and its path ends `addiu a1,zero,0x7F` /
  `j .L80074A20` / `addiu a2,zero,0x7F` with the single `jal func_8005C650`
  shared at `.L80074A20` (:115-117, :161-162). Target's if/else join is itself
  followed by a jump, i.e. exactly the configuration that threads arm jumps
  away. **Target's selection block is the SINGLE-call two-arm form**, whose one
  `jal` was own-label-merged 1 insn deep into case 2's - the same 1-insn merge
  our duplicated-call form gets, but on a block that only ever had one call.

### 5. Mechanism probe on B9 was inconclusive by construction

`if (field64 != 0 || field65 != 0) sound = 0; else sound = 4;` measures 10/205
with hunks 18/19 reading `lhu a0,100(v0)` / `sltiu a0,a0,1` / `sll a0,a0,0x2`:
the two ADJACENT byte tests are coalesced into one halfword comparison, so no
TRUTH_ORIF drop-through CODE_LABEL is ever emitted and B9 is never exercised.
Testing B9 needs a compound condition whose operands cannot be coalesced.

- [s7] Chassis re-confirmed: candidate.c on src/text1b.c -> sandbox --disable all score 6, build_insns 208 == target_insns 208; --diff (tmp/grind/func_800747D8/s7/diff_base.txt) reproduces the s1-s6 hunk classification unchanged (2 source-level 18/19, 2 operand-only 9 and 17, 26 not-scored).
- [s7] RE-ATTRIBUTION: on the two-arm chassis the store-flag transform at tools/gcc-2.7.2/jump.c:1178-1181 is unreachable until the normalisation at tools/gcc-2.7.2/jump.c:754-860 has already run, because (a) the store-flag block's own precondition at tools/gcc-2.7.2/jump.c:1065-1068 (`reallabelprev == temp` or the next active insn after temp is a simplejump to the same label) is false for an un-normalised two-arm shape, and (b) reg_set_last (tools/gcc-2.7.2/rtlanal.c:886-888) returns 0 there anyway, its backward scan hitting the `selection_sound:` CODE_LABEL. Blocking the normalisation is therefore SUFFICIENT; no separate store-flag escape has to be engineered. This supersedes the s6 framing that the two gates must be defeated together.
- [s7] The s6 "single stated tension" (`a0 = 4` must be simultaneously inside the branch's basic block and invisible to reg_set_last) is specific to the SINGLE-ARM `x = 4; if (c) x = 0;` chassis. On the two-arm chassis it does not apply: target's `li a0,4` is stolen from the branch's TARGET thread, so the arm never needs to sit in the branch's own basic block.
- [s7] PASS-INPUT ENUMERATION (hypotheses.md H19): eleven C-visible preconditions of tools/gcc-2.7.2/jump.c:754-860, each with a file:line predicate. B2 (jump.c:775) and B6 (jump.c:784-789) mean either arm having 2+ insns kills the normalisation - which is exactly why the duplicated-call family escapes the fold at 7/211 while every one-insn-arm spelling folds at 10/205. B10 (jump.c:833) - the condition may not mention the result pseudo - is the floor-6 baseline's escape and the direct cause of its `lbu a0` register seat. B4 (jump.c:766-768, a branch-target arm whose value comes from MEMORY), B8 (jump.c:794-812,831, an unresolvable LABEL_NUSES on the else-label) and B9 (jump.c:832, a CODE_LABEL between the last condjump and the then-arm's terminating jump) are the three predicates no banked spelling has yet tripped.
- [s7] The duplicated-call arms DO cross-jump-merge 3 insns deep (jal / a2=127 / a1=127, mismatching only at a0=4 vs a0=0) when the arm jump's own label is still the if/else join. PROVED by the BB2_XJUMP_DEBUG trace at tmp/grind/func_800747D8/s7/dumps/xjdbg.txt:1901-1907 (`enter e1=406 e2=421 min=1 (own-label)` -> three MATCH lines -> `DO_CROSS_JUMP jump=406 newjpos=400 newlpos=414`). The in-place spelling loses that merge because the join label is immediately followed by `goto confirm;`, so jump.c threads both arm jumps past the join to `confirm`, whose physical predecessor is case 2's call - and find_cross_jump (tools/gcc-2.7.2/jump.c:2403) walks `i2 = PREV_INSN (e2)` skipping only NOTEs and CODE_LABELs, so the label's physical predecessor is what decides match depth. This names the CAUSE behind the s6 symptom and retires s6's suggested probe (reordering case 2's call), which attacks the wrong end.
- [s7] The relocation that buys the merge costs more than the merge: duplicated-call relocated to immediately before `confirm:` measures 32/217, and the single-call CONTROL with the same relocation measures 29/214, i.e. +9 insns of pure layout over the in-place 10/205. Both banked in rejected/.
- [s7] TARGET'S LAYOUT EXCLUDES THE DUPLICATED-CALL FAMILY on this chassis: the selection block sits FIRST, before case 1's label (asm/funcs/func_800747D8.s:107-118 precede `jlabel .L80074984` at :120), and ends `addiu a1,zero,0x7F` / `j .L80074A20` / `addiu a2,zero,0x7F` with the single `jal func_8005C650` shared at .L80074A20 (:115-117, :161-162) - one a1/a2 pair for the whole block plus a 1-insn own-label merge at the shared jal. Target's join is itself followed by a jump, i.e. exactly the configuration that threads arm jumps away, so target's selection block is the SINGLE-call two-arm form and the residual is entirely "which of B4/B8/B9 does the original C trip".
- [s7] MECHANISM PROBE (not a candidate, deliberately not semantics-preserving): `if (field64 != 0 || field65 != 0) sound = 0; else sound = 4;` measures 10/205 with hunks 18/19 reading `lhu a0,100(v0)` / `sltiu a0,a0,1` / `sll a0,a0,0x2` - the two ADJACENT byte tests coalesce into one halfword comparison, so no TRUTH_ORIF drop-through CODE_LABEL is emitted and B9 is never exercised. Testing B9 needs a compound condition whose operands cannot be coalesced into a single comparison.
- [s7] KILL RE-AUDIT (mandated): variant F (tmp/grind/func_800747D8/s6/var_F.c) re-measures score 8 / build_insns 208 on the current chassis, unchanged from s6, so that instance kill re-stands. `python3 tools/fake_ablate.py --func func_800747D8 --file text1b --candidate memory/grind/func_800747D8/candidate.c` reports "no FAKE-annotated constructs found ... nothing to ablate", so the FAKE-ablation prong of the re-audit is vacuous for this ledger: no banked kill was ever measured with a FAKE carrier occupying a target pseudo.
- [s7] Hunk 9's jtbl_80015A0C table-offset residual is unchanged and remains a cross-file final-submission step (delete the hand-transcribed array from src/text1a_b_mid_rodata.c in the same change that lands the C body), per the s2 analysis and the executed func_8006B578/jtbl_80015988 precedent.
- [s7] src/text1b.c restored to INCLUDE_ASM("asm/funcs", func_800747D8); at session end per asm-until-matched; candidate.c unchanged (still the floor-6 same-variable form, zero FAKE constructs).

- [s7] Chassis re-confirmed: candidate.c on src/text1b.c measures sandbox --disable all score 6, build_insns 208 == target_insns 208; --diff (tmp/grind/func_800747D8/s7/diff_base.txt) reproduces the s1-s6 classification unchanged (2 source-level hunks 18/19, 2 operand-only hunks 9 and 17, 26 not-scored).

- [s7] The 10/205 fold is produced by TWO passes in series and the DECIDING one is the first: tools/gcc-2.7.2/jump.c:754-860 normalises `if (c) x = a; else x = b;` into `x = b; if (c) x = a;`, and only then can the store-flag gate at tools/gcc-2.7.2/jump.c:1178-1181 see a CONST_INT. On the un-normalised two-arm shape the store-flag block fails its own precondition at tools/gcc-2.7.2/jump.c:1065-1068 AND reg_set_last (tools/gcc-2.7.2/rtlanal.c:886-888) returns 0 at the selection_sound CODE_LABEL - so blocking the normalisation alone is sufficient.

- [s7] The s6 'single stated tension' (a0=4 must be both inside the branch's basic block for reorg's backward fill and invisible to reg_set_last) applies only to the single-arm `x = 4; if (c) x = 0;` chassis. On the two-arm chassis target's li a0,4 is stolen from the branch's TARGET thread, so the arm never needs to live in the branch's own basic block and the two requirements are not in conflict.

- [s7] PASS-INPUT ENUMERATION (hypotheses.md H19): eleven C-visible preconditions of tools/gcc-2.7.2/jump.c:754-860 with file:line predicates. B2 (jump.c:775) and B6 (jump.c:784-789) - an arm of 2+ insns defeats the normalisation, which is exactly why every duplicated-call spelling escapes at 7/211 while every one-insn-arm spelling folds at 10/205. B10 (jump.c:833) - the condition may not mention the result pseudo - is the floor-6 baseline's escape and the direct cause of its lbu a0 seat. B4 (jump.c:766-768, branch-target arm sourced from MEMORY), B8 (jump.c:794-812,831, unresolvable LABEL_NUSES on the else-label) and B9 (jump.c:832, a CODE_LABEL between the last condjump and the then-arm's terminating jump) are untried.

- [s7] find_cross_jump (tools/gcc-2.7.2/jump.c:2403) walks i2 = PREV_INSN(e2) skipping only NOTEs and CODE_LABELs, so the insn PHYSICALLY PRECEDING a jump's label decides the cross-jump match depth. That is the cause behind the s6 symptom: with the trailing `goto confirm;` present, jump.c threads both duplicated-call arm jumps past the if/else join to `confirm`, whose predecessor is case 2's call (1-insn match); remove it and the own label is the join, whose predecessor is the sibling arm (3-insn match).

- [s7] PROVED in trace: with the selection block relocated to immediately before `confirm:`, tmp/grind/func_800747D8/s7/dumps/xjdbg.txt:1901-1907 shows `enter e1=406 e2=421 min=1 (own-label)` followed by three MATCH lines (i1=404/i2=418 parallel; i1=402/i2=416 set(reg<-127); i1=400/i2=414 set(reg<-127)), `PAT-MISMATCH i1=398 set(reg<-4) vs i2=412 set(reg<-0) lose=0`, and `DO_CROSS_JUMP jump=406 newjpos=400 newlpos=414` - jal / a2=127 / a1=127 merged, mismatching only at a0=4 vs a0=0.

- [s7] The relocation that buys that merge costs +9 insns of layout: the single-call control moves 10/205 -> 29/214, and the duplicated-call form moves 7/211 -> 32/217 (the +9 minus the 3 the merge recovers). Both banked in memory/grind/func_800747D8/rejected/.

- [s7] Target's layout excludes the duplicated-call family on this chassis: the selection block sits FIRST, before case 1's label (asm/funcs/func_800747D8.s:107-118 precede jlabel .L80074984 at :120), and its path ends addiu a1,zero,0x7F / j .L80074A20 / addiu a2,zero,0x7F with the single jal func_8005C650 shared at .L80074A20 (:115-117, :161-162) - one a1/a2 pair for the whole block plus a 1-insn own-label merge at the shared jal. Target's join is itself followed by a jump, i.e. the very configuration that threads arm jumps away, so target's selection block is the SINGLE-call two-arm form.

- [s7] Mechanism probe (not a candidate, deliberately not semantics-preserving): `if (field64 != 0 || field65 != 0) sound = 0; else sound = 4;` measures 10/205 with hunks 18/19 reading lhu a0,100(v0) / sltiu a0,a0,1 / sll a0,a0,0x2 - the two ADJACENT byte tests coalesce into one halfword comparison, so no TRUTH_ORIF drop-through CODE_LABEL is emitted and predicate B9 is never exercised.

- [s7] KILL RE-AUDIT: variant F (tmp/grind/func_800747D8/s6/var_F.c) re-measures score 8 / build_insns 208 on the current chassis, unchanged from s6. tools/fake_ablate.py reports zero FAKE-annotated constructs in candidate.c, so no banked kill on this ledger was ever taken with a FAKE carrier occupying a target pseudo.

- [s7] Hunk 9's jtbl_80015A0C table-offset residual is unchanged and remains a cross-file final-submission step (delete the hand-transcribed array from src/text1a_b_mid_rodata.c in the same change that lands the C body), per the s2 analysis and the executed func_8006B578/jtbl_80015988 precedent.

- [s7] src/text1b.c restored to INCLUDE_ASM("asm/funcs", func_800747D8); at session end per asm-until-matched; candidate.c unchanged (still the floor-6 same-variable form, zero FAKE constructs); git status shows nothing outside memory/grind/func_800747D8/, tmp/ and the pre-existing metrics/events.jsonl drift.

- [s8] rederive: fresh m2c decompile of asm/funcs/func_800747D8.s is blocked (jr at line 74 needs a jump-table input, unrelated to the selection_sound residual) - not retried this session since supplying it means editing src/text1a_b_mid_rodata.c, out of this session's file scope.
- [s8] Mechanism probe (not a candidate, deliberately not semantics-preserving): replacing the selection_sound else-arm's constant `0` with a MEMORY read (`sound2 = D_800A35D0;`) while keeping test/result on separate pseudos (`flag2`/`sound2`) measures score 4, build_insns 209 (1 LONG) and reproduces target's exact v0(test)/a0(result) register seat - every previously-flagged register-seat hunk disappears, leaving only the one expected source-level hunk from the deliberately-wrong value's extra load. Full probe: tmp/grind/func_800747D8/s8/var_G_mem_source.c.
- [s8] CLASS KILL: predicate B4 (tools/gcc-2.7.2/jump.c:1052-1054 REG/SUBREG/CONST_INT gate feeding the jump.c:1190-1191 disjunct) can never be the real function's blocker, because target's real else-arm value is a literal `0`, which is CONST_INT pre-reload and trivially satisfies the gate. B4 is therefore proven NOT the missing precondition for ANY semantics-preserving spelling of this exact value-flow (a two-constant {4,0} selection) - only B8/B9-class predicates (tied to the shared call-tail shape at `.L80074A20`, not to the else-arm's value) remain live for a future session. predicate_cite: tools/gcc-2.7.2/jump.c:1190-1191.
- [s8] src/text1b.c restored to INCLUDE_ASM("asm/funcs", func_800747D8); at session end per asm-until-matched; candidate.c unchanged (still the floor-6 same-variable form, zero FAKE constructs); git status shows only memory/grind/func_800747D8/hypotheses.md + the pre-existing metrics/events.jsonl drift.

- [s8] Chassis re-confirmed at floor 6 (score 6, build_insns 208) with candidate.c applied verbatim before any new probe.

- [s8] sandbox --disable all --diff on the current chassis: 2 source-level hunks (18/19, the a0-vs-v0 inversion) + 2 operand-only hunks (9, the jtbl base offset; 17, lbu a0 vs lbu v0) + 26 not-scored masked branch/jump-target hunks - unchanged classing from s7.

- [s8] m2c fresh decompile of func_800747D8.s fails without a jtbl input for an unrelated jr at line 74; not pursued further this session since fixing it means editing src/text1a_b_mid_rodata.c (out of scope; the hunk-9 jtbl residual is already a documented final-submission-only cross-file step from prior sessions).

- [s8] The MEM-sourced mechanism probe (flag2/sound2 with D_800A35D0 as the else value) measures score 4 / build_insns 209 and reproduces target's exact v0/a0 register seat, proving the seat difference in every prior distinct-pseudo attempt was a side effect of the branchless store-flag fold's own codegen, not an independent allocation question.

- [s8] Predicate B4 (jump.c:1052-1054 / jump.c:1190-1191) is proven satisfied-by-construction for target's real literal-0 else value, so it can never be the blocking predicate for a real spelling of this block; B8/B9 remain the only live untried predicates, now understood to be about the shared call-tail shape (.L80074A20, entered from >=2 sites) rather than about the else-arm's value.

- [s8] Sibling ledgers re-checked per dispatch: func_8006B578 (floor 2, unspent since its s3) already transplant-checked at this ledger's s6 with no useful selection_sound idiom; main/ings.c and func_800692C0 (both COMPLETED-C, on-main bodies) already compared at s6 with no shared construct. No new sibling movement since s6/s7 last-mention.

- [s8] src/text1b.c ends this session back at INCLUDE_ASM("asm/funcs", func_800747D8); candidate.c unchanged (still the floor-6 same-variable-reuse form, zero FAKE constructs); git status --short shows only memory/grind/func_800747D8/hypotheses.md and memory/grind/func_800747D8/evidence.md plus the pre-existing metrics/events.jsonl drift.
