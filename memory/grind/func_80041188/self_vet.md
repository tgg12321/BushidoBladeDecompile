# SELF-VET — func_80041188

CONSTRUCTS: (1) `do { loop1: ... } while (0);` single-level wrap around loop1's
leading half, with the `loop1:` label inside the wrap and the back-`goto loop1`
entering from outside; (2) reuse of the (dead-after-loop1) `stptr` local as
loop2's output pointer, re-initialized `stptr = (s32)(((u8 *) pa4) + 0x20);`;
(3) staging of loop2's out-pointer through the (dead-after-loop1) `out2` local,
`out2 = (s32 *) stptr;` immediately before `func_800523E0(pa4, out2, ...)`.

## T1 semantic purpose
(1) The wrap has no observable effect on the function's output — the body
executes exactly once per entry either way; it is present purely as a codegen
device and is FAKE-annotated as such. It is the sanctioned `do{}while(0)`
family, which by owner ruling 2026-07-06 is allowed for ANY codegen effect
provided it is annotated. (2) Real: `stptr` carries the loop2 output pointer
and is read by `func_8004A348(buf, (s32 *) stptr)` every iteration; the only
question the construct answers is WHICH local carries it, and reusing an
existing one is ordinary C. (3) Real: the assigned value is consumed by the
very next statement's argument list; the construct is one honest assignment
split out of a cast-in-the-argument, with the destination being an existing
local rather than a fresh one. No construct in the diff is dead, unused, or
write-only; there is no `(void)`, no address-of-a-dead-object, no volatile,
no pin, no `__asm__`, no declaration-only object.

## T2 human-programmer
(2) and (3) read as ordinary C to someone who has never seen the target bytes:
a function with two phases that reuses its two scratch pointers across both
phases, and an explicit local for a call argument. A reader would not ask "why
is this here?" of either. (1) is the one construct a reader WOULD ask about,
which is exactly why the project requires it to be FAKE-annotated in place
rather than silently merged; it is annotated, greppable, and the annotation
states the measured effect (9 -> 0). It is also the canonical PsyQ-era macro
body idiom, so its presence in original 1998 source would be unremarkable.

## T3 GCC-internals justification
Yes for all three, and the annotations say so explicitly (the family rules
REQUIRE a named mechanism in the annotation, so naming one here is compliance,
not a smell): (1) the loop note the wrap emits changes flow.c's loop-depth ref
weighting, which feeds global.c allocno priority and reseats the callee-saved
set (out2 s6 / carrier s7 / a3 fp); (2) global.c allocno priority — the merged
multi-ref `stptr` pseudo (pri 3409, banked in evidence.md s3) allocates 3rd and
lands s3, where a fresh low-ref local loses the ordering race; (3) flow.c's
loop-weighted ref count for `out2`, lifting its global.c priority above the
pa4-carrier's. This is disclosure of the mechanism inside sanctioned families,
which each require it — none of the three is an attempt to impose bytes or a
register outcome from OUTSIDE compiled C (no regfix, no pin, no asm, no
volatile, no cross-symbol address derivation, no semantic-lie C). Every byte
comes from the pristine compiler consuming legal C, and the committed source is
a true compilable origin of those bytes.

## T4 permuter/search provenance
Constructs (1) and (3) were PROPOSED by this session's permuter campaign
(tmp/grind/func_80041188/s4/perm2, outputs output-50-1 and output-63-1) on the
pa4-read chassis. They are not accepted because the permuter found them: each
was re-measured by hand in the honest sandbox (15 -> 9 -> 0), each was matched
to a pre-existing sanctioned family whose rule file was read end-to-end, and
each survives that family's stated bounds (below). The permuter also produced
two proposals that were REJECTED here rather than submitted: a
loop-invariant hoist of `stptr2 + 6` out of loop2 (semantically WRONG — it
would write the same address every iteration) and a `(new_var = i) < 0x12`
dead-store, neither of which is in the diff. Construct (2) is inherited from
s3's hand-derived floor-1 form, not from search.

## T5 family check
(1) `do { ... } while (0);` — sanctioned family, `.claude/rules/
do-while-zero-exception.md`. Single level, so prerequisite 3 (nested-wrap
justification) does not apply; prerequisite 1 (inline FAKE naming the observed
effect) is satisfied; prerequisite 2 (prefer natural geometry) is satisfied
with receipts — three prior sessions of natural statement-placement,
declaration-order, split-init and re-init spellings are banked in
hypotheses.md/evidence.md and the split-RA axis was proven dead end-to-end in
s3. (2) and (3) are the variable-reuse / staged-value family,
`.claude/rules/staged-value-reused-variable.md`, whose six bounds all hold:
the values are real and consumed (bound 1); both locals already exist for real
jobs in loop1 (bound 2 — nothing was INVENTED to be borrowed); both borrows are
provably safe, the loop1 values being dead at the staging point and the staged
values not needed after (bound 3, liveness stated in each annotation); both are
FAKE-annotated with what/mechanism/lever-exhaustion (bound 4); both are last
resort with receipts in the ledger (bound 5). No construct matches any entry in
the forbidden-family catalog: not a register pin, not hardcoded-$N asm, not a
regfix/asmfix rule, not a scheduling barrier, not volatile coercion in any
spelling, not an alias rename, not a dead local/array/pad, not a dead-param
assign, not an empty-body `if`, not an `if (1)` wrapper, not a dead goto label,
not a DImode chain, not an opaque constant holder, not a redundant width cast,
not a linker-script reorder.

## T6 naming-announces-intent
No name in the diff announces coercion intent. The three identifiers involved
are `stptr`, `out2` and `loop1`, all pre-existing and all named for their real
program roles (a walking structure pointer, the second output buffer, the first
loop's head). There is no `pad`, `dummy`, `unused`, `spill`, `tmp`, `slack`,
`new_var` or `_buf` anywhere — the permuter's own `new_var`/`new_var2` names
were not carried into the diff. Every one of these identifiers has real reads.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: do-while(0) wrap
  SCOPE: "**`do { <any body> } while (0);` — including empty bodies — is a sanctioned pure-C match device for ANY codegen effect, including register allocation.**"
  PRECEDENT: .claude/rules/do-while-zero-exception.md:99
  FAMILY: staged value through a reused variable (variable reuse for codegen control)
  SCOPE: "Sometimes the only way to make our compiler produce the exact same machine code as the original game is to write one line of C as two"
  PRECEDENT: .claude/rules/staged-value-reused-variable.md:46
  FAMILY: do-while(0) wrap — SOTN-master corroboration
  SCOPE: "`do { ... } while (0)` wraps"
  PRECEDENT: docs/reference/sotn-construct-index.md:582

ANNOTATION-CONFORMANCE:
  /* FAKE: do-while(0) wrap of loop1's leading half (the `loop1:` label sits
     inside it; the back-goto enters from outside), mechanism: the loop note
     the wrap emits re-weights flow.c's ref counts feeding global.c allocno
     priority, which seats the callee-saved set exactly as target (out2 s6,
     pa4-carrier s7, a3 fp); measured effect: identical form without the wrap
     scores 9, with it 0. lever-exhaustion: memory/grind/func_80041188/
     hypotheses.md + evidence.md (s1-s4 lever ladder; s4 permuter campaign
     tmp/grind/func_80041188/s4/perm2/output-50-1) */
  /* FAKE: loop2's output pointer reuses the (dead-after-loop1) `stptr`
     walker local instead of a fresh local, mechanism: GCC 2.7.2 global.c
     gives the merged multi-ref pseudo priority 3409 so it allocates 3rd and
     lands s3 == target (a fresh low-ref local loses the s3 ordering race).
     Liveness: stptr's loop1 value is dead here - it is overwritten before any
     later read. lever-exhaustion: memory/grind/func_80041188/evidence.md s3 */
  /* FAKE: the loop2 out-pointer is staged through the (dead-after-loop1)
     `out2` local, whose value is real and consumed by the very next call,
     mechanism: GCC 2.7.2 flow.c counts the extra loop-weighted ref so out2's
     global.c allocno priority clears the pa4-carrier's and out2 keeps s6 ==
     target. Liveness: out2's loop1 value is dead here; the staged value is
     not needed after. lever-exhaustion: memory/grind/func_80041188/
     evidence.md s3-s4 (pa4-read re-init alone scores 15, +this ref 9,
     +the loop1 wrap 0) */
All three carry what + mechanism + lever-exhaustion. Each construct in the
diff is annotated; there is no un-annotated FAKE-family construct.
