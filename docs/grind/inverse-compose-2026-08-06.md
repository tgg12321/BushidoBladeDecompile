# Inverse solver — cross-model composition (Phase 6, 2026-08-06)

Tool: `tools/ra_solver/inverse_compose.py`. Policy layer: `levers.py` (extended
with the pre-RA / CSE classes). No existing solver file was modified.

This is the last piece of the suite. The three backends each answer a question
about ONE model and each *assumes its model is the right place to ask*. That
assumption is what failed in Phase 4/5, and this module supplies the two things
that were missing: triage, and a way to test an upstream change downstream.

## 1. `classify` — which model does the residual FIRST appear in?

The pipeline is a funnel, and each stage can only permute what the stage above
built:

```
front end + cse/combine/loop   builds the INSN MULTISET
    -> global/local alloc      assigns REGISTERS to a fixed multiset
        -> sched.c             ORDERS a fixed, already-allocated stream
```

So comparing the honest stream against target's answers the question directly:

| test | verdict | next tool |
|---|---|---|
| register-blanked multisets differ | **PRE-RA** | none — upstream of every model |
| multisets match, exact texts differ | **RA** | `inverse.py` |
| texts match as a multiset, order differs | **SCHED** | `inverse_sched.py` |

One refinement, added after it mislabelled a real case: a **`nop`-only**
multiset difference is reported as SCHED, not PRE-RA. maspsx inserts load-delay
nops after the fact, so their count follows from what the schedule put in the
delay slot — downstream of RA, not upstream of it.

**This check is cheap and should run BEFORE any backend search.** It needs only
the two asm streams.

### Validation — 7 functions, against independently-recorded classifications

| function | verdict | corroboration |
|---|---|---|
| `func_800611A4` | RA | matches Phase 2 (local-alloc v0/v1) |
| `func_80033550` | RA | matches Phase 2 ($a1-vs-$a3) |
| `special_camera_get_rot_dir` | RA | matches Phase 2 (s-register rotation) |
| `gnd_init_80041688` | SCHED, 3 slots | matches Phase 4 (sched1 lbu emit-order) |
| `ang_hosei_80056FE8` | SCHED (nop-only, target +1) | **explains** the Phase 2 UNREACHABLE: the 42-vs-43 count is one delay-slot nop |
| `func_80037A20` | **PRE-RA** | matches the park reason's *second* clause, "cse REG_WAS_0 fold" — see below |
| `func_80072CD4` | **RA** | **corrects my Phase 5 conclusion** — see below |

Five confirm prior findings. Two change them, and both changes matter.

### Correction 1 — `func_80072CD4` is an RA residual, not a CSE one

Phase 5 concluded "target materialises 252 once into `$3`, ours twice into `$2`
— the instruction multiset differs". **That was wrong.** The register-blanked
multisets are *equal*: target also materialises 252 twice, just into different
registers. I reached the wrong conclusion by reading the two streams
side-by-side at equal indices instead of at their *aligned* positions, so a
register difference looked like a structural one.

Running the derivation properly: the residual is **4 substituted operands**,
`$v1 -> $v0` x2 and `$v0 -> $v1` x2 — a plain RA exchange at local-alloc scale.
Following the classifier's routing to the local backend (block 5, qty 3 `$v0`
<-> qty 1 `$v1`) yields **22 single-atom vectors**, cheapest being
`live_shrink` on qty 1 (born later, span 14->3) and `refs_up` qty 1 (3->7).

So the Phase 5 verdict "NOT A SCHEDULING RESIDUAL" was right for the wrong
reason: it is not a scheduling residual because it is an *allocation* residual,
not because the RTL differs. The function is now actionable.

### Correction 2 — `func_80037A20` has a PRE-RA component

Phase 3 reported a clean RA answer (pseudo 75 acquires an `$s1` copy
preference). The classifier finds an instruction-shape difference *as well*:

```
ours only  : addiu $#,$#,1        ours:   addiu $16,$zero,1   (materialise 1)
target only: addu  $#,$#,1        target: addu  $17,$17,1     (increment)
```

Ours materialises the constant 1 into a fresh register where target increments
in place. That is precisely the **"cse REG_WAS_0 fold"** named in the second
clause of the function's park reason, which my Phase 3 analysis looked past.

This does not invalidate the Phase 3 RA vector, but it re-orders the work: the
pre-RA difference is upstream, so it should be resolved first — the allocation
question may not survive it.

## 2. `hypothesis` — replay the downstream model under an upstream change

An upstream change cannot be written into a model file; it comes from different
C. But its *consequence* is expressible. `--merge P,Q` models "these two values
become one", which is what cse.c does when it unifies two materialisations: the
survivor takes the SUM of the references, the UNION of the live range, and the
UNION of the conflicts. The RA forward model is then replayed.

Two guards keep this honest, and both earned their place during validation:

- **None-safety**: a merged-away pseudo has no assignment; it prints `(gone)`.
- **Faithfulness guard**: a merge *deletes* Q, so a goal naming Q is
  unsatisfiable by construction, and a goal that merely *omits* Q is a fragment
  of the real target. Either way the RESULT line would read as encouraging
  while meaning nothing. The tool now says so loudly. I hit this myself: a
  partial goal `{74: $s0}` reported "the upstream change CLOSES the residual",
  which is false.

### Validation on `func_80037A20` — mechanism works, hypothesis is WRONG

The classifier routes this function to the pre-RA stage, so it is the right
place to test the composed question: *if CSE folded the constant, would RA then
produce target's assignment?*

```
baseline allocation : {74: $s1, 75: $s0}
under the hypothesis: {74: $s0}          (75 merged away)
goal {74: $s0, 75: $s1}
  baseline reaches it   : False
  hypothesis reaches it : False
RESULT: does NOT close it. Secondary residual: pseudo 75: got (gone), want $s1
```

**The honest reading: the merge is not a faithful model of this function's
fold.** Target still holds TWO distinct callee-saved values (`$s0` and `$s1`);
a merge models one. The `REG_WAS_0` fold there changes how a *local* quantity
is materialised (constant vs increment) without reducing the number of live
global allocnos — and a global-model pseudo merge cannot represent that.

That is a real limit, and it is the answer to the phase's honest-outcome
clause. What is missing, exactly:

> To express "this constant is materialised by an increment instead of a fresh
> `addiu`", the transformation has to act on the **instruction stream**, not on
> the allocno set: remove one insn, retarget its consumers, and re-derive the
> live ranges. The model files carry allocnos and their aggregate flow data
> (refs, live length, conflicts), not the insn-level RTL those were computed
> from. So this class of hypothesis needs an **RTL-level extraction pathway**
> — dumping the pre-RA insn stream with def/use links — which no current
> extractor produces. `merge-pseudos` covers the sub-class where the upstream
> change genuinely removes a *value*; the `REG_WAS_0` fold removes an
> *instruction* while keeping the value.

## 3. Lever mapping

Three new classes in `levers.py`, tier-disciplined as usual:

- **`cse_merge`** (target materialises once, we twice) → `store-const-reload-cse`
  (PLAIN, re-read the global instead of caching a local), single-named-
  intermediate (PLAIN), then `named-local-fake-exception` and
  `staged-value-reused-variable` as SANCTIONED last resorts with their
  prerequisites attached.
- **`cse_split`** (target twice, we once) → `split-read-defeats-hoist` (PLAIN),
  `defeat-combine-symbol-fold` (PLAIN), `duplicated-statement-into-arms`
  (SANCTIONED).
- **`rtl_shape`** → no lever: states plainly that no RA/scheduler perturbation
  can reach it, and that the expression the C computes has to change first.

The standing FORBIDDEN block prints on every report, unchanged.

## Reproduce

```bash
python3 tools/ra_solver/inverse_compose.py classify text1b func_80072CD4
python3 tools/ra_solver/inverse_compose.py classify code6cac_c func_80037A20
python3 tools/ra_solver/inverse_compose.py hypothesis \
        tmp/inverse_work/ra/func_80037A20.model.json --merge 74,75 \
        --goal '{"74": 16, "75": 17}'
python3 tools/ra_solver/inverse.py local tmp/inverse_work/ra/text1b.local.json \
        --func func_80072CD4 --block 5 --swap 3,1 --depth 1
```

## LIMITATION — `classify` returns a FICTITIOUS verdict on `replace_with_asmfile` functions

Found 2026-08-06 (nearmatch, the four retained Wave-5 near-matches). **Do not act on a
`classify` verdict for any function wired with an asmfix `replace_with_asmfile` rule.**
The verdict is an artifact of text conventions, not a measurement.

Two independent failures compound:

1. **The target is unreadable.** `mkasm_honest.sh` builds `<stem>.tgt.s` by running the
   real cheat pipeline, so a `replace_with_asmfile` function arrives as the split asm
   file's own text: a `glabel F` / `endlabel F` block whose instruction lines are prefixed
   `/* offset addr bytes */`. `goalmap.asm_body` looks for `F:` and skips lines starting
   with `/*`, so it raises `KeyError: F not found in <stem>.tgt.s` — or, worse, silently
   returns an empty/partial body if the name happens to appear another way.

2. **Normalizing the labels is not enough — the two sides are different languages.**
   `.hon.s` is ASSEMBLER SOURCE (maspsx output); the target block is DISASSEMBLY. They
   spell the same instruction differently, and `classify` compares register-blanked TEXT:

   | | ours (`.hon.s`) | target (asm/funcs) |
   |---|---|---|
   | frame setup | `subu $sp,$sp,144` | `addiu $sp, $sp, -0x90` |
   | immediates | decimal, no spaces | hex, comma-space |

   Both assemble to the identical word. `classify` sees a different instruction MULTISET
   and reports **PRE-RA / `rtl_shape`** — the one verdict that tells you to stop, because
   "no perturbation of RA or scheduler can reach it." On `func_80089F3C` it reported
   PRE-RA with a 287-vs-318 insn gap when the true residual was **zero codegen difference
   at all** (20 link-neutral relocation addends; see `memory/wip/func_80089F3C/notes.md`).

**Use `tools/pairdiff.py <stem> <func>` instead** (added 5c654c3b). It diffs the two
OBJECT files the score is actually computed from — `tmp/sandbox/<func>/<stem>.o` against
`build/src/<stem>.o` — through `engine.score.normalized_insns` with the engine's own
control-flow masking. Object-vs-object is what `sandbox` reports, so its diff reconciles
with the score by construction, and relocations render identically on both sides.

Note the corollary for the artifact family: because `pairdiff` compares OBJECTS, a
HI16/LO16 relocation-addend difference (`sw s0,4(at)` against `D_800A2888+4` vs
`sw s0,0(at)` against `D_800A288C`) still shows as a diff and still counts toward the
score, even though the linker erases it. That is a scoring artifact, not a codegen gap —
see the saEft00Add / AllocRobRmd rulings in `docs/grind/decisions.md`.
