# SELF-VET — func_8002CA8C  (s2 annotation-fix, 2026-09-15, re-filed WITH a `candidate-ready` after the 21:32 Judge FAIL on annotation presence)

Body under vet: `memory/grind/func_8002CA8C/candidate.c` (= tmp/grind/func_8002CA8C/s2/v13_ascii_comment.c; same body as the Judge-reviewed v12_final.c, only the FAKE comment text differs: a cp1252 0x97 byte replaced by an ASCII hyphen),
spliced into src/code6cac_b.c replacing the `INCLUDE_ASM("asm/funcs", func_8002CA8C);` line. Measured
THIS session with the edits in place: `sandbox func_8002CA8C --disable all` == **0**
(target_insns 179, build_insns 179), on the -mel -msoft-float chassis. `git diff src/code6cac_b.c` is
one hunk (the function body); no other tracked file outside memory/grind/func_8002CA8C/ is touched.

CONSTRUCTS: (1) the existing `hit` status local reused to hold the AABB reject flag (`hit = 0; ... hit = 1` on
reject; `if (hit != 0) continue;`; then `hit = func_8002D780/D320(...)`) — FAKE-annotated; (2) three
per-axis locals `x`, `y`, `z` for the probe-position reads (one written-once local per axis instead of one
`c` local assigned three times); (3) `extern u8 D_800F5F68[];` declared as a byte array and the record base
spelled `&D_800F5F68[id * 0x1B8]`; (4) forward `extern` prototypes for func_8002D320 / func_8002D780;
(5) integer-offset scratchpad reads `*(s32 *)((u8 *)0x1F8000A8 + off)` and the `&SCR[id].j[i + 4]` call
argument; (6) the record pointer `rec` walked as a for-increment induction variable alongside `i`.

## T1 semantic purpose
(1) The reject flag is a REAL value with an observable effect: it is read exactly once by the `continue`
test and decides whether the callee runs. What has no semantic purpose is the CHOICE of which local carries
it — a separate `rejected` local produces the same behavior and scores 5 (rejected/separate-rejected-var-
takes-a1-seat-5.c). That choice is the FAKE construct and it carries the annotation.
(2) Semantic — each holds the probe coordinate its own AABB test reads twice (`x - r`, `x + r`). The
single-`c` spelling is behaviorally identical (rejected/one-status-var-but-multiset-c-load-2.c, score 2); a
per-axis local is the plainer of the two spellings, not a device.
(3) Semantic — D_800F5F68 IS a per-character 0x1B8-byte record blob indexed by character id (census
g_per_char_record_440); a byte array indexed by `id * 0x1B8` states that truthfully. It replaces the
`(u8 *)&D_800F5F68 + id * 0x1B8` scalar-address pun flagged by the dispatch DECLARATION PUNS scan.
Byte-neutral: 0 both ways (v10a vs v11a).
(4) Semantic — the callees need prototypes; the D320 one is redundant with its definition earlier in
src/code6cac_b.c and can be dropped without effect.
(5) Semantic — they are the actual probe-position reads and the actual pointer passed to the callee.
(6) Semantic — it is the record cursor; every iteration reads five fields through it.

## T2 human-programmer
(1) Partly. `flag = 0; if (outside) flag = 1; if (flag) continue; flag = test(...)` is a common 1990s
scratch-flag idiom, but with the name `hit` a reader would ask why a rejected record sets `hit = 1`. That
question is exactly why the construct is annotated under the variable-reuse family rather than passed off
as plain style.
(2) Yes — three named coordinates is what one writes for a three-axis box test.
(3) Yes — a byte array indexed by a record stride is the honest declaration of an untyped record blob.
(4)(5)(6) Yes — ordinary C.

## T3 GCC-internals justification
(1) The MECHANISM of the seat is GCC's: global.c find_reg pass 0 seats a pseudo with zero calls crossed in
the lowest-numbered free hard reg already in `regs_used_so_far` (global.c:972, :1000, loop at :1058-1076),
which is $a1; the target's flag lives in $s0, the callee-saved seat that only the call-crossing `hit`
pseudo can own. The program logic does not explain the reuse; the annotation names the pass. This is the
T3 signal the family entry exists to license, with its prerequisites (T5) met.
(2) The per-axis locals were chosen after reading sched.c: `birthing_insn_p` (reg_n_sets == 1) bumps a
single-set load to LAUNCH_PRIORITY and a three-times-set `c` never gets the bump, which is why the lhu
landed after the x-load. But the construct has a truthful semantic reading (three coordinates) and, per
.claude/rules/ordinary-c-judge-decidable.md ruling 3, a semantically truthful respelling chosen after
observing the scheduler is not a FAIL ground. No annotation claimed.
(3)-(6) None — ordinary C; the s1 mechanism notes on the call-arg fold explain why the straightforward
spelling matches, they are not a device.

## T4 permuter/search provenance
No permuter or automated search was run in s1 or s2. Every form was hand-written from the target asm and
the cc1 dumps (tmp/grind/func_8002CA8C/dumps/, s2/sched_v9c_fn.txt) and measured individually
(tmp/grind/func_8002CA8C/s2/diff_*.txt).

## T5 family check
(1) Variable reuse for codegen control — the SOTN-accepted list entry at
.claude/rules/no-new-park-categories.md:185 ("reusing one C variable for two unrelated values to influence
loop-invariant detection or RA"), with the borrow gated by .claude/rules/staged-value-reused-variable.md
bounds 1-6: (b1) the staged value is real and read by the next statement's `continue` test; (b2) `hit`
exists for a real job (the callee result, read by the seen-mask tail); (b3) liveness — the previous
iteration's `hit` is dead at `hit = 0` (its last read is the seen-mask tail of the prior iteration), and the
reject value is dead after the `continue` test, before the callee overwrites it; (b4) annotated in the
mandated what/mechanism/lever-exhaustion form; (b5) last resort — separate-flag spellings v8a (5), v9a
(13), v9b (5) measured, and the seat proven unreachable for any separate non-call-crossing flag pseudo
(hypotheses.md s2-H2, class kill with global.c predicate); (b6) nothing else in the catalog is touched.
Not the LICM-rule mechanism, but the family entry itself names RA as an in-scope effect and SOTN PSX
master ships an RA-purposed reuse with the same comment (docs/reference/sotn-construct-index.md:113,
src/weapon/w_037.c:300 "FAKE but makes register allocation work").
(2)-(6) Not in any forbidden family; not a named-intermediate claim (each of x/y/z is a plain local with
a semantic reading, no FAKE needed under ordinary-c ruling 3), not an aggregate merge (no struct is
declared; the byte array asserts only the stride the target computes).

## T6 naming-announces-intent
No `pad`, `dummy`, `unused`, `spill`, `tmp`, `slack` or similar. `hit`, `x`, `y`, `z`, `r`, `rec`,
`recbase`, `base`, `off`, `hitMask`, `seenMask`, `id`, `scr`, `i` all name what they hold.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: variable-reuse (Variable reuse for codegen control), borrow bounds per staged-value-reused-variable
  SCOPE: "When GCC hoists a loop-invariant (e.g. limit-1) that the target recomputes INLINE, reuse one C variable for a used loop-variant AND the invariant — multi-set pseudo isn't a loop.c movable, so it's not hoisted. Pure C, no asm."
  SCOPE: "SANCTIONED 2026-07-03 — a real, immediately-used value staged through an existing (currently-dead) local to fix instruction order; FAKE-annotated, lever-exhaustion required; zero dead code"
  PRECEDENT: .claude/rules/no-new-park-categories.md:185
  PRECEDENT: docs/reference/sotn-construct-index.md:113
  PRECEDENT: .claude/rules/staged-value-reused-variable.md:58

ANNOTATION-CONFORMANCE: one FAKE block, present THIS session in BOTH memory/grind/func_8002CA8C/candidate.c and src/code6cac_b.c (verified by `grep -n "FAKE:" src/code6cac_b.c` after the splice; the Judge's 21:32 defect was its absence from src), immediately above `hit = 0;`, spelled verbatim as:
  /* FAKE: the AABB reject flag is staged through the existing `hit`
   * status local (hit = 1 on reject, read once by the `continue` test
   * below, then overwritten by the callee result), mechanism: global.c
   * find_reg pass 0 - a separate non-call-crossing flag pseudo takes
   * the lowest free already-used caller-saved reg ($a1), while the
   * target seats it in $s0 = the call-crossing `hit` pseudo,
   * lever-exhaustion: memory/grind/func_8002CA8C/hypotheses.md s1-H5..s2-H3 */
  It carries what (the reject flag staged through `hit`), mechanism (global.c find_reg pass 0, lines :972/:1000/:1058-1076 re-verified this session), and lever-exhaustion (hypotheses.md s1-H5..s2-H3; the s2 rows were banked into hypotheses.md THIS session from the measured diffs and rejected/ forms, s2-H2 being the class kill with predicate global.c:972). Comments are byte-neutral: sandbox 0/179 with the annotated body in src.
