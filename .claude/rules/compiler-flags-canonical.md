---
name: compiler-flags-canonical
paths: [".claude/rules/compiler-flags-canonical.md"]
description: "Compiler FLAGS are a dead avenue for unmatched functions. -O2 (+ per-file GP_FILES and NO_SR_FILES exceptions) is PROVEN canonical for every file with remaining work. Don't flag-hunt; the walls are source-structure."
metadata:
  type: reference
---

# Compiler flags are canonical — do not flag-hunt

> **Historical framing note (2026-08-30):** this rule predates the removal of the
> regfix/asmfix rule system (retired at zero rules; machinery deleted). Where the
> symptom text says a function "carries a rule", read it as "the honest build shows
> this diff shape vs target". The technique itself is unchanged.

When a function won't match, it is tempting to wonder "did the original build use
different compiler flags than ours?" **It did not.** This was settled with a
project-wide proof (2026-05-20). Do not spend time toggling `-O` levels or
`-f*` flags hoping a remaining function will fall out — the divergence is always
in the C **source structure** (scheduling, register allocation, cross-jump
merging — see [[cross-jump-call-merge]]), never in flags.

## The proof (closed loop)

1. **GCC 2.7.2 has per-translation-unit flags only.** No `#pragma optimize`, no
   per-function flag mechanism. A function's flags are its `.c` file's flags.
2. **Every `.c` file containing remaining work also contains many byte-exact
   `-O2` matches.** Coverage scan (`tmp/flag_coverage.py`): main.c = 95 compiled
   matches + 17 bridged; text1b.c = 277 + 71; code6cac.c = 89 + 17; etc. **Zero**
   files have remaining work without compiled matches.
3. If a file held a sub-group compiled with *different* flags, those functions
   would NOT match at our `-O2` — **but all non-bridged functions in every file
   match.** So each file is provably **flag-uniform at `-O2`**.
4. Therefore the bridged/cheated functions share their file's flags (`-O2`); no
   alternative flag can change their canonical output.

## `-mel` (2026-08-04) — part of the canonical flag set, not a flag-hunt precedent

`-mel` was added to `CC_FLAGS`/`CC_FLAGS_GP` (Makefile + engine/buildconfig.py)
by owner election on 2026-08-04. It is a CONFIGURATION-FIDELITY correction, not
a tuning flag: the cc1 build's `mips-mips-gnu` triple defaulted to big-endian for
a little-endian game, which (a) shifted every 4-byte reload spill slot to
4 mod 8 where the original compiler provably emitted 0 mod 8 (func_80060E38
nine-session evidence chain, docs/grind/decisions.md 2026-08-03), (b) reversed
struct bitfield allocation ([[bitfield-direction-divergence]] — now resolved),
and (c) emitted big-endian lwl/lwr offsets (the reason the now-retired
`fix_lwl` pipeline stage existed). A 4-build controlled experiment validated
the switch: full-build SHA1 == oracle, func_80060E38 COMPLETED-C with zero
rules, zero regressions after the OTag field-order restore + a 2-rule decBs0
re-fit. This does NOT reopen flag-hunting: the flag set (now including -mel)
remains frozen and the walls remain source-structure.

## The only real per-file flag variation (already encoded)

| Mechanism (Makefile) | Flag | Why |
|---|---|---|
| `GP_FILES` | `-G8` instead of `-G0` | small-data threshold for GP-relative files. **First member: text1a (owner-approved 2026-08-05** — target %gp_rel loads + MEM_IN_STRUCT_P dependence + cost-model proof; PsyQ's ccpsx defaulted to -G8 and the maspsx sdata_syms machinery had been compensating). Census screening rule for further adoptions: a file is G8-safe only if every <=8-byte extern is in-gp-range (sdata_syms.txt) or honestly non-small-typed, and any file-scope `__asm__` is extracted to asm/funcs first (-G8 defers function bodies; top-level asm floats to .text 0). |
| `NO_SR_FILES` | `-fno-strength-reduce` | files where strength-reduction diverges |
| `FIX_LWL_FILES` | RETIRED 2026-08-04 (empty) | fix_lwl XOR-corrected big-endian lwl/lwr offsets; obsolete under -mel |

These ARE the per-file flag variation the original build used. Nothing else.
If a file isn't on these lists, it is plain `-O2 -G0`.

## The 24-flag sweep (empirical, on the hardest case)

saTan0Main was swept across `-O0/-O1/-O2/-O3` + scheduling (`-fno-schedule-insns`,
`-insns2`), `-fno-thread-jumps`, `-fno-delayed-branch`, all four CSE flags,
`-fno-function-cse`, `-fcaller-saves`/`-fno-caller-saves`, `-fno-omit-frame-pointer`,
`-fno-defer-pop`, `-fno-force-mem`, `-fsigned-char`, `-funroll-loops`. Result:
**`-O0` → 9 jalr (cross-jumping off), every `-O1/-O2/-O3` combination → 1 jalr.**
The target's 3 jalr is a *partial* merge that no flag produces — it comes from
source structure. (`tmp/flag_sweep.sh` is the reusable ~10s harness.)

## When flag-hunting is warranted

Effectively never, at our file granularity. The proof above covers all 22 src
files. The only theoretical hole is if the ORIGINAL had finer TU boundaries than
splat's `.c` files AND a bridged function sat in a sub-TU with different flags —
but step 3 rules this out per file (mixed-flag sub-TUs would leave non-bridged
functions unmatched, and none are). Treat "maybe it's the flags" as answered: no.

## Related
- [[no-compiler-divergence]] — the standing HARD RULE: no cc1/maspsx patches, no cc1psx-switch, no fork. The compiler is frozen; this rule (flags) is a corollary.
- [[cross-jump-call-merge]] — the real wall for multi-jalr dispatch functions
- [[compiler-patch-low-roi]] — patching cc1 itself is also low-ROI (measured) — superseded as POLICY by [[no-compiler-divergence]]
- `no-new-regfix-rules` (retired rule, deleted 2026-08-30) — the remaining walls are closed with C structure, not flags
