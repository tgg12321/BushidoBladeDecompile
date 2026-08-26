# SELF-VET — func_80056FE8 (ang_hosei_80056FE8), grind session s7c (forensics)

Measured THIS session with this exact body in `src/text1b.c`:
`sandbox func_80056FE8 --disable all` = **score 0**, `build_insns 43 == target_insns 43`,
`rules_dropped 0`, `scorable true`. `verify-oracle` = `ok true`,
`build_sha1 == original_sha1_locked == 62efab4f73f992798c43e8c730aa43baa10bb4fa`.

CONSTRUCTS: `base += <arm value>;` written once per dispatch arm (three copies) instead of caching the adjustment in a temp and adding it once after the join.

That is the ONLY match-hack construct in the diff. Everything else in the body is
ordinary C: the m2c-style pointer casts for the struct fields, the three table
lookups, and the inline `*arg0` re-read in the return expression. No register pins,
no `__asm__`, no `volatile`, no dead stores, no dead locals, no variable reuse, no
scheduling barrier, no regfix/asmfix rule (rule count is zero; the function is
committed as `INCLUDE_ASM` at HEAD and this diff replaces that line only).

## T1 semantic purpose
`base += <arm value>` has a real semantic purpose on every path it appears on: it is
the arm's angle adjustment being added into the accumulator. Remove any one copy and
that arm computes a different answer — the function's output changes. The construct
is therefore NOT behaviour-neutral in the T1 sense (a dead store or a discarded read
would be). What IS byte-neutral is the CHOICE of writing the statement three times
rather than once after the join: both spellings compute the same values, and GCC's
`jump2` post-reload cross-jumping re-merges the three copies into one instruction, so
the duplication materialises no extra bytes. That is exactly the condition the
`duplicated-statement-into-arms` family is defined around (prerequisite 2), not a T1
failure.

## T2 human-programmer
Yes, a human would write this. "Each case adds its own correction into the running
total" is the plainest reading of the routine, and it is DRY-neutral: the alternative
spelling costs an extra named temporary whose only job is to carry a value across a
join it does not need to cross. A reader would not ask "why is this here?" about any
of the three statements — each one is the arm's whole point. The target asm supports
the same reading: `asm/funcs/func_80056FE8.s` has three arm-local adds branching to a
single join `addu $a1,$a1,$v0` at `L47874`, which is precisely what this C emits.

## T3 GCC-internals justification
Disclosed rather than denied, because the family's own prerequisites require the
mechanism to be named. The program-logic explanation stands on its own (T2 above);
the GCC mechanism is why THIS spelling of the same logic is the one that matches.
Mechanism, measured this session on the annotated form (`pwsh tools/grinder/dump.ps1
func_80056FE8`, dumps in `tmp/grind/func_80056FE8/dumps/`, function slices in
`tmp/grind/func_80056FE8/s7/s7c_*.txt`):
  - `flow.c` records 8 expand-time / 9 post-combine references for pseudo 77 (`base`)
    on the per-arm form vs 4 on the post-join form.
  - `global.c` `global_alloc` therefore sorts `base` FIRST:
    `.greg` reads `;; 3 regs to allocate: 77 73 72` → dispositions
    `72 in 4 ($a0)  77 in 5 ($a1)  73 in 6 ($a2)` = target-exact.
    The post-join spelling reads `;; 4 regs to allocate: 82 73 77 72` →
    `73 in 5  77 in 6` = the $a1/$a2 swap, plus a fourth allocno (the temp) the target
    does not have. (Rejected form banked at
    `memory/grind/func_80056FE8/rejected/postjoin-temp-combine-p73-colored-first.c`.)
  - `jump2` cross-jumping collapses the accumulate insns 4 → 2 (`.greg` 4, `.sched2` 4,
    `.jump2` 2, `.dbr` 2): the three per-arm `addu`s become the target's single join
    `addu`, and the remaining one is the return add. Byte-neutrality proven twice over
    (43 == 43 insns; full-build SHA1 == oracle).
This is a DISCLOSED mechanism inside a sanctioned family, which the family's
prerequisite 4 (`/* FAKE: ... */` naming the mechanism) mandates — not an undisclosed
internals lever offered in place of program logic.

## T4 permuter/search provenance
NOT search-derived, and this is the specific charge the 2026-08-26 02:34 layer-1 FAIL
raised. It is answered by re-derivation: session s7b reproduced the entire finding
WITHOUT the RA solver, from (a) the target asm's three-arm-add / one-join-add shape in
`asm/funcs/func_80056FE8.s`, and (b) a single standard `-da` `.greg` dump of the
plainest alternative spelling, which prints its colour order directly. This session
(s7c) re-ran that dump from scratch on the submitted body and reproduces
`;; 3 regs to allocate: 77 73 72` → `77 in 5  73 in 6`. No permuter output is in this
diff; s4/s5's permuter campaigns produced only the `base++; base--;` dead-op cheat,
which is banked as REJECTED and is not this form. The construct also passes the
detectors because there is nothing to detect: it is three real assignments.

## T5 family check
Match: **duplicated-statement-into-arms**, the frozen SOTN-list entry at
`.claude/rules/no-new-park-categories.md:285-293`. The family-selection table's row
for "A REAL statement duplicated into 2+ arms" points at exactly this rule file, and
its hard bounds ("statement must be real + byte-neutral re-merge; a same-value re-store
is dead-store, not this. FAKE required") are all satisfied: the statements are real
(T1), the re-merge is byte-neutral (jump2 4→2, 43==43, SHA1==oracle), none of them is
a same-value re-store. I checked the adjacent families and they do NOT fit:
`dead-store-fake-exception` (these are not dead — removing one changes the result),
`hoist-shared-arm-computation-defeats-copy-pref` (that family hoists computation OUT of
arms; this sinks a statement IN — and citing it is BANNED for this function by the
2026-07-12 ruling; it is not cited here), `split-read-defeats-hoist` (reads, not
stores), `defeat-licm-hoist-var-reuse` / `staged-value-reused-variable` (no variable is
borrowed or reused here — `base` is a fresh local carrying its own single value),
`proven-spelling-class-reconstruction` (ruled the wrong door by the 2026-08-26 02:52
Judge ruling; not cited here). No forbidden-catalog family matches: nothing is a
register pin, a hardcoded-`$N` asm, an alias rename, a volatile coercion, a dead
local, a frame pad, an `if (1)` wrapper, or a scheduling barrier.

## T6 naming-announces-intent
No coercion-announcing names. The identifiers are `arg0`, `a2`, `a3`, `base` — `base`
is the accumulator and is read in the return expression; every local is written AND
read. Nothing is named `pad`, `dummy`, `unused`, `spill`, `tmp`, or `slack`; nothing is
address-of'd, discarded, or declared-only. The three duplicated statements carry
`/* FAKE: ... */` markers, which announce MATCH-MOTIVATION as the family's
prerequisite 4 requires — the opposite of hiding intent.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: duplicated-statement-into-arms
  SCOPE: "Writing the SAME real statement in two or more control-flow arms — instead of sharing one copy via a label/goto — is a legitimate matching technique, **including** when: GCC's jump2 cross-jump re-merges the copies so the final bytes are identical to the shared-label form, and the duplication's surviving effect is the extra `reg_n_refs` count flow.c records (allocno-priority lift for global RA), and the label placement among the copies is chosen to steer the merge DIRECTION (which copy survives inline vs becomes the jump)."
  PRECEDENT: `.claude/rules/duplicated-statement-into-arms.md:63`
  PRECEDENT: `.claude/rules/no-new-park-categories.md:285`

Prerequisite roll-call for that family (rule file lines 48-59):
  1. Statement REAL on its path — YES (T1; removing a copy changes the result).
  2. Byte-neutrality verified — YES (`build_insns 43 == target 43`, sandbox score 0,
     full-build SHA1 == oracle 62efab4f73f992798c43e8c730aa43baa10bb4fa, this session).
  3. Lever-exhaustion documented — YES, `memory/grind/func_80056FE8/hypotheses.md` and
     `evidence.md` sessions s1–s7b: structural reassociation (21 forms, s2/s3),
     permuter (4 chassis / ~150k iters, s4/s5), copy-preference (s6 — `$a1` has no ABI
     anchor in this 1-argument leaf; `set_preference`, global.c:1591, only makes
     hard-reg prefs from reg-to-hard-reg copies), scheduling wrappers (s6 — do-while(0)
     at 3 placements never shrinks `base`'s live length), live-range shortening (s6 —
     backfires, raises the competitor's priority), register pins (s6 — reschedule to 41
     insns, 2 load-delay nops lost). Summarised in the function's preamble comment.
  4. `/* FAKE: ... */` annotation — YES, see ANNOTATION-CONFORMANCE below.
  5. Layer-1 + layer-2 review — requested via this submission.

ANNOTATION-CONFORMANCE:
  /* FAKE: `base += <arm value>` duplicated into all three dispatch arms
   * instead of a post-join combine; mechanism: GCC 2.7.2 jump2 post-reload
   * cross-jumping tail-merges the three copies into the target's single join
   * `addu $a1,$a1,$v0` (byte-neutral, build 43 == target 43), while the
   * reference-count lift flow.c records (reg_n_refs 4 -> 8) raises `base`'s
   * global.c allocno priority above the struct pointer's so global_alloc
   * colours `base` first (.greg `;; 3 regs to allocate: 77 73 72` ->
   * `77 in 5  73 in 6`) -- which the post-join spelling provably cannot
   * (`;; 4 regs to allocate: 82 73 77 72` -> `73 in 5  77 in 6`);
   * lever-exhaustion: see the ladder in this function's preamble comment and
   * memory/grind/func_80056FE8/hypotheses.md s1-s7b. */
  plus a short `/* FAKE: duplicated copy (see above) */` on each of the three
  duplicated `base +=` statements, so every copy carries the annotation as
  prerequisite 4 words it ("annotation on the duplicated copy").
  All three parts present: WHAT (per-arm duplication vs post-join combine),
  MECHANISM (named GCC passes: `jump2` post-reload cross-jumping; `flow.c`
  reg_n_refs; `global.c` `global_alloc` colour order), LEVER-EXHAUSTION (pointer to
  the s1–s7b ladder in hypotheses.md/evidence.md and the preamble comment).

Citations deliberately NOT made (both previously ruled wrong for this function):
`hoist-shared-arm-computation-defeats-copy-pref` (BANNED, owner ruling 2026-07-12) and
`proven-spelling-class-reconstruction` (ruled the wrong door, Judge 2026-08-26 02:52).
