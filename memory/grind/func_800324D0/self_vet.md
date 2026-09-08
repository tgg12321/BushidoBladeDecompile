# SELF-VET — func_800324D0

CONSTRUCTS: (1) the header advance `ptr += 5` written as a four-statement
chain `ptr++; ptr++; ptr++; ptr += 2;`; (2) the 0xFF command's advance
`ptr += 6` written as `ptr++; ptr += 5;`; (3) the loop tail
`c = *ptr; ptr++;` duplicated into the first five switch arms (which then
`continue`) instead of being reached by falling out of the switch.

## T1 semantic purpose
(1) and (2) are LIVE computations of the walker pointer: every intermediate
value is the real pointer value and the final value is the one the loop
consumes. They are algebraically identical to the single-statement form and
combine folds them back to it, so the emitted bytes are the single-statement
form's bytes (build_insns 68 == target_insns 68; full-EXE SHA1 ==
62efab4f73f992798c43e8c730aa43baa10bb4fa). Their only surviving effect is the
`reg_n_refs` count flow.c records before the fold — which is precisely the
sanctioned combine-foldable chain-extender clause, and precisely why the
clause requires the FAKE annotation. (3) is a REAL statement (the next
command byte must be fetched and the pointer advanced) executed on those five
paths; duplicating it rather than sharing one copy is the sanctioned
duplicated-statement-into-arms spelling, and jump2's cross-jump re-merges the
copies so no duplicated instruction materialises. All three are
FAKE-annotated in the body.

## T2 human-programmer
(3) reads as ordinary code a human writes without thinking about it (each
command arm finishes by fetching the next command byte); SOTN master ships
7-arm and 11-arm instances of exactly this. (1) and (2) would make a reader
ask "why not `ptr += 5` / `ptr += 6`?" — they do NOT pass the naive
human-programmer test on their own, which is exactly why they are claimed
under a named FAKE-annotated family rather than as ordinary C, and why each
carries a `/* FAKE: ... */` line naming the pass and the exhaustion ledger.

## T3 GCC-internals justification
Yes, and it is stated openly in the annotations rather than disguised as
program logic: flow.c's life_analysis sets `reg_n_refs` (weighted by loop
depth) BEFORE combine folds the chains, and global.c's `allocno_compare`
ranks allocnos by `floor_log2(nrefs) * nrefs * size / live_length`. Measured
this session from the instrumented cc1's ALLOCDBG output: the walker allocno
must outrank the payload carrier (47272) for find_reg to seat it in $v1, and
this body puts it at 53333 with the 0xFF head-test constant still hoisted
(loop.c:1631, insn_count 58 <= threshold 58). Naming a GCC pass is a
REQUIREMENT of both claimed families, not a workaround of the checklist —
both are FAKE-annotated last-resort families whose annotation template
mandates a named mechanism.

## T4 permuter/search provenance
No permuter or automated spelling search was used. The form was derived
forward from the measured allocno-priority arithmetic (ra_solver
extract.py/simulate.py + the .greg/.lreg/.loop dumps), then confirmed by
sandbox and by a full-EXE SHA1 build. The exact ref/live-length deltas of
each construct were measured individually before the body was assembled.

## T5 family check
(1) and (2): combine-foldable chain-extender, the 2026-07-01 same-day scope
extension of dead-store-fake-exception. Its extra prerequisite ("verify the
fold actually emits zero bytes — same insn count + no new address
materialization") is verified: build_insns 68 == target_insns 68, no new
address materialization, SHA1 == oracle. (3): duplicated-statement-into-arms,
byte-neutrality verified by the same measurement. Neither is a register pin,
hardcoded-$N asm, scheduling barrier, volatile coercion, dead local/array,
dead store, alias rename, or any other forbidden-family spelling; nothing in
the body is dead (every statement's value is consumed).

## T6 naming-announces-intent
No new names are introduced. The three locals are `ptr`, `c`, `val`, all
inherited from the pre-existing chassis and all read; no `pad`/`dummy`/
`unused`/`spill`/`tail`/`slack` names appear.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: combine-foldable chain-extender (dead-store-fake-exception scope extension, owner ruling 2026-07-01 same-day)
  SCOPE: "**combine-foldable chain-extender** (scope extension, owner ruling 2026-07-01 same-day): a LIVE store/computation routed through an algebraically-equivalent detour that combine folds back to the direct form with ZERO emitted bytes — its only surviving effect is the extra `reg_n_refs` count flow.c records before the fold."
  PRECEDENT: .claude/rules/dead-store-fake-exception.md:51

  FAMILY: duplicated-statement-into-arms
  SCOPE: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): duplicating a REAL statement into 2+ arms (instead of label-sharing) is legitimate — incl. when cross-jump re-merges the copies to identical bytes and the effect is a reg_n_refs priority lift."
  PRECEDENT: .claude/rules/duplicated-statement-into-arms.md:5

ANNOTATION-CONFORMANCE:
  /* FAKE: the header advance `ptr += 5` is spelled as a four-step chain, mechanism: combine folds the four `addiu` insns back to the single `addiu $v1,$v1,5` the target carries (zero emitted bytes, build_insns 68 == target_insns 68), and the only surviving effect is the extra reg_n_refs count flow.c records BEFORE the fold, which lifts the walker allocno's global.c allocno_compare priority above the payload carrier's so find_reg seats the walker in $v1, lever-exhaustion: memory/grind/func_800324D0/hypotheses.md s1-s23 */
  /* FAKE: same combine-foldable chain-extender as above, applied to the 0xFF command's `ptr += 6` advance, mechanism: combine folds `addiu 1; addiu 5` back to the target's single `addiu $v1,$v1,6`, contributing reg_n_refs inside the loop (loop depth 2) without adding a final instruction, lever-exhaustion: memory/grind/func_800324D0/hypotheses.md */
  /* FAKE: the loop tail `c = *ptr; ptr++;` is duplicated into the first five command arms instead of being reached by falling out of the switch, mechanism: flow.c's reg_n_refs census counts the duplicated walker references before global.c's allocno_compare ranks the allocnos, and jump2's cross-jump pass (after reload) re-merges the identical tails so not one duplicated instruction materialises, lever-exhaustion: memory/grind/func_800324D0/hypotheses.md */
  All three carry what + named GCC pass + lever-exhaustion pointer.
