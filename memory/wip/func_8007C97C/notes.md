# func_8007C97C — WIP resume notes

## Standing policy — canonical-asm authorization IS REJECTED for this function

> 2026-06-02 user policy (codified after round 1 worker re-proposed it):
> **DO NOT propose canonical-asm authorization for func_8007C97C unless you
> first surface a fresh measured S1/S2/S6/S8 audit demonstrating STRONG-tier
> evidence per `.claude/rules/hand-coded-asm-recognition.md`.** Target has
> ZERO hand-coded signals (S1=0 `/* handwritten instruction */`, S2=none
> hardcoded register signatures, S6=standard `$a0..$a3` ABI, S8=no
> packed-multiply tell); no GTE/BIOS/scratchpad/no-C-form constructs.
> The function is structurally normal C — see `meta.json` rejected_forms[0]
> for the full evidence. Round-1 worker's proposal was the same misguided
> framing as B844's session-3 proposal; both rejected on the same evidence.
>
> **2026-06-04 SOTN-precedent reinforcement (round 6):** Fresh SOTN-decomp
> research (`tmp/sotn_research_func_8007C97C.md`) confirms the policy. SOTN
> has NO documented policy on dead stack writes; SOTN's `INCLUDE_ASM(...)`
> with auto-`.NON_MATCHING` suffix and `nonmatchings/` paths is the
> equivalent of BB2 PARK, **not** of COMPLETED-INLINE-ASM-CANONICAL.
> SOTN has no curated canonical-asm list with positive S1/S2/S6/S8
> criteria. Under SOTN precedent, C97C would sit as `INCLUDE_ASM` (= PARK)
> indefinitely — not promoted to a "finished" tier. The standing rejection
> stands.

## TL;DR

- A `Match:` commit (`0e845d2`, 2026-06-01) was **reverted** (`45a7bb0`,
  2026-06-02) because the independent cheat-reviewer agent flagged a
  `s32 sp[4]; sp[0..3] = ...;` write-only frame coercion as a cheat-by-
  spelling — same family as `s32 buf[N];` in dead-vars-local-array, but
  spelled with writes-but-no-reads instead of declared-but-unreferenced.
- **The inverted-null-check structural insight IS a real, legitimate
  lever** — it's the SOTN-clean replacement for the now-archived
  `goto-end-prologue-delay-slot` technique. The reviewer FAILED ONLY on
  the sp[4] part. The candidate body in `candidate.c` preserves the
  legitimate parts (inverted null-check + m2c body + r_e2 + var-reuse).
- **NOT yet measured.** The next session's first task: apply
  `candidate.c` to `src/display.c`, run `sandbox func_8007C97C
  --disable all`, record the floor in `meta.json.scores.candidate_floor`.
- The rejected form is preserved at `rejected/sp4_frame_coercion.c` with
  the reviewer's evidence inline — DO NOT re-derive it.

## How to resume in one read

1. Read `meta.json` — note `reviewer.verdict` is `FAIL_on_prior_form` for
   the rejected sp[4] form, and the candidate's verdict is NOT yet run.
2. Read `rejected/sp4_frame_coercion.c` — understand what the cheat
   was, so you don't re-introduce it under a different name (`spill`,
   `stk`, `_pad`, `buf` — same family).
3. Apply `candidate.c`'s body to `src/display.c` (replace HEAD's
   func_8007C97C body).
4. `& tools/eng.ps1 sandbox func_8007C97C --disable all` — record the
   floor. Update `meta.json` accordingly.
5. **Re-run cheat-reviewer** on the candidate as a sanity check. The
   removal of the sp[4] writes should clear the prior FAIL.
6. From there, the work is finding a SOTN-clean lever (likely
   register-allocation or frame-layout structural) that closes whatever
   gap remains.

## Lessons preserved here

1. **The inverted-null-check pattern is legitimate.** Don't lose that
   in the panic of the revert. It's the right replacement for the
   forbidden goto-end accumulator that the archived
   `goto-end-prologue-delay-slot` rule was trying to encode.

2. **Mechanical detectors are necessary but not sufficient.** The
   find_unused_local_arrays detector missed sp[4] because it checks
   reference COUNT, not reference KIND (read vs write). A write-only-
   array detector is needed; that's filed as a separate engine work
   item in the revert commit.

3. **Workflow workers self-administering the cheat-reviewer checklist
   is unreliable.** The session-0e845d2 worker self-administered the
   6-test checklist and claimed PASS. The independent reviewer caught
   the cheat. Future workflow runs must invoke the reviewer subagent
   from the orchestrator side, not delegate self-review to the worker.

## What to AVOID re-deriving

- Any `T name[N];` (any name, any type) where the array is declared,
  written to, but never read. Same family as the rejected form. The
  rejected/ file documents the specific evidence.
- Any other frame-coercion construct: `volatile T pad;`, `(void)&local;`,
  declared-but-unused structures, etc. See
  `.claude/rules/inline-asm-policy.md` § expanded cheat catalog.
- The earlier `goto end; ... end: return ret_val;` accumulator pattern
  (now archived in `goto-end-prologue-delay-slot.md`). The inverted-
  null-check in candidate.c is the legitimate replacement.

## Related

- `.claude/rules/goto-end-prologue-delay-slot.md` — the archived rule
  this WIP's candidate cleanly replaces
- `.claude/rules/dead-vars-local-array.md` — the catalog rule the
  rejected form violates
- `.claude/rules/review-discipline-before-commit.md` — the gate that
  caught the cheat (independent reviewer invocation, post-fact)
- Revert commit `45a7bb0` — the full reviewer evidence + the
  next-detector-gap filing

## Session 2026-06-02 (workflow round 1)

Structural re-analysis of target asm confirmed the 4 dead `sw` stores + 0x10
frame allocation. Considered two alternative coercion shapes (read+write array,
struct-member variant) — both fail human-programmer test and naming-intent test
and were rejected without measurement per default-to-NO_PROGRESS. Worker could
not measure the existing candidate in the isolated worktree. Suggested next
step: escalate for canonical-asm authorization. CAUTION: same caveat applies
as B844's user_policy_note — canonical-asm needs explicit S1/S2/S6/S8 signals,
not evidence-by-absence-of-pure-C-path; user adjudication required.

## Session 2026-06-03 (workflow round 3)

**Floor unchanged at 24 — NO MEASUREMENT POSSIBLE.** Applied WIP candidate.c
and tested a natural-C form; sandbox returned non-scorable due to display.c-wide
cheat-asm-strip pipeline truncation (the same blocker that affected C7A0 and
CBB0 workers in parallel sessions). Worker recommended surfacing as a policy
escalation toward canonical-asm — REJECTED per the **standing policy at the
top of this file** (no fresh S1/S2/S6/S8 audit evidence; absence-of-pure-C-path
is not itself the canonical-asm threshold per `.claude/rules/hand-coded-asm-recognition.md`).

**Unblocker for round 4:** the C86C round-3 worker discovered `--keep-cheat-asm`
restores indices for display.c functions whose own body has no cheat-asm; this
should be tried for C97C next round to get a measurable baseline. Once
baseline measurable, the next-step menu reverts to `meta.json` next_hypotheses:
apply candidate body, measure, re-run cheat-reviewer.

## Session 2026-06-04 (workflow round 4) — FIRST SUCCESSFUL MEASUREMENT

**candidate_floor MEASURED for the first time: 29 (was null).**

The actual unblocker was `pwsh tools/setup_worktree.ps1` — it creates
junctions to main's `.venv`, `build/`, `tools/gcc-2.7.2`, `disc/`. With those
junctions in place, plain `tools/eng.ps1 sandbox func_8007C97C --disable all`
runs cleanly without needing the `--keep-cheat-asm` workaround. Round-3's
pipeline-truncation issue did not reproduce (sandbox returned `scorable=true`,
`score=29`, `rules_dropped=9`, `cheat_asm_stripped=446`, `build_insns=25`,
`target_insns=33`).

**Why 29, not 24 (the prior session's documented head_floor):** the 24 was an
artifact baseline from before the candidate body was clean of the sp[4] coercion;
it was never the legitimate-candidate's measured floor. The true honest pure-C
floor of the candidate body — the closest legitimate form, inverted-null-check +
m2c-shape + r_e2 named intermediate + temp_v0 var-reuse — is **29**.

**Variants measured (all ≥ 29):**
- candidate.c body (preserved): score **29** ✓ base
- Natural-C single-expression body: score **30**

**The un-closable gap (8 insns) is the 4 dead `sw` stores + frame allocation:**
target writes `sp[0..3] = r/g/b1/b2` but never reads them; these dead writes
came from either hand-coded asm or from C constructs in the forbidden cheat
catalog (write-only array, dead self-assign, volatile coercion, address-of-void).
DCE in cc1 removes any local that isn't read — no pure-C source produces 4 dead
stack writes without being itself a cheat-by-spelling.

**No new lever; no candidate.c modification; src reverted; oracle green.**
Outcome: NO_PROGRESS. The candidate remains the legitimate base; the function
remains INCOMPLETE pending either (a) a novel non-cheat C structural lever that
produces dead stack writes (unlikely — DCE catches the class) or (b)
reconsideration of the canonical-asm-authorization standing rejection (also
unlikely — S1/S2/S6/S8 signals remain absent).

## Session 2026-06-05 (round 7) — FLOOR LOWERED 29 → 27 via narrow integer type (Lever B)

**Outcome: WIP_PROGRESS.** First measurable floor reduction in 3 rounds.

The user policy for this round explicitly authorized continued work past
the SOTN-precedent PARK conclusion. Executed: (1) re-measure round-5 candidate
floor 29 (confirmed); (2) apply 3 NEW structural variants outside
rejected_forms; (3) 12-minute permuter run with 6 workers from candidate base.

### The lever — u8 color channel temps + u32-cast OR pack (Lever B)

```c
s32 func_8007C97C(u8 *arg0) {
    if (arg0 != 0) {
        u8 r  = arg0[0] >> 3;
        u8 g  = (-*(s16*)(arg0 + 4)) >> 3;
        u8 b1 = arg0[2] >> 3;
        u8 b2 = (-*(s16*)(arg0 + 6)) >> 3;
        return ((u32)b1 << 0xF) | ((u32)r << 0xA) | 0xE2000000u | ((u32)b2 << 5) | (u32)g;
    }
    return 0;
}
```

Measured: **score 27**, build_insns=24, target_insns=33, rules_dropped=9
(down from 29/25 with the wider-typed candidate). This is Lever B per
`.claude/rules/register-alloc-pure-c.md` — narrow integer typing is a
documented legitimate technique. u8 IS the natural type for color channels
(RGB components are 0-255 by domain). The narrow type changes cc1's
value-width tracking AND its register allocation, dropping one
instruction from the build vs the u32-typed form.

### Variants measured this round (all from round-5 candidate base, floor 29)

| Variant | Score | build_insns | Notes |
|---|---|---|---|
| Round-5 candidate (u32 r, s32 g) | 29 | 25 | Re-confirmed |
| V1 halfword-pointer alias (`s16 *hp = (s16*)(arg0+4)`) | 29 | 26 | No improvement |
| V2 in-place OR accumulator (`ret \|= ...`) | 30 | 25 | Regression |
| V3 u8 temps + explicit u32 casts | **27** | **24** | **Winner** |
| V4 V3 simplified (no inner u8 casts on arg0[N]) | 27 | 24 | Same |
| V5 V3 + named e2 intermediate | 27 | 25 | Slightly different masking |

V3 saved as new `candidate.c`. The other variants ruled out.

### Permuter (12 min wallclock, ~6749 iters, 6 workers, --stop-on-zero)

Started from round-5 candidate base (`base.c` = the wider-typed form,
score baseline ~3100 in permuter weighted metric). Best legitimate-shape
weighted score: **1010** (output-1010-1). All best finds are the same
address-coercion family:

- output-1010-1: `s32 *new_var = &temp_v1; ... (*new_var) << 5`
- output-1095-1: `u32 *new_var2 = &temp_v0;` (address-coerce + dead writes)
- output-1205-1+: same family with different `&temp_*` aliases

**ALL REJECTED** per `[[no-new-park-categories]]` cheats-by-any-spelling and
`[[dead-vars-local-array]]` scalar-address-coercion form (detected by
`engine/volatile_cheats.find_addr_coerced_locals`). Zero semantic purpose;
the address-of-local pattern exists only to force the target's dead `sw`
frame writes via &local materialization.

Saved at `rejected/addr_coerce_temp.c` so the next agent doesn't re-derive
it. The permuter's `PERM_GENERAL` random mutation pass will keep finding
this same family — ignore them all.

### What this round established

1. **The 29 floor was NOT terminal.** A documented Lever B (narrow int type)
   moved it to 27 — Lever B was simply un-applied in prior rounds; the
   focus had been on structural-shape levers (inverted-null-check, m2c
   form, var-reuse) and missed the type-narrowing axis.
2. **The 27 floor IS structural.** The remaining 9-insn gap is still
   exactly the 4 dead `sw` stores + frame slop that sessions 4-6
   established cannot be reached via any legitimate pure-C construct.
   DCE removes any pure-C local that isn't read; the only way to
   produce a dead stack store is via a coercion construct, all of
   which are in the forbidden catalog.
3. **The permuter's `PERM_GENERAL` random mutation is exhausted** for
   this function. Sub-baseline finds are all in the address-coercion
   family (forbidden). Future permuter work would need directed
   `PERM_*` macros (e.g. PERM_INT_TYPE) targeting specific type
   axes the random pass under-explores.

### Concrete next-attempt avenues (for next session)

Per user policy "PARK_CANDIDATE requires concrete next action":

1. **Directed permuter with `PERM_INT_TYPE` on residual variables.**
   The u8 lever worked on the 4 color channels; the `r_e2` intermediate
   and the OR-pack return type may have analogous wins.
2. **Apply Lever A (block-local var split) to the OR-pack.** The current
   form computes the entire OR expression in one statement; splitting
   into block-local sub-pieces may shift cc1's RA in the prologue/return
   block to closer match target.
3. **Investigate `arg0` parameter type.** Currently `u8 *`. Target's
   first load is `lbu 0($a0)` (byte), but later loads are `lh 4/6($a0)`
   (halfword) — suggesting arg0 may have been declared as a struct
   pointer with mixed-width fields. A struct typedef matching the
   actual layout (`struct { u8 r, _, b1, _; s16 g, b2; }`) would
   produce naturally-typed reads without the manual casts and may
   shift scheduling.
4. **GCC `-da` dump of the round-7 candidate** to see exactly which
   pseudo-registers conflict at the prologue surface — the remaining
   9-insn gap may have a Lever C (precompute) angle the manual variants
   missed.

These are concrete, mechanism-grounded next steps — NOT a policy
escalation request. The function continues to grind toward COMPLETED-C.

## Session 2026-06-04 (round 6) — SOTN-precedent research, definitive PARK confirmation

**Floor unchanged at 29 (not re-measured this round — sandbox cost not
spent; sessions 4 and 5 both measured 29 and round-6's lever is policy,
not C). Outcome: NO_PROGRESS. Src untouched; oracle green at HEAD 88b90686.**

Round-6 lever was *not* a code lever — it was a fresh SOTN-decomp research
agent (sources: STYLE.md, README.md, wiki, include/include_asm.h,
src/st/, src/main_psp/) to determine whether SOTN provides precedent for
widening the standing canonical-asm-rejection on this function. The
research report is at `tmp/sotn_research_func_8007C97C.md`. Three findings:

1. **SOTN has no documented policy on dead stack writes.** Searches across
   SOTN's docs, wiki, and issue tracker for "dead store", "dead stack",
   "frame coercion", "unused frame", "stack write" returned **zero hits**.
   The pattern of "compile to dead `sw` stores that the C source did not
   ask for" is not a SOTN-known problem with a SOTN-known answer.

2. **SOTN's `INCLUDE_ASM` is PARK, not canonical-asm.** From
   `include/include_asm.h`, every `INCLUDE_ASM(FOLDER, NAME)` block
   auto-suffixes `NAME.NON_MATCHING` and is conventionally placed under
   a `nonmatchings/` path. This is SOTN's "TODO, not yet matched" marker.
   SOTN does **not** require positive S1/S2/S6/S8 signals before leaving
   a function as `INCLUDE_ASM`. There is **no SOTN equivalent** of BB2's
   curated `inline_asm_canonical.txt` "originally asm, accepted finished
   form" tier. The two-tier mapping is:
   - **BB2 PARK** ≡ SOTN `INCLUDE_ASM(... nonmatchings/...)`
   - **BB2 COMPLETED-INLINE-ASM-CANONICAL** ≡ no SOTN analog (BB2-specific)

3. **SOTN's matched corpus has no analog for this shape.** Searches of
   SOTN code for `addiu $sp` in C-source returned one hit — a comment
   in `tools/make-config.py` about prologue detection. Zero matched
   SOTN functions emit inline-asm prologue spillage to manufacture
   dead frame stores. SOTN's matched GP0 builders use clean OR-pack
   idioms (same as our siblings `func_8007C8AC` / `func_8007C938`).

**Conclusion: SOTN precedent reinforces PARK, refuses canonical-asm.**
Under SOTN's actual practice, C97C would sit as `INCLUDE_ASM(...)` under
a `nonmatchings/` path indefinitely — counted as "not yet matched," not
promoted to a finished tier. Under BB2's stricter policy, that maps to
PARK (current state), NOT to COMPLETED-INLINE-ASM-CANONICAL.

The standing canonical-asm-rejection (top of this file, codified
2026-06-02) is therefore correct AND now externally corroborated by
the SOTN-precedent question that round-5's `next_hypotheses[0]` had
raised. The policy axis is closed: **no further iteration on
canonical-asm authorization for this function is expected to produce
a different answer.** The function remains PARKED. Future work, if
any, must be a novel pure-C structural lever capable of producing
4 dead `sw` stack stores that survive cc1's DCE pass without being
itself a cheat-by-spelling — a search-space that prior sessions'
mechanism analyses (sessions 4-5) have argued a-priori-rules-out.

## Session 2026-06-05 (round 10) — extended permuter + GCC -da dump; NO_PROGRESS, floor 27

**Outcome: NO_PROGRESS. Floor unchanged at 27.** Two new concrete investigations
executed; both add to the empirical record without surfacing a new lever.

### Investigation #1 — GCC -da dump on round-7 candidate (`tmp/c97c_dump/`)

Compiled the round-7 u8 candidate with `cc1 -O2 -G0 -funsigned-char -da` to dump
all RTL passes. Findings from `base.c.greg`:

- **Register dispositions:** the candidate uses ONLY hard regs $v0/$v1/$a0/$a1
  (regs 2/3/4/5). ZERO callee-save regs used.
- **Hard regs used: 2 3 4 5** — confirmed.
- The 24-insn body fits entirely in scratch regs because (a) no function calls,
  (b) live ranges fit, (c) no locals taken-address. cc1 has no reason to spill
  to frame.

To match target's 33 insns with the 4 dead `sw $rN, K($sp)` stores + frame
alloc/dealloc + 2 nops, cc1 would need to be forced into using callee-save regs
OR spilling to frame. The pure-C constructs that achieve this are all in the
forbidden cheat catalog:
- `register T x asm("$sN")` pins (forbidden cheat-asm)
- `volatile T x` coercion (forbidden)
- `s32 buf[4]; buf[i] = ...;` write-only array (forbidden)
- `T *p = &local;` address-of-local (forbidden)
- function call inside the if (semantic change — not the original)

The candidate's compact codegen is exactly what cc1 *should* produce from
clean pure C; the target's frame-using codegen is what hand-written asm OR
a now-forbidden coercion construct produces.

### Investigation #2 — index-aligned objdump diff (sandbox-stripped vs target.s)

The exact 9-insn structural gap:
| Region | Target (33 insns) | Candidate (24 insns) |
|---|---|---|
| NULL handling | bnez/addiu(sp,-0x10)(delay)/j-END/addu(v0,0,0) = 4 | bnez/move(v0,zero)(delay)/jr = 3 |
| Computation | 21 insns | 19 insns |
| Frame stores | 4 × sw $rN, K($sp) | 0 |
| Epilogue | addiu(sp,0x10)/jr/nop | jr/nop |

The NULL-path frame allocation (`addiu $sp,-0x10` in the bnez delay slot, even
when the function returns 0 immediately) is the structural signature target
emits. No pure-C source produces a mandatory frame even on a NULL early-return
path without the frame being USED for some local — and DCE removes any unused
local.

### Investigation #3 — extended PERM_GENERAL permuter (12000+ iters / 20 min)

Re-baselined permuter base.c to the round-7 u8 candidate (was wider-typed).
Base score: **3065** weighted. Run: `timeout 1200 permuter.py -j 6 --best-only`.

Best-five sub-baseline finds, ALL cheats:

| Score | Output | Cheat category |
|---|---|---|
| 1625 | output-1625-1 | `u8 *new_var2 = &b1;` — scalar-address coercion ([[dead-vars-local-array]] / [[inline-asm-injection]]) |
| 1790 | output-1790-1 | `volatile int b2` + `if(1){}` wrap noise — volatile coercion |
| 1800 | output-1800-1 | Chained `& 0xFF & 0xFF & 0xFF...` bit-noise — no-semantic-purpose construct |
| 1850 | output-1850-1 | `volatile int r` + `if(1)` wrap — volatile coercion |
| 1915 | output-1915-1+2 | `volatile int new_var` / `volatile unsigned int r` — volatile coercion |

**ZERO sub-baseline LEGITIMATE candidates** in 12000+ iters. The PERM_GENERAL
random-mutation pass from the u8 base produces the same family of forbidden
coercion constructs as round 7's 6749-iter run from the wider base — local
mutation does not synthesize a non-coercion structural lever capable of
producing target's 4 dead stack writes. The permuter weighted score is NOT
directly comparable to sandbox masked-distance (1625 weighted does not mean
< 27 masked distance), but every legitimate-shape mutation tried in 19000+
combined iters across rounds 7 + 10 has held at or above the round-7 floor.

### Cumulative exhaustion ledger (sessions 1-10)

- 30+ structural variants tested by hand across sessions 0-9 + this session
- 19000+ permuter iterations across rounds 7 + 10 (PERM_GENERAL random)
- GCC -da RTL dump on the candidate (this round) — confirms minimal-reg codegen
- Sibling cross-reference (func_8007C8AC GP0(0xE4) / func_8007C938 GP0(0xE5)
  both pure-C with zero dead stores; this function is structurally anomalous
  in its own GP0 cluster)
- SOTN-precedent research (round 6, definitive negative result)
- Instrumented mechanism analysis (sessions 4-5: DCE removes any local not
  read; 4 stack stores require &local / volatile / write-only-array /
  struct-return — all forbidden or ABI-ruled-out by caller analysis)
- 25+ variants in rejected_forms across rounds 0-8

Src reverted; oracle green at session end (verify-oracle ok:true, SHA1 ==
62efab4f73f992798c43e8c730aa43baa10bb4fa).

### Concrete next-attempt avenues (carry-through)

NOT a policy carve-out request. Per [[no-new-park-categories]] PARK_CANDIDATE
is NOT terminal for this; the function continues to grind toward COMPLETED-C.

1. **Hand-authored PERM_GENERAL macro embeddings** — the default random
   alternative-snippets pass produces no-purpose mutations (round 7 + round 10
   both confirm). Designing PERM_GENERAL(a, b, c) directives that explore
   semantically-equivalent NON-COERCION structural alternatives per C-statement
   (e.g. alternative orderings of the 4 channel computations, alternative
   typedefs of the OR expression, alternative null-check positions) is the
   actual remaining frontier. Requires hand-authoring directives in base.c
   that ENFORCE legitimate-only mutations.
2. **Inspect dis_data_layout** — the original Bushido Blade 2 build's overall
   data layout MAY have placed `func_8007C97C` in a TU compiled with subtly
   different flags or in a context where cc1 chose to spill (e.g. local variable
   pressure from sibling functions in the same TU). Investigating splat re-
   attribution evidence for `display.c` (whether the GP0-builder cluster
   actually belongs in its own .c file with different compilation context).
3. **The function is INCOMPLETE indefinitely** — same status as
   `cpu_side_move_dir_4` / `marionation_Exec` / `saEft00Add` cluster
   (documented allocno-priority-tie / cross-jump-merge cluster per
   [[register-alloc-pure-c]] § confirmed limits). The grind continues on
   the C-source axis; the user policy precludes compiler patches or
   canonical-asm authorization.
