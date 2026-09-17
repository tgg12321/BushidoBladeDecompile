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
- measured_on: func_8006B578, src/text1b.c, ordinary C only, no FAKE/cheat constructs present in either the killed variant or the restored floor-34 form
