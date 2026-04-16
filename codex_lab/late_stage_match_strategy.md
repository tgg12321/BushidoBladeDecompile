# Late-Stage Matching Strategy

This strategy is for functions that are already mostly decompiled and are no longer
blocked on basic semantics. It is especially meant for PS1 GCC 2.7.2 + ASPSX-style
projects like Bushido Blade 2, where the last gaps are often register allocation,
instruction scheduling, or assembler-emulation quirks.

## Core View

There are three different failure modes, and they should be attacked differently:

1. Source-shape mismatch
- The C still does not describe the same logic or control flow as the target.
- Fix this by reading the original asm directly and rebuilding the semantic shape.

2. Compiler-behavior mismatch
- The logic is right, but GCC 2.7.2 chooses different live ranges, temporaries,
  declaration-driven register assignments, or scheduling.
- Fix this with reduced experiments, manual perm macros, declaration order tests,
  temp splitting, same-line grouping, and targeted barriers only when justified.

3. Post-compile mismatch
- The emitted asm is structurally identical, but register names or small scheduling
  details still differ from the target compiler/assembler behavior.
- Fix this with `regfix` or maspsx-oriented investigation, not by endlessly
  degrading the source.

## Preferred Workflow

1. Establish semantic baseline
- Read the original function asm.
- Write or recover the cleanest C that preserves the target control flow.
- Do not start from permuter garbage if it breaks meaning.

2. Classify the remaining diff
- Use per-function debug diff and byte checks.
- When a text asm dump and the scorer disagree, trust `target.o` and the live
  score path over a possibly stale `target.s`.
- Ask: is the remaining drift mostly branches, statement order, or registers?

3. Build a reduced experiment
- Isolate only the stubborn region in a scratch or lab file.
- Keep inputs, calls, and live values that matter for the allocator.
- Use this reduced case to test declaration order, temp aliasing, grouping, and
  barrier behavior.

4. Promote only defensible improvements
- If a permuter result improves score but invents dead stores, duplicated writes,
  unreachable code, or nonsense reloads, reject it.
- If a change is semantically neutral and improves the diff, keep it and rerun from
  that new baseline.

5. Escalate to manual PERM search
- When random permuter runs plateau, switch to hand-authored `PERM_GENERAL`,
  `PERM_LINESWAP`, `PERM_FORCE_SAMELINE`, `PERM_ONCE`, and tightly scoped
  `PERM_RANDOMIZE` around the hot region only.

6. Escalate to regfix when warranted
- If the C is structurally right and only allocator/scheduler differences remain,
  use `regfix`.
- This repo already accepts that workflow. It is not cheating if the function is
  otherwise properly decompiled.

## Practical Heuristics

Useful late-stage levers:
- Declaration order
- Temp variable introduction or splitting
- Reusing a typed alias instead of repeated casts
- Preloading a value before overwriting a live register-like variable
- Grouping a block with `do { ... } while (0)` or same-line shaping
- Preserving repeated loads instead of caching
- Matching signed vs unsigned reloads exactly
- Matching pointer base choice, even if two expressions are equivalent

Levers to use carefully:
- `asm("")` barriers
- `register ... asm("sX")` pinning
- Artificial gotos or dead stores

These can help, but they often overconstrain GCC and make the source worse. Use
them only after identifying a specific allocator problem.

## Exit Criteria

A function is ready to move out of the lab when one of these is true:

1. It matches cleanly in C.
2. It is semantically correct in C and only needs a small, documented `regfix`.
3. It is still semantically uncertain, in which case more asm reading or runtime
   validation is needed before further matching work.

## Tools To Prefer

- `tools/check_func.py` for isolated byte checks
- `tools/dump_rtl.sh` for GCC behavior inspection
- `tools/test_decl_order.py`, `tools/test_newvar_split.py`, and
  `tools/test_asm_barrier.py` as examples of focused compiler experiments
- `tools/regfix.py` and `regfix.txt` for accepted post-compile correction
- `objdiff` as an optional improvement for object-level diffing and register/value
  inspection

## Strategy For `tslPrintScreen`

Current read:
- The function is already in late stage.
- Recent score wins came from semantically harmless codegen nudges, not logic
  rediscovery.
- That means the next work should focus on reduced-region experiments and manual
  variation search, not blind whole-function random churn.

Plan:
- Keep an isolated Codex lab copy.
- Use the best sane current baseline as the source of truth.
- Identify the remaining hot regions:
  - sentinel/token handling at the top and loop bottom
  - `func_80052C10` call neighborhood
  - `s2` setup and `0x18/0x14` store ordering
  - the grouped block after `func_8003FA24`
  - marker-scan tail
- Test changes locally in the lab first.
