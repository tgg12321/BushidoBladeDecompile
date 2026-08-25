# SELF-VET — func_8003800C (damage_DebugDisp), grind session 16 (synthesis)

Diff: `src/code6cac_c_mid.c` — `INCLUDE_ASM("asm/funcs", func_8003800C);` replaced by
the C body in `memory/grind/func_8003800C/candidate.c`.
Measurements THIS session, with the diff in place:
`sandbox func_8003800C --disable all` = **score 0**, target 79 / build 79,
**rules_dropped 0** (this function has no regfix/asmfix rules); full `build`
SHA1 = `62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle (**MATCH**).
No file outside `src/code6cac_c_mid.c` is touched by the diff.

CONSTRUCTS: (1) single function-scope counter `j` reused by the checksum loop and the
fixup loop (previously two locals `j` and `k`); (2) unsigned bound `j < 0x24U` on the
checksum loop; (3) index-based Region-B addressing `base + j*4 + 0x78` / `base + j*2 + 0xD0`
(inherited, Judge-PASSed 2026-07-22 06:02); (4) statement order `sum = 0; bp = ...; j = 0;`
in the inner-loop preheader.
NOTE — constructs REMOVED by this session that earlier candidates carried: both
`do { x = 0; } while (0);` brackets (Region A RA-weighting, Region B biv-init fold) and
the `for(;;) { ...; if (p != end) continue; ...; break; }` fence on the CopyBlock loop.
The matched body contains no do-while(0), no dead store, no dead local, no alias, no
volatile, no `__asm__`, no register pin, no constant holder.

## T1 semantic purpose
(1) `j` is a live, read-and-written loop counter in BOTH loops; each loop needs a counter,
and removing the variable is impossible. Its value is consumed by the loop tests and (in the
fixup loop) by the address arithmetic. It is not a no-op: with two counters instead of one the
program is identical in behaviour but the register allocator seats `sum` differently — that is
a codegen effect of a REAL program element, not the effect of an inserted non-element.
(2) `0x24U` makes the index comparison unsigned; `j` is a byte offset that is never negative,
so unsigned is the semantically correct comparison, and it selects `sltiu` (target) over `slti`.
(3) The index expressions address exactly the memory the walking-pointer form addresses
(0x78 stride 4, 0xD0 stride 2) — proven byte-exact and by the full-build SHA1.
(4) Statement order is ordinary source order; the three inits are independent.
No construct in the diff is behaviour-neutral-but-present: every statement computes or
consumes a value the function needs.

## T2 human-programmer
Yes. The body is C89 with all scalars declared at the top of their block, one shared loop
counter, a `do/while` copy loop and a `do/while` fixup loop. A reader asks "why is this here?"
about nothing in it: there is no wrapper, no duplicated statement, no assignment whose value
is never read. Compared with the s1–s15 candidate this form has FEWER declarations and FEWER
constructs — the two-separate-counters spelling was the agent-invented shape; one reused `j`
is what a 1998 PsyQ-era author would write (SOTN master ships exactly this, annotated:
`// fake reuse of i?`).

## T3 GCC-internals justification
Disclosed in full, because it is how the form was FOUND: `tools/ra_solver` (global.c model,
validated exact on this function — sort order MATCH, dispositions 17/17) shows that on the
target's source order (`sum=0` first) the seats are decided by `allocno_compare`
priority = floor_log2(n_refs)*n_refs/live_length*10000*size: two-counter chassis gives
sum(77) 27272 (n=10, LL=11) vs j(79) 47142 (n=11, LL=7), so j is allocated first and takes
`$a0`. `inverse.py --goal '{"77":4,"79":5,"78":3}'` returned ONE single-atom vector:
`live_extend pseudo 79: live length 7 -> 15`, i.e. lengthen j's live range. Merging the two
counters is the C that does it. The mechanism is named because the rules require it — but the
construct is not a mechanism-only artifact: it survives the "would a human write this"
question on its own (T2), which is the test that distinguishes a lever from a cheat here.
The pass attribution is measured, not guessed (ra_solver model + sched_solver dependence dump).

## T4 permuter/search provenance
Not from a permuter. ~173k permuter iterations across s4/s5/s13/s14 never found this form
(they searched statement order and the sum-def mechanism, never variable identity). This form
came from two exact models: `tools/sched_solver` (100% order- and clock-exact on this TU;
19/19 blocks exact for this function) proved the preheader residual is not a schedulable
tie-break, and `tools/ra_solver`'s inverse solver named the live-range atom. It is not
"passes the detectors because they don't catch this spelling": it contains nothing any
detector targets, and it is confirmed by the oracle SHA1, not by a score heuristic.

## T5 family check
No forbidden family matches, by shape or by analogy: no register-asm pin, no `$N` asm, no
regfix, no scheduling barrier, no volatile of any spelling, no alias rename, no unused local
or array, no dead/self/conditional store, no `if (1)`/`do{}while(0)`/dead-goto wrapper, no
DImode chain, no opaque constant holder, no combine-foldable chain-extender, no redundant
width cast, no linker/rodata reorder. The one construct with a codegen motive — reusing one
variable for two values — is the FIRST entry of the frozen SOTN-accepted list (claimed below).
`0x24U` is a type choice on a comparison, not a width cast on a value (F2 is about redundant
casts inserted to change an operation's width; nothing here is cast).

## T6 naming-announces-intent
Every name is semantic: `base`, `chkptr`, `offset`, `sum`, `bp`, `i`, `j`, `src`, `dst`,
`sp2`, `end`, `ptr`. No `pad`, `dummy`, `unused`, `spill`, `tmp`, `slack`. Every declared
variable is both written and read on a live path.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Variable reuse for codegen control
  SCOPE: "**Variable reuse for codegen control** ([[defeat-licm-hoist-var-reuse]]): reusing one C variable for two unrelated values to influence loop-invariant detection or RA. SOTN ships `idxSub = idxSub;` and `randy = basePoint.x; baseX = randy;` with "FAKE but makes register allocation work" comments."
  PRECEDENT: .claude/rules/no-new-park-categories.md:170
  PRECEDENT: docs/reference/sotn-construct-index.md:51
  (the second is SOTN master `src/boss/mar/cutscene.c:172` — `// fake reuse of i?`, the
   same construct: an annotated reuse of a loop counter. PSX/GCC entry, untagged.)
  PREREQUISITES: lever-exhaustion documented across 15 prior sessions in
  `memory/grind/func_8003800C/hypotheses.md` (structural s1–s3/s10–s12, forensics s6/s7/s15,
  rederive s8/s9, permuter s4/s5/s13/s14 ≈173k iters) — this is not a first-reach lever;
  mechanism named (global.c `allocno_compare` live_length input); annotation present.

ANNOTATION-CONFORMANCE:
/* FAKE: one counter 'j' serves both the per-record checksum loop and the
   0x16-entry fixup loop (C89 counter reuse), mechanism: global.c
   allocno_compare -- the merged live range lifts reg_live_length(j) so j's
   allocno priority falls below sum's and sum takes $a0 (target's seat),
   lever-exhaustion: memory/grind/func_8003800C/hypotheses.md s1-s15 */
(what = the counter reuse; mechanism = global.c allocno_compare via reg_live_length;
 lever-exhaustion = the named ledger file. Present on the declaration of `j` in
 `src/code6cac_c_mid.c`.)
