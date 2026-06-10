# func_8001EEB4 WIP — score 6 → 3 via [[hoist-call-arg-local-flips-jal-delay]]

## TL;DR

`cpu_get_move_pattern_table_number` (kengo:HIGH). Entry checker that
gates a 3-call sequence (`func_800218C8` → `func_80021A3C` →
`func_80021A98`) on 4 conditions reading `entry+0x6A/0x72/0x96`.

HEAD floor: sandbox `--disable all` = 6. Candidate floor: 3.

## How to resume

1. Apply `candidate.c` to `src/code6cac.c` line 1901–1920 (function body).
2. Confirm: `& tools/eng.ps1 sandbox func_8001EEB4 --disable all` reports
   score 3, build_insns 58, target 59.
3. Continue from there. Three diffs remain in the comparison block (see
   `meta.json#remaining_gap`).

## What this session did

Applied [[hoist-call-arg-local-flips-jal-delay]] to the second call
(`func_80021A98`). Adding `s32 idx2 = D_800A3748;` as the FIRST stmt
in the inner block before `*(entry+0x5E)=1` gives `idx2`'s RTL pseudo
a low LUID, scheduling its `lb $a0,...` arg-setup EARLIER. cc1's
`reorg` then picks the store `sh $s0, 94($s1)` as the natural delay-
slot fill — exactly target's pattern. This retires the
`reorder 36,38,40,39,37,41 @ 36-41` regfix rule and 3 score points.

Critical detail: use **`s32`** for `idx2`, not `s8`. Despite
`D_800A3748` being `s8`, declaring `idx2` as `s8` triggers
`lbu + sll 0x18 + sra 0x18` expansion (+2 insns regression).
`s32 idx2 = D_800A3748;` keeps it as a single `lb` instruction.

## Session-2 (2026-06-07, HEAD 0d960614) — andi materialization lever found

`s32 a1_s = (s16)a1;` declared as a separate top-level local AND referenced in
the `(u32)(a1_s - 0x17) >= 2` arm of the && chain DOES trigger target's andi
emission. Confirmed via objdump:

- ✓ `andi $v1, $a1, 0xFFFF` materializes at idx 18 (matching target's
  exact position and operands)
- ✓ `addiu $v0, $a1, -23` in bnez delay slot uses `$a1` (matches target;
  was `$v1` before the lever)
- ✓ `lhu` lands in `$a1` register (matches target; was `$v1` before)
- ✓ build_insns 58 -> **59** = target's exact count

Two coupled regressions cost net +6 score (3 -> 9):

- ✗ load at idx 16 is `lh $a1, 106($s1)` (sign-extend) — target has
  `lhu` (zero-extend). 1 byte opcode diff.
- ✗ stack frame inflates by 8 bytes (addiu sp,sp,-40 vs target -32;
  three saved-reg sw offsets shift +8). The extra local needs a spill
  slot apparently.

Mechanism (hypothesis): the (s16) cast on a u16-loaded value forces GCC
combine to emit a SImode sign-extension RTL pattern. That RTL pattern
includes the `andi 0xFFFF` extraction the target wants. But combine
ALSO folds the sign-extension INTO the load itself (lhu -> lh).
Decoupling the andi-emission from the load-opcode change is the
remaining puzzle.

### Resume target (next session)

PRIMARY: find a C form that gets the (s16)-cast-on-local effect (andi
emission, register pattern) WITHOUT changing lhu -> lh and WITHOUT
inflating the stack. Try block-local declaration; try (s32)(s16)a1
vs (s16)(s32)a1 ordering; try reading a1 separately via different
type pointers in two distinct syntactic positions.

## Session-5 (2026-06-10, HEAD 43b3ae26) — 4 new variants tested, none lowered floor

Re-confirmed candidate floor: score=3, build_insns=58, target=59. Tested 4
new variants beyond the prior rejected_forms catalog:

1. Pointer alias intermediate (`u16 *p_a1 = ...; u16 a1 = *p_a1;`) — score 3
   unchanged. Combine folds indirection.
2. Reordered && chain (subtract check first) — score 9 regression. Build_insns
   matches target (59) but registers still wrong, no andi.
3. Explicit u32 widening at load (`u32 a1 = (u32)*(u16 *)...`) — score 3
   unchanged. Same as session-4 V1/V2.
4. Dual-read with `(s32)*(s16 *)(entry+0x6A) - 0x17` precomputed — score 15
   regression. Two loads emitted (lhu + lh) since combine treats different
   ptr-type reads as distinct CSE classes.

All 4 added to `rejected_forms` in meta.json. Floor remains 3. Resume avenues
from session 4 still untried (instrumented combine.c probe, directed permuter,
sibling back-port).

## Session-10 (2026-06-10, HEAD 2406e680) — 3 NEW structural variants tested, floor stays 3

Re-verified candidate.c floor at fresh HEAD 2406e680: score=3, build_insns=58,
target=59 — matches sessions 1+3..9. Tested 3 NEW structural variants beyond
the now-27-form rejected_forms catalog:

1. **V1 — early-return-on-fail form** — replace the && chain with sequential
   `if (cond) goto cleanup;` statements (a1==0xA, 0x72!=0, subtract<2,
   0x96!=0), single shared `cleanup:` label at end (game_Cleanup + globals
   are genuinely shared) — score 3 unchanged. GCC tree-ssa branch-merge
   collapses this form back to the same && short-circuit RTL before
   flow1/combine. Identical to the rejected nested-if form (session 6) —
   distinct syntactic spelling, same RTL the andi-emission decision sees.
   NOT the forbidden goto-end-prologue-delay-slot pattern (the cleanup
   work is real shared logic, not a return-value accumulator).

2. **V2 — 0x72-first && chain reorder** — putting
   `*(s16 *)(entry + 0x72) == 0` first, then `a1 != 0xA`, then subtract,
   then `*(s16 *)(entry + 0x96) == 0` — score 9 REGRESSION, build_insns 57.
   The lh load of entry+0x72 hoists eagerly into prologue, removing target's
   interleaved load+test scheduling. Same regression family as session-8 V-C
   (hoist all entry-fields) and session-5 V2 (subtract-first).

3. **V3 — call-arg hoist for func_80021A3C** — `s32 a_arg = *(s16 *)(entry +
   0xA);` declared FIRST in the inner block (parallel to idx2 hoist, applied
   to func_80021A3C's second arg) — score 3 unchanged. The a_arg hoist
   doesn't disrupt the comparison block's RTL coupling — a1's load and the
   && chain compare site are upstream of the inner block. The hoist's lever
   effect (per [[hoist-call-arg-local-flips-jal-delay]]) is localized to the
   inner block's delay-slot fill decisions, which were already correct under
   the idx2 hoist; no additional codegen consequence.

All 3 measured variants added to `rejected_forms` in meta.json (catalog now
30 forms across 10 sessions). 10 sessions of cumulative surface-syntactic +
structural enumeration continue to reinforce the structural finding: the
3-diff cluster's coupling to the (s16)-cast-on-local mechanism is robust to
ALL tested rephrasings.

### Resume avenues unchanged from sessions 5-9

- (a) **Instrumented cc1 combine.c probe** — `tools/gcc-2.7.2/combine.c`
  holds the simplify rule folding `(and:SI (lhu:HI) 0xFFFF) → (lhu:HI)`.
  Identify a SImode-pseudo shape that escapes the fold without affecting
  the load opcode. The tmp/gccdbg/cc1 from saEft00Add session has reorg.c
  hooks but needs combine.c DBG hooks added + rebuild.
- (b) **PERM_*-directed permuter from candidate.c** — workspace READY at
  `permuter/func_8001EEB4/`. Author `PERM_GENERAL`, `PERM_INT_TYPE`,
  `PERM_TYPECAST` macros constraining mutations to semantically-equivalent
  axes (random mode in session 9 surfaced only cheats so directed mode
  requires careful macro authoring).
- (c) **Cross-reference matched siblings** — none new at HEAD 2406e680
  (`func_8001A820` still INCOMPLETE empty body; `func_8001F938` still
  INCOMPLETE score 11; `calc_loc_mat_fw_80055B60` still parked).
- (d) **Wait for sibling resolution + back-port** — passive.

## Session-9 (2026-06-10, HEAD 848bf32d) — directed permuter SETUP executed (primary documented avenue across 8 prior sessions); random run produced only semantic-cheat candidates

Re-verified candidate.c floor at fresh HEAD 848bf32d: score=3, build_insns=58,
target=59 — matches all prior sessions. **Executed the PRIMARY documented
resume avenue across 8 prior sessions: SET UP the directed permuter
workspace at `permuter/func_8001EEB4/`.**

Setup artifacts now committed for future-session directed runs:
- `target.s` assembled from `asm/funcs/func_8001EEB4.s` + `prelude.inc` (with
  `.set gp=64` stripped per the r3000 variant).
- `target.o` has func_8001EEB4 at offset 0 (verified via objdump -t).
- `base.c` = the candidate body + self-contained externs.
- `compile.sh` = the cheat-free cc1 | prologue_fix | maspsx | as pipeline.

**Initial random-mode run (10 min, -j 4, --stop-on-zero):**
- Permuter base score: **110** (weighted = ~22 reg-diff × 5 + scheduling
  penalties, dominated by the documented 3-diff cluster + register cascade).
- **3 candidates** — all rejected:
  1. `(char)((s32)a1 - 0x17) >= 2` (permuter 100) — 8-bit truncation;
     semantically DIFFERENT (rejects different a1 values).
  2. `(((s32)a1) & 0xFFu) - 0x17 >= 2` + `(a1 = 1)` mid-expression
     (permuter 105) — 0xFF mask + assignment; both semantic changes.
  3. **`(u32)(unsigned short)((s32)a1 - 0x17) >= 2` (permuter 0,
     SANDBOX 0, build_insns 59 = target).** Tested against real sandbox
     under HEAD's idx2 hoist + this cast extension: score=0,
     build_insns=59=target, 4 rules dropped. **LOOKED legitimate.**
     Cheat-reviewer invoked per [[review-discipline-before-commit]] —
     **verdict: FAIL.** 6-test breakdown:
     - Test-1: the `(unsigned short)` cast between s32 subtract and u32
       widening has ZERO observable effect for u16-bounded a1 (verified
       for all a1 ∈ {0..0xFFFF} the boolean outcome is identical).
     - Test-2: no programmer writes truncate-then-widen sandwich
       `(u32)(unsigned short)(...)` instead of `(u32)(...)`.
     - Test-3: only honest mechanism — the cast introduces a
       `(truncate:HI ...)` RTL node combine.c cannot fold via
       `(and:SI (lhu:HI) 0xFFFF) → (lhu:HI)`, exposing the andi.
       Pure GCC-internals reasoning.
     - Test-4: permuter random-mode discovery.
     - Test-5: same narrow-type-cast-to-defeat-combine-fold family as
       rejected `(char)` cheat AND rejected `(s16)` cast — cheats by
       any spelling.
     Reverted src/ to HEAD. Form added to rejected_forms.

Per [[no-new-park-categories]] §"Auto-search tools": "When an auto-search
tool returns a 'closing form', you MUST vet it against the cheat catalog
BEFORE proposing it to the user." All 3 candidates fail vetting. Added
to `rejected_forms` (catalog now 27 forms across 9 sessions).

### Random-mode permuter limit + the path forward

The vanilla random permuter mutates types/casts/declarations stochastically
and finds codegen overlaps via **semantically-invalid** mutations. Without
`PERM_*` macros in `base.c` naming SEMANTICALLY-EQUIVALENT variation axes
(operand-order swaps within associative+commutative expressions are now
forbidden per [[or-tree-shape-shift]], but type-equivalent casts like
`u32`/`s32`/`u16`+widen are still valid axes), the random mode will keep
finding the same syntactic-coincidence cheats.

**The DIRECTED permuter run (genuinely-untried in 9 sessions) requires
authoring `PERM_*` macros enumerating valid semantic equivalents.** Left as
the resume avenue for session 10+; the workspace is preserved at
`permuter/func_8001EEB4/`.

### Resume avenues (updated for session 10+)

- (a) **Instrumented cc1 combine.c probe** — unchanged from sessions 5-8.
  The `tmp/gccdbg/cc1` from saEft00Add session has reorg.c hooks but needs
  combine.c DBG hooks added + rebuild. Identify the simplify rule folding
  `(and:SI (lhu:HI) 0xFFFF) → (lhu:HI)` and a SImode-pseudo shape that
  escapes the fold without affecting the load opcode.
- (b) **PERM_*-directed permuter from candidate.c** — workspace now READY at
  `permuter/func_8001EEB4/`. Next session: author `PERM_GENERAL`,
  `PERM_INT_TYPE`, `PERM_TYPECAST` macros in `base.c` for the candidate's
  cast-and-type axes, re-run with `--stop-on-zero`. Only LEGITIMATE
  semantically-equivalent mutations explored.
- (c) **Cross-reference matched siblings** — none new at HEAD 848bf32d
  (`func_8001A820` still INCOMPLETE empty body; `func_8001F938` still
  INCOMPLETE score 11; `calc_loc_mat_fw_80055B60` still parked).
- (d) **Wait for sibling resolution + back-port** — passive.

## Session-8 (2026-06-10, HEAD 7b3fd4f9) — 3 NEW (s32)-cast / function-scope-idx2 / hoist-fields variants, floor stays 3

Re-verified candidate.c floor at fresh HEAD 7b3fd4f9: score=3, build_insns=58,
target=59 — matches sessions 1+3+4+5+6+7. Tested 3 NEW variants beyond the
22-form rejected_forms catalog (1 fourth variant short-circuited from V-C's
regression direction):

1. **V-A — (s32) cast on first compare** — `if ((s32)a1 != 0xA && ...)` —
   score 3 unchanged. The (s32) cast is treated by combine as no-op widening
   (identical to rejected `(u32)a1 != 0xA`). Signed vs unsigned widening of
   an already-zero-extended HImode value canonicalizes to the same SImode
   pseudo. Distinct syntactic spelling, same RTL.

2. **V-B — function-scope idx2 declaration** — `s32 idx2;` at function-top,
   `idx2 = D_800A3748;` inside the inner block (separating decl from init) —
   score 3 unchanged. The [[hoist-call-arg-local-flips-jal-delay]] lever
   effect survives: ASSIGNMENT position inside the block is what matters for
   reorg's delay-slot fill decision, not DECLARATION position. Extended live
   range of idx2 doesn't pull into a1's comparison-block allocation.

3. **V-C — hoist all entry-field reads to function-top** —
   `s16 cond_b = *(s16*)(entry+0x72); s16 cond_c = *(s16*)(entry+0x96);`
   declared before the if, replacing the inline `*(s16*)(...)` reads in the
   && chain — score 10 REGRESSION, build_insns 56 (target 59, -3 short).
   The hoisted lh loads emit eagerly in the prologue area, removing target's
   interleaved load+test scheduling. The hoist strategy from
   [[hoist-flag-load-defeat-add-combine]] (sibling func_800484A0) doesn't
   apply here: that function had an intervening `arg0 += 4;` pointer advance
   creating a real RTL use; func_8001EEB4 has no advance, so hoisting just
   emits loads without breaking any combine fold.

V-D (hoist only one cond field) was planned but short-circuited from V-C's
regression direction — the && chain's embedded loads are themselves part of
target's scheduling pattern; removing any of them perturbs the comparison
block's RTL.

All 3 measured variants added to `rejected_forms` in meta.json (catalog now
25 forms). 8 sessions of cumulative surface-syntactic enumeration reinforce
the structural finding: the 3-diff cluster's coupling to the
(s16)-cast-on-local mechanism is robust to ALL tested rephrasings.

### Resume avenues unchanged from sessions 5-7

- (a) **Instrumented cc1 combine.c probe** — `tools/gcc-2.7.2/combine.c`
  holds the simplify rule folding `(and:SI (lhu:HI) 0xFFFF) → (lhu:HI)`.
  Identify a SImode-pseudo shape that escapes the fold without affecting
  the load opcode. The tmp/gccdbg/cc1 from saEft00Add has reorg.c hooks
  but needs combine.c DBG hooks added + rebuild.
- (b) **Directed permuter** — `PERM_GENERAL` / `PERM_INT_TYPE` /
  `PERM_TYPECAST` from candidate.c with `--stop-on-zero`. 8 sessions of
  manual search have now exhausted ~25 surface variants; the directed
  permuter surface (untouched on this function) may surface a SImode-shape
  construct manual enumeration misses.
- (c) **Cross-reference matched siblings** — none new at HEAD 7b3fd4f9.
- (d) **Wait for sibling resolution + back-port** — passive.

## Session-7 (2026-06-10, HEAD 5e973f53) — 3 NEW dual-pseudo / u32-storage / u32-truthy variants tested, floor stays 3

Re-verified candidate floor at fresh HEAD 5e973f53: score=3, build_insns=58,
target=59 — matches sessions 1+3+4+5+6. Tested 3 NEW variants beyond the now-22
form rejected_forms catalog (none duplicates of prior rejections):

1. **V-A — dual-pseudo widening** — `u16 a1 = *(u16*)X; s32 a1_w = a1;`
   with `(u32)(a1_w - 0x17) >= 2` in the && chain — score 3 unchanged.
   Hypothesis: two C-level locals (HImode + SImode) might force combine to
   maintain two distinct pseudos through to the comparison block, producing
   the andi as the SImode pseudo materialization. Empirically: combine treats
   `s32 a1_w = a1` as a zero-extension and folds it into the load; no separate
   SImode pseudo emerges. Identical RTL to the rejected_form `s32 a1 = *(u16*)…`
   (session 4 V1).
2. **V-B — u32 storage with cast-free subtract** — `u32 a1 = *(u16*)X;`
   with `(a1 - 0x17) >= 2` (no `(s32)` cast on the subtract operand) —
   score 8 REGRESSION. The canonical form's `(u32)((s32)cast - K) >= 2` is
   structurally load-bearing for the && chain's scheduling; eliminating the
   cast doesn't simplify, it perturbs.
3. **V-C — explicit (u32) cast on truthy compare** — `if ((u32)a1 - 0xA && …)`
   instead of `if (a1 != 0xA && …)` — score 8 REGRESSION. The (u32) cast is a
   no-op for combine, but the truthy `expr - K` form emits via different
   fold-const-prop paths than `expr != K`, producing the same regression as V-B.

All 3 added to `rejected_forms` in meta.json (catalog now 22 forms). The
strengthened empirical signal from session-7: even cast-elimination variants
(V-B, V-C) regress, confirming the canonical form's `(u32)((s32)a1 - 0x17) >= 2`
+ `a1 != 0xA` shape is doing real structural work — not just no-op widening
that combine folds. The 3-diff cluster remains structurally coupled to the
(s16)-cast-on-local mechanism session-2 identified.

### Resume avenues unchanged from session-6

- (a) **Instrumented cc1 combine.c probe** — `tools/gcc-2.7.2/combine.c`
  holds the simplify rule folding `(and:SI (lhu:HI) 0xFFFF) → (lhu:HI)`.
  Identify a SImode-pseudo shape that escapes the fold without affecting
  the load opcode. `tmp/gccdbg/cc1` exists (May 30 build, sched.c-hooked)
  but needs combine.c DBG hooks added + rebuild.
- (b) **Directed permuter** — `PERM_GENERAL` / `PERM_INT_TYPE` /
  `PERM_TYPECAST` from candidate.c with `--stop-on-zero`. 7 sessions of
  manual search have exhausted ~22 surface variants; the directed permuter
  surface (untouched on this function) may surface a SImode-shape construct
  manual enumeration misses.
- (c) **Cross-reference matched siblings** — none new since session 4.
- (d) **Wait for sibling resolution + back-port** — passive.

## Session-6 (2026-06-10, HEAD f15b6dfe) — 4 NEW mask/precompute/truthy/nested variants tested, floor stays 3

Re-verified candidate floor at fresh HEAD f15b6dfe: score=3, build_insns=58,
target=59 — matches sessions 1+3+4+5. Tested 4 NEW variants beyond the now-15
form rejected_forms catalog (none duplicates of prior rejections):

1. **Split load + explicit u32 mask** — `u16 a1_raw = *(u16*)(entry+0x6A);
   u32 a1 = (u32)a1_raw & 0xFFFFu;` — score 3 unchanged. Combine recognizes
   `(u32)(u16-load) & 0xFFFF` as the no-op cast it folds away; explicit
   `& 0xFFFFu` on an already-zero-extended HImode load adds no SImode pseudo.
2. **Precomputed subtract** — `s32 a1_sub = (s32)a1 - 0x17; ...
   (u32)a1_sub >= 2` — score 5 regression (build_insns 57). Hoists the
   subtract from the &&-chain block into the prologue; documented as
   regression in existing rejected_forms.
3. **Subtract-truthy form** — `if (a1 - 0xA && ...)` replaces `if (a1 != 0xA
   && ...)` — score 3 unchanged. GCC's tree-to-RTL canonicalizes
   `(expr - const)` truthiness to `(expr != const)`; the != and - forms share
   the same RTL post-canonicalization. No codegen change.
4. **Nested-if form** — split outer `if (a1 != 0xA && rest...)` into
   `if (a1 != 0xA) { if (rest...) {} }` — score 3 unchanged. GCC's tree-ssa
   branch-merge collapses nested-if back to && short-circuit RTL before
   flow1; the andi-emission decision sees the same RTL shape.

All 4 added to `rejected_forms` in meta.json (catalog now 19 forms). The
3-diff cluster (lhu→$v1 vs $a1; missing andi; addiu uses wrong reg) remains
structurally coupled to the (s16)-cast-on-local mechanism that session-2
identified — which costs lh-load + 8-byte stack inflation, exceeding the 3
diffs it would close.

The cheap-lever search space (no permuter, no instrumented compiler probe,
no novel structural construct) is empirically exhausted. Across 6 sessions
the floor is reproducibly 3 at the candidate.c form; every surface
syntactic rephrasing either preserves the 3-diff cluster or regresses.

### Resume avenues unchanged from session-5

- (a) **Instrumented cc1 combine.c probe** — `tools/gcc-2.7.2/combine.c`
  holds the simplify rule folding `(and:SI (lhu:HI) 0xFFFF) → (lhu:HI)`.
  Identify a SImode-pseudo shape that escapes the fold without affecting
  the load opcode. `tmp/gccdbg/cc1` exists (May 30 build, sched.c-hooked)
  but needs combine.c DBG hooks added + rebuild.
- (b) **Directed permuter** — `PERM_GENERAL` / `PERM_INT_TYPE` /
  `PERM_TYPECAST` from candidate.c with `--stop-on-zero`. Each session's
  manual search exhausts ~4 variants; directed permuter systematically
  explores the type/cast space and may surface a SImode construct manual
  enumeration misses.
- (c) **Cross-reference matched siblings** — none new since session 4.
  `func_8001A820` still INCOMPLETE empty body; `func_8001F938` still
  INCOMPLETE score 11; `calc_loc_mat_fw_80055B60` still ASM-STRUCTURAL.
- (d) **Wait for sibling resolution + back-port** — passive, but a
  matched sibling with the same lhu/andi/beq shape would resolve the
  irreducibility question definitively.

## Session-3 (2026-06-07, HEAD 23e0ff7c) — confirmed reproducible; no new lever

Re-applied candidate.c to src/code6cac.c: sandbox `--disable all` reports
score=3, build_insns=58, target=59 (matches session-1 measurement exactly).

Tested one untried form from session-2's next_hypotheses (ii): inline
`*(s16 *)(entry + 0x6A)` re-read at the 3rd-test position, keeping `u16 a1`
for the cached read used in tests 1 and 2. Result: score 3 but
build_insns 61 (+2 over target). Same +2 sll/sra penalty as the inline
(s16)a1 rejected_form — the s16 pointer dereference at the comparison
position emits sign-extension shifts when not folded into a stored local,
and the load opcode at the inline s16 read is its own `lh` (not the
desired `lhu`). The hoped-for separation between cached u16 load and
comparison-site sign-extension doesn't help.

Other candidate forms considered but NOT measured (vetted out as cheats
or duplicates of rejected forms):
- nested struct/union punning to read same memory as u16 then s16 —
  cheats-by-spelling concern, no semantic purpose
- opaque mask variable `u32 mask = 0xFFFF; (a1 & mask) != 0xA` —
  already documented as cheats-by-spelling violation (see § Why parked)
- DImode chain — forbidden per [[global-label-drift-sibling-cheat]]
- compound (s32)(s16)a1 vs (s16)(s32)a1 cast orderings — equivalent RTL

The 3-diff cluster is structurally coupled: the only known mechanism
that triggers target's andi emission (a separate top-level
`s32 a1_s = (s16)a1;` local) also forces (lh load + 8-byte stack
inflation) which costs more than the 3 diffs it would close.

Floor remains 3. The function stays at this WIP checkpoint awaiting
a fundamentally different angle (e.g. permuter from the candidate-3
base with PERM_GENERAL operand directives; instrumented combine.c
probe per session-2's hypothesis 3; or escalation as a documented
plateau if no further lever surfaces).

## What session-1 DID NOT solve

The comparison block at maspsx idx 14-22:

```
target:                          build:
lhu  $a1, 0x6A($s1)              lhu  $v1, 0x6A($s1)
li   $v0, 0xA                    li   $v0, 0xA
andi $v1, $a1, 0xFFFF    <---    [no andi]
beq  $v1, $v0, .L                beq  $v1, $v0, .L
nop                              nop
lh   $v0, 0x72($s1)              lh   $v0, 0x72($s1)
nop                              nop
bnez $v0, .L                     bnez $v0, .L
addiu $v0, $a1, -23              addiu $v0, $v1, -23
```

3 diffs: lhu reg, missing andi, addiu reg. The andi is REDUNDANT
semantically (lhu already zero-extends to 32 bits) but target emits
it. cc1's `combine` pass folds the redundant mask in build. Forms
tested for forcing the andi all collapsed (see
`meta.json#rejected_forms`).

## Sibling functions with the same pattern

Same `lhu/andi/beq` target shape appears in:
- `func_8001A820` — INCOMPLETE, body empty in src (no source to
  diff). 1 rule, distance 574 (ASM-PARTIAL).
- `func_8001F938` — INCOMPLETE, distance 11, 13 rules. Source has
  `u16 kind = *((u16 *)(arg0 + 0x6A));` and `if (kind == 0x11 ||
  kind == 0xF || ...)` — same lhu-then-equality-on-low-16 shape.
- `calc_loc_mat_fw_80055B60` — ASM-STRUCTURAL park bucket.

None has solved the andi materialization yet. The pattern is a
recurring unsolved issue in this codebase.

## Why this is parked (not COMPLETED-C)

The remaining 3 diffs cluster around forcing GCC to materialize a
redundant `andi $v1, $a1, 0xFFFF` from C source. Per
[[no-new-park-categories]] cheats-by-any-spelling, using an opaque
mask variable (`u32 mask = 0xFFFF; if ((a1 & mask) != 0xA ...)`)
purely to defeat combine's fold is a borderline cheat (no semantic
purpose to the mask on an already-u16 value). Other pure-C levers
tested in `rejected_forms` all left score at 3.

Resume avenues in `meta.json#next_hypotheses` — including the
instrumented-cc1 combine.c probe approach per
[[difficult-is-not-impossible]].

## Related rules
- [[hoist-call-arg-local-flips-jal-delay]] — the applied lever
- [[register-alloc-pure-c]] — RA-via-C-structure playbook
- [[difficult-is-not-impossible]] — keep grinding (the andi gap is
  a coupling, not a wall)
- [[no-new-park-categories]] — cheats-by-any-spelling policy that
  ruled out the opaque-mask-var "fix"
