# SELF-VET — func_80047A90

CONSTRUCTS: merged-counter-variable-i (one counter reused across both loops), goto-form-loops (loop1 + loop2 + inner), hand-hoisted-jb-pointer (s16 *jb = Judge), do-while(0)-wrap-on-a3-init, do-while(0)-wrap-on-pa2-init, do-while(0)-wrap-on-pt3-init, duplicated-inner-tail-into-if-arms

## T1 semantic purpose
- merged counter i: full semantic purpose — it IS the iteration counter of both loops (loop 1 counts 0..16, loop 2 counts rows 1..8 and drives the i==8 fade-curve store). Every ref is byte-materialized in target (addu t0,zero,zero / addiu t0,t0,1 / slti / addiu t0,zero,1 / li v0,8+bne / slti t0,9). Target itself uses ONE register ($t0) for both counters.
- goto-form loops: semantic purpose = the loop control flow itself; every branch/test is a target byte.
- jb pointer: semantic purpose = the Judge table base used by the lhu index read (target's $a2 lui/addiu pair).
- the three wraps: NO semantic purpose beyond executing their single body statement once — purely-for-matching constructs, FAKE-annotated, claimed under the sanctioned do-while-zero family below.
- duplicated tail: the statements are REAL (pa1/pa2/a3 advances execute on every inner iteration); the DUPLICATION itself has no extra semantic purpose — claimed under the sanctioned duplicated-statement-into-arms family below.

## T2 human-programmer
- merged counter, goto-form loops, jb: yes — a 1998 C programmer writing a two-phase table update with a reused counter, explicit gotos and a hoisted base pointer is ordinary PsyQ-era code (the goto-form loop 2 was already established as byte-required in s1: no loop notes).
- wraps: no — a reader would ask why; that is exactly why the family sanction + inline FAKE annotation exist (SOTN ships the same construct with the same annotations, incl. one-line-store bodies do { var = 0; } while (0); — w_045.c).
- duplicated tail: no as a first-choice spelling; sanctioned family with FAKE annotation (SOTN 7-arm/11-arm precedents).

## T3 GCC-internals justification
- merged counter/goto-form/jb: justified by program logic first (they compute the function's behavior); their register OUTCOME was derived via GCC forensics but the constructs assert only true program facts.
- wraps + duplication: yes, their justification IS a GCC mechanism (flow.c loop_depth ref weighting / reg_n_refs + jump2 cross-jump). Both are inside sanctioned families whose rulings explicitly cover register-allocation effects (do-while-zero-exception: "ANY codegen effect, including register allocation"; duplicated-statement-into-arms: "incl. when ... the effect is a reg_n_refs priority lift").

## T4 permuter/search provenance
None of the constructs came from auto-search. The closing geometry was derived this session from FINDREGDBG/ALLOCDBG traces of the instrumented cc1 (the s4/s5 permuter campaigns, ~182k iterations, never found it). The dup-arms construct was derived and byte-verified in s2.

## T5 family check
- merged counter: matches SOTN-accepted "variable reuse for codegen control" (frozen list); also plainly natural code.
- wraps ×3: family do-while-zero-exception — all three SINGLE-LEVEL (no nesting anywhere), each with inline FAKE annotation at the construct site. Multiple independent single-level wraps in one function have direct project precedent (marionation_Exec, four wraps, the case that prompted the 2026-07-06 ruling).
- duplicated tail: family duplicated-statement-into-arms; byte-neutrality VERIFIED (cross-jump re-merges to the single shared tail; 84/84 instruction-for-instruction target match including registers).
- No forbidden family touched: zero pins, zero __asm__, zero volatile, zero dead stores/reads, zero unused locals, zero alias renames, zero regfix/asmfix.

## T6 naming-announces-intent
Names are role-descriptive program names (i, a3, v1, a0, jb, p558, p59C, pt1, pt2, pt3, pa1, pa2, temp — inherited ledger naming mirroring target registers/symbols). No pad/dummy/unused/spill/slack names. No name announces coercion.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: do-while-zero-exception
  SCOPE: "SANCTIONED (owner ruling 2026-07-06, supersedes the 2026-06-04 mechanism-scoping): `do { ... } while (0);` (any body, incl. empty) is an allowed pure-C match device for ANY codegen effect incl. register allocation, with mandatory inline FAKE annotation; nested wraps need a single-level-insufficient justification."
  PRECEDENT: cf3e6ce7

  FAMILY: duplicated-statement-into-arms
  SCOPE: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): duplicating a REAL statement into 2+ arms (instead of label-sharing) is legitimate — incl. when cross-jump re-merges the copies to identical bytes and the effect is a reg_n_refs priority lift. SOTN duplicates assignments across arms routinely (7-arm, 11-arm instances). Prerequisites: byte-neutrality verified, lever-exhaustion, FAKE annotation when match-motivated."
  PRECEDENT: .claude/rules/duplicated-statement-into-arms.md:5

  FAMILY: variable reuse for codegen control
  SCOPE: "Variable reuse for codegen control ([[defeat-licm-hoist-var-reuse]]): reusing one C variable for two unrelated values to influence loop-invariant detection or RA."
  PRECEDENT: .claude/rules/no-new-park-categories.md:214

ANNOTATION-CONFORMANCE: four inline FAKE annotations present at the construct sites in src/sound.c:
  /* FAKE: loop-note ref weighting lifts a3's allocno priority above the shared counter i, seating a3 in $a3 and i in $t0 */
  /* FAKE: loop-note ref weighting lifts pa2 above the shared counter i, seating pa2 in $a2 (shared with the loop-1 Judge base) */
  /* FAKE: loop-note ref weighting keeps pt1 ahead of pt2 in allocation order (pt1->$t1, pt2->$t2) after the pa2 wrap's weighted pt2 use lifted pt2 */
  /* FAKE: loop tail duplicated into both arms (cross-jump re-merges, byte-neutral); reg_n_refs lift lands pa2->$a2, a3->$a3 */
Each names the observed effect (effect-level, per the rule's prerequisite 1). Lever-exhaustion context: single-level wraps carry no exhaustion gate under the 2026-07-06 final ruling; the dup-arms exhaustion is the s1–s5 ledger (structural, decl-order, type, ref-weight and ~182k permuter iterations all measured dead against the priority wall it lifts).
