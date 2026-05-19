# Targeted permuter plan

**Status:** Planning. Not yet implemented.
**Goal:** Extend `decomp-permuter` with BB2-specific mutation passes that
explore SOTN-accepted pure-C techniques the upstream tool doesn't try, so
we can drive a meaningful subset of the 228 regfix-burdened functions to
zero rules.

## Why this, why now

Investigation on `coli_HitPauseKatana_2` (committed and reverted across
`f7c7379` / `09fb361` / `d9e5b4f`) established that:

1. The current `decomp-permuter` random pass plateaus quickly (~150
   weighted score for coli) because its mutation set doesn't include
   the BB2-relevant techniques: register-pin combinations, inline-move
   aliasing block insertion, or intermediate-variable injection at
   structurally-meaningful points.
2. `cc1psx` (the actual original compiler) also fails to reach target
   bytes from our pure-C body — proving for *that one function*
   the source is unrecoverable. But that's the worst case; cheaper
   regfix functions in the queue have target sources that ARE
   structurally reachable from our C, just not from any C the random
   permuter happens to produce.
3. The community standard (SOTN/Vagrant/ESA/CTR) explicitly accepts
   the patterns we'd add as mutation passes (see
   `memory/rules/community-standard.md`):
     - `register T x asm("$N");` pins
     - Single-instruction `__asm__ volatile("op %0,%1,$zero" : "=r"(d) : "r"(s));`
       with placeholders + GCC constraints (the libgte.h shape)

A tool that automates exploring these mutations against our 228
regfix-burdened functions could realistically retire ~30-40 of them
cleanly, without re-doing manual coercion per function.

## Architecture choice: extend, don't rebuild

`decomp-permuter` is mature: AST manipulation via `pycparser`, well-defined
randomization passes in `src/randomizer.py`, a scoring pipeline that
diffs the compiled object against `target.o`. Rebuilding that infrastructure
from scratch would take weeks. We extend.

The right extension points:
- **Add new randomizer passes** in `src/randomizer.py` for the BB2 patterns
- **Optionally swap the scoring function** to be "bridge-aware" (penalize
  diffs whose remaining-bridge regfix would require non-uniform-rename rules)
- **Optionally add a per-pass weight knob** so we can tune which patterns
  the random search exploits

No fork of upstream needed initially — patches can land on
`tools/decomp-permuter/` directly. If the patches prove valuable, upstream
them.

## Phase plan

### Phase 1 — auto-aliasing pass (MVP, 2-3 days)

**The single highest-leverage feature.** Insert `__asm__ volatile("op %0,%1,$zero" : "=r"(dst) : "r"(src));` blocks at randomly-chosen C points with randomly-chosen register pin combinations.

Implementation:

1. New randomizer pass: `try_insert_aliasing_block`
2. Walk the AST for a candidate function-body block (between two statements)
3. Pick two local-scope `s32`/`u32` variables already in scope at that
   point — call them `dst` and `src`
4. Emit AST nodes for:
   ```c
   register T dst_pinned asm("$RD");
   register T src_pinned asm("$RS");
   __asm__ volatile("addu %0, %1, $zero" : "=r"(dst_pinned) : "r"(src_pinned));
   ```
   where `$RD` / `$RS` are chosen from {`t0`-`t9`, `s0`-`s7`, `v0`, `v1`}
5. Replace subsequent reads of `dst` with `dst_pinned` (or wrap the alias
   so original variable name still works)

Test on a function we KNOW responds to manual aliasing (one of the
existing matched functions that uses `inline-move-aliasing`, e.g.
`calc_fc_frame_800203B4` per `memory/reference/inline-move-aliasing.md`).
Strip the manual aliasing block, see if the pass reconstructs it. If
yes, validation done.

### Phase 2 — register-pin saturation pass (1-2 days)

Add a randomizer pass that tries `register T x asm("$N")` annotations on
local declarations.

Implementation:
1. New pass: `try_add_register_pin`
2. Walk AST for `Decl` nodes with `s32`/`u32`/pointer type
3. Pick one randomly, add `register T x asm("$N")` annotation
4. `$N` rotates through `t0`-`t9`, `s0`-`s7`, `a0`-`a3`, `v0`-`v1`

Stack with phase 1: the permuter randomly inserts aliasing blocks AND
adds register pins. Together they should cover the existing
`memory/reference/inline-move-aliasing.md` recipes' search space.

### Phase 3 — intermediate variable injection (2-3 days)

Permuter's existing passes don't try inserting `s32 alias = expr; ... alias`
chains, which is the pattern we discovered manually on coli
(`new_var2 = mask` alias trick that closed the mask-routing diffs).

Implementation:
1. New pass: `try_inject_alias_var`
2. Walk AST for expressions used in multiple places
3. Pick one randomly, replace first use with `s32 alias = expr;`,
   subsequent uses with `alias`
4. Variant: same but place the alias declaration in an unreachable
   position (before a `case` label inside a switch) — that's a
   surprisingly load-bearing GCC quirk we hit on coli

### Phase 4 — bridge-aware scoring (3-5 days, optional)

Modify the scoring function so it doesn't just count byte-diff but
estimates "how many auditor-acceptable regfix rules would close this gap."

A variant is in the "auditor-clean" position if:
- All remaining diffs are register renames consistent with a `swap $X <-> $Y`
- OR all remaining diffs are uniform substs that match a uniform pattern
- AND no diffs require `delete`/`insert`/`splice`/structural reorder

Variants in the "auditor-clean" position get score reduction proportional
to how few rules would close them, making them strictly preferred over
equally-byte-diff variants that need cheating rules.

Risk: this is the most complex phase and could be skipped if phases 1-3
prove enough.

## Validation strategy

Before integrating any phase into the project's `dc.sh` pipeline:

1. **Regression test set:** Pick 5-10 functions that are *already*
   matched in pure C with single-insn aliasing (e.g., the
   `calc_fc_frame_800203B4`-family, the text1b.c GTE wrappers). For
   each, strip the aliasing back to a "before" state, run the targeted
   permuter, confirm it reconstructs the working C.
2. **Plateau probe:** Pick 5-10 functions from the regfix-burdened
   list ordered by smallest current rule count (1-3 rules). Run the
   targeted permuter for 10 minutes each. Score the rule-count
   reduction.
3. **Negative test:** Run the targeted permuter on `coli_HitPauseKatana_2`.
   We expect plateau at score 150 (the cc1psx-confirmed lower bound). If
   the targeted permuter beats that, something's wrong in the scoring;
   if it doesn't, the tool is correctly identifying the unrecoverable
   case.

The regression suite catches the worst failure mode (tool produces
non-compiling C). The plateau probe measures real-world utility. The
negative test confirms the tool doesn't oversell.

## Risk & ROI estimate

| Risk | Likelihood | Mitigation |
|---|---|---|
| Phase 1 produces compile-error C (operand binding wrong) | Med | Regression suite |
| AST manipulation breaks decomp-permuter's existing passes | Low | Keep changes confined to new pass files |
| Tool finds variants but they regress sibling functions | Low (single-function scope) | Existing `verify --all` catches this |
| Tool doesn't help meaningfully (false-negative) | Med | Negative test on coli is honest |

**Estimated investment:** 1-2 weeks for phases 1-3. Phase 4 optional.

**Estimated payoff:**
- Direct: 30-40 of 228 regfix-burdened functions cleanly retired
- Indirect: rules-pattern library to inform manual work on the rest
- Knock-on: pattern recognition might surface "yes target sources of these N functions all share shape X" — useful for the unrecoverable-source group

**Stop condition:** If after Phase 1 the regression suite isn't reliably
reconstructing manual aliasing on the easy cases, abandon (the
representation isn't tractable). If Phase 1 works but Phase 2-3 produce
no plateau-probe improvements, ship Phase 1 alone and move on.

## Out-of-scope

- Patching GCC to match cc1psx — separately investigated, ruled out
- Building a "source archaeology" tool to infer target's original C —
  research problem, not engineering
- Replacing decomp-permuter's core scoring — extension only
- Anything specific to `coli_HitPauseKatana_2` — that function is the
  unrecoverable-source case and a new tool won't help it. Re-bridged and
  left in queue for future agents.

## Open questions to discuss before starting

1. Pass weight / search budget: how often should random search pick each
   new pass vs the existing passes? Default 1:1 with existing, but tunable.
2. Should `$a0-$a3` be in the pin set? They conflict with parameter passing
   in ways the random search may not understand. Initial pass: yes, let
   it try. If it produces broken C on parameter functions, restrict.
3. Where to ship: as a fork of `tools/decomp-permuter` checked into our
   tree, or as patches we maintain externally? Initial: in-tree, lower
   friction. Upstream if it works.
