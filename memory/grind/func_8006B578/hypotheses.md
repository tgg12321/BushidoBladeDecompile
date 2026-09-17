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
1. **Cross-jump/tail-merge shape around case-2 → shared-block transition** (target-insn ~122-158
   region). `sandbox --diff` shows target's instruction stream differs in COUNT/shape from ours
   right where case 2 falls into the shared post-switch block and where case 3's tail
   (`D_800A34F8 = ...0x4000; var_s2=2;`) is laid out. Next probe: run
   `pwsh tools/grinder/dump.ps1 func_8006B578` and read the `.sched`/`.combine` dump for this
   region BEFORE guessing further — this is exactly the situation the PASS ATTRIBUTION step
   exists for and it was not done this session (turn budget). Do not re-guess branch/statement
   order blindly; read the dump first.
2. **We are 199 insns vs target's 200 (one UNDER).** Check whether target keeps an explicit `nop`
   (delay-slot filler, e.g. after a taken branch with nothing to fill) that our current structure
   optimizes away, rather than assuming we're "missing" real logic. Compare insn-for-insn around
   the hunk-16/17 region of the last `--diff` run (already captured in evidence.md).
3. **Case 3's tail store/const ordering** — target's asm at that point is `... sw v0,0(gp); j
   23980; li s2,2` as a group; try reordering the `D_800A34F8 = ...; var_s2 = 2;` statements in
   case 3 (swap which is written first) and re-measure; a 2-line reorder is cheap to try before
   reaching for the dump.

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
