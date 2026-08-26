# SELF-VET — func_80061658 (grind session s5, 2026-08-25)

Diff under vet: `src/text1b.c` line 3328 — the `INCLUDE_ASM("asm/funcs", func_80061658);`
line replaced by the C body saved at `memory/grind/func_80061658/candidate.c`.
Nothing else in the tree is touched (no rules, no headers, no build files).

Measurements backing this vet (all THIS session, engine on main):
  - `sandbox func_80061658 --disable all` = **0** (target_insns 46, build_insns 46,
    rules_dropped 0, zero pins / zero `__asm__` in this function)
  - full-tree `verify-oracle` = `62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle
  - `canonical func_80061658` verdict = **C**

CONSTRUCTS: (1) `s32 *v1 = (s32 *)&D_800F116C;` — C-level pointer alias to a global
[FAKE-annotated]. (2) `s32 val;` carrying the switch-arm constant to the shared tail
[ordinary C, load-bearing, not a coercion]. (3) `p = arg0; D_800F1140 = *p++; ...`
walking-pointer element stores [ordinary C, the cluster idiom]. (4) `default: goto done;`
switch exit past the shared tail [ordinary C, mirrors the target's `j .L800616C4`].

## T1 semantic purpose
- (1) alias `v1`: no semantic effect vs `D_800F116C` used directly — this is the
  redundant-second-handle shape, and it is exactly why it needs the FAKE annotation
  and the sanctioned-family claim below. Measured effect: without it the build is
  47 insns / distance 5 (rejected/noalias-direct-global-5.c).
- (2) `val`: YES, real semantic purpose. The two switch arms select different code
  words (0x21000C / 0x21000D) and the store `*v1 = val;` lives in the tail SHARED by
  both arms (target: both arms `j .L800616B4`). A shared tail cannot store a constant
  that was materialized inside an arm, so a variable carrying the arm's value across
  the join is required by the control-flow shape, not by codegen. Inlining the
  constants into each arm is a different program shape and measures 12
  (rejected/walkptr-inlined-switch-constants-12.c).
- (3) walking pointer: YES. `p` genuinely walks arg0[0..2]; every increment is
  consumed by the next dereference. Nothing is dead.
- (4) `default: goto done;`: YES. It is the function's real behaviour — for
  arg1 not in {0,1} nothing is written to D_800F1180 / D_800F116C and control
  proceeds to the call. It reproduces target's default edge.

## T2 human-programmer
Yes for every construct except (1). (2)/(3)/(4) are the literal shape of five
functions already sitting COMPLETED-C in this same file, written by this project's
own prior sessions and accepted: func_800611A4 (src/text1b.c:3205),
func_8006133C (:3232), func_800613C8 (:3247), func_80061710 (:3365),
func_800617C8 (:3403). func_80061710 is a structural TWIN (same switch, same
`default: goto done;`, same shared tail, same walking-pointer tail, different
constants) and was committed COMPLETED-C in 28f49ad0. A reader asks "why is this
here?" about NONE of (2)/(3)/(4). Construct (1) is the one a reader would ask about,
which is why it is FAKE-annotated and claimed under a sanctioned family.

## T3 GCC-internals justification
The program logic IS the explanation for (2)/(3)/(4): they are the shape the
function's own control flow and its five sibling functions dictate. GCC internals are
cited in the ledger only to EXPLAIN why the earlier hand-written `t = arg0[i]` shape
missed (local-alloc.c:472's `reg_n_deaths == 1` gate deferred the shared 3-death load
temp to global_alloc and let the 1-death mask constant take $v0) — the fix was not
designed around that gate, it was adopted from the sibling idiom and the gate
explains it after the fact. Construct (1) does cite a GCC pass by design
(address-materialization caching / base-register allocation) — that naming is
REQUIRED by prerequisite 2 of the pointer-alias rule, and is the same annotation the
accepted sibling carries.

## T4 permuter/search provenance
No permuter run produced this form and none was run this session. The form was
derived by reading the five COMPLETED-C siblings in src/text1b.c and cloning the
structural twin func_80061710. The s4 permuter's score-0 output (constant staged
through a reused local) is REJECTED and NOT in this diff; it is banked at
rejected/single-var-reuse-for-mask-BANNED-FAMILY.c together with the s5
single-variable variant of the same banned family, both explicitly not submitted.

## T5 family check
- (1) matches the sanctioned pointer-alias family exactly (claim block below).
- (2) is NOT the "named-intermediate" coercion family and NOT dead-store: `val` is
  written once per arm and read once in the shared tail, and its value is required
  there by control flow, so it is ordinary C rather than an inserted intermediate.
- (3) is the walking-pointer idiom documented at
  `.claude/rules/walking-pointer-serializes-parallel-loads.md` (metadata type:
  reference — ordinary C, no FAKE mandated).
- (4) is the mixed-exit / shared-tail shape covered by
  `.claude/rules/cross-jump-store-tail-merge.md` (ordinary C, no FAKE mandated).
- Nothing in the diff matches any entry in the forbidden-family catalog: no register
  pin, no `__asm__`, no volatile, no dead store, no self-assign, no unused local, no
  constant holder, no alias rename, no `do {} while (0)`, no reuse of a live local
  for a second unrelated value.

## T6 naming-announces-intent
Names are `v1`, `p`, `q`, `val` — all splat/idiomatic decomp names carried over from
the accepted siblings (func_80061710 uses `v1`, `p`, `q`, `val` for the identical
roles). None is `pad`/`dummy`/`unused`/`spill`/`slack`/`tail`. Every one of them is
read on a live path; none exists only to be discarded, address-taken, or declared.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: C-level pointer alias to a global (pointer-alias-fake-exception)
  SCOPE: "a C-level local pointer alias (second handle) to a global — `Type* t = &g_Thing;`, `s16 (*p)[] = &D_xxx;`, FakePrim-style reinterpret views — is allowed as a LAST-RESORT lever with `/* FAKE: ... */` annotation + lever-exhaustion. SOTN idiom incl. self-admitted `// n.b.! unused, required for PSP`. asm(\"sym\") alias-RENAMES stay forbidden."
  PRECEDENT: src/text1b.c:3366
  (prereq 1 lever-exhaustion: the direct-global no-alias form was measured FIRST
   this session at distance 5 / 47 insns — rejected/noalias-direct-global-5.c —
   on top of the s1-s3 structural sweep and the s4/s4b permuter campaigns;
   prereq 2 GCC pass named in the annotation; prereq 3 annotation present.)

ANNOTATION-CONFORMANCE:
  /* FAKE: local pointer alias to D_800F116C, mechanism: base-register
   * allocation / address-materialization caching in local-alloc (the alias
   * gives GCC one pseudo holding &D_800F116C, kept live in $a0 across the
   * switch instead of being re-materialized per use), lever-exhaustion:
   * memory/grind/func_80061658/hypotheses.md (s1-s3 structural + s4/s4b
   * permuter all measured dead on the direct-global form; identical alias
   * carried by the COMPLETED-C sibling func_80061710, src/text1b.c:3366). */
  Carries all three: WHAT (local pointer alias to D_800F116C), MECHANISM (named GCC
  pass — base-register allocation / address-materialization caching in local-alloc),
  LEVER-EXHAUSTION (ledger pointer + the measured direct-global form at distance 5).
