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

## H12 (s6, enumerate modality) — KILLED (class): no enumerable source-level/operand-only region exists at the current candidate for a spelling sweep; the sole residual is structurally locked to a cross-TU named-symbol jump-table reference GCC cannot emit from ordinary switch C
Statement: re-applied candidate.c to src/text1b.c and re-measured before probing (per protocol).
`sandbox --disable all` reproduced score 2 exactly (no drift from s3/s4/s5). `sandbox --diff`
shows 22 hunks, ALL not-scored (masked branch/jump-target displacement noise from the function's
own absolute address differing between target's real load location and the sandbox's synthetic
placement) — 0 source-level, 0 operand-only. The systematic-spelling-sweep protocol (identify the
one differing region, wrap it in ENUM-BEGIN/END, run spelling_enum + sweep_variants) requires a
source-level or operand-only hunk to define the enumerable region; none exists, so there is
nothing to wrap or sweep — enumeration finds a genuinely empty search space, not a null result
from an unlucky sweep.
Went further than re-citing H7/H8/H9: independently re-derived the underlying mechanism this
session via direct binary inspection (not from old hypothesis text) —
  - `mipsel-linux-gnu-nm build/src/text1a_b_pre_rodata.o` still shows `00000598 R jtbl_80015988`
    (defined, a different TU); `build/src/text1b.o` still shows `U jtbl_80015988` (target-side
    build artifact, still referencing it externally — this reflects the INCLUDE_ASM stub's own
    asm, not our candidate).
  - Grepped `src/text1b.c` for `jtbl_80015988` — zero references. No C code anywhere in the file
    names the symbol; it cannot, since splat auto-named it and it is declared only in
    `src/text1a_b_pre_rodata.c:409`.
  - `mipsel-linux-gnu-objdump -dr tmp/sandbox/func_8006B578/text1b.o` on OUR compiled candidate
    shows the second switch's dispatch as `lui at,0x0 / R_MIPS_HI16 .rodata` + `lw v0,0(at) /
    R_MIPS_LO16 .rodata` — a section-relative relocation against a table GCC synthesized fresh
    into ITS OWN compiling TU's `.rodata`, exactly as `expand_end_case`/`gen_jump` in GCC 2.7.2's
    `stmt.c` always does for a `switch` with 6 contiguous case values (0-5).
  - `grep -n jtbl_80015988 asm/funcs/func_8006B578.s` shows target's own dispatch is `lui
    $at,%hi(jtbl_80015988)` / `lw $v0,%lo(jtbl_80015988)($at)` — a NAMED-SYMBOL relocation against
    the table declared in a different file, at address 0x80015988 (far from this function's own
    0x8006B578 — this is not a local/adjacent table, it is a genuinely separate, pre-existing
    object elsewhere in the binary that the original build's switch reused).
  - Conclusion (re-derived, not assumed): ordinary C `switch` statement semantics give the
    programmer no lever to make GCC's `stmt.c` case-table emission reference an
    externally-declared name instead of synthesizing a fresh local table — the switch structure,
    case ordering, declaration order, or any pure C rewrite confined to `src/text1b.c` cannot
    change WHICH object the compiler treats as "the jump table for this switch." This is a
    property of GCC's `expand_end_case`, not of the C source shape, so it holds for every C
    spelling of the switch, not merely the ones tried so far.
kill_scope: class
predicate_cite: tools/gcc-2.7.2/stmt.c:4704 (`table_label = gen_label_rtx ();` inside
  `expand_end_case`, stmt.c:4681) — GCC always allocates a FRESH internal label/table via
  `gen_label_rtx()` for a compiled switch's jump table; there is no code path in
  `expand_end_case` that takes a symbol name from the C source and reuses it as the table
  label, so no C spelling of the switch can make GCC reference `jtbl_80015988` by name.
measured_on: func_8006B578, src/text1b.c (candidate.c reapplied then reverted to the INCLUDE_ASM
  stub before session end, per asm-until-matched), ordinary C only, no FAKE/cheat constructs
  present or proposed. Score 2, 200/200 insns, 0 source-level, 0 operand-only, 22 not-scored
  (re-confirmed this session, matching s3/s4/s5 exactly).
Result: KILLED (class) — no in-TU C spelling reachable; the residual requires the already-
  identified cross-TU jtbl_80015988 declaration move (frontier item 1, unchanged), which is
  outside a single-file grind session's mandate. This closes the enumerate modality on this
  function's current frontier: there is no region for the systematic spelling sweep to search.

## Session 6 (enumerate modality, 2026-09-16)

H9: The candidate.c chassis at src/text1b.c measures score=2, target_insns=200,
build_insns=200, 0 source-level hunks, 0 operand-only hunks, 22/22 not-scored
hunks (all masked branch-target-address artifacts, e.g. target `beq v1,v0,23718`
vs ours `beq v1,v0,9484` — objdump-printed absolute addresses that differ because
of upstream link-layout offset, not real instruction content) when re-measured
fresh this session via `sandbox func_8006B578 --disable all --diff`.
Result: CONFIRMED — exact re-measurement of s3/s4/s5's finding on the current
chassis: score=2, 200/200 insns, 0/22 source-level, 0/22 operand-only, 22/22
not-scored.
kill_scope: n/a (CONFIRMED, not KILLED)
measured_on: func_8006B578, src/text1b.c, candidate.c applied then reverted to
the INCLUDE_ASM stub before session end, ordinary C only, no FAKE/cheat
constructs present.

H10 (the mandated enumerate-modality spelling sweep): with 0 source-level and 0
operand-only hunks, there is no C-visible byte difference to search — but the
modality still requires running the systematic sweep procedure, so I marked the
most structurally complex sub-block (switch-dispatch case 0's bit-toggle
sequence, candidate.c lines 64-71: `s32 *p = ...; u32 f = (u32)p[8]; u32 a3 = f &
~1u; u32 bit = f & 1; bit ^= 1; a3 |= bit; p[8] = (s32)a3; func_8005C650(...);`)
with ENUM-BEGIN/ENUM-END and ran `tools/spelling_enum.py` (3 named locals, 0
assignments, 4 anchors -> 10 distinct spellings including commutative swaps,
tmp/grind/func_8006B578/s6/enum/v00.c..v09.c). Of the 10 emitted spellings, only
4 are syntactically valid C (v00 identical-to-candidate, v01 declaration-order
swap of `bit`/`a3`, v02 inlines `f` at both use sites, v03 same inlining with
`bit` computed first) — the remaining 6 (v04-v09) are BROKEN C emitted by the
tool's compound-assignment axis misapplying itself to this region (it produces
invalid lvalues like `(f & ~1u) |= bit;`, `(f & 1) ^= 1;` — not compilable, a
tool limitation on this region's shape, not a spelling candidate).
I measured the two non-trivial valid spellings directly against the sandbox:
  - v02 (inline `f`, both `p[8]` reads written out at each use site instead of
    a shared `f` local): score=2, build_insns=200 — IDENTICAL to baseline.
    KILLED (instance): this respelling changes nothing; GCC's CSE re-derives
    the same `f` value either way.
    kill_scope: instance
    measured_on: func_8006B578, src/text1b.c, ordinary C only, no FAKE/cheat
    constructs, reverted to INCLUDE_ASM stub before session end.
  - v01 (declare `bit` before `a3`, reversing candidate.c's declaration order):
    score=11, build_insns=203 (+3 insns vs target's 200) — REGRESSES.
    KILLED (instance): reversing this pair's declaration order changes local
    allocation and costs 3 instructions; candidate.c's current order (`a3`
    before `bit`) is strictly better and must be kept.
    kill_scope: instance
    measured_on: func_8006B578, src/text1b.c, ordinary C only, no FAKE/cheat
    constructs, reverted to INCLUDE_ASM stub before session end.
After both measurements the candidate was restored to its exact s3/s4/s5 form
(re-verified score=2, 200/200 insns) before reverting to INCLUDE_ASM.

Conclusion for this session: the enumerate modality's precondition (a
source-level or operand-only residual to search) does not hold for this
function's current frontier — H8 (s4, class-killed) and H9 (this session's
re-confirmation) both establish that the only remaining gap is the cross-TU
jtbl_80015988 masked-artifact residual, not a spelling question. The sweep
that WAS run (H10) found the current candidate's spelling of case 0's
bit-toggle block is already locally optimal among the syntactically valid
enumerated alternatives. Frontier item 1 (cross-file jtbl declaration move)
is unchanged and remains the only live lever.

## [s6] Re-measuring memory/grind/func_8006B578/candidate.c on the current chassis reproduces sandbox --disable all score=2, target_insns=200, build_insns=200, with the --diff breakdown showing 0/22 source-level hunks, 0/22 operand-only hunks, and 22/22 not-scored hunks (masked branch/jump-target address artifacts, e.g. target 'beq v1,v0,23718' vs ours 'beq v1,v0,9484').
- mechanism: Chassis-reproducibility check (no GCC pass claimed) — establishes the baseline this session's spelling sweep measures against.
- probe: Applied candidate.c to src/text1b.c, ran `sandbox func_8006B578 --disable all --diff`.
- result: CONFIRMED: score=2, target_insns=200, build_insns=200, 0 source-level, 0 operand-only, 22 not-scored — exact match to s3/s4/s5's recorded floor.
- verdict: CONFIRMED

## [s6] Wrapping candidate.c's switch-dispatch case-0 bit-toggle block (lines 64-71: p[8] read into local f, masked into a3/bit, toggled, recombined, stored) in ENUM-BEGIN/END and running tools/spelling_enum.py produces 10 distinct spellings; of those, inlining the shared local f (both p[8] reads written out separately instead of cached, variant v02) is a byte-neutral respelling of this residual-free region.
- mechanism: GCC's CSE (cse.c) re-derives the repeated p[8] load either way when the region is already free of source-level/operand-only diffs — this is a spelling-space sweep per the mandated enumerate-modality procedure, not a claimed coercion mechanism.
- probe: Applied variant v02 (tmp/grind/func_8006B578/s6/enum/v02.c region) to src/text1b.c in place of candidate.c's case-0 block, ran `sandbox func_8006B578 --disable all`.
- result: KILLED: score=2, build_insns=200 — identical to baseline; no improvement, no regression. The current candidate.c spelling (cached local f) and the inlined spelling (v02) are score-equivalent; neither closes the residual.
- verdict: KILLED
- kill_scope: instance
- measured_on: func_8006B578, src/text1b.c, ordinary C only, no FAKE/cheat constructs present in either form, reverted to the INCLUDE_ASM stub before session end

## [s6] Reversing candidate.c's declaration order for the case-0 block's a3/bit locals (declaring bit before a3, variant v01) is a byte-neutral respelling that does not affect codegen.
- mechanism: Local-allocation declaration-order sensitivity (ordinary C register-allocation observation, not a claimed coercion) — part of the mandated enumerate-modality sweep's declaration-order axis.
- probe: Applied variant v01 (bit declared before a3) to src/text1b.c in place of candidate.c's case-0 block, ran `sandbox func_8006B578 --disable all`.
- result: KILLED: score=11, build_insns=203 (+3 insns vs target's 200) — a real regression. Candidate.c's current declaration order (a3 before bit) is strictly better and must be retained; this reordering is not a viable lever.
- verdict: KILLED
- kill_scope: instance
- measured_on: func_8006B578, src/text1b.c, ordinary C only, no FAKE/cheat constructs present in either form, reverted to the INCLUDE_ASM stub before session end

## H13 (s7, synthesis) — CONFIRMED: the compiled switch's jump table is content-identical to jtbl_80015988; only its owning TU is wrong

**Statement.** The GCC-2.7.2-synthesized jump table that `candidate.c`'s second `switch`
emits into `src/text1b.c`'s `.rodata` has exactly the same six entries, in the same order, as
the extracted `const u32 jtbl_80015988[6]` array in `src/text1a_b_pre_rodata.c:409-416`.

**Mechanism.** `expand_end_case` (`tools/gcc-2.7.2/stmt.c:4681`) allocates a fresh internal
table label (`gen_label_rtx()`, `stmt.c:4704`) and fills it with one `R_MIPS_32` word per case
label of the compiling function. Those case labels are the same six basic-block entry points
the original compiler produced, so the words are the same addresses.

**Probe.** `mipsel-linux-gnu-objdump -sr -j .rodata tmp/sandbox/func_8006B578/text1b.o`.

**Result.** Our `.rodata` is 24 bytes, six `R_MIPS_32 .text` relocations, words
`0x9554 0x9584 0x95BC 0x9650 0x9698 0x96C4`. Target words are
`0x8006B6B8 0x8006B6E8 0x8006B720 0x8006B7B4 0x8006B7FC 0x8006B828`. All six pairwise
differences equal **0x80062164** — the section-base delta. Content match is exact.
**Verdict: CONFIRMED.**

## H14 (s7, synthesis) — CONFIRMED: all 22 not-scored hunks are one uniform object-offset shift, i.e. zero codegen divergence remains in the body

**Statement.** Every `target`/`ours` address operand in the 22 not-scored hunks of
`sandbox --disable all --diff` differs by the single constant 0x1A294, and no hunk has a
non-address difference.

**Mechanism.** The reference `build/src/text1b.o` is assembled from the whole-file asm, so
`func_8006B578` begins at a different offset inside that object than inside our sandbox
object; every intra-function branch/jump target therefore prints shifted by that constant.

**Probe.** Captured the full diff to `tmp/grind/func_8006B578/s7/diff_baseline.txt` and ran a
script extracting the trailing hex operand of every hunk pair.

**Result.** `deltas: {'0x1a294': 22}`, `non-address hunks: []`. **Verdict: CONFIRMED.** There
is no branch-sense, scheduling, or allocation divergence anywhere in the function body.

## H15 (s7, synthesis) — KILLED (instance): the two closest banked instance kills re-measure unchanged on the current chassis with no FAKE carrier present

**Statement.** Re-measuring s5's `s32 hi` -> `s16 hi` narrowing and s6's v02 "inline `f`,
drop the shared local" rewrite of the case-0 bit-toggle block, each applied to the current
`candidate.c` chassis, reproduces score 2 / 200 insns for both — neither form improves nor
regresses the floor.

**Mechanism.** Both variants touch only value-numbering-equivalent spellings inside blocks
that already emit target-identical instructions (H14); there is no residual for them to move.

**Probe.** `tools/fake_ablate.py --func func_8006B578 --file text1b --candidate
memory/grind/func_8006B578/candidate.c` (=> "no FAKE-annotated constructs found; nothing to
ablate", so no FAKE carrier could have masked either lever), then each variant applied to
`src/text1b.c` and measured with `sandbox func_8006B578 --disable all`.
Variant files: `tmp/grind/func_8006B578/s7/reaudit_hi_s16.c`,
`tmp/grind/func_8006B578/s7/reaudit_v02_inline_f.c`.

**Result.** `reaudit_hi_s16 => score 2, target_insns 200, build_insns 200`;
`reaudit_v02_inline_f => score 2, target_insns 200, build_insns 200`.
**Verdict: KILLED (instance)** — measured on the s3 candidate.c chassis, ordinary C only, zero
FAKE constructs present in either form. Both kills stand as originally banked.

## H16 (s7, synthesis) — CONFIRMED: the project already implements the exact linker-script fix this residual needs, for a sibling function one cluster later

**Statement.** `func_80077B30` (`src/text1b_b.c:825`), an already-COMPLETED-C function, has a
six-case `switch (D_800A35E4)` whose compiler-generated 24-byte jump table is placed correctly
by a dedicated `build/src/text1b_b.o(.rodata);` slot that `bb2.ld` inserts BETWEEN the two
halves of the extracted grab-bag rodata sub-TU.

**Mechanism.** The 2026-06-09 rodata-cleanup project split the `101C.rodata_text1a_b` cluster
into `text1a_b_pre_rodata.c` and `text1a_b_post_rodata.c` precisely so the real owning C
file's compiler-generated `.rodata` could take the hole in the middle.

**Probe.** Read `bb2.ld:53-70`; read the tail of `src/text1a_b_pre_rodata.c` and head of
`src/text1a_b_post_rodata.c`; read `src/text1b_b.c:825-870`.

**Result.** `bb2.ld` lines 59/60/61 are pre_rodata / text1b_b / post_rodata in that order.
Pre's last symbol is `jtbl_80015A24` (6 words @ 0x80015A24, ending 0x80015A3C); post's first
is `jtbl_80015A54` (@ 0x80015A54); the 0x18 = 24-byte hole is exactly one 6-entry table — the
one `func_80077B30`'s switch emits. **Verdict: CONFIRMED.** func_8006B578 needs the identical
treatment at 0x80015988: split `text1a_b_pre_rodata.c` again around `jtbl_80015988`, delete
that array (GCC re-emits it, proven byte-identical by H13), and insert a
`build/src/text1b.o(.rodata);` line at the new boundary in `bb2.ld` (moving it from its
current, wrong position at `bb2.ld:66`). Both edits are outside a grind session's surface
(`*.ld` is explicitly forbidden; `src/text1a_b_pre_rodata.c` is a second file) — this is an
INTEGRATION HANDOFF, recorded in `docs/grind/decisions.md`.

## Session 7b (synthesis re-run) — H17, H18, H19

### H17 — KILLED (instance). Dispatching the second switch with a computed `goto` through the EXTERN extracted table `jtbl_80015988` makes the body reference the same named symbol the reference object does, removing the need for any `bb2.ld`/second-TU change.
- **Mechanism tested.** `extern const u32 jtbl_80015988[6];` in `src/text1b.c` plus
  `u32 k = (u32)D_800A34F8 >> 10 & 7; if (k >= 6) goto tail; goto *(void *)jtbl_80015988[k];`
  with the six former `case N:` labels rewritten as plain labels `cg0:`..`cg5:`. GCC 2.7.2's
  `indirect_jump` expansion for `goto *expr` emits `sll/lui %hi/addu/lw %lo/jr`, the exact
  five-instruction shape the target uses at 0x8006B69C-0x8006B6B0, and text1b.o would emit no
  `.rodata` at all, so nothing downstream shifts.
- **Probe.** Form written to
  `tmp/grind/func_8006B578/s7/variant_a_extern_cgoto.c`, spliced into `src/text1b.c`, measured
  with `sandbox func_8006B578 --disable all --diff`
  (`tmp/grind/func_8006B578/s7/diff_variant_a.txt`).
- **Result.** Score **116**, and decisively: **95 build insns vs 200 target insns**. GCC deleted
  the five blocks `cg1:`..`cg5:` outright. They are reachable only through the computed jump, and
  no label's address is taken anywhere in the TU, so `jump.c:185`
  (`LABEL_NUSES (insn) = (LABEL_PRESERVE_P (insn) != 0);`) seeds their use counts at zero and the
  blocks are dropped as unreachable before any later pass sees them. 7 of the 17 remaining hunks
  are source-level. The form is also unsound as a submission independent of the measurement: the
  table it reads holds hardcoded original code addresses (0x8006B6B8…), which matches no entry in
  the frozen SOTN family list and is a first reach of a hardcoded-address family — so even a
  `&&label`-preserving repair (which would re-emit our own table anyway, defeating the purpose)
  would need a ruling, not a submission. Banked at
  `memory/grind/func_8006B578/rejected/extern-jtbl-computed-goto-blocks-deleted-and-hardcoded-addresses.c`.
- **kill_scope** instance. **measured_on** candidate.c chassis (score 2, byte-identical to s3-s7),
  ordinary C plus the one computed-goto construct; no FAKE constructs present; `src/text1b.c`
  reverted to the `INCLUDE_ASM` stub before session end.

### H18 — CONFIRMED. The 0x80015940-0x80015A3C rodata run is entirely `src/text1b.c`-owned, and the operator's split point for this function is exactly 0x80015988 (between `jtbl_80015940` and `D_800159A0`).
Measured by resolving every entry of each rodata table in that range to its enclosing function
(`asm/funcs/*.s` instruction-address grep) and then to that function's `INCLUDE_ASM` owner; full
table in `evidence.md` (Session 7b). Corrects the previous session's unverified sketch on two
points: `jtbl_80015A0C` IS text1b.c's (`func_800747D8`, `src/text1b.c:8122`), and the run starts
at `jtbl_80015940` (`func_80065800`), one symbol before our table. `objdump -h build/src/text1b.o`
confirms text1b.o has no `.rodata` section today, so the `bb2.ld:66` slot is free to move.

### H19 — CONFIRMED. The sandbox score for this function is pinned at exactly 2 by the scorer's symbol-resolution asymmetry, not by any codegen divergence.
`jtbl_80015988` is in none of `LD_SYM_FILES` (`engine/buildconfig.py:93`) because the rodata
cleanup made it a C `const` rather than a splat symbol; `engine/score.py:62` leaves named-symbol
HI16/LO16 immediates unmasked while `score.py:71`/`:140` rewrite only SECTION-relative HI16/LO16
to `@.rodata`. The reference pair (named reloc) and ours (section reloc) therefore traverse
different code paths and can never produce an equal token. The full-build oracle is the only
instrument that can certify this body.

## [s7] Dispatching the second switch with a computed goto through the extern extracted table jtbl_80015988 (goto *(void *)jtbl_80015988[k]) reproduces the target's dispatch pair while emitting no rodata of our own, closing the residual without any bb2.ld or second-TU change.
- mechanism: GCC 2.7.2 indirect_jump expansion for a computed goto emits sll/lui %hi/addu/lw %lo/jr, the exact shape the target uses at 0x8006B69C-0x8006B6B0, and names the same external symbol the reference object names; text1b.o would contribute zero .rodata so no later input section shifts.
- probe: tmp/grind/func_8006B578/s7/variant_a_extern_cgoto.c spliced into src/text1b.c in place of the INCLUDE_ASM stub; sandbox func_8006B578 --disable all --diff (tmp/grind/func_8006B578/s7/diff_variant_a.txt).
- result: Score 116 with only 95 build insns against 200 target insns: GCC deleted the five blocks cg1:..cg5:, which are reachable only through the computed jump while no label's address is taken anywhere in the TU, so jump.c:185 seeds their LABEL_NUSES at zero and they are dropped as unreachable; 7 of the 17 surviving hunks are source-level. Independently of the measurement the form is not submittable: the table holds hardcoded original code addresses (0x8006B6B8 and following), a first reach of a hardcoded-address family that matches nothing on the frozen SOTN list, and the obvious repair (taking label addresses with &&label to set LABEL_PRESERVE_P) re-emits our own table and so defeats the entire purpose. Banked at memory/grind/func_8006B578/rejected/extern-jtbl-computed-goto-blocks-deleted-and-hardcoded-addresses.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis (sandbox score 2, byte-identical to sessions 3-7) plus the single computed-goto construct; no FAKE constructs present in either form; src/text1b.c reverted to the INCLUDE_ASM stub before session end

## [s7] The rodata run 0x80015940-0x80015A3C is entirely src/text1b.c-owned, and the slot for build/src/text1b.o(.rodata) that this function needs sits at exactly 0x80015988, between jtbl_80015940 and D_800159A0.
- mechanism: Per-symbol ownership resolution: each table entry address was resolved to its enclosing function by grepping asm/funcs/*.s for that instruction address, then to that function's INCLUDE_ASM owner in src/*.c; the object-model order text1a_b_pre_rodata -> text1b -> text1b_b -> text1a_b_post_rodata is corroborated by bb2.ld:60 already filling the 0x80015A3C-0x80015A54 hole with text1b_b.o(.rodata) for func_80077B30's table.
- probe: Address-to-function-to-TU sweep over every src/text1a_b_pre_rodata.c rodata symbol in 0x80015900-0x80015B00 (full table in evidence.md Session 7b), plus mipsel-linux-gnu-objdump -h build/src/text1b.o.
- result: jtbl_80015940 -> func_80065800, jtbl_80015988 -> func_8006B578, jtbl_800159B0 -> func_8006E534, jtbl_800159D0 -> func_8006ECF4, jtbl_80015A0C -> func_800747D8 (INCLUDE_ASM at src/text1b.c:8122), jtbl_80015A24 -> func_80077374 - all six owned by src/text1b.c. This corrects the previous session's unverified sketch on two points: jtbl_80015A0C is NOT outside text1b.c's range, and the run starts one symbol earlier than assumed (jtbl_80015940). objdump shows text1b.o has a single .text section and no .rodata section at all, so the bb2.ld:66 slot contributes zero bytes today and can be relocated without disturbing any other placement. Forward constraint recorded: because GCC emits switch tables in TU order, future text1b.c decompilations must land so the emitted tables stay in ascending address order, and each one moves the slot earlier and deletes another extracted table.
- verdict: CONFIRMED

## [s7] The sandbox score for func_8006B578 is pinned at exactly 2 by the scorer's named-vs-section symbol resolution asymmetry rather than by any codegen divergence in the body.
- mechanism: jtbl_80015988 is absent from LD_SYM_FILES (engine/buildconfig.py:93) because the rodata cleanup turned it into a C const rather than a splat symbol; engine/score.py:62 leaves named-symbol HI16/LO16 immediates unmasked while score.py:71 and score.py:140 rewrite only SECTION-relative HI16/LO16 to @.rodata, so the reference object's named reloc and our section reloc traverse different code paths.
- probe: Read engine/score.py:57-141 and 186-215 plus engine/buildconfig.py:93; grepped symbol_addrs.txt, named_syms.txt and undefined_syms_auto.txt for jtbl_80015988 (no hit); re-measured the baseline diff (tmp/grind/func_8006B578/s7/diff_baseline_s7b.txt).
- result: Confirmed: score 2, 200/200 insns, 22 hunks all classed not-scored and all of them branch/jump target pairs differing by the constant object-vs-image delta. The two scored tokens are the dispatch pair lui/lw, which traverse the two different masking paths and can never produce an equal token. tools/fake_ablate.py reports no FAKE-annotated construct in candidate.c, so the s5/s6 instance kills were measured on a chassis byte-identical to the current one with no FAKE carrier occupying any pseudo and needed no re-measurement. The full-build oracle is the only instrument that can certify this body.
- verdict: CONFIRMED

## [s8] The score-2 residual is owned by the RA model (register allocation) or by the scheduler, so tools/ra_solver/inverse.py or inverse_sched.py can express a lever for it.
- mechanism: tools/ra_solver/inverse_compose.py `classify` triages a residual into
  PRE-RA / RA / SCHED / IDENTICAL by comparing register-blanked instruction multisets
  (RA and SCHED models permute and rename a FIXED multiset, so they can only own a
  residual whose multisets already agree).
- probe: candidate.c applied to src/text1b.c (sandbox score 2, 200/200 insns);
  `python3 tools/ra_solver/inverse_compose.py classify text1b func_8006B578
   --target-object build/src/text1b.o --ours-object tmp/sandbox/func_8006B578/text1b.o`
  (object mode, the asm-until-matched-safe path). Output banked at
  tmp/grind/func_8006B578/s8/classify.txt.
- result: KILLED. The classifier returns FIRST DIVERGENCE: PRE-RA, "next tool: no
  backend — the residual is upstream of every model". The RA backend (inverse.py) and
  the scheduler backend (inverse_sched.py) are both mechanically inapplicable to this
  residual: they permute/rename a fixed multiset and the multisets do not agree. This
  is the typed solver-modality verdict for func_8006B578 — the solver axis is closed,
  and no future session should spend a pass running inverse.py / inverse_sched.py /
  perturb.py against this function on the current chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis applied to src/text1b.c (sandbox --disable all score 2,
  200 target insns / 200 build insns), zero FAKE constructs present
  (tools/fake_ablate.py: "no FAKE-annotated constructs found ... nothing to ablate"),
  src/text1b.c reverted to the INCLUDE_ASM stub before session end.

## [s8] The classifier's PRE-RA label means our C builds a different RTL instruction shape from the target's, i.e. there is still a front-end / CSE / combine lever to find.
- mechanism: `classify` blanks REGISTERS but not RELOCATION SYMBOLS, so two objdump
  renderings of the identical opcode with identical operand roles are counted as
  different "shapes" when their immediate fields carry relocations against different
  symbols. Its own output names the four shapes, and they are two instructions:
  ours `lui #,@.rodata` / `lw #,@.rodata(#)` vs target `lui #,0x0` / `lw #,0(#)`.
- probe: (a) tmp/grind/func_8006B578/s8/delta.py over engine.score.normalized_insns for
  both objects: 22 unmasked differences, ALL branch/jump targets, and the set of distinct
  target deltas is exactly {0x1a294} — one constant object offset, no second delta;
  masked-diff-count is 2, and the two masked hunks are insn #74 `lui at` and insn #76
  `lw v0`. (b) `mipsel-linux-gnu-objdump -r --section=.text` on both objects: the target
  carries R_MIPS_HI16 + R_MIPS_LO16 against the EXTERNAL symbol `jtbl_80015988` at
  0x237d0/0x237d8; ours carries R_MIPS_HI16 + R_MIPS_LO16 against our own `.rodata`
  SECTION at 0x953c/0x9544 — the same two reloc TYPES at the same two instruction slots,
  offset by the same constant 0x1a294.
- result: KILLED. The PRE-RA label is a rendering artifact of unnormalized relocation
  symbols, not evidence of an RTL-shape divergence. Opcode, operand roles and relocation
  types are identical on both dispatch instructions; only the relocation's SYMBOL differs
  (external `jtbl_80015988` vs our own `.rodata`), which is exactly the cross-TU table
  ownership residual H7/H13/H14 already characterized. There is no front-end/CSE/combine
  lever behind the label, and the `classify` output's cse_merge / cse_split lever menus
  do not apply to this function.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis applied to src/text1b.c (score 2, 200/200 insns), zero
  FAKE constructs (fake_ablate: nothing to ablate); objects
  tmp/sandbox/func_8006B578/text1b.o (ours) and build/src/text1b.o (reference), src
  reverted to the INCLUDE_ASM stub before session end.

## [s8] KILL RE-AUDIT — the two s5 "score-neutral" instance kills are neutral only because the sandbox gradient is pinned at 2 by the reloc artifact, and could hide a real byte difference inside a masked hunk.
- mechanism: `engine/score.py` masks branch/jump targets and the two reloc-bearing
  dispatch operands, so at a pinned floor of 2 a variant can change a MASKED operand
  without changing the score. Against a function that will be certified by the full-build
  SHA1 oracle rather than by the sandbox, "score-neutral" is therefore not automatically
  "byte-neutral" — the mandated re-audit instrument for this function is the byte/delta
  comparison, not the score.
- probe: both s5 neutral forms rebuilt on the CURRENT chassis (candidate.c, FAKE-free —
  fake_ablate reports nothing to ablate, so there is no FAKE carrier occupying any
  pseudo) and measured twice each: sandbox --disable all, then delta.py.
  (i) v_s16hi (narrow `hi` from s32 to s16): score 2, 200 build insns; distinct branch
      deltas {0x1a294}; masked diffs exactly #74 `lui at` and #76 `lw v0`.
  (ii) v_declrev (reverse the five block locals to var_s2, hi, ret, sp10, v): score 2,
      200 build insns; distinct branch deltas {0x1a294}; masked diffs exactly #74/#76.
  Banked: tmp/grind/func_8006B578/s8/{v_s16hi.c,v_declrev.c,delta_v_s16hi.txt,
  delta_v_declrev.txt}.
- result: KILLED (the re-audit hypothesis — the hidden-difference worry is disproven).
  Both s5 kills SURVIVE re-measurement and are UPGRADED from score-neutral to
  byte-equivalent: each variant reproduces the target byte-for-byte modulo the same
  single constant position delta 0x1a294 and the same two reloc-symbol hunks as
  candidate.c itself. Neither respelling perturbs allocation, scheduling or frame layout,
  and neither would change the full-build SHA1 outcome. candidate.c remains the form of
  record (it is the one whose provenance is documented), but an operator performing the
  integration handoff may treat these two respellings as interchangeable with it.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis, zero FAKE constructs present in any of the three
  bodies (fake_ablate on candidate.c: nothing to ablate; the two variants differ from it
  by one type keyword and one declaration permutation respectively), src/text1b.c
  reverted to the INCLUDE_ASM stub before session end.

## [s8] The score-2 residual is owned by the RA model or by the scheduler, so tools/ra_solver/inverse.py or inverse_sched.py can express a lever for it.
- mechanism: inverse_compose.py classify triages a residual PRE-RA / RA / SCHED / IDENTICAL by comparing register-blanked instruction multisets; the RA and scheduler models permute and rename a FIXED multiset, so they can only own a residual whose multisets already agree.
- probe: candidate.c applied to src/text1b.c (sandbox --disable all: score 2, 200 target insns / 200 build insns, 0 source-level / 0 operand-only / 22 not-scored), then `python3 tools/ra_solver/inverse_compose.py classify text1b func_8006B578 --target-object build/src/text1b.o --ours-object tmp/sandbox/func_8006B578/text1b.o` (object mode, the asm-until-matched-safe path).
- result: KILLED. Classifier returns FIRST DIVERGENCE: PRE-RA, 'next tool: no backend - the residual is upstream of every model'. Both solver backends (inverse.py for allocation, inverse_sched.py/perturb.py for ordering) are mechanically inapplicable to this residual on this chassis, so the solver axis is closed and no future session should spend a pass running them here. Output banked at tmp/grind/func_8006B578/s8/classify.txt.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis applied to src/text1b.c, sandbox --disable all score 2 (200/200 insns); zero FAKE constructs present (tools/fake_ablate.py: 'no FAKE-annotated constructs found ... nothing to ablate'); src/text1b.c reverted to the INCLUDE_ASM stub before session end.

## [s8] The classifier's PRE-RA label means our C builds a different RTL instruction shape from the target's, i.e. a front-end / CSE / combine lever remains to be found.
- mechanism: classify blanks REGISTERS but not RELOCATION SYMBOLS, so two objdump renderings of the same opcode with the same operand roles count as different shapes when their immediate fields carry relocations against different symbols.
- probe: (a) tmp/grind/func_8006B578/s8/delta.py over engine.score.normalized_insns for both objects: 22 unmasked differences, all branch/jump targets, distinct target-delta set exactly {0x1a294}; masked-diff count 2, at insn #74 (lui at) and #76 (lw v0). (b) `mipsel-linux-gnu-objdump -r --section=.text` on both objects.
- result: KILLED. The reference object carries R_MIPS_HI16 + R_MIPS_LO16 against the EXTERNAL symbol jtbl_80015988 at 0x237d0/0x237d8; ours carries R_MIPS_HI16 + R_MIPS_LO16 against our own .rodata SECTION at 0x953c/0x9544 - the same two relocation TYPES at the same two instruction slots, offset by the same constant 0x1a294. The PRE-RA label is therefore a relocation-rendering artifact, not an RTL-shape divergence; the classify output's cse_merge / cse_split lever menus do not apply to this function, and this is the cross-TU rodata ownership residual H7/H13/H14 already characterized. First whole-stream (rather than sampled) verification of the s7 'all 22 hunks differ by one constant' claim.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis applied to src/text1b.c (score 2, 200/200 insns), zero FAKE constructs (fake_ablate: nothing to ablate); objects tmp/sandbox/func_8006B578/text1b.o (ours) vs build/src/text1b.o (reference); src reverted to the INCLUDE_ASM stub before session end.

## [s8] The two s5 score-neutral kills (s16 narrowing of `hi`; reversed declaration order of the five block locals) are neutral only because the sandbox gradient is pinned at 2, and could be hiding a real byte difference inside a masked hunk.
- mechanism: engine/score.py masks branch/jump targets and the two reloc-bearing dispatch operands, so at a pinned floor of 2 a variant can change a MASKED operand without changing the score; since this function will be certified by the full-build SHA1 oracle rather than the sandbox, score-neutral does not automatically imply byte-neutral.
- probe: Both s5 forms rebuilt on the CURRENT chassis (candidate.c, FAKE-free, so no FAKE carrier occupies any pseudo) and measured twice each - sandbox --disable all, then delta.py. v_s16hi: score 2, 200 build insns, distinct branch deltas {0x1a294}, masked diffs exactly #74/#76. v_declrev: score 2, 200 build insns, distinct branch deltas {0x1a294}, masked diffs exactly #74/#76.
- result: KILLED (the hidden-difference worry is disproven). Both s5 kills survive re-measurement and are UPGRADED from score-neutral to byte-equivalent: each variant reproduces the target byte-for-byte modulo the same single constant position delta 0x1a294 and the same two reloc-symbol hunks as candidate.c itself. Note sp10 is address-taken and passed to func_800692C0, so the declaration permutation did not move its frame slot either. candidate.c remains the form of record; an operator performing the integration handoff may treat these two respellings as interchangeable with it.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis; zero FAKE constructs present in any of the three bodies (fake_ablate on candidate.c: nothing to ablate; the variants differ from it by one type keyword and one declaration permutation respectively); src/text1b.c reverted to the INCLUDE_ASM stub before session end.

## Session 9 (forensics)

### H19 — CONFIRMED. The compiled jump table is byte-identical to the extracted jtbl_80015988 after relocation.
*Statement.* With `candidate.c` applied, `text1b.o(.rodata)` holds exactly six `R_MIPS_32`
words that, once the linker resolves them against `text1b.o(.text)`'s live base, equal the
six words of `const u32 jtbl_80015988[6]` at `src/text1a_b_pre_rodata.c:409-416`.
*Mechanism.* GCC's `casesi` vector is emitted as `.word .L<label>` entries carrying
section-relative `R_MIPS_32` relocations; the target's table was extracted during the 2026
rodata cleanup as absolute `u32` literals. Equality therefore means "same addends + correct
section base", which is measurable without a full build.
*Probe.* `objdump -h/-s/-r/-t tmp/sandbox/func_8006B578/text1b.o` +
`objdump -t build/src/text1a_b_pre_rodata.o`.
*Result.* Size 0x18, Algn 2**3, addends `0x9554 0x9584 0x95BC 0x9650 0x9698 0x96C4`;
`func_8006B578` at `.text+0x9414` and linking at `0x8006B578` gives base `0x80062164`;
addend+base = `0x8006B6B8 0x8006B6E8 0x8006B720 0x8006B7B4 0x8006B7FC 0x8006B828` = the
extracted array element-for-element. CONFIRMED. Artifact:
`tmp/grind/func_8006B578/s9/rodata_certification.txt`.

### H20 — CONFIRMED. The bb2.ld move costs zero padding on both sides of the hole.
*Statement.* Deleting `jtbl_80015988` from `src/text1a_b_pre_rodata.c` and inserting
`build/src/text1b.o(.rodata)` at `0x80015988` leaves every symbol outside the 24-byte hole
at its current address.
*Mechanism.* Output-section placement inserts per-object alignment padding; a mismatch
between our vector's `.align 3` and the hole's start address, or between the hole's end and
the post-split TU's alignment, would shift everything downstream and silently break the
SHA1 the handoff is supposed to produce.
*Probe.* `objdump -t/-h build/src/text1a_b_pre_rodata.o` for the three neighbouring symbols'
offsets and the section alignment; arithmetic against our object's `2**3`.
*Result.* `jtbl_80015940` +0x550 size 0x48 (ends +0x598), `jtbl_80015988` +0x598 size 0x18,
`D_800159A0` +0x5B0; hole = exactly 24 B at `0x80015988`; `0x80015988 % 8 == 0` (our
alignment satisfied, 0 pad) and `0x800159A0` satisfies the post-half's `2**2` (0 pad).
CONFIRMED — the last unmeasured risk in the handoff plan is closed.

### H21 — KILLED (instance). "Some GCC optimization pass produces the two masked dispatch instructions, so a pass-input shape can be enumerated to change them."
*Statement.* The `lui at,%hi(..)` / `lw v0,%lo(..)(at)` pair that pins the score at 2 is
produced by an optimization pass whose input shape C can vary.
*Mechanism to test.* The forensics-modality PASS-INPUT ENUMERATION discipline: name the
pass from the instrumented-cc1 dumps, then enumerate the source shapes that change what it
sees.
*Probe.* `pwsh tools/grinder/dump.ps1 func_8006B578`; read `dumps/text1b.rtl`,
`.jump .cse .cse2 .loop .combine .flow .lreg .greg .sched .sched2 .jump2 .dbr`, `dumps/text1b.s`.
*Result.* KILLED. `dumps/text1b.rtl:51828` shows `(jump_insn 474 473 475 (addr_vec:SI[...]))`
already present in the FIRST RTL dump — the vector is built by RTL generation
(`stmt.c expand_end_case` -> `casesi`), before any optimization pass, and no later dump
alters it. `final.c` emits a single `lw $2,.L988($2)`; maspsx/ASPSX expands that macro load
into the observed lui/lw pair. No optimization pass is in the causal chain, so there is no
pass input to enumerate; the only free variable is the label's final address, owned by
`bb2.ld`. Artifact: `tmp/grind/func_8006B578/s9/pass_attribution.txt`.
*kill_scope.* instance — measured on the candidate.c chassis (score 2, 200/200 insns), zero
FAKE constructs present (`fake_ablate`: nothing to ablate), `src/text1b.c` reverted to the
`INCLUDE_ASM` stub before session end.

### H22 — KILLED (instance). Re-audit of the stale s2 kill on the current chassis.
*Statement.* Inlining the shared `0x100010` tail-check + `return var_s2` into all three of
switch cases 3/4/5 (removing every `goto tail` from them) improves the score on the CURRENT
chassis, even though it regressed on the superseded floor-34 chassis of session 2.
*Mechanism to test.* s2 measured this variant against a body that still carried two
structural bugs fixed in s3, so the kill's chassis is stale; the mandated kill re-audit
requires re-measuring the stale kill that sat closest to the target.
*Probe.* `tmp/grind/func_8006B578/s9/v_inline_tail_345.c` applied to `src/text1b.c`;
`sandbox func_8006B578 --disable all`.
*Result.* KILLED — score **15** at 200/200 insns (baseline 2 at 200/200). Still a
regression, so the s2 kill stands, but its character changed: s2 recorded 44 at 202 insns,
i.e. an instruction-count divergence; on this chassis the count is right and the damage is
ordering/operand only. Banked at
`rejected/all-three-cases-inline-tail-recheck-s2-kill-on-current-chassis-score15.c`.
*kill_scope.* instance — candidate.c chassis, ordinary C in both bodies, no FAKE constructs
in either (`fake_ablate`: nothing to ablate), `src/text1b.c` reverted to the `INCLUDE_ASM`
stub before session end.

### H23 — CONFIRMED (tooling). The dispatch-time chassis check failed on an encoding defect, not a regression.
*Statement.* The driver's "measurement unavailable" chassis reading for this function was
caused by a stray cp1252 `0x97` byte inside `memory/grind/func_8006B578/candidate.c`.
*Mechanism.* `engine/inlineasm.py:393` reads the candidate-applied `src/<stem>.c` with
`encoding="utf-8"`; a lone `0x97` makes that read raise `UnicodeDecodeError`, which aborts
`sandbox_score` before any compilation happens.
*Probe.* Reproduced the traceback verbatim on first apply; sanitized the byte; re-ran.
*Result.* CONFIRMED — after sanitization the chassis measures 2 / 200 / 200 exactly as in
sessions 3-8. Guidance for future sessions: keep ledger C files UTF-8-decodable.

## [s9] With candidate.c applied, text1b.o(.rodata) holds six R_MIPS_32 words that, once resolved against text1b.o(.text)'s live base, equal the six words of the extracted const u32 jtbl_80015988[6] at src/text1a_b_pre_rodata.c:409-416.
- mechanism: GCC's casesi vector is emitted as .word .L<label> entries carrying section-relative R_MIPS_32 relocations, while the target's table was extracted by the 2026 rodata cleanup as absolute u32 literals; equality reduces to 'same addends plus correct section base', which objdump can measure without a full build.
- probe: objdump -h/-s/-r/-t tmp/sandbox/func_8006B578/text1b.o (the candidate's own object) and objdump -t build/src/text1a_b_pre_rodata.o.
- result: CONFIRMED. .rodata size 0x18 (6 words), Algn 2**3, six R_MIPS_32 relocs against .text with addends 0x9554 0x9584 0x95BC 0x9650 0x9698 0x96C4. func_8006B578 is at .text+0x9414 and links at 0x8006B578, so the section base is 0x80062164; addend+base = 0x8006B6B8 0x8006B6E8 0x8006B720 0x8006B7B4 0x8006B7FC 0x8006B828, element-for-element the extracted array. Session 7 asserted this content-identity from the C source; this is the first measurement from the compiled object, including relocation type, slot and addend.
- verdict: CONFIRMED

## [s9] Deleting jtbl_80015988 from src/text1a_b_pre_rodata.c and inserting build/src/text1b.o(.rodata) at 0x80015988 leaves every symbol outside the 24-byte hole at its current address.
- mechanism: Output-section placement inserts per-object alignment padding; a mismatch between our case vector's .align 3 and the hole's start, or between the hole's end and the post-split TU's alignment, would shift everything downstream and break the SHA1 the handoff is meant to produce.
- probe: objdump -t/-h build/src/text1a_b_pre_rodata.o for the three neighbouring symbol offsets and the section alignment, checked against our object's 2**3 alignment.
- result: CONFIRMED. jtbl_80015940 at .rodata+0x550 size 0x48 (ends +0x598), jtbl_80015988 at +0x598 size 0x18, D_800159A0 at +0x5B0; the hole is exactly 24 bytes at 0x80015988. 0x80015988 % 8 == 0, so our 8-aligned section lands with zero leading padding and ends exactly at 0x800159A0, which satisfies the post-split half's 2**2 alignment with zero padding. The last unmeasured risk in the handoff plan is closed.
- verdict: CONFIRMED

## [s9] The two masked dispatch instructions (lui at,%hi / lw v0,%lo(at)) that pin the sandbox score at 2 are produced by a GCC optimization pass whose input shape the C can vary.
- mechanism: The forensics-modality PASS-INPUT ENUMERATION discipline: name the pass from the instrumented-cc1 dumps, then enumerate the source shapes that change what that pass sees.
- probe: pwsh tools/grinder/dump.ps1 func_8006B578; read dumps/text1b.rtl plus .jump .cse .cse2 .loop .combine .flow .lreg .greg .sched .sched2 .jump2 .dbr and dumps/text1b.s.
- result: KILLED. dumps/text1b.rtl:51828 carries (jump_insn 474 473 475 (addr_vec:SI[...])) in the FIRST RTL dump - the vector is built by RTL generation (stmt.c expand_end_case -> the casesi expander) before any optimization pass runs, and no later dump alters it. final.c emits a single macro load 'lw $2,.L988($2)' plus the .rodata/.align 3/.word vector; maspsx/ASPSX expands that one load into the observed lui/lw pair. No optimization pass sits in the causal chain on this chassis, so this session found no pass input to enumerate; the free variable is the local label's final address, which bb2.ld owns.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis applied to src/text1b.c, sandbox --disable all score 2 (200/200 insns, 22/22 hunks not-scored); zero FAKE constructs present (tools/fake_ablate.py has nothing to ablate); src/text1b.c reverted to the INCLUDE_ASM stub before session end.

## [s9] Inlining the shared 0x100010 tail-check plus return var_s2 into all three of switch cases 3/4/5 (removing every goto tail from them) improves the score on the current chassis, even though it regressed on the superseded floor-34 chassis of session 2.
- mechanism: Session 2 measured this cross-jump/tail-merge variant against a body still carrying two structural bugs that session 3 fixed, so that kill's chassis is stale; the mandated kill re-audit requires re-measuring the stale kill whose form sat closest to the target.
- probe: tmp/grind/func_8006B578/s9/v_inline_tail_345.c applied to src/text1b.c; sandbox func_8006B578 --disable all.
- result: KILLED - score 15 at 200/200 insns against the baseline's 2 at 200/200. The session-2 kill stands (the form is still a regression) but its character changed: s2 recorded 44 at 202 insns, an instruction-count divergence, whereas on this chassis the count is right and the damage is ordering/operand only. Banked at memory/grind/func_8006B578/rejected/all-three-cases-inline-tail-recheck-s2-kill-on-current-chassis-score15.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis (score 2, 200/200) vs the variant; ordinary C in both bodies, no FAKE constructs in either (fake_ablate: nothing to ablate); src/text1b.c reverted to the INCLUDE_ASM stub before session end.

## [s9] The driver's dispatch-time CHASSIS CHECK reported 'measurement unavailable' because of an encoding defect in the ledger's candidate.c, not because of a chassis regression or a tooling outage.
- mechanism: engine/inlineasm.py:393 reads the candidate-applied src/<stem>.c with encoding='utf-8'; a lone cp1252 0x97 byte inside candidate.c makes that read raise UnicodeDecodeError, aborting sandbox_score before any compilation happens.
- probe: Applied candidate.c verbatim and reproduced the traceback (UnicodeDecodeError on byte 0x97 at position 203196 of the spliced src/text1b.c); located the byte at candidate.c offset 3192; sanitized it to ASCII and re-ran the sandbox.
- result: CONFIRMED. After sanitization the chassis measures score 2, target_insns 200, build_insns 200, 22/22 hunks not-scored - identical to sessions 3-8. candidate.c is now UTF-8-decodable; future sessions writing ledger C from the Windows side must keep it so, or the driver's chassis probe silently fails again.
- verdict: CONFIRMED

## H17 (s10, rederive) — CONFIRMED: candidate.c chassis re-verified at floor 2 (200/200 insns, 0 source-level, 0 operand-only, 22 not-scored) with candidate.c applied fresh to src/text1b.c this session
- mechanism: plain re-measurement; no C change from the banked body.
- probe: applied memory/grind/func_8006B578/candidate.c verbatim to src/text1b.c, ran `sandbox func_8006B578 --disable all --diff`.
- result: score 2, target_insns 200, build_insns 200, 22/22 hunks not-scored, all 22 unmasked hunks are branch/jump absolute-address literals (e.g. `beq v1,v0,23718` vs `beq v1,v0,9484`) that differ only because the whole-object build places the function at a different absolute address than the target link — expected and consistent with sessions 3-9's account. No new source-level or operand-only hunk appeared.
- verdict: CONFIRMED
- kill_scope: n/a (confirm, not kill)

## H18 (s10, rederive) — KILLED (instance): rewriting the second dispatch (`(D_800A34F8>>10)&7`, cases 0-5) as an if-else chain instead of a `switch` regresses the score and does not touch the jtbl_80015988 residual
Statement: replacing the `switch ((u32)D_800A34F8 >> 10 & 7) { case 0: ... case 5: ... }` block with
an equivalent `s32 dk = ...; if (dk==0) {...} else if (dk==1) {...} ... else if (dk==5) {...}` chain
(same statements, same goto targets, same shared_400040/tail labels) was hypothesized as a
structurally-different rederivation that might let the compiler reach the target's dispatch bytes
through a different codegen path than the ADDR_VEC jump table our switch produces, potentially
sidestepping the jtbl_80015988 cross-TU symbol issue documented in H13/H16 and candidate.c's header.
mechanism: GCC 2.7.2's stmt.c `expand_end_case` only emits an ADDR_VEC jump-table dispatch
(`case_stmt` → `casesi`/`tablejump`) for a `switch` whose case density crosses its jump-table
threshold; a hand-written if-else chain instead compiles to a linear chain of `beq`/`bne` compares,
never synthesizing a jump table at all. Confirmed by direct read of asm/funcs/func_8006B578.s:83-88
(`lui $at,%hi(jtbl_80015988); addu $at,$at,$v0; lw $v0,%lo(jtbl_80015988)($at); jr $v0`) — the
target itself dispatches through the SAME kind of ADDR_VEC jump table, so an if-else chain is
structurally further from target, not closer.
probe: edited src/text1b.c in place (candidate.c chassis with only the second switch rewritten to
the if-else form above), ran `sandbox func_8006B578 --disable all`.
result: score regressed 2 -> 16, insn count 200 -> 206 (build now emits 6 more insns than target,
consistent with a `beq` chain replacing a 4-insn jump-table dispatch). Reverted immediately back to
the switch form, re-verified score 2 / 200 insns, then reverted src/text1b.c to the committed
`INCLUDE_ASM("asm/funcs", func_8006B578);` stub (git diff clean against HEAD after revert).
verdict: KILLED
kill_scope: instance
measured_on: func_8006B578 candidate.c chassis with the second switch replaced by an if-else chain
(the rest of the body byte-identical to memory/grind/func_8006B578/candidate.c); zero FAKE/cheat
constructs in either form (ordinary switch vs ordinary if-else, both ordinary C).

## [s10] candidate.c chassis re-verified at floor 2 (200/200 insns, 0 source-level, 0 operand-only, 22 not-scored) with candidate.c applied fresh to src/text1b.c this session
- mechanism: plain re-measurement; no C change from the banked body
- probe: applied memory/grind/func_8006B578/candidate.c verbatim to src/text1b.c, ran sandbox func_8006B578 --disable all --diff
- result: score 2, target_insns 200, build_insns 200, 22/22 hunks not-scored; all 22 unmasked hunks are branch/jump absolute-address literals differing only by whole-object base address; no new source-level or operand-only hunk appeared
- verdict: CONFIRMED

## [s10] rewriting the second dispatch (D_800A34F8>>10&7, cases 0-5) as an if-else chain instead of a switch regresses the score and does not avoid the jtbl_80015988 residual
- mechanism: GCC 2.7.2 stmt.c expand_end_case only synthesizes an ADDR_VEC jump-table dispatch for a real switch statement past its case-density threshold; a hand-written if-else chain compiles to a linear beq/bne compare chain and never reaches that RTL shape. asm/funcs/func_8006B578.s:80-88 shows the target itself dispatches this range through the identical kind of ADDR_VEC jump table, so an if-else chain is structurally further from target, not closer.
- probe: edited src/text1b.c in place (candidate.c chassis with only the second switch rewritten to an equivalent if-else chain, same statements/gotos/labels), ran sandbox func_8006B578 --disable all, then reverted
- result: score regressed 2 -> 16, insn count 200 -> 206 (6 more insns than target). Reverted immediately, re-verified score 2/200 insns on the switch form, then reverted src/text1b.c to the committed INCLUDE_ASM stub (git diff clean against HEAD)
- verdict: KILLED
- kill_scope: instance
- measured_on: func_8006B578 candidate.c chassis with only the second switch replaced by an if-else chain (rest of body byte-identical to memory/grind/func_8006B578/candidate.c); zero FAKE/cheat constructs in either form
