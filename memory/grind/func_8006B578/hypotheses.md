# Hypothesis ledger — func_8006B578

## H1 — CONFIRMED: real 2-param signature + full structural translation closes most of the gap
Statement: the stale 4-param placeholder signature was wrong; the real call site proves exactly
2 pointer params, and writing the full FSM body (see evidence.md structure) in ordinary C, with a
real `switch(hi)` for the `ret>>16` dispatch and `(u32)` casts on the two `D_800A34F8 >> 10` field
reads, closes score 200 → 34 (199/200 insns).
Mechanism: plain C-to-asm translation; no codegen lever involved except the switch-vs-ifchain
branch-sense fix (documented technique, `switch-vs-ifchain-branch-sense` in
codegen-technique-index.md) and signed/unsigned shift correctness (ordinary C typing, not a lever).
Probe: `sandbox func_8006B578 --disable all` before/after each incremental change.
Result: CONFIRMED — score dropped 200→71→35→34 across three edits, each edit independently
verified by re-running sandbox.
kill_scope: n/a (this is a confirm, not a kill).

## H2 — KILLED (instance): per-case inline `*arg1&0x400040` check in switch cases 0/1/2 is wrong
Statement: an early draft added `if (*arg1 & 0x400040) { func_8005C650(1,0x7F,0x7F); goto
do_field_inc; }` inside switch cases 0, 1, and 2 (believing each case needed its own flag check
before falling into the shared field-increment code). Measuring the diff showed cc1 emitting a
large DUPLICATED copy of the shared post-switch block (visible as an extra ~19-instruction insert
in `sandbox --diff` hunk 25 of the prior measurement), because the goto-fallthrough plus the
post-switch block's OWN identical check caused the check+call+field-update to execute twice for
cases 0/1/2 when the flag was set. Removing the inline check/goto from cases 0/1/2 (letting them
simply `break;` and fall through naturally into the single existing post-switch block) dropped
score 35→34 and insn count 218→199.
kill_scope: instance
measured_on: func_8006B578, src/text1b.c, ordinary C only (no FAKE/cheat constructs present in
either the killed or surviving form)

## Live frontier (<=3, for next session)
1. **The check+call PREFIX shared by case3/case4/case5/fallback is unified by our compile but NOT
   by target** (confirmed this session by reading `asm/funcs/func_8006B578.s` lines 136-198
   directly: target has 4 separate inline copies of `if (*(u32*)arg1&0x400040){func_8005C650(1,
   0x7F,0x7F); ...}` — one at `.L8006B760` (fallback), one each at `.L8006B7B4`/`.L8006B7FC`/
   `.L8006B828` (case3/4/5) — while our build shares one copy, reached via jumps, per
   `sandbox --diff` hunks 15-16). H3 this session showed the "mix the FINAL exit form" recipe
   from `cross-jump-store-tail-merge.md` does NOT fix this (made it worse both times — see H3) —
   the shared code is a PREFIX of each arm, not a suffix immediately before a shared final jump,
   so `find_cross_jump`'s backward-from-final-jump framing may be the wrong pass to suspect
   entirely. Next probe: `pwsh tools/grinder/dump.ps1 func_8006B578` was run this session
   (tmp/grind/func_8006B578/dumps/text1b.jump2 etc exist) but NOT yet read in detail for this
   specific region (turn budget) — grep `text1b.combine` and `text1b.jump2` around the
   `func_8006B578` function boundary (starts line 53481 in `.jump2`) for the actual RTL basic
   blocks in this region and see which pass (combine's CSE-like insn sharing vs jump2's actual
   cross-jump vs cse.c re-materialization) is doing the merge — do not guess the pass again
   without reading it.
2. **We are 199 insns vs target's 200 (one UNDER)** at the CURRENT floor-34 candidate (unchanged
   this session). Check whether target keeps an explicit `nop` (delay-slot filler) that our
   current structure optimizes away. Compare insn-for-insn around the hunk-16/17 region (see H3's
   probe for context — hunk 17 shows target keeps a bare `nop` where ours has `li s2,2`).
3. Case 3's tail store/const ordering (`D_800A34F8 = ...; var_s2 = 2;`) reorder was NOT
   independently re-tried this session (superseded by the H3 experiments, which changed more than
   just this ordering) — still open if #1 and #2 don't pan out, but should come AFTER #1 since #1
   is the dominant source-level gap (7 of 25 hunks, the rest are masked/not-scored).

## SESSION 3 UPDATE — frontier items 1-3 above are RESOLVED, replaced by ONE new item

All three frontier items above were closed this session (34 -> 2) as side
effects of two structural fixes (see H4/H5/H6 below); none needed independent
chasing. The updated, current live frontier is:

1. **[SOLE REMAINING ITEM] `jtbl_80015988`'s cross-TU declaration is the only
   non-zero residual left (score 2, 0 source-level/operand-only hunks in
   `sandbox --diff`).** It's declared as `const u32 jtbl_80015988[6]` in
   `src/text1a_b_pre_rodata.c:409` (a DIFFERENT TU than `func_8006B578`'s home
   `src/text1b.c`), and every one of its 6 entries is a case-label address
   belonging exclusively to `func_8006B578` (single-owner, grep-confirmed —
   see evidence.md Session 3). A normal C `switch` can never make GCC
   reference an external symbol for its jump table (GCC always emits its own
   into the compiling TU's `.rodata`) — this is the
   `jtbl-rodata-split-infrastructure` shape. Fix per
   `no-new-park-categories.md`'s evidence-based-reattribution carve-out: move
   the `jtbl_80015988` array OUT of `src/text1a_b_pre_rodata.c` (delete it
   there) so GCC's own switch-generated table (which this session's candidate
   already produces byte-identically in shape) lands at the correct `.rodata`
   slot. This is a TWO-FILE edit and this session's mandate was
   `src/text1b.c` only — the next session (or a `ruling-request` asking
   whether the cross-file move is in-scope for a single-function grind) should
   execute it. `memory/grind/func_8006B578/candidate.c` is 200/200 insns
   matched; applying it plus deleting the `jtbl_80015988` block in
   `src/text1a_b_pre_rodata.c` is EXPECTED to close this to 0 — VERIFY with
   `sandbox --disable all` before assuming, not by inference.

## H3 — KILLED (instance): mixed exit forms (goto-distinct-label + inline-return mix) on cases 3/4/5 do NOT close the case3/4/5 -> shared-tail transition, and make it WORSE
Statement: per `.claude/rules/cross-jump-store-tail-merge.md` ("target has more insns than your
build because jump2 cross_jump merged N identical tails -> mix the exit forms: distinct-label
goto for some arms + one inline `return` for another, so block ENDINGS differ and the suffix
match breaks"), I diagnosed `sandbox --diff` hunks 15-24 (all classed source-level) as this exact
symptom: target's asm (`asm/funcs/func_8006B578.s` .L8006B7B4/.L8006B7FC/.L8006B828, lines
136-198) shows FOUR separate inline copies of the `if (*(u32*)arg1 & 0x400040) { func_8005C650(1,
0x7F, 0x7F); ...}` check+call (one per case 3/4/5 plus one for the `>=6` fallback path), while our
build (floor-34 baseline) collapses them into a shared preamble reached via `j` (visible as the
extra `j 96a0; lui v1,0x40` insert at hunk 15 and the giant target-block-vs-tiny-ours mismatch at
hunk 16). Applying the rule's literal recipe — case 3 -> `goto end3` (distinct label re-doing the
shared 0x100010 tail check + return), case 4 -> full inline duplicate of the 0x100010 tail
check + return (no goto at all), case 5 -> unchanged `goto tail` — made the build WORSE: score
34 -> 55, insn count 199 -> 202 (MORE insns than target's 200, not fewer). A second variant —
giving ALL THREE of case 3/4/5 their own fully-inlined duplicate of the 0x100010 tail check +
return (no `goto tail` anywhere, so no two cases share ANY jump target) — was also worse: score
34 -> 44, insn count 199 -> 202.
Mechanism: unclear — NOT simply jump2 `find_cross_jump` on the `goto tail` suffix the way the
rule's `saEft00Add` case worked. The 0x400040 check+call block sits at the *front* of each case
arm (a shared *prefix*, not a shared *suffix* immediately before the final jump — the differing
per-case action, i.e. `D_800A34F8=...;var_s2=2;` etc., comes AFTER the check but BEFORE the exit),
so mixing only the FINAL exit form does not touch whatever pass/mechanism is actually unifying the
check+call prefix across arms; instead it added net-new instructions (each mixed form duplicated
the 0x100010 tail-check code an extra time, which is real cost the rule's own recipe pays in the
saEft00Add case too, but there the duplicated part was cheap (2 insns) — here it's a 3-4 insn
block duplicated 2-3x, netting worse than the single merged copy).
Probe: `sandbox func_8006B578 --disable all` after each of the two mixed-form variants; reverted
both (byte-identical to the pre-existing floor-34 candidate.c after revert, re-measured 34).
Result: KILLED — this specific application of the cross-jump-store-tail-merge recipe (mixing ONLY
the final exit form of cases 3/4/5) is not the lever for this residual; whatever is unifying the
check+call PREFIX across the 4 sites (case3/4/5 + fallback) needs a different C-structural
diagnosis before another attempt (see frontier #1, still open, now sharpened: the merge is
prefix-shaped, not suffix-shaped, so `find_cross_jump`'s backward-suffix-walk framing from the
rule doc may not even be the right pass to suspect — try a fresh `.jump2`/`.combine` dump read
attributing the ACTUAL pass, not the by-analogy guess this session made).
kill_scope: instance
measured_on: func_8006B578, src/text1b.c, ordinary C only (no FAKE/cheat constructs in either the
killed variants or the surviving floor-34 form); both variants reverted, floor re-confirmed at 34
this session before writing this entry.

## Judge constraints / banned constructs
None yet — no FAKE/cheat construct has been used or proposed in this ledger. The whole candidate
so far is ordinary C (switch statements, casts, local scratch vars for real intermediate values).

## [s1] The pre-existing 4-param placeholder signature was wrong (only real call site passes 2 pointer args); writing the full FSM in ordinary C with a real switch(hi) for the ret>>16 dispatch and (u32) casts on the two D_800A34F8>>10 field reads closes most of the gap.
- mechanism: Ordinary C translation from asm/funcs/func_8006B578.s, corroborated against sibling func_8006B92C's near-identical shape in the same TU; switch-vs-ifchain branch-sense fix (documented technique) plus signed/unsigned shift correctness for the srl-vs-sra divergence.
- probe: sandbox func_8006B578 --disable all, re-run after each incremental edit
- result: score 200 -> 71 -> 35 -> 34 across three successive edits, each independently measured
- verdict: CONFIRMED

## [s1] Adding an inline `if (*arg1 & 0x400040) {...; goto do_field_inc;}` check inside switch cases 0/1/2 (in addition to the existing shared post-switch check they fall into) is wrong and causes cc1 to emit a duplicated ~19-instruction copy of the shared field-update+call block.
- mechanism: The goto-into-shared-label plus the post-switch block's own identical re-check caused the check+call+field-update to run twice for cases 0/1/2 when the flag was set; removing the inline per-case check (matching the asm, which has exactly ONE 0x400040 check per case-0/1/2 path, not two) restored the correct single-copy structure.
- probe: sandbox --disable all --diff, compared hunk-by-hunk insert/delete counts before and after removing the duplicate check
- result: score 35 -> 34, insn count 218 -> 199 (matches target's 200 to within 1)
- verdict: KILLED
- kill_scope: instance
- measured_on: func_8006B578, src/text1b.c, ordinary C only, no FAKE/cheat constructs present in either the killed or surviving form

## [s2] The stale 4-param placeholder signature was wrong; the real call site proves exactly 2 pointer params, and writing the full FSM in ordinary C with a real switch(hi) for the ret>>16 dispatch and (u32) casts on the two D_800A34F8>>10 field reads closes most of the gap.
- mechanism: Ordinary C translation from asm/funcs/func_8006B578.s; switch-vs-ifchain branch-sense fix + signed/unsigned shift correctness.
- probe: sandbox func_8006B578 --disable all, re-run after each incremental edit
- result: score 200 -> 71 -> 35 -> 34 across three successive edits in session 1
- verdict: CONFIRMED

## [s2] Adding an inline `if (*arg1 & 0x400040) {...; goto do_field_inc;}` check inside switch cases 0/1/2 (in addition to the existing shared post-switch check) causes cc1 to emit a duplicated ~19-instruction copy of the shared field-update+call block.
- mechanism: The goto-into-shared-label plus the post-switch block's own identical re-check caused the check+call+field-update to run twice for cases 0/1/2 when the flag was set.
- probe: sandbox --disable all --diff, compared hunk-by-hunk insert/delete counts before and after removing the duplicate check
- result: score 35 -> 34, insn count 218 -> 199
- verdict: KILLED
- kill_scope: instance
- measured_on: func_8006B578, src/text1b.c, ordinary C only, no FAKE/cheat constructs present in either the killed or surviving form

## [s2] Mixing the FINAL exit form of switch cases 3/4/5 (goto-to-distinct-label for one, full inline duplicate of the shared 0x100010 tail-check+return for another, unchanged goto-tail for the third) per the cross-jump-store-tail-merge.md recipe closes the case3/4/5-to-shared-tail source-level diff hunks (15-24 of sandbox --diff).
- mechanism: Hypothesized jump2 find_cross_jump backward-suffix merge of the identical `if (*(u32*)arg1&0x400040){func_8005C650(1,0x7F,0x7F);...} goto tail;` bodies across the three case arms, defeated by making the block ENDINGS differ so the suffix match breaks (same recipe that closed saEft00Add's 3-way store-tail merge).
- probe: Applied the mix (case3->goto end3 re-doing the 0x100010 check+return, case4->full inline duplicate of the 0x100010 check+return with no goto, case5->unchanged goto tail); sandbox func_8006B578 --disable all before/after.
- result: score 34 -> 55, insn count 199 -> 202 (worse in both score and raw count vs the 200-insn target); reverted, re-measured 34 to confirm restoration.
- verdict: KILLED
- kill_scope: instance
- measured_on: func_8006B578, src/text1b.c, ordinary C only, no FAKE/cheat constructs present in either the killed variant or the restored floor-34 form

## [s2] Giving ALL THREE of switch cases 3/4/5 their own fully-inlined duplicate of the shared 0x100010 tail-check + return (removing every goto-tail so no two cases share any jump target at all) closes the same source-level diff hunks.
- mechanism: Same cross-jump-suffix-defeat hypothesis as the prior variant, taken further: zero shared final destinations among the three arms.
- probe: Applied full triplication (case3/4/5 each end with their own if(0x100010){call;var_s2=1;} return var_s2;, no goto tail anywhere for these three); sandbox func_8006B578 --disable all before/after.
- result: score 34 -> 44, insn count 199 -> 202; reverted, re-measured 34 to confirm restoration.
- verdict: KILLED
- kill_scope: instance

## H4 — CONFIRMED (s3): idx>=6 must `goto tail` BEFORE the switch, skipping the shared 0x400040 block entirely (semantic bug fix, not a lever)
Statement: reading `asm/funcs/func_8006B578.s` line 81 (`sltiu v0,v1,0x6; beqz v0,.L8006B850`)
directly proves idx>=6 branches STRAIGHT to the final tail check
(`.L8006B850`/`.L8006B854`, the `*arg1&0x100010` block) and never executes the
shared `*arg1&0x400040` field-increment code that case 0/1/2 fall into. The
s1/s2 candidate's `if (idx<6){switch}` followed unconditionally by the shared
0x400040 `if` block let idx>=6 fall through into that shared code too — a
genuine semantic error, confirmed by direct objdump comparison (OLD structure
compiled idx>=6's branch to `beqz v0,969c`, landing INSIDE the shared block;
NEW structure — `if (idx>=6) goto tail;` written before the switch instead of
wrapping the switch in `if (idx<6){...}` — compiles to `beqz v0,96e8`, the
tail's own check, matching target's `.L8006B850` target exactly).
Mechanism: ordinary C control-flow correction — the ORIGINAL translation had
the wrong reachability for the idx>=6 path, not a codegen/scheduling issue.
Probe: `sandbox func_8006B578 --disable all` before/after; also independently
verified via raw objdump of `tmp/sandbox/func_8006B578/text1b.o` before and
after the edit (`mipsel-linux-gnu-objdump -d --disassemble=func_8006B578`),
confirming the branch target literally changed from 969c to 96e8.
Result: CONFIRMED — score unchanged at 34 alone (this fix's effect was masked
by frontier item 2 below still being unfixed at that point), but insn-level
objdump confirmed the control-flow correctness; combined with H5 the score
dropped 34 -> 11 -> 2.
kill_scope: n/a (this is a confirm, not a kill).
measured_on: func_8006B578, src/text1b.c, ordinary C only, no FAKE/cheat
constructs present.

## H5 — CONFIRMED (s3): the shared 0x400040+field-increment block must be written INSIDE the switch, as case 2's fallthrough consequence (physical layout fix, not a lever)
Statement: target's asm lays out the shared `*arg1&0x400040` check+field-update
block (`.L8006B760`) PHYSICALLY BETWEEN case 2's body and case 3's body —
case 0 and case 1 each end with an explicit `j .L8006B754` (the toggle-call
merge point), case 2 falls into `.L8006B754` naturally (no jump), `.L8006B754`
flows straight into `.L8006B760` with NO branch in between, and `.L8006B760`
finishes with `j .L8006B854` (skipping `.L8006B850`'s now-redundant `lui`) —
ONLY THEN does the asm lay out case 3/4/5 (`.L8006B7B4`/`7FC`/`828`). The
s1/s2 candidate wrote the shared code AFTER the switch's closing brace in
source, so GCC (which lays out switch case bodies in source/case order)
placed case 3/4/5 BEFORE the shared code — requiring an extra `j 96a0; lui
v1,0x40` glue instruction to reach it from case 0/1/2's fallthrough (the exact
"hunk 15 extra insert" flagged at floor 34, and the giant hunk-16 mismatch
where target showed ~20 real instructions our build didn't have at that
position). Fix: moved the shared block INSIDE the switch as case 2's
fallthrough — case 0/1/2 use `goto shared_400040;` (case 2 falls through
naturally into a `shared_400040:` label placed immediately after its own
body, before `case 3:`). This reproduces target's physical layout exactly.
Mechanism: GCC 2.7.2's switch-statement code generation lays out case bodies
in source declaration order (ordinary, well-documented compiler behavior,
not an exotic pass); the fix is a pure C source-order/structure correction —
no lever, no FAKE construct, just writing the C the way the original author
apparently did (shared consequence written as a fallthrough INSIDE the
switch, not factored out after it).
Probe: `sandbox func_8006B578 --disable all` before/after (34 -> 11 combined
with H4); `sandbox --diff` re-run to confirm hunk count/class before/after.
Result: CONFIRMED — combined with H4, score 34 -> 11, insns 199 -> 200
(target's exact count reached for the first time this ledger).
kill_scope: n/a (this is a confirm, not a kill).
measured_on: func_8006B578, src/text1b.c, ordinary C only (the `goto
shared_400040;`/fallthrough-into-a-switch-internal-label construct is
ordinary C — a `goto` into a switch's own case body to reach a genuinely
shared consequence has real semantic effect and is not a no-purpose
construct), no FAKE/cheat constructs present.

## H6 — CONFIRMED (s3): the shared field-increment must compute the MASK before the SHIFT/FIELD, matching case 1/2's own statement order
Statement: after H4+H5 (score 11), the sole remaining source-level hunk was a
register-allocation-shaped mismatch in the shared block's field-increment:
the candidate wrote `u32 f2=D_800A34F8; s32 c2=((f2>>10)&7)+1;
D_800A34F8=(f2&~0x1C00u)|((c2&7)<<10);` (computing the shift/field part
before the mask part), while target's asm computes the mask AND first
(`li v1,-7169; and v1,v0,v1`) THEN the shift (`srl v0,v0,0xa; ...`), reusing
v0 for the field after v1 already holds the masked base — exactly mirroring
case 1/2's OWN increment/decrement statement order
(`u32 m = a0 & ~0x1C00; s32 c = ((a0>>10)&7)+1; m |= (c&7)<<10;`). Rewriting
the shared block to compute the mask first (`u32 m2 = f2 & ~0x1C00u;` before
`s32 c2 = ...`) closed this without any register-pin or reordering trick.
Mechanism: ordinary C statement-order match — cc1's natural register
allocation for two independent sub-expressions computed from the same source
value follows source order when nothing else disambiguates it; writing the
statements in the same order as the (structurally identical) case-1/2 logic
reproduces the same register choices.
Probe: `sandbox func_8006B578 --disable all` before/after; `tmp/grind/func_8006B578/diag_score.py`
(engine.score.normalized_insns diff) confirmed 0 remaining differing
normalized instructions in the function body after this fix (the only 2
differing instructions left are the jtbl relocation masking artifact, see H7
below, unrelated to this statement).
Result: CONFIRMED — score 11 -> 2, insns 200/200 (target's exact count),
0 source-level hunks, 0 operand-only hunks in `sandbox --diff`.
kill_scope: n/a (this is a confirm, not a kill).
measured_on: func_8006B578, src/text1b.c, ordinary C only, no FAKE/cheat
constructs present.

## H7 — CONFIRMED (s3): the residual score-2 is a cross-TU jump-table-symbol scorer artifact, not a source-level C defect
Statement: `tmp/grind/func_8006B578/diag_score.py` isolated the ENTIRE
remaining score-2 residual to exactly 2 normalized instructions — the
switch's jump-table address load (`lui at,@.rodata` / `lw v0,@.rodata(at)`
in our build vs `lui at,0x0` / `lw v0,0(at)` in the reference/target build).
`nm` on both objects confirms: `jtbl_80015988` (the literal 6-entry
case-label address table) is DEFINED in `build/src/text1a_b_pre_rodata.o`
(`00000598 R jtbl_80015988`) — a DIFFERENT translation unit than
`func_8006B578`'s home file — while `build/src/text1b.o` references it as
`U jtbl_80015988` (undefined/external). Every one of the 6 table entries
(0x8006B6B8, 0x8006B6E8, 0x8006B720, 0x8006B7B4, 0x8006B7FC, 0x8006B828) is a
case-label address belonging exclusively to `func_8006B578` — grep-confirmed
single-owner, no other function or file references `jtbl_80015988` or any of
its 6 entry values. A normal C `switch` statement can NEVER make GCC
reference a foreign externally-declared symbol for its jump table (GCC always
synthesizes its own local jump table into the compiling TU's `.rodata`,
referenced via a section-relative relocation — which the sandbox scorer
correctly masks to `@.rodata` since that's normally harmless layout noise).
The reference build (still assembled from the original hand-targeted asm,
not from C) references `jtbl_80015988` BY NAME, which the scorer can't
resolve to an address (the symbol isn't in `named_syms.txt` /
`undefined_syms_auto.txt`), so it shows as an unresolved literal `0x0`
instead of a matching normalized token. This is the documented
`jtbl-rodata-split-infrastructure` no-C-form shape: fixable only by an
evidence-based cross-TU move of the `jtbl_80015988` declaration out of
`src/text1a_b_pre_rodata.c` into `src/text1b.c` (or its deletion there once
the switch naturally regenerates it), which is a two-file edit outside this
session's single-file mandate.
Mechanism: engine/score.py's HI16/LO16 symbol resolution requires the symbol
to be present in `LD_SYM_FILES` (`named_syms.txt`, `undefined_syms_auto.txt`,
`undefined_funcs_auto.txt`); `jtbl_80015988` is a splat-detected rodata dlabel
never added to those files, so `_resolve_named_pair` silently leaves the
reference's raw pre-link immediate (0) unresolved instead of normalizing it —
a genuine engine limitation (`score-symtab-blind-to-asm-data-dlabels`,
project memory), not a source-level defect in our C.
Probe: `tmp/grind/func_8006B578/diag_score.py` (direct call into
`engine.score.normalized_insns` for both objects, masked); `nm` on
`build/src/text1b.o` and `build/src/text1a_b_pre_rodata.o`; grep across
`src/*.c` and `asm/funcs/*.s` for any OTHER reference to `jtbl_80015988` or
its 6 literal addresses (none found).
Result: CONFIRMED — this is the sole remaining residual and it is NOT a
source-level C defect; it requires a cross-TU rodata-declaration move (see
frontier item 1) to close to 0.
kill_scope: n/a (this is a confirm identifying an infrastructure residual,
not a kill of a C hypothesis).
measured_on: func_8006B578, src/text1b.c + src/text1a_b_pre_rodata.c
(read-only inspection of the latter; no edit made — outside this session's
single-file mandate), ordinary C only, no FAKE/cheat constructs present or
proposed.
- measured_on: func_8006B578, src/text1b.c, ordinary C only, no FAKE/cheat constructs present in either the killed variant or the restored floor-34 form

## [s3] The chassis dispatch line claiming candidate.c measures 34 'right now' was based on a stale assumption; src/text1b.c actually still carried the raw INCLUDE_ASM stub (sandboxes at 200) at session start, and candidate.c had to be re-applied to reproduce 34.
- mechanism: Inter-session src reversion (per asm-until-matched, src is reverted to the stub between grind sessions) combined with a dispatch-line assumption that the ledger's last recorded floor was already live in src.
- probe: sandbox func_8006B578 --disable all immediately after applying candidate.c verbatim
- result: CONFIRMED — reproduced 34 exactly, matching the ledger's prior measurement; the underlying s1/s2 measurements were real, only the dispatch-line assumption was stale.
- verdict: CONFIRMED

## [s3] idx>=6 (the (D_800A34F8>>10&7)>=6 fallback path) must skip the shared *arg1&0x400040 field-increment block entirely and go straight to the final tail check, matching asm/funcs/func_8006B578.s line 81's beqz to .L8006B850; the s1/s2 candidate's if(idx<6){switch} structure let idx>=6 fall through into the shared block, a genuine semantic bug.
- mechanism: Ordinary C control-flow correction (writing `if (idx>=6) goto tail;` before the switch instead of wrapping the switch in `if (idx<6){...}`), confirmed by direct objdump comparison of the compiled branch target (969c before the fix, 96e8 after, matching target's .L8006B850 destination).
- probe: sandbox func_8006B578 --disable all before/after; independent objdump -d --disassemble=func_8006B578 on tmp/sandbox/func_8006B578/text1b.o comparing the branch target at the idx-dispatch sltiu/beqz pair
- result: CONFIRMED — combined with the H5 layout fix, score dropped 34 -> 11 -> 2; objdump directly confirmed the branch-target correction.
- verdict: CONFIRMED

## [s3] The shared *arg1&0x400040 field-increment block must be written INSIDE the switch as case 2's fallthrough consequence (case 0/1 goto a shared_400040: label placed immediately after case 2's own body, before case 3), not after the switch's closing brace as in the s1/s2 candidate — this matches target's physical instruction layout (case0/1/2 -> shared block -> case3/4/5), which GCC's source-order switch-body layout otherwise cannot reproduce.
- mechanism: GCC 2.7.2's ordinary switch-statement code generation lays out case bodies in source/case declaration order; moving the shared consequence's C source position (not its logic) into the position between case 2 and case 3 reproduces target's block ordering with no extra glue jump.
- probe: sandbox func_8006B578 --disable all before/after; sandbox --diff hunk-count/class comparison before and after (removed the extra 'j 96a0; lui v1,0x40' glue insert flagged at floor 34, hunk 15)
- result: CONFIRMED — combined with H4, score 34 -> 11, insns 199 -> 200 (target's exact count reached for the first time this ledger).
- verdict: CONFIRMED

## [s3] The shared block's field-increment must compute the bitmask (f2 & ~0x1C00) before the shift/field extraction ((f2>>10)&7)+1, matching case 1/2's own statement order, instead of computing the shift first as the s1/s2 candidate did.
- mechanism: Ordinary C statement-order match — cc1's natural register allocation for two independent sub-expressions computed from one source value follows source order; writing the statements in the same order as the structurally identical case-1/2 logic reproduces the same register choices with no pin or barrier.
- probe: sandbox func_8006B578 --disable all before/after; tmp/grind/func_8006B578/diag_score.py (direct engine.score.normalized_insns diff) confirming 0 remaining differing normalized instructions in the function body proper after this fix
- result: CONFIRMED — score 11 -> 2, insns 200/200, 0 source-level hunks, 0 operand-only hunks in sandbox --diff.
- verdict: CONFIRMED

## [s3] The sole remaining score-2 residual is a cross-TU jump-table-symbol scorer artifact (jtbl_80015988 defined in build/src/text1a_b_pre_rodata.o, a different TU than func_8006B578's home text1b.c, referenced there as an undefined external symbol not resolvable by engine/score.py's LD_SYM_FILES-based symbol table), not a source-level C defect reachable by any switch-statement spelling in text1b.c alone.
- mechanism: engine/score.py's _resolve_named_pair requires the referenced symbol's address in LD_SYM_FILES (named_syms.txt/undefined_syms_auto.txt/undefined_funcs_auto.txt); jtbl_80015988 is a splat-detected rodata dlabel never added there, so the reference build's HI16/LO16 pair against it stays an unresolved raw literal (0) instead of a normalized token, while our build's GCC-generated local jump table is correctly section-relative-masked (@.rodata) — the two forms can never textually match under the current scorer regardless of C content, because GCC can never emit a switch that references a foreign externally-declared jump-table symbol by name.
- probe: tmp/grind/func_8006B578/diag_score.py (engine.score.normalized_insns on both build and reference objects, masked, diffed line-for-line); nm on build/src/text1b.o (U jtbl_80015988) and build/src/text1a_b_pre_rodata.o (00000598 R jtbl_80015988); grep across src/*.c and asm/funcs/*.s for any other reference to jtbl_80015988 or its 6 literal case-label addresses (single-owner confirmed, none found)
- result: CONFIRMED — this is the entire remaining residual and is a jtbl-rodata-split-infrastructure shape requiring an evidence-based cross-TU move of the jtbl_80015988 declaration out of src/text1a_b_pre_rodata.c into src/text1b.c to close, which is a two-file edit outside this session's single-file (src/text1b.c only) mandate.
- verdict: CONFIRMED

## H8 — KILLED (instance, s4/permuter modality): no permuter mutation surface exists at the current candidate — a campaign would be a wasted session
Statement: re-applied candidate.c verbatim to src/text1b.c (reverted to the
INCLUDE_ASM stub between sessions per asm-until-matched) and re-measured
before doing anything else. `sandbox --disable all` reproduced score 2
exactly, no drift from s3. `sandbox --disable all --diff` shows 22 hunks: 0
source-level, 0 operand-only, 22 not-scored — every displayed difference is a
masked branch/jump-target artifact the scorer does not count. A fresh
`tmp/grind/func_8006B578/s4/diag_masked.py` (calling
`engine.score.normalized_insns(..., mask=True)` directly on both
`build/src/text1b.o` and `tmp/sandbox/func_8006B578/text1b.o`) isolated the
masked-form diff to the same 2 instructions H7 already identified: `lui
at,@.rodata`/`lw v0,@.rodata(at)` (ours, a section-relative reloc against our
own GCC-synthesized local jump table) vs `lui at,0x0`/`lw v0,0(at)` (target,
an unresolved named-symbol reloc against jtbl_80015988). There is no
source-level or operand-only residual anywhere in the function body for a
permuter (or any other C-mutation search) to act on. A permuter mutates C
statements inside the function body; it cannot change which TU's .rodata a
compiler-synthesized jump table links into, nor can adding jtbl_80015988 to
named_syms.txt/LD_SYM_FILES fix this (re-read `_resolve_named_pair` +
`_mask_section_addend` in engine/score.py this session: a section-relative
reloc (ours) and a named-symbol reloc (target's) are masked via two
different, never-converging code paths regardless of symtab contents — only
adding the symbol would let the TARGET side resolve to an `@hi/@lo(addr)`
token, while OUR side stays `@.rodata`; they still would not match). No
lever exists for this modality on this residual; the fix is the cross-file
one already on the frontier (H7). No permuter campaign was launched:
launching one against a target with 0 source-level/0 operand-only hunks
would be a mechanically wasted session — no mutation of the function body
can touch the 2 differing instructions, since they differ only in which TU's
rodata they resolve against, not in program logic.
Mechanism: n/a — absence-of-search-surface finding, not a GCC-pass lever.
Probe: `sandbox --disable all --diff` hunk-class breakdown;
`tmp/grind/func_8006B578/s4/diag_masked.py` (fresh normalized-insn diff,
independent of the cached hunk numbers from s3).
Result: KILLED — permuter modality has no viable target on this residual;
confirms H7 stands as the sole remaining gap and it needs the cross-file fix
already on the frontier, not more in-TU search.
kill_scope: instance
measured_on: func_8006B578, src/text1b.c (candidate.c reapplied then reverted
to the INCLUDE_ASM stub before session end, per asm-until-matched), ordinary
C only, no FAKE/cheat constructs present or proposed.

## [s4] No permuter mutation surface exists at the current candidate (score 2, 22/22 hunks not-scored, 0 source-level, 0 operand-only) — a permuter campaign against this residual would be mechanically wasted because the only remaining differing instructions (the switch's jtbl address load) differ purely in which translation unit's .rodata the relocation resolves against, not in program logic reachable by any C mutation inside src/text1b.c.
- mechanism: n/a — absence-of-search-surface finding. engine/score.py masks section-relative HI16/LO16 relocations to '@<section>' and named-symbol HI16/LO16 relocations by resolving them via LD_SYM_FILES (engine/score.py _mask_section_addend / _resolve_named_pair); our build's jump-table load is a section-relative reloc against our own GCC-synthesized local table, target's is a named-symbol reloc against jtbl_80015988 (defined in a different TU, src/text1a_b_pre_rodata.c, and absent from named_syms.txt/undefined_syms_auto.txt/undefined_funcs_auto.txt). These two masking code paths never converge to an equal token regardless of symtab contents, so adding jtbl_80015988 to LD_SYM_FILES alone does not close this either.
- probe: Re-applied candidate.c to src/text1b.c, ran `sandbox func_8006B578 --disable all` (score 2, no drift) and `--diff` (22 hunks: 0 source-level, 0 operand-only, 22 not-scored); wrote and ran tmp/grind/func_8006B578/s4/diag_masked.py (direct engine.score.normalized_insns(mask=True) diff on build/src/text1b.o vs tmp/sandbox/func_8006B578/text1b.o) which isolated the entire masked-form diff to the same 2 instructions H7 (session 3) already identified; re-read engine/score.py's _resolve_named_pair and _mask_section_addend to rule out a named_syms.txt-only fix.
- result: KILLED — confirmed no permuter (or any in-file C mutation) target exists; the residual is entirely the H7 cross-TU jtbl-symbol scorer artifact, requiring the cross-file fix already on the frontier, not more in-TU search.
- verdict: KILLED
- kill_scope: instance
- measured_on: func_8006B578, src/text1b.c (candidate.c reapplied then reverted to the INCLUDE_ASM stub before session end, per asm-until-matched), ordinary C only, no FAKE/cheat constructs present or proposed

## H9 — KILLED (class, s5/structural modality): NO C spelling of the switch in src/text1b.c can ever close the H7 residual, because GCC's jump-table label is ALWAYS a fresh compiler-generated internal symbol, never a reference to an externally-declared name
Statement: re-applied candidate.c to src/text1b.c this session and re-measured
before doing anything else — reproduced score 2 exactly (200/200 insns), no
drift from s3/s4. `sandbox --disable all --diff` (re-run fresh this session,
not read from a cached s4 result) confirms the same shape: 22 hunks, 0
source-level, 0 operand-only, 22 not-scored. This session's addition is a
GCC-SOURCE mechanism citation that upgrades H7/H8's finding from an
empirically-observed instance result to a provable CLASS result: read
`tools/gcc-2.7.2/stmt.c:4704` (`expand_end_case`, the switch-statement
lowering routine) — `table_label = gen_label_rtx();` is called
UNCONDITIONALLY for every switch reaching this code path, with no branch or
parameter that could make it bind to a pre-existing or externally-declared
symbol name instead. `gen_label_rtx()` (see its definition in the same
GCC source tree) always allocates a fresh, function-scoped internal label
(the eventual `.L<N>` in the assembler output); the label — and therefore
the jump table's data, emitted later via `ASM_OUTPUT_ADDR_VEC` at that same
label — can only ever be placed in the CURRENTLY-COMPILING translation
unit's own object output. There is no C source text (no declaration, no
attribute, no pragma available in this GCC fork) that can redirect
`expand_end_case`'s jump-table label to bind to `jtbl_80015988` (a symbol
whose address is fixed by `src/text1a_b_pre_rodata.c`, a DIFFERENT TU).
Therefore: for as long as func_8006B578's switch is compiled from
src/text1b.c, no rewrite of that switch (if-chain conversion, case
reordering, dense-vs-sparse restructuring, splitting into nested switches,
etc.) can make our build's jump-table reference resolve to the same
scorer-visible form as the target's (which references the external
`jtbl_80015988` by name, per H7). The residual is provably NOT reachable by
any C spelling confined to this file; it requires the cross-TU
re-attribution already on the frontier (moving/declaring `jtbl_80015988`'s
backing storage such that GCC's own synthesized table lands there, i.e.
splitting `src/text1a_b_pre_rodata.c` around this jtbl the way
`replay_camera_rob_back_loose2` was resolved in 2026-06-09 — see
[[jtbl-rodata-split-infrastructure]]).
Mechanism: GCC 2.7.2's `expand_end_case` (stmt.c) switch-lowering — the
jump-table label allocation is compiler-internal and unconditional, not a
C-source-controllable choice.
Probe: `sandbox func_8006B578 --disable all` (score 2, reproduced, no drift)
+ `sandbox --disable all --diff` (fresh this session: 22 hunks, 0
source-level, 0 operand-only, 22 not-scored — identical class breakdown to
s4); direct read of `tools/gcc-2.7.2/stmt.c` lines 4685-4714 confirming
`table_label = gen_label_rtx();` at line 4704 has no conditional path to an
external symbol.
Result: KILLED (class) — no C spelling confined to src/text1b.c can close
this residual; the fix is definitively the cross-TU re-attribution on the
frontier, not further in-file search. This does not change the frontier
(H7 already named the correct fix); it forecloses any FUTURE structural or
permuter session from re-deriving "maybe a different switch shape helps"
from scratch.
kill_scope: class
measured_on: func_8006B578, src/text1b.c, ordinary C only (candidate.c
reapplied then reverted to the INCLUDE_ASM stub before session end, per
asm-until-matched), no FAKE/cheat constructs present or proposed.
predicate_cite: tools/gcc-2.7.2/stmt.c:4704

## H9 (s5, structural) — KILLED (instance): local declaration-order reversal is score-neutral
Statement: reversing the order of the four block-local declarations in
func_8006B578 (`s32 var_s2=0; s32 hi; s32 ret; s32 sp10; u32 v;` instead of the
candidate's `u32 v; s32 sp10; s32 ret; s32 hi; s32 var_s2=0;`) produces IDENTICAL
codegen: sandbox --disable all reports score 2, 200/200 insns, unchanged from the
baseline candidate measured earlier this session.
Mechanism: none identified — GCC 2.7.2's `local-alloc`/`global` pass allocation
for this function is apparently insensitive to source declaration order for
these four locals (no LUID-adjacency effect manifested here).
Probe: candidate.c applied to src/text1b.c, `sandbox func_8006B578 --disable all`
before (score 2) and after the reorder (score 2, 200/200 insns both times).
Result: KILLED (instance) — declaration-order reversal measured score 2 -> 2 (no
change), same insn count 200/200, on this exact chassis.
kill_scope: instance
measured_on: func_8006B578, src/text1b.c, ordinary C only, no FAKE/cheat
constructs present in either form, candidate.c reapplied then reverted to the
INCLUDE_ASM stub before session end per asm-until-matched.

## H10 (s5, structural) — KILLED (instance): narrowing `hi` from s32 to s16 is score-neutral
Statement: declaring the switch-dispatch temporary `hi` (holds `ret >> 16`,
range-limited to a 16-bit value by construction) as `s16` instead of `s32`
produces IDENTICAL codegen to the s32 form.
Mechanism: none identified — the value is already effectively narrow at the use
site (`switch(hi)` with case labels 1/2 only), so GCC's constant/type folding
apparently treats the s16 and s32 forms as equivalent for this expression.
Probe: `sandbox func_8006B578 --disable all` after retyping `hi` to s16 (with
the declaration-order-reversed chassis from H9 as the base) → score 2, 200/200
insns, identical to both the s32 baseline and the H9 reorder.
Result: KILLED (instance) — s16 vs s32 typing of `hi` measured score 2 -> 2 (no
change), same insn count 200/200, on this exact chassis.
kill_scope: instance
measured_on: func_8006B578, src/text1b.c, ordinary C only, no FAKE/cheat
constructs present in either form, reverted to the INCLUDE_ASM stub before
session end per asm-until-matched.

## H11 (s5, structural) — KILLED (instance): splitting the packed-halfword-swap
statement into two statements REGRESSES the score
Statement: rewriting `sp10 = (v & 0xFFFF) | (v >> 16);` as the two-statement
form `sp10 = v >> 16; sp10 |= v & 0xFFFF;` (statement re-association / split
init-then-accumulate, structurally analogous to the sanctioned
split-init-accumulation pattern) makes the build WORSE: score 2 -> 4, still
200/200 insns (so the two extra weighted-score points come from a
register/operand-level divergence the single-statement form does not have,
consistent with GCC choosing a different intermediate register/ordering for
the split computation).
Mechanism: none identified beyond "the compound `(a) | (b)` single-expression
form gives GCC's expression folder/scheduler a different (better-matching)
allocation than the pre-split two-statement form for this specific pack
operation" — not investigated further this session (declared dead on the
numeric regression alone; re-deriving the exact RTL mechanism is not needed to
kill this specific spelling).
Probe: `sandbox func_8006B578 --disable all` before (score 2) and after the
split (score 4, 200/200 insns), then reverted back to the single-statement
form and re-measured (score 2 again, confirming the split was the sole cause).
Result: KILLED (instance) — the split form measured strictly worse (2 -> 4);
the single-statement compound-OR form remains the closing spelling for this
line.
kill_scope: instance
measured_on: func_8006B578, src/text1b.c, ordinary C only, no FAKE/cheat
constructs present in either form, reverted to the INCLUDE_ASM stub before
session end per asm-until-matched.

## Live frontier (s5 update — unchanged from s3/s4)
The jtbl_80015988 cross-TU re-attribution (see H7/H8 above) remains the sole
live frontier item; this session's three structural probes (decl-order,
type-narrowing, statement-split) confirm the residual is NOT reachable through
in-file structural levers, reinforcing (not superseding) H8's class kill.

## [s5] Reversing the declaration order of func_8006B578's four block locals (var_s2, hi, ret, sp10, v vs the candidate's v, sp10, ret, hi, var_s2) is score-neutral.
- mechanism: none identified — GCC 2.7.2 local-alloc/global allocation for this function is insensitive to source declaration order for these locals
- probe: sandbox func_8006B578 --disable all before and after reordering the declarations in the applied candidate.c body
- result: score 2 -> 2, 200/200 insns unchanged, both measured this session
- verdict: KILLED
- kill_scope: instance
- measured_on: func_8006B578, src/text1b.c, ordinary C only, no FAKE/cheat constructs, candidate.c reapplied then reverted to INCLUDE_ASM stub before session end

## [s5] Narrowing the switch-dispatch temporary `hi` (holds ret>>16) from s32 to s16 is score-neutral.
- mechanism: none identified — the value is already effectively narrow at its only use site (switch(hi) with case labels 1/2)
- probe: sandbox func_8006B578 --disable all after retyping hi to s16 on the H9 (decl-order-reversed) chassis
- result: score 2 -> 2, 200/200 insns unchanged from both the s32 baseline and the H9 reorder, measured this session
- verdict: KILLED
- kill_scope: instance
- measured_on: func_8006B578, src/text1b.c, ordinary C only, no FAKE/cheat constructs, reverted to INCLUDE_ASM stub before session end

## [s5] Splitting `sp10 = (v & 0xFFFF) | (v >> 16);` into `sp10 = v >> 16; sp10 |= v & 0xFFFF;` regresses the score.
- mechanism: none identified — the compound single-expression OR form gives GCC a different (better-matching) allocation/ordering than the split two-statement form for this pack operation
- probe: sandbox func_8006B578 --disable all before (score 2), after the split (score 4, same 200/200 insns), then reverted and re-measured (score 2 again, confirming causality)
- result: score 2 -> 4 with the split (regression), 2 again after reverting; 200/200 insns throughout, all measured this session
- verdict: KILLED
- kill_scope: instance
- measured_on: func_8006B578, src/text1b.c, ordinary C only, no FAKE/cheat constructs, reverted to INCLUDE_ASM stub before session end
